# vcpkg de principio a fin, con `crskit` como ejemplo

> Tutorial pensado para alguien que domina **C++ moderno, CMake y MSBuild**, pero que no tiene claro
> **cómo funciona vcpkg por dentro**: qué es un *port*, cómo se integra con CMake y con Visual Studio, y
> qué pinta `vcpkg_from_git`. Usa como ejemplo real el empaquetado que acabamos de montar en este repo:
> la librería **crskit** (antes `Digi21.OpenGIS`) y el proyecto **Prueba** que la consume.

---

## 0. La idea de una frase

vcpkg es **un gestor de dependencias que compila desde fuente**. En vez de descargar binarios opacos,
descarga el *código* de cada dependencia, lo compila **con tu mismo compilador/opciones** y te lo deja
listo para que CMake o MSBuild lo encuentren. Un **port** es "la receta para compilar e instalar una
librería"; consumir vcpkg es "pedir esas librerías y que tu build las vea sin que tú toques rutas".

Hay, por tanto, **dos lados**:

- **Consumidor** (nuestro `Prueba`): declara qué necesita y deja que vcpkg lo provea.
- **Productor** (nuestra `crskit`): escribe el *port* que enseña a vcpkg a compilar la librería.

---

## 1. Conceptos base

### 1.1. Los tres directorios de trabajo
Cuando vcpkg construye algo, usa tres carpetas (dentro de `VCPKG_ROOT` o de tu build):

| Carpeta | Qué es |
|---|---|
| `buildtrees/<port>` | Donde compila (out-of-source). Aquí van los `.obj`, logs, CMakeCache… |
| `packages/<port>_<triplet>` | El resultado "en crudo" de instalar ese port (headers, libs, config). |
| `installed/<triplet>` | La **fusión** de todos los packages instalados: un único árbol `include/`, `lib/`, `bin/`, `share/`. Esto es lo que ve tu compilador. |

Clave mental: **todas** las dependencias acaban fusionadas en `installed/<triplet>/include` y `.../lib`.
Por eso, cuando `crskit` depende de `sqlite3`, tu `#include <sqlite3.h>` funciona: ambos headers viven en
el mismo `include/`.

### 1.2. Triplets
Un **triplet** es "para qué plataforma/ABI compilo": `x64-windows`, `x64-linux`, `x64-windows-static`…
`x64-windows` significa x64 + CRT dinámica + DLLs. Todo lo que se mezcle en un `installed/` debe ser del
**mismo triplet** (no puedes enlazar una lib `x64-windows-static` con un exe `x64-windows`). Nosotros
usamos `x64-windows`.

### 1.3. Dos modos de uso
- **Classic mode**: `vcpkg install sqlite3` instala en el vcpkg global (`VCPKG_ROOT/installed`). Estado
  compartido por toda la máquina. Simple pero "sucio".
- **Manifest mode** (el bueno, el que usamos): pones un **`vcpkg.json`** en tu proyecto listando
  dependencias; vcpkg las instala en un `vcpkg_installed/` **local a ese proyecto**. Reproducible y
  aislado. Es el equivalente a un `package.json` / `Cargo.toml`.

---

## 2. El lado CONSUMIDOR

### 2.1. El manifiesto `vcpkg.json`
Nuestro `Prueba/vcpkg.json`:
```json
{
  "name": "prueba",
  "version": "0.0.0",
  "dependencies": [ "crskit" ]
}
```
"Este proyecto necesita crskit." vcpkg resolverá que crskit a su vez necesita sqlite3, vcpkg-cmake, etc.,
y lo instalará todo.

### 2.2. Cómo se integra con **CMake**
vcpkg se engancha a CMake por un **toolchain file**. Al configurar pasas:
```
cmake -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
```
(en este repo lo hace `CMakePresets.json` con el preset `windows-msvc`). Ese toolchain hace dos cosas:
1. Si detecta un `vcpkg.json`, **instala las dependencias** antes de configurar (manifest mode).
2. Redirige `find_package(...)` para que mire primero en `vcpkg_installed/<triplet>`.

Así, en un consumidor CMake te basta con:
```cmake
find_package(crskit CONFIG REQUIRED)
target_link_libraries(mi_app PRIVATE crskit::crskit)
```
y CMake ya sabe los `include/` y `lib/` correctos. **No tocas rutas a mano.** El `CONFIG` significa
"búscalo por su fichero `crskit-config.cmake`", no por un módulo `Find*.cmake`.

