#include "DecisionService.h"

#include "AppError.h"
#include "AuthService.h"

#include "groupdecision/ConsensusFeedback.h"
#include "groupdecision/Engine.h"
#include "groupdecision/Sensitivity.h"

#include <QSet>

#include <cmath>
#include <stdexcept>

namespace {

const QList<gd::Method> kAllMethods = {gd::Method::Topsis, gd::Method::WeightedSum, gd::Method::Borda};

void requireUnique(const QStringList& names, const QString& label)
{
    QSet<QString> seen;
    for (const QString& n : names) {
        if (n.isEmpty())
            throw AppError::validation(QStringLiteral("Every %1 needs a name.").arg(label));
        if (seen.contains(n.toLower()))
            throw AppError::validation(QStringLiteral("The %1 '%2' is listed more than once.").arg(label, n));
        seen.insert(n.toLower());
    }
}

bool validWeight(double w) { return std::isfinite(w) && w >= 0.0; }

} // namespace

// ---------- Reading ----------

QList<DecisionSummary> DecisionService::list(int userId) const
{
    return store_.listForUser(userId);
}

Decision DecisionService::get(int decisionId, int userId) const
{
    return loadForMember(decisionId, userId);
}

Decision DecisionService::loadForMember(int decisionId, int userId) const
{
    std::optional<Decision> d = store_.load(decisionId);
    // Outsiders get "not found" rather than "forbidden", so they cannot tell the decision exists
    if (!d || !d->isMember(userId))
        throw AppError::notFound(QStringLiteral("This decision does not exist or you are not part of it."));
    return *d;
}

int DecisionService::resolveRound(const Decision& d, int round)
{
    const int r = round == 0 ? d.currentRound : round;
    if (r < 1 || r > d.currentRound)
        throw AppError::validation(
            QStringLiteral("Round %1 does not exist. This decision is in round %2.").arg(r).arg(d.currentRound));
    return r;
}

ScoreSheet DecisionService::scores(int decisionId, int userId, int expertId, int round) const
{
    const Decision d = loadForMember(decisionId, userId);
    const ExpertRow* expert = d.expert(expertId);
    if (!expert)
        throw AppError::notFound(QStringLiteral("This expert is not part of the decision."));

    const bool own = expert->userId && *expert->userId == userId;
    if (!own && !d.isOwner(userId))
        throw AppError::forbidden(QStringLiteral("You can only see your own scores."));

    return d.sheet(resolveRound(d, round), expertId);
}

// ---------- Changing ----------

void DecisionService::validate(NewDecision& in)
{
    in.title = in.title.trimmed();
    in.description = in.description.trimmed();
    if (in.title.isEmpty())
        throw AppError::validation(QStringLiteral("Please give the decision a title."));
    if (in.title.size() > 200)
        throw AppError::validation(QStringLiteral("The title can have at most 200 characters."));
    if (!std::isfinite(in.consensusTarget) || in.consensusTarget < 0.0 || in.consensusTarget > 1.0)
        throw AppError::validation(QStringLiteral("The agreement target must be between 0 and 1."));

    for (QString& o : in.options)
        o = o.trimmed();
    if (in.options.size() < 2)
        throw AppError::validation(QStringLiteral("Add at least two options to choose between."));
    requireUnique(in.options, QStringLiteral("option"));

    if (in.criteria.isEmpty())
        throw AppError::validation(QStringLiteral("Add at least one criterion."));
    QStringList names;
    double criterionTotal = 0.0;
    for (NewCriterion& c : in.criteria) {
        c.name = c.name.trimmed();
        names << c.name;
        if (!validWeight(c.weight))
            throw AppError::validation(QStringLiteral("Criterion weights must be zero or more."));
        criterionTotal += c.weight;
    }
    requireUnique(names, QStringLiteral("criterion"));
    if (criterionTotal <= 0.0)
        throw AppError::validation(QStringLiteral("At least one criterion weight must be positive."));

    if (in.experts.isEmpty())
        throw AppError::validation(QStringLiteral("Add at least one expert."));
    QStringList emails;
    names.clear();
    double expertTotal = 0.0;
    for (NewExpert& e : in.experts) {
        e.name = e.name.trimmed();
        e.email = e.email.trimmed().toLower();
        names << e.name;
        emails << e.email;
        if (!AuthService::looksLikeEmail(e.email))
            throw AppError::validation(QStringLiteral("'%1' is not a valid email address.").arg(e.email));
        if (!validWeight(e.weight))
            throw AppError::validation(QStringLiteral("Expert weights must be zero or more."));
        expertTotal += e.weight;
    }
    requireUnique(names, QStringLiteral("expert"));
    requireUnique(emails, QStringLiteral("email"));
    if (expertTotal <= 0.0)
        throw AppError::validation(QStringLiteral("At least one expert weight must be positive."));
}

