# CrsKit pour Python — tutoriel

[English](TUTORIAL.md) · [Español](TUTORIAL.es.md) · **Français** · [Italiano](TUTORIAL.it.md) · [Deutsch](TUTORIAL.de.md)

Voici la voie longue. Elle suppose que vous connaissez Python et pas la moindre géodésie : chaque
notion est introduite au moment où vous en avez besoin, et toutes les sorties ci-dessous ont été
produites en exécutant le code.

Si vous ne cherchez que la référence, le [README](README.md) est plus court. Si vous voulez des
versions exécutables de ce qui suit, elles sont dans [`examples/`](examples/).

---

## 1. Installation

```bash
pip install crskit[epsg]
```

Deux paquets arrivent : `crskit`, la bibliothèque, et `crskit-epsg`, le catalogue EPSG — la base de
données de tous les systèmes de référence de coordonnées que le monde s'est accordé à nommer. Le
catalogue est une donnée qui appartient à l'IOGP ; il vit donc dans son propre paquet, et c'est aussi
pourquoi il est versionné d'après le jeu de données plutôt que d'après la bibliothèque.

```python
import crskit

crskit.init()
print(crskit.epsg_version())     # 12.057
```

`init()` trouve le catalogue tout seul. Indiquez-lui un autre emplacement si vous conservez votre
propre copie :

```python
crskit.init("/path/to/epsg.sqlite", data_directory="/path/to/grids")
```

Le `data_directory` est l'endroit où vivent les *fichiers de grille*. Ignorez-le pour l'instant — le
§6 explique quand vous en avez besoin.

---

## 2. Ce qu'est réellement un système de référence de coordonnées

Une paire de nombres comme `(440291, 4474254)` ne signifie rien en soi. Elle ne signifie quelque chose
qu'une fois que vous précisez **dans quel système** ces nombres sont exprimés : quelle forme on
suppose à la Terre, où se trouve son centre, et comment la surface courbe a été aplatie sur un plan.

EPSG attribue un code à chacun de ces systèmes. Vous en rencontrerez trois sortes :

- **Géographique** — latitude et longitude en degrés, sur un ellipsoïde. `EPSG:4326`, c'est le WGS 84,
  celui que votre téléphone affiche. `EPSG:4258`, c'est l'ETRS89, l'européen.
- **Projeté** — des mètres sur un plan, après projection. `EPSG:25830`, c'est ETRS89 / UTM zone 30N,
  qui couvre l'Espagne.
- **Vertical** — des hauteurs, et rien que des hauteurs. `EPSG:5782`, c'est le système d'altitudes
  d'Alicante, le niveau de la mer espagnol.

Demandez à la bibliothèque :

```python
utm30n = crskit.crs_from_epsg(25830)

utm30n.name             # 'ETRS89 / UTM zone 30N'
utm30n.authority_code   # 25830
utm30n.dimension        # 2
utm30n.axes             # [('E', 'East'), ('N', 'North')]
```

**C'est cette dernière ligne qui piège tout le monde.** Un SRC déclare l'*ordre* de ses coordonnées,
et cet ordre n'est pas toujours celui que vous supposez :

```python
>>> crskit.crs_from_epsg(4258).axes
[('Lat', 'North'), ('Lon', 'East')]
```

L'EPSG 4258 est *latitude d'abord*. Le 4326 aussi. CrsKit prend les coordonnées dans l'ordre que le
système déclare — si vous lui donnez la longitude en premier, vous obtiendrez un résultat faux, pas
une erreur, parce que votre point est tout simplement ailleurs. Dans le doute, affichez `.axes`.

---

## 3. Votre première transformation

Une **transformation** va d'un système à un autre. Construisez-la une fois, utilisez-la souvent.

```python
etrs89 = crskit.crs_from_epsg(4258)      # latitude, longitude
utm30n = crskit.crs_from_epsg(25830)     # coordonnée Est, coordonnée Nord

transformation = crskit.transformation(etrs89, utm30n)

madrid = [40.416775, -3.703790]          # latitude, longitude — comme le déclare 4258
transformation.transform(madrid)
# [440291.2843479216, 4474254.600145094]
```

Le retour est une autre transformation, construite dans l'autre sens :

```python
crskit.transformation(utm30n, etrs89).transform([440291.284, 4474254.600])
# [40.41677499..., -3.70379000...]
```

Une transformation sait ce qu'elle est :

```python
transformation.source_dimension   # 2
transformation.target_dimension   # 2
transformation.is_identity        # False
transformation.source             # <Crs EPSG:4258 "ETRS89" (2D)>
```

---

## 4. Le Well-Known Text : comment les systèmes voyagent

Un fichier `.prj` à côté d'un shapefile, le champ `crs` d'un GeoTIFF, la chaîne stockée dans une
colonne de base de données — tous transportent un SRC sous forme de **Well-Known Text**. Il en existe
plusieurs dialectes, et CrsKit les lit tous, en détectant lequel c'est à partir du texte :

