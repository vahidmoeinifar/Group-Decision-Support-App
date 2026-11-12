#pragma once

class Database;

/// Demo accounts and one example decision with two consensus rounds.
/// Runs only when the database has no users yet.
namespace seed {

constexpr const char* kDemoPassword = "Demo123!";

/// Returns true when demo data was added.
bool addDemoData(const Database& db);

} // namespace seed
