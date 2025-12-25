#pragma once

#include "sabutay_a_calculateSignChanges/common/include/common.hpp"
#include "task/include/task.hpp"

namespace sabutay_a_calculateSignChanges {

class SabutayAcalculateSignChangesSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit SabutayAcalculateSignChangesSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace sabutay_a_calculateSignChanges
