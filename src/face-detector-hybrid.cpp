#include <obs-module.h>
#include <util/platform.h>
#include <util/threading.h>
#include <algorithm>
#include <cmath>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/objdetect.hpp>
#include "plugin-macros.generated.h"
#include "face-detector-hybrid.h"
#include "texture-object.h"

#define MAX_ERROR 2

struct face_detector_hybrid::private_s
{
	std::shared_ptr<texture_object> tex;
	std::vector<detection_s> results;
	std::string yunet_path;
	std::string nanodet_path;
	cv::Ptr<cv::FaceDetectorYN> yunet;
	cv::dnn::Net nanodet;
	bool yunet_loaded = false;
	bool nanodet_loaded = false;
	int crop_l = 0, crop_r = 0, crop_t = 0, crop_b = 0;
	int n_error = 0;
};

static cv::Rect2f clamp_box(const cv::Rect2f &box, int img_w, int img_h)
{
	float x1 = std::max(0.0f, box.x);
	float y1 = std::max(0.0f, box.y);
	float x2 = std::min((float)img_w - 1.0f, box.x + box.width);
	float y2 = std::min((float)img_h - 1.0f, box.y + box.height);
	return cv::Rect2f(x1, y1, x2 - x1, y2 - y1);
}

static rect_s estimate_face_from_body(const rect_s &body)
{
	float bw = (float)(body.x1 - body.x0);
	float bh = (float)(body.y1 - body.y0);
	if (bw <= 0.0f || bh <= 0.0f)
		return body;

	float aspect = bw / bh;
	float clamped = std::clamp(aspect, 0.6f, 1.2f);
	float t = (1.2f - clamped) / 0.6f;
	float face_y_pct = 0.18f + t * 0.17f; // 18% (standing) ~ 35% (sitting)

	float face_cx = (float)(body.x0 + body.x1) * 0.5f;
	float face_cy = (float)body.y0 + bh * face_y_pct;
	float face_size = bw * 0.35f;
	float hw = face_size * 0.5f;

	rect_s r;
	r.x0 = (int)(face_cx - hw);
	r.y0 = (int)(face_cy - hw);
	r.x1 = (int)(face_cx + hw);
	r.y1 = (int)(face_cy + hw);
	r.score = body.score * 0.8f;
	return r;
}

static float dfl_distance(const float *logits)
{
	float max_logit = logits[0];
	for (int i = 1; i < 8; i++)
		max_logit = std::max(max_logit, logits[i]);

	float sum = 0.0f;
	float weighted_sum = 0.0f;
	for (int i = 0; i < 8; i++) {
		float value = std::exp(logits[i] - max_logit);
		sum += value;
		weighted_sum += value * (float)i;
	}
	return weighted_sum / sum;
}

face_detector_hybrid::face_detector_hybrid()
{
	p = new private_s;
}

face_detector_hybrid::~face_detector_hybrid()
{
	delete p;
}

void face_detector_hybrid::set_texture(std::shared_ptr<texture_object> &tex, int crop_l, int crop_r, int crop_t,
				       int crop_b)
{
	p->tex = tex;
	p->crop_l = crop_l;
	p->crop_r = crop_r;
	p->crop_t = crop_t;
	p->crop_b = crop_b;
}

void face_detector_hybrid::get_faces(std::vector<struct detection_s> &detections)
{
	detections = p->results;
}

void face_detector_hybrid::set_yunet_model(const char *filename)
{
	if (p->yunet_path != filename) {
		p->yunet_path = filename;
		p->yunet_loaded = false;
		p->yunet.release();
	}
}

void face_detector_hybrid::set_nanodet_model(const char *filename)
{
	if (p->nanodet_path != filename) {
		p->nanodet_path = filename;
		p->nanodet_loaded = false;
		p->nanodet = cv::dnn::Net();
	}
}

