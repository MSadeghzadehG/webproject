

#ifndef __XMLUTILS_P_H__
#define __XMLUTILS_P_H__


#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QString;
class QChar;
class QXmlCharRange;


class QXmlUtils
{
public:
    static bool isEncName(const QString &encName);
    static bool isChar(const QChar c);
    static bool isNameChar(const QChar c);
    static bool isLetter(const QChar c);
    static bool isNCName(const QStringRef &ncName);
    static inline bool isNCName(const QString &ncName) { return isNCName(&ncName); }
    static bool isPublicID(const QString &candidate);

private:
    typedef const QXmlCharRange *RangeIter;
    static bool rangeContains(RangeIter begin, RangeIter end, const QChar c);
    static bool isBaseChar(const QChar c);
    static bool isDigit(const QChar c);
    static bool isExtender(const QChar c);
    static bool isIdeographic(const QChar c);
    static bool isCombiningChar(const QChar c);
};

QT_END_NAMESPACE

#endif
