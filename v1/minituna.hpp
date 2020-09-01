#pragma once
#include <cstddef>
#include <string>
#include <vector>
#include <memory>

#include "absl/container/flat_hash_map.h"
#include "absl/random/random.h"
#include "absl/strings/string_view.h"
#include "absl/strings/str_join.h"
#include "absl/types/any.h"


namespace minituna_v1 {

enum class TrialState {
  Running,
  Completed,
  Failed,
};

class FrozenTrial {
 private:
  size_t trial_id_;
  TrialState state_;
  double value_;
  absl::flat_hash_map<std::string, absl::any> params_;

 public:
  FrozenTrial();
  FrozenTrial(
      const size_t&,
      const TrialState&,
      const double*,
      const absl::flat_hash_map<std::string, absl::any>*
  );

  auto IsFinished() const noexcept -> const bool;
  auto Value() const noexcept -> const double;
  auto SetValue(const double&) noexcept -> void;
  auto SetState(const TrialState&) noexcept -> void;
  auto SetParam(const std::string &, const absl::any) noexcept -> void;
};

class Storage {
 private:
  std::vector<FrozenTrial> trials_;

 public:
  Storage();
  auto CreateNewTrial() noexcept -> const size_t;
  auto GetTrial(const size_t&) const -> FrozenTrial;
  auto GetBestTrial() const noexcept -> const FrozenTrial;
  auto SetTrialValue(const size_t&, const double&) noexcept -> void;
  auto SetTrialState(const size_t&, const TrialState&) noexcept -> void;
  auto SetTrialParam(const size_t&, const std::string &, const absl::any&) -> void;
};

class Study;

class Trial {
 private:
  std::shared_ptr<Study> study_ptr_;
  size_t trial_id_;
  TrialState state_;

 public:
  Trial(std::shared_ptr<Study>, const size_t&);
  auto SuggestFloat(
      const std::string &,
      const double &,
      const double &
  ) -> const double;
};

class Sampler {
 private:
  absl::BitGen bitgen_;

 public:
  Sampler();
  auto SampleIndependent(
      const Study& study,
      FrozenTrial &,
      const std::string &,
      const absl::flat_hash_map<std::string, absl::any> &
  ) -> const double;
};

class Study {
 private:
  Storage storage_{};
  Sampler sampler_{};

 public:
  Study();
  Study(const Storage &, const Sampler &);
  auto GetBestTrial() const -> const FrozenTrial;
  auto GetStorage() noexcept -> Storage;
  auto SampleIndependent(
      FrozenTrial &,
      const std::string &,
      const absl::flat_hash_map<std::string, absl::any> &
  ) -> const double;

  template <typename Func>
  auto Optimize(const Func, const size_t &) -> void;
};

auto CreateStudy(const Storage *, const Sampler *) -> Study;

} // namespace minituna_v1
