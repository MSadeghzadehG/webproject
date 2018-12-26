
#ifndef _WX_TYPEINFO_H_
#define _WX_TYPEINFO_H_


#include "wx/defs.h"

#ifndef wxNO_RTTI

#if wxCHECK_VISUALC_VERSION(9)
    #define wxTRUST_CPP_RTTI    1
#else
    #define wxTRUST_CPP_RTTI    0
#endif

#include <typeinfo>
#include <string.h>

#define _WX_DECLARE_TYPEINFO_CUSTOM(CLS, IDENTFUNC)
#define WX_DECLARE_TYPEINFO_INLINE(CLS)
#define WX_DECLARE_TYPEINFO(CLS)
#define WX_DEFINE_TYPEINFO(CLS)
#define WX_DECLARE_ABSTRACT_TYPEINFO(CLS)

#if wxTRUST_CPP_RTTI

#define wxTypeId    typeid

#else 


class wxTypeIdentifier
{
public:
    wxTypeIdentifier(const char* className)
    {
        m_className = className;
    }

    bool operator==(const wxTypeIdentifier& other)
    {
        return strcmp(m_className, other.m_className) == 0;
    }

    bool operator!=(const wxTypeIdentifier& other)
    {
        return strcmp(m_className, other.m_className) != 0;
    }
private:
    const char* m_className;
};

#define wxTypeId(OBJ) wxTypeIdentifier(typeid(OBJ).name())

#endif 

#else 
#define wxTRUST_CPP_RTTI    0


typedef void (*wxTypeIdentifier)();

#define _WX_DECLARE_TYPEINFO_CUSTOM(CLS, IDENTFUNC) \
public: \
    virtual wxTypeIdentifier GetWxTypeId() const \
    { \
        return reinterpret_cast<wxTypeIdentifier> \
            (&IDENTFUNC); \
    }

#define WX_DECLARE_TYPEINFO(CLS) \
private: \
    static CLS sm_wxClassInfo(); \
_WX_DECLARE_TYPEINFO_CUSTOM(CLS, sm_wxClassInfo)

#define WX_DEFINE_TYPEINFO(CLS) \
CLS CLS::sm_wxClassInfo() { return CLS(); }

#define WX_DECLARE_TYPEINFO_INLINE(CLS) \
private: \
    static CLS sm_wxClassInfo() { return CLS(); } \
_WX_DECLARE_TYPEINFO_CUSTOM(CLS, sm_wxClassInfo)

#define wxTypeId(OBJ) (OBJ).GetWxTypeId()

#define WX_DECLARE_ABSTRACT_TYPEINFO(CLS) \
public: \
    virtual wxTypeIdentifier GetWxTypeId() const = 0;

#endif 
#endif 