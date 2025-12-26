#include <gtest/gtest.h>

#include "sabutay_a_increaseContrast/common/include/common.hpp"
#include "sabutay_a_increaseContrast/mpi/include/ops_mpi.hpp"
#include "sabutay_a_increaseContrast/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace sabutay_a_increaseContrast {

class SabutayAincreaseContrastPerfTest : public ppc::util::BaseRunPerfTests<InType, OutType> {
  const int kCount_ = 100;
  InType input_data_{};

  void SetUp() override {
    input_data_ = kCount_;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return input_data_ == output_data;
  }

  InType GetTestInputData() final {
    return input_data_;
  }
};

TEST_P(SabutayAincreaseContrastPerfTest, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, SabutayAincreaseContrastMPI, SabutayAincreaseContrastSEQ>(PPC_SETTINGS_sabutay_a_increaseContrast);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = SabutayAincreaseContrastPerfTest::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, SabutayAincreaseContrastPerfTest, kGtestValues, kPerfTestName);

}  // namespace sabutay_a_increaseContrast

