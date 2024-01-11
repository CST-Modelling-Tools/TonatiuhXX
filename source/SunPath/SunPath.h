#pragma once

#include <QtCore/qglobal.h>

//#define SunPath

#ifdef SunPath_EXPORT
    #define SunPath Q_DECL_EXPORT
#else
    #define SunPath Q_DECL_IMPORT
#endif
