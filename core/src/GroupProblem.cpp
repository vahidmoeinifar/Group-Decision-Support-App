#include "groupdecision/GroupProblem.h"

#include <cmath>
#include <stdexcept>

namespace gd {

namespace {

void require(bool condition, const std::string& message)
{
    if (!condition)
        throw std::invalid_argument(message);
}

bool validWeight(double w) { return std::isfinite(w) && w >= 0.0; }

} // namespace

GroupProblem::GroupProblem(std::vector<std::string> options,
                           std::vector<Criterion> criteria,
                           std::vector<ExpertInput> experts)
    : options_(std::move(options)), criteria_(std::move(criteria)), experts_(std::move(experts))
{
    require(options_.size() >= 2, "At least two options are required.");
    require(!criteria_.empty(), "At least one criterion is required.");
    require(!experts_.empty(), "At least one expert is required.");

    double criterionTotal = 0.0;
    for (const auto& c : criteria_) {
        require(validWeight(c.weight), "Criterion weights must be finite and non-negative.");
        criterionTotal += c.weight;
    }
    require(criterionTotal > 0.0, "At least one criterion weight must be positive.");

    double expertTotal = 0.0;
    for (const auto& e : experts_) {
        require(validWeight(e.weight), "Expert weights must be finite and non-negative.");
        expertTotal += e.weight;

        require(e.scores.rows() == options_.size() && e.scores.cols() == criteria_.size(),
                "Expert '" + e.name + "' must have a " + std::to_string(options_.size()) + "x"
                    + std::to_string(criteria_.size()) + " score matrix.");
        for (double v : e.scores.values())
            require(std::isfinite(v), "Expert '" + e.name + "' has a score that is not a number.");
    }
    require(expertTotal > 0.0, "At least one expert weight must be positive.");
}

GroupProblem GroupProblem::withCriteria(std::vector<Criterion> criteria) const
{
    return GroupProblem(options_, std::move(criteria), experts_);
}

std::string toString(Method method)
{
    switch (method) {
    case Method::Topsis: return "TOPSIS";
    case Method::WeightedSum: return "Weighted sum";
    case Method::Borda: return "Borda count";
    }
    return "Unknown";
}

} // namespace gd
