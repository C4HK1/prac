#pragma once

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(FUNCTIONLIB_LIBRARY)
#    define FUNCTIONLIB_EXPORT __declspec(dllexport)
#  else
#    define FUNCTIONLIB_EXPORT __declspec(dllimport)
#  endif
#else
#  if __GNUC__ >= 4
#    define FUNCTIONLIB_EXPORT __attribute__((visibility("default")))
#  else
#    define FUNCTIONLIB_EXPORT
#  endif
#endif


