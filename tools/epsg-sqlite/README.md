# Conversor EPSG → SQLite (fiel)

Genera un **SQLite fiel al EPSG Dataset oficial** a partir de los **scripts SQL (sabor MySQL)**
que publica EPSG. El esquema resultante es el de EPSG (tablas `epsg_*`, nombres de columna,
booleanos `1`/`0`, valores nativos), sin renombrar ni remapear nada.

Motivación: EPSG publica el dataset en **MS Access** y en **scripts SQL** (MySQL/PostgreSQL/Oracle),
pero **no** en SQLite. Para mantener el proyecto multiplataforma se genera el SQLite a partir de los
scripts SQL, sin depender de MS Access, ODBC ni `mdbtools`. La librería
(`Digi21::OpenGis::Epsg::SQliteProvider`) consulta directamente este esquema oficial.

## Requisitos

- Python 3 (solo biblioteca estándar). Funciona en Windows/Linux/macOS.

## De dónde sacar el script SQL de EPSG

1. Descarga el dataset en formato **SQL (MySQL)** desde
   <https://epsg.org/download-dataset.html> (requiere registro gratuito). El ZIP trae:
   - `MySQL_Table_Script.sql` — DDL (se usa para derivar el esquema del SQLite).
   - `MySQL_Data_Script.sql` — los `INSERT` (los datos).
   - `MySQL_FKey_Script.sql` — claves ajenas (no se usa).

## Uso

```sh
python epsg_sql_to_sqlite.py D:/epsg/MySQL_Data_Script.sql -o D:/epsg/epsg.sqlite
# busca MySQL_Table_Script.sql junto al de datos; o indícalo con --table-script RUTA
```

Copia el `epsg.sqlite` resultante a la ubicación que use la aplicación
(`C:/ProgramData/Digi3D.NET/OpenGis/…`, o la indicada por la variable de entorno
`DIGI21_EPSG_SQLITE`).

## Qué hace

1. **Esquema**: convierte el DDL de `MySQL_Table_Script.sql` a `CREATE TABLE` de SQLite
   (tipos MySQL→SQLite; `COLLATE NOCASE` en columnas de texto para conservar las comparaciones
   insensibles a mayúsculas que hacen las consultas; conserva las `PRIMARY KEY`). Al derivarse del
   propio DDL, **se adapta solo** a columnas/tablas nuevas entre versiones de EPSG.
2. **Datos**: parsea cada `INSERT` de `MySQL_Data_Script.sql` y lo inserta con *binding*
   parametrizado, manejando el escapado MySQL (`\'`, `\\`…), `Null`, y `VALUES(` con o sin espacio.
   No se altera ningún valor (booleanos `1`/`0`, `OBJECT_TABLE_NAME` con valores `epsg_*`, etc.).

> Fidelidad: el SQLite es equivalente al EPSG oficial; lo único que se añade es `COLLATE NOCASE`
> en el esquema (una collation, no cambia los datos) y se omiten las claves ajenas.

## La librería consulta el esquema oficial

`SQliteProvider` consulta directamente las tablas `epsg_*` y las convenciones de EPSG
(`SHOW_OPERATION=1`, `OBJECT_TABLE_NAME='epsg_…'`, columna `coord_axis_order`…). `GetData()`
normaliza a MAYÚSCULAS el nombre de columna que reporta SQLite (que respeta el case del esquema),
porque el resto del provider lee las claves en MAYÚSCULAS.

(`AccessProvider`, que lee el `.mdb` con el esquema de Access, mantiene sus propias consultas.)

## Validación

- **Funcional con EPSG real (v12.x)**: generado el SQLite desde `MySQL_Data_Script.sql`
  (87 461 filas, 28 tablas) y ejecutada la batería GIGS con `DIGI21_EPSG_SQLITE` apuntando a él:
  **146/146 tests verdes** (todas las variantes `Epsg`/`WktEpsg` y las transformaciones por código
  de operación).

## Archivos

- `epsg_sql_to_sqlite.py` — el conversor (deriva esquema del DDL + carga datos con *binding*).
