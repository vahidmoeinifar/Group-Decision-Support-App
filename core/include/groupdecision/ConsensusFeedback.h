//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////
#pragma once

#include "groupdecision/GroupProblem.h"

#include <string>
#include <vector>

namespace gd {

struct CellFeedback {
    int optionIndex = 0;
    int criterionIndex = 0;
    double score = 0.0;          ///< The expert's own score.
    double groupScore = 0.0;     ///< Weighted mean of all experts.
    double deviation = 0.0;      ///< |score - group| divided by the criterion's spread (0..1).
    double suggestedScore = 0.0; ///< Score moved part of the way toward the group.
    bool flagged = false;        ///< deviation above the threshold.
};

struct ExpertFeedback {
    std::string expert;
    /// Spearman correlation between the expert's own TOPSIS ranking and the group's.
    double agreement = 0.0;
    std::vector<CellFeedback> cells; ///< One per option x criterion, row by row.

    int flaggedCount() const;
};

/// Feedback for one round of a consensus reaching process: shows an expert where their
/// scores are far from the group and suggests values that bring them closer.
/// threshold: cells with a scaled deviation above it are flagged (0..1).
/// step: how far a suggestion moves toward the group (0 = not at all, 1 = all the way).
ExpertFeedback consensusFeedback(const GroupProblem& problem, int expertIndex,
                                 double threshold = 0.2, double step = 0.5);

} // namespace gd
