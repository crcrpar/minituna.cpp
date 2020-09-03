#include "v2/minituna.hpp"

#include <asm-generic/errno.h>
#include <initializer_list>
#include <list>
#include <math.h>
#include <memory>
#include <random>
#include <string>
#include <type_traits>

#include "absl/types/any.h"
#include "absl/types/variant.h"

namespace minituna_v2 {

UniformDist::UniformDist(double low, double high) : low_{std::move(low)}, high_{std::move(high)} {}

auto UniformDist::Low() const noexcept -> const double {
  return low_;
}

auto UniformDist::High() const noexcept -> const double {
  return high_;
}

LogUniformDist::LogUniformDist(double low, double high) : low_{std::move(low)}, high_{std::move(high)} {}

auto LogUniformDist::Low() const noexcept -> const double {
  return low_;
}

auto LogUniformDist::High() const noexcept -> const double {
  return high_;
}

IntUniformDist::IntUniformDist(int low, int high) : low_{std::move(low)}, high_{std::move(high)} {}

auto IntUniformDist::Low() const noexcept -> const int {
  return low_;
}

auto IntUniformDist::High() const noexcept -> const int {
  return high_;
}

template <typename CategoryType>
CategoricalDist::CategoricalDist(std::initializer_list<CategoryType> choices) : choices_(choices) {}

template <typename CategoryType>
auto CategoricalDist::Choices() const noexcept -> const std::vector<CategoryType> {
  return choices_;
}

FrozenTrial::FrozenTrial(const size_t & trial_id, const TrialState & state)
  : trial_id_{trial_id}, state_{state}, value_{0.0}, internal_params_{}, distributions_{} {}

auto FrozenTrial::IsFinished() const noexcept -> const bool {
  return state_ == TrialState::Completed;
}

auto FrozenTrial::Value() const noexcept -> const double {
  return value_;
}

auto FrozenTrial::GetParams() const -> const absl::flat_hash_map<std::string, absl::any> {
  return internal_params_;
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
  internal_params_[name] = param;
}

Storage::Storage() : trials_{} {}

auto Storage::CreateNewTrial() noexcept -> const size_t {
  const size_t trial_id = trials_.size();
  trials_.emplace_back(trial_id, TrialState::Running);
  return trial_id;
}

auto Storage::GetTrial(const size_t & trial_id) const noexcept -> FrozenTrial {
  return trials_.at(trial_id);
}

auto Storage::GetAllTrials() const noexcept -> std::vector<FrozenTrial> {
  return trials_;
}

auto Storage::SetTrialValue(const size_t& trial_id, const double& value) -> void {
  trials_[trial_id].SetValue(value);
}

auto Storage::SetTrialState(const size_t& trial_id, const TrialState& state) -> void {
  trials_[trial_id].SetState(state);
}

auto Storage::SetTrialParam(const size_t& trial_id, const std::string & name, BaseDist distribution, const absl::any& param) -> void {
  FrozenTrial& trial = trials_.at(trial_id);
  assert(!trial.IsFinished() && "Cannot update finished trial");
  trial.SetParam(name, param);
}

Trial::Trial(Study * study_ptr, const size_t& trial_id)
  : study_ptr_{study_ptr}, trial_id_{trial_id}, state_{TrialState::Running} {}

template <typename DistType>
auto Trial::suggest(const std::string & name, const DistType & distribution) -> const absl::any {
  auto trial = study_ptr_->GetStorage().GetTrial(trial_id_);
  auto param = study_ptr_->SampleIndependent(trial, name, distribution);
  study_ptr_->GetStorage().SetTrialParam(trial_id_, name, param);
  return absl::any(param);
}

auto Trial::SuggestUniform(const std::string & name, const double & low, const double & high) noexcept -> const double {
  return absl::any_cast<double>(suggest(name, UniformDist(low, high)));
}

auto Trial::SuggestLogUniform(const std::string & name, const double & low, const double & high) noexcept -> const double {
  return absl::any_cast<double>(suggest(name, LogUniformDist(low, high)));
}

auto Trial::SuggestInt(const std::string & name, const int & low, const int & high) noexcept -> const int {
  return absl::any_cast<int>(suggest(name, IntUniformDist(low, high)));
}

template <typename CategoryType>
auto Trial::SuggestCategorical(const std::string & name, const std::vector<CategoryType> & choices) noexcept -> const CategoryType {
  auto categorical_dist = CategoricalDist(choices);
  return absl::any_cast<CategoryType>(suggest(name, categorical_dist));
}

Sampler::Sampler() : bitgen_{} {}

template <typename DistType>
auto Sampler::SampleIndependent(Study * study, FrozenTrial & trial, const std::string & name, const DistType & distribution) -> absl::any {
  if (std::is_same<DistType, UniformDist>::value) {
    return absl::any(absl::Uniform<double>(bitgen_, distribution.Low(), distribution.High()));
  } else if (std::is_same<DistType, LogUniformDist>::value) {
    return absl::any(std::exp(absl::Uniform<double>(std::log(distribution.Low()), std::log(distribution.High()))));
  } else if (std::is_same<DistType, IntUniformDist>::value) {
    return absl::any(absl::Uniform<int>(bitgen_, distribution.Low(), distribution.High()));
  } else if (std::is_same<DistType, CategoricalDist>::value) {
    const size_t index = absl::Uniform<int>(bitgen_, 0, distribution.Choices().size());
    return absl::any(choices_.at(index));
  } else {
    static_assert(false, "Invalid Distribution.");
  }
}

Study::Study() : storage_{}, sampler_{} {}

auto Study::GetStorage() noexcept -> Storage {
  return storage_;
}

template <typename DistType>
auto Study::SampleIndependent(
    FrozenTrial & trial,
    const std::string & name,
    const DistType & distribution) -> absl::any {
  return sampler_.SampleIndependent(this, trial, name, distribution);
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

} // namespace minituna_v2
