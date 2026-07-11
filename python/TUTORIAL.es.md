# CrsKit para Python — tutorial

[English](TUTORIAL.md) · **Español** · [Français](TUTORIAL.fr.md) · [Italiano](TUTORIAL.it.md) · [Deutsch](TUTORIAL.de.md)

Este es el camino largo. Damos por hecho que sabes Python y nada de geodesia: cada idea se presenta
cuando la necesitas por primera vez, y todas las salidas que verás abajo se obtuvieron ejecutando el
código.

Si solo quieres la referencia, el [README](README.md) es más corto. Si quieres versiones ejecutables
de lo que viene a continuación, están en [`examples/`](examples/).

---

## 1. Instalación

```bash
pip install crskit[epsg]
```

Llegan dos paquetes: `crskit`, la librería, y `crskit-epsg`, el catálogo EPSG — la base de datos de
todos los sistemas de referencia de coordenadas que el mundo ha acordado nombrar. El catálogo son
datos propiedad de la IOGP, así que vive en un paquete aparte; por eso también se versiona según el
conjunto de datos y no según la librería.

```python
import crskit

crskit.init()
print(crskit.epsg_version())     # 12.057
```

`init()` encuentra el catálogo por su cuenta. Apúntalo a otro sitio si mantienes tu propia copia:

```python
crskit.init("/path/to/epsg.sqlite", data_directory="/path/to/grids")
```

El `data_directory` es donde viven los *ficheros de rejilla*. Ignóralo de momento — el §6 explica
cuándo necesitas uno.

---

## 2. Qué es en realidad un sistema de referencia de coordenadas

Un par de números como `(440291, 4474254)` no significa nada por sí solo. Significa algo únicamente
cuando dices **en qué sistema** están: qué forma se supone que tiene la Tierra, dónde está su centro
y cómo se aplanó la superficie curva sobre un plano.

EPSG asigna un código a cada uno de esos sistemas. Te encontrarás con tres tipos:

- **Geográficos** — latitud y longitud en grados, sobre un elipsoide. `EPSG:4326` es WGS 84, el que
  te da el móvil. `EPSG:4258` es ETRS89, el europeo.
- **Proyectados** — metros sobre un plano, tras una proyección. `EPSG:25830` es ETRS89 / UTM zona 30N,
  que cubre España.
- **Verticales** — alturas, y nada más que alturas. `EPSG:5782` es la altura de Alicante, el nivel del
  mar español.

Pregúntale a la librería:

```python
utm30n = crskit.crs_from_epsg(25830)

utm30n.name             # 'ETRS89 / UTM zone 30N'
utm30n.authority_code   # 25830
utm30n.dimension        # 2
utm30n.axes             # [('E', 'East'), ('N', 'North')]
```

**Esa última línea es la que pilla a todo el mundo.** Un CRS declara el *orden* de sus coordenadas, y
ese orden no siempre es el que das por supuesto:

```python
>>> crskit.crs_from_epsg(4258).axes
[('Lat', 'North'), ('Lon', 'East')]
```

En EPSG 4258 va *primero la latitud*. Y en 4326 también. CrsKit toma las coordenadas en el orden que
declara el sistema — si le pasas primero la longitud, obtendrás una respuesta equivocada, no un error,
porque tu punto sencillamente está en otro sitio. Ante la duda, imprime `.axes`.

---

## 3. Tu primera transformación

Una **transformación** va de un sistema a otro. Constrúyela una vez y úsala muchas.

```python
etrs89 = crskit.crs_from_epsg(4258)      # latitud, longitud
utm30n = crskit.crs_from_epsg(25830)     # X/Este, Y/Norte

transformation = crskit.transformation(etrs89, utm30n)

madrid = [40.416775, -3.703790]          # latitud, longitud — como declara 4258
transformation.transform(madrid)
# [440291.2843479216, 4474254.600145094]
```

La vuelta es otra transformación, construida al revés:

```python
crskit.transformation(utm30n, etrs89).transform([440291.284, 4474254.600])
# [40.41677499..., -3.70379000...]
```

Una transformación sabe lo que es:

```python
transformation.source_dimension   # 2
transformation.target_dimension   # 2
transformation.is_identity        # False
transformation.source             # <Crs EPSG:4258 "ETRS89" (2D)>
```

---

## 4. Well-Known Text: cómo viajan los sistemas

Un fichero `.prj` junto a un shapefile, el campo `crs` de un GeoTIFF, la cadena en una columna de una
base de datos — todos ellos llevan un CRS en forma de **Well-Known Text**. Hay varios dialectos, y
CrsKit los lee todos, detectando cuál es a partir del propio texto:

```python
crs = crskit.crs_from_wkt(open("layer.prj").read())
```

Al escribir, eliges dialecto:

```python
utm30n.to_wkt()                                # OGC WKT 1 — el de por defecto, el más comprendido
utm30n.to_wkt(crskit.WktVersion.WKT1_ESRI)     # lo que ArcGIS espera en un .prj
utm30n.to_wkt(crskit.WktVersion.WKT2_2019)     # ISO 19162:2019, el moderno
```

