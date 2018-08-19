
#ifndef _WX_DEBUG_H_
#define _WX_DEBUG_H_

#include  <assert.h>

#include <limits.h>          
#include "wx/chartype.h"     #include "wx/cpp.h"          #include "wx/dlimpexp.h"     
class WXDLLIMPEXP_FWD_BASE wxString;
class WXDLLIMPEXP_FWD_BASE wxCStrData;




#if !defined(wxDEBUG_LEVEL)
    #define wxDEBUG_LEVEL 1
#endif 

#if wxDEBUG_LEVEL > 0
    #ifndef __WXDEBUG__
        #define __WXDEBUG__
    #endif
#else
    #undef __WXDEBUG__
#endif

#ifdef __WXDEBUG__
    #if !defined(WXDEBUG) || !WXDEBUG
        #undef WXDEBUG
        #define WXDEBUG 1
    #endif #endif 


typedef void (*wxAssertHandler_t)(const wxString& file,
                                  int line,
                                  const wxString& func,
                                  const wxString& cond,
                                  const wxString& msg);

#if wxDEBUG_LEVEL

extern WXDLLIMPEXP_DATA_BASE(wxAssertHandler_t) wxTheAssertHandler;


inline wxAssertHandler_t wxSetAssertHandler(wxAssertHandler_t handler)
{
    const wxAssertHandler_t old = wxTheAssertHandler;
    wxTheAssertHandler = handler;
    return old;
}


extern void WXDLLIMPEXP_BASE wxSetDefaultAssertHandler();

#else 
inline wxAssertHandler_t wxSetAssertHandler(wxAssertHandler_t )
{
    return NULL;
}

inline void wxSetDefaultAssertHandler() { }

#endif 
inline void wxDisableAsserts() { wxSetAssertHandler(NULL); }


#ifdef NDEBUG
    #define wxDISABLE_ASSERTS_IN_RELEASE_BUILD() wxDisableAsserts()
#else
    #define wxDISABLE_ASSERTS_IN_RELEASE_BUILD()
#endif

#if wxDEBUG_LEVEL



#if wxUSE_UNICODE

extern WXDLLIMPEXP_BASE void wxOnAssert(const char *file,
                                        int line,
                                        const char *func,
                                        const char *cond);

extern WXDLLIMPEXP_BASE void wxOnAssert(const char *file,
                                        int line,
                                        const char *func,
                                        const char *cond,
                                        const char *msg);

extern WXDLLIMPEXP_BASE void wxOnAssert(const char *file,
                                        int line,
                                        const char *func,
                                        const char *cond,
                                        const wxChar *msg) ;
#endif 

extern WXDLLIMPEXP_BASE void wxOnAssert(const wxChar *file,
                                        int line,
                                        const char *func,
                                        const wxChar *cond,
                                        const wxChar *msg = NULL);

extern WXDLLIMPEXP_BASE void wxOnAssert(const wxString& file,
                                        int line,
                                        const wxString& func,
                                        const wxString& cond,
                                        const wxString& msg);

extern WXDLLIMPEXP_BASE void wxOnAssert(const wxString& file,
                                        int line,
                                        const wxString& func,
                                        const wxString& cond);

extern WXDLLIMPEXP_BASE void wxOnAssert(const char *file,
                                        int line,
                                        const char *func,
                                        const char *cond,
                                        const wxCStrData& msg);

extern WXDLLIMPEXP_BASE void wxOnAssert(const char *file,
                                        int line,
                                        const char *func,
                                        const char *cond,
                                        const wxString& msg);

#endif 



