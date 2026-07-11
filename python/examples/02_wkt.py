"""Read and write Well-Known Text, and compare systems by what they mean.

CrsKit parses WKT 1 (OGC and ESRI) and WKT 2, detecting the grammar from the text, and emits any of
those flavours. Equality asks whether two systems are the same CRS *mathematically* -- same datum,
projection and units -- so a system parsed back from its own WKT equals the original even though its
name, authority and axis order need not survive the round trip.
"""

from _setup import initialise

import crskit

initialise()

utm30n = crskit.crs_from_epsg(25830)

for version in crskit.WktVersion.__members__.values():
    wkt = utm30n.to_wkt(version)
    print(f"{version.name:<24} {wkt[:64]}...")

# A .prj file, whatever dialect it is written in, comes back as a CRS.
from_wkt = crskit.crs_from_wkt(utm30n.to_wkt(crskit.WktVersion.WKT2_2019))
print(f"\nparsed back    : {from_wkt.name} (EPSG:{from_wkt.authority_code})")
print(f"same CRS?      : {from_wkt == utm30n}")

# Equality ignores names and authorities: it is the mathematics that must agree.
etrs89 = crskit.crs_from_epsg(4258)
print(f"UTM 30N == ETRS89?  {utm30n == etrs89}   (similarity {utm30n.compare(etrs89)}/100)")

# WGS 84 and ETRS89 are different systems, but their definitions are nearly the same.
wgs84 = crskit.crs_from_epsg(4326)
print(f"ETRS89 vs WGS 84    similarity {etrs89.compare(wgs84)}/100")
