//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////

#include "DecisionController.h"

#include "AppError.h"
#include "DecisionService.h"
#include "Session.h"

#include <QLocale>

#include <cmath>

namespace {

QString methodName(gd::Method m)
{
    switch (m) {
    case gd::Method::Topsis: return QStringLiteral("Topsis");
    case gd::Method::WeightedSum: return QStringLiteral("WeightedSum");
    case gd::Method::Borda: return QStringLiteral("Borda");
    }
    return {};
}

QVariant optionalNumber(const std::optional<double>& v)
{
    return v ? QVariant(*v) : QVariant();
}

QVariantList toVariantList(const QList<int>& ids)
{
    QVariantList list;
    for (int id : ids)
        list << id;
    return list;
}

QVariantMap rankingToMap(const ResultView& r)
{
    QVariantList ranking;
    const double top = r.ranking.isEmpty() ? 0.0 : r.ranking.first().score;
    for (const RankingEntry& e : r.ranking) {
        ranking << QVariantMap{{"rank", e.rank}, {"optionId", e.optionId}, {"option", e.option},
                               {"score", e.score}, {"share", top > 0.0 ? e.score / top : 0.0}};
    }
    QVariantList experts;
    for (const ExpertAgreement& a : r.experts)
        experts << QVariantMap{{"expertId", a.expertId}, {"expert", a.expert}, {"agreement", a.agreement}};

    return {{"ok", true},
            {"method", methodName(r.method)},
            {"round", r.round},
            {"ranking", ranking},
            {"consensus", optionalNumber(r.consensus)},
            {"experts", experts},
            {"skipped", r.skipped}};
}

} // namespace

DecisionController::DecisionController(DecisionService& service, Session& session, QObject* parent)
    : QObject(parent), service_(service), session_(session)
{
}

// ---------- Properties ----------

QString DecisionController::createdText() const
{
    return decision_ ? QLocale().toString(decision_->createdAt.toLocalTime().date(), QLocale::LongFormat) : QString();
}

bool DecisionController::isOwner() const
{
    return decision_ && decision_->isOwner(session_.userId());
}

int DecisionController::myExpertId() const
{
    if (!decision_)
        return -1;
    const ExpertRow* me = decision_->expertForUser(session_.userId());
    return me ? me->id : -1;
}

int DecisionController::completeCount() const
{
    if (!decision_)
        return 0;
    int n = 0;
    for (const ExpertRow& e : decision_->experts)
        if (decision_->isComplete(decision_->currentRound, e.id))
            ++n;
    return n;
}

QVariantList DecisionController::options() const
{
    QVariantList list;
    if (decision_)
        for (int i = 0; i < decision_->options.size(); ++i)
            list << QVariantMap{{"id", decision_->options[i].id}, {"name", decision_->options[i].name}, {"index", i}};
    return list;
}

QVariantList DecisionController::criteria() const
{
    QVariantList list;
    if (!decision_)
        return list;
    double total = 0.0;
    for (const auto& c : decision_->criteria)
        total += c.weight;
    for (const auto& c : decision_->criteria) {
        list << QVariantMap{{"id", c.id}, {"name", c.name}, {"weight", c.weight},
                            {"share", total > 0.0 ? c.weight / total : 0.0},
                            {"isCost", c.type == gd::CriterionType::Cost}};
    }
    return list;
}

QVariantList DecisionController::experts() const
{
    QVariantList list;
    if (!decision_)
        return list;
    double total = 0.0;
    for (const auto& e : decision_->experts)
        total += e.weight;
    const int round = decision_->currentRound;
    for (const auto& e : decision_->experts) {
        list << QVariantMap{{"id", e.id},
                            {"name", e.name},
                            {"email", e.email},
                            {"weight", e.weight},
                            {"share", total > 0.0 ? e.weight / total : 0.0},
                            {"hasAccount", e.userId.has_value()},
                            {"isMe", e.userId && *e.userId == session_.userId()},
                            {"isComplete", decision_->isComplete(round, e.id)},
                            {"filled", static_cast<int>(decision_->sheet(round, e.id).size())},
                            {"total", decision_->cellCount()}};
    }
    return list;
}

// ---------- Opening ----------

void DecisionController::open(int decisionId)
{
    try {
        decision_ = service_.get(decisionId, session_.userId());
    } catch (const AppError& e) {
        decision_.reset();
        emit notify(e.message(), QStringLiteral("error"));
    }
    ++revision_;
    emit changed();
}

