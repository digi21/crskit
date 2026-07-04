#!/usr/bin/env python3
"""Genera un SQLite FIEL al EPSG Dataset a partir de los scripts SQL (sabor MySQL)
que publica EPSG en https://epsg.org/download-dataset.html.

Multiplataforma y sin dependencias externas (solo la stdlib de Python): no necesita
MS Access, ODBC ni mdbtools.

El SQLite resultante conserva el esquema oficial de EPSG (tablas `epsg_*`, nombres de
columna, booleanos 1/0, valores de OBJECT_TABLE_NAME `epsg_*`, etc.); NO se renombra ni
se remapea nada. El esquema se deriva del propio `MySQL_Table_Script.sql` (conversion de
dialecto MySQL->SQLite), por lo que se adapta solo a cambios entre versiones de EPSG.
La libreria (Epsg::SQliteProvider) consulta directamente este esquema oficial.

Uso:
    python epsg_sql_to_sqlite.py D:/epsg/MySQL_Data_Script.sql -o D:/epsg/epsg.sqlite
    # (busca MySQL_Table_Script.sql junto al de datos; o indicalo con --table-script)
"""

import argparse
import os
import re
import sqlite3
import sys


# ---------------------------------------------------------------------------
# 1) Conversion del DDL de EPSG (MySQL) -> CREATE TABLE de SQLite.
# ---------------------------------------------------------------------------
def _sqlite_type(mysql_type: str) -> tuple:
    """Devuelve (tipo_sqlite, es_texto) para un tipo de columna MySQL de EPSG."""
    t = mysql_type.upper()
    if t.startswith(("INTEGER", "SMALLINT", "BIGINT", "INT")):
        return "INTEGER", False
    if t.startswith(("FLOAT", "DOUBLE", "REAL", "DECIMAL", "NUMERIC")):
        return "REAL", False
    # VARCHAR, CHAR, TEXT, DATE, TIMESTAMP, ... -> TEXT (con COLLATE NOCASE).
    return "TEXT", True


_CREATE_RE = re.compile(r"CREATE\s+TABLE\s+(\w+)\s*\((.*)\)\s*$", re.IGNORECASE | re.DOTALL)
_PK_RE = re.compile(r"PRIMARY\s+KEY\s*\(([^)]*)\)", re.IGNORECASE)


def _split_top_level(s: str):
    """Trocea por comas de nivel 0 respetando parentesis y cadenas."""
    parts, depth, i, n, start = [], 0, 0, len(s), 0
    while i < n:
        c = s[i]
        if c == "'":
            i += 1
            while i < n:
                if s[i] == "\\":
                    i += 2
                elif s[i] == "'":
                    i += 1
                    if i < n and s[i] == "'":
                        i += 1; continue
                    break
                else:
                    i += 1
        elif c == "(":
            depth += 1; i += 1
        elif c == ")":
            depth -= 1; i += 1
        elif c == "," and depth == 0:
            parts.append(s[start:i]); start = i + 1; i += 1
        else:
            i += 1
    parts.append(s[start:])
    return parts


def convert_ddl(text: str) -> str:
    """Convierte el MySQL_Table_Script.sql de EPSG en CREATE TABLE validos para SQLite."""
    out = []
    for stmt in iter_statements(text):
        m = _CREATE_RE.match(stmt.strip())
        if not m:
            continue
        table, body = m.group(1), m.group(2)
        cols, pk = [], None
        for item in _split_top_level(body):
            item = item.strip()
            if not item:
                continue
            up = item.upper()
            if up.startswith("CONSTRAINT") or up.startswith("PRIMARY KEY"):
                mk = _PK_RE.search(item)
                if mk:
                    pk = [c.strip().strip('`"') for c in mk.group(1).split(",")]
                continue  # se ignoran UNIQUE/FK a nivel de tabla
            toks = item.split()
            name = toks[0].strip('`"')
            mysql_type = toks[1] if len(toks) > 1 else "VARCHAR"
            sqlt, is_text = _sqlite_type(mysql_type)
            notnull = " NOT NULL" in (" " + up + " ")
            coldef = f'  "{name}" {sqlt}{" COLLATE NOCASE" if is_text else ""}{" NOT NULL" if notnull else ""}'
            cols.append(coldef)
        if pk:
            cols.append("  PRIMARY KEY (" + ", ".join(f'"{c}"' for c in pk) + ")")
        out.append(f'CREATE TABLE "{table}" (\n' + ",\n".join(cols) + "\n);")
    return "\n\n".join(out)


# ---------------------------------------------------------------------------
# 2) Carga de datos: parsea cada INSERT y lo inserta con binding parametrizado.
# ---------------------------------------------------------------------------
_MYSQL_ESCAPES_PY = {"'": "'", "\\": "\\", '"': '"', "n": "\n", "r": "\r",
                     "t": "\t", "b": "\b", "0": "\x00", "Z": "\x1a"}


def _unescape_mysql(body: str) -> str:
    out, i, n = [], 0, len(body)
    while i < n:
        c = body[i]
        if c == "\\" and i + 1 < n:
            out.append(_MYSQL_ESCAPES_PY.get(body[i + 1], body[i + 1])); i += 2
        elif c == "'" and i + 1 < n and body[i + 1] == "'":
            out.append("'"); i += 2
        else:
            out.append(c); i += 1
    return "".join(out)


