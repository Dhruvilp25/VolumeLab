PRAGMA foreign_keys = ON;
BEGIN TRANSACTION;

-- Remove previous demo fixtures (safe to re-run)
DELETE FROM session_sets
WHERE session_exercise_id IN (
    SELECT se.session_exercise_id
    FROM session_exercises se
    JOIN workout_sessions ws ON ws.session_id = se.session_id
    JOIN users u ON u.user_id = ws.user_id
    WHERE u.username IN ('demo', 'demo_alt')
);

DELETE FROM session_exercises
WHERE session_id IN (
    SELECT ws.session_id
    FROM workout_sessions ws
    JOIN users u ON u.user_id = ws.user_id
    WHERE u.username IN ('demo', 'demo_alt')
);

DELETE FROM workout_sessions
WHERE user_id IN (
    SELECT user_id FROM users WHERE username IN ('demo', 'demo_alt')
);

DELETE FROM planned_workouts
WHERE user_id IN (
    SELECT user_id FROM users WHERE username IN ('demo', 'demo_alt')
);

DELETE FROM goals
WHERE user_id IN (
    SELECT user_id FROM users WHERE username IN ('demo', 'demo_alt')
);

DELETE FROM streaks
WHERE user_id IN (
    SELECT user_id FROM users WHERE username IN ('demo', 'demo_alt')
);

DELETE FROM template_sets
WHERE template_exercise_id IN (
    SELECT te.template_exercise_id
    FROM template_exercises te
    JOIN template_days td ON td.template_day_id = te.template_day_id
    JOIN programs p ON p.program_id = td.program_id
    JOIN users u ON u.user_id = p.user_id
    WHERE u.username IN ('demo', 'demo_alt')
);

DELETE FROM template_exercises
WHERE template_day_id IN (
    SELECT td.template_day_id
    FROM template_days td
    JOIN programs p ON p.program_id = td.program_id
    JOIN users u ON u.user_id = p.user_id
    WHERE u.username IN ('demo', 'demo_alt')
);

DELETE FROM template_days
WHERE program_id IN (
    SELECT p.program_id
    FROM programs p
    JOIN users u ON u.user_id = p.user_id
    WHERE u.username IN ('demo', 'demo_alt')
);

DELETE FROM exercises
WHERE user_id IN (
    SELECT user_id FROM users WHERE username IN ('demo', 'demo_alt')
);

DELETE FROM programs
WHERE user_id IN (
    SELECT user_id FROM users WHERE username IN ('demo', 'demo_alt')
);

DELETE FROM users WHERE username IN ('demo', 'demo_alt');

-- demo credentials:
-- username: demo
-- password: demo123
-- salt: volumelabDemoSalt
-- hash: sha256(salt + password)
INSERT INTO users (username, password_hash, salt)
VALUES ('demo', '21efe14b99dc947c465d2062116dccc77b1ca915bcc67d6aa35a5ac08b70ac9c', 'volumelabDemoSalt');

INSERT INTO users (username, password_hash, salt)
VALUES ('demo_alt', '21efe14b99dc947c465d2062116dccc77b1ca915bcc67d6aa35a5ac08b70ac9c', 'volumelabDemoSalt');

-- Programs for demo
INSERT INTO programs (user_id, name)
SELECT user_id, 'Push Pull Legs'
FROM users
WHERE username = 'demo';

INSERT INTO programs (user_id, name)
SELECT user_id, 'Upper / Lower'
FROM users
WHERE username = 'demo';

-- Template days
INSERT INTO template_days (program_id, name, sort_order)
SELECT p.program_id, 'Push Day', 1
FROM programs p
JOIN users u ON u.user_id = p.user_id
WHERE u.username = 'demo' AND p.name = 'Push Pull Legs';

INSERT INTO template_days (program_id, name, sort_order)
SELECT p.program_id, 'Pull Day', 2
FROM programs p
JOIN users u ON u.user_id = p.user_id
WHERE u.username = 'demo' AND p.name = 'Push Pull Legs';

