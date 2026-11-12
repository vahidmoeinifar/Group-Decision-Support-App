//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////
///
#pragma once

#include "DecisionStore.h"
#include "Domain.h"

#include "groupdecision/GroupProblem.h"

#include <optional>

class Database;

/// The rules of the app. Every method takes the signed-in user's id and checks what they may do:
///  - Only the organizer and invited experts can see a decision (others get "not found").
///  - Experts enter only their own scores, in the current round.
///  - Only the organizer can start a new round or delete the decision.
/// Errors are thrown as AppError with a message ready to show.
class DecisionService {
public:
    explicit DecisionService(const Database& db) : store_(db) {}

    QList<DecisionSummary> list(int userId) const;
    Decision get(int decisionId, int userId) const;
    int create(int userId, NewDecision input);
    void remove(int decisionId, int userId);

    /// Scores of one expert. round 0 means the current round.
    ScoreSheet scores(int decisionId, int userId, int expertId, int round = 0) const;
    void saveScores(int decisionId, int userId, int expertId, const QList<ScoreEntry>& entries);

    ResultView result(int decisionId, int userId, gd::Method method, int round = 0) const;
    QList<ResultView> compare(int decisionId, int userId, int round = 0) const;
    QList<RoundSummary> history(int decisionId, int userId) const;
    FeedbackView feedback(int decisionId, int userId, int expertId) const;
    SensitivityView sensitivity(int decisionId, int userId, gd::Method method, int round = 0) const;

    /// Organizer only. Needs at least two experts with complete scores. Returns the new round number.
    int startNextRound(int decisionId, int userId);

    /// Version number of a decision (-1 when deleted); used to notice changes from other windows.
    int version(int decisionId) const { return store_.version(decisionId); }

private:
    /// The experts with complete scores in one round, turned into an engine problem.
    struct Prepared {
        int round = 1;
        QList<ExpertRow> included;
        QStringList skipped;
        std::optional<gd::GroupProblem> problem;
    };

    Decision loadForMember(int decisionId, int userId) const;
    static int resolveRound(const Decision& d, int round);
    static Prepared prepare(const Decision& d, int round);
    static ResultView toView(const Decision& d, const Prepared& p, const gd::GroupResult& r);
    static void validate(NewDecision& input);

    DecisionStore store_;
};
