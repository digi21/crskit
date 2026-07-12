#pragma once

// Defines the _modeCrsKit export/import attribute used by every public class in the
// library. Factored out of the CrsKit.h umbrella so that individual headers can
// be self-contained: a header that exports a type only needs to include this one to get
// the right __declspec/visibility attribute, instead of relying on include order.
//
//   _CREATING_CRSKIT defined -> we are building the DLL          -> export
//   _CREATING_CRSKIT undefined -> a consumer is including us     -> import (and, on MSVC,
//                                                                   auto-link the .lib)

#ifdef _WIN32
#  ifdef _CREATING_CRSKIT
#    define _modeCrsKit __declspec(dllexport)
#  else
#    define _modeCrsKit __declspec(dllimport)
#    ifdef _MSC_VER
#      ifdef _DEBUG
#        pragma comment(lib, "crskitD.lib")
#      else
#        pragma comment(lib, "crskit.lib")
#      endif
#    endif
#  endif
#else
#  ifdef _CREATING_CRSKIT
#    define _modeCrsKit __attribute__((visibility("default")))
#  else
#    define _modeCrsKit
#  endif
#endif

// Visibility for header-only types whose type_info must be ONE object across every binary: the
// exception hierarchy. Mach-O matches a catch handler by comparing type_info POINTERS, so when a
// consumer built with -fvisibility=hidden (which is what pybind11 does to a module, by design) keeps
// a private copy of the type_info, the `catch (AuthorityCodeNotFoundException const&)` inside the
// binding never matches what libcrskit.dylib threw: the typed exception decays to a generic one.
// ELF merges those weak symbols by name and MSVC matches by decorated name, which is why only macOS
// ever showed this.
//
// Deliberately NOT _modeCrsKit: these classes are header-only and fully inline, so __declspec(dllimport)
// would send a consumer looking in the DLL for symbols that were never put there.
#ifdef _WIN32
#  define _visibleCrsKit
#else
#  define _visibleCrsKit __attribute__((visibility("default")))
#endif