INSERT INTO template_days (program_id, name, sort_order)
SELECT p.program_id, 'Leg Day', 3
FROM programs p
JOIN users u ON u.user_id = p.user_id
WHERE u.username = 'demo' AND p.name = 'Push Pull Legs';

INSERT INTO template_days (program_id, name, sort_order)
SELECT p.program_id, 'Upper A', 1
FROM programs p
JOIN users u ON u.user_id = p.user_id
WHERE u.username = 'demo' AND p.name = 'Upper / Lower';

INSERT INTO template_days (program_id, name, sort_order)
SELECT p.program_id, 'Lower A', 2
FROM programs p
JOIN users u ON u.user_id = p.user_id
WHERE u.username = 'demo' AND p.name = 'Upper / Lower';

-- Exercise library for demo user
INSERT INTO exercises (user_id, name)
SELECT u.user_id, e.name
FROM users u
JOIN (
    SELECT 'Bench Press' AS name UNION ALL
    SELECT 'Incline Dumbbell Press' UNION ALL
    SELECT 'Overhead Press' UNION ALL
    SELECT 'Lateral Raise' UNION ALL
    SELECT 'Tricep Pushdown' UNION ALL
    SELECT 'Pull Up' UNION ALL
    SELECT 'Barbell Row' UNION ALL
    SELECT 'Seated Cable Row' UNION ALL
    SELECT 'Barbell Curl' UNION ALL
    SELECT 'Back Squat' UNION ALL
    SELECT 'Romanian Deadlift' UNION ALL
    SELECT 'Leg Press' UNION ALL
    SELECT 'Hamstring Curl' UNION ALL
    SELECT 'Calf Raise'
) e
WHERE u.username = 'demo';

-- Template exercises with targets
INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 1, 4, '6-8', 80
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Bench Press'
WHERE u.username = 'demo' AND td.name = 'Push Day';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 2, 3, '8-10', 26
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Incline Dumbbell Press'
WHERE u.username = 'demo' AND td.name = 'Push Day';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 3, 3, '6-8', 50
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Overhead Press'
WHERE u.username = 'demo' AND td.name = 'Push Day';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 4, 3, '12-15', 8
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Lateral Raise'
WHERE u.username = 'demo' AND td.name = 'Push Day';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 5, 3, '10-12', 25
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Tricep Pushdown'
WHERE u.username = 'demo' AND td.name = 'Push Day';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 1, 3, '6-10', 0
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Pull Up'
WHERE u.username = 'demo' AND td.name = 'Pull Day';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 2, 3, '6-8', 70
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Barbell Row'
WHERE u.username = 'demo' AND td.name = 'Pull Day';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 3, 3, '8-12', 55
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Seated Cable Row'
WHERE u.username = 'demo' AND td.name = 'Pull Day';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 4, 3, '10-12', 18
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Barbell Curl'
WHERE u.username = 'demo' AND td.name = 'Pull Day';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 1, 4, '5-8', 100
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Back Squat'
WHERE u.username = 'demo' AND td.name = 'Leg Day';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 2, 3, '8-10', 90
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Romanian Deadlift'
WHERE u.username = 'demo' AND td.name = 'Leg Day';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 3, 3, '10-12', 160
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Leg Press'
WHERE u.username = 'demo' AND td.name = 'Leg Day';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 4, 3, '10-12', 45
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Hamstring Curl'
WHERE u.username = 'demo' AND td.name = 'Leg Day';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 5, 4, '12-15', 70
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Calf Raise'
WHERE u.username = 'demo' AND td.name = 'Leg Day';

-- Upper / Lower day presets so all template days are runnable
INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 1, 4, '6-8', 80
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Bench Press'
WHERE u.username = 'demo' AND p.name = 'Upper / Lower' AND td.name = 'Upper A';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 2, 3, '8-10', 26
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Incline Dumbbell Press'
WHERE u.username = 'demo' AND p.name = 'Upper / Lower' AND td.name = 'Upper A';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 3, 3, '6-8', 70
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Barbell Row'
WHERE u.username = 'demo' AND p.name = 'Upper / Lower' AND td.name = 'Upper A';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 1, 4, '5-8', 100
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Back Squat'
WHERE u.username = 'demo' AND p.name = 'Upper / Lower' AND td.name = 'Lower A';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 2, 3, '8-10', 90
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Romanian Deadlift'
WHERE u.username = 'demo' AND p.name = 'Upper / Lower' AND td.name = 'Lower A';

