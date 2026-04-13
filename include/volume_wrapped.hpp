#pragma once
#include <string>
#include <vector>
#include "weekly_trends.hpp"

// Author: Adrian Caricari

// Aggregate stats for one exercise in the selected range.
struct VolumeWrappedExerciseStat {
    int exerciseId = 0;
    std::string exerciseName;
    int sessionCount = 0;
    int totalSets = 0;
    long long totalReps = 0;
    double totalVolume = 0.0;
};

// Summary for the most performed template day.
struct VolumeWrappedTemplateDayStat {
    int templateDayId = 0;
    std::string dayName;
    int sessionCount = 0;
};

// Summary for the highest-volume workout day.
struct VolumeWrappedVolumeDay {
    int sessionId = 0;
    std::string performedAtIso;
    std::string sourceLabel;
    int totalSets = 0;
    long long totalReps = 0;
    double totalVolume = 0.0;
};

// User streak summary.
struct VolumeWrappedStreakStats {
    int currentStreak = 0;
    int bestStreak = 0;
    std::string lastWorkoutDate;
};

// Full Volume Wrapped report for a date range.
struct VolumeWrappedReport {
    bool hasWorkouts = false;

    std::string startIsoInclusive;
    std::string endIsoExclusive;

    int workoutsCompleted = 0;
    int totalSets = 0;
    long long totalReps = 0;
    double totalVolume = 0.0;

    bool hasTrainingTimeData = false;
    long long totalTrainingSeconds = 0;

    VolumeWrappedStreakStats streaks;

    int totalActiveWeeks = 0;
    int totalCalendarWeeks = 0;
    double averageWorkoutsPerWeek = 0.0;

    std::vector<WeeklyTrendRow> weeklyTrends;
    std::vector<VolumeWrappedExerciseStat> top5ExercisesByVolume;
    std::vector<VolumeWrappedExerciseStat> top5ExercisesBySets;

    bool hasMostRepeatedExercise = false;
    VolumeWrappedExerciseStat mostRepeatedExercise;

    bool hasMostPerformedTemplateDay = false;
    VolumeWrappedTemplateDayStat mostPerformedTemplateDay;

    bool hasBiggestVolumeDay = false;
    VolumeWrappedVolumeDay biggestVolumeDay;
};