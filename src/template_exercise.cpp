/**
 * @file template_exercise.cpp
 * @brief Implements TemplateExerciseRepository and TemplateExerciseService methods.
 *
 * Contains all SQLite queries for template exercise CRUD, including
 * target-prescription-aware insert and retrieval, and the service-layer
 * sort-order logic.
 *
 * @author Negar Dehghaneian
 */

#include "template_exercise.hpp"
#include <stdexcept>

// TemplateExerciseRepository - Database Operations

/// @brief Insert without targets; delegates to insertWithTargets with zeroed defaults.
int TemplateExerciseRepository::insert(int templateDayId, int exerciseId, int sortOrder) {
    return insertWithTargets(templateDayId, exerciseId, sortOrder, 0, "", 0);
}

/// @brief Insert a template exercise row with full target prescription.
int TemplateExerciseRepository::insertWithTargets(int templateDayId, int exerciseId, int sortOrder,
                                                   int targetSets, const std::string& targetReps,
                                                   double targetWeightKg) {
    const std::string sql =
        "INSERT INTO template_exercises "
        "(template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg) "
        "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, templateDayId);
        Database::bindInt(stmt, 2, exerciseId);
        Database::bindInt(stmt, 3, sortOrder);
        Database::bindInt(stmt, 4, targetSets);
        Database::bindText(stmt, 5, targetReps);
        Database::bindDouble(stmt, 6, targetWeightKg);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            throw std::runtime_error("insert template_exercise failed");
        }

        sqlite3_stmt* idStmt = db_.prepare("SELECT last_insert_rowid();");
        int templateExerciseId = 0;
        if (sqlite3_step(idStmt) == SQLITE_ROW) {
            templateExerciseId = Database::colInt(idStmt, 0);
        }
        db_.finalize(idStmt);
        db_.finalize(stmt);

        return templateExerciseId;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Look up a template exercise by primary key.
TemplateExercise TemplateExerciseRepository::findById(int templateExerciseId) {
    const std::string sql = 
        "SELECT template_exercise_id, template_day_id, exercise_id, sort_order, "
        "target_sets, target_reps, target_weight_kg "
        "FROM template_exercises WHERE template_exercise_id = ?;";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, templateExerciseId);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            throw std::runtime_error("TemplateExercise not found");
        }

        TemplateExercise te;
        te.templateExerciseId = Database::colInt(stmt, 0);
        te.templateDayId = Database::colInt(stmt, 1);
        te.exerciseId = Database::colInt(stmt, 2);
        te.sortOrder = Database::colInt(stmt, 3);
        te.targetSets = Database::colInt(stmt, 4);
        te.targetReps = Database::colText(stmt, 5);
        te.targetWeightKg = Database::colDouble(stmt, 6);

        db_.finalize(stmt);
        return te;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Get all exercises for a template day, ordered by sort order.
std::vector<TemplateExercise> TemplateExerciseRepository::findByTemplateDayId(int templateDayId) {
    const std::string sql = 
        "SELECT template_exercise_id, template_day_id, exercise_id, sort_order, "
        "target_sets, target_reps, target_weight_kg "
        "FROM template_exercises WHERE template_day_id = ? "
        "ORDER BY sort_order ASC, template_exercise_id ASC;";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, templateDayId);

        std::vector<TemplateExercise> out;
        while (true) {
            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE) break;
            if (rc != SQLITE_ROW) {
                throw std::runtime_error("findByTemplateDayId: sqlite3_step failed");
            }

            TemplateExercise te;
            te.templateExerciseId = Database::colInt(stmt, 0);
            te.templateDayId = Database::colInt(stmt, 1);
            te.exerciseId = Database::colInt(stmt, 2);
            te.sortOrder = Database::colInt(stmt, 3);
            te.targetSets = Database::colInt(stmt, 4);
            te.targetReps = Database::colText(stmt, 5);
            te.targetWeightKg = Database::colDouble(stmt, 6);
            out.push_back(te);
        }

        db_.finalize(stmt);
        return out;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Get exercises with names by joining the exercises table.
