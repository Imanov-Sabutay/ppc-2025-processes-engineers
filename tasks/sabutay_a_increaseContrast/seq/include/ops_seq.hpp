#pragma once

#include "sabutay_a_increaseContrast/common/include/common.hpp"
#include "task/include/task.hpp"

namespace sabutay_a_increaseContrast {

class SabutayAincreaseContrastSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit SabutayAincreaseContrastSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace sabutay_a_increaseContrast
