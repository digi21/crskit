# CrsKit per Python — tutorial

[English](TUTORIAL.md) · [Español](TUTORIAL.es.md) · [Français](TUTORIAL.fr.md) · **Italiano** · [Deutsch](TUTORIAL.de.md)

Questa è la strada lunga. Dà per scontato che tu conosca Python e zero geodesia: ogni concetto viene
introdotto quando ti serve per la prima volta, e ogni output qui sotto è stato prodotto eseguendo il
codice.

Se ti basta il riferimento, il [README](README.md) è più breve. Se vuoi le versioni eseguibili di
quel che segue, sono in [`examples/`](examples/).

---

## 1. Installazione

```bash
pip install crskit[epsg]
```

Arrivano due pacchetti: `crskit`, la libreria, e `crskit-epsg`, il catalogo EPSG — il database di
tutti i sistemi di riferimento di coordinate a cui il mondo ha dato un nome. Il catalogo è un dato di
proprietà di IOGP, quindi vive in un pacchetto a parte; ed è anche il motivo per cui è versionato in
base al dataset e non alla libreria.

```python
import crskit

crskit.init()
print(crskit.epsg_version())     # 12.057
```

`init()` trova il catalogo da solo. Indirizzalo altrove se ne tieni una copia tua:

```python
crskit.init("/path/to/epsg.sqlite", data_directory="/path/to/grids")
```

`data_directory` è dove risiedono i *file di griglia*. Per ora ignoralo — il §6 spiega quando te ne
serve uno.

---

## 2. Cos'è davvero un sistema di riferimento di coordinate

Una coppia di numeri come `(440291, 4474254)` non significa niente di per sé. Significa qualcosa solo
quando dichiari **in quale sistema** stanno quei numeri: che forma si presume abbia la Terra, dove sta
il suo centro e come la superficie curva è stata appiattita su un piano.

EPSG assegna un codice a ciascuno di questi sistemi. Ne incontrerai tre tipi:

- **Geografico** — latitudine e longitudine in gradi, su un ellissoide. `EPSG:4326` è WGS 84, quello
  che ti riporta il telefono. `EPSG:4258` è ETRS89, quello europeo.
- **Proiettato** — metri su un piano, dopo una proiezione. `EPSG:25830` è ETRS89 / UTM zone 30N, che
  copre la Spagna.
- **Verticale** — quote, e solo quote. `EPSG:5782` è la quota di Alicante, il livello del mare
  spagnolo.

Chiedilo alla libreria:

```python
utm30n = crskit.crs_from_epsg(25830)

utm30n.name             # 'ETRS89 / UTM zone 30N'
utm30n.authority_code   # 25830
utm30n.dimension        # 2
utm30n.axes             # [('E', 'East'), ('N', 'North')]
```

**È quest'ultima riga quella che frega tutti.** Un CRS dichiara l'*ordine* delle sue coordinate, e
quell'ordine non è sempre quello che dai per scontato:

```python
>>> crskit.crs_from_epsg(4258).axes
[('Lat', 'North'), ('Lon', 'East')]
```

EPSG 4258 vuole *prima la latitudine*. E così anche 4326. CrsKit prende le coordinate nell'ordine che
il sistema dichiara: se gli passi prima la longitudine otterrai una risposta sbagliata, non un errore,
perché il tuo punto è semplicemente da un'altra parte. Nel dubbio, stampa `.axes`.

---

## 3. La tua prima trasformazione

Una **trasformazione** porta da un sistema a un altro. Costruiscila una volta, usala molte.

```python
etrs89 = crskit.crs_from_epsg(4258)      # latitudine, longitudine
utm30n = crskit.crs_from_epsg(25830)     # coordinata Est, coordinata Nord

transformation = crskit.transformation(etrs89, utm30n)

madrid = [40.416775, -3.703790]          # latitudine, longitudine — come dichiara 4258
transformation.transform(madrid)
# [440291.2843479216, 4474254.600145094]
```

Tornare indietro è un'altra trasformazione, costruita al contrario:

```python
crskit.transformation(utm30n, etrs89).transform([440291.284, 4474254.600])
# [40.41677499..., -3.70379000...]
```

Una trasformazione sa cos'è:

```python
transformation.source_dimension   # 2
transformation.target_dimension   # 2
transformation.is_identity        # False
transformation.source             # <Crs EPSG:4258 "ETRS89" (2D)>
```

---

## 4. Well-Known Text: come viaggiano i sistemi

Un file `.prj` accanto a uno shapefile, il campo `crs` di un GeoTIFF, la stringa in una colonna di
database — tutti trasportano un CRS come **Well-Known Text**. Ne esistono vari dialetti, e CrsKit li
legge tutti, riconoscendo dal testo di quale si tratta:

```python
crs = crskit.crs_from_wkt(open("layer.prj").read())
```