```
WKT1        PROJCS["ETRS89 / UTM zone 30N",GEOGCS["ETRS89",DATUM["European T...
WKT1_ESRI   PROJCS["ETRS_1989_UTM_Zone_30N",GEOGCS["GCS_ETRS_1989",DATUM["D_...
WKT2_2019   PROJCRS["ETRS89 / UTM zone 30N",BASEGEOGCRS["ETRS89",DATUM["Euro...
```

### Comparar sistemas

Dos sistemas son iguales cuando significan lo mismo matemáticamente — mismo datum, misma proyección,
mismos parámetros, mismas unidades. Los nombres, las autoridades y el orden de ejes no entran en la
ecuación:

```python
>>> crskit.crs_from_wkt(utm30n.to_wkt()) == utm30n
True
```

Ahí el `==` está trabajando de verdad: la ida y vuelta por WKT pierde el código EPSG, y los sistemas
siguen comparando iguales porque *son* el mismo sistema. Cuando quieras saber *cuánto* se parecen dos
sistemas, pide una puntuación sobre 100:

```python
>>> etrs89.compare(crskit.crs_from_epsg(4326))    # ETRS89 frente a WGS 84
40
```

Cuarenta, no cien: ETRS89 y WGS 84 son hoy casi lo mismo, pero no son el mismo sistema, y la librería
no va a fingir lo contrario.

---

## 5. Cuando hay más de una respuesta correcta

Aquí es donde la geodesia deja de ser una tabla de consulta.

Pasar de un **datum** a otro — digamos del viejo ED50 europeo al moderno ETRS89 — no es una fórmula,
es una medición. Distintos organismos la midieron de distinta manera, en distintos lugares y con
distintas precisiones. EPSG registra **doce** transformaciones de ED50 a ETRS89, y no hay forma de
que una librería sepa cuál quieres tú.

Así que CrsKit se niega a adivinar:

```python
>>> crskit.transformation(crskit.crs_from_epsg(4230), crskit.crs_from_epsg(4258))
TransformationNotFoundError: Multiple transformations were located between the 4230 (ED50)
coordinate system and the 4258 (ETRS89) coordinate system.
```

Eliges tú, con un callback que recibe las candidatas:

```python
def choose(source_name, target_name, operations):
    for operation in sorted(operations, key=lambda o: o.accuracy or 99):
        print(f"EPSG:{operation.code}  {operation.accuracy} m  {operation.area_of_use}")
    ...

crskit.transformation(ed50, etrs89, select_operation=choose)
```

```
EPSG:15932   0.2 m  Spain - mainland and Balearic Islands onshore   (needs SPED2ETV2.gsb)
EPSG:1588    1.0 m  Norway - offshore north of 65°N; Svalbard
EPSG:1628    1.0 m  Gibraltar
EPSG:1630    1.5 m  Spain - Balearic Islands
EPSG:1632    1.5 m  Spain - mainland except northwest
EPSG:1634    1.5 m  Spain - mainland northwest
EPSG:1650    2.0 m  France
EPSG:1783    2.0 m  Turkey
EPSG:5040    5.0 m  Portugal - mainland - onshore
...
```

Lee esa lista como un geógrafo, no como un programador: **la respuesta no es el número más pequeño,
es el ámbito de aplicación correcto.** Una operación válida en Noruega te dará tan campante
coordenadas para un punto en España, y estarán mal por cientos de metros. Cada candidata te dice lo
que necesitas para decidir:

```python
operation.code                # 1632
operation.accuracy            # 1.5   (metros)
operation.area_of_use         # 'Spain - mainland except northwest'
operation.grid_files          # []    — o la rejilla que necesita, véase §6
operation.information_source  # quién la publicó
```

Devuelve la operación que quieras (o simplemente su código), y obtienes la transformación:

```python
def for_mainland_spain(source_name, target_name, operations):
    spain = [o for o in operations if "Spain - mainland" in o.area_of_use and not o.grid_files]
    return min(spain, key=lambda o: o.accuracy)

transformation = crskit.transformation(ed50, etrs89, select_operation=for_mainland_spain)
transformation.transform([40.417659, -3.702557])
# [40.416478, -3.703862]     — unos 130 m de diferencia: así se ve un cambio de datum
```

---

## 6. Alturas, geoides y ficheros de rejilla

Tu receptor GNSS te da una **altura elipsoidal**: la distancia hasta un elipsoide matemático. Nadie
entiende eso por «altitud». La altura de un mapa es **ortométrica**: se mide desde el geoide, la
superficie que seguiría el nivel medio del mar. La diferencia entre ambas — la *ondulación* — es de
unos +52 m sobre Madrid y de −28 m sobre el Everest, y varía de un sitio a otro, y por eso viene en
forma de **fichero de rejilla**: un modelo de geoide.

