#pragma once

#include "groupdecision/GroupProblem.h"
#include "groupdecision/Types.h"

#include <vector>

namespace gd {

/// Combines several experts' scores into one group ranking.
GroupResult solve(const GroupProblem& problem, Method method);

/// Indexes of the options ranked first (more than one when tied), sorted.
std::vector<int> winners(const GroupResult& result);

/// Weighted mean of the expert score matrices, using the experts' weights.
Matrix aggregateMatrix(const GroupProblem& problem);

} // namespace gd
