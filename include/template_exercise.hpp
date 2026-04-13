#pragma once
#include <string>
#include <vector>
#include "database.hpp"

//Author: Negar Dehghaneian

// TemplateExercise struct - represents an exercise in a template day
struct TemplateExercise {
    int templateExerciseId = 0;
    int templateDayId = 0;
    int exerciseId = 0;
    int sortOrder = 0;
    int targetSets = 0;
    std::string targetReps;
    double targetWeightKg = 0;
};

// Extended version with exercise name for display
struct TemplateExerciseDetail {
    int templateExerciseId = 0;
    int templateDayId = 0;
    int exerciseId = 0;
    std::string exerciseName;
    int sortOrder = 0;
    int targetSets = 0;
    std::string targetReps;
    double targetWeightKg = 0;
};

// Repository class - handles database operations for template exercises
class TemplateExerciseRepository {
public:
    explicit TemplateExerciseRepository(Database& db) : db_(db) {}

    // Insert a new template exercise
    int insert(int templateDayId, int exerciseId, int sortOrder);

    // Insert with target prescription
    int insertWithTargets(int templateDayId, int exerciseId, int sortOrder,
                          int targetSets, const std::string& targetReps, double targetWeightKg);

    // Find template exercise by ID
    TemplateExercise findById(int templateExerciseId);

    // Get all exercises for a template day
    std::vector<TemplateExercise> findByTemplateDayId(int templateDayId);

    // Get exercises with names for display
    std::vector<TemplateExerciseDetail> findByTemplateDayIdWithDetails(int templateDayId);

    // Check if exercise already exists in template day
    bool existsInTemplateDay(int templateDayId, int exerciseId);

    // Get max sort order for a template day
    int getMaxSortOrder(int templateDayId);

private:
    Database& db_;
};

// Service class - handles business logic for template exercises
class TemplateExerciseService {
public:
    explicit TemplateExerciseService(TemplateExerciseRepository& repo) : repo_(repo) {}

    // Add an exercise to a template day
    TemplateExercise addExerciseToTemplateDay(int templateDayId, int exerciseId);

    // Add an exercise with target prescription
    TemplateExercise addExerciseWithTargets(int templateDayId, int exerciseId,
                                            int targetSets, const std::string& targetReps,
                                            double targetWeightKg);

    // Get all exercises for a template day
    std::vector<TemplateExercise> getExercisesByTemplateDay(int templateDayId);

    // Get exercises with details for display
    std::vector<TemplateExerciseDetail> getExercisesWithDetails(int templateDayId);

    // Check if an exercise already exists in a template day
    bool existsInTemplateDay(int templateDayId, int exerciseId);

private:
    TemplateExerciseRepository& repo_;
};
