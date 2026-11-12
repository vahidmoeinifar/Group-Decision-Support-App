#include "groupdecision/Statistics.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <stdexcept>

namespace gd::stats {

std::vector<double> averageRanks(const std::vector<double>& values)
{
    const std::size_t n = values.size();
    std::vector<std::size_t> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::stable_sort(order.begin(), order.end(),
                     [&](std::size_t a, std::size_t b) { return values[a] < values[b]; });

    std::vector<double> ranks(n);
    std::size_t pos = 0;
    while (pos < n) {
        std::size_t end = pos;
        while (end + 1 < n && values[order[end + 1]] == values[order[pos]])
            ++end;
        const double average = (static_cast<double>(pos + end)) / 2.0 + 1.0;
        for (std::size_t k = pos; k <= end; ++k)
            ranks[order[k]] = average;
        pos = end + 1;
    }
    return ranks;
}

double spearman(const std::vector<double>& a, const std::vector<double>& b)
{
    if (a.size() != b.size())
        throw std::invalid_argument("Vectors must have the same length.");
    if (a.empty())
        return 1.0;

    const std::vector<double> ra = averageRanks(a);
    const std::vector<double> rb = averageRanks(b);
    const double n = static_cast<double>(ra.size());
    const double ma = std::accumulate(ra.begin(), ra.end(), 0.0) / n;
    const double mb = std::accumulate(rb.begin(), rb.end(), 0.0) / n;

    double cov = 0.0, va = 0.0, vb = 0.0;
    for (std::size_t i = 0; i < ra.size(); ++i) {
        cov += (ra[i] - ma) * (rb[i] - mb);
        va += (ra[i] - ma) * (ra[i] - ma);
        vb += (rb[i] - mb) * (rb[i] - mb);
    }
    if (va == 0.0 && vb == 0.0)
        return 1.0; // both rankings fully tied
    if (va == 0.0 || vb == 0.0)
        return 0.0; // one ranking carries no order
    return cov / std::sqrt(va * vb);
}

} // namespace gd::stats
