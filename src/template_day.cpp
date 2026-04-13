/**
 * @file template_day.cpp
 * @brief Implements TemplateDayRepository and TemplateDayService methods.
 *
 * Contains all SQLite queries for template day CRUD, summary aggregation,
 * and the service-layer validation logic.
 *
 * @author Negar Dehghaneian
 */

#include "template_day.hpp"
#include <stdexcept>

// TemplateDayRepository - Database Operations

/// @brief Insert a new template day row and return the generated ID.
int TemplateDayRepository::insert(int programId, const std::string& name, int sortOrder) {
    const std::string sql = R"SQL(
        INSERT INTO template_days (program_id, name, sort_order)
        VALUES (?, ?, ?);
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, programId);
        Database::bindText(stmt, 2, name);
        Database::bindInt(stmt, 3, sortOrder);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            throw std::runtime_error("insert template_day failed");
        }

        // Get the last inserted row ID
        const std::string lastIdSql = "SELECT last_insert_rowid();";
        sqlite3_stmt* idStmt = db_.prepare(lastIdSql);
        int templateDayId = 0;
        if (sqlite3_step(idStmt) == SQLITE_ROW) {
            templateDayId = Database::colInt(idStmt, 0);
        }
        db_.finalize(idStmt);
        db_.finalize(stmt);

        return templateDayId;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Look up a template day by primary key.
TemplateDay TemplateDayRepository::findById(int templateDayId) {
    const std::string sql = R"SQL(
        SELECT template_day_id, program_id, name, sort_order
        FROM template_days
        WHERE template_day_id = ?;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, templateDayId);

        int rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) {
            throw std::runtime_error("TemplateDay not found");
        }

        TemplateDay td;
        td.templateDayId = Database::colInt(stmt, 0);
        td.programId = Database::colInt(stmt, 1);
        td.name = Database::colText(stmt, 2);
        td.sortOrder = Database::colInt(stmt, 3);

        db_.finalize(stmt);
        return td;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Get all template days for a program, ordered by sort order.
std::vector<TemplateDay> TemplateDayRepository::findByProgramId(int programId) {
    const std::string sql = R"SQL(
        SELECT template_day_id, program_id, name, sort_order
        FROM template_days
        WHERE program_id = ?
        ORDER BY sort_order ASC, template_day_id ASC;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, programId);

        std::vector<TemplateDay> out;
        while (true) {
            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE) break;
            if (rc != SQLITE_ROW) {
                throw std::runtime_error("findByProgramId: sqlite3_step failed");
            }

            TemplateDay td;
            td.templateDayId = Database::colInt(stmt, 0);
            td.programId = Database::colInt(stmt, 1);
            td.name = Database::colText(stmt, 2);
            td.sortOrder = Database::colInt(stmt, 3);
            out.push_back(td);
        }

        db_.finalize(stmt);
        return out;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Get template days with aggregated exercise and set counts.
std::vector<TemplateDaySummary> TemplateDayRepository::findByProgramIdWithSummary(int programId) {
    const std::string sql = R"SQL(
        SELECT 
            td.template_day_id,
            td.program_id,
            td.name,
            td.sort_order,
            COUNT(DISTINCT te.template_exercise_id) AS exercise_count,
            COALESCE(SUM(te.target_sets), 0) AS total_sets
        FROM template_days td
        LEFT JOIN template_exercises te ON te.template_day_id = td.template_day_id
        WHERE td.program_id = ?
        GROUP BY td.template_day_id
        ORDER BY td.sort_order ASC, td.template_day_id ASC;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, programId);

        std::vector<TemplateDaySummary> out;
        while (true) {
            int rc = sqlite3_step(stmt);
            if (rc == SQLITE_DONE) break;
            if (rc != SQLITE_ROW) {
                throw std::runtime_error("findByProgramIdWithSummary: sqlite3_step failed");
            }

            TemplateDaySummary tds;
            tds.templateDayId = Database::colInt(stmt, 0);
            tds.programId = Database::colInt(stmt, 1);
            tds.name = Database::colText(stmt, 2);
            tds.sortOrder = Database::colInt(stmt, 3);
            tds.exerciseCount = Database::colInt(stmt, 4);
            tds.totalSets = Database::colInt(stmt, 5);
            out.push_back(tds);
        }

        db_.finalize(stmt);
        return out;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Return true if the name already exists in the program.
