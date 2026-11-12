#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace gd {

/// Whether a higher value is better (Benefit) or worse (Cost).
enum class CriterionType { Benefit, Cost };

/// How expert opinions are combined into one group ranking.
enum class Method {
    Topsis,      ///< Weighted mean of expert scores, then TOPSIS.
    WeightedSum, ///< Weighted mean of expert scores, then weighted sum.
    Borda        ///< Each expert ranks with TOPSIS, rankings combined by weighted Borda count.
};

struct Criterion {
    std::string name;
    double weight = 1.0;
    CriterionType type = CriterionType::Benefit;
};

/// A small dense matrix stored row by row. Used for scores[option][criterion].
class Matrix {
public:
    Matrix() = default;
    Matrix(std::size_t rows, std::size_t cols, double value = 0.0)
        : rows_(rows), cols_(cols), data_(rows * cols, value) {}

    double& operator()(std::size_t row, std::size_t col) { return data_[row * cols_ + col]; }
    double operator()(std::size_t row, std::size_t col) const { return data_[row * cols_ + col]; }

    std::size_t rows() const { return rows_; }
    std::size_t cols() const { return cols_; }
    const std::vector<double>& values() const { return data_; }

private:
    std::size_t rows_ = 0;
    std::size_t cols_ = 0;
    std::vector<double> data_;
};

/// One expert's scores for every option on every criterion.
struct ExpertInput {
    std::string name;
    double weight = 1.0;
    Matrix scores; ///< rows = options, cols = criteria
};

struct RankedOption {
    int rank = 0;        ///< 1 = best. Equal scores share a rank (1, 1, 3, ...).
    int optionIndex = 0; ///< Position of the option in the problem.
    std::string option;
    double score = 0.0;  ///< Method score, higher is better.
};

struct GroupResult {
    Method method = Method::Topsis;
    std::vector<RankedOption> ranking;
    /// Weighted mean Spearman correlation between each expert and the group (-1..1).
    /// Empty when there is only one expert.
    std::optional<double> consensus;
    /// Spearman correlation of each expert's own ranking with the group ranking, in input order.
    std::vector<double> expertAgreement;
};

/// Human-readable method name, e.g. "TOPSIS".
std::string toString(Method method);

} // namespace gd
