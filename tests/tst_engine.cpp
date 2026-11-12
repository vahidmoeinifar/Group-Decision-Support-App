// Unit tests for the decision engine (core/). No database, no UI.
// Reference values were checked against an independent NumPy implementation.

#include "groupdecision/ConsensusFeedback.h"
#include "groupdecision/Engine.h"
#include "groupdecision/Methods.h"
#include "groupdecision/Sensitivity.h"
#include "groupdecision/Statistics.h"

#include <QtTest>

#include <cmath>
#include <stdexcept>

using namespace gd;

namespace {

Matrix matrix(const std::vector<std::vector<double>>& rows)
{
    Matrix m(rows.size(), rows.front().size());
    for (std::size_t i = 0; i < rows.size(); ++i)
        for (std::size_t j = 0; j < rows[i].size(); ++j)
            m(i, j) = rows[i][j];
    return m;
}

const std::vector<std::string> kOptions = {"Supplier A", "Supplier B", "Supplier C", "Supplier D"};

const std::vector<Criterion> kCriteria = {
    {"Price", 0.35, CriterionType::Cost},
    {"Quality", 0.30, CriterionType::Benefit},
    {"Delivery", 0.20, CriterionType::Cost},
    {"Support", 0.15, CriterionType::Benefit},
};

// The example decision: three experts, round 1 (disagree) and round 2 (closer)
GroupProblem round1()
{
    return GroupProblem(kOptions, kCriteria, {
        {"Head of lab", 2, matrix({{120, 8, 14, 7}, {95, 5, 21, 5}, {150, 10, 10, 9}, {110, 6, 12, 7}})},
        {"Engineer", 1, matrix({{120, 6, 14, 8}, {95, 8, 21, 7}, {150, 7, 10, 6}, {110, 9, 12, 9}})},
        {"Finance officer", 1, matrix({{120, 6, 14, 5}, {95, 8, 21, 8}, {150, 6, 10, 5}, {110, 6, 12, 6}})},
    });
}

GroupProblem round2()
{
    return GroupProblem(kOptions, kCriteria, {
        {"Head of lab", 2, matrix({{120, 8, 14, 7}, {95, 6, 21, 6}, {150, 9, 10, 9}, {110, 7, 12, 8}})},
        {"Engineer", 1, matrix({{120, 7, 14, 8}, {95, 7, 21, 6}, {150, 8, 10, 7}, {110, 8, 12, 8}})},
        {"Finance officer", 1, matrix({{120, 7, 14, 6}, {95, 8, 21, 7}, {150, 7, 10, 6}, {110, 7, 12, 7}})},
    });
}

bool near(double a, double b, double tolerance = 1e-6) { return std::abs(a - b) <= tolerance; }

} // namespace

class EngineTests : public QObject {
    Q_OBJECT

private slots:
    // ---- Methods ----

    void topsisGivesOneToDominatingOption()
    {
        const auto s = methods::topsis(matrix({{10, 1}, {5, 2}}),
                                       {{"Quality", 1, CriterionType::Benefit}, {"Cost", 1, CriterionType::Cost}});
        QVERIFY(near(s[0], 1.0));
        QVERIFY(near(s[1], 0.0));
    }

    void topsisIdenticalOptionsAreTied()
    {
        const auto s = methods::topsis(matrix({{3, 3}, {3, 3}}),
                                       {{"A", 1, CriterionType::Benefit}, {"B", 1, CriterionType::Benefit}});
        QVERIFY(near(s[0], 0.5));
        QVERIFY(near(s[1], 0.5));
    }

    void weightedSumUsesMinMaxScaling()
    {
        // Benefit: 0 -> 0, 10 -> 1. Cost: 100 -> 1, 200 -> 0. Equal weights.
        const auto s = methods::weightedSum(matrix({{0, 100}, {10, 200}, {5, 150}}),
                                            {{"Quality", 1, CriterionType::Benefit}, {"Price", 1, CriterionType::Cost}});
        QVERIFY(near(s[0], 0.5));
        QVERIFY(near(s[1], 0.5));
        QVERIFY(near(s[2], 0.5));
    }

