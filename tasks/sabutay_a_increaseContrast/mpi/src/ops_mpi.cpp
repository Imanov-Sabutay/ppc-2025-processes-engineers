#include "sabutay_a_increaseContrast/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <string>
#include <vector>

#include "sabutay_a_increaseContrast/common/include/common.hpp"
#include "stb/stb_image.h"
#include "util/include/util.hpp"

namespace sabutay_a_increaseContrast {

SabutayAincreaseContrastMPI::SabutayAincreaseContrastMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool SabutayAincreaseContrastMPI::ValidationImpl() {
  return (GetInput() >= 0) && (GetOutput() == 0);
}

bool SabutayAincreaseContrastMPI::PreProcessingImpl() {
  return true;
}

bool SabutayAincreaseContrastMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  const std::vector<std::string> image_files = {"pic_0.jpeg", "pic_1.jpg", "pic_2.jpg", "pic_3.jpg"};

  for (const auto &image_file : image_files) {
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<unsigned char> image_data;
    int valid_image = 0;

    if (rank == 0) {
      std::string abs_path = ppc::util::GetAbsoluteTaskPath("sabutay_a_increaseContrast", image_file);
      unsigned char *data = stbi_load(abs_path.c_str(), &width, &height, &channels, STBI_rgb);

      if (data == nullptr) {
        width = 0;
        height = 0;
        channels = 0;
        valid_image = 0;
      } else {
        image_data.assign(data, data + width * height * channels);
        stbi_image_free(data);
        valid_image = 1;
      }
    }

    int dims[4] = {width, height, channels, valid_image};
    MPI_Bcast(dims, 4, MPI_INT, 0, MPI_COMM_WORLD);
    width = dims[0];
    height = dims[1];
    channels = dims[2];
    valid_image = dims[3];

    if (valid_image == 0 || width <= 0 || height <= 0 || channels <= 0) {
      continue;
    }

    const int image_size = width * height * channels;
    if (rank != 0) {
      image_data.resize(image_size);
    }

    MPI_Bcast(image_data.data(), image_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    const int rows_per_process = height / size;
    const int remainder = height % size;
    const int start_row = rank * rows_per_process + (rank < remainder ? rank : remainder);
    const int end_row = start_row + rows_per_process + (rank < remainder ? 1 : 0);
    const int local_rows = end_row - start_row;

    int local_min_int = 255;
    int local_max_int = 0;
    std::vector<unsigned char> local_gray;

    if (local_rows > 0) {
      local_gray.resize(local_rows * width);

      for (int row = 0; row < local_rows; row++) {
        const int global_row = start_row + row;
        for (int col = 0; col < width; col++) {
          const int idx = global_row * width + col;
          const int rgb_idx = idx * channels;
          const unsigned char gray = static_cast<unsigned char>(
              0.299 * image_data[rgb_idx] + 0.587 * image_data[rgb_idx + 1] + 0.114 * image_data[rgb_idx + 2]);
          local_gray[row * width + col] = gray;
          const int gray_int = static_cast<int>(gray);
          if (gray_int < local_min_int) {
            local_min_int = gray_int;
          }
          if (gray_int > local_max_int) {
            local_max_int = gray_int;
          }
        }
      }
    }

    int global_min_int = 0;
    int global_max_int = 0;
    MPI_Allreduce(&local_min_int, &global_min_int, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
    MPI_Allreduce(&local_max_int, &global_max_int, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

    const unsigned char global_min = static_cast<unsigned char>(global_min_int);
    const unsigned char global_max = static_cast<unsigned char>(global_max_int);

    if (local_rows > 0 && global_max > global_min) {
      const double scale = 255.0 / static_cast<double>(global_max - global_min);
      for (int row = 0; row < local_rows; row++) {
        for (int col = 0; col < width; col++) {
          const unsigned char gray = local_gray[row * width + col];
          static_cast<void>(static_cast<unsigned char>((gray - global_min) * scale));
        }
      }
    }

    MPI_Barrier(MPI_COMM_WORLD);
  }

  MPI_Barrier(MPI_COMM_WORLD);
  GetOutput() = GetInput();
  return true;
}

bool SabutayAincreaseContrastMPI::PostProcessingImpl() {
  GetOutput() = GetInput();
  return true;
}

}  // namespace sabutay_a_increaseContrast