INSERT INTO template_exercises (template_day_id, exercise_id, sort_order, target_sets, target_reps, target_weight_kg)
SELECT td.template_day_id, ex.exercise_id, 3, 3, '10-12', 160
FROM template_days td
JOIN programs p ON p.program_id = td.program_id
JOIN users u ON u.user_id = p.user_id
JOIN exercises ex ON ex.user_id = u.user_id AND ex.name = 'Leg Press'
WHERE u.username = 'demo' AND p.name = 'Upper / Lower' AND td.name = 'Lower A';

-- Past sessions for History/Analytics showcase
INSERT INTO workout_sessions (user_id, performed_at, duration_seconds, source_label)
SELECT user_id, '2026-04-01 18:00:00', 3500, 'Push Day' FROM users WHERE username = 'demo';
INSERT INTO workout_sessions (user_id, performed_at, duration_seconds, source_label)
SELECT user_id, '2026-04-03 18:15:00', 3600, 'Pull Day' FROM users WHERE username = 'demo';
INSERT INTO workout_sessions (user_id, performed_at, duration_seconds, source_label)
SELECT user_id, '2026-04-05 17:50:00', 3900, 'Leg Day' FROM users WHERE username = 'demo';

-- Session 1 data (Push)
INSERT INTO session_exercises (session_id, exercise_id, exercise_name_snapshot, sort_order)
SELECT s.session_id, e.exercise_id, 'Bench Press', 1
FROM workout_sessions s
JOIN users u ON u.user_id = s.user_id
JOIN exercises e ON e.user_id = u.user_id AND e.name = 'Bench Press'
WHERE u.username = 'demo' AND s.performed_at = '2026-04-01 18:00:00';

INSERT INTO session_exercises (session_id, exercise_id, exercise_name_snapshot, sort_order)
SELECT s.session_id, e.exercise_id, 'Overhead Press', 2
FROM workout_sessions s
JOIN users u ON u.user_id = s.user_id
JOIN exercises e ON e.user_id = u.user_id AND e.name = 'Overhead Press'
WHERE u.username = 'demo' AND s.performed_at = '2026-04-01 18:00:00';

INSERT INTO session_sets (session_exercise_id, set_number, reps, weight)
SELECT se.session_exercise_id, 1, 8, 80
FROM session_exercises se
JOIN workout_sessions s ON s.session_id = se.session_id
JOIN users u ON u.user_id = s.user_id
WHERE u.username = 'demo' AND s.performed_at = '2026-04-01 18:00:00' AND se.exercise_name_snapshot = 'Bench Press';
INSERT INTO session_sets (session_exercise_id, set_number, reps, weight)
SELECT se.session_exercise_id, 2, 8, 80
FROM session_exercises se
JOIN workout_sessions s ON s.session_id = se.session_id
JOIN users u ON u.user_id = s.user_id
WHERE u.username = 'demo' AND s.performed_at = '2026-04-01 18:00:00' AND se.exercise_name_snapshot = 'Bench Press';
INSERT INTO session_sets (session_exercise_id, set_number, reps, weight)
SELECT se.session_exercise_id, 1, 8, 50
FROM session_exercises se
JOIN workout_sessions s ON s.session_id = se.session_id
JOIN users u ON u.user_id = s.user_id
WHERE u.username = 'demo' AND s.performed_at = '2026-04-01 18:00:00' AND se.exercise_name_snapshot = 'Overhead Press';

-- Session 2 data (Pull)
INSERT INTO session_exercises (session_id, exercise_id, exercise_name_snapshot, sort_order)
SELECT s.session_id, e.exercise_id, 'Barbell Row', 1
FROM workout_sessions s
JOIN users u ON u.user_id = s.user_id
JOIN exercises e ON e.user_id = u.user_id AND e.name = 'Barbell Row'
WHERE u.username = 'demo' AND s.performed_at = '2026-04-03 18:15:00';

