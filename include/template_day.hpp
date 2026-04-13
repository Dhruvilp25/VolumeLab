#pragma once
#include <string>
#include <vector>
#include "database.hpp"

//Author: Negar Dehghaneian

// TemplateDay struct - represents a workout template (eg: "Leg Day")
struct TemplateDay {
    int templateDayId = 0;
    int programId = 0;
    std::string name;
    int sortOrder = 0;
};

// Summary with exercise count for display
struct TemplateDaySummary {
    int templateDayId = 0;
    int programId = 0;
    std::string name;
    int sortOrder = 0;
    int exerciseCount = 0;
    int totalSets = 0;
};

// Repository class - handles database operations for template days
class TemplateDayRepository {
public:
    explicit TemplateDayRepository(Database& db) : db_(db) {}

    // Insert a new template day
    int insert(int programId, const std::string& name, int sortOrder);

    // Find template day by ID
    TemplateDay findById(int templateDayId);

    // Get all template days for a program
    std::vector<TemplateDay> findByProgramId(int programId);

    // Get template days with exercise/set counts
    std::vector<TemplateDaySummary> findByProgramIdWithSummary(int programId);

    // Check if name exists in program
    bool existsByName(int programId, const std::string& name);

    // Check if name exists excluding a specific template day
    bool existsByNameExcluding(int programId, const std::string& name, int excludeTemplateId);

    // Get max sort order for a program
    int getMaxSortOrder(int programId);

    // Update template day name
    bool update(int templateDayId, const std::string& newName);

    // Delete a template day
    bool deleteById(int templateDayId);

private:
    Database& db_;
};

// Service class - handles business logic for template days
class TemplateDayService {
public:
    explicit TemplateDayService(TemplateDayRepository& repo) : repo_(repo) {}

    // Create a new template day
    TemplateDay createTemplateDay(int programId, const std::string& name);

    // Get all template days for a program
    std::vector<TemplateDay> getTemplateDaysByProgram(int programId);

    // Get template days with summary info
    std::vector<TemplateDaySummary> getTemplateDaysWithSummary(int programId);

    // Get a template day by ID
    TemplateDay getTemplateDayById(int templateDayId);

    // Rename a template day
    bool renameTemplateDay(int templateDayId, const std::string& newName);

    // Delete a template day
    bool deleteTemplateDay(int templateDayId);

private:
    TemplateDayRepository& repo_;
    std::string trim(const std::string& str);
};
