#include "sabutay_a_calculateSignChanges/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <vector>

#include "sabutay_a_calculateSignChanges/common/include/common.hpp"
#include "util/include/util.hpp"

namespace sabutay_a_calculateSignChanges {

SabutayAcalculateSignChangesMPI::SabutayAcalculateSignChangesMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool SabutayAcalculateSignChangesMPI::ValidationImpl() {
  return (GetInput() >= 0) && (GetOutput() == 0);
}

bool SabutayAcalculateSignChangesMPI::PreProcessingImpl() {
  return true;
}

bool SabutayAcalculateSignChangesMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  const InType input_size = GetInput();

  if (input_size <= 1) {
    GetOutput() = GetInput();
    MPI_Barrier(MPI_COMM_WORLD);
    return true;
  }

  int elements_per_process = input_size / size;
  int remainder = input_size % size;
  int start_idx = rank * elements_per_process + std::min(rank, remainder);
  int end_idx = start_idx + elements_per_process + (rank < remainder ? 1 : 0);

  InType local_sign_changes = 0;

  for (int i = start_idx; i < end_idx - 1 && i < input_size - 1; i++) {
    InType val1 = (i % 2 == 0) ? (i + 1) : -(i + 1);
    InType val2 = ((i + 1) % 2 == 0) ? (i + 2) : -(i + 2);
    if ((val1 > 0 && val2 < 0) || (val1 < 0 && val2 > 0)) {
      local_sign_changes++;
    }
  }

  InType global_sign_changes = 0;
  MPI_Reduce(&local_sign_changes, &global_sign_changes, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  GetOutput() = GetInput();

  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}

bool SabutayAcalculateSignChangesMPI::PostProcessingImpl() {
  GetOutput() = GetInput();
  return true;
}

}  // namespace sabutay_a_calculateSignChanges