### 2.3. Cómo se integra con **MSBuild / Visual Studio** (el caso de `Prueba`)
`Prueba` es un `.vcxproj`, no CMake. vcpkg trae una **integración MSBuild** (`vcpkg.props` +
`vcpkg.targets` en `VCPKG_ROOT/scripts/buildsystems/msbuild/`). La activamos **por proyecto** (sin
`vcpkg integrate install` global) importándola en el `.vcxproj`:

```xml
<PropertyGroup Label="Globals">
  <VcpkgRoot Condition="'$(VcpkgRoot)'==''">$(VCPKG_ROOT)</VcpkgRoot>
  <VcpkgEnableManifest>true</VcpkgEnableManifest>
  <VcpkgManifestInstall>true</VcpkgManifestInstall>
  <VcpkgTriplet>x64-windows</VcpkgTriplet>
  <VcpkgEnabled Condition="'$(Platform)'=='x64'">true</VcpkgEnabled>
</PropertyGroup>
...
<Import Project="$(VcpkgRoot)\scripts\buildsystems\msbuild\vcpkg.props" Condition="Exists(...)" />
...
<Import Project="$(VcpkgRoot)\scripts\buildsystems\msbuild\vcpkg.targets" Condition="Exists(...)" />
```

Con eso, al compilar `Prueba`, MSBuild:
1. Ejecuta `vcpkg install` del manifiesto (`Prueba/vcpkg.json`).
2. Añade `vcpkg_installed/x64-windows/include` a los includes y `.../lib` a las rutas de librería.
3. **Auto-enlaza** las `.lib` instaladas (por eso `Prueba` no lista `Digi21.OpenGIS.lib` ni `sqlite3.lib`
   en "Dependencias adicionales": vcpkg lo hace).
4. **Despliega las DLLs** (`applocal`): copia `Digi21.OpenGISD.dll` y `sqlite3.dll` junto al `.exe` tras
   compilar. Por eso el exe arranca sin configurar el `PATH`.

Detalle importante: MSBuild elige **Debug o Release** del árbol vcpkg según tu `$(Configuration)`. En
Debug coge `Digi21.OpenGISD.dll` (con la "D"); en Release, `Digi21.OpenGIS.dll`. Eso lo resuelve el
config exportado (ver §3.3).

---

## 3. El lado PRODUCTOR: crear el paquete `crskit`

Un port necesita **dos** cosas de nosotros:
- Que **la librería sea instalable con CMake** (reglas `install(...)` + un fichero de config).
- El **port** propiamente dicho (`vcpkg.json` + `portfile.cmake`).

### 3.1. Hacer la librería instalable (en `CMakeLists.txt`)
El punto que a todo el mundo se le olvida: para que otro proyecto haga `find_package(crskit)`, tu CMake
tiene que **instalar y exportar** un *target*. Tres bloques:

**(a) Que el target sepa sus includes en build y en instalado.** Antes apuntaba solo a la carpeta fuente;
ahora distingue:
```cmake
target_include_directories(Digi21OpenGis PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Digi21.OpenGIS>"  # al compilar aquí
    "$<INSTALL_INTERFACE:include>")                                   # una vez instalado
```
Los `$<BUILD_INTERFACE>`/`$<INSTALL_INTERFACE>` son **generator expressions**: la primera vale cuando se
usa el target dentro de este build; la segunda, cuando se consume ya instalado (ruta relativa al prefijo
de instalación).

**(b) Instalar binarios + headers + exportar el target:**
```cmake
install(TARGETS Digi21OpenGis
    EXPORT crskit-targets                       # agrupa el target bajo un "export set"
    RUNTIME DESTINATION bin                      # .dll
    LIBRARY DESTINATION lib                      # .so
    ARCHIVE DESTINATION lib)                     # .lib

install(DIRECTORY Digi21.OpenGIS/ DESTINATION include FILES_MATCHING PATTERN "*.h" ...)  # el árbol de headers

install(EXPORT crskit-targets
    FILE crskit-targets.cmake
    NAMESPACE crskit::                           # -> el target instalado se llama crskit::crskit
    DESTINATION share/crskit)
```
`EXPORT_NAME crskit` en el target hace que, con `NAMESPACE crskit::`, el consumidor vea **`crskit::crskit`**
(no `crskit::Digi21OpenGis`).

