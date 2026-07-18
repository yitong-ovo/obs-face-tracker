#!/bin/bash
set -euo pipefail
BASE="$(dirname "$0")/../data/hybrid"
YUNET_SHA256="8f2383e4dd3cfbb4553ea8718107fc0423210dc964f9f4280604804ed2552fa4"
NANODET_SHA256="59d2f166088889c902f523bf08079391993491324f0d84847e3c4016a8f7cc3d"

verify_sha256() {
	local expected="$1" file="$2" actual
	if command -v sha256sum >/dev/null 2>&1; then
		actual="$(sha256sum "$file" | cut -d ' ' -f 1)"
	elif command -v shasum >/dev/null 2>&1; then
		actual="$(shasum -a 256 "$file" | cut -d ' ' -f 1)"
	else
		echo "Error: sha256sum or shasum is required." >&2
		return 1
	fi
	if [ "$actual" != "$expected" ]; then
		echo "Error: SHA-256 mismatch for $file" >&2
		echo "Expected: $expected" >&2
		echo "Actual:   $actual" >&2
		return 1
	fi
}

download_model() {
	local name="$1" url="$2" output="$3" expected="$4"
	local tmp="${output}.tmp"
	if [ -f "$output" ] && verify_sha256 "$expected" "$output" >/dev/null 2>&1; then
		echo "[SKIP] $name already exists and passed checksum verification: $output"
		return
	fi
	rm -f "$tmp"
	echo "[DOWNLOAD] $name..."
	curl --fail --location --retry 3 --retry-all-errors --output "$tmp" "$url"
	verify_sha256 "$expected" "$tmp"
	mv "$tmp" "$output"
	echo "[OK] $name saved to $output"
}

echo "=== obs-face-tracker hybrid model downloader ==="
echo ""

# --- YuNet (MIT, from OpenCV Zoo) ---
mkdir -p "$BASE/yunet"
YUNET_URL="https://media.githubusercontent.com/media/opencv/opencv_zoo/47534e27c9851bb1128ccc0102f1145e27f23f98/models/face_detection_yunet/face_detection_yunet_2023mar.onnx"
YUNET_FILE="$BASE/yunet/face_detection_yunet_2023mar.onnx"
download_model "YuNet from OpenCV Zoo (MIT)" "$YUNET_URL" "$YUNET_FILE" "$YUNET_SHA256"

# --- NanoDet-Plus-m (Apache 2.0, official release) ---
echo ""
mkdir -p "$BASE/nanodet"
NANODET_FILE="$BASE/nanodet/nanodet-plus-m_416.onnx"
download_model "NanoDet-Plus-m from the official release (Apache 2.0)" \
	"https://github.com/RangiLyu/nanodet/releases/download/v1.0.0-alpha-1/nanodet-plus-m_416.onnx" \
	"$NANODET_FILE" "$NANODET_SHA256"

curl --fail --location --retry 3 --retry-all-errors --output "$BASE/LICENSE-YuNet" \
	"https://raw.githubusercontent.com/opencv/opencv_zoo/47534e27c9851bb1128ccc0102f1145e27f23f98/models/face_detection_yunet/LICENSE"
curl --fail --location --retry 3 --retry-all-errors --output "$BASE/LICENSE-NanoDet" \
	"https://raw.githubusercontent.com/RangiLyu/nanodet/08bad3294608953b1e1a8aacc50f67336420a435/LICENSE"

echo ""
echo "=== Done ==="
echo "Model files expected at:"
echo "  YuNet:    $YUNET_FILE"
echo "  NanoDet:  $NANODET_FILE"
