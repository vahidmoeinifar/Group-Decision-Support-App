#include "groupdecision/ConsensusFeedback.h"

#include "groupdecision/Engine.h"
#include "groupdecision/Methods.h"
#include "groupdecision/Statistics.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace gd {

int ExpertFeedback::flaggedCount() const
{
    return static_cast<int>(std::count_if(cells.begin(), cells.end(),
                                          [](const CellFeedback& c) { return c.flagged; }));
}

ExpertFeedback consensusFeedback(const GroupProblem& problem, int expertIndex,
                                 double threshold, double step)
{
    if (expertIndex < 0 || expertIndex >= static_cast<int>(problem.expertCount()))
        throw std::out_of_range("Expert index is out of range.");
    if (threshold < 0.0 || threshold > 1.0)
        throw std::invalid_argument("Threshold must be between 0 and 1.");
    if (step < 0.0 || step > 1.0)
        throw std::invalid_argument("Step must be between 0 and 1.");

    const std::size_t m = problem.optionCount();
    const std::size_t n = problem.criterionCount();
    const Matrix group = aggregateMatrix(problem);
    const Matrix& own = problem.experts()[expertIndex].scores;

    // Spread of each criterion over all experts and options, used to scale distances,
    // so "3 days" and "3 kPLN" are compared fairly.
    std::vector<double> spread(n);
    for (std::size_t j = 0; j < n; ++j) {
        double lo = std::numeric_limits<double>::max();
        double hi = std::numeric_limits<double>::lowest();
        for (const auto& e : problem.experts())
            for (std::size_t i = 0; i < m; ++i) {
                lo = std::min(lo, e.scores(i, j));
                hi = std::max(hi, e.scores(i, j));
            }
        spread[j] = hi - lo;
    }

    ExpertFeedback feedback;
    feedback.expert = problem.experts()[expertIndex].name;
    feedback.cells.reserve(m * n);
    for (std::size_t i = 0; i < m; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            CellFeedback cell;
            cell.optionIndex = static_cast<int>(i);
            cell.criterionIndex = static_cast<int>(j);
            cell.score = own(i, j);
            cell.groupScore = group(i, j);
            cell.deviation = spread[j] == 0.0 ? 0.0 : std::abs(cell.score - cell.groupScore) / spread[j];
            cell.suggestedScore = cell.score + step * (cell.groupScore - cell.score);
            cell.flagged = cell.deviation > threshold;
            feedback.cells.push_back(cell);
        }
    }

    feedback.agreement = stats::spearman(methods::topsis(own, problem.criteria()),
                                         methods::topsis(group, problem.criteria()));
    return feedback;
}

} // namespace gd
