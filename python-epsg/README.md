# crskit-epsg

The **EPSG Geodetic Parameter Dataset** as a SQLite database, packaged so that
[CrsKit](https://github.com/digi21/crskit) works out of the box:

```bash
pip install crskit[epsg]
```

```python
import crskit

crskit.init()                       # finds the database in this package
crskit.crs_from_epsg(25830)
```

The package is data only — no code beyond three helpers:

```python
import crskit_epsg

crskit_epsg.EPSG_VERSION            # '12.057', the dataset version
crskit_epsg.database_path()         # the SQLite file, to hand to crskit.init()
crskit_epsg.terms_of_use_path()     # the terms you are bound by
```

It is versioned after the dataset, not after CrsKit: `crskit-epsg 12.57` *is* EPSG 12.057. Upgrading
the catalogue is `pip install -U crskit-epsg`, with no new release of the library.

## Attribution and terms

The data are the **EPSG Geodetic Parameter Dataset** (<https://epsg.org>), **owned by the
International Association of Oil & Gas Producers (IOGP)** and used under the
[EPSG Terms of Use](https://epsg.org/terms-of-use.html), a copy of which ships inside the package
(`EPSG-TERMS-OF-USE.md`) — **by using this package you accept them**.

The database is a faithful format conversion of the SQL scripts IOGP publishes: EPSG's own schema,
with no parameter value altered. IOGP publishes the dataset at no charge and **does not endorse
CrsKit**. The dataset is provided **"as is"**, without warranty of any kind, express or implied,
including the implied warranties of merchantability and fitness for a particular purpose; IOGP
disclaims all liability for any use made of it. Use is at your own risk.

The packaging is Apache-2.0, like CrsKit; the *data* remain IOGP's, under the terms above.