void DecisionController::close()
{
    decision_.reset();
    ++revision_;
    emit changed();
    emit closed();
}

void DecisionController::reload()
{
    if (decision_)
        open(decision_->id);
}

void DecisionController::checkForOutsideChange()
{
    if (!decision_)
        return;
    const int version = service_.version(decision_->id);
    if (version == decision_->version)
        return;

    if (version < 0) {
        close();
        emit notify(tr("This decision was deleted by its organizer."), QStringLiteral("info"));
        return;
    }
    reload();
    if (decision_ && !decision_->lastChange.isEmpty())
        emit notify(decision_->lastChange + tr(" Results are updated."), QStringLiteral("info"));
}

// ---------- Results ----------

std::optional<gd::Method> DecisionController::parseMethod(const QString& method)
{
    if (method == QLatin1String("Topsis"))
        return gd::Method::Topsis;
    if (method == QLatin1String("WeightedSum"))
        return gd::Method::WeightedSum;
    if (method == QLatin1String("Borda"))
        return gd::Method::Borda;
    return std::nullopt;
}

QVariantMap DecisionController::failure(const QString& message) const
{
    return {{"ok", false}, {"error", message}};
}

QString DecisionController::cellKey(int optionId, int criterionId)
{
    return QStringLiteral("%1:%2").arg(optionId).arg(criterionId);
}

QVariantMap DecisionController::result(const QString& method) const
{
    const auto m = parseMethod(method);
    if (!decision_ || !m)
        return failure(tr("Nothing to show."));
    try {
        return rankingToMap(service_.result(decision_->id, session_.userId(), *m));
    } catch (const AppError& e) {
        return failure(e.message());
    }
}

QVariantMap DecisionController::compare() const
{
    if (!decision_)
        return failure(tr("Nothing to show."));
    try {
        QVariantList methods;
        for (const ResultView& r : service_.compare(decision_->id, session_.userId()))
            methods << rankingToMap(r);
        return {{"ok", true}, {"methods", methods}};
    } catch (const AppError& e) {
        return failure(e.message());
    }
}

QVariantList DecisionController::history() const
{
    QVariantList list;
    if (!decision_)
        return list;
    try {
        for (const RoundSummary& s : service_.history(decision_->id, session_.userId())) {
            list << QVariantMap{{"round", s.round}, {"consensus", optionalNumber(s.consensus)},
                                {"winners", s.winners}, {"expertCount", s.expertCount}, {"isCurrent", s.isCurrent}};
        }
    } catch (const AppError&) {
    }
    return list;
}

QVariantMap DecisionController::sensitivity(const QString& method) const
{
    const auto m = parseMethod(method);
    if (!decision_ || !m)
        return failure(tr("Nothing to show."));
    try {
        const SensitivityView s = service_.sensitivity(decision_->id, session_.userId(), *m);
        QVariantList criteria;
        for (const CriterionSensitivityView& c : s.criteria) {
            QVariantList segments;
            for (const SegmentView& g : c.segments) {
                segments << QVariantMap{{"from", g.from}, {"to", g.to},
                                        {"winnerOptionIds", toVariantList(g.winnerOptionIds)},
                                        {"winners", g.winners}};
            }
            criteria << QVariantMap{{"criterionId", c.criterionId}, {"criterion", c.criterion},
                                    {"currentShare", c.currentShare}, {"stableFrom", c.stableFrom},
                                    {"stableTo", c.stableTo}, {"segments", segments}};
        }
        return {{"ok", true},
                {"method", methodName(s.method)},
                {"currentWinnerIds", toVariantList(s.currentWinnerIds)},
                {"currentWinners", s.currentWinners},
                {"criteria", criteria},
                {"mostSensitiveCriterionId", s.mostSensitiveCriterionId ? QVariant(*s.mostSensitiveCriterionId) : QVariant()}};
    } catch (const AppError& e) {
        return failure(e.message());
    }
}

QVariantMap DecisionController::scores(int expertId) const
{
    if (!decision_)
        return failure(tr("Nothing to show."));
    try {
        const ScoreSheet sheet = service_.scores(decision_->id, session_.userId(), expertId);
        QVariantMap cells;
        for (auto it = sheet.cbegin(); it != sheet.cend(); ++it)
            cells.insert(cellKey(it.key().first, it.key().second), it.value());
        return {{"ok", true}, {"cells", cells}};
    } catch (const AppError& e) {
        return failure(e.message());
    }
}