    void bordaSplitsPointsOnTies()
    {
        const auto p = methods::bordaPoints({3, 1, 3});
        QCOMPARE(p, (std::vector<double>{1.5, 0.0, 1.5}));
    }

    // ---- Statistics ----

    void averageRanksHandleTies()
    {
        QCOMPARE(stats::averageRanks({10, 20, 10, 30}), (std::vector<double>{1.5, 3, 1.5, 4}));
    }

    void spearmanPerfectAndReversed()
    {
        QVERIFY(near(stats::spearman({1, 2, 3, 4}, {10, 20, 30, 40}), 1.0));
        QVERIFY(near(stats::spearman({1, 2, 3, 4}, {40, 30, 20, 10}), -1.0));
        QVERIFY_THROWS_EXCEPTION(std::invalid_argument, stats::spearman({1, 2}, {1, 2, 3}));
    }

    // ---- Validation ----

    void problemRejectsBadInput()
    {
        const ExpertInput e{"E", 1, matrix({{1}, {2}})};
        QVERIFY_THROWS_EXCEPTION(std::invalid_argument, GroupProblem({"Only one"}, {{"C", 1, CriterionType::Benefit}}, {e}));
        QVERIFY_THROWS_EXCEPTION(std::invalid_argument, GroupProblem({"A", "B"}, {{"C", -1, CriterionType::Benefit}}, {e}));
        QVERIFY_THROWS_EXCEPTION(std::invalid_argument, GroupProblem({"A", "B"}, {{"C", 0, CriterionType::Benefit}}, {e}));
        QVERIFY_THROWS_EXCEPTION(std::invalid_argument, GroupProblem({"A", "B", "C"}, {{"C", 1, CriterionType::Benefit}}, {e}));
    }

    // ---- Group results (reference values) ----

    void topsisRound2MatchesReference()
    {
        const GroupResult r = solve(round2(), Method::Topsis);
        QCOMPARE(r.ranking[0].option, std::string("Supplier D"));
        QVERIFY(near(r.ranking[0].score, 0.725321));
        QVERIFY(near(r.ranking[1].score, 0.578944));
        QVERIFY(near(r.ranking[2].score, 0.505008));
        QVERIFY(near(r.ranking[3].score, 0.494992));
        QVERIFY(near(*r.consensus, 0.9));
        QCOMPARE(r.expertAgreement.size(), std::size_t(3));
        QVERIFY(near(r.expertAgreement[0], 1.0));
        QVERIFY(near(r.expertAgreement[1], 0.8));
    }

    void weightedSumRound2MatchesReference()
    {
        const GroupResult r = solve(round2(), Method::WeightedSum);
        QCOMPARE(r.ranking[0].option, std::string("Supplier D"));
        QVERIFY(near(r.ranking[0].score, 0.668182));
        QVERIFY(near(r.ranking[1].score, 0.65));
        QVERIFY(near(*r.consensus, 0.55));
    }

    void agreementRisesFromRound1ToRound2()
    {
        QVERIFY(near(*solve(round1(), Method::Topsis).consensus, 0.3));
        QVERIFY(near(*solve(round1(), Method::WeightedSum).consensus, 0.1));
        QVERIFY(near(*solve(round1(), Method::Borda).consensus, 0.316228));
        QVERIFY(near(*solve(round2(), Method::Borda).consensus, 0.948683));
    }

    void methodChangesWinnerInRound1()
    {
        QCOMPARE(winners(solve(round1(), Method::Topsis)), std::vector<int>{3});      // Supplier D
        QCOMPARE(winners(solve(round1(), Method::WeightedSum)), std::vector<int>{2}); // Supplier C
    }

