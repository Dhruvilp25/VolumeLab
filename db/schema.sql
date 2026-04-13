-- db/schema.sql
-- SQLite schema for VolumeLab

PRAGMA foreign_keys = ON;

-- USERS TABLE
CREATE TABLE IF NOT EXISTS users (
  user_id       INTEGER PRIMARY KEY AUTOINCREMENT,
  username      TEXT NOT NULL UNIQUE,
  password_hash TEXT NOT NULL,
  salt          TEXT NOT NULL,
  created_at    TEXT NOT NULL DEFAULT (datetime('now'))
);

-- PROGRAMS TABLE
CREATE TABLE IF NOT EXISTS programs (
  program_id INTEGER PRIMARY KEY AUTOINCREMENT,
  user_id    INTEGER NOT NULL,
  name       TEXT NOT NULL,
  created_at TEXT NOT NULL DEFAULT (datetime('now')),
  FOREIGN KEY (user_id) REFERENCES users(user_id)
    ON DELETE CASCADE
);

CREATE INDEX IF NOT EXISTS idx_programs_user_id ON programs(user_id);

-- TEMPLATE_DAYS TABLE
CREATE TABLE IF NOT EXISTS template_days (
  template_day_id INTEGER PRIMARY KEY AUTOINCREMENT,
  program_id      INTEGER NOT NULL,
  name            TEXT NOT NULL,
  sort_order      INTEGER NOT NULL DEFAULT 0,
  FOREIGN KEY (program_id) REFERENCES programs(program_id)
    ON DELETE CASCADE,
  UNIQUE(program_id, name)
);

CREATE INDEX IF NOT EXISTS idx_template_days_program_id ON template_days(program_id);

-- EXERCISES TABLE 
CREATE TABLE IF NOT EXISTS exercises (
  exercise_id  INTEGER PRIMARY KEY AUTOINCREMENT,
  user_id      INTEGER NOT NULL,
  name         TEXT NOT NULL,
  is_archived  INTEGER NOT NULL DEFAULT 0,
  created_at   TEXT NOT NULL DEFAULT (datetime('now')),
  FOREIGN KEY (user_id) REFERENCES users(user_id)
    ON DELETE CASCADE,
  UNIQUE(user_id, name),
  CHECK (is_archived IN (0,1))
);

CREATE INDEX IF NOT EXISTS idx_exercises_user_id ON exercises(user_id);

-- TEMPLATE_EXERCISES TABLE
CREATE TABLE IF NOT EXISTS template_exercises (
  template_exercise_id INTEGER PRIMARY KEY AUTOINCREMENT,
  template_day_id      INTEGER NOT NULL,
  exercise_id          INTEGER NOT NULL,
  sort_order           INTEGER NOT NULL DEFAULT 0,
  target_sets          INTEGER NOT NULL DEFAULT 0,
  target_reps          TEXT    NOT NULL DEFAULT '',
  target_weight_kg     REAL   NOT NULL DEFAULT 0,
  FOREIGN KEY (template_day_id) REFERENCES template_days(template_day_id)
    ON DELETE CASCADE,
  FOREIGN KEY (exercise_id) REFERENCES exercises(exercise_id)
    ON DELETE RESTRICT
);

CREATE INDEX IF NOT EXISTS idx_template_exercises_day ON template_exercises(template_day_id);
CREATE INDEX IF NOT EXISTS idx_template_exercises_exercise ON template_exercises(exercise_id);

-- TEMPLATE_SETS TABLE
CREATE TABLE IF NOT EXISTS template_sets (
  template_set_id      INTEGER PRIMARY KEY AUTOINCREMENT,
  template_exercise_id INTEGER NOT NULL,
  set_number           INTEGER NOT NULL,
  target_reps          INTEGER NOT NULL,
  target_weight        REAL NOT NULL,
  FOREIGN KEY (template_exercise_id) REFERENCES template_exercises(template_exercise_id)
    ON DELETE CASCADE,
  UNIQUE(template_exercise_id, set_number),
  CHECK (set_number >= 1),
  CHECK (target_reps >= 0),
  CHECK (target_weight >= 0)
);

CREATE INDEX IF NOT EXISTS idx_template_sets_teid ON template_sets(template_exercise_id);

-- WORKOUT_SESSIONS TABLE
CREATE TABLE IF NOT EXISTS workout_sessions (
  session_id             INTEGER PRIMARY KEY AUTOINCREMENT,
  user_id                INTEGER NOT NULL,
  performed_at           TEXT NOT NULL DEFAULT (datetime('now')),
  duration_seconds       INTEGER NULL,
  source_template_day_id INTEGER NULL,
  source_label           TEXT NOT NULL,
  FOREIGN KEY (user_id) REFERENCES users(user_id)
    ON DELETE CASCADE,
  FOREIGN KEY (source_template_day_id) REFERENCES template_days(template_day_id)
    ON DELETE SET NULL
);

