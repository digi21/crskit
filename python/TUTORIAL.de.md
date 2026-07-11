# CrsKit für Python — Tutorial

[English](TUTORIAL.md) · [Español](TUTORIAL.es.md) · [Français](TUTORIAL.fr.md) · [Italiano](TUTORIAL.it.md) · **Deutsch**

Das hier ist der ausführliche Einstieg. Er setzt voraus, dass du Python kannst und von Geodäsie
keine Ahnung hast: Jeder Begriff wird eingeführt, sobald du ihn zum ersten Mal brauchst, und jede
Ausgabe weiter unten stammt aus einem echten Lauf des Codes.

Wenn du nur die Referenz willst, ist das [README](README.md) kürzer. Wenn du lauffähige Fassungen
des Folgenden willst: Die liegen in [`examples/`](examples/).

---

## 1. Installation

```bash
pip install crskit[epsg]
```

Damit kommen zwei Pakete: `crskit`, die Bibliothek, und `crskit-epsg`, der EPSG-Katalog — die
Datenbank sämtlicher Koordinatenreferenzsysteme, auf deren Namen sich die Welt geeinigt hat. Der
Katalog sind Daten der IOGP, deshalb steckt er in einem eigenen Paket; und deshalb wird er auch nach
dem Datensatz versioniert und nicht nach der Bibliothek.

```python
import crskit

crskit.init()
print(crskit.epsg_version())     # 12.057
```

`init()` findet den Katalog von allein. Zeig woanders hin, wenn du eine eigene Kopie pflegst:

```python
crskit.init("/path/to/epsg.sqlite", data_directory="/path/to/grids")
```

Im `data_directory` liegen die *Gitterdateien*. Ignorier das vorerst — §6 erklärt, wann du eines
brauchst.

---

## 2. Was ein Koordinatenreferenzsystem eigentlich ist

Ein Zahlenpaar wie `(440291, 4474254)` bedeutet für sich genommen gar nichts. Es bedeutet erst
etwas, wenn du sagst, in **welchem System** die Zahlen stehen: welche Form die Erde haben soll, wo
ihr Mittelpunkt liegt und wie die gekrümmte Oberfläche in die Ebene abgewickelt wurde.

EPSG vergibt jedem dieser Systeme einen Code. Drei Sorten wirst du kennenlernen:

- **Geographisch** — Breite und Länge in Grad, auf einem Ellipsoid. `EPSG:4326` ist WGS 84, das
  System, das dein Handy meldet. `EPSG:4258` ist ETRS89, das europäische.
- **Projiziert** — Meter in der Ebene, nach einer Projektion. `EPSG:25830` ist ETRS89 / UTM Zone 30N
  und deckt Spanien ab.
- **Vertikal** — Höhen, und nur Höhen. `EPSG:5782` ist Alicante-Höhe, der spanische Meeresspiegel.

Frag die Bibliothek:

```python
utm30n = crskit.crs_from_epsg(25830)

utm30n.name             # 'ETRS89 / UTM zone 30N'
utm30n.authority_code   # 25830
utm30n.dimension        # 2
utm30n.axes             # [('E', 'East'), ('N', 'North')]
```

**Die letzte Zeile ist die, über die alle stolpern.** Ein CRS legt die *Reihenfolge* seiner
Koordinaten fest, und die ist nicht immer die, die du erwartest:

```python
>>> crskit.crs_from_epsg(4258).axes
[('Lat', 'North'), ('Lon', 'East')]
```

EPSG 4258 kommt *mit der Breite zuerst*. 4326 auch. CrsKit nimmt Koordinaten in genau der
Reihenfolge entgegen, die das System deklariert — fütterst du es mit der Länge zuerst, bekommst du
ein falsches Ergebnis, keinen Fehler, denn dein Punkt liegt dann eben schlicht woanders. Im Zweifel:
`.axes` ausgeben.

---

## 3. Deine erste Transformation

Eine **Transformation** führt von einem System in ein anderes. Einmal bauen, oft benutzen.