QVariantMap DecisionController::feedback(int expertId) const
{
    if (!decision_ || decision_->currentRound < 2)
        return failure(QString());
    try {
        const FeedbackView f = service_.feedback(decision_->id, session_.userId(), expertId);
        QVariantMap cells;
        for (const CellFeedbackView& c : f.cells) {
            cells.insert(cellKey(c.optionId, c.criterionId),
                         QVariantMap{{"score", c.score}, {"group", c.groupScore}, {"deviation", c.deviation},
                                     {"suggested", c.suggestedScore}, {"flagged", c.flagged}});
        }
        return {{"ok", true}, {"expert", f.expert}, {"agreement", f.agreement},
                {"flaggedCount", f.flaggedCount}, {"cells", cells}};
    } catch (const AppError& e) {
        return failure(e.message());
    }
}

// ---------- Changes ----------

bool DecisionController::saveScores(int expertId, const QVariantMap& cells)
{
    if (!decision_)
        return false;

    QList<ScoreEntry> entries;
    for (auto it = cells.cbegin(); it != cells.cend(); ++it) {
        const QStringList ids = it.key().split(QLatin1Char(':'));
        bool okNumber = false;
        const double value = it.value().toDouble(&okNumber);
        if (ids.size() != 2 || !okNumber || !std::isfinite(value)) {
            emit notify(tr("Some scores are not numbers."), QStringLiteral("error"));
            return false;
        }
        entries.append({ids[0].toInt(), ids[1].toInt(), value});
    }

    try {
        service_.saveScores(decision_->id, session_.userId(), expertId, entries);
        reload();
        emit notify(tr("Scores saved."), QStringLiteral("success"));
        return true;
    } catch (const AppError& e) {
        emit notify(e.message(), QStringLiteral("error"));
        return false;
    }
}

bool DecisionController::startNextRound()
{
    if (!decision_)
        return false;
    try {
        const int round = service_.startNextRound(decision_->id, session_.userId());
        reload();
        emit listChanged();
        emit notify(tr("Round %1 has started. Experts can now revise their scores.").arg(round),
                    QStringLiteral("success"));
        return true;
    } catch (const AppError& e) {
        emit notify(e.message(), QStringLiteral("error"));
        return false;
    }
}

bool DecisionController::removeDecision()
{
    if (!decision_)
        return false;
    try {
        service_.remove(decision_->id, session_.userId());
        close();
        emit listChanged();
        emit notify(tr("Decision deleted."), QStringLiteral("info"));
        return true;
    } catch (const AppError& e) {
        emit notify(e.message(), QStringLiteral("error"));
        return false;
    }
}

int DecisionController::create(const QVariantMap& form)
{
    NewDecision input;
    input.title = form.value("title").toString();
    input.description = form.value("description").toString();
    input.consensusTarget = form.value("target", 0.8).toDouble();

    for (const QVariant& o : form.value("options").toList())
        input.options << o.toString();

    for (const QVariant& v : form.value("criteria").toList()) {
        const QVariantMap c = v.toMap();
        input.criteria.append({c.value("name").toString(), c.value("weight").toDouble(),
                               c.value("isCost").toBool() ? gd::CriterionType::Cost : gd::CriterionType::Benefit});
    }

    for (const QVariant& v : form.value("experts").toList()) {
        const QVariantMap e = v.toMap();
        input.experts.append({e.value("name").toString(), e.value("email").toString(), e.value("weight").toDouble()});
    }

    if (form.value("addMe").toBool()) {
        bool listed = false;
        for (const NewExpert& e : input.experts)
            listed = listed || e.email.trimmed().compare(session_.email(), Qt::CaseInsensitive) == 0;
        if (!listed)
            input.experts.prepend({session_.displayName(), session_.email(), 1.0});
    }

    try {
        const int id = service_.create(session_.userId(), input);
        emit listChanged();
        open(id);
        emit notify(tr("Decision created."), QStringLiteral("success"));
        return id;
    } catch (const AppError& e) {
        emit notify(e.message(), QStringLiteral("error"));
        return -1;
    }
}
