#pragma once

#include <cstddef>
#include <initializer_list>
#include <list>
#include <memory>
#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/functional/function_ref.h"
#include "absl/random/random.h"
#include "absl/strings/string_view.h"
#include "absl/strings/str_join.h"
#include "absl/types/any.h"
#include "absl/types/variant.h"


namespace minituna_v2 {

enum class TrialState {
  Running,
  Completed,
  Failed,
};

template <typename DerivedDist>
class BaseDist {
 public:

  template <typename T1>
  auto ToInternalRepr(const T1 & external_repr) const -> double {
    return static_cast<DerivedDist*>(this)->ToInternalReprImpl(external_repr);
  }

  auto ToExternalRepr(const double & internal_repr) const -> absl::any {
    return static_cast<DerivedDist*>(this)->ToExternalReprImpl(internal_repr);
  }
};

class UniformDist : public BaseDist<UniformDist> {
 public:
  UniformDist(double low, double high);

  auto Low() const noexcept -> const double;
  auto High() const noexcept -> const double;

  template <typename T1>
  auto ToInternalReprImpl(const T1 & external_repr) const -> double;

  auto ToExternalReprImpl(const double & internal_repr) const -> absl::any;

 private:
  double low_, high_;
};

class LogUniformDist : public BaseDist<LogUniformDist> {
 public:
  LogUniformDist(double low, double high);

  auto Low() const noexcept -> const double;
  auto High() const noexcept -> const double;

  template <typename T1>
  auto ToInternalReprImpl(const T1 & external_repr) const -> double;

  auto ToExternalReprImpl(const double & internal_repr) const -> absl::any;

 private:
  double low_, high_;
};

class IntUniformDist : public BaseDist<IntUniformDist> {
 public:
  IntUniformDist(int low, int high);

  auto Low() const noexcept -> const int;
  auto High() const noexcept -> const int;

  template <typename T1>
  auto ToInternalReprImpl(const T1 & external_repr) const -> double;

  auto ToExternalReprImpl(const double & internal_repr) const -> absl::any;

 private:
  int low_, high_;
};

class CategoricalDist : public BaseDist<CategoricalDist> {
 public:
  CategoricalDist(std::initializer_list<absl::variant<bool, int, double, std::string>> choices);

  auto Choices() const noexcept -> const std::list<absl::variant<bool, int, double, std::string>>;

  template <typename T1>
  auto ToInternalReprImpl(const T1 & external_repr) const -> double;

  auto ToExternalReprImpl(const double & internal_repr) const -> absl::any;

 private:
  std::vector<absl::variant<bool, int, double, std::string>> choices_;
};

class FrozenTrial {
 public:
  FrozenTrial(const size_t & trial_id, const TrialState & state);

  auto IsFinished() const noexcept -> const bool;
  auto Value() const noexcept -> const double;
  auto GetParams() const -> const absl::flat_hash_map<std::string, absl::any>
  auto SetValue(const double&) noexcept -> void;
  auto SetState(const TrialState&) noexcept -> void;
  auto SetParam(const std::string &, const absl::any) noexcept -> void;

 private:
  size_t trial_id_;
  TrialState state_;
  double value_;
  absl::flat_hash_map<std::string, absl::any> internal_params_;
  absl::flat_hash_map<std::string, BaseDist> distributions_;
};

class Storage {
 public:
  auto CreateNewTrial() noexcept -> const size_t;
  auto GetTrial(const size_t & trial_id) const noexcept -> FrozenTrial;
  auto GetAllTrials() const noexcept -> std::vector<FrozenTrial>;
  auto SetTrialValue(const size_t & trial_id, const double & value) -> void;
  auto SetTrialState(const size_t & trial_id, const TrialState & state) -> void;
  auto SetTrialParam(const size_t & trial_id, const std::string & name, std::shared_ptr<BaseDist> distribution, const double & value) -> void;

 private:
  std::vector<FrozenTrial> trials_;
};

// Forward Declaration for Trial
class Study;

class Trial {
 public:
  Trial(Study * study, const size_t & trial_id);
  auto SuggestUniform(const std::string & name, const double & low, const double & high) noexcept -> const double;
  auto SuggestLogUniform(const std::string & name, const double & low, const double & high) noexcept -> const double;
  auto SuggestInt(const std::string & name, const int & low, const int & high) noexcept -> const int;
  auto SuggestCategorical(const std::string & name, const std::vector<absl::variant<bool, int, double, std::string> & choices) noexcept -> const absl::variant<bool, int, double, std::string>;

 private:
  Study * study_ptr_;
  size_t trial_id_;
  TrialState state_;
  auto suggest(const std::string & name, const BaseDist & distribution) -> const absl::any;
};

class Sampler {

 public:
  Sampler();

  template <typename DistType>
  auto SampleIndependent(Study * study, FrozenTrial & trial, const std::string & name, const DistType & distribution) -> std::any;

 private:
  absl::BitGen bitgen_;
};

class Study {

 public:
  Study();
  auto GetStorage() noexcept -> Storage;
  auto SampleIndependent(FrozenTrial & trial, const std::string & name, const BaseDist & distribution) -> absl::any;
  auto Optimize(const absl::FunctionRef<const double(Trial)> objective, const size_t & n_trials) -> void;

 private:
  Storage storage_;
  Sampler sampler_;
};

auto CreateStudy() -> Study;

} // namespace minituna_v2