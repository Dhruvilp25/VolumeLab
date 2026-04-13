#include "analytics_dashboard_service.hpp"
#include <stdexcept>

/**
 * @brief Uses SQLite datetime() to shift a timestamp by a given modifier.
 *
 * Executes a single-row query "SELECT datetime(?, ?)" to leverage SQLite's
 * built-in date arithmetic and returns the resulting datetime string.
 *
 * @param endIsoExclusive  The base datetime string to shift.
 * @param modifier         SQLite modifier (e.g. "-7 days", "-1 month").
 * @return The shifted datetime as an ISO string.
 * @throws std::runtime_error If the query returns no row.
 *
 * @author Adrian Caricari
 */
std::string AnalyticsDashboardService::sqliteDatetimeShift(
    const std::string& endIsoExclusive,
    const std::string& modifier
) {
    // SQL query to apply SQLite datetime arithmetic to the input timestamp.
    const std::string sql = "SELECT datetime(?, ?);";
    sqlite3_stmt* stmt = db_.prepare(sql);

    try {
        // Binds the datetime value and shift modifier to the SQL query.
        Database::bindText(stmt, 1, endIsoExclusive);
        Database::bindText(stmt, 2, modifier);

        // Execute the query
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            throw std::runtime_error("sqliteDatetimeShift: datetime() returned no row");
        }

        // Reads the datetime result and returns it.
        std::string out = Database::colText(stmt, 0);
        db_.finalize(stmt);
        return out;
    } catch (...) {
        // Finalizes the statement before rethrowing any error.
        db_.finalize(stmt);
        throw;
    }
}

/**
 * @brief Converts a RangePreset into its corresponding start datetime.
 *
 * Maps each preset enum value to a SQLite date modifier string and delegates
 * to sqliteDatetimeShift to compute the actual start date.
 *
 * @param preset           The preset to convert (must not be Custom).
 * @param endIsoExclusive  The reference end date to shift backwards from.
 * @return ISO datetime string for the computed start date.
 * @throws std::runtime_error If preset is Custom.
 *
 * @author Adrian Caricari
 */
std::string AnalyticsDashboardService::computeStartFromPreset(
    RangePreset preset,
    const std::string& endIsoExclusive
) {
    // Selects the correct SQLite date shift based on the chosen preset.
    switch (preset) {
        case RangePreset::W1: return sqliteDatetimeShift(endIsoExclusive, "-7 days");
        case RangePreset::M1: return sqliteDatetimeShift(endIsoExclusive, "-1 month");
        case RangePreset::M3: return sqliteDatetimeShift(endIsoExclusive, "-3 months");
        case RangePreset::Y1: return sqliteDatetimeShift(endIsoExclusive, "-1 year");
        default:
            throw std::runtime_error("computeStartFromPreset: preset must not be Custom");
    }
}

/**
 * @brief Builds a dashboard using a preset date range relative to a given end date.
 *
 * Converts the preset into a concrete start date via computeStartFromPreset,
 * then delegates to getDashboardCustom to assemble the full dashboard.
 *
 * @param userId           The ID of the user.
 * @param preset           One of W1, M1, M3, or Y1 (must not be Custom).
 * @param endIsoExclusive  ISO datetime for the exclusive end of the range.
 * @return A fully populated AnalyticsDashboard.
 * @throws std::runtime_error If preset is Custom.
 *
 * @author Adrian Caricari
 */
AnalyticsDashboard AnalyticsDashboardService::getDashboard(
    int userId,
    RangePreset preset,
    const std::string& endIsoExclusive
) {
    if (preset == RangePreset::Custom) {
        throw std::runtime_error("getDashboard: use getDashboardCustom for Custom range");
    }

    // Computes the preset start date and forwards to the shared dashboard builder.
    const std::string startIsoInclusive = computeStartFromPreset(preset, endIsoExclusive);
    return getDashboardCustom(userId, startIsoInclusive, endIsoExclusive);
}

/**
 * @brief Builds a dashboard for a user using an explicitly provided date range.
 *
 * Fetches aggregate totals from the totals repository and, if workouts exist
 * in the range, also loads weekly trend data from the weekly repository.
 *
 * @param userId             The ID of the user.
 * @param startIsoInclusive  ISO datetime for the inclusive start of the range.
 * @param endIsoExclusive    ISO datetime for the exclusive end of the range.
 * @return A fully populated AnalyticsDashboard.
 *
 * @author Adrian Caricari
 */
AnalyticsDashboard AnalyticsDashboardService::getDashboardCustom(
    int userId,
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive
) {
    // Initializes the dashboard object with the requested date range.
    AnalyticsDashboard dash;
    dash.startIsoInclusive = startIsoInclusive;
    dash.endIsoExclusive = endIsoExclusive;

    // Retrieves aggregate totals for the range.
    RangeTotals totals = totalsRepo_.getRangeTotals(userId, startIsoInclusive, endIsoExclusive);

    // Copies the returned totals into the dashboard fields.
    dash.workoutCount = totals.workoutCount;
    dash.totalSets    = totals.totalSets;
    dash.totalReps    = totals.totalReps;
    dash.totalVolume  = totals.totalVolume;

    // Marks whether the dashboard range contains any workouts.
    dash.hasWorkouts = (dash.workoutCount > 0);

    // Loads weekly trend data only when workout data exists in the range.
    if (dash.hasWorkouts) {
        dash.weeklyTrends = weeklyRepo_.getWeeklyTrends(userId, startIsoInclusive, endIsoExclusive);
    }

    // Returns the fully populated dashboard object.
    return dash;
}