**(c) Generar el `crskit-config.cmake`** (lo que busca `find_package(crskit CONFIG)`):
```cmake
configure_package_config_file(cmake/crskit-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/crskit-config.cmake
    INSTALL_DESTINATION share/crskit)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/crskit-config.cmake DESTINATION share/crskit)
```
La plantilla `cmake/crskit-config.cmake.in` es mínima: solo carga el fichero de targets exportado.
```cmake
@PACKAGE_INIT@
include("${CMAKE_CURRENT_LIST_DIR}/crskit-targets.cmake")
check_required_components(crskit)
```
Si crskit **enlazara** contra sqlite3, aquí pondríamos `find_dependency(unofficial-sqlite3 CONFIG)` para
propagar esa dependencia transitiva. En nuestro caso el `SqliteProvider` es *header-only* y lo enlaza el
**consumidor**, así que basta con que sqlite3 esté declarado como dependencia del **port** (§3.2): eso
mete `sqlite3.h`/`sqlite3.lib` en el mismo `installed/`.

### 3.2. El manifiesto del port (`ports/crskit/vcpkg.json`)
```json
{
  "name": "crskit",
  "version": "0.1.0",
  "description": "CRS toolkit: ...",
  "supports": "windows | linux",
  "dependencies": [
    { "name": "vcpkg-cmake", "host": true },
    { "name": "vcpkg-cmake-config", "host": true },
    "sqlite3"
  ]
}
```
- `vcpkg-cmake` / `vcpkg-cmake-config` son **herramientas de host** (`"host": true`): dan las funciones
  `vcpkg_cmake_configure`, `vcpkg_cmake_install`, `vcpkg_cmake_config_fixup`.
- `sqlite3` es la dependencia real de la librería.

### 3.3. La receta (`ports/crskit/portfile.cmake`)
```cmake
get_filename_component(SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)  # ¡ver §4!

vcpkg_cmake_configure(SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS -DDIGI21_BUILD_TESTS=OFF -DDIGI21_INSTALL=ON)

vcpkg_cmake_install()                              # = cmake --build + cmake --install, Debug y Release

vcpkg_cmake_config_fixup(PACKAGE_NAME crskit CONFIG_PATH share/crskit)

file(REMOVE_RECURSE                                # una librería instala headers una sola vez
    "${CURRENT_PACKAGES_DIR}/debug/include"
    "${CURRENT_PACKAGES_DIR}/debug/share")

file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/copyright" "...")  # vcpkg exige un copyright
```
Qué hace cada pieza:
- **`vcpkg_cmake_configure` + `vcpkg_cmake_install`** compilan la librería **dos veces** (Debug y Release)
  y ejecutan tus reglas `install(...)`. El resultado cae en `packages/crskit_x64-windows/` con
  `include/`, `lib/`, `bin/`, `debug/lib`, `debug/bin`, `share/crskit`.
- **`vcpkg_cmake_config_fixup`** es el paso "mágico" imprescindible: coge el `crskit-config.cmake` +
  `crskit-targets-*.cmake` y los **fusiona/corrige** para que un solo config sepa apuntar a la lib Release
  (`lib/Digi21.OpenGIS.lib`) o a la Debug (`debug/lib/Digi21.OpenGISD.lib`) según el consumidor. Sin este
  paso, el `find_package` no encontraría bien las dos configuraciones.
- Se borra `debug/include` y `debug/share` porque los headers y el config no dependen de la config.

### 3.4. Qué queda instalado
Tras el build, `vcpkg_installed/x64-windows/` contiene:
```
include/CrsKit.h, include/SqliteProvider.h, include/CoordinateSystems/..., include/sqlite3.h
lib/Digi21.OpenGIS.lib, lib/sqlite3.lib
bin/Digi21.OpenGIS.dll, bin/sqlite3.dll
share/crskit/crskit-config.cmake (+ targets)
```
Exactamente lo que `Prueba` necesita.

---

## 4. Overlay ports, registries y el modo "in-tree"

### 4.1. ¿Dónde vive un port?
- El **registro curado** oficial de vcpkg (miles de ports). `sqlite3` sale de ahí.
- Un **overlay port**: una carpeta tuya con ports propios que vcpkg mira **además** del registro. Es lo que
  usamos: `Prueba/vcpkg-configuration.json` apunta a `../ports`:
  ```json
  { "overlay-ports": [ "../ports" ] }
  ```
  Así vcpkg encuentra nuestro `ports/crskit` sin publicarlo en ningún sitio.
- Un **registry** propio (un repo git de ports): el paso siguiente al overlay cuando quieres versionar y
  compartir tus ports "en serio".

### 4.2. El truco "in-tree" que usamos (y su pega)
En `portfile.cmake`, `SOURCE_PATH` apunta a la **raíz de este mismo repo** (`../..`). Es decir: el port
compila **el árbol de trabajo actual**, sin descargar nada. Ventaja: editas la librería y reconstruyes el
consumidor sin trámites.

