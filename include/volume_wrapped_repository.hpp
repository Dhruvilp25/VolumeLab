#pragma once
#include <string>
#include <vector>
#include "database.hpp"
#include "volume_wrapped.hpp"

// Author: Adrian Caricari

// Repository responsible for querying Volume Wrapped analytics data from the database.
class VolumeWrappedRepository {
public:
    // Initializes the repository with a database reference.
    explicit VolumeWrappedRepository(Database& db) : db_(db) {}

    // Returns total recorded training time in seconds for a user over a date range.
    long long getTotalTrainingSeconds(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive
    );

    // Returns the user's current and best workout streak stats.
    VolumeWrappedStreakStats getStreakStats(int userId);

    // Returns the top exercises ranked by total volume over a date range.
    std::vector<VolumeWrappedExerciseStat> getTopExercisesByVolume(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive,
        int limit = 5
    );

    // Returns the top exercises ranked by total sets over a date range.
    std::vector<VolumeWrappedExerciseStat> getTopExercisesBySets(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive,
        int limit = 5
    );

    // Retrieves the most frequently repeated exercise over a date range.
    bool getMostRepeatedExercise(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive,
        VolumeWrappedExerciseStat& out
    );

    // Retrieves the most frequently performed template day over a date range.
    bool getMostPerformedTemplateDay(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive,
        VolumeWrappedTemplateDayStat& out
    );

    // Retrieves the single workout session with the highest total volume over a date range.
    bool getBiggestVolumeDay(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive,
        VolumeWrappedVolumeDay& out
    );

private:
    // Shared helper used to build ranked exercise leaderboards with different sort orders.
    std::vector<VolumeWrappedExerciseStat> getExerciseLeaderboard(
        int userId,
        const std::string& startIsoInclusive,
        const std::string& endIsoExclusive,
        const std::string& orderBySql,
        int limit
    );

    // Stores the database reference used by the repository.
    Database& db_;
};