    void equalScoresShareARank()
    {
        // Borda in round 1: A and C both get 1.5 points
        const GroupResult r = solve(round1(), Method::Borda);
        QCOMPARE(r.ranking[1].rank, 2);
        QCOMPARE(r.ranking[2].rank, 2);
        QCOMPARE(r.ranking[3].rank, 4);
    }

    void singleExpertHasNoConsensus()
    {
        const GroupProblem p({"A", "B"}, {{"C", 1, CriterionType::Benefit}}, {{"E", 1, matrix({{1}, {2}})}});
        QVERIFY(!solve(p, Method::Topsis).consensus.has_value());
    }

    void expertWeightsChangeTheAggregate()
    {
        const GroupProblem p({"A", "B"}, {{"C", 1, CriterionType::Benefit}},
                             {{"Heavy", 3, matrix({{10}, {0}})}, {"Light", 1, matrix({{0}, {10}})}});
        const Matrix m = aggregateMatrix(p);
        QVERIFY(near(m(0, 0), 7.5));
        QVERIFY(near(m(1, 0), 2.5));
    }

    // ---- Consensus feedback ----

    void feedbackFlagsScoresFarFromGroup()
    {
        const ExpertFeedback f = consensusFeedback(round2(), 2); // finance officer
        QCOMPARE(f.flaggedCount(), 6);
        QVERIFY(near(f.agreement, 0.8));
        for (const CellFeedback& c : f.cells)
            QVERIFY(near(c.suggestedScore, (c.score + c.groupScore) / 2.0)); // halfway
    }

    void feedbackRejectsBadArguments()
    {
        QVERIFY_THROWS_EXCEPTION(std::out_of_range, consensusFeedback(round2(), 5));
        QVERIFY_THROWS_EXCEPTION(std::invalid_argument, consensusFeedback(round2(), 0, 1.5));
    }

    // ---- Sensitivity ----

    void withShareKeepsOtherProportions()
    {
        const GroupProblem p = withShare(round2(), 0, 0.5);
        const auto& c = p.criteria();
        QVERIFY(near(c[0].weight, 0.5));
        QVERIFY(near(c[1].weight / c[2].weight, 0.30 / 0.20)); // ratio kept
        QVERIFY(near(c[1].weight + c[2].weight + c[3].weight, 0.5));
    }

    void sensitivityFindsStableRanges()
    {
        const SensitivityResult s = analyzeSensitivity(round2(), Method::Topsis);
        QCOMPARE(s.currentWinners, std::vector<int>{3});
        QCOMPARE(s.criteria.size(), std::size_t(4));

        QVERIFY(near(s.criteria[0].stableFrom, 0.1731, 1e-3)); // price
        QVERIFY(near(s.criteria[0].stableTo, 0.5937, 1e-3));
        QVERIFY(near(s.criteria[1].stableFrom, 0.0));           // quality
        QVERIFY(near(s.criteria[1].stableTo, 0.55, 1e-3));
        QVERIFY(near(s.criteria[3].stableFrom, 0.0));           // support never changes the winner
        QVERIFY(near(s.criteria[3].stableTo, 1.0));

        QVERIFY(s.mostSensitiveCriterion.has_value());
        QCOMPARE(*s.mostSensitiveCriterion, 2);                 // delivery time
    }

    void sensitivitySegmentsCoverWholeRange()
    {
        const SensitivityResult s = analyzeSensitivity(round2(), Method::WeightedSum);
        for (const CriterionSensitivity& c : s.criteria) {
            QVERIFY(near(c.segments.front().from, 0.0));
            QVERIFY(near(c.segments.back().to, 1.0));
            for (std::size_t k = 1; k < c.segments.size(); ++k)
                QVERIFY(near(c.segments[k].from, c.segments[k - 1].to));
        }
    }
};

QTEST_GUILESS_MAIN(EngineTests)
#include "tst_engine.moc"
