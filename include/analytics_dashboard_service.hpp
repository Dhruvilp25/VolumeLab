#pragma once
#include <string>
#include "analytics_dashboard.hpp"
#include "analytics_dashboard_repository.hpp"
#include "weekly_trends_repository.hpp"

/**
 * @brief Service layer that assembles a complete AnalyticsDashboard for the UI.
 *
 * Coordinates the AnalyticsDashboardRepository (for aggregate totals) and the
 * WeeklyTrendsRepository (for per-week breakdowns) to build a single dashboard
 * object. Supports both preset date ranges and fully custom ranges.
 *
 * @author Adrian Caricari
 */
class AnalyticsDashboardService {
public:
    /**
     * @brief Constructs the service with its required repositories and database.
     * @param totalsRepo  Repository for aggregate range totals.
     * @param weeklyRepo  Repository for weekly trend rows.
     * @param db          Database connection used for date arithmetic queries.
     */
    AnalyticsDashboardService(
        AnalyticsDashboardRepository& totalsRepo,
        WeeklyTrendsRepository& weeklyRepo,
        Database& db
    ) : totalsRepo_(totalsRepo), weeklyRepo_(weeklyRepo), db_(db) {}

    /**
     * @brief Builds a dashboard using a preset date range relative to a given end date.
     *
     * Converts the preset into a concrete start date using SQLite date arithmetic,
     * then delegates to getDashboardCustom.
     *
     * @param userId           The ID of the user.
     * @param preset           One of W1, M1, M3, or Y1 (must not be Custom).
     * @param endIsoExclusive  ISO datetime for the exclusive end of the range.
     * @return A fully populated AnalyticsDashboard.
     * @throws std::runtime_error If preset is Custom.
     */
    AnalyticsDashboard getDashboard(
        int userId,
        RangePreset preset,
        const std::string& endIsoExclusive
    );

    /**
     * @brief Builds a dashboard for a user using an explicitly provided date range.
     *
     * Fetches aggregate totals and, if workouts exist, weekly trend data for the
     * specified range.
     *
     * @param userId             The ID of the user.
     * @param startIsoInclusive  ISO datetime for the inclusive start of the range.
     * @param endIsoExclusive    ISO datetime for the exclusive end of the range.
     * @return A fully populated AnalyticsDashboard.
     */
    AnalyticsDashboard getDashboardCustom(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive
    );

private:
    /**
     * @brief Converts a RangePreset into a concrete start datetime string.
     * @param preset           The preset to convert (must not be Custom).
     * @param endIsoExclusive  The reference end date to shift backwards from.
     * @return ISO datetime string for the computed start date.
     */
    std::string computeStartFromPreset(RangePreset preset, const std::string& endIsoExclusive);

    /**
     * @brief Uses SQLite datetime() to shift a timestamp by a given modifier.
     * @param endIsoExclusive  The base datetime string.
     * @param modifier         SQLite modifier string (e.g. "-7 days", "-1 month").
     * @return The shifted datetime as a string.
     */
    std::string sqliteDatetimeShift(const std::string& endIsoExclusive, const std::string& modifier);

    AnalyticsDashboardRepository& totalsRepo_;  ///< Repository for aggregate totals.
    WeeklyTrendsRepository& weeklyRepo_;        ///< Repository for weekly trends.
    Database& db_;                               ///< Database connection for date queries.
};