INSERT INTO session_exercises (session_id, exercise_id, exercise_name_snapshot, sort_order)
SELECT s.session_id, e.exercise_id, 'Barbell Curl', 2
FROM workout_sessions s
JOIN users u ON u.user_id = s.user_id
JOIN exercises e ON e.user_id = u.user_id AND e.name = 'Barbell Curl'
WHERE u.username = 'demo' AND s.performed_at = '2026-04-03 18:15:00';

INSERT INTO session_sets (session_exercise_id, set_number, reps, weight)
SELECT se.session_exercise_id, 1, 8, 70
FROM session_exercises se
JOIN workout_sessions s ON s.session_id = se.session_id
JOIN users u ON u.user_id = s.user_id
WHERE u.username = 'demo' AND s.performed_at = '2026-04-03 18:15:00' AND se.exercise_name_snapshot = 'Barbell Row';
INSERT INTO session_sets (session_exercise_id, set_number, reps, weight)
SELECT se.session_exercise_id, 2, 8, 70
FROM session_exercises se
JOIN workout_sessions s ON s.session_id = se.session_id
JOIN users u ON u.user_id = s.user_id
WHERE u.username = 'demo' AND s.performed_at = '2026-04-03 18:15:00' AND se.exercise_name_snapshot = 'Barbell Row';
INSERT INTO session_sets (session_exercise_id, set_number, reps, weight)
SELECT se.session_exercise_id, 1, 10, 18
FROM session_exercises se
JOIN workout_sessions s ON s.session_id = se.session_id
JOIN users u ON u.user_id = s.user_id
WHERE u.username = 'demo' AND s.performed_at = '2026-04-03 18:15:00' AND se.exercise_name_snapshot = 'Barbell Curl';

-- Session 3 data (Legs)
INSERT INTO session_exercises (session_id, exercise_id, exercise_name_snapshot, sort_order)
SELECT s.session_id, e.exercise_id, 'Back Squat', 1
FROM workout_sessions s
JOIN users u ON u.user_id = s.user_id
JOIN exercises e ON e.user_id = u.user_id AND e.name = 'Back Squat'
WHERE u.username = 'demo' AND s.performed_at = '2026-04-05 17:50:00';

INSERT INTO session_exercises (session_id, exercise_id, exercise_name_snapshot, sort_order)
SELECT s.session_id, e.exercise_id, 'Romanian Deadlift', 2
FROM workout_sessions s
JOIN users u ON u.user_id = s.user_id
JOIN exercises e ON e.user_id = u.user_id AND e.name = 'Romanian Deadlift'
WHERE u.username = 'demo' AND s.performed_at = '2026-04-05 17:50:00';

INSERT INTO session_sets (session_exercise_id, set_number, reps, weight)
SELECT se.session_exercise_id, 1, 6, 100
FROM session_exercises se
JOIN workout_sessions s ON s.session_id = se.session_id
JOIN users u ON u.user_id = s.user_id
WHERE u.username = 'demo' AND s.performed_at = '2026-04-05 17:50:00' AND se.exercise_name_snapshot = 'Back Squat';
INSERT INTO session_sets (session_exercise_id, set_number, reps, weight)
SELECT se.session_exercise_id, 2, 6, 100
FROM session_exercises se
JOIN workout_sessions s ON s.session_id = se.session_id
JOIN users u ON u.user_id = s.user_id
WHERE u.username = 'demo' AND s.performed_at = '2026-04-05 17:50:00' AND se.exercise_name_snapshot = 'Back Squat';
INSERT INTO session_sets (session_exercise_id, set_number, reps, weight)
SELECT se.session_exercise_id, 1, 8, 90
FROM session_exercises se
JOIN workout_sessions s ON s.session_id = se.session_id
JOIN users u ON u.user_id = s.user_id
WHERE u.username = 'demo' AND s.performed_at = '2026-04-05 17:50:00' AND se.exercise_name_snapshot = 'Romanian Deadlift';

COMMIT;
