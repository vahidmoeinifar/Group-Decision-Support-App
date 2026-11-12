#pragma once

#include "groupdecision/Types.h"

#include <string>
#include <vector>

namespace gd {

/// A validated group decision problem: options, criteria and expert scores.
/// The constructor throws std::invalid_argument when the input does not make sense,
/// so every GroupProblem that exists is safe to solve.
class GroupProblem {
public:
    GroupProblem(std::vector<std::string> options,
                 std::vector<Criterion> criteria,
                 std::vector<ExpertInput> experts);

    const std::vector<std::string>& options() const { return options_; }
    const std::vector<Criterion>& criteria() const { return criteria_; }
    const std::vector<ExpertInput>& experts() const { return experts_; }

    std::size_t optionCount() const { return options_.size(); }
    std::size_t criterionCount() const { return criteria_.size(); }
    std::size_t expertCount() const { return experts_.size(); }

    /// Same problem with different criterion weights (used by sensitivity analysis).
    GroupProblem withCriteria(std::vector<Criterion> criteria) const;

private:
    std::vector<std::string> options_;
    std::vector<Criterion> criteria_;
    std::vector<ExpertInput> experts_;
};

} // namespace gd
