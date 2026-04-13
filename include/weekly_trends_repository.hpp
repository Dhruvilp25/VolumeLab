#pragma once
#include <vector>
#include <string>
#include "database.hpp"
#include "weekly_trends.hpp"

/**
 * @brief Repository that queries the database for per-week workout trend data.
 *
 * Groups workout sessions by ISO week (Monday start) and computes weekly
 * totals for workouts, sets, reps, and volume. Used by WeeklyTrendsService
 * and AnalyticsDashboardService to populate trend charts.
 *
 * @author Adrian Caricari
 */
class WeeklyTrendsRepository {
public:
    /**
     * @brief Constructs the repository with a reference to an open database.
     * @param db Reference to the Database object used for queries.
     */
    explicit WeeklyTrendsRepository(Database& db) : db_(db) {}

    /**
     * @brief Retrieves weekly workout trend rows for a user within a date range.
     *
     * Returns one WeeklyTrendRow per week that contains at least one workout.
     * Rows are sorted in ascending order by week start date.
     *
     * @param userId             The ID of the user.
     * @param startIsoInclusive  ISO datetime marking the start of the range (inclusive).
     * @param endIsoExclusive    ISO datetime marking the end of the range (exclusive).
     * @return Vector of WeeklyTrendRow sorted by weekStart ascending.
     */
    std::vector<WeeklyTrendRow> getWeeklyTrends(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive
    );

private:
    Database& db_;  ///< Reference to the database connection.
};
