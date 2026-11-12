#pragma once

#include <vector>

namespace gd::stats {

/// Ranks of the values (1 = smallest). Tied values get the average of their ranks.
std::vector<double> averageRanks(const std::vector<double>& values);

/// Spearman rank correlation in [-1, 1]. Throws std::invalid_argument on different lengths.
double spearman(const std::vector<double>& a, const std::vector<double>& b);

} // namespace gd::stats
