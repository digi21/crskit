#pragma once

// Defines the _modeCrsKit export/import attribute used by every public class in the
// library. Factored out of the CrsKit.h umbrella so that individual headers can
// be self-contained: a header that exports a type only needs to include this one to get
// the right __declspec/visibility attribute, instead of relying on include order.
//
//   CRSKIT_STATIC defined       -> the library is compiled into the consumer (the Python
//                                  extension module) -> no attribute at all
//   _CREATING_CRSKIT defined    -> we are building the DLL         -> export
//   _CREATING_CRSKIT undefined  -> a consumer is including us      -> import (and, on MSVC,
//                                                                    auto-link the .lib)

#ifdef CRSKIT_STATIC
#  define _modeCrsKit
#elif defined(_WIN32)
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
