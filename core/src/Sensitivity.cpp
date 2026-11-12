#include "groupdecision/Sensitivity.h"

#include "groupdecision/Engine.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace gd {

namespace {

double round4(double x) { return std::round(x * 10000.0) / 10000.0; }

/// Drops zero-width segments (exact ties at one point) and merges neighbours with the same winners.
std::vector<WeightSegment> tidy(const std::vector<WeightSegment>& segments)
{
    std::vector<WeightSegment> kept;
    for (const auto& s : segments)
        if (s.to - s.from >= 0.001)
            kept.push_back(s);
    if (kept.empty())
        return segments;

    std::vector<WeightSegment> merged{kept.front()};
    merged.front().from = 0.0;
    for (std::size_t k = 1; k < kept.size(); ++k) {
        WeightSegment& last = merged.back();
        if (last.winners == kept[k].winners) {
            last.to = kept[k].to;
        } else {
            WeightSegment next = kept[k];
            next.from = last.to;
            merged.push_back(next);
        }
    }
    merged.back().to = 1.0;
    return merged;
}

} // namespace

GroupProblem withShare(const GroupProblem& problem, int j, double s)
{
    const auto& criteria = problem.criteria();
    double others = 0.0;
    for (std::size_t k = 0; k < criteria.size(); ++k)
        if (static_cast<int>(k) != j)
            others += criteria[k].weight;

    std::vector<Criterion> adjusted = criteria;
    for (std::size_t k = 0; k < adjusted.size(); ++k) {
        if (static_cast<int>(k) == j)
            adjusted[k].weight = s;
        else if (others > 0.0)
            adjusted[k].weight = criteria[k].weight / others * (1.0 - s);
        else
            adjusted[k].weight = (1.0 - s) / static_cast<double>(criteria.size() - 1);
    }
    return problem.withCriteria(std::move(adjusted));
}

SensitivityResult analyzeSensitivity(const GroupProblem& problem, Method method, int steps)
{
    if (steps < 2)
        throw std::invalid_argument("At least two steps are needed.");

    SensitivityResult result;
    result.method = method;
    result.currentWinners = winners(solve(problem, method));

    double total = 0.0;
    for (const auto& c : problem.criteria())
        total += c.weight;

    for (std::size_t jj = 0; jj < problem.criterionCount(); ++jj) {
        const int j = static_cast<int>(jj);
        CriterionSensitivity cs;
        cs.criterionIndex = j;
        cs.criterion = problem.criteria()[jj].name;
        cs.currentShare = round4(problem.criteria()[jj].weight / total);

        if (problem.criterionCount() == 1) {
            cs.currentShare = 1.0;
            cs.segments = {{0.0, 1.0, result.currentWinners}};
            result.criteria.push_back(cs);
            continue;
        }

        auto winnersAt = [&](double s) { return winners(solve(withShare(problem, j, s), method)); };

        std::vector<WeightSegment> segments;
        double start = 0.0;
        std::vector<int> previous = winnersAt(0.0);

        for (int k = 1; k <= steps; ++k) {
            const double s = static_cast<double>(k) / steps;
            std::vector<int> now = winnersAt(s);
            if (now == previous)
                continue;

            // Narrow down where the change happens between the two grid points
            double lo = static_cast<double>(k - 1) / steps;
            double hi = s;
            for (int it = 0; it < 20; ++it) {
                const double mid = (lo + hi) / 2.0;
                if (winnersAt(mid) == previous)
                    lo = mid;
                else
                    hi = mid;
            }

            const double boundary = round4((lo + hi) / 2.0);
            segments.push_back({start, boundary, previous});
            start = boundary;
            previous = now;
        }
        segments.push_back({start, 1.0, previous});
        cs.segments = tidy(segments);

        // The stable range is the segment that holds the current share
        const WeightSegment* home = nullptr;
        for (const auto& g : cs.segments)
            if (cs.currentShare >= g.from && cs.currentShare <= g.to && g.winners == result.currentWinners)
                home = &g;
        if (!home)
            for (const auto& g : cs.segments)
                if (cs.currentShare >= g.from && cs.currentShare <= g.to) {
                    home = &g;
                    break;
                }
        if (home) {
            cs.stableFrom = home->from;
            cs.stableTo = home->to;
        }
        result.criteria.push_back(cs);
    }

    double best = std::numeric_limits<double>::max();
    for (const auto& c : result.criteria) {
        const double down = c.stableFrom > 0.0 ? c.currentShare - c.stableFrom : std::numeric_limits<double>::max();
        const double up = c.stableTo < 1.0 ? c.stableTo - c.currentShare : std::numeric_limits<double>::max();
        const double distance = std::min(down, up);
        if (distance < best) {
            best = distance;
            result.mostSensitiveCriterion = c.criterionIndex;
        }
    }
    return result;
}

} // namespace gd