```python
crs = crskit.crs_from_wkt(open("layer.prj").read())
```

À l'écriture, le dialecte est votre choix :

```python
utm30n.to_wkt()                                # OGC WKT 1 — par défaut, le plus largement compris
utm30n.to_wkt(crskit.WktVersion.WKT1_ESRI)     # ce qu'ArcGIS attend dans un .prj
utm30n.to_wkt(crskit.WktVersion.WKT2_2019)     # ISO 19162:2019, le moderne
```

```
WKT1        PROJCS["ETRS89 / UTM zone 30N",GEOGCS["ETRS89",DATUM["European T...
WKT1_ESRI   PROJCS["ETRS_1989_UTM_Zone_30N",GEOGCS["GCS_ETRS_1989",DATUM["D_...
WKT2_2019   PROJCRS["ETRS89 / UTM zone 30N",BASEGEOGCRS["ETRS89",DATUM["Euro...
```

### Comparer des systèmes

Deux systèmes sont égaux lorsqu'ils signifient la même chose mathématiquement — même datum, même
projection, mêmes paramètres, mêmes unités. Les noms, les autorités et l'ordre des axes n'entrent pas
en ligne de compte :

```python
>>> crskit.crs_from_wkt(utm30n.to_wkt()) == utm30n
True
```

C'est `==` qui fait un vrai travail : l'aller-retour par le WKT perd le code EPSG, et les systèmes se
comparent tout de même comme égaux parce qu'ils *sont* le même système. Quand vous voulez savoir *à
quel point* deux systèmes sont proches, demandez une note sur 100 :

```python
>>> etrs89.compare(crskit.crs_from_epsg(4326))    # ETRS89 vs WGS 84
40
```

Quarante, pas cent : ETRS89 et WGS 84 sont quasiment identiques aujourd'hui, mais ce ne sont pas le
même système, et la bibliothèque ne fera pas semblant du contraire.

---

## 5. Quand il y a plus d'une bonne réponse

C'est ici que la géodésie cesse d'être une simple table de correspondance.

Passer d'un **datum** à un autre — disons du vieil ED50 européen à l'ETRS89 moderne — n'est pas une
formule, c'est une mesure. Différents organismes l'ont mesurée différemment, à différents endroits,
avec différentes précisions. EPSG recense **douze** transformations de l'ED50 vers l'ETRS89, et une
bibliothèque n'a aucun moyen de savoir laquelle vous voulez.

CrsKit refuse donc de deviner :

```python
>>> crskit.transformation(crskit.crs_from_epsg(4230), crskit.crs_from_epsg(4258))
TransformationNotFoundError: Multiple transformations were located between the 4230 (ED50)
coordinate system and the 4258 (ETRS89) coordinate system.
```

C'est vous qui choisissez, au moyen d'un callback qui reçoit les candidates :

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

Lisez cette liste en géographe, pas en programmeur : **le plus petit nombre n'est pas la réponse, le
bon domaine de validité l'est.** Une opération valable en Norvège produira très volontiers des
coordonnées pour un point situé en Espagne, et elles seront fausses de plusieurs centaines de mètres.
Chaque candidate vous dit ce dont vous avez besoin pour décider :

```python
operation.code                # 1632
operation.accuracy            # 1.5   (mètres)
operation.area_of_use         # 'Spain - mainland except northwest'
operation.grid_files          # []    — ou la grille dont elle a besoin, voir §6
operation.information_source  # qui l'a publiée
```

Renvoyez l'opération que vous voulez (ou simplement son code), et vous obtenez la transformation :

```python
def for_mainland_spain(source_name, target_name, operations):
    spain = [o for o in operations if "Spain - mainland" in o.area_of_use and not o.grid_files]
    return min(spain, key=lambda o: o.accuracy)

transformation = crskit.transformation(ed50, etrs89, select_operation=for_mainland_spain)
transformation.transform([40.417659, -3.702557])
# [40.416478, -3.703862]     — environ 130 m plus loin : voilà à quoi ressemble un changement de datum
```

---

## 6. Hauteurs, géoïdes et fichiers de grille

Votre récepteur GNSS indique une **hauteur ellipsoïdale** : la distance à un ellipsoïde mathématique.
Personne n'entend cela par « altitude ». L'altitude d'une carte est **orthométrique** :
mesurée à partir du géoïde, la surface que suivrait le niveau moyen des mers. L'écart entre les deux —
l'*ondulation* — est d'environ +52 m au-dessus de Madrid et de −28 m au-dessus de l'Everest, et il
varie d'un endroit à l'autre : c'est pourquoi il se présente sous la forme d'un **fichier de grille**,
un modèle de géoïde.

Les fichiers de grille appartiennent aux organismes qui les publient : aucune bibliothèque ne les
distribue. CrsKit vous dit exactement celui qu'il lui faut :

```python
try:
    crskit.transformation(source, target)
except crskit.GridFileNotFoundError as error:
    error.grid_file           # 'EGM08_REDNAP.txt'
    error.operation_code      # 9410
    error.information_source  # 'National Geographic Institute of Spain (IGN)...'
    error.searched_path       # où il a cherché
```

