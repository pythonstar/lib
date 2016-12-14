#ifndef LIBXL_SETUP_CPP_H
#define LIBXL_SETUP_CPP_H

#if !defined(LIBXL_STATIC) && (defined(_MSC_VER) || defined(__WATCOMC__))

  #ifdef libxl_EXPORTS
      #define XLAPI __declspec(dllexport)
  #else
      #define XLAPI __declspec(dllimport)
  #endif

  #define XLAPIENTRY __cdecl

#else

  #define XLAPI

  #if defined(__MINGW32__)
    #define XLAPIENTRY __cdecl
  #else
    #define XLAPIENTRY
  #endif

#endif

#endif