Las rejillas pertenecen a los organismos que las publican, así que ninguna librería las distribuye.
CrsKit te dice exactamente cuál necesita:

```python
try:
    crskit.transformation(source, target)
except crskit.GridFileNotFoundError as error:
    error.grid_file           # 'EGM08_REDNAP.txt'
    error.operation_code      # 9410
    error.information_source  # 'National Geographic Institute of Spain (IGN)...'
    error.searched_path       # dónde la ha buscado
```

Descárgala (el [README](README.md#where-to-get-the-grids) enumera las habituales), déjala en una
carpeta y pásale esa carpeta a `init(data_directory=...)`.

### Dos formas de pedir una altura

**Un CRS compuesto** empareja un sistema horizontal con uno vertical. EPSG no tiene código para la
mayoría de las combinaciones, así que lo construyes a partir de sus dos partes:

```python
etrs89_3d = crskit.crs_from_epsg(4937)                  # latitud, longitud, altura elipsoidal
utm_msl = crskit.compound_crs(25830, 5782)              # UTM 30N + altura de Alicante

crskit.transformation(etrs89_3d, utm_msl).transform([40.416775, -3.703790, 700.0])
# [440291.284, 4474254.600, 648.888]
```

700 m sobre el elipsoide son 648,888 m sobre el mar: allí el geoide está 51,112 m por encima.

**O transforma directamente a un CRS vertical**, cuando lo único que te importa es la altura. El punto
vuelve con sus coordenadas horizontales intactas y solo la altura convertida:

```python
to_egm2008 = crskit.transformation(
    crskit.crs_from_epsg(4979),                          # WGS 84 3D
    crskit.crs_from_epsg(3855),                          # EGM2008 height — el geoide global
    select_operation=lambda source, target, ops: ops[0])

to_egm2008.transform([27.988056, 86.925278, 8820.43])    # Everest, elipsoidal
# [27.988056, 86.925278, 8848.86]
```

Ese último número es la altura del Everest tal y como todo el mundo la conoce, y ha salido de una
altura elipsoidal 28 m menor. Si quieres una única comprobación de que todo el engranaje funciona, es
esa.

---

## 7. Muchos puntos a la vez

`transform()` cruza de Python a C++ una vez por punto. Para una nube de puntos, entrégale el búfer
entero: `transform_points()` toma un array de NumPy de forma `(n, dimensions)`, libera el GIL y
devuelve otro array.

```python
import numpy as np

points = np.column_stack([latitudes, longitudes])        # (1_000_000, 2)
projected = transformation.transform_points(points)      # (1_000_000, 2)
```

```
transform_points: 1,000,000 points in 0.394 s
transform()     : the same, one point at a time, would take about 2.2 s
```

Cinco o seis veces más rápido, y la brecha se ensancha según crece el número de puntos. NumPy es
opcional — `transform()` funciona con listas normales, y el módulo se importa sin NumPy instalado.

---

## 8. Cuando algo va mal

Todos los errores que lanza la librería descienden de `crskit.CrsError`, así que un solo `except` los
atrapa todos. Los tipos concretos están ahí para cuando puedes hacer algo al respecto:

| | |
|---|---|
| `AuthorityCodeNotFoundError` | no existe ese código EPSG |
| `WktParseError` | el WKT está mal formado |
| `TransformationNotFoundError` | no hay operación entre esos sistemas — o hay varias y no elegiste (§5) |
| `GridFileNotFoundError` | falta un fichero de rejilla; la excepción dice cuál (§6) |
| `CoordinateOutsideDomainError` | el punto está fuera de la rejilla o de la zona de validez de la proyección |
| `DimensionMismatchError` | le has dado un punto 2D a una transformación 3D, o al revés |
| `UnsupportedFormatError` | el tipo de CRS o el método de la operación no están soportados |

`CoordinateOutsideDomainError` merece un comentario. Normalmente significa lo que dice — el punto está
fuera del área que cubre la rejilla —, pero es también lo que obtienes cuando pasas las coordenadas en
el orden de ejes equivocado, porque el punto traspuesto aterriza en el mar. Si una transformación que
debería funcionar te dice que el punto está fuera de su dominio, comprueba `.axes` antes que ninguna
otra cosa.

---

## 9. Dentro de una aplicación que embebe Python

Si ejecutas scripts dentro de una aplicación que ya usa CrsKit, el módulo se enlaza con la librería
que la aplicación tiene cargada y comparte su estado — su catálogo EPSG, su configuración. No hay nada
que inicializar:

```python
if not crskit.is_initialized():
    crskit.init()
```

---

## Por dónde seguir

- [`examples/`](examples/) — las versiones ejecutables de todo lo anterior.
- [README](README.md) — la referencia, y de dónde descargar los ficheros de rejilla.
- [La librería C++](https://github.com/digi21/crskit) — este binding es una capa fina sobre ella, y la
  API de C++ es donde encontrarás todo lo que aquí no se expone.