```python
etrs89 = crskit.crs_from_epsg(4258)      # Breite, Länge
utm30n = crskit.crs_from_epsg(25830)     # Rechtswert, Hochwert

transformation = crskit.transformation(etrs89, utm30n)

madrid = [40.416775, -3.703790]          # Breite, Länge — so wie 4258 es deklariert
transformation.transform(madrid)
# [440291.2843479216, 4474254.600145094]
```

Der Rückweg ist eine eigene Transformation, andersherum gebaut:

```python
crskit.transformation(utm30n, etrs89).transform([440291.284, 4474254.600])
# [40.41677499..., -3.70379000...]
```

Eine Transformation weiß, was sie ist:

```python
transformation.source_dimension   # 2
transformation.target_dimension   # 2
transformation.is_identity        # False
transformation.source             # <Crs EPSG:4258 "ETRS89" (2D)>
```

---

## 4. Well-Known Text: wie Systeme reisen

Eine `.prj`-Datei neben einem Shapefile, das `crs`-Feld eines GeoTIFF, der String in einer
Datenbankspalte — sie alle transportieren ein CRS als **Well-Known Text**. Es gibt mehrere Dialekte,
und CrsKit liest sie alle und erkennt am Text, um welchen es sich handelt:

```python
crs = crskit.crs_from_wkt(open("layer.prj").read())
```

Beim Schreiben hast du die Wahl des Dialekts:

```python
utm30n.to_wkt()                                # OGC WKT 1 — der Standard, am weitesten verstanden
utm30n.to_wkt(crskit.WktVersion.WKT1_ESRI)     # was ArcGIS in einer .prj erwartet
utm30n.to_wkt(crskit.WktVersion.WKT2_2019)     # ISO 19162:2019, der moderne
```

```
WKT1        PROJCS["ETRS89 / UTM zone 30N",GEOGCS["ETRS89",DATUM["European T...
WKT1_ESRI   PROJCS["ETRS_1989_UTM_Zone_30N",GEOGCS["GCS_ETRS_1989",DATUM["D_...
WKT2_2019   PROJCRS["ETRS89 / UTM zone 30N",BASEGEOGCRS["ETRS89",DATUM["Euro...
```

### Systeme vergleichen

Zwei Systeme sind gleich, wenn sie mathematisch dasselbe meinen — gleiches Datum, gleiche Projektion,
gleiche Parameter, gleiche Einheiten. Namen, Autoritäten und Achsenreihenfolge spielen dabei keine
Rolle:

```python
>>> crskit.crs_from_wkt(utm30n.to_wkt()) == utm30n
True
```

Hier leistet `==` echte Arbeit: Der Umweg über WKT verliert den EPSG-Code, und trotzdem sind die
Systeme gleich, weil sie *dasselbe* System sind. Wenn du wissen willst, *wie* nah sich zwei Systeme
sind, lass dir eine Punktzahl von 100 geben:

```python
>>> etrs89.compare(crskit.crs_from_epsg(4326))    # ETRS89 vs. WGS 84
40
```

Vierzig, nicht hundert: ETRS89 und WGS 84 sind heute fast dasselbe, aber eben nicht dasselbe System,
und die Bibliothek tut nicht so, als ob.

---

## 5. Wenn es mehr als eine richtige Antwort gibt

Hier hört Geodäsie auf, eine Nachschlagetabelle zu sein.

Der Wechsel zwischen zwei **Datums** — sagen wir vom alten europäischen ED50 zum modernen ETRS89 —
ist keine Formel, sondern eine Messung. Verschiedene Behörden haben ihn unterschiedlich gemessen, an
unterschiedlichen Orten, mit unterschiedlicher Genauigkeit. EPSG führt **zwölf** Transformationen von
ED50 nach ETRS89, und keine Bibliothek kann wissen, welche du willst.

Also weigert sich CrsKit zu raten:

```python
>>> crskit.transformation(crskit.crs_from_epsg(4230), crskit.crs_from_epsg(4258))
TransformationNotFoundError: Multiple transformations were located between the 4230 (ED50)
coordinate system and the 4258 (ETRS89) coordinate system.
```

Du entscheidest, über einen Callback, der die Kandidaten bekommt:

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

Lies diese Liste wie ein Geograph, nicht wie ein Programmierer: **Die kleinste Zahl ist nicht die
Antwort, der richtige Gültigkeitsbereich ist es.** Eine Operation, die für Norwegen gilt, liefert
klaglos auch Koordinaten für einen Punkt in Spanien — und die liegen dann um Hunderte Meter daneben.
Jeder Kandidat sagt dir, was du zum Entscheiden brauchst:

```python
operation.code                # 1632
operation.accuracy            # 1.5   (Meter)
operation.area_of_use         # 'Spain - mainland except northwest'
operation.grid_files          # []    — oder das Gitter, das sie braucht, siehe §6
operation.information_source  # wer sie veröffentlicht hat
```

Gib die gewünschte Operation zurück (oder einfach ihren Code), und du bekommst die Transformation:

```python
def for_mainland_spain(source_name, target_name, operations):
    spain = [o for o in operations if "Spain - mainland" in o.area_of_use and not o.grid_files]
    return min(spain, key=lambda o: o.accuracy)

transformation = crskit.transformation(ed50, etrs89, select_operation=for_mainland_spain)
transformation.transform([40.417659, -3.702557])
# [40.416478, -3.703862]     — rund 130 m entfernt: so sieht ein Datumswechsel aus
```

---

## 6. Höhen, Geoide und Gitterdateien

Dein GNSS-Empfänger meldet eine **ellipsoidische Höhe**: den Abstand zu einem mathematischen
Ellipsoid. Niemand meint das, wenn er "Höhe" sagt. Die Höhe auf einer Karte ist **orthometrisch**:
gemessen vom Geoid, jener Fläche, der der mittlere Meeresspiegel folgen würde. Der Unterschied
zwischen beiden — die *Geoidundulation* — beträgt über Madrid etwa +52 m und über dem Everest −28 m,
und er ändert sich von Ort zu Ort. Genau deshalb kommt er als **Gitterdatei** daher: als Geoidmodell.

Gitterdateien gehören den Behörden, die sie veröffentlichen, also liefert sie keine Bibliothek mit.
CrsKit sagt dir genau, welche es haben will:

```python
try:
    crskit.transformation(source, target)
except crskit.GridFileNotFoundError as error:
    error.grid_file           # 'EGM08_REDNAP.txt'
    error.operation_code      # 9410
    error.information_source  # 'National Geographic Institute of Spain (IGN)...'
    error.searched_path       # wo gesucht wurde
```

