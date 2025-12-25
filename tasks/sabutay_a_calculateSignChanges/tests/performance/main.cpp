#include <gtest/gtest.h>

#include "sabutay_a_calculateSignChanges/common/include/common.hpp"
#include "sabutay_a_calculateSignChanges/mpi/include/ops_mpi.hpp"
#include "sabutay_a_calculateSignChanges/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace sabutay_a_calculateSignChanges {

class SabutayAcalculateSignChangesPerfTest : public ppc::util::BaseRunPerfTests<InType, OutType> {
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

TEST_P(SabutayAcalculateSignChangesPerfTest, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, SabutayAcalculateSignChangesMPI, SabutayAcalculateSignChangesSEQ>(PPC_SETTINGS_sabutay_a_calculateSignChanges);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = SabutayAcalculateSignChangesPerfTest::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, SabutayAcalculateSignChangesPerfTest, kGtestValues, kPerfTestName);

}  // namespace sabutay_a_calculateSignChanges
