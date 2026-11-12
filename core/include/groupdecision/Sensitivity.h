#pragma once

#include "groupdecision/GroupProblem.h"
#include "groupdecision/Types.h"

#include <optional>
#include <string>
#include <vector>

namespace gd {

/// A range of weight shares in which the same options come first.
struct WeightSegment {
    double from = 0.0;
    double to = 1.0;
    std::vector<int> winners;
};

struct CriterionSensitivity {
    int criterionIndex = 0;
    std::string criterion;
    double currentShare = 0.0; ///< The criterion's current share of the total weight.
    double stableFrom = 0.0;   ///< Lowest share at which the current winner stays first.
    double stableTo = 1.0;     ///< Highest share at which the current winner stays first.
    std::vector<WeightSegment> segments;
};

struct SensitivityResult {
    Method method = Method::Topsis;
    std::vector<int> currentWinners;
    std::vector<CriterionSensitivity> criteria;
    /// Criterion whose weight needs the smallest change to change the winner.
    /// Empty when no single weight change can change it.
    std::optional<int> mostSensitiveCriterion;
};

/// One-at-a-time weight sensitivity. Each criterion's share of the total weight is moved
/// from 0 to 1 (the other weights keep their proportions) to find where the winner changes.
/// A grid of `steps` points finds each change, then bisection pins down where it happens.
SensitivityResult analyzeSensitivity(const GroupProblem& problem, Method method, int steps = 200);

/// Gives criterion j the share s (0..1); the others share the rest in their original proportions.
GroupProblem withShare(const GroupProblem& problem, int j, double s);

} // namespace gd
