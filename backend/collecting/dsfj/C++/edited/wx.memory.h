
#ifndef _WX_MEMORY_H_
#define _WX_MEMORY_H_

#include "wx/defs.h"
#include "wx/string.h"
#include "wx/msgout.h"

#if wxUSE_MEMORY_TRACING || wxUSE_DEBUG_CONTEXT

#include <stddef.h>

WXDLLIMPEXP_BASE void * wxDebugAlloc(size_t size, wxChar * fileName, int lineNum, bool isObject, bool isVect = false);
WXDLLIMPEXP_BASE void wxDebugFree(void * buf, bool isVect = false);





#if wxUSE_GLOBAL_MEMORY_OPERATORS

#ifdef new
    #undef new
#endif

#if defined(__SUNCC__)
    #define wxUSE_ARRAY_MEMORY_OPERATORS 0
#elif defined (__SGI_CC_)
        #ifndef __EDG_ABI_COMPATIBILITY_VERSION
        #define wxUSE_ARRAY_MEMORY_OPERATORS 0
    #endif
#else
    #define wxUSE_ARRAY_MEMORY_OPERATORS 1
#endif

#if defined(__WINDOWS__) && (defined(WXUSINGDLL) || defined(WXMAKINGDLL_BASE))
inline void * operator new (size_t size, wxChar * fileName, int lineNum)
{
    return wxDebugAlloc(size, fileName, lineNum, false, false);
}

inline void * operator new (size_t size)
{
    return wxDebugAlloc(size, NULL, 0, false);
}

inline void operator delete (void * buf)
{
    wxDebugFree(buf, false);
}

#if wxUSE_ARRAY_MEMORY_OPERATORS
inline void * operator new[] (size_t size)
{
    return wxDebugAlloc(size, NULL, 0, false, true);
}

inline void * operator new[] (size_t size, wxChar * fileName, int lineNum)
{
    return wxDebugAlloc(size, fileName, lineNum, false, true);
}

inline void operator delete[] (void * buf)
{
    wxDebugFree(buf, true);
}
#endif 
#else

void * operator new (size_t size, wxChar * fileName, int lineNum);

void * operator new (size_t size);

void operator delete (void * buf);

#if wxUSE_ARRAY_MEMORY_OPERATORS
void * operator new[] (size_t size);

void * operator new[] (size_t size, wxChar * fileName, int lineNum);

void operator delete[] (void * buf);
#endif #endif 
#if defined(__VISUALC__)
inline void operator delete(void* pData, wxChar* , int )
{
    wxDebugFree(pData, false);
}
inline void operator delete[](void* pData, wxChar* , int )
{
    wxDebugFree(pData, true);
}
#endif #endif 

typedef unsigned int wxMarkerType;



class WXDLLIMPEXP_BASE wxMemStruct {

friend class WXDLLIMPEXP_FWD_BASE wxDebugContext; 
public:
public:
    int AssertList ();

    size_t RequestSize () { return m_reqSize; }
    wxMarkerType Marker () { return m_firstMarker; }

        inline void SetDeleted ();
    inline int IsDeleted ();

    int Append ();
    int Unlink ();

            int AssertIt ();

        int ValidateNode ();

        int CheckBlock ();
    int CheckAllPrevious ();

        void PrintNode ();

        void ErrorMsg (const char *);
    void ErrorMsg ();

    inline void *GetActualData(void) const { return m_actualData; }

    void Dump(void);

public:
            wxMarkerType        m_firstMarker;

        wxChar*             m_fileName;
    int                 m_lineNum;

        size_t              m_reqSize;

                wxMarkerType        m_id;

    wxMemStruct *       m_prev;
    wxMemStruct *       m_next;

    void *              m_actualData;
    bool                m_isObject;
};


typedef void (wxMemStruct::*PmSFV) ();

typedef void (*wxShutdownNotifyFunction)();



class WXDLLIMPEXP_BASE wxDebugContext {

protected:
        static size_t CalcAlignment ();

                static size_t GetPadding (size_t size) ;

        static void TraverseList (PmSFV, wxMemStruct *from = NULL);

    static int debugLevel;
    static bool debugOn;

    static int m_balign;                static int m_balignmask;        public:
            static wxMemStruct *checkPoint;

    wxDebugContext(void);
    ~wxDebugContext(void);

    static int GetLevel(void) { return debugLevel; }
    static void SetLevel(int level) { debugLevel = level; }

    static bool GetDebugMode(void) { return debugOn; }
    static void SetDebugMode(bool flag) { debugOn = flag; }

    static void SetCheckpoint(bool all = false);
    static wxMemStruct *GetCheckpoint(void) { return checkPoint; }

            static size_t PaddedSize (size_t reqSize);

            static size_t TotSize (size_t reqSize);

        static char * StructPos (const char * buf);
    static char * MidMarkerPos (const char * buf);
    static char * CallerMemPos (const char * buf);
    static char * EndMarkerPos (const char * buf, size_t size);

            static char * StartPos (const char * caller);

        static wxMemStruct * GetHead () { return m_head; }
    static wxMemStruct * GetTail () { return m_tail; }

        static wxMemStruct * SetHead (wxMemStruct * st) { return (m_head = st); }
    static wxMemStruct * SetTail (wxMemStruct * st) { return (m_tail = st); }

            static bool GetCheckPrevious () { return m_checkPrevious; }
    static void SetCheckPrevious (bool value) { m_checkPrevious = value; }

        static int Check(bool checkAll = false);

        static bool PrintList(void);

        static bool Dump(void);

        static bool PrintStatistics(bool detailed = true);

        static bool PrintClasses(void);

            static int CountObjectsLeft(bool sinceCheckpoint = false);

        static void OutputDumpLine(const wxChar *szFormat, ...);

    static void SetShutdownNotifyFunction(wxShutdownNotifyFunction shutdownFn);

private:
            static wxMemStruct*         m_head;
    static wxMemStruct*         m_tail;

            static bool                 m_checkPrevious;

        static wxShutdownNotifyFunction sm_shutdownFn;

        friend class wxDebugContextDumpDelayCounter;
};


class WXDLLIMPEXP_BASE wxDebugContextDumpDelayCounter
{
public:
    wxDebugContextDumpDelayCounter();
    ~wxDebugContextDumpDelayCounter();

private:
    void DoDump();
    static int sm_count;
};

static wxDebugContextDumpDelayCounter wxDebugContextDumpDelayCounter_File;
#define WXDEBUG_DUMPDELAYCOUNTER \
    static wxDebugContextDumpDelayCounter wxDebugContextDumpDelayCounter_Extra;

void WXDLLIMPEXP_BASE wxTrace(const wxChar *fmt ...) WX_ATTRIBUTE_PRINTF_1;
void WXDLLIMPEXP_BASE wxTraceLevel(int level, const wxChar *fmt ...) WX_ATTRIBUTE_PRINTF_2;

#define WXTRACE wxTrace
#define WXTRACELEVEL wxTraceLevel

#else 
#define WXDEBUG_DUMPDELAYCOUNTER

#if 0
    inline void wxTrace(const wxChar *WXUNUSED(fmt)) {}
    inline void wxTraceLevel(int WXUNUSED(level), const wxChar *WXUNUSED(fmt)) {}
#else
    #define wxTrace(fmt)
    #define wxTraceLevel(l, fmt)
#endif

#define WXTRACE true ? (void)0 : wxTrace
#define WXTRACELEVEL true ? (void)0 : wxTraceLevel

#endif 
#endif 