In scrittura scegli tu il dialetto:

```python
utm30n.to_wkt()                                # OGC WKT 1 — il predefinito, il più compreso ovunque
utm30n.to_wkt(crskit.WktVersion.WKT1_ESRI)     # quello che ArcGIS si aspetta in un .prj
utm30n.to_wkt(crskit.WktVersion.WKT2_2019)     # ISO 19162:2019, il moderno
```

```
WKT1        PROJCS["ETRS89 / UTM zone 30N",GEOGCS["ETRS89",DATUM["European T...
WKT1_ESRI   PROJCS["ETRS_1989_UTM_Zone_30N",GEOGCS["GCS_ETRS_1989",DATUM["D_...
WKT2_2019   PROJCRS["ETRS89 / UTM zone 30N",BASEGEOGCRS["ETRS89",DATUM["Euro...
```

### Confrontare i sistemi

Due sistemi sono uguali quando significano la stessa cosa dal punto di vista matematico — stesso
datum, stessa proiezione, stessi parametri, stesse unità. Nomi, autorità e ordine degli assi non
c'entrano:

```python
>>> crskit.crs_from_wkt(utm30n.to_wkt()) == utm30n
True
```

Qui `==` sta facendo lavoro vero: il giro di andata e ritorno per WKT perde il codice EPSG, eppure i
sistemi risultano uguali perché *sono* lo stesso sistema. Quando invece vuoi sapere *quanto* sono
vicini due sistemi, chiedi un punteggio su 100:

```python
>>> etrs89.compare(crskit.crs_from_epsg(4326))    # ETRS89 vs WGS 84
40
```

Quaranta, non cento: ETRS89 e WGS 84 oggi sono quasi la stessa cosa, ma non sono lo stesso sistema, e
la libreria non fa finta del contrario.

---

## 5. Quando c'è più di una risposta giusta

È qui che la geodesia smette di essere una tabella di consultazione.

Passare da un **datum** a un altro — poniamo dal vecchio ED50 europeo al moderno ETRS89 — non è una
formula, è una misura. Enti diversi l'hanno misurata in modi diversi, in posti diversi, con
accuratezze diverse. EPSG registra **dodici** trasformazioni da ED50 a ETRS89, e non c'è modo che una
libreria indovini quale vuoi tu.

Perciò CrsKit si rifiuta di tirare a indovinare:

```python
>>> crskit.transformation(crskit.crs_from_epsg(4230), crskit.crs_from_epsg(4258))
TransformationNotFoundError: Multiple transformations were located between the 4230 (ED50)
coordinate system and the 4258 (ETRS89) coordinate system.
```

Scegli tu, con una callback che riceve i candidati:

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

Leggi quell'elenco da geografo, non da programmatore: **la risposta non è il numero più piccolo, è
l'ambito di validità giusto.** Un'operazione valida in Norvegia produrrà tranquillamente coordinate
per un punto in Spagna, e saranno sbagliate di centinaia di metri. Ogni candidato ti dice quel che ti
serve per decidere:

```python
operation.code                # 1632
operation.accuracy            # 1.5   (metri)
operation.area_of_use         # 'Spain - mainland except northwest'
operation.grid_files          # []    — oppure la griglia di cui ha bisogno, vedi §6
operation.information_source  # chi l'ha pubblicata
```

Restituisci l'operazione che vuoi (o anche solo il suo codice) e ottieni la trasformazione:

```python
def for_mainland_spain(source_name, target_name, operations):
    spain = [o for o in operations if "Spain - mainland" in o.area_of_use and not o.grid_files]
    return min(spain, key=lambda o: o.accuracy)

transformation = crskit.transformation(ed50, etrs89, select_operation=for_mainland_spain)
transformation.transform([40.417659, -3.702557])
# [40.416478, -3.703862]     — circa 130 m più in là: ecco che aspetto ha un cambio di datum
```

---

## 6. Quote, geoidi e file di griglia

Il tuo ricevitore GNSS riporta una **quota ellissoidica**: la distanza da un ellissoide matematico.
Nessuno intende quello, quando dice "altitudine". La quota su una mappa è **ortometrica**: misurata a
partire dal geoide, la superficie che il livello medio del mare seguirebbe. La differenza fra le due —
l'*ondulazione* — è di circa +52 m su Madrid e di −28 m sull'Everest, e varia da luogo a luogo: ecco
perché arriva sotto forma di **file di griglia**, cioè un modello di geoide.

I file di griglia appartengono agli enti che li pubblicano, quindi nessuna libreria li distribuisce.
CrsKit ti dice esattamente quale gli serve:

```python
try:
    crskit.transformation(source, target)
except crskit.GridFileNotFoundError as error:
    error.grid_file           # 'EGM08_REDNAP.txt'
    error.operation_code      # 9410
    error.information_source  # 'National Geographic Institute of Spain (IGN)...'
    error.searched_path       # dove ha cercato
```