#if wxDEBUG_LEVEL
                            #ifdef __VISUALC__
        #define wxTrap() __debugbreak()
    #elif defined(__GNUC__)
        #if defined(__i386) || defined(__x86_64)
            #define wxTrap() asm volatile ("int $3")
        #endif
    #endif

    #ifndef wxTrap
                extern WXDLLIMPEXP_BASE void wxTrap();
    #endif

                extern WXDLLIMPEXP_DATA_BASE(bool) wxTrapInAssert;

                                    #define wxASSERT_MSG_AT(cond, msg, file, line, func)                      \
        wxSTATEMENT_MACRO_BEGIN                                               \
            if ( wxTheAssertHandler && !(cond) &&                             \
                    (wxOnAssert(file, line, func, #cond, msg),                \
                     wxTrapInAssert) )                                        \
            {                                                                 \
                wxTrapInAssert = false;                                       \
                wxTrap();                                                     \
            }                                                                 \
        wxSTATEMENT_MACRO_END

        #define wxASSERT_MSG(cond, msg) \
        wxASSERT_MSG_AT(cond, msg, __FILE__, __LINE__, __WXFUNCTION__)

            #define wxASSERT(cond) wxASSERT_MSG(cond, (const char*)NULL)

            #define wxFAIL_COND_MSG_AT(cond, msg, file, line, func)                   \
        wxSTATEMENT_MACRO_BEGIN                                               \
            if ( wxTheAssertHandler &&                                        \
                    (wxOnAssert(file, line, func, #cond, msg),                \
                     wxTrapInAssert) )                                        \
            {                                                                 \
                wxTrapInAssert = false;                                       \
                wxTrap();                                                     \
            }                                                                 \
        wxSTATEMENT_MACRO_END

    #define wxFAIL_MSG_AT(msg, file, line, func) \
        wxFAIL_COND_MSG_AT("Assert failure", msg, file, line, func)

    #define wxFAIL_COND_MSG(cond, msg) \
        wxFAIL_COND_MSG_AT(cond, msg, __FILE__, __LINE__, __WXFUNCTION__)

    #define wxFAIL_MSG(msg) wxFAIL_COND_MSG("Assert failure", msg)
    #define wxFAIL wxFAIL_MSG((const char*)NULL)
#else     #define wxTrap()

    #define wxASSERT(cond)
    #define wxASSERT_MSG(cond, msg)
    #define wxFAIL
    #define wxFAIL_MSG(msg)
    #define wxFAIL_COND_MSG(cond, msg)
#endif  
#if wxDEBUG_LEVEL >= 2
    #define wxASSERT_LEVEL_2_MSG(cond, msg) wxASSERT_MSG(cond, msg)
    #define wxASSERT_LEVEL_2(cond) wxASSERT(cond)
#else     #define wxASSERT_LEVEL_2_MSG(cond, msg)
    #define wxASSERT_LEVEL_2(cond)
#endif

extern void WXDLLIMPEXP_BASE wxAbort();



#define wxCHECK2_MSG(cond, op, msg)                                       \
    if ( cond )                                                           \
    {}                                                                    \
    else                                                                  \
    {                                                                     \
        wxFAIL_COND_MSG(#cond, msg);                                      \
        op;                                                               \
    }                                                                     \
    struct wxDummyCheckStruct 

#define wxCHECK_MSG(cond, rc, msg)   wxCHECK2_MSG(cond, return rc, msg)

#define wxCHECK(cond, rc)            wxCHECK_MSG(cond, rc, (const char*)NULL)

#define wxCHECK2(cond, op)           wxCHECK2_MSG(cond, op, (const char*)NULL)

#define wxCHECK_RET(cond, msg)       wxCHECK2_MSG(cond, return, msg)





#define wxMAKE_UNIQUE_ASSERT_NAME           wxMAKE_UNIQUE_NAME(wxAssert_)


#if defined( __VMS )
namespace wxdebug{

template <bool x> struct STATIC_ASSERTION_FAILURE;

template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };

template<int x> struct static_assert_test{};

}
    #define WX_JOIN( X, Y ) X##Y
    #define WX_STATIC_ASSERT_BOOL_CAST(x) (bool)(x)
    #define wxCOMPILE_TIME_ASSERT(expr, msg) \
       typedef ::wxdebug::static_assert_test<\
          sizeof(::wxdebug::STATIC_ASSERTION_FAILURE< WX_STATIC_ASSERT_BOOL_CAST( expr ) >)>\
            WX_JOIN(wx_static_assert_typedef_, __LINE__)
#else
    #define wxCOMPILE_TIME_ASSERT(expr, msg) \
        struct wxMAKE_UNIQUE_ASSERT_NAME { unsigned int msg: expr; }
#endif


#define wxMAKE_UNIQUE_ASSERT_NAME2(text) wxCONCAT(wxAssert_, text)

#define wxCOMPILE_TIME_ASSERT2(expr, msg, text) \
    struct wxMAKE_UNIQUE_ASSERT_NAME2(text) { unsigned int msg: expr; }

#define wxMAKE_BITSIZE_MSG(type, size) type ## SmallerThan ## size ## Bits

#define wxASSERT_MIN_BITSIZE(type, size) \
    wxCOMPILE_TIME_ASSERT(sizeof(type) * CHAR_BIT >= size, \
                          wxMAKE_BITSIZE_MSG(type, size))




#if defined(__WIN32__)
    extern bool WXDLLIMPEXP_BASE wxIsDebuggerRunning();
#else     inline bool wxIsDebuggerRunning() { return false; }
#endif 
extern bool WXDLLIMPEXP_BASE wxAssertIsEqual(int x, int y);

extern WXDLLIMPEXP_DATA_BASE(const bool) wxFalse;

#define wxAssertFailure wxFalse

#if wxDEBUG_LEVEL
    #define WXUNUSED_UNLESS_DEBUG(param)  param
#else
    #define WXUNUSED_UNLESS_DEBUG(param)  WXUNUSED(param)
#endif


#endif 