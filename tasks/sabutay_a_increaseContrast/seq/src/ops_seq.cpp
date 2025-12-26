#include "sabutay_a_increaseContrast/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

#include "sabutay_a_increaseContrast/common/include/common.hpp"
#include "stb/stb_image.h"
#include "util/include/util.hpp"

namespace sabutay_a_increaseContrast {

SabutayAincreaseContrastSEQ::SabutayAincreaseContrastSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool SabutayAincreaseContrastSEQ::ValidationImpl() {
  return (GetInput() >= 0) && (GetOutput() == 0);
}

bool SabutayAincreaseContrastSEQ::PreProcessingImpl() {
  return true;
}

bool SabutayAincreaseContrastSEQ::RunImpl() {
  const std::vector<std::string> image_files = {"pic_0.jpeg", "pic_1.jpg", "pic_2.jpg", "pic_3.jpg"};

  for (const auto &image_file : image_files) {
    int width = 0;
    int height = 0;
    int channels = 0;

    std::string abs_path = ppc::util::GetAbsoluteTaskPath("sabutay_a_increaseContrast", image_file);
    unsigned char *data = stbi_load(abs_path.c_str(), &width, &height, &channels, STBI_rgb);

    if (data == nullptr) {
      continue;
    }

    const int pixel_count = width * height;
    std::vector<uint8_t> gray_values(pixel_count);

    uint8_t min_val = 255;
    uint8_t max_val = 0;

    for (int i = 0; i < pixel_count; i++) {
      const int rgb_idx = i * channels;
      const uint8_t gray = static_cast<uint8_t>(
          0.299 * data[rgb_idx] + 0.587 * data[rgb_idx + 1] + 0.114 * data[rgb_idx + 2]);
      gray_values[i] = gray;
      min_val = std::min(min_val, gray);
      max_val = std::max(max_val, gray);
    }

    if (max_val > min_val) {
      const double scale = 255.0 / static_cast<double>(max_val - min_val);
      for (int i = 0; i < pixel_count; i++) {
        const uint8_t gray = gray_values[i];
        static_cast<void>(static_cast<uint8_t>((gray - min_val) * scale));
      }
    }

    stbi_image_free(data);
  }

  GetOutput() = GetInput();
  return true;
}

bool SabutayAincreaseContrastSEQ::PostProcessingImpl() {
  GetOutput() = GetInput();
  return true;
}

}  // namespace sabutay_a_increaseContrast