CREATE INDEX IF NOT EXISTS idx_sessions_user_time ON workout_sessions(user_id, performed_at);

-- SESSION_EXERCISES TABLE 
CREATE TABLE IF NOT EXISTS session_exercises (
  session_exercise_id    INTEGER PRIMARY KEY AUTOINCREMENT,
  session_id             INTEGER NOT NULL,
  exercise_id            INTEGER NOT NULL,
  exercise_name_snapshot TEXT NOT NULL,
  sort_order             INTEGER NOT NULL DEFAULT 0,
  FOREIGN KEY (session_id) REFERENCES workout_sessions(session_id)
    ON DELETE CASCADE,
  FOREIGN KEY (exercise_id) REFERENCES exercises(exercise_id)
    ON DELETE RESTRICT
);

CREATE INDEX IF NOT EXISTS idx_session_exercises_session ON session_exercises(session_id);
CREATE INDEX IF NOT EXISTS idx_session_exercises_exercise ON session_exercises(exercise_id);

-- SESSION_SETS TABLE
CREATE TABLE IF NOT EXISTS session_sets (
  session_set_id      INTEGER PRIMARY KEY AUTOINCREMENT,
  session_exercise_id INTEGER NOT NULL,
  set_number          INTEGER NOT NULL,
  reps                INTEGER NOT NULL,
  weight              REAL NOT NULL,
  FOREIGN KEY (session_exercise_id) REFERENCES session_exercises(session_exercise_id)
    ON DELETE CASCADE,
  UNIQUE(session_exercise_id, set_number),
  CHECK (set_number >= 1),
  CHECK (reps >= 0),
  CHECK (weight >= 0)
);

CREATE INDEX IF NOT EXISTS idx_session_sets_seid ON session_sets(session_exercise_id);

-- PLANNED_WORKOUTS TABLE (scheduling + rescheduling + completion linking)
CREATE TABLE IF NOT EXISTS planned_workouts (
  planned_id           INTEGER PRIMARY KEY AUTOINCREMENT,
  user_id              INTEGER NOT NULL,
  planned_for          TEXT NOT NULL,
  template_day_id      INTEGER NULL,
  title_snapshot       TEXT NOT NULL,
  status               TEXT NOT NULL DEFAULT 'planned', 
  completed_session_id INTEGER NULL,
  original_planned_for TEXT NULL,
  reschedule_count     INTEGER NOT NULL DEFAULT 0,
  created_at           TEXT NOT NULL DEFAULT (datetime('now')),

  FOREIGN KEY (user_id) REFERENCES users(user_id)
    ON DELETE CASCADE,
  FOREIGN KEY (template_day_id) REFERENCES template_days(template_day_id)
    ON DELETE SET NULL,
  FOREIGN KEY (completed_session_id) REFERENCES workout_sessions(session_id)
    ON DELETE SET NULL,

  CHECK (status IN ('planned','completed','missed','cancelled')),
  CHECK (reschedule_count >= 0)
);

CREATE INDEX IF NOT EXISTS idx_planned_user_date ON planned_workouts(user_id, planned_for);
CREATE INDEX IF NOT EXISTS idx_planned_user_status ON planned_workouts(user_id, status);

-- STREAKS TABLE (stateful streak tracking)
CREATE TABLE IF NOT EXISTS streaks (
  user_id           INTEGER PRIMARY KEY,
  current_streak    INTEGER NOT NULL DEFAULT 0,
  best_streak       INTEGER NOT NULL DEFAULT 0,
  last_workout_date TEXT NULL,
  updated_at        TEXT NOT NULL DEFAULT (datetime('now')),

  FOREIGN KEY (user_id) REFERENCES users(user_id)
    ON DELETE CASCADE,

  CHECK (current_streak >= 0),
  CHECK (best_streak >= 0)
);

-- GOALS TABLE (user-defined goals + progression targets)
CREATE TABLE IF NOT EXISTS goals (
  goal_id      INTEGER PRIMARY KEY AUTOINCREMENT,
  user_id      INTEGER NOT NULL,
  exercise_id  INTEGER NULL,
  goal_type    TEXT NOT NULL,
  target_value REAL NOT NULL,
  start_date   TEXT NOT NULL,
  end_date     TEXT NULL,
  is_active    INTEGER NOT NULL DEFAULT 1,
  created_at   TEXT NOT NULL DEFAULT (datetime('now')),

  FOREIGN KEY (user_id) REFERENCES users(user_id)
    ON DELETE CASCADE,
  FOREIGN KEY (exercise_id) REFERENCES exercises(exercise_id)
    ON DELETE SET NULL,

  CHECK (is_active IN (0,1)),
  CHECK (target_value >= 0)
);

CREATE INDEX IF NOT EXISTS idx_goals_user_active ON goals(user_id, is_active);
CREATE INDEX IF NOT EXISTS idx_goals_user_type ON goals(user_id, goal_type);