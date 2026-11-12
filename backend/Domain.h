//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////
///
#pragma once

#include "groupdecision/Types.h"

#include <QDateTime>
#include <QHash>
#include <QList>
#include <QMap>
#include <QPair>
#include <QString>
#include <QStringList>

#include <optional>

// Plain data types shared by the storage, the services and the UI layer.

struct User {
    int id = 0;
    QString email;
    QString displayName;
};

struct OptionRow {
    int id = 0;
    QString name;
};

struct CriterionRow {
    int id = 0;
    QString name;
    double weight = 1.0;
    gd::CriterionType type = gd::CriterionType::Benefit;
};

struct ExpertRow {
    int id = 0;
    QString name;
    QString email;
    std::optional<int> userId; ///< Empty while the invited person has no account.
    double weight = 1.0;
};

using CellKey = QPair<int, int>;            ///< (optionId, criterionId)
using ScoreSheet = QHash<CellKey, double>;  ///< One expert's scores in one round.

struct ScoreEntry {
    int optionId = 0;
    int criterionId = 0;
    double score = 0.0;
};

/// A decision with everything that belongs to it, loaded in one go.
struct Decision {
    int id = 0;
    QString title;
    QString description;
    int ownerId = 0;
    QString ownerName;
    int currentRound = 1;
    double consensusTarget = 0.8;
    int version = 0;     ///< Goes up on every change; used for live updates.
    QString lastChange;  ///< Short text describing the latest change.
    QDateTime createdAt;

    QList<OptionRow> options;
    QList<CriterionRow> criteria;
    QList<ExpertRow> experts;
    QMap<int, QHash<int, ScoreSheet>> scores; ///< round -> expertId -> scores

    int cellCount() const { return static_cast<int>(options.size() * criteria.size()); }

    ScoreSheet sheet(int round, int expertId) const { return scores.value(round).value(expertId); }

    bool isComplete(int round, int expertId) const
    {
        return cellCount() > 0 && sheet(round, expertId).size() == cellCount();
    }

    const ExpertRow* expert(int expertId) const
    {
        for (const auto& e : experts)
            if (e.id == expertId)
                return &e;
        return nullptr;
    }

    const ExpertRow* expertForUser(int userId) const
    {
        for (const auto& e : experts)
            if (e.userId && *e.userId == userId)
                return &e;
        return nullptr;
    }

    bool isOwner(int userId) const { return ownerId == userId; }
    bool isMember(int userId) const { return isOwner(userId) || expertForUser(userId) != nullptr; }
};

struct DecisionSummary {
    int id = 0;
    QString title;
    int currentRound = 1;
    bool isOwner = false;
    bool isExpert = false;
    QDateTime createdAt;
};

// ---- Input for creating a decision ----

struct NewCriterion {
    QString name;
    double weight = 1.0;
    gd::CriterionType type = gd::CriterionType::Benefit;
};

struct NewExpert {
    QString name;
    QString email;
    double weight = 1.0;
};

struct NewDecision {
    QString title;
    QString description;
    double consensusTarget = 0.8;
    QStringList options;
    QList<NewCriterion> criteria;
    QList<NewExpert> experts;
};

// ---- Results, ready for display ----

struct RankingEntry {
    int rank = 0;
    int optionId = 0;
    QString option;
    double score = 0.0;
};

struct ExpertAgreement {
    int expertId = 0;
    QString expert;
    double agreement = 0.0;
};

struct ResultView {
    gd::Method method = gd::Method::Topsis;
    int round = 1;
    QList<RankingEntry> ranking;
    std::optional<double> consensus; ///< Empty when only one expert is complete.
    QList<ExpertAgreement> experts;
    QStringList skipped;             ///< Experts left out because their scores are incomplete.
};

struct RoundSummary {
    int round = 1;
    std::optional<double> consensus;
    QStringList winners;
    int expertCount = 0;
    bool isCurrent = false;
};

struct CellFeedbackView {
    int optionId = 0;
    int criterionId = 0;
    double score = 0.0;
    double groupScore = 0.0;
    double deviation = 0.0;
    double suggestedScore = 0.0;
    bool flagged = false;
};

struct FeedbackView {
    int expertId = 0;
    QString expert;
    int round = 1;
    double agreement = 0.0;
    int flaggedCount = 0;
    QList<CellFeedbackView> cells;
};

struct SegmentView {
    double from = 0.0;
    double to = 1.0;
    QList<int> winnerOptionIds;
    QStringList winners;
};

struct CriterionSensitivityView {
    int criterionId = 0;
    QString criterion;
    double currentShare = 0.0;
    double stableFrom = 0.0;
    double stableTo = 1.0;
    QList<SegmentView> segments;
};

struct SensitivityView {
    gd::Method method = gd::Method::Topsis;
    int round = 1;
    QList<int> currentWinnerIds;
    QStringList currentWinners;
    QList<CriterionSensitivityView> criteria;
    std::optional<int> mostSensitiveCriterionId;
};