int DecisionService::create(int userId, NewDecision input)
{
    validate(input);
    return store_.insert(input, userId);
}

void DecisionService::remove(int decisionId, int userId)
{
    const Decision d = loadForMember(decisionId, userId);
    if (!d.isOwner(userId))
        throw AppError::forbidden(QStringLiteral("Only the organizer can delete this decision."));
    store_.remove(decisionId);
}

void DecisionService::saveScores(int decisionId, int userId, int expertId, const QList<ScoreEntry>& entries)
{
    const Decision d = loadForMember(decisionId, userId);
    const ExpertRow* expert = d.expert(expertId);
    if (!expert)
        throw AppError::notFound(QStringLiteral("This expert is not part of the decision."));
    if (!expert->userId || *expert->userId != userId)
        throw AppError::forbidden(QStringLiteral("You can only enter your own scores."));
    if (entries.isEmpty())
        throw AppError::validation(QStringLiteral("There are no scores to save."));

    QSet<int> optionIds, criterionIds;
    for (const auto& o : d.options)
        optionIds.insert(o.id);
    for (const auto& c : d.criteria)
        criterionIds.insert(c.id);

    for (const ScoreEntry& s : entries) {
        if (!optionIds.contains(s.optionId))
            throw AppError::validation(QStringLiteral("Option %1 does not belong to this decision.").arg(s.optionId));
        if (!criterionIds.contains(s.criterionId))
            throw AppError::validation(QStringLiteral("Criterion %1 does not belong to this decision.").arg(s.criterionId));
        if (!std::isfinite(s.score))
            throw AppError::validation(QStringLiteral("Scores must be numbers."));
    }

    store_.saveScores(expertId, d.currentRound, entries);
    store_.touch(decisionId, QStringLiteral("%1 saved new scores.").arg(expert->name));
}

int DecisionService::startNextRound(int decisionId, int userId)
{
    const Decision d = loadForMember(decisionId, userId);
    if (!d.isOwner(userId))
        throw AppError::forbidden(QStringLiteral("Only the organizer can start a new round."));

    const Prepared p = prepare(d, d.currentRound);
    if (p.included.size() < 2)
        throw AppError::validation(QStringLiteral("At least two experts need complete scores before a new round."));

    store_.startNextRound(d);
    const int next = d.currentRound + 1;
    store_.touch(decisionId, QStringLiteral("Round %1 has started.").arg(next));
    return next;
}

// ---------- Results ----------

DecisionService::Prepared DecisionService::prepare(const Decision& d, int round)
{
    Prepared p;
    p.round = round;

    std::vector<gd::ExpertInput> experts;
    for (const ExpertRow& e : d.experts) {
        if (!d.isComplete(round, e.id)) {
            p.skipped << e.name;
            continue;
        }
        const ScoreSheet sheet = d.sheet(round, e.id);
        gd::Matrix m(static_cast<std::size_t>(d.options.size()), static_cast<std::size_t>(d.criteria.size()));
        for (int i = 0; i < d.options.size(); ++i)
            for (int j = 0; j < d.criteria.size(); ++j)
                m(i, j) = sheet.value(CellKey(d.options[i].id, d.criteria[j].id));
        experts.push_back({e.name.toStdString(), e.weight, m});
        p.included << e;
    }

    if (experts.empty())
        throw AppError::validation(QStringLiteral("No expert has finished scoring in round %1 yet.").arg(round));

    std::vector<std::string> options;
    for (const auto& o : d.options)
        options.push_back(o.name.toStdString());
    std::vector<gd::Criterion> criteria;
    for (const auto& c : d.criteria)
        criteria.push_back({c.name.toStdString(), c.weight, c.type});

    try {
        p.problem.emplace(std::move(options), std::move(criteria), std::move(experts));
    } catch (const std::invalid_argument& ex) {
        throw AppError::validation(QString::fromStdString(ex.what()));
    }
    return p;
}

ResultView DecisionService::toView(const Decision& d, const Prepared& p, const gd::GroupResult& r)
{
    ResultView v;
    v.method = r.method;
    v.round = p.round;
    v.consensus = r.consensus;
    v.skipped = p.skipped;
    for (const auto& item : r.ranking)
        v.ranking.append({item.rank, d.options[item.optionIndex].id, d.options[item.optionIndex].name, item.score});
    for (int k = 0; k < p.included.size(); ++k)
        v.experts.append({p.included[k].id, p.included[k].name, r.expertAgreement[static_cast<std::size_t>(k)]});
    return v;
}

