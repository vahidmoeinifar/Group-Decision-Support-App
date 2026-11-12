//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////
///
#pragma once

#include "Domain.h"

#include <optional>

class Database;

/// All SQL for decisions lives here. No business rules: DecisionService decides
/// who may do what, this class only reads and writes rows.
class DecisionStore {
public:
    explicit DecisionStore(const Database& db) : db_(db) {}

    std::optional<Decision> load(int decisionId) const;
    QList<DecisionSummary> listForUser(int userId) const;

    /// Inserts the decision with its options, criteria and experts. Returns the new id.
    /// Experts whose email already has an account are linked to it.
    int insert(const NewDecision& input, int ownerId);
    void remove(int decisionId);

    /// Inserts or updates scores of one expert in one round.
    void saveScores(int expertId, int round, const QList<ScoreEntry>& entries);

    /// Copies every expert's scores from the current round into the next one and moves the decision on.
    void startNextRound(const Decision& decision);

    /// Marks the decision as changed, so other windows notice it.
    void touch(int decisionId, const QString& change);

    /// Version of a decision, or -1 when it no longer exists.
    int version(int decisionId) const;

private:
    const Database& db_;
};
