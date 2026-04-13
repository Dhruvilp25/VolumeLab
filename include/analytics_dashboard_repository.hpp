#pragma once
#include <string>
#include "database.hpp"

/**
 * @brief Holds aggregate workout totals for a user over a chosen time range.
 *
 * Stores the count of workouts, sets, reps, and total volume returned by
 * a single summary query against the database.
 *
 * @author Adrian Caricari
 */
struct RangeTotals {
    int workoutCount = 0;        ///< Number of distinct workout sessions.
    int totalSets = 0;           ///< Total number of sets performed.
    long long totalReps = 0;     ///< Total number of reps performed.
    double totalVolume = 0.0;    ///< Total volume (reps x weight).
};

/**
 * @brief Repository that queries the database for analytics dashboard totals.
 *
 * Encapsulates the SQL logic needed to compute aggregate workout statistics
 * for a given user and date range. The AnalyticsDashboardService calls this
 * repository and then passes the results to the UI layer.
 *
 * @author Adrian Caricari
 */
class AnalyticsDashboardRepository {
public:
    /**
     * @brief Constructs the repository with a reference to an open database.
     * @param db Reference to the Database object used for queries.
     */
    explicit AnalyticsDashboardRepository(Database& db) : db_(db) {}

    /**
     * @brief Retrieves aggregate workout totals for a user within a date range.
     *
     * Executes a single SQL query that counts distinct workouts, total sets,
     * total reps, and total volume (reps x weight) for the specified user
     * between the given start and end timestamps.
     *
     * @param userId             The ID of the user whose totals are requested.
     * @param startIsoInclusive  ISO datetime marking the start of the range (inclusive).
     * @param endIsoExclusive    ISO datetime marking the end of the range (exclusive).
     * @return A RangeTotals struct populated with the query results.
     */
    RangeTotals getRangeTotals(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive
    );

private:
    Database& db_;  ///< Reference to the database connection.
};