bool TemplateDayRepository::existsByName(int programId, const std::string& name) {
    const std::string sql = R"SQL(
        SELECT COUNT(*) FROM template_days
        WHERE program_id = ? AND name = ?;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, programId);
        Database::bindText(stmt, 2, name);

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

/// @brief Return the highest sort order in the program (0 if empty)
int TemplateDayRepository::getMaxSortOrder(int programId) {
    const std::string sql = R"SQL(
        SELECT COALESCE(MAX(sort_order), 0) FROM template_days
        WHERE program_id = ?;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, programId);

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

/// @brief Check for duplicate name, excluding one template day (used during rename).
bool TemplateDayRepository::existsByNameExcluding(int programId, const std::string& name, int excludeTemplateId) {
    const std::string sql = R"SQL(
        SELECT COUNT(*) FROM template_days
        WHERE program_id = ? AND name = ? AND template_day_id != ?;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, programId);
        Database::bindText(stmt, 2, name);
        Database::bindInt(stmt, 3, excludeTemplateId);

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

/// @brief Update a template day's name in the database.
bool TemplateDayRepository::update(int templateDayId, const std::string& newName) {
    const std::string sql = R"SQL(
        UPDATE template_days SET name = ? WHERE template_day_id = ?;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindText(stmt, 1, newName);
        Database::bindInt(stmt, 2, templateDayId);

        int rc = sqlite3_step(stmt);
        db_.finalize(stmt);

        if (rc != SQLITE_DONE) {
            throw std::runtime_error("update template_day failed");
        }
        return true;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}

/// @brief Delete a template day row by its ID.
bool TemplateDayRepository::deleteById(int templateDayId) {
    const std::string sql = R"SQL(
        DELETE FROM template_days WHERE template_day_id = ?;
    )SQL";

    sqlite3_stmt* stmt = db_.prepare(sql);
    try {
        Database::bindInt(stmt, 1, templateDayId);

        int rc = sqlite3_step(stmt);
        db_.finalize(stmt);

        if (rc != SQLITE_DONE) {
            throw std::runtime_error("delete template_day failed");
        }
        return true;
    } catch (...) {
        db_.finalize(stmt);
        throw;
    }
}


// TemplateDayService - Business Logic 

/// @brief Remove leading/trailing whitespace.
std::string TemplateDayService::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

/// @brief Create a template day after validating name and assigning sort order.
TemplateDay TemplateDayService::createTemplateDay(int programId, const std::string& name) {
    // Validate name
    std::string trimmedName = trim(name);
    if (trimmedName.empty()) {
        throw std::invalid_argument("Template day name cannot be empty");
    }

    // Check for duplicate name in this program
    if (repo_.existsByName(programId, trimmedName)) {
        throw std::runtime_error("Template day name already exists in this program");
    }

    // Get next sort order
    int sortOrder = repo_.getMaxSortOrder(programId) + 1;

    // Insert and return the created template day
    int templateDayId = repo_.insert(programId, trimmedName, sortOrder);
    return repo_.findById(templateDayId);
}

/// @brief Delegate to repository to list template days.
std::vector<TemplateDay> TemplateDayService::getTemplateDaysByProgram(int programId) {
    return repo_.findByProgramId(programId);
}

/// @brief Delegate to repository to get days with summary counts.
std::vector<TemplateDaySummary> TemplateDayService::getTemplateDaysWithSummary(int programId) {
    return repo_.findByProgramIdWithSummary(programId);
}

/// @brief Delegate to repository to fetch a single template day.
TemplateDay TemplateDayService::getTemplateDayById(int templateDayId) {
    return repo_.findById(templateDayId);
}

/// @brief Rename a template day, enforcing uniqueness within the program.
bool TemplateDayService::renameTemplateDay(int templateDayId, const std::string& newName) {
    // Validate name
    std::string trimmedName = trim(newName);
    if (trimmedName.empty()) {
        throw std::invalid_argument("Template day name cannot be empty");
    }

    // Get existing template day to check program and existence
    TemplateDay existing = repo_.findById(templateDayId);

    // Check for duplicate name in this program (excluding current)
    if (repo_.existsByNameExcluding(existing.programId, trimmedName, templateDayId)) {
        throw std::runtime_error("Template day name already exists in this program");
    }

    return repo_.update(templateDayId, trimmedName);
}

/// @brief Delete a template day after verifying it exists.
bool TemplateDayService::deleteTemplateDay(int templateDayId) {
    // Verify template day exists first
    repo_.findById(templateDayId); // throws if not found
    
    return repo_.deleteById(templateDayId);
}
