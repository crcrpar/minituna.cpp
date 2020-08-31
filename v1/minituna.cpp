#include "minituna.hpp"

#include <assert.h>
#include <algorithm>

namespace minituna_v1 {

FrozenTrial::FrozenTrial(
    const size_t& trial_id,
    const TrialState & state,
    const double * value = nullptr,
    absl::flat_hash_map<absl::string_view, absl::any>> * params = nullptr)
  : trial_id_{trial_id}, state_{state} {
    if (value) {
      value_ = *value;
    } else {
      value_ = 0.0;
    }
    if (params) {
      params_ = *params;
    } else {
      params_ = {};
    }
  }

auto FrozenTrial::IsFinished() const noexcept -> const bool {
  return state_ == TrialState::Completed;
}

auto FrozenTrial::Value() const noexcept -> const double {
  return value_;
}

auto FrozenTrial::SetValue(const double& value) noexcept -> void {
  value_ = value;
}

auto FrozenTrial::SetState(const TrialState& state) noexcept -> void {
  state_ = state;
}

auto SetParam(const absl::string_view name, const absl::any param) noexcept -> void {
  params_[name] = param;
}

auto Storage::CreateNewTrial() noexcept -> const size_t {
  const size_t trial_id = trials_.size();
  trials_.emplace_back(trial_id, "running");
  return trial_id;
}

auto Storage::GetTrial(const size_t& trial_id) const -> FrozenTrial {
  return trials_.at(trial_id);
}

auto Storage::GetBestTrial() const noexcept -> const FrozenTrial {
  if (trials_.size() == 0) {
    return {};
  }
  const FrozenTrial best_trial = std::sort(
      trials_.begin(), trials_.end(),
      [](const FrozenTrial & rhs, const FrozenTrial & lhs) -> bool {
        return rhs.Value() < lhs.Value();
      });
}

auto Storage::SetTrialValue(const size_t& trial_id, const double& value) noexcept -> void {
  trials_.at(trial_id).SetValue(value);
}

auto Storage::SetTrialState(const size_t& trial_id, const TrialState& state) noexcept -> void {
  trials_.at(trial_id).SetState(state);
}

auto Storage::SetTrialParam(const size_t& trial_id, const absl::string_view name, const absl::any& param) -> void {
  FrozenTrial& trial = trials_.at(trial_id);
  assert(!trial.IsFinished() && "Cannot update finished trial");
  trial.SetParam(name, param);
}

Trial::Trial(std::shared_ptr<Study> study_ptr, const size_t& size_t trial_id)
  : study_ptr_{study_ptr}, trial_id_{trial_id}, state_{TrialState::Running} {}

auto Trial::SuggestFloat(const absl::string_view name, const double& low, const double& high) -> const double {
  auto trial = study_ptr_->Storage().GetTrial(trial_id_);
  auto param = study_ptr_->Sampler().SampleIndependent(
      *study_ptr_, trial, name,
      {{"low", low}, {"high", high}});
  study_ptr_->Storage().SetTrialParam(trial_id_, name, param);
}

auto Sampler::SampleIndependent(const Study& study, Frozen& trial, absl::string_view name, const absl::flat_hash_map<absl::string_view, absl::any> & distribution) -> const double {
  return absl::Uniform(bitgen_, distribution["low"], distribution["high"]);
}

auto Study::GetBestTrial() const -> const FrozenTrial {
  storage_.GetBestTrial();
}

template <typename Func>
auto Study::Optimize(const Func objective, const size_t& n_trials) -> void {
  for (size_t i = 0; i < n_trials; i++) {
    const size_t trial_id = storage_.CreateNewTrial();
    Trial trial(this, trial_id);

    try {
      const double value = objective(trial);
      storage_.SetTrialValue(trial_id, value);
      storage_.SetTrialState(trial_id, TrialState::Completed);
    } catch (const std::exception& e) {
      storage_.SetTrialState(trial_id, TrialState::Failed);
    }
  }
}

auto CreateStudy(const Storage * storage = nullptr, const Sampler * sampler = nullptr) -> Study {
  return {
    storage ? *storage : Storage(),
    sampler ? *sampler : Sampler()
  };
}

} // namespace minituna_v1
