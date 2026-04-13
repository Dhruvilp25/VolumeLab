#include "volume_wrapped_service.hpp"
#include <stdexcept>

// Author: Adrian Caricari

// Computes how many calendar weeks are touched by the given date range
int VolumeWrappedService::computeCalendarWeeksInRange(
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive
) {
    // SQL query to to count the weeks between them
    const std::string sql = R"SQL(
        WITH bounds AS (
            SELECT
                date(?, printf('-%d days', ((CAST(strftime('%w', ?) AS INTEGER) + 6) % 7))) AS start_week,
                date(datetime(?, '-1 second'), printf('-%d days', ((CAST(strftime('%w', datetime(?, '-1 second')) AS INTEGER) + 6) % 7))) AS end_week
        )
        SELECT CAST(((julianday(end_week) - julianday(start_week)) / 7) AS INTEGER) + 1
        FROM bounds;
    )SQL";

    // Prepares the SQL statement 
    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        // Binds the range boundary values to the query.
        Database::bindText(stmt, 1, startIsoInclusive);
        Database::bindText(stmt, 2, startIsoInclusive);
        Database::bindText(stmt, 3, endIsoExclusive);
        Database::bindText(stmt, 4, endIsoExclusive);

        // Execute the query 
        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            throw std::runtime_error("computeCalendarWeeksInRange: sqlite3_step failed");
        }

        // Read the computed week count and guarantees a minimum of one week
        int weeks = Database::colInt(stmt, 0);
        db_.finalize(stmt);
        return weeks > 0 ? weeks : 1;
    } catch (...) {
        // Ensures the statement is finalized if an error happens
        db_.finalize(stmt);
        throw;
    }
}

// Builds a complete Volume Wrapped report for a user over a given date range.
VolumeWrappedReport VolumeWrappedService::getVolumeWrapped(
    int userId,
    const std::string& startIsoInclusive,
    const std::string& endIsoExclusive
) {
    // Validate that the date range
    if (endIsoExclusive <= startIsoInclusive) {
        throw std::runtime_error("getVolumeWrapped: endIsoExclusive must be after startIsoInclusive");
    }

    // Loads the shared dashboard totals and weekly trend data for the range.
    const AnalyticsDashboard dash =
        dashboardService_.getDashboardCustom(userId, startIsoInclusive, endIsoExclusive);

    // Initializes the report and copies basic dashboard totals into it.
    VolumeWrappedReport report;
    report.startIsoInclusive = startIsoInclusive;
    report.endIsoExclusive = endIsoExclusive;
    report.hasWorkouts = dash.hasWorkouts;

    report.workoutsCompleted = dash.workoutCount;
    report.totalSets = dash.totalSets;
    report.totalReps = dash.totalReps;
    report.totalVolume = dash.totalVolume;
    report.weeklyTrends = dash.weeklyTrends;

    // Loads streak and training time summary data from the repository.
    report.streaks = repo_.getStreakStats(userId);

    report.totalTrainingSeconds =
        repo_.getTotalTrainingSeconds(userId, startIsoInclusive, endIsoExclusive);
    report.hasTrainingTimeData = (report.totalTrainingSeconds > 0);

    // Computes report level weekly summary metrics.
    report.totalActiveWeeks = static_cast<int>(report.weeklyTrends.size());
    report.totalCalendarWeeks = computeCalendarWeeksInRange(startIsoInclusive, endIsoExclusive);
    report.averageWorkoutsPerWeek =
        (report.totalCalendarWeeks > 0)
            ? static_cast<double>(report.workoutsCompleted) / report.totalCalendarWeeks
            : 0.0;

    // Returns early when the range contains no workouts
    if (!report.hasWorkouts) {
        return report;
    }

    // Loads leaderboard exercise summaries for the report
    report.top5ExercisesByVolume =
        repo_.getTopExercisesByVolume(userId, startIsoInclusive, endIsoExclusive, 5);
    report.top5ExercisesBySets =
        repo_.getTopExercisesBySets(userId, startIsoInclusive, endIsoExclusive, 5);

    // Loads single highlight stats for most repeated exercise, most performed day, and biggest volume day.
    report.hasMostRepeatedExercise =
        repo_.getMostRepeatedExercise(userId, startIsoInclusive, endIsoExclusive, report.mostRepeatedExercise);

    report.hasMostPerformedTemplateDay =
        repo_.getMostPerformedTemplateDay(userId, startIsoInclusive, endIsoExclusive, report.mostPerformedTemplateDay);

    report.hasBiggestVolumeDay =
        repo_.getBiggestVolumeDay(userId, startIsoInclusive, endIsoExclusive, report.biggestVolumeDay);

    // Returns the fully populated Volume Wrapped report.
    return report;
}