void face_detector_hybrid::detect_main()
{
	p->results.clear();
	if (!p->tex)
		return;

	const uint8_t *frame_data = nullptr;
	int width = 0, height = 0, linesize = 0;
	float coordinate_scale = 1.0f;
	video_format fmt;
	if (!p->tex->get_raw_frame(frame_data, width, height, linesize, fmt, coordinate_scale))
		return;

	if (width <= 0 || height <= 0)
		return;
	if (fmt != VIDEO_FORMAT_BGR3 && fmt != VIDEO_FORMAT_BGRX && fmt != VIDEO_FORMAT_BGRA &&
	    fmt != VIDEO_FORMAT_RGBA) {
		blog(LOG_ERROR, "hybrid: unsupported frame format %d", (int)fmt);
		p->tex.reset();
		return;
	}

	int x0 = 0, y0 = 0, x1 = width, y1 = height;
	if (p->crop_l > 0 || p->crop_r > 0 || p->crop_t > 0 || p->crop_b > 0) {
		x0 = (int)(p->crop_l / coordinate_scale);
		y0 = (int)(p->crop_t / coordinate_scale);
		x1 = width - (int)(p->crop_r / coordinate_scale);
		y1 = height - (int)(p->crop_b / coordinate_scale);
		if (x1 - x0 < 80 || y1 - y0 < 80) {
			if (p->n_error++ < MAX_ERROR)
				blog(LOG_ERROR, "hybrid: too small image cropped %dx%d", x1 - x0, y1 - y0);
			return;
		} else if (p->n_error) {
			p->n_error--;
		}
	}

	cv::Mat frame_bgr;
	cv::Mat frame_cropped;
	int type = fmt == VIDEO_FORMAT_BGR3 ? CV_8UC3 : CV_8UC4;
	cv::Mat frame(height, width, type, (void *)frame_data, (size_t)linesize);
	if (x0 > 0 || y0 > 0 || x1 < width || y1 < height) {
		cv::Rect crop_rect(x0, y0, x1 - x0, y1 - y0);
		frame_cropped = frame(crop_rect).clone();
	} else {
		frame.copyTo(frame_cropped);
	}
	switch (fmt) {
	case VIDEO_FORMAT_BGR3:
		frame_bgr = frame_cropped;
		break;
	case VIDEO_FORMAT_BGRX:
	case VIDEO_FORMAT_BGRA:
		cv::cvtColor(frame_cropped, frame_bgr, cv::COLOR_BGRA2BGR);
		break;
	case VIDEO_FORMAT_RGBA:
		cv::cvtColor(frame_cropped, frame_bgr, cv::COLOR_RGBA2BGR);
		break;
	default:
		return;
	}

	int crop_w = frame_bgr.cols;
	int crop_h = frame_bgr.rows;

	// --- load models lazily ---
	if (!p->yunet_loaded && !p->yunet_path.empty()) {
		p->yunet_loaded = true;
		try {
			p->yunet = cv::FaceDetectorYN::create(p->yunet_path, "", cv::Size(320, 320), 0.6f, 0.3f, 5000,
							      cv::dnn::DNN_BACKEND_DEFAULT, cv::dnn::DNN_TARGET_CPU);
			blog(LOG_INFO, "hybrid: loaded YuNet from '%s'", p->yunet_path.c_str());
		} catch (const cv::Exception &e) {
			blog(LOG_ERROR, "hybrid: failed to load YuNet from '%s': %s", p->yunet_path.c_str(), e.what());
			p->yunet.release();
		}
	}

	if (!p->nanodet_loaded && !p->nanodet_path.empty()) {
		p->nanodet_loaded = true;
		try {
			p->nanodet = cv::dnn::readNet(p->nanodet_path);
			p->nanodet.setPreferableBackend(cv::dnn::DNN_BACKEND_DEFAULT);
			p->nanodet.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
			blog(LOG_INFO, "hybrid: loaded NanoDet from '%s'", p->nanodet_path.c_str());
		} catch (const cv::Exception &e) {
			blog(LOG_ERROR, "hybrid: failed to load NanoDet from '%s': %s", p->nanodet_path.c_str(),
			     e.what());
			p->nanodet = cv::dnn::Net();
		}
	}

	// --- Step 1: YuNet face detection ---
	if (!p->yunet.empty()) {
		cv::Mat input, faces;
		cv::resize(frame_bgr, input, cv::Size(320, 320));
		p->yunet->setInputSize(input.size());
		p->yunet->detect(input, faces);
		float scale_x = (float)crop_w / 320.0f;
		float scale_y = (float)crop_h / 320.0f;

		for (int i = 0; i < faces.rows; i++) {
			const float *face = faces.ptr<float>(i);
			float conf = face[14];
			float fx1 = face[0] * scale_x;
			float fy1 = face[1] * scale_y;
			float fx2 = (face[0] + face[2]) * scale_x;
			float fy2 = (face[1] + face[3]) * scale_y;

			cv::Rect2f box(fx1, fy1, fx2 - fx1, fy2 - fy1);
			box = clamp_box(box, crop_w, crop_h);
			if (box.width < 10.0f || box.height < 10.0f)
				continue;

			detection_s det;
			det.rect.x0 = (int)((box.x + x0) * coordinate_scale);
			det.rect.y0 = (int)((box.y + y0) * coordinate_scale);
			det.rect.x1 = (int)((box.x + box.width + x0) * coordinate_scale);
			det.rect.y1 = (int)((box.y + box.height + y0) * coordinate_scale);
			det.rect.score = conf;
			det.source = detection_source_e::source_hybrid_yunet;
			det.confidence = conf;
			det.original_box = det.rect;
			p->results.push_back(det);
		}
	}

	// --- Step 2: NanoDet body detection (always run, merge if no face found) ---
	if (!p->nanodet.empty()) {
		cv::Mat input;
		cv::resize(frame_bgr, input, cv::Size(416, 416));
		input.convertTo(input, CV_32FC3);
		std::vector<cv::Mat> channels;
		cv::split(input, channels);
		channels[0] = (channels[0] - 103.53f) / 57.375f;
		channels[1] = (channels[1] - 116.28f) / 57.12f;
		channels[2] = (channels[2] - 123.675f) / 58.395f;
		cv::merge(channels, input);
		cv::Mat blob = cv::dnn::blobFromImage(input, 1.0, cv::Size(), cv::Scalar(), false, false);
		p->nanodet.setInput(blob);
		cv::Mat nanodet_out = p->nanodet.forward();

		std::vector<rect_s> person_boxes;
		if (nanodet_out.dims == 3 && nanodet_out.size[2] == 112) {
			std::vector<cv::Rect> boxes;
			std::vector<float> scores;
			const int grid_sizes[] = {52, 26, 13, 7};
			const int strides[] = {8, 16, 32, 64};
			int offset = 0;
			for (int level = 0; level < 4; level++) {
				int grid = grid_sizes[level];
				int stride = strides[level];
				for (int index = 0; index < grid * grid; index++) {
					const float *row = nanodet_out.ptr<float>(0, offset + index);
					float score = row[0]; // COCO class 0 is person.
					if (score < 0.5f)
						continue;
					float cx = (float)(index % grid) * stride;
					float cy = (float)(index / grid) * stride;
					float left = dfl_distance(row + 80) * stride;
					float top = dfl_distance(row + 88) * stride;
					float right = dfl_distance(row + 96) * stride;
					float bottom = dfl_distance(row + 104) * stride;
					int bx1 = std::clamp((int)(cx - left), 0, 415);
					int by1 = std::clamp((int)(cy - top), 0, 415);
					int bx2 = std::clamp((int)(cx + right), 0, 415);
					int by2 = std::clamp((int)(cy + bottom), 0, 415);
					if (bx2 > bx1 && by2 > by1) {
						boxes.emplace_back(bx1, by1, bx2 - bx1, by2 - by1);
						scores.push_back(score);
					}
				}
				offset += grid * grid;
			}

			std::vector<int> kept;
			cv::dnn::NMSBoxes(boxes, scores, 0.5f, 0.6f, kept, 1.0f, 100);
			for (int index : kept) {
				const cv::Rect &box = boxes[index];
				rect_s body;
				body.x0 = box.x * crop_w / 416;
				body.y0 = box.y * crop_h / 416;
				body.x1 = (box.x + box.width) * crop_w / 416;
				body.y1 = (box.y + box.height) * crop_h / 416;
				body.score = scores[index];
				person_boxes.push_back(body);
			}
		} else {
			blog(LOG_ERROR, "hybrid: unsupported NanoDet output shape");
		}

		// Sort by confidence descending
		std::sort(person_boxes.begin(), person_boxes.end(),
			  [](const rect_s &a, const rect_s &b) { return a.score > b.score; });

		// Add body boxes as person detections
		for (size_t i = 0; i < person_boxes.size(); i++) {
			detection_s det_p;
			det_p.rect = person_boxes[i];
			det_p.rect.x0 = (int)((det_p.rect.x0 + x0) * coordinate_scale);
			det_p.rect.y0 = (int)((det_p.rect.y0 + y0) * coordinate_scale);
			det_p.rect.x1 = (int)((det_p.rect.x1 + x0) * coordinate_scale);
			det_p.rect.y1 = (int)((det_p.rect.y1 + y0) * coordinate_scale);
			det_p.source = detection_source_e::source_hybrid_nanodet_person;
			det_p.confidence = person_boxes[i].score;
			det_p.original_box = det_p.rect;
			p->results.push_back(det_p);

			// Add estimated face from body
			bool have_face_in_body =
				std::any_of(p->results.begin(), p->results.end(), [&](const detection_s &d) {
					if (d.source != detection_source_e::source_hybrid_yunet)
						return false;
					int face_x = (d.rect.x0 + d.rect.x1) / 2;
					int face_y = (d.rect.y0 + d.rect.y1) / 2;
					return face_x >= det_p.rect.x0 && face_x <= det_p.rect.x1 &&
					       face_y >= det_p.rect.y0 && face_y <= det_p.rect.y1;
				});
			if (have_face_in_body)
				continue;

			rect_s face_est = estimate_face_from_body(person_boxes[i]);
			face_est.x0 = (int)((face_est.x0 + x0) * coordinate_scale);
			face_est.y0 = (int)((face_est.y0 + y0) * coordinate_scale);
			face_est.x1 = (int)((face_est.x1 + x0) * coordinate_scale);
			face_est.y1 = (int)((face_est.y1 + y0) * coordinate_scale);
			detection_s det_f;
			det_f.rect = face_est;
			det_f.source = detection_source_e::source_hybrid_nanodet_estimated;
			det_f.confidence = person_boxes[i].score * 0.8f;
			det_f.original_box = det_p.original_box;
			p->results.push_back(det_f);
		}
	}

	p->tex.reset();
}
