#include "sabutay_a_increaseContrast/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <numeric>
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

    if (rank == 0) {
      std::string abs_path = ppc::util::GetAbsoluteTaskPath("sabutay_a_increaseContrast", image_file);
      unsigned char *data = stbi_load(abs_path.c_str(), &width, &height, &channels, STBI_rgb);

      if (data == nullptr) {
        width = -1;
      } else {
        image_data.assign(data, data + width * height * channels);
        stbi_image_free(data);
      }
    }

    int dims[3] = {width, height, channels};
    MPI_Bcast(dims, 3, MPI_INT, 0, MPI_COMM_WORLD);
    width = dims[0];
    height = dims[1];
    channels = dims[2];

    if (width <= 0 || height <= 0) {
      continue;
    }

    int image_size = width * height * channels;
    if (rank != 0) {
      image_data.resize(image_size);
    }
    MPI_Bcast(image_data.data(), image_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

    int rows_per_process = height / size;
    int remainder = height % size;
    int start_row = rank * rows_per_process + std::min(rank, remainder);
    int end_row = start_row + rows_per_process + (rank < remainder ? 1 : 0);
    int local_rows = end_row - start_row;

    uint8_t local_min = 255;
    uint8_t local_max = 0;

    for (int row = 0; row < local_rows; row++) {
      int global_row = start_row + row;
      for (int col = 0; col < width; col++) {
        int idx = global_row * width + col;
        uint8_t gray = static_cast<uint8_t>(0.299 * image_data[idx * 3] + 0.587 * image_data[idx * 3 + 1] +
                                            0.114 * image_data[idx * 3 + 2]);
        local_min = std::min(local_min, gray);
        local_max = std::max(local_max, gray);
      }
    }

    uint8_t global_min = 0;
    uint8_t global_max = 0;
    MPI_Reduce(&local_min, &global_min, 1, MPI_UNSIGNED_CHAR, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_max, &global_max, 1, MPI_UNSIGNED_CHAR, MPI_MAX, 0, MPI_COMM_WORLD);

    uint8_t minmax[2] = {global_min, global_max};
    MPI_Bcast(minmax, 2, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
    global_min = minmax[0];
    global_max = minmax[1];

    if (global_max > global_min) {
      const double scale = 255.0 / (global_max - global_min);
      for (int row = 0; row < local_rows; row++) {
        int global_row = start_row + row;
        for (int col = 0; col < width; col++) {
          int idx = global_row * width + col;
          uint8_t gray = static_cast<uint8_t>(0.299 * image_data[idx * 3] + 0.587 * image_data[idx * 3 + 1] +
                                              0.114 * image_data[idx * 3 + 2]);
          static_cast<void>(static_cast<uint8_t>((gray - global_min) * scale));
        }
      }
    }
  }

  GetOutput() = GetInput();

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool SabutayAincreaseContrastMPI::PostProcessingImpl() {
  GetOutput() = GetInput();
  return true;
}

}  // namespace sabutay_a_increaseContrast
