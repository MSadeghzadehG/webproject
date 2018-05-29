
#ifndef _WX_CPPUNIT_H_
#define _WX_CPPUNIT_H_

#ifdef __VISUALC__
        #pragma warning(disable:4097)

        #pragma warning(disable:4702)

        #pragma warning(disable:4786)
#endif 
#ifdef __BORLANDC__
    #pragma warn -8022
#endif

#ifndef CPPUNIT_STD_NEED_ALLOCATOR
    #define CPPUNIT_STD_NEED_ALLOCATOR 0
#endif


#ifndef CPPUNIT_COMPILER_LOCATION_FORMAT
    #define CPPUNIT_COMPILER_LOCATION_FORMAT "%p:%l:"
#endif



#include "wx/beforestd.h"
#ifdef __VISUALC__
    #pragma warning(push)

            #pragma warning(disable:4701)

            #pragma warning(disable:4100)
#endif

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/CompilerOutputter.h>

#ifdef __VISUALC__
    #pragma warning(pop)
#endif
#include "wx/afterstd.h"

#include "wx/string.h"



#define WXTEST_ANY_WITH_CONDITION(suiteName, Condition, testMethod, anyTest) \
    if (Condition) \
        { anyTest; } \
    else \
        wxLogInfo(wxString::Format(wxT("skipping: %s.%s\n  reason: %s equals false\n"), \
                                    wxString(suiteName, wxConvUTF8).c_str(), \
                                    wxString(#testMethod, wxConvUTF8).c_str(), \
                                    wxString(#Condition, wxConvUTF8).c_str()))

#define WXTEST_WITH_CONDITION(suiteName, Condition, testMethod) \
    WXTEST_ANY_WITH_CONDITION(suiteName, Condition, testMethod, CPPUNIT_TEST(testMethod))
#define WXTEST_FAIL_WITH_CONDITION(suiteName, Condition, testMethod) \
    WXTEST_ANY_WITH_CONDITION(suiteName, Condition, testMethod, CPPUNIT_TEST_FAIL(testMethod))

CPPUNIT_NS_BEGIN

inline void
assertEquals(const char *expected,
             const char *actual,
             CppUnit::SourceLine sourceLine,
             const std::string& message)
{
    assertEquals(wxString(expected), wxString(actual), sourceLine, message);
}

inline void
assertEquals(const char *expected,
             const wxString& actual,
             CppUnit::SourceLine sourceLine,
             const std::string& message)
{
    assertEquals(wxString(expected), actual, sourceLine, message);
}

inline void
assertEquals(const wxString& expected,
             const char *actual,
             CppUnit::SourceLine sourceLine,
             const std::string& message)
{
    assertEquals(expected, wxString(actual), sourceLine, message);
}

inline void
assertEquals(const wchar_t *expected,
             const wxString& actual,
             CppUnit::SourceLine sourceLine,
             const std::string& message)
{
    assertEquals(wxString(expected), actual, sourceLine, message);
}

inline void
assertEquals(const wxString& expected,
             const wchar_t *actual,
             CppUnit::SourceLine sourceLine,
             const std::string& message)
{
    assertEquals(expected, wxString(actual), sourceLine, message);
}

CPPUNIT_NS_END

#define WX_CPPUNIT_ASSERT_EQUALS(T1, T2)                                      \
    inline void                                                               \
    assertEquals(T1 expected,                                                 \
                 T2 actual,                                                   \
                 CppUnit::SourceLine sourceLine,                              \
                 const std::string& message)                                  \
    {                                                                         \
        if ( !assertion_traits<T1>::equal(expected,actual) )                  \
        {                                                                     \
            Asserter::failNotEqual( assertion_traits<T1>::toString(expected), \
                                    assertion_traits<T2>::toString(actual),   \
                                    sourceLine,                               \
                                    message );                                \
        }                                                                     \
    }

#define WX_CPPUNIT_ALLOW_EQUALS_TO_INT(T) \
    CPPUNIT_NS_BEGIN \
        WX_CPPUNIT_ASSERT_EQUALS(int, T) \
        WX_CPPUNIT_ASSERT_EQUALS(T, int) \
    CPPUNIT_NS_END

WX_CPPUNIT_ALLOW_EQUALS_TO_INT(long)
WX_CPPUNIT_ALLOW_EQUALS_TO_INT(short)
WX_CPPUNIT_ALLOW_EQUALS_TO_INT(unsigned)
WX_CPPUNIT_ALLOW_EQUALS_TO_INT(unsigned long)

#if defined( __VMS ) && defined( __ia64 )
WX_CPPUNIT_ALLOW_EQUALS_TO_INT(std::basic_streambuf<char>::pos_type);
#endif

#ifdef wxHAS_LONG_LONG_T_DIFFERENT_FROM_LONG
WX_CPPUNIT_ALLOW_EQUALS_TO_INT(wxLongLong_t)
WX_CPPUNIT_ALLOW_EQUALS_TO_INT(unsigned wxLongLong_t)
#endif 
#define WX_ASSERT_STRARRAY_EQUAL(s, a)                                        \
    {                                                                         \
        wxArrayString expected(wxSplit(s, '|', '\0'));                        \
                                                                              \
        CPPUNIT_ASSERT_EQUAL( expected.size(), a.size() );                    \
                                                                              \
        for ( size_t n = 0; n < a.size(); n++ )                               \
        {                                                                     \
            CPPUNIT_ASSERT_EQUAL( expected[n], a[n] );                        \
        }                                                                     \
    }

#define WX_ASSERT_MESSAGE(msg, cond) \
    CPPUNIT_ASSERT_MESSAGE(std::string(wxString::Format msg .mb_str()), (cond))

#define WX_ASSERT_EQUAL_MESSAGE(msg, expected, actual) \
    CPPUNIT_ASSERT_EQUAL_MESSAGE(std::string(wxString::Format msg .mb_str()), \
                                 (expected), (actual))

#if !wxUSE_STD_IOSTREAM

#include "wx/string.h"

#include <iostream>

inline std::ostream& operator<<(std::ostream& o, const wxString& s)
{
#if wxUSE_UNICODE
    return o << (const char *)wxSafeConvertWX2MB(s.wc_str());
#else
    return o << s.c_str();
#endif
}

#endif 

#ifdef __BORLANDC__
    #pragma warn .8022
#endif

#ifdef _MSC_VER
  #pragma warning(default:4702)
#endif 
#ifdef __VISUALC__
  #ifdef NDEBUG
    #pragma comment(lib, "cppunit.lib")
  #else     #pragma comment(lib, "cppunitd.lib")
  #endif #endif

#endif 
