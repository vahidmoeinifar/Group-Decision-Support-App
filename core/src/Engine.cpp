#include "groupdecision/Engine.h"

#include "groupdecision/Methods.h"
#include "groupdecision/Statistics.h"

#include <algorithm>
#include <cmath>
#include <numeric>

namespace gd {

namespace {

std::vector<double> expertWeights(const GroupProblem& problem)
{
    double total = 0.0;
    for (const auto& e : problem.experts())
        total += e.weight;

    std::vector<double> w;
    for (const auto& e : problem.experts())
        w.push_back(e.weight / total);
    return w;
}

Matrix aggregate(const GroupProblem& problem, const std::vector<double>& w)
{
    Matrix result(problem.optionCount(), problem.criterionCount());
    for (std::size_t k = 0; k < problem.expertCount(); ++k) {
        const Matrix& s = problem.experts()[k].scores;
        for (std::size_t i = 0; i < s.rows(); ++i)
            for (std::size_t j = 0; j < s.cols(); ++j)
                result(i, j) += w[k] * s(i, j);
    }
    return result;
}

std::vector<double> weightedBorda(const std::vector<std::vector<double>>& individual,
                                  const std::vector<double>& w)
{
    std::vector<double> total(individual.front().size(), 0.0);
    for (std::size_t k = 0; k < individual.size(); ++k) {
        const std::vector<double> points = methods::bordaPoints(individual[k]);
        for (std::size_t i = 0; i < total.size(); ++i)
            total[i] += w[k] * points[i];
    }
    return total;
}

/// Sorts options by score (best first). Equal scores share a rank: 1, 1, 3, ...
std::vector<RankedOption> buildRanking(const std::vector<std::string>& options,
                                       const std::vector<double>& scores)
{
    constexpr double eps = 1e-9;
    std::vector<int> order(scores.size());
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(),
                     [&](int a, int b) { return scores[a] > scores[b]; });

    std::vector<RankedOption> ranking;
    ranking.reserve(order.size());
    for (std::size_t pos = 0; pos < order.size(); ++pos) {
        const int i = order[pos];
        int rank = static_cast<int>(pos) + 1;
        if (pos > 0 && std::abs(scores[i] - scores[order[pos - 1]]) <= eps)
            rank = ranking.back().rank;
        ranking.push_back({rank, i, options[i], scores[i]});
    }
    return ranking;
}

} // namespace

Matrix aggregateMatrix(const GroupProblem& problem)
{
    return aggregate(problem, expertWeights(problem));
}

GroupResult solve(const GroupProblem& problem, Method method)
{
    const std::vector<double> w = expertWeights(problem);

    // Each expert's own scores: used by Borda and to measure agreement with the group
    std::vector<std::vector<double>> individual;
    for (const auto& e : problem.experts()) {
        individual.push_back(method == Method::WeightedSum
                                 ? methods::weightedSum(e.scores, problem.criteria())
                                 : methods::topsis(e.scores, problem.criteria()));
    }

    std::vector<double> groupScores;
    switch (method) {
    case Method::Topsis:
        groupScores = methods::topsis(aggregate(problem, w), problem.criteria());
        break;
    case Method::WeightedSum:
        groupScores = methods::weightedSum(aggregate(problem, w), problem.criteria());
        break;
    case Method::Borda:
        groupScores = weightedBorda(individual, w);
        break;
    }

    GroupResult result;
    result.method = method;
    result.ranking = buildRanking(problem.options(), groupScores);

    double consensus = 0.0;
    for (std::size_t k = 0; k < individual.size(); ++k) {
        const double agreement = stats::spearman(individual[k], groupScores);
        result.expertAgreement.push_back(agreement);
        consensus += w[k] * agreement;
    }
    if (problem.expertCount() >= 2)
        result.consensus = consensus;

    return result;
}

std::vector<int> winners(const GroupResult& result)
{
    std::vector<int> first;
    for (const auto& r : result.ranking)
        if (r.rank == 1)
            first.push_back(r.optionIndex);
    std::sort(first.begin(), first.end());
    return first;
}

} // namespace gd
