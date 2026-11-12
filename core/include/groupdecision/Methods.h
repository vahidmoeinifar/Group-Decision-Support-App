#pragma once

#include "groupdecision/Types.h"

#include <vector>

namespace gd::methods {

/// TOPSIS: closeness of each option to the ideal option, in [0, 1] (higher is better).
/// Steps: vector normalization, weighting, ideal and anti-ideal points, relative closeness.
std::vector<double> topsis(const Matrix& scores, const std::vector<Criterion>& criteria);

/// Weighted sum with min-max normalization, in [0, 1] (higher is better).
std::vector<double> weightedSum(const Matrix& scores, const std::vector<Criterion>& criteria);

/// Borda points from a score vector: one point for each option with a lower score,
/// half a point for each other option with an equal score.
std::vector<double> bordaPoints(const std::vector<double>& scores);

/// Criterion weights scaled to sum to 1.
std::vector<double> normalizedWeights(const std::vector<Criterion>& criteria);

} // namespace gd::methods
