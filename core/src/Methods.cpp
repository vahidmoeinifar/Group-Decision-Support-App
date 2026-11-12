#include "groupdecision/Methods.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace gd::methods {

std::vector<double> normalizedWeights(const std::vector<Criterion>& criteria)
{
    double total = 0.0;
    for (const auto& c : criteria)
        total += c.weight;

    std::vector<double> w;
    w.reserve(criteria.size());
    for (const auto& c : criteria)
        w.push_back(c.weight / total);
    return w;
}

std::vector<double> topsis(const Matrix& scores, const std::vector<Criterion>& criteria)
{
    const std::size_t m = scores.rows();
    const std::size_t n = scores.cols();
    const std::vector<double> w = normalizedWeights(criteria);

    // 1. Vector normalization and weighting
    Matrix v(m, n);
    for (std::size_t j = 0; j < n; ++j) {
        double norm = 0.0;
        for (std::size_t i = 0; i < m; ++i)
            norm += scores(i, j) * scores(i, j);
        norm = std::sqrt(norm);

        for (std::size_t i = 0; i < m; ++i)
            v(i, j) = norm == 0.0 ? 0.0 : w[j] * scores(i, j) / norm;
    }

    // 2. Ideal (best) and anti-ideal (worst) points
    std::vector<double> best(n), worst(n);
    for (std::size_t j = 0; j < n; ++j) {
        double hi = std::numeric_limits<double>::lowest();
        double lo = std::numeric_limits<double>::max();
        for (std::size_t i = 0; i < m; ++i) {
            hi = std::max(hi, v(i, j));
            lo = std::min(lo, v(i, j));
        }
        const bool benefit = criteria[j].type == CriterionType::Benefit;
        best[j] = benefit ? hi : lo;
        worst[j] = benefit ? lo : hi;
    }

    // 3. Distances and relative closeness
    std::vector<double> result(m);
    for (std::size_t i = 0; i < m; ++i) {
        double dBest = 0.0, dWorst = 0.0;
        for (std::size_t j = 0; j < n; ++j) {
            dBest += (v(i, j) - best[j]) * (v(i, j) - best[j]);
            dWorst += (v(i, j) - worst[j]) * (v(i, j) - worst[j]);
        }
        dBest = std::sqrt(dBest);
        dWorst = std::sqrt(dWorst);

        const double total = dBest + dWorst;
        result[i] = total == 0.0 ? 0.5 : dWorst / total; // all options identical: a tie
    }
    return result;
}

std::vector<double> weightedSum(const Matrix& scores, const std::vector<Criterion>& criteria)
{
    const std::size_t m = scores.rows();
    const std::size_t n = scores.cols();
    const std::vector<double> w = normalizedWeights(criteria);
    std::vector<double> result(m, 0.0);

    for (std::size_t j = 0; j < n; ++j) {
        double hi = std::numeric_limits<double>::lowest();
        double lo = std::numeric_limits<double>::max();
        for (std::size_t i = 0; i < m; ++i) {
            hi = std::max(hi, scores(i, j));
            lo = std::min(lo, scores(i, j));
        }
        const double range = hi - lo;

        for (std::size_t i = 0; i < m; ++i) {
            double normalized = 1.0; // criterion does not separate the options
            if (range != 0.0) {
                normalized = criteria[j].type == CriterionType::Benefit
                    ? (scores(i, j) - lo) / range
                    : (hi - scores(i, j)) / range;
            }
            result[i] += w[j] * normalized;
        }
    }
    return result;
}

std::vector<double> bordaPoints(const std::vector<double>& scores)
{
    constexpr double eps = 1e-12;
    std::vector<double> points(scores.size(), 0.0);
    for (std::size_t i = 0; i < scores.size(); ++i) {
        for (std::size_t k = 0; k < scores.size(); ++k) {
            if (i == k)
                continue;
            const double diff = scores[i] - scores[k];
            if (diff > eps)
                points[i] += 1.0;
            else if (std::abs(diff) <= eps)
                points[i] += 0.5;
        }
    }
    return points;
}

} // namespace gd::methods
