#!/usr/bin/env bash
set -euo pipefail

mkdir -p data
DB_PATH="data/volumelab.db"

rm -f "$DB_PATH"
sqlite3 "$DB_PATH" < db/schema.sql

echo "Created: $DB_PATH"
echo "Tip: your app must run: PRAGMA foreign_keys = ON; after opening the connection."
