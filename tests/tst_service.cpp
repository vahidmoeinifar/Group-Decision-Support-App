// Tests for accounts, storage and app rules (backend/), using a fresh in-memory SQLite database per test.

#include "AppError.h"
#include "AuthService.h"
#include "Database.h"
#include "DecisionService.h"
#include "PasswordHasher.h"
#include "SeedData.h"

#include <QtTest>

#include <functional>
#include <memory>
#include <optional>

namespace {

/// Runs f and returns the kind of AppError it threw, or nothing if it did not throw.
std::optional<AppError::Kind> errorOf(const std::function<void()>& f)
{
    try {
        f();
    } catch (const AppError& e) {
        return e.kind();
    }
    return std::nullopt;
}

QList<ScoreEntry> fullSheet(const Decision& d, const QList<double>& values)
{
    QList<ScoreEntry> list;
    int k = 0;
    for (const auto& o : d.options)
        for (const auto& c : d.criteria)
            list.append({o.id, c.id, values.at(k++ % values.size())});
    return list;
}

} // namespace

class ServiceTests : public QObject {
    Q_OBJECT

private:
    std::unique_ptr<Database> db;
    std::unique_ptr<AuthService> auth;
    std::unique_ptr<DecisionService> service;
    int counter = 0;

    User demo(const QString& email) { return auth->signIn(email, QString::fromLatin1(seed::kDemoPassword)); }
    int demoDecisionId() { return service->list(demo("organizer@demo.local").id).first().id; }

private slots:
    void init()
    {
        // A new connection name each time, so every test starts from an empty database
        db = std::make_unique<Database>(QStringLiteral(":memory:"), QStringLiteral("test%1").arg(++counter));
        seed::addDemoData(*db);
        auth = std::make_unique<AuthService>(*db);
        service = std::make_unique<DecisionService>(*db);
    }

    void cleanup()
    {
        service.reset();
        auth.reset();
        db.reset();
    }

    // ---- Accounts ----

    void passwordsAreHashedAndSalted()
    {
        const QString a = passwords::hash(QStringLiteral("secret123"), 1000);
        const QString b = passwords::hash(QStringLiteral("secret123"), 1000);
        QVERIFY(!a.contains(QStringLiteral("secret123")));
        QVERIFY(a != b); // different salt each time
        QVERIFY(passwords::verify(QStringLiteral("secret123"), a));
        QVERIFY(!passwords::verify(QStringLiteral("secret124"), a));
        QVERIFY(!passwords::verify(QStringLiteral("secret123"), QStringLiteral("garbage")));
    }

    void wrongPasswordIsRejected()
    {
        QCOMPARE(errorOf([&] { auth->signIn(QStringLiteral("head@demo.local"), QStringLiteral("wrong-password")); }),
                 AppError::Kind::Auth);
        QCOMPARE(errorOf([&] { auth->signIn(QStringLiteral("nobody@demo.local"), QStringLiteral("Demo123!")); }),
                 AppError::Kind::Auth);
    }

    void emailIsCaseInsensitive()
    {
        QCOMPARE(auth->signIn(QStringLiteral("  HEAD@Demo.Local "), QStringLiteral("Demo123!")).email,
                 QStringLiteral("head@demo.local"));
    }

    void registrationIsValidated()
    {
        QCOMPARE(errorOf([&] { auth->registerUser(QStringLiteral("head@demo.local"), QStringLiteral("X"), QStringLiteral("password1")); }),
                 AppError::Kind::Conflict);
        QCOMPARE(errorOf([&] { auth->registerUser(QStringLiteral("not-an-email"), QStringLiteral("X"), QStringLiteral("password1")); }),
                 AppError::Kind::Validation);
        QCOMPARE(errorOf([&] { auth->registerUser(QStringLiteral("new@demo.local"), QStringLiteral("X"), QStringLiteral("short")); }),
                 AppError::Kind::Validation);
    }

    void lateRegistrationLinksInvitation()
    {
        // advisor@demo.local was invited before having an account
        const User advisor = auth->registerUser(QStringLiteral("advisor@demo.local"), QStringLiteral("Ola"), QStringLiteral("password1"));
        const QList<DecisionSummary> list = service->list(advisor.id);
        QCOMPARE(list.size(), 1);
        QVERIFY(list.first().isExpert);
        QVERIFY(!list.first().isOwner);
    }

    // ---- Who can see and do what ----

    void outsiderGetsNotFound()
    {
        const User outsider = auth->registerUser(QStringLiteral("out@demo.local"), QStringLiteral("Out"), QStringLiteral("password1"));
        QVERIFY(service->list(outsider.id).isEmpty());
        const int id = demoDecisionId();
        QCOMPARE(errorOf([&] { service->get(id, outsider.id); }), AppError::Kind::NotFound);
        QCOMPARE(errorOf([&] { service->result(id, outsider.id, gd::Method::Topsis); }), AppError::Kind::NotFound);
    }