ResultView DecisionService::result(int decisionId, int userId, gd::Method method, int round) const
{
    const Decision d = loadForMember(decisionId, userId);
    const Prepared p = prepare(d, resolveRound(d, round));
    return toView(d, p, gd::solve(*p.problem, method));
}

QList<ResultView> DecisionService::compare(int decisionId, int userId, int round) const
{
    const Decision d = loadForMember(decisionId, userId);
    const Prepared p = prepare(d, resolveRound(d, round));
    QList<ResultView> list;
    for (gd::Method m : kAllMethods)
        list.append(toView(d, p, gd::solve(*p.problem, m)));
    return list;
}

QList<RoundSummary> DecisionService::history(int decisionId, int userId) const
{
    const Decision d = loadForMember(decisionId, userId);
    QList<RoundSummary> list;
    for (int r = 1; r <= d.currentRound; ++r) {
        RoundSummary s;
        s.round = r;
        s.isCurrent = r == d.currentRound;
        try {
            const Prepared p = prepare(d, r);
            const gd::GroupResult result = gd::solve(*p.problem, gd::Method::Topsis);
            s.consensus = result.consensus;
            s.expertCount = static_cast<int>(p.included.size());
            for (int w : gd::winners(result))
                s.winners << d.options[w].name;
        } catch (const AppError&) {
            // Nobody finished this round yet: keep it in the list without a value
        }
        list.append(s);
    }
    return list;
}

FeedbackView DecisionService::feedback(int decisionId, int userId, int expertId) const
{
    const Decision d = loadForMember(decisionId, userId);
    const ExpertRow* expert = d.expert(expertId);
    if (!expert)
        throw AppError::notFound(QStringLiteral("This expert is not part of the decision."));
    const bool own = expert->userId && *expert->userId == userId;
    if (!own && !d.isOwner(userId))
        throw AppError::forbidden(QStringLiteral("You can only see feedback on your own scores."));

    const Prepared p = prepare(d, d.currentRound);
    int index = -1;
    for (int k = 0; k < p.included.size(); ++k)
        if (p.included[k].id == expertId)
            index = k;
    if (index < 0)
        throw AppError::validation(QStringLiteral("%1 has not finished scoring in this round yet.").arg(expert->name));
    if (p.included.size() < 2)
        throw AppError::validation(QStringLiteral("Feedback needs at least two experts with complete scores."));

    const gd::ExpertFeedback f = gd::consensusFeedback(*p.problem, index);
    FeedbackView v;
    v.expertId = expertId;
    v.expert = expert->name;
    v.round = d.currentRound;
    v.agreement = f.agreement;
    v.flaggedCount = f.flaggedCount();
    for (const auto& c : f.cells) {
        v.cells.append({d.options[c.optionIndex].id, d.criteria[c.criterionIndex].id, c.score, c.groupScore,
                        c.deviation, c.suggestedScore, c.flagged});
    }
    return v;
}

SensitivityView DecisionService::sensitivity(int decisionId, int userId, gd::Method method, int round) const
{
    const Decision d = loadForMember(decisionId, userId);
    const Prepared p = prepare(d, resolveRound(d, round));
    const gd::SensitivityResult s = gd::analyzeSensitivity(*p.problem, method);

    auto names = [&](const std::vector<int>& indexes, QList<int>& ids, QStringList& labels) {
        for (int i : indexes) {
            ids << d.options[i].id;
            labels << d.options[i].name;
        }
    };

    SensitivityView v;
    v.method = method;
    v.round = p.round;
    names(s.currentWinners, v.currentWinnerIds, v.currentWinners);
    for (const auto& c : s.criteria) {
        CriterionSensitivityView cv;
        cv.criterionId = d.criteria[c.criterionIndex].id;
        cv.criterion = d.criteria[c.criterionIndex].name;
        cv.currentShare = c.currentShare;
        cv.stableFrom = c.stableFrom;
        cv.stableTo = c.stableTo;
        for (const auto& seg : c.segments) {
            SegmentView sv;
            sv.from = seg.from;
            sv.to = seg.to;
            names(seg.winners, sv.winnerOptionIds, sv.winners);
            cv.segments.append(sv);
        }
        v.criteria.append(cv);
    }
    if (s.mostSensitiveCriterion)
        v.mostSensitiveCriterionId = d.criteria[*s.mostSensitiveCriterion].id;
    return v;
}
