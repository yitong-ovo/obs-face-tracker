#pragma once

#include <obs-module.h>
#include <util/platform.h>
#include <util/threading.h>
#include <memory>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include "plugin-macros.generated.h"
#include "face-detector-base.h"

class face_detector_hybrid : public face_detector_base {
	struct private_s;
	private_s *p;

	void detect_main() override;

public:
	face_detector_hybrid();
	virtual ~face_detector_hybrid();
	void set_texture(std::shared_ptr<texture_object> &, int crop_l, int crop_r, int crop_t, int crop_b) override;
	void get_faces(std::vector<struct detection_s> &) override;

	void set_yunet_model(const char *filename);
	void set_nanodet_model(const char *filename);
};
