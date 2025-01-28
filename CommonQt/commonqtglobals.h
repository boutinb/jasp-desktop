#ifndef COMMOTQTGLOBAL_H
#define COMMOTQTGLOBAL_H

#include <QtCore/QtGlobal>

#if defined(JASP_COMMONQT_LIB)
#  define COMMONQT_EXPORTS Q_DECL_EXPORT
#else
#  define COMMONQT_EXPORTS Q_DECL_IMPORT
#endif

#endif // COMMOTQTGLOBAL_H