Scaricalo (il [README](README.md#where-to-get-the-grids) elenca i più comuni), mettilo in una cartella
e passa quella cartella a `init(data_directory=...)`.

### Due modi di chiedere una quota

**Un CRS composto** accoppia un sistema orizzontale con uno verticale. EPSG non ha un codice per la
maggior parte delle combinazioni, quindi lo costruisci dalle sue due parti:

```python
etrs89_3d = crskit.crs_from_epsg(4937)                  # latitudine, longitudine, quota ellissoidica
utm_msl = crskit.compound_crs(25830, 5782)              # UTM 30N + quota di Alicante

crskit.transformation(etrs89_3d, utm_msl).transform([40.416775, -3.703790, 700.0])
# [440291.284, 4474254.600, 648.888]
```

700 m sull'ellissoide sono 648,888 m sul mare: lì il geoide sta 51,112 m più in alto.

**Oppure trasforma direttamente verso un CRS verticale**, quando l'unica cosa che ti interessa è la
quota. Il punto torna indietro con le coordinate orizzontali intatte e solo la quota convertita:

```python
to_egm2008 = crskit.transformation(
    crskit.crs_from_epsg(4979),                          # WGS 84 3D
    crskit.crs_from_epsg(3855),                          # quota EGM2008 — il geoide globale
    select_operation=lambda source, target, ops: ops[0])

to_egm2008.transform([27.988056, 86.925278, 8820.43])    # Everest, ellissoidica
# [27.988056, 86.925278, 8848.86]
```

Quell'ultimo numero è la quota dell'Everest come la conoscono tutti, ed è saltata fuori da una quota
ellissoidica 28 m più bassa. Se vuoi un'unica verifica che tutto lo stack funzioni, è questa.

---

## 7. Molti punti in una volta

`transform()` attraversa il confine fra Python e C++ una volta per punto. Per una nuvola di punti,
passa invece l'intero buffer: `transform_points()` accetta un array NumPy di forma `(n, dimensions)`,
rilascia il GIL e torna con un array.

```python
import numpy as np

points = np.column_stack([latitudes, longitudes])        # (1_000_000, 2)
projected = transformation.transform_points(points)      # (1_000_000, 2)
```

```
transform_points: 1,000,000 points in 0.394 s
transform()     : the same, one point at a time, would take about 2.2 s
```

Cinque o sei volte più veloce, e il divario cresce col numero di punti. NumPy è opzionale —
`transform()` funziona con normali liste, e il modulo si importa anche senza NumPy installato.

---

## 8. Quando le cose vanno storte

Ogni errore sollevato dalla libreria discende da `crskit.CrsError`, quindi un solo `except` li prende
tutti. I tipi specifici ci sono per i casi in cui puoi davvero farci qualcosa:

| | |
|---|---|
| `AuthorityCodeNotFoundError` | quel codice EPSG non esiste |
| `WktParseError` | il WKT è malformato |
| `TransformationNotFoundError` | nessuna operazione fra quei sistemi — oppure diverse, e non hai scelto (§5) |
| `GridFileNotFoundError` | manca un file di griglia; l'eccezione dice quale (§6) |
| `CoordinateOutsideDomainError` | il punto è fuori dalla griglia o dall'area di validità della proiezione |
| `DimensionMismatchError` | hai dato un punto 2D a una trasformazione 3D, o viceversa |
| `UnsupportedFormatError` | quel tipo di CRS o quel metodo di operazione non è supportato |

`CoordinateOutsideDomainError` merita una parola. Di solito significa proprio quel che dice — il punto
è fuori dall'area coperta dalla griglia — ma è anche quel che ottieni quando passi le coordinate
nell'ordine degli assi sbagliato, perché il punto trasposto finisce in mezzo al mare. Se una
trasformazione che dovrebbe funzionare ti segnala un punto fuori dominio, controlla `.axes` prima di
qualunque altra cosa.

---

## 9. Dentro un'applicazione che incorpora Python

Se stai eseguendo script dentro un'applicazione che già usa CrsKit, il modulo si aggancia alla
libreria che l'applicazione ha caricato e ne condivide lo stato — il suo catalogo EPSG, le sue
impostazioni. Non c'è niente da inizializzare:

```python
if not crskit.is_initialized():
    crskit.init()
```

---

## Dove andare adesso

- [`examples/`](examples/) — le versioni eseguibili di tutto quel che precede.
- [README](README.md) — il riferimento, e dove scaricare i file di griglia.
- [La libreria C++](https://github.com/digi21/crskit) — questo binding è uno strato sottile sopra di
  essa, e l'API C++ è dove trovi tutto ciò che qui non è esposto.
