#include "minituna.hpp"

#include <vector>
#include <iostream>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"


ABSL_FLAG(uint64_t, n_trials, 100, "The number of trials");


auto objective(minituna_v1::Trial trial) -> const double {
  const double x = trial.SuggestFloat("x", 0, 10);
  const double y = trial.SuggestFloat("y", 0, 10);
  return (x - 3) * (x - 3) + (y - 5) * (y - 5);
}


auto main(int argc, char** argv) -> int {
  absl::ParseCommandLine(argc, argv);

  auto study = minituna_v1::CreateStudy();
  study.Optimize(objective, absl::GetFlag(FLAGS_n_trials));

  const std::vector<minituna_v1::FrozenTrial> all_trials = study.GetStorage().GetAllTrials();
  const minituna_v1::FrozenTrial best_trial = *(std::min_element(
        all_trials.begin(), all_trials.end(),
        [&](const minituna_v1::FrozenTrial& lhs, const minituna_v1::FrozenTrial& rhs) {
          return lhs.Value() < rhs.Value();
        }
        ));
  std::cout << "Best trial| ID: " << best_trial.Number() << ", value: " << best_trial.Value() << '\n';
  return 0;
}
