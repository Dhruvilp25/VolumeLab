### `users`

**Attributes:** `user_id`, `username`, `password_hash`, `salt`, `created_at`
**What it’s for:** Stores user accounts and secure login credentials.

### `programs`

**Attributes:** `program_id`, `user_id`, `name`, `created_at`
**What it’s for:** Stores a user’s named program (a container for template days).

### `template_days`

**Attributes:** `template_day_id`, `program_id`, `name`, `sort_order`
**What it’s for:** Stores reusable workout day templates (e.g., “Leg Day”) inside a program.

### `exercises`

**Attributes:** `exercise_id`, `user_id`, `name`, `is_archived`, `created_at`
**What it’s for:** Stores a user’s exercise library so templates/sessions reference consistent exercise IDs.

### `template_exercises`

**Attributes:** `template_exercise_id`, `template_day_id`, `exercise_id`, `sort_order`
**What it’s for:** Stores the ordered exercises that belong to a template day.

### `template_sets`

**Attributes:** `template_set_id`, `template_exercise_id`, `set_number`, `target_reps`, `target_weight`
**What it’s for:** Stores the planned set targets for each exercise in a template.

### `workout_sessions`

**Attributes:** `session_id`, `user_id`, `performed_at`, `source_template_day_id`, `source_label`
**What it’s for:** Stores each completed workout session (the top-level history record).

### `session_exercises`

**Attributes:** `session_exercise_id`, `session_id`, `exercise_id`, `exercise_name_snapshot`, `sort_order`
**What it’s for:** Stores the ordered exercises performed in a specific session.

### `session_sets`

**Attributes:** `session_set_id`, `session_exercise_id`, `set_number`, `reps`, `weight`
**What it’s for:** Stores the actual performed sets for each session exercise.

### `planned_workouts`

**Attributes:** `planned_id`, `user_id`, `planned_for`, `template_day_id`, `title_snapshot`, `status`, `completed_session_id`, `original_planned_for`, `reschedule_count`, `created_at`
**What it’s for:** Stores scheduled workouts and their status, including reschedules and links to completed sessions.

### `streaks`

**Attributes:** `user_id`, `current_streak`, `best_streak`, `last_workout_date`, `updated_at`
**What it’s for:** Stores streak tracking state per user.

### `goals`

**Attributes:** `goal_id`, `user_id`, `exercise_id`, `goal_type`, `target_value`, `start_date`, `end_date`, `is_active`, `created_at`
**What it’s for:** Stores user-defined goals (optionally tied to an exercise) with targets and time windows.