def iter_statements(text: str):
    """Itera sentencias SQL completas (';' a nivel 0), respetando cadenas, backticks y
    comentarios -- / # / bloque."""
    i, n, start = 0, len(text), 0
    while i < n:
        c = text[i]
        if c == "'":
            i += 1
            while i < n:
                if text[i] == "\\":
                    i += 2
                elif text[i] == "'":
                    i += 1; break
                else:
                    i += 1
        elif c == "`":
            j = text.find("`", i + 1); i = n if j == -1 else j + 1
        elif c == "/" and i + 1 < n and text[i + 1] == "*":
            j = text.find("*/", i + 2); i = n if j == -1 else j + 2
        elif c == "#" or (c == "-" and i + 1 < n and text[i + 1] == "-"):
            j = text.find("\n", i); i = n if j == -1 else j + 1
        elif c == ";":
            s = text[start:i].strip()
            if s:
                yield s
            i += 1; start = i
        else:
            i += 1
    tail = text[start:].strip()
    if tail:
        yield tail


_INSERT_RE = re.compile(r"^INSERT\s+(?:IGNORE\s+)?INTO\b", re.IGNORECASE)
_INSERT_FULL_RE = re.compile(
    r"^INSERT\s+(?:IGNORE\s+)?INTO\s+(`[^`]+`|\"[^\"]+\"|\[[^\]]+\]|\w+)\s*"
    r"\(([^)]*)\)\s+VALUES\s*(.*)$", re.IGNORECASE | re.DOTALL)


def _parse_value(tok: str):
    t = tok.strip()
    if t.lower() == "null" or t == "":
        return None
    if t[0] == "'":
        return _unescape_mysql(t[1:-1])
    try:
        return int(t)
    except ValueError:
        try:
            return float(t)
        except ValueError:
            return t


def _iter_tuples(values_part: str):
    i, n = 0, len(values_part)
    while i < n:
        if values_part[i] == "(":
            depth, j = 1, i + 1
            while j < n and depth:
                c = values_part[j]
                if c == "'":
                    j += 1
                    while j < n:
                        if values_part[j] == "\\":
                            j += 2
                        elif values_part[j] == "'":
                            j += 1; break
                        else:
                            j += 1
                    continue
                if c == "(":
                    depth += 1
                elif c == ")":
                    depth -= 1
                j += 1
            yield values_part[i + 1:j - 1]
            i = j
        else:
            i += 1


def parse_insert(stmt: str):
    m = _INSERT_FULL_RE.match(stmt)
    if not m:
        return None
    table = m.group(1).strip('`"[]')
    cols = [c.strip().strip('`"[]') for c in m.group(2).split(",")]
    rows = [[_parse_value(v) for v in _split_top_level(t)]
            for t in _iter_tuples(m.group(3))]
    return table, cols, rows


# ---------------------------------------------------------------------------
def build(data_path, table_script_path, out_path):
    with open(table_script_path, encoding="utf-8-sig", errors="replace") as f:
        schema_sql = convert_ddl(f.read())
    if os.path.exists(out_path):
        os.remove(out_path)

    con = sqlite3.connect(out_path)
    con.executescript(schema_sql)
    known = {r[0]: {c[1].upper(): c[1] for c in con.execute(f'PRAGMA table_info("{r[0]}")')}
             for r in con.execute("SELECT name FROM sqlite_master WHERE type='table'")}

    with open(data_path, encoding="utf-8-sig", errors="replace") as f:
        text = f.read()

    con.execute("PRAGMA foreign_keys=OFF")
    con.execute("BEGIN")
    counts, skipped, unknown, errors = {}, 0, {}, []
    for stmt in iter_statements(text):
        if not _INSERT_RE.match(stmt):
            skipped += 1
            continue
        parsed = parse_insert(stmt)
        if parsed is None:
            if len(errors) < 50:
                errors.append(f"INSERT no parseable: {stmt[:120]}")
            continue
        table, cols, rows = parsed
        if table not in known:
            unknown[table] = unknown.get(table, 0) + 1
            continue
        collist = ", ".join('"%s"' % c for c in cols)
        ph = ", ".join("?" * len(cols))
        sql = f'INSERT INTO "{table}" ({collist}) VALUES ({ph})'
        for row in rows:
            try:
                con.execute(sql, row)
                counts[table] = counts.get(table, 0) + 1
            except sqlite3.Error as e:
                if len(errors) < 50:
                    errors.append(f"{table}: {e}")
    con.execute("COMMIT")
    con.commit()

    total = sum(counts.values())
    print(f"SQLite FIEL generado: {total} filas en {len(counts)} tablas "
          f"(no-INSERT ignorados: {skipped}).")
    if unknown:
        print("AVISO: INSERT a tablas no presentes en el DDL: "
              + ", ".join(f"{k} (x{v})" for k, v in sorted(unknown.items())), file=sys.stderr)
    if errors:
        print(f"AVISO: {len([e for e in errors if e])} INSERT con error:", file=sys.stderr)
        for e in [x for x in errors if x][:10]:
            print("   " + e, file=sys.stderr)
    con.close()
    return counts


def main(argv=None):
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("data", help="Script de datos de EPSG (MySQL_Data_Script.sql).")
    p.add_argument("-o", "--output", default="epsg.sqlite", help="SQLite de salida.")
    p.add_argument("--table-script", help="DDL de EPSG (MySQL_Table_Script.sql). "
                   "Por defecto se busca junto al script de datos.")
    a = p.parse_args(argv)

    ts = a.table_script
    if not ts:
        cand = os.path.join(os.path.dirname(os.path.abspath(a.data)), "MySQL_Table_Script.sql")
        if not os.path.exists(cand):
            p.error("No encuentro MySQL_Table_Script.sql junto al de datos; usa --table-script.")
        ts = cand

    build(a.data, ts, a.output)
    return 0


if __name__ == "__main__":
    sys.exit(main())
