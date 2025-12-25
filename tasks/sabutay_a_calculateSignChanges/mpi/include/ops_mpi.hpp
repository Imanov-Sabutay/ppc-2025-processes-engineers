#pragma once

#include "sabutay_a_calculateSignChanges/common/include/common.hpp"
#include "task/include/task.hpp"

namespace sabutay_a_calculateSignChanges {

class SabutayAcalculateSignChangesMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit SabutayAcalculateSignChangesMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace sabutay_a_calculateSignChanges