    void expertsScoreOnlyForThemselves()
    {
        const int id = demoDecisionId();
        const User head = demo(QStringLiteral("head@demo.local"));
        const Decision d = service->get(id, head.id);
        const int engineerId = d.experts[1].id;

        QCOMPARE(errorOf([&] { service->saveScores(id, head.id, engineerId, fullSheet(d, {5})); }),
                 AppError::Kind::Forbidden);
        QCOMPARE(errorOf([&] { service->scores(id, head.id, engineerId); }), AppError::Kind::Forbidden);
    }

    void organizerReadsButDoesNotWriteScores()
    {
        const int id = demoDecisionId();
        const User organizer = demo(QStringLiteral("organizer@demo.local"));
        const Decision d = service->get(id, organizer.id);

        QCOMPARE(service->scores(id, organizer.id, d.experts[0].id).size(), d.cellCount());
        QCOMPARE(errorOf([&] { service->saveScores(id, organizer.id, d.experts[0].id, fullSheet(d, {5})); }),
                 AppError::Kind::Forbidden);
    }

    void scoresMustBelongToDecision()
    {
        const int id = demoDecisionId();
        const User head = demo(QStringLiteral("head@demo.local"));
        const Decision d = service->get(id, head.id);
        QCOMPARE(errorOf([&] { service->saveScores(id, head.id, d.experts[0].id, {{9999, d.criteria[0].id, 1}}); }),
                 AppError::Kind::Validation);
        QCOMPARE(errorOf([&] { service->saveScores(id, head.id, d.experts[0].id, {{d.options[0].id, d.criteria[0].id, qQNaN()}}); }),
                 AppError::Kind::Validation);
    }

    void onlyOrganizerDeletes()
    {
        const int id = demoDecisionId();
        QCOMPARE(errorOf([&] { service->remove(id, demo(QStringLiteral("head@demo.local")).id); }), AppError::Kind::Forbidden);
        service->remove(id, demo(QStringLiteral("organizer@demo.local")).id);
        QVERIFY(service->list(demo(QStringLiteral("organizer@demo.local")).id).isEmpty());
        QCOMPARE(service->version(id), -1);
    }

    // ---- Results ----

    void resultsSkipIncompleteExperts()
    {
        const int id = demoDecisionId();
        const ResultView r = service->result(id, demo(QStringLiteral("finance@demo.local")).id, gd::Method::Topsis);
        QCOMPARE(r.round, 2);
        QCOMPARE(r.ranking.first().option, QStringLiteral("Supplier D"));
        QCOMPARE(r.experts.size(), 3);
        QCOMPARE(r.skipped, QStringList{QStringLiteral("External advisor")});
        QVERIFY(qAbs(*r.consensus - 0.9) < 1e-6);
    }

    void historyShowsEveryRound()
    {
        const QList<RoundSummary> h = service->history(demoDecisionId(), demo(QStringLiteral("organizer@demo.local")).id);
        QCOMPARE(h.size(), 2);
        QVERIFY(qAbs(*h[0].consensus - 0.3) < 1e-6);
        QVERIFY(qAbs(*h[1].consensus - 0.9) < 1e-6);
        QVERIFY(h[1].isCurrent);
    }

    void unknownRoundIsRejected()
    {
        const User organizer = demo(QStringLiteral("organizer@demo.local"));
        QCOMPARE(errorOf([&] { service->result(demoDecisionId(), organizer.id, gd::Method::Topsis, 7); }),
                 AppError::Kind::Validation);
    }

    void sensitivityNamesMostSensitiveCriterion()
    {
        const int id = demoDecisionId();
        const SensitivityView s = service->sensitivity(id, demo(QStringLiteral("organizer@demo.local")).id, gd::Method::Topsis);
        QCOMPARE(s.currentWinners, QStringList{QStringLiteral("Supplier D")});
        QVERIFY(s.mostSensitiveCriterionId.has_value());
        const Decision d = service->get(id, demo(QStringLiteral("organizer@demo.local")).id);
        QCOMPARE(*s.mostSensitiveCriterionId, d.criteria[2].id); // delivery time
    }

    void feedbackOnlyForOwnScoresOrOrganizer()
    {
        const int id = demoDecisionId();
        const User finance = demo(QStringLiteral("finance@demo.local"));
        const Decision d = service->get(id, finance.id);
        const FeedbackView f = service->feedback(id, finance.id, d.experts[2].id);
        QCOMPARE(f.flaggedCount, 6);
        QCOMPARE(errorOf([&] { service->feedback(id, finance.id, d.experts[0].id); }), AppError::Kind::Forbidden);
        QCOMPARE(service->feedback(id, demo(QStringLiteral("organizer@demo.local")).id, d.experts[0].id).expertId,
                 d.experts[0].id);
    }