std::vector<TemplateExerciseDetail> TemplateExerciseRepository::findByTemplateDayIdWithDetails(int templateDayId) {
    const std::string sql = 
        "SELECT te.template_exercise_id, te.template_day_id, te.exercise_id, e.name, te.sort_order, "
        "te.target_sets, te.target_reps, te.target_weight_kg "
        "FROM template_exercises te "
        "JOIN exercises e ON e.exercise_id = te.exercise_id "
        "WHERE te.template_day_id = ? "
        "ORDER BY te.sort_order ASC, te.template_exercise_id ASC;";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, templateDayId);

        std::vector<TemplateExerciseDetail> out;
        while (true) {
            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE) break;
            if (rc != SQLITE_ROW) {
                throw std::runtime_error("findByTemplateDayIdWithDetails: sqlite3_step failed");
            }

            TemplateExerciseDetail ted;
            ted.templateExerciseId = Database::colInt(stmt, 0);
            ted.templateDayId = Database::colInt(stmt, 1);
            ted.exerciseId = Database::colInt(stmt, 2);
            ted.exerciseName = Database::colText(stmt, 3);
            ted.sortOrder = Database::colInt(stmt, 4);
            ted.targetSets = Database::colInt(stmt, 5);
            ted.targetReps = Database::colText(stmt, 6);
            ted.targetWeightKg = Database::colDouble(stmt, 7);
            out.push_back(ted);
        }

        db_.finalize(stmt);
        return out;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Return true if the exercise is already in the template day.
bool TemplateExerciseRepository::existsInTemplateDay(int templateDayId, int exerciseId) {
    const std::string sql = 
        "SELECT COUNT(*) FROM template_exercises "
        "WHERE template_day_id = ? AND exercise_id = ?;";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, templateDayId);
        Database::bindInt(stmt, 2, exerciseId);

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = Database::colInt(stmt, 0);
        }

        db_.finalize(stmt);
        return count > 0;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Return the highest sort order in the template day (0 if empty).
int TemplateExerciseRepository::getMaxSortOrder(int templateDayId) {
    const std::string sql = 
        "SELECT COALESCE(MAX(sort_order), 0) FROM template_exercises "
        "WHERE template_day_id = ?;";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, templateDayId);

        int maxOrder = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            maxOrder = Database::colInt(stmt, 0);
        }

        db_.finalize(stmt);
        return maxOrder;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}


// TemplateExerciseService - Business Logic

/// @brief Add an exercise without targets; auto-assigns sort order.
TemplateExercise TemplateExerciseService::addExerciseToTemplateDay(int templateDayId, int exerciseId) {
    int sortOrder = repo_.getMaxSortOrder(templateDayId) + 1;
    int templateExerciseId = repo_.insert(templateDayId, exerciseId, sortOrder);
    return repo_.findById(templateExerciseId);
}

/// @brief Add an exercise with target prescription; auto-assigns sort order.
TemplateExercise TemplateExerciseService::addExerciseWithTargets(int templateDayId, int exerciseId,
                                                                 int targetSets, const std::string& targetReps,
                                                                 double targetWeightKg) {
    int sortOrder = repo_.getMaxSortOrder(templateDayId) + 1;
    int templateExerciseId = repo_.insertWithTargets(templateDayId, exerciseId, sortOrder,
                                                     targetSets, targetReps, targetWeightKg);
    return repo_.findById(templateExerciseId);
}

/// @brief Delegate to repository to list exercises for a template day.
std::vector<TemplateExercise> TemplateExerciseService::getExercisesByTemplateDay(int templateDayId) {
    return repo_.findByTemplateDayId(templateDayId);
}

/// @brief Delegate to repository to get exercises with display names.
std::vector<TemplateExerciseDetail> TemplateExerciseService::getExercisesWithDetails(int templateDayId) {
    return repo_.findByTemplateDayIdWithDetails(templateDayId);
}

bool TemplateExerciseService::existsInTemplateDay(int templateDayId, int exerciseId) {
    return repo_.existsInTemplateDay(templateDayId, exerciseId);
}
