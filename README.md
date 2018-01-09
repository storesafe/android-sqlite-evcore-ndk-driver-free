# Android-sqlite-evcore-native-driver-free module

Provides a native build of sqlite with a low-level API for Cordova sqlite enterprise versions __(common build)__. _XXX TODO: include (merge) extbuild update from another branch_

Based on [liteglue / Android-sqlite-native-driver](https://github.com/liteglue/Android-sqlite-native-driver).

by Christopher J. Brody aka Chris Brody mailto: <brodybits@litehelpers.net>

License: GPL v3 (<http://www.gnu.org/licenses/gpl.txt>) or commercial license options

Includes JSMN (<http://zserge.com/jsmn.html>) under the MIT license

## About

Android-sqlite-evcore-native-driver-free provides:
- single `EVCoreNativeDriver` class with native Java interface to the needed C functions
- automatic build for major Android targets (`armeabi`, `armeabi-v7a`, `x86`, `x86_64`, `arm64-v8a`) that is accessible from the native Java interface _(TBD no BASE64 or REGEXP in common build module version)_

__TBD additional extbuild modules - NOT included by this common build version:__
  - `REGEXP` integrated from [brodybits / sqlite3-regexp-cached](https://github.com/brodybits/sqlite3-regexp-cached) (based on <http://git.altlinux.org/people/at/packages/?p=sqlite3-pcre.git> by Alexey Tourbin, public domain)
  - `BASE64` integrated from [brodybits / sqlite3-base64](https://github.com/brodybits/sqlite3-base64), using [brodybits / libb64-encode](https://github.com/brodybits/libb64-encode) (based on <http://libb64.sourceforge.net/> by Chris Venter, public domain)

This is accomplished by using [GlueGen](http://jogamp.org/gluegen/www/) around the C module.

**NOTE:** This project references the `gluegentools` and `sqlite-amalgamation` subprojects, which are resolved by: $ `make init` (as described below).

**WARNING:** The reference handles that are returned by the `EVCoreNativeDriver` library functions are raw C pointer values (with `0x100000000` added). If someone uses a reference handle that is not valid, or no longer valid with the `EVCoreNativeDriver` library the behavior is undefined (may crash, for example). It is *NOT* recommended to use this API directly unless you really understand how this library works internally.

## SQLite build information

**SQLite version:** `3.15.2`

**SQLite build flags:**
- `-DSQLITE_TEMP_STORE=2`
- `-DSQLITE_THREADSAFE=2`
- `-DSQLITE_ENABLE_FTS3`
- `-DSQLITE_ENABLE_FTS3_PARENTHESIS`
- `-DSQLITE_ENABLE_FTS4`
- `-DSQLITE_ENABLE_RTREE`

Additional common sqlite build flags ref: <http://sqlite.org/pgszchng2016.html>
- `-DSQLITE_DEFAULT_PAGE_SIZE=4096`
- `-DSQLITE_DEFAULT_CACHE_SIZE=-2000`

## Dependencies

- JSMN (MIT license)
- SQLite (public domain)

## Major TODO(s)

- Support BLOB type

# Building

## Normal build

Initialize with the `gluegentools` and `sqlite-amalgamation` subprojects:

$ `make init`

Then to build __(for common build)__:

$ `make ndkbuild`

## Regenerage Java & C glue code

$ `make regen`