Lad sie herunter (das [README](README.md#where-to-get-the-grids) listet die üblichen auf), leg sie in
einen Ordner und gib diesen Ordner an `init(data_directory=...)`.

### Zwei Wege, nach einer Höhe zu fragen

**Ein zusammengesetztes CRS** koppelt ein horizontales System mit einem vertikalen. Für die meisten
Kombinationen hat EPSG keinen Code, also baust du es aus seinen beiden Teilen:

```python
etrs89_3d = crskit.crs_from_epsg(4937)                  # Breite, Länge, ellipsoidische Höhe
utm_msl = crskit.compound_crs(25830, 5782)              # UTM 30N + Alicante-Höhe

crskit.transformation(etrs89_3d, utm_msl).transform([40.416775, -3.703790, 700.0])
# [440291.284, 4474254.600, 648.888]
```

700 m über dem Ellipsoid sind 648.888 m über dem Meer: Das Geoid liegt dort 51.112 m höher.

**Oder du transformierst direkt in ein vertikales CRS**, wenn dich nur die Höhe interessiert. Der
Punkt kommt mit unveränderten Horizontalkoordinaten zurück, umgerechnet wird allein die Höhe:

```python
to_egm2008 = crskit.transformation(
    crskit.crs_from_epsg(4979),                          # WGS 84 3D
    crskit.crs_from_epsg(3855),                          # EGM2008-Höhe — das globale Geoid
    select_operation=lambda source, target, ops: ops[0])

to_egm2008.transform([27.988056, 86.925278, 8820.43])    # Everest, ellipsoidisch
# [27.988056, 86.925278, 8848.86]
```

Die letzte Zahl ist die Höhe des Everest, wie sie jeder kennt — und sie ist aus einer ellipsoidischen
Höhe herausgefallen, die 28 m niedriger liegt. Wenn du eine einzige Probe willst, ob der ganze Stapel
funktioniert: Das ist sie.

---

## 7. Viele Punkte auf einmal

`transform()` wechselt pro Punkt einmal von Python nach C++. Für eine Punktwolke übergibst du besser
gleich den ganzen Puffer: `transform_points()` nimmt ein NumPy-Array der Form `(n, dimensions)`, gibt
das GIL frei und liefert ein Array zurück.

```python
import numpy as np

points = np.column_stack([latitudes, longitudes])        # (1_000_000, 2)
projected = transformation.transform_points(points)      # (1_000_000, 2)
```

```
transform_points: 1,000,000 points in 0.394 s
transform()     : the same, one point at a time, would take about 2.2 s
```

Fünf- bis sechsmal schneller, und der Abstand wächst mit der Punktzahl. NumPy ist optional —
`transform()` funktioniert mit gewöhnlichen Listen, und das Modul lässt sich auch ohne installiertes
NumPy importieren.

---

## 8. Wenn etwas schiefgeht

Jeder Fehler, den die Bibliothek wirft, stammt von `crskit.CrsError` ab, ein einziges `except` fängt
also alles. Die speziellen Typen gibt es für die Fälle, in denen du tatsächlich etwas tun kannst:

| | |
|---|---|
| `AuthorityCodeNotFoundError` | diesen EPSG-Code gibt es nicht |
| `WktParseError` | der WKT ist fehlerhaft |
| `TransformationNotFoundError` | keine Operation zwischen diesen Systemen — oder mehrere, und du hast nicht gewählt (§5) |
| `GridFileNotFoundError` | eine Gitterdatei fehlt; die Ausnahme sagt, welche (§6) |
| `CoordinateOutsideDomainError` | der Punkt liegt außerhalb des Gitters oder des Gültigkeitsbereichs der Projektion |
| `DimensionMismatchError` | du hast einer 3D-Transformation einen 2D-Punkt gegeben, oder umgekehrt |
| `UnsupportedFormatError` | die CRS-Art oder die Operationsmethode wird nicht unterstützt |

`CoordinateOutsideDomainError` verdient eine Bemerkung. Meist heißt er, was er sagt — der Punkt liegt
außerhalb des Gebiets, das das Gitter abdeckt — aber genau ihn bekommst du auch, wenn du Koordinaten
in der falschen Achsenreihenfolge fütterst, denn der vertauschte Punkt landet im Meer. Wenn eine
Transformation, die eigentlich funktionieren müsste, einen Punkt außerhalb ihres Definitionsbereichs
meldet: erst `.axes` prüfen, dann alles andere.

---

## 9. Innerhalb einer Anwendung, die Python einbettet

Wenn du Skripte in einer Anwendung ausführst, die CrsKit bereits nutzt, bindet sich das Modul an die
Bibliothek, die die Anwendung geladen hat, und teilt deren Zustand — ihren EPSG-Katalog, ihre
Einstellungen. Da gibt es nichts zu initialisieren:

```python
if not crskit.is_initialized():
    crskit.init()
```

---

## Wie es weitergeht

- [`examples/`](examples/) — die lauffähigen Fassungen von allem oben.
- [README](README.md) — die Referenz, und wo du die Gitterdateien herunterlädst.
- [Die C++-Bibliothek](https://github.com/digi21/crskit) — dieses Binding ist nur eine dünne Schicht
  darüber, und in der C++-API findest du alles, was hier nicht offengelegt ist.
