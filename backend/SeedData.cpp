#include "SeedData.h"

#include "AuthService.h"
#include "Database.h"
#include "DecisionStore.h"

#include <QSqlQuery>
#include <QVariant>

#include <array>
#include <vector>

namespace seed {

namespace {

using Sheet = std::array<std::array<double, 4>, 4>; // [option][criterion]

struct SeedExpert {
    QString name;
    QString email;
    double weight;
    std::vector<Sheet> rounds; // empty: invited, no account, no scores yet
};

} // namespace

bool addDemoData(const Database& db)
{
    if (db.exec(QStringLiteral("SELECT 1 FROM users LIMIT 1")).next())
        return false;

    AuthService auth(db);
    const User organizer = auth.registerUser(QStringLiteral("organizer@demo.local"), QStringLiteral("Anna Nowak"), kDemoPassword);
    auth.registerUser(QStringLiteral("head@demo.local"), QStringLiteral("Piotr Wiśniewski"), kDemoPassword);
    auth.registerUser(QStringLiteral("engineer@demo.local"), QStringLiteral("Kasia Zielińska"), kDemoPassword);
    auth.registerUser(QStringLiteral("finance@demo.local"), QStringLiteral("Marek Lewandowski"), kDemoPassword);

    // Columns: price (kPLN), quality (1-10), delivery (days), support (1-10).
    // Round 1: the experts disagree. Round 2: after feedback they moved closer.
    const std::vector<SeedExpert> experts = {
        {QStringLiteral("Head of lab"), QStringLiteral("head@demo.local"), 2.0,
         {Sheet{{{120, 8, 14, 7}, {95, 5, 21, 5}, {150, 10, 10, 9}, {110, 6, 12, 7}}},
          Sheet{{{120, 8, 14, 7}, {95, 6, 21, 6}, {150, 9, 10, 9}, {110, 7, 12, 8}}}}},
        {QStringLiteral("Engineer"), QStringLiteral("engineer@demo.local"), 1.0,
         {Sheet{{{120, 6, 14, 8}, {95, 8, 21, 7}, {150, 7, 10, 6}, {110, 9, 12, 9}}},
          Sheet{{{120, 7, 14, 8}, {95, 7, 21, 6}, {150, 8, 10, 7}, {110, 8, 12, 8}}}}},
        {QStringLiteral("Finance officer"), QStringLiteral("finance@demo.local"), 1.0,
         {Sheet{{{120, 6, 14, 5}, {95, 8, 21, 8}, {150, 6, 10, 5}, {110, 6, 12, 6}}},
          Sheet{{{120, 7, 14, 6}, {95, 8, 21, 7}, {150, 7, 10, 6}, {110, 7, 12, 7}}}}},
        {QStringLiteral("External advisor"), QStringLiteral("advisor@demo.local"), 1.0, {}},
    };

    NewDecision input;
    input.title = QStringLiteral("Selecting a supplier for lab equipment");
    input.description = QStringLiteral("Four suppliers, judged by the lab team on price, quality, delivery and support.");
    input.consensusTarget = 0.8;
    input.options = {QStringLiteral("Supplier A"), QStringLiteral("Supplier B"), QStringLiteral("Supplier C"),
                     QStringLiteral("Supplier D")};
    input.criteria = {
        {QStringLiteral("Price (kPLN)"), 0.35, gd::CriterionType::Cost},
        {QStringLiteral("Quality (1-10)"), 0.30, gd::CriterionType::Benefit},
        {QStringLiteral("Delivery time (days)"), 0.20, gd::CriterionType::Cost},
        {QStringLiteral("Support (1-10)"), 0.15, gd::CriterionType::Benefit},
    };
    for (const auto& e : experts)
        input.experts.append({e.name, e.email, e.weight});

    DecisionStore store(db);
    const int id = store.insert(input, organizer.id);
    const Decision decision = *store.load(id);

    for (int k = 0; k < decision.experts.size(); ++k) {
        const auto& rounds = experts[static_cast<std::size_t>(k)].rounds;
        for (std::size_t r = 0; r < rounds.size(); ++r) {
            QList<ScoreEntry> entries;
            for (int i = 0; i < decision.options.size(); ++i)
                for (int j = 0; j < decision.criteria.size(); ++j)
                    entries.append({decision.options[i].id, decision.criteria[j].id, rounds[r][i][j]});
            store.saveScores(decision.experts[k].id, static_cast<int>(r) + 1, entries);
        }
    }
    db.exec(QStringLiteral("UPDATE decisions SET current_round = 2 WHERE id = ?"), {id});
    return true;
}

} // namespace seed
