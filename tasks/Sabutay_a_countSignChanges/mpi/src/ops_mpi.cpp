#include "sabutay_a_countSignChanges/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <vector>

#include "sabutay_a_countSignChanges/common/include/common.hpp"

namespace sabutay_a_countSignChanges {

SabutayACountSignChangesMPI::SabutayACountSignChangesMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool SabutayACountSignChangesMPI::ValidationImpl() {
  return (GetInput() > 0) && (GetOutput() == 0);
}

bool SabutayACountSignChangesMPI::PreProcessingImpl() {
  GetOutput() = 0;
  return true;
}

bool SabutayACountSignChangesMPI::RunImpl() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // For empty vector, return 0
  if (GetInput() == 0) {
    GetOutput() = 0;
    MPI_Barrier(MPI_COMM_WORLD);
    return true;
  }

  // Create the full vector with alternating signs
  std::vector<InType> full_vec(GetInput() + 1);
  for (InType i = 0; i <= GetInput(); i++) {
    full_vec[i] = (i % 2 == 0) ? (i + 1) : -(i + 1);
  }

  // Calculate work distribution
  const InType pairs_count = GetInput(); // n+1 elements create n pairs
  const InType pairs_per_process = (pairs_count + size - 1) / size; // Round up
  const InType start_pair = rank * pairs_per_process;
  const InType end_pair = std::min(start_pair + pairs_per_process, pairs_count);

  // Count local sign changes
  InType local_count = 0;
  for (InType i = start_pair; i < end_pair && i < pairs_count; i++) {
    if ((full_vec[i] > 0 && full_vec[i + 1] < 0) || (full_vec[i] < 0 && full_vec[i + 1] > 0)) {
      local_count++;
    }
  }

  // Reduce all local counts to process 0
  InType total_count = 0;
  MPI_Reduce(&local_count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  // Broadcast the result to all processes
  MPI_Bcast(&total_count, 1, MPI_INT, 0, MPI_COMM_WORLD);

  GetOutput() = total_count;
  return true;
}

bool SabutayACountSignChangesMPI::PostProcessingImpl() {
  // No post-processing needed
  return true;
}

}  // namespace sabutay_a_countSignChanges
