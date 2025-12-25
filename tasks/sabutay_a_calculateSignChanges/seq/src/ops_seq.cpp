#include "sabutay_a_calculateSignChanges/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <vector>

#include "sabutay_a_calculateSignChanges/common/include/common.hpp"
#include "util/include/util.hpp"

namespace sabutay_a_calculateSignChanges {

SabutayAcalculateSignChangesSEQ::SabutayAcalculateSignChangesSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool SabutayAcalculateSignChangesSEQ::ValidationImpl() {
  return (GetInput() >= 0) && (GetOutput() == 0);
}

bool SabutayAcalculateSignChangesSEQ::PreProcessingImpl() {
  return true;
}

bool SabutayAcalculateSignChangesSEQ::RunImpl() {
  if (GetInput() <= 1) {
    GetOutput() = GetInput();
    return true;
  }

  const InType size = GetInput();
  InType sign_changes = 0;

  for (InType i = 0; i < size - 1; i++) {
    InType val1 = (i % 2 == 0) ? (i + 1) : -(i + 1);
    InType val2 = ((i + 1) % 2 == 0) ? (i + 2) : -(i + 2);
    if ((val1 > 0 && val2 < 0) || (val1 < 0 && val2 > 0)) {
      sign_changes++;
    }
  }

  GetOutput() = GetInput();
  return true;
}

bool SabutayAcalculateSignChangesSEQ::PostProcessingImpl() {
  GetOutput() = GetInput();
  return true;
}

}  // namespace sabutay_a_calculateSignChanges