    // ---- Rounds ----

    void startingRoundCopiesScores()
    {
        const int id = demoDecisionId();
        const User organizer = demo(QStringLiteral("organizer@demo.local"));
        QCOMPARE(errorOf([&] { service->startNextRound(id, demo(QStringLiteral("head@demo.local")).id); }),
                 AppError::Kind::Forbidden);

        const int versionBefore = service->version(id);
        QCOMPARE(service->startNextRound(id, organizer.id), 3);
        QVERIFY(service->version(id) > versionBefore);

        const Decision d = service->get(id, organizer.id);
        QCOMPARE(d.currentRound, 3);
        QCOMPARE(d.sheet(3, d.experts[0].id), d.sheet(2, d.experts[0].id));
        // Same scores, so the same agreement
        QVERIFY(qAbs(*service->result(id, organizer.id, gd::Method::Topsis).consensus - 0.9) < 1e-6);
    }

    // ---- A whole decision from start to finish ----

    void fullConsensusFlow()
    {
        const User boss = auth->registerUser(QStringLiteral("boss@x.pl"), QStringLiteral("Boss"), QStringLiteral("password1"));
        const User ala = auth->registerUser(QStringLiteral("ala@x.pl"), QStringLiteral("Ala"), QStringLiteral("password1"));

        NewDecision input;
        input.title = QStringLiteral("Team laptop");
        input.options = {QStringLiteral("Laptop A"), QStringLiteral("Laptop B")};
        input.criteria = {{QStringLiteral("Price"), 1, gd::CriterionType::Cost},
                          {QStringLiteral("Speed"), 1, gd::CriterionType::Benefit}};
        input.experts = {{QStringLiteral("Ala"), QStringLiteral("ala@x.pl"), 1},
                         {QStringLiteral("Olek"), QStringLiteral("olek@x.pl"), 1}};
        const int id = service->create(boss.id, input);

        // Nobody has scored yet
        QCOMPARE(errorOf([&] { service->result(id, boss.id, gd::Method::Topsis); }), AppError::Kind::Validation);

        Decision d = service->get(id, ala.id);
        service->saveScores(id, ala.id, d.experts[0].id, fullSheet(d, {1000, 5, 1500, 9}));
        QVERIFY(!service->result(id, boss.id, gd::Method::Topsis).consensus.has_value()); // one expert only
        QCOMPARE(errorOf([&] { service->startNextRound(id, boss.id); }), AppError::Kind::Validation);

        // Olek registers later and is linked to his invitation
        const User olek = auth->registerUser(QStringLiteral("olek@x.pl"), QStringLiteral("Olek"), QStringLiteral("password1"));
        d = service->get(id, olek.id);
        service->saveScores(id, olek.id, d.experts[1].id, fullSheet(d, {1000, 9, 1500, 5}));

        const ResultView r1 = service->result(id, boss.id, gd::Method::Topsis);
        QVERIFY(*r1.consensus < 1.0); // they rank the laptops differently

        QCOMPARE(service->startNextRound(id, boss.id), 2);
        service->saveScores(id, olek.id, d.experts[1].id, fullSheet(d, {1000, 5, 1500, 9})); // Olek agrees now
        QVERIFY(qAbs(*service->result(id, boss.id, gd::Method::Topsis).consensus - 1.0) < 1e-9);
        QCOMPARE(service->history(id, boss.id).size(), 2);
    }

    void createIsValidated()
    {
        const User boss = auth->registerUser(QStringLiteral("boss@x.pl"), QStringLiteral("Boss"), QStringLiteral("password1"));
        NewDecision input;
        input.title = QStringLiteral("T");
        input.options = {QStringLiteral("A"), QStringLiteral("a")}; // duplicate (case-insensitive)
        input.criteria = {{QStringLiteral("C"), 1, gd::CriterionType::Benefit}};
        input.experts = {{QStringLiteral("E"), QStringLiteral("e@x.pl"), 1}};
        QCOMPARE(errorOf([&] { service->create(boss.id, input); }), AppError::Kind::Validation);

        input.options = {QStringLiteral("A"), QStringLiteral("B")};
        input.criteria = {{QStringLiteral("C"), 0, gd::CriterionType::Benefit}}; // no positive weight
        QCOMPARE(errorOf([&] { service->create(boss.id, input); }), AppError::Kind::Validation);

        input.criteria = {{QStringLiteral("C"), 1, gd::CriterionType::Benefit}};
        input.experts = {{QStringLiteral("E"), QStringLiteral("bad-email"), 1}};
        QCOMPARE(errorOf([&] { service->create(boss.id, input); }), AppError::Kind::Validation);
    }
};

QTEST_GUILESS_MAIN(ServiceTests)
#include "tst_service.moc"