**La pega (impórtala bien):** vcpkg decide si tiene que reconstruir un port calculando un **ABI hash**
sobre el `portfile.cmake`, el `vcpkg.json` y sus dependencias — **NO** sobre tu código fuente. Como el
código está fuera del hash, si editas la librería, vcpkg **cree que ya está compilada** y te da la versión
antigua de la caché. Para forzar el rebuild tras tocar la lib:
```
vcpkg remove crskit            # (o borrar vcpkg_installed del consumidor)
# y reconstruir con la caché binaria desactivada:
vcpkg install --no-binarycaching
```
o, más "correcto", **subir la versión** del port (`0.1.0 -> 0.1.1`), que sí cambia el ABI. En desarrollo
diario, `--no-binarycaching` (o borrar el zip de la caché en `%LOCALAPPDATA%\vcpkg\archives`) es lo
práctico.

### 4.3. La caché binaria
vcpkg guarda cada build (por su ABI hash) en `%LOCALAPPDATA%\vcpkg\archives`. La próxima vez que alguien
pida ese mismo ABI, lo **restaura en milisegundos** en vez de recompilar. Genial para dependencias
estables (sqlite3), y la causa de la pega de §4.2 para un port in-tree.

---

## 5. `vcpkg_from_git` / `vcpkg_from_github`: el modo publicable

Cuando `crskit` tenga su propio repo y quieras un port "de verdad" (reproducible, compartible, apto para
CI o para el registro curado), se sustituye el `SOURCE_PATH` in-tree por una **descarga fijada a un
commit**:

```cmake
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO tuusuario/crskit
    REF v0.1.0                 # un tag o un commit exacto
    SHA512 <hash de 128 hex>   # hash del tarball descargado
    HEAD_REF main)
```
(`vcpkg_from_git` es lo mismo para un git genérico con `URL`; `vcpkg_from_github` es el atajo para GitHub.)

**Por qué existe y por qué el SHA512:** un port debe ser **reproducible y verificable**. `REF` fija *qué*
código (un commit/tag inmutable, no "lo último de main") y `SHA512` **verifica** que el tarball descargado
es exactamente el esperado (integridad + seguridad: si alguien altera el archivo, el hash no cuadra y
vcpkg aborta). Ese hash **sí** entra en el ABI, así que aquí cambiar de versión reconstruye solo — se
acabó la pega de §4.2.

**Cómo se obtiene el SHA512:** la primera vez pones `SHA512 0` (o cualquier cosa), lanzas el build, y
vcpkg falla diciéndote el hash real esperado; lo copias al portfile. (También `vcpkg hash <fichero>`.)

**El flujo de trabajo publicable** queda: haces cambios en la lib → tag/commit → actualizas `REF` +
`SHA512` + `version` en el port → vcpkg reconstruye de forma limpia y cacheable. Es más ceremonia que el
in-tree, pero es lo correcto cuando la librería y el consumidor ya no viven en el mismo árbol.

---

## 6. Recapitulación: los ficheros que creamos

| Fichero | Rol |
|---|---|
| `CMakeLists.txt` (bloque install) | Hace la librería instalable y exporta `crskit::crskit`. |
| `cmake/crskit-config.cmake.in` | Plantilla del `find_package(crskit CONFIG)`. |
| `ports/crskit/vcpkg.json` | Manifiesto del port (deps: sqlite3, helpers cmake). |
| `ports/crskit/portfile.cmake` | Receta: compila desde el árbol in-tree e instala. |
| `Prueba/vcpkg.json` | El consumidor pide `crskit`. |
| `Prueba/vcpkg-configuration.json` | Le dice a vcpkg dónde está el overlay (`../ports`). |
| `Prueba/Prueba.vcxproj` | Importa la integración MSBuild de vcpkg (x64). |

Y el resultado comprobado: **`Prueba.exe` compila, enlaza y ejecuta** contra `crskit` servido por vcpkg,
con `Digi21.OpenGISD.dll` + `sqlite3.dll` desplegadas solas.

---

## 7. Chuleta de comandos

```powershell
# Instalar/validar el port aislado (modo manifiesto + overlay), sin ensuciar el vcpkg global:
#   (una carpeta con vcpkg.json que dependa de crskit + vcpkg-configuration.json con el overlay)
vcpkg install --triplet x64-windows

# Forzar rebuild del port in-tree tras tocar la librería:
vcpkg install --triplet x64-windows --no-binarycaching

# Compilar el consumidor MSBuild (dispara el vcpkg install del manifiesto):
MSBuild Prueba\Prueba.vcxproj /p:Configuration=Debug /p:Platform=x64

# Ver el hash de un tarball (para vcpkg_from_github):
vcpkg hash archivo.tar.gz
```
