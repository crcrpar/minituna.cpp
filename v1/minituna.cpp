#include "v1/minituna.hpp"

#include <assert.h>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <iostream>

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

auto FrozenTrial::Number() const noexcept -> const size_t {
  return trial_id_;
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
  return trials_[trial_id];
}

auto Storage::SetTrialValue(const size_t& trial_id, const double& value) noexcept -> void {
  trials_[trial_id].SetValue(value);
}

auto Storage::SetTrialState(const size_t& trial_id, const TrialState& state) noexcept -> void {
  trials_[trial_id].SetState(state);
}

auto Storage::SetTrialParam(const size_t& trial_id, const std::string & name, const absl::any& param) -> void {
  FrozenTrial& trial = trials_.at(trial_id);
  assert(!trial.IsFinished() && "Cannot update finished trial");
  trial.SetParam(name, param);
}

auto Storage::GetAllTrials() const noexcept -> const std::vector<FrozenTrial> {
  return trials_;
}

Trial::Trial(Study * study_ptr, const size_t& trial_id)
  : study_ptr_{study_ptr}, trial_id_{trial_id}, state_{TrialState::Running} {}

auto Trial::SuggestFloat(const std::string & name, const double& low, const double& high) -> const double {
  auto trial = study_ptr_->GetStorage().GetTrial(trial_id_);
  absl::any low_any = low;
  absl::any high_any = high;
  const absl::flat_hash_map<std::string, absl::any> distribution = {{"low", low_any}, {"high", high_any}};
  auto param = study_ptr_->SampleIndependent(trial, name, distribution);
  study_ptr_->GetStorage().SetTrialParam(trial_id_, name, param);
  return param;
}

Sampler::Sampler(): bitgen_{} {}

auto Sampler::SampleIndependent(const Study& study, FrozenTrial& trial, const std::string & name, const absl::flat_hash_map<std::string, absl::any> & distribution) -> const double {
  const auto low{distribution.find("low")};
  const auto high{distribution.find("high")};
  return absl::Uniform<double>(bitgen_, absl::any_cast<double>(low->second), absl::any_cast<double>(high->second));
}

Study::Study(): storage_{}, sampler_{} {}

Study::Study(const Storage & storage, const Sampler & sampler): storage_{}, sampler_{} {}

auto Study::GetStorage() noexcept -> Storage {
  return storage_;
}

auto Study::SampleIndependent(FrozenTrial & trial, const std::string & name, const absl::flat_hash_map<std::string, absl::any> & distribution) -> const double {
  return sampler_.SampleIndependent(*this, trial, name, distribution);
}

auto Study::Optimize(const absl::FunctionRef<const double(Trial)> objective, const size_t& n_trials) -> void {
  for (size_t i = 0; i < n_trials; i++) {
    const size_t trial_id = storage_.CreateNewTrial();
    Trial trial(this, trial_id);

    std::cout << "Trial ID: " << trial_id;
    try {
      const double value = objective(trial);
      storage_.SetTrialValue(trial_id, value);
      storage_.SetTrialState(trial_id, TrialState::Completed);
      std::cout << ", Value: " << value << "\n";
    } catch (const std::exception& e) {
      std::cerr << "Trial " << trial_id << " failed because " << e.what() << '\n';
      storage_.SetTrialState(trial_id, TrialState::Failed);
    }
  }
}

auto CreateStudy() -> Study {
  return Study{};
}

} // namespace minituna_v1
