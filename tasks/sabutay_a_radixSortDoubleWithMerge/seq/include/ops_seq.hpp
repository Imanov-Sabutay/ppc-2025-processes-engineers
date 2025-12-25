#pragma once

#include "sabutay_a_radixSortDoubleWithMerge/common/include/common.hpp"
#include "task/include/task.hpp"

namespace sabutay_a_radixSortDoubleWithMerge {

class SabutayAradixSortDoubleWithMergeSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit SabutayAradixSortDoubleWithMergeSEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace sabutay_a_radixSortDoubleWithMerge
