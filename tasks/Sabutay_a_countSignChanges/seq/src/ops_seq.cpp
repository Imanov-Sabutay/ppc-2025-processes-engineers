#include "sabutay_a_countSignChanges/seq/include/ops_seq.hpp"

#include <numeric>
#include <vector>

#include "sabutay_a_countSignChanges/common/include/common.hpp"
#include "util/include/util.hpp"

namespace sabutay_a_countSignChanges {

SabutayACountSignChangesSEQ::SabutayACountSignChangesSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool SabutayACountSignChangesSEQ::ValidationImpl() {
  return (GetInput() > 0) && (GetOutput() == 0);
}

bool SabutayACountSignChangesSEQ::PreProcessingImpl() {
  GetOutput() = 0;
  return true;
}

bool SabutayACountSignChangesSEQ::RunImpl() {
  // Create a vector of size (GetInput() + 1) with alternating signs
  std::vector<InType> vec;
  vec.reserve(GetInput() + 1);

  for (InType i = 0; i <= GetInput(); i++) {
    vec.push_back((i % 2 == 0) ? (i + 1) : -(i + 1));
  }

  // Count sign changes between adjacent elements
  InType sign_changes = 0;
  for (size_t i = 0; i < vec.size() - 1; i++) {
    if ((vec[i] > 0 && vec[i + 1] < 0) || (vec[i] < 0 && vec[i + 1] > 0)) {
      sign_changes++;
    }
  }

  GetOutput() = sign_changes;
  return true;
}

bool SabutayACountSignChangesSEQ::PostProcessingImpl() {
  // No post-processing needed
  return true;
}

}  // namespace sabutay_a_countSignChanges
