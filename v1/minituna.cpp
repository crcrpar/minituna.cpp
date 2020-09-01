#include "minituna.hpp"

#include <assert.h>
#include <algorithm>
#include <memory>

namespace minituna_v1 {

FrozenTrial::FrozenTrial(): trial_id_{0}, state_{TrialState::Running}, value_{0}, params_{} {}

FrozenTrial::FrozenTrial(
    const size_t& trial_id,
    const TrialState & state,
    const double * value = nullptr,
    const absl::flat_hash_map<std::string, absl::any> * params = nullptr)
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

auto FrozenTrial::SetParam(const std::string & name, const absl::any param) noexcept -> void {
  params_[name] = param;
}

Storage::Storage(): trials_{} {}

auto Storage::CreateNewTrial() noexcept -> const size_t {
  const size_t trial_id = trials_.size();
  trials_.emplace_back(trial_id, TrialState::Running);
  return trial_id;
}

auto Storage::GetTrial(const size_t& trial_id) const -> FrozenTrial {
  return trials_.at(trial_id);
}

auto Storage::GetBestTrial() const noexcept -> const FrozenTrial {
  assert(trials_.size() > 0);
  const FrozenTrial best_trial = *std::max_element(
      trials_.begin(), trials_.end(),
      [](const FrozenTrial & rhs, const FrozenTrial & lhs) -> bool {
        return rhs.Value() < lhs.Value();
      });
  return best_trial;
}

auto Storage::SetTrialValue(const size_t& trial_id, const double& value) noexcept -> void {
  trials_.at(trial_id).SetValue(value);
}

auto Storage::SetTrialState(const size_t& trial_id, const TrialState& state) noexcept -> void {
  trials_.at(trial_id).SetState(state);
}

auto Storage::SetTrialParam(const size_t& trial_id, const std::string & name, const absl::any& param) -> void {
  FrozenTrial& trial = trials_.at(trial_id);
  assert(!trial.IsFinished() && "Cannot update finished trial");
  trial.SetParam(name, param);
}

Trial::Trial(std::shared_ptr<Study> study_ptr, const size_t& trial_id)
  : study_ptr_{study_ptr}, trial_id_{trial_id}, state_{TrialState::Running} {}

auto Trial::SuggestFloat(const std::string & name, const double& low, const double& high) -> const double {
  auto trial = study_ptr_->GetStorage().GetTrial(trial_id_);
  const absl::flat_hash_map<std::string, absl::any> distribution{{{"low", low}, {"high", high}}};
  auto param = study_ptr_->SampleIndependent(trial, name, distribution);
  study_ptr_->GetStorage().SetTrialParam(trial_id_, name, param);
}

Sampler::Sampler(): bitgen_{} {}

auto Sampler::SampleIndependent(const Study& study, FrozenTrial& trial, const std::string & name, const absl::flat_hash_map<std::string, absl::any> & distribution) -> const double {
  const std::string low_key{"low"}, high_key{"high"};
  absl::any low{*distribution.find(low_key)};
  absl::any high{*distribution.find(high_key)};
  return absl::Uniform<double>(bitgen_, *absl::any_cast<double>(&low), *absl::any_cast<double>(&high));
}

Study::Study(): storage_{}, sampler_{} {}

Study::Study(const Storage & storage, const Sampler & sampler): storage_{}, sampler_{} {}

auto Study::GetBestTrial() const -> const FrozenTrial {
  storage_.GetBestTrial();
}

auto Study::GetStorage() noexcept -> Storage {
  return storage_;
}

auto Study::SampleIndependent(FrozenTrial & trial, const std::string & name, const absl::flat_hash_map<std::string, absl::any> & distribution) -> const double {
  return sampler_.SampleIndependent(*this, trial, name, distribution);
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
  return Study{};
}

} // namespace minituna_v1