Téléchargez-le (le [README](README.md#where-to-get-the-grids) recense les plus courants), déposez-le
dans un dossier, et passez ce dossier à `init(data_directory=...)`.

### Deux façons de demander une hauteur

**Un SRC composé** associe un système horizontal à un système vertical. EPSG n'a pas de code pour la
plupart des combinaisons, vous le construisez donc à partir de ses deux parties :

```python
etrs89_3d = crskit.crs_from_epsg(4937)                  # latitude, longitude, hauteur ellipsoïdale
utm_msl = crskit.compound_crs(25830, 5782)              # UTM 30N + altitudes d'Alicante

crskit.transformation(etrs89_3d, utm_msl).transform([40.416775, -3.703790, 700.0])
# [440291.284, 4474254.600, 648.888]
```

700 m au-dessus de l'ellipsoïde font 648,888 m au-dessus de la mer : le géoïde y est 51,112 m plus
haut.

**Ou transformez directement vers un SRC vertical**, quand seule la hauteur vous intéresse. Le point
revient avec ses coordonnées horizontales intactes et seule la hauteur convertie :

```python
to_egm2008 = crskit.transformation(
    crskit.crs_from_epsg(4979),                          # WGS 84 3D
    crskit.crs_from_epsg(3855),                          # altitudes EGM2008 — le géoïde mondial
    select_operation=lambda source, target, ops: ops[0])

to_egm2008.transform([27.988056, 86.925278, 8820.43])    # Everest, hauteur ellipsoïdale
# [27.988056, 86.925278, 8848.86]
```

Ce dernier nombre est l'altitude de l'Everest telle que tout le monde la connaît, et il est tombé
d'une hauteur ellipsoïdale inférieure de 28 m. Si vous voulez une seule vérification pour savoir que
toute la chaîne fonctionne, c'est celle-là.

---

## 7. Beaucoup de points d'un coup

`transform()` franchit la frontière entre Python et C++ une fois par point. Pour un nuage de points,
passez plutôt tout le tampon d'un bloc : `transform_points()` prend un tableau NumPy de forme
`(n, dimensions)`, libère le GIL, et vous rend un tableau.

```python
import numpy as np

points = np.column_stack([latitudes, longitudes])        # (1_000_000, 2)
projected = transformation.transform_points(points)      # (1_000_000, 2)
```

```
transform_points: 1,000,000 points in 0.394 s
transform()     : the same, one point at a time, would take about 2.2 s
```

Cinq ou six fois plus rapide, et l'écart se creuse avec le nombre de points. NumPy est optionnel —
`transform()` fonctionne avec de simples listes, et le module s'importe sans que NumPy soit installé.

---

## 8. Quand les choses tournent mal

Toutes les erreurs que lève la bibliothèque descendent de `crskit.CrsError` : un seul `except` les
attrape toutes. Les types spécifiques sont là pour les cas où vous pouvez vraiment y faire quelque
chose :

| | |
|---|---|
| `AuthorityCodeNotFoundError` | ce code EPSG n'existe pas |
| `WktParseError` | le WKT est mal formé |
| `TransformationNotFoundError` | aucune opération entre ces systèmes — ou plusieurs, et vous n'avez pas choisi (§5) |
| `GridFileNotFoundError` | il manque un fichier de grille ; l'exception dit lequel (§6) |
| `CoordinateOutsideDomainError` | le point est en dehors de la grille ou du domaine de validité de la projection |
| `DimensionMismatchError` | vous avez donné un point 2D à une transformation 3D, ou l'inverse |
| `UnsupportedFormatError` | ce type de SRC ou cette méthode d'opération n'est pas pris en charge |

`CoordinateOutsideDomainError` mérite un mot. Le plus souvent, elle veut dire ce qu'elle dit — le
point est en dehors de l'emprise couverte par la grille — mais c'est aussi ce que vous obtenez quand
vous fournissez les coordonnées dans le mauvais ordre des axes, parce que le point transposé atterrit
en pleine mer. Si une transformation qui devrait marcher signale un point hors de son domaine,
vérifiez `.axes` avant toute chose.

---

## 9. À l'intérieur d'une application qui embarque Python

Si vous exécutez des scripts dans une application qui utilise déjà CrsKit, le module se lie à la
bibliothèque que l'application a chargée et partage son état — son catalogue EPSG, ses réglages. Il
n'y a rien à initialiser :

```python
if not crskit.is_initialized():
    crskit.init()
```

---

## Pour aller plus loin

- [`examples/`](examples/) — les versions exécutables de tout ce qui précède.
- [README](README.md) — la référence, et où télécharger les fichiers de grille.
- [La bibliothèque C++](https://github.com/digi21/crskit) — ce binding n'en est qu'une fine couche, et
  l'API C++ est l'endroit où trouver tout ce qui n'est pas exposé ici.
