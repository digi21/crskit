# EPSG → SQLite converter

Builds a SQLite database **faithful to the official EPSG Dataset** from the **SQL scripts (MySQL
flavour)** that EPSG publishes. The resulting schema is EPSG's own (`epsg_*` tables, original column
names, `1`/`0` booleans, native values) — nothing is renamed or remapped.

CrsKit's `SqliteProvider` queries this official schema directly.

> **Don't need to build it?** A prebuilt copy is attached to the
> [`epsg-data`](https://github.com/digi21/crskit/releases/tag/epsg-data) release
> (`epsg-vX.Y.sqlite`, named after the EPSG dataset version). Build it yourself only when you want a
> newer EPSG version than the one published there.

## Why this tool

EPSG distributes the dataset as **MS Access** and as **SQL scripts** (MySQL/PostgreSQL/Oracle), but
**not** as SQLite. To keep the project cross-platform, the SQLite is generated from the SQL scripts,
with no dependency on MS Access, ODBC or `mdbtools`.

## Requirements

- Python 3 (standard library only). Works on Windows, Linux and macOS.

## Getting the EPSG SQL scripts

Download the dataset in **SQL (MySQL)** format from <https://epsg.org/download-dataset.html> (free
registration required). The ZIP contains:

- `MySQL_Table_Script.sql` — DDL (used to derive the SQLite schema).
- `MySQL_Data_Script.sql` — the `INSERT`s (the data).
- `MySQL_FKey_Script.sql` — foreign keys (not used).

## Usage

```sh
python epsg_sql_to_sqlite.py path/to/MySQL_Data_Script.sql -o epsg.sqlite
# The table script is looked up next to the data script; override it with --table-script PATH.
```

Then point the tests (or your application) at the result with the `DIGI21_EPSG_SQLITE` environment
variable, or pass `-DDIGI21_EPSG_SQLITE=/path/to/epsg.sqlite` to CMake.

**Tip:** name the file after its EPSG version (e.g. `epsg-v12.057.sqlite`) — the version lives in the
`epsg_versionhistory` table, so the exact dataset is reproducible.

## What it does

1. **Schema**: converts the DDL in `MySQL_Table_Script.sql` to SQLite `CREATE TABLE` (MySQL→SQLite
   types; `COLLATE NOCASE` on text columns to preserve the case-insensitive comparisons the queries
   rely on; keeps the `PRIMARY KEY`s). Because it is derived from the DDL, it **adapts automatically**
   to new columns/tables between EPSG versions.
2. **Data**: parses each `INSERT` in `MySQL_Data_Script.sql` and inserts it with parameterised binding,
   handling MySQL escaping (`\'`, `\\`, …), `Null`, and `VALUES(` with or without a space. No value is
   altered (`1`/`0` booleans, `OBJECT_TABLE_NAME` values `epsg_*`, etc.).

The only additions over the official dataset are `COLLATE NOCASE` in the schema (a collation — it does
not change the data) and the omission of the foreign keys.

## Validation

Generated from a real EPSG `MySQL_Data_Script.sql` and exercised by the full GIGS test suite (all the
`Epsg` / `WktEpsg` variants and the by-operation-code transformations) — green.
