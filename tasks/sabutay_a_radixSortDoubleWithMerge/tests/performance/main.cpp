#include <gtest/gtest.h>

#include "sabutay_a_radixSortDoubleWithMerge/common/include/common.hpp"
#include "sabutay_a_radixSortDoubleWithMerge/mpi/include/ops_mpi.hpp"
#include "sabutay_a_radixSortDoubleWithMerge/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace sabutay_a_radixSortDoubleWithMerge {

class ExampleRunPerfTestProcesses3 : public ppc::util::BaseRunPerfTests<InType, OutType> {
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

TEST_P(ExampleRunPerfTestProcesses3, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, SabutayAradixSortDoubleWithMergeMPI, SabutayAradixSortDoubleWithMergeSEQ>(PPC_SETTINGS_sabutay_a_radixSortDoubleWithMerge);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = ExampleRunPerfTestProcesses3::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, ExampleRunPerfTestProcesses3, kGtestValues, kPerfTestName);

}  // namespace sabutay_a_radixSortDoubleWithMerge
