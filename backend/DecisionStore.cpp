#include "DecisionStore.h"

#include "Database.h"

#include <QSqlQuery>
#include <QVariant>

namespace {

QString nowText() { return QDateTime::currentDateTimeUtc().toString(Qt::ISODate); }

QVariant nullableInt(const std::optional<int>& v) { return v ? QVariant(*v) : QVariant(); }

} // namespace

std::optional<Decision> DecisionStore::load(int decisionId) const
{
    QSqlQuery q = db_.exec(QStringLiteral(
        "SELECT d.id, d.title, d.description, d.owner_id, u.display_name, d.current_round, "
        "       d.consensus_target, d.version, d.last_change, d.created_at "
        "FROM decisions d JOIN users u ON u.id = d.owner_id WHERE d.id = ?"), {decisionId});
    if (!q.next())
        return std::nullopt;

    Decision d;
    d.id = q.value(0).toInt();
    d.title = q.value(1).toString();
    d.description = q.value(2).toString();
    d.ownerId = q.value(3).toInt();
    d.ownerName = q.value(4).toString();
    d.currentRound = q.value(5).toInt();
    d.consensusTarget = q.value(6).toDouble();
    d.version = q.value(7).toInt();
    d.lastChange = q.value(8).toString();
    d.createdAt = QDateTime::fromString(q.value(9).toString(), Qt::ISODate);

    q = db_.exec(QStringLiteral("SELECT id, name FROM options WHERE decision_id = ? ORDER BY position"), {decisionId});
    while (q.next())
        d.options.append({q.value(0).toInt(), q.value(1).toString()});

    q = db_.exec(QStringLiteral(
        "SELECT id, name, weight, type FROM criteria WHERE decision_id = ? ORDER BY position"), {decisionId});
    while (q.next()) {
        d.criteria.append({q.value(0).toInt(), q.value(1).toString(), q.value(2).toDouble(),
                           q.value(3).toInt() == 1 ? gd::CriterionType::Cost : gd::CriterionType::Benefit});
    }

    q = db_.exec(QStringLiteral(
        "SELECT id, name, email, user_id, weight FROM experts WHERE decision_id = ? ORDER BY position"), {decisionId});
    while (q.next()) {
        ExpertRow e;
        e.id = q.value(0).toInt();
        e.name = q.value(1).toString();
        e.email = q.value(2).toString();
        if (!q.value(3).isNull())
            e.userId = q.value(3).toInt();
        e.weight = q.value(4).toDouble();
        d.experts.append(e);
    }

    q = db_.exec(QStringLiteral(
        "SELECT v.round, v.expert_id, v.option_id, v.criterion_id, v.score "
        "FROM evaluations v JOIN experts x ON x.id = v.expert_id WHERE x.decision_id = ?"), {decisionId});
    while (q.next()) {
        d.scores[q.value(0).toInt()][q.value(1).toInt()].insert(
            CellKey(q.value(2).toInt(), q.value(3).toInt()), q.value(4).toDouble());
    }
    return d;
}

QList<DecisionSummary> DecisionStore::listForUser(int userId) const
{
    QSqlQuery q = db_.exec(QStringLiteral(
        "SELECT d.id, d.title, d.current_round, d.owner_id = ?, "
        "       EXISTS (SELECT 1 FROM experts x WHERE x.decision_id = d.id AND x.user_id = ?), d.created_at "
        "FROM decisions d "
        "WHERE d.owner_id = ? OR EXISTS (SELECT 1 FROM experts x WHERE x.decision_id = d.id AND x.user_id = ?) "
        "ORDER BY d.created_at DESC, d.id DESC"), {userId, userId, userId, userId});

    QList<DecisionSummary> list;
    while (q.next()) {
        list.append({q.value(0).toInt(), q.value(1).toString(), q.value(2).toInt(), q.value(3).toBool(),
                     q.value(4).toBool(), QDateTime::fromString(q.value(5).toString(), Qt::ISODate)});
    }
    return list;
}

int DecisionStore::insert(const NewDecision& input, int ownerId)
{
    Transaction tx(db_);

    db_.exec(QStringLiteral(
        "INSERT INTO decisions (title, description, owner_id, current_round, consensus_target, created_at) "
        "VALUES (?, ?, ?, 1, ?, ?)"),
        // A null QString would be stored as SQL NULL, so pass an empty string instead
        {input.title, input.description.isNull() ? QStringLiteral("") : input.description, ownerId,
         input.consensusTarget, nowText()});
    const int id = db_.lastInsertId();

    for (int i = 0; i < input.options.size(); ++i)
        db_.exec(QStringLiteral("INSERT INTO options (decision_id, name, position) VALUES (?, ?, ?)"),
                 {id, input.options[i], i});

    for (int i = 0; i < input.criteria.size(); ++i) {
        const NewCriterion& c = input.criteria[i];
        db_.exec(QStringLiteral("INSERT INTO criteria (decision_id, name, weight, type, position) VALUES (?, ?, ?, ?, ?)"),
                 {id, c.name, c.weight, c.type == gd::CriterionType::Cost ? 1 : 0, i});
    }

    for (int i = 0; i < input.experts.size(); ++i) {
        const NewExpert& e = input.experts[i];
        std::optional<int> userId;
        QSqlQuery u = db_.exec(QStringLiteral("SELECT id FROM users WHERE email = ?"), {e.email});
        if (u.next())
            userId = u.value(0).toInt();

        db_.exec(QStringLiteral(
            "INSERT INTO experts (decision_id, name, email, user_id, weight, position) VALUES (?, ?, ?, ?, ?, ?)"),
            {id, e.name, e.email, nullableInt(userId), e.weight, i});
    }

    tx.commit();
    return id;
}

void DecisionStore::remove(int decisionId)
{
    // Child rows go too, through ON DELETE CASCADE
    db_.exec(QStringLiteral("DELETE FROM decisions WHERE id = ?"), {decisionId});
}

void DecisionStore::saveScores(int expertId, int round, const QList<ScoreEntry>& entries)
{
    Transaction tx(db_);
    for (const ScoreEntry& s : entries) {
        db_.exec(QStringLiteral(
            "INSERT INTO evaluations (expert_id, option_id, criterion_id, round, score) VALUES (?, ?, ?, ?, ?) "
            "ON CONFLICT (expert_id, option_id, criterion_id, round) DO UPDATE SET score = excluded.score"),
            {expertId, s.optionId, s.criterionId, round, s.score});
    }
    tx.commit();
}

void DecisionStore::startNextRound(const Decision& decision)
{
    Transaction tx(db_);
    const int next = decision.currentRound + 1;
    db_.exec(QStringLiteral(
        "INSERT OR REPLACE INTO evaluations (expert_id, option_id, criterion_id, round, score) "
        "SELECT v.expert_id, v.option_id, v.criterion_id, ?, v.score "
        "FROM evaluations v JOIN experts x ON x.id = v.expert_id "
        "WHERE x.decision_id = ? AND v.round = ?"),
        {next, decision.id, decision.currentRound});
    db_.exec(QStringLiteral("UPDATE decisions SET current_round = ? WHERE id = ?"), {next, decision.id});
    tx.commit();
}

void DecisionStore::touch(int decisionId, const QString& change)
{
    db_.exec(QStringLiteral("UPDATE decisions SET version = version + 1, last_change = ? WHERE id = ?"),
             {change, decisionId});
}

int DecisionStore::version(int decisionId) const
{
    QSqlQuery q = db_.exec(QStringLiteral("SELECT version FROM decisions WHERE id = ?"), {decisionId});
    return q.next() ? q.value(0).toInt() : -1;
}
