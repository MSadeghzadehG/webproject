


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_LOG

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/arrstr.h"
    #include "wx/intl.h"
    #include "wx/string.h"
    #include "wx/utils.h"
#endif 
#include "wx/apptrait.h"
#include "wx/datetime.h"
#include "wx/file.h"
#include "wx/msgout.h"
#include "wx/textfile.h"
#include "wx/thread.h"
#include "wx/private/threadinfo.h"
#include "wx/crt.h"
#include "wx/vector.h"

#include <errno.h>

#include <stdlib.h>

#include <time.h>

#if defined(__WINDOWS__)
    #include "wx/msw/private.h" #endif

#undef wxLOG_COMPONENT
const char *wxLOG_COMPONENT = "";

#define WX_DEFINE_GLOBAL_VAR(type, name)                                      \
    inline type& Get##name()                                                  \
    {                                                                         \
        static type s_##name;                                                 \
        return s_##name;                                                      \
    }                                                                         \
                                                                              \
    type *gs_##name##Ptr = &Get##name()

#if wxUSE_THREADS

namespace
{

typedef wxVector<wxLogRecord> wxLogRecords;
wxLogRecords gs_bufferedLogRecords;

#define WX_DEFINE_LOG_CS(name) WX_DEFINE_GLOBAL_VAR(wxCriticalSection, name##CS)

WX_DEFINE_LOG_CS(BackgroundLog);

WX_DEFINE_LOG_CS(TraceMask);

WX_DEFINE_LOG_CS(Levels);

} 
#endif 


#ifdef  LOG_PRETTY_WRAP
  static void wxLogWrap(FILE *f, const char *pszPrefix, const char *psz);
#endif


namespace
{

struct PreviousLogInfo
{
    PreviousLogInfo()
    {
        numRepeated = 0;
    }


        wxString msg;

        wxLogLevel level;

        wxLogRecordInfo info;

        unsigned numRepeated;
};

PreviousLogInfo gs_prevLog;


WX_DEFINE_GLOBAL_VAR(wxStringToNumHashMap, ComponentLevels);


class wxLogOutputBest : public wxLog
{
public:
    wxLogOutputBest() { }

protected:
    virtual void DoLogText(const wxString& msg) wxOVERRIDE
    {
        wxMessageOutputBest().Output(msg);
    }

private:
    wxDECLARE_NO_COPY_CLASS(wxLogOutputBest);
};

} 


void wxSafeShowMessage(const wxString& title, const wxString& text)
{
#ifdef __WINDOWS__
    ::MessageBox(NULL, text.t_str(), title.t_str(), MB_OK | MB_ICONSTOP);
#else
    wxFprintf(stderr, wxS("%s: %s\n"), title.c_str(), text.c_str());
    fflush(stderr);
#endif
}


wxString
wxLogFormatter::Format(wxLogLevel level,
                       const wxString& msg,
                       const wxLogRecordInfo& info) const
{
    wxString prefix;

        #ifdef __WINDOWS__
    if ( level != wxLOG_Debug && level != wxLOG_Trace )
#endif         prefix = FormatTime(info.timestamp);

    switch ( level )
    {
    case wxLOG_Error:
        prefix += _("Error: ");
        break;

    case wxLOG_Warning:
        prefix += _("Warning: ");
        break;

                #ifndef __WINDOWS__
    case wxLOG_Debug:
                        prefix += "Debug: ";
        break;

    case wxLOG_Trace:
        prefix += "Trace: ";
        break;
#endif     }

    return prefix + msg;
}

wxString
wxLogFormatter::FormatTime(time_t t) const
{
    wxString str;
    wxLog::TimeStamp(&str, t);

    return str;
}



unsigned wxLog::LogLastRepeatIfNeeded()
{
    const unsigned count = gs_prevLog.numRepeated;

    if ( gs_prevLog.numRepeated )
    {
        wxString msg;
#if wxUSE_INTL
        if ( gs_prevLog.numRepeated == 1 )
        {
                                    msg = _("The previous message repeated once.");
        }
        else
        {
                                                msg.Printf(wxPLURAL("The previous message repeated %u time.",
                                "The previous message repeated %u times.",
                                gs_prevLog.numRepeated),
                       gs_prevLog.numRepeated);
        }
#else
        msg.Printf(wxS("The previous message was repeated %u time(s)."),
                   gs_prevLog.numRepeated);
#endif
        gs_prevLog.numRepeated = 0;
        gs_prevLog.msg.clear();
        DoLogRecord(gs_prevLog.level, msg, gs_prevLog.info);
    }

    return count;
}

wxLog::~wxLog()
{
            if ( gs_prevLog.numRepeated )
    {
        wxMessageOutputDebug().Printf
        (
#if wxUSE_INTL
            wxPLURAL
            (
                "Last repeated message (\"%s\", %u time) wasn't output",
                "Last repeated message (\"%s\", %u times) wasn't output",
                gs_prevLog.numRepeated
            ),
#else
            wxS("Last repeated message (\"%s\", %u time(s)) wasn't output"),
#endif
            gs_prevLog.msg,
            gs_prevLog.numRepeated
        );
    }

    delete m_formatter;
}



void
wxLog::OnLog(wxLogLevel level, const wxString& msg, time_t t)
{
    wxLogRecordInfo info;
    info.timestamp = t;
#if wxUSE_THREADS
    info.threadId = wxThread::GetCurrentId();
#endif 
    OnLog(level, msg, info);
}


void
wxLog::OnLog(wxLogLevel level,
             const wxString& msg,
             const wxLogRecordInfo& info)
{
            if ( level == wxLOG_FatalError )
    {
        wxSafeShowMessage(wxS("Fatal Error"), msg);

        wxAbort();
    }

    wxLog *logger;

#if wxUSE_THREADS
    if ( !wxThread::IsMain() )
    {
        logger = wxThreadInfo.logger;
        if ( !logger )
        {
            if ( ms_pLogger )
            {
                                                wxCriticalSectionLocker lock(GetBackgroundLogCS());

                gs_bufferedLogRecords.push_back(wxLogRecord(level, msg, info));

                                wxWakeUpIdle();
            }
                        
            return;
        }
                    }
    else
#endif     {
        logger = GetMainThreadActiveTarget();
        if ( !logger )
            return;
    }

    logger->CallDoLogNow(level, msg, info);
}

void
wxLog::CallDoLogNow(wxLogLevel level,
                    const wxString& msg,
                    const wxLogRecordInfo& info)
{
    if ( GetRepetitionCounting() )
    {
        if ( msg == gs_prevLog.msg )
        {
            gs_prevLog.numRepeated++;

                                    return;
        }

        LogLastRepeatIfNeeded();

                gs_prevLog.msg = msg;
        gs_prevLog.level = level;
        gs_prevLog.info = info;
    }

        wxString prefix, suffix;
    wxUIntPtr num = 0;
    if ( info.GetNumValue(wxLOG_KEY_SYS_ERROR_CODE, &num) )
    {
        const long err = static_cast<long>(num);

        suffix.Printf(_(" (error %ld: %s)"), err, wxSysErrorMsg(err));
    }

#if wxUSE_LOG_TRACE
    wxString str;
    if ( level == wxLOG_Trace && info.GetStrValue(wxLOG_KEY_TRACE_MASK, &str) )
    {
        prefix = "(" + str + ") ";
    }
#endif 
    DoLogRecord(level, prefix + msg + suffix, info);
}

void wxLog::DoLogRecord(wxLogLevel level,
                             const wxString& msg,
                             const wxLogRecordInfo& info)
{
#if WXWIN_COMPATIBILITY_2_8
                            DoLog(level, (const char*)msg.mb_str(), info.timestamp);
    DoLog(level, (const wchar_t*)msg.wc_str(), info.timestamp);
#else     wxUnusedVar(info);
#endif 
        DoLogTextAtLevel(level, m_formatter->Format (level, msg, info));
}

void wxLog::DoLogTextAtLevel(wxLogLevel level, const wxString& msg)
{
                if ( level == wxLOG_Debug || level == wxLOG_Trace )
    {
        wxMessageOutputDebug().Output(msg + wxS('\n'));
    }
    else
    {
        DoLogText(msg);
    }
}

void wxLog::DoLogText(const wxString& WXUNUSED(msg))
{
        #if !WXWIN_COMPATIBILITY_2_8
    wxFAIL_MSG( "must be overridden if it is called" );
#endif }

#if WXWIN_COMPATIBILITY_2_8

void wxLog::DoLog(wxLogLevel WXUNUSED(level), const char *szString, time_t t)
{
    DoLogString(szString, t);
}

void wxLog::DoLog(wxLogLevel WXUNUSED(level), const wchar_t *wzString, time_t t)
{
    DoLogString(wzString, t);
}

#endif 

wxLog *wxLog::GetActiveTarget()
{
#if wxUSE_THREADS
    if ( !wxThread::IsMain() )
    {
                wxLog * const logger = wxThreadInfo.logger;

                                return logger ? logger : ms_pLogger;
    }
#endif 
    return GetMainThreadActiveTarget();
}


wxLog *wxLog::GetMainThreadActiveTarget()
{
    if ( ms_bAutoCreate && ms_pLogger == NULL ) {
                        static bool s_bInGetActiveTarget = false;
        if ( !s_bInGetActiveTarget ) {
            s_bInGetActiveTarget = true;

                        if ( wxTheApp != NULL )
                ms_pLogger = wxTheApp->GetTraits()->CreateLogTarget();
            else
                ms_pLogger = new wxLogOutputBest;

            s_bInGetActiveTarget = false;

                    }
    }

    return ms_pLogger;
}

wxLog *wxLog::SetActiveTarget(wxLog *pLogger)
{
    if ( ms_pLogger != NULL ) {
                        ms_pLogger->Flush();
    }

    wxLog *pOldLogger = ms_pLogger;
    ms_pLogger = pLogger;

    return pOldLogger;
}

#if wxUSE_THREADS

wxLog *wxLog::SetThreadActiveTarget(wxLog *logger)
{
    wxASSERT_MSG( !wxThread::IsMain(), "use SetActiveTarget() for main thread" );

    wxLog * const oldLogger = wxThreadInfo.logger;
    if ( oldLogger )
        oldLogger->Flush();

    wxThreadInfo.logger = logger;

    return oldLogger;
}
#endif 
void wxLog::DontCreateOnDemand()
{
    ms_bAutoCreate = false;

                ClearTraceMasks();
}

void wxLog::DoCreateOnDemand()
{
    ms_bAutoCreate = true;
}



void wxLog::SetComponentLevel(const wxString& component, wxLogLevel level)
{
    if ( component.empty() )
    {
        SetLogLevel(level);
    }
    else
    {
        wxCRIT_SECT_LOCKER(lock, GetLevelsCS());

        GetComponentLevels()[component] = level;
    }
}


wxLogLevel wxLog::GetComponentLevel(wxString component)
{
    wxCRIT_SECT_LOCKER(lock, GetLevelsCS());

    const wxStringToNumHashMap& componentLevels = GetComponentLevels();
    while ( !component.empty() )
    {
        wxStringToNumHashMap::const_iterator
            it = componentLevels.find(component);
        if ( it != componentLevels.end() )
            return static_cast<wxLogLevel>(it->second);

        component = component.BeforeLast('/');
    }

    return GetLogLevel();
}


namespace
{

wxArrayString& TraceMasks()
{
    static wxArrayString s_traceMasks;

    return s_traceMasks;
}

} 
 const wxArrayString& wxLog::GetTraceMasks()
{
            
    return TraceMasks();
}

void wxLog::AddTraceMask(const wxString& str)
{
    wxCRIT_SECT_LOCKER(lock, GetTraceMaskCS());

    TraceMasks().push_back(str);
}

void wxLog::RemoveTraceMask(const wxString& str)
{
    wxCRIT_SECT_LOCKER(lock, GetTraceMaskCS());

    int index = TraceMasks().Index(str);
    if ( index != wxNOT_FOUND )
        TraceMasks().RemoveAt((size_t)index);
}

void wxLog::ClearTraceMasks()
{
    wxCRIT_SECT_LOCKER(lock, GetTraceMaskCS());

    TraceMasks().Clear();
}

 bool wxLog::IsAllowedTraceMask(const wxString& mask)
{
    wxCRIT_SECT_LOCKER(lock, GetTraceMaskCS());

    const wxArrayString& masks = GetTraceMasks();
    for ( wxArrayString::const_iterator it = masks.begin(),
                                        en = masks.end();
          it != en;
          ++it )
    {
        if ( *it == mask)
            return true;
    }

    return false;
}


#if wxUSE_DATETIME

void wxLog::TimeStamp(wxString *str)
{
    if ( !ms_timestamp.empty() )
    {
        *str = wxDateTime::UNow().Format(ms_timestamp);
        *str += wxS(": ");
    }
}

void wxLog::TimeStamp(wxString *str, time_t t)
{
    if ( !ms_timestamp.empty() )
    {
        *str = wxDateTime(t).Format(ms_timestamp);
        *str += wxS(": ");
    }
}

#else 
void wxLog::TimeStamp(wxString*)
{
}

void wxLog::TimeStamp(wxString*, time_t)
{
}

#endif 
#if wxUSE_THREADS

void wxLog::FlushThreadMessages()
{
        wxLogRecords bufferedLogRecords;

    {
        wxCriticalSectionLocker lock(GetBackgroundLogCS());
        bufferedLogRecords.swap(gs_bufferedLogRecords);

                    }

    if ( !bufferedLogRecords.empty() )
    {
        for ( wxLogRecords::const_iterator it = bufferedLogRecords.begin();
              it != bufferedLogRecords.end();
              ++it )
        {
            CallDoLogNow(it->level, it->msg, it->info);
        }
    }
}


bool wxLog::IsThreadLoggingEnabled()
{
    return !wxThreadInfo.loggingDisabled;
}


bool wxLog::EnableThreadLogging(bool enable)
{
    const bool wasEnabled = !wxThreadInfo.loggingDisabled;
    wxThreadInfo.loggingDisabled = !enable;
    return wasEnabled;
}

#endif 
wxLogFormatter *wxLog::SetFormatter(wxLogFormatter* formatter)
{
    wxLogFormatter* formatterOld = m_formatter;
    m_formatter = formatter ? formatter : new wxLogFormatter;

    return formatterOld;
}

void wxLog::Flush()
{
    LogLastRepeatIfNeeded();
}


void wxLog::FlushActive()
{
    if ( ms_suspendCount )
        return;

    wxLog * const log = GetActiveTarget();
    if ( log )
    {
#if wxUSE_THREADS
        if ( wxThread::IsMain() )
            log->FlushThreadMessages();
#endif 
        log->Flush();
    }
}


void wxLogBuffer::Flush()
{
    wxLog::Flush();

    if ( !m_str.empty() )
    {
        wxMessageOutputBest out;
        out.Printf(wxS("%s"), m_str.c_str());
        m_str.clear();
    }
}

void wxLogBuffer::DoLogTextAtLevel(wxLogLevel level, const wxString& msg)
{
            switch ( level )
    {
        case wxLOG_Debug:
        case wxLOG_Trace:
            wxLog::DoLogTextAtLevel(level, msg);
            break;

        default:
            m_str << msg << wxS("\n");
    }
}


wxLogStderr::wxLogStderr(FILE *fp)
{
    if ( fp == NULL )
        m_fp = stderr;
    else
        m_fp = fp;
}

void wxLogStderr::DoLogText(const wxString& msg)
{
                wxMessageOutputStderr(m_fp).Output(msg);

                    if ( m_fp == stderr )
    {
        wxAppTraits *traits = wxTheApp ? wxTheApp->GetTraits() : NULL;
        if ( traits && !traits->HasStderr() )
        {
            wxMessageOutputDebug().Output(msg + wxS('\n'));
        }
    }
}


#if wxUSE_STD_IOSTREAM
#include "wx/ioswrap.h"
wxLogStream::wxLogStream(wxSTD ostream *ostr)
{
    if ( ostr == NULL )
        m_ostr = &wxSTD cerr;
    else
        m_ostr = ostr;
}

void wxLogStream::DoLogText(const wxString& msg)
{
    (*m_ostr) << msg << wxSTD endl;
}
#endif 

wxLogChain::wxLogChain(wxLog *logger)
{
    m_bPassMessages = true;

    m_logNew = logger;

                        m_logOld = wxLog::GetActiveTarget();
    wxLog::SetActiveTarget(this);
}

wxLogChain::~wxLogChain()
{
    wxLog::SetActiveTarget(m_logOld);

    if ( m_logNew != this )
        delete m_logNew;
}

void wxLogChain::SetLog(wxLog *logger)
{
    if ( m_logNew != this )
        delete m_logNew;

    m_logNew = logger;
}

void wxLogChain::Flush()
{
    if ( m_logOld )
        m_logOld->Flush();

        if ( m_logNew && m_logNew != this )
        m_logNew->Flush();
}

void wxLogChain::DoLogRecord(wxLogLevel level,
                             const wxString& msg,
                             const wxLogRecordInfo& info)
{
        if ( m_logOld && IsPassingMessages() )
        m_logOld->LogRecord(level, msg, info);

        if ( m_logNew )
    {
                        if ( m_logNew != this )
            m_logNew->LogRecord(level, msg, info);
        else
            wxLog::DoLogRecord(level, msg, info);
    }
}

#ifdef __VISUALC__
        #pragma warning(disable:4355)
#endif 

wxLogInterposer::wxLogInterposer()
                : wxLogChain(this)
{
}


wxLogInterposerTemp::wxLogInterposerTemp()
                : wxLogChain(this)
{
    DetachOldLog();
}

#ifdef __VISUALC__
    #pragma warning(default:4355)
#endif 


bool            wxLog::ms_bRepetCounting = false;

wxLog          *wxLog::ms_pLogger      = NULL;
bool            wxLog::ms_doLog        = true;
bool            wxLog::ms_bAutoCreate  = true;
bool            wxLog::ms_bVerbose     = false;

wxLogLevel      wxLog::ms_logLevel     = wxLOG_Max;  
size_t          wxLog::ms_suspendCount = 0;

wxString        wxLog::ms_timestamp(wxS("%X"));  
#if WXWIN_COMPATIBILITY_2_8
wxTraceMask     wxLog::ms_ulTraceMask  = (wxTraceMask)0;
#endif 

#ifdef LOG_PRETTY_WRAP
static void wxLogWrap(FILE *f, const char *pszPrefix, const char *psz)
{
    size_t nMax = 80;     size_t nStart = strlen(pszPrefix);
    fputs(pszPrefix, f);

    size_t n;
    while ( *psz != '\0' ) {
        for ( n = nStart; (n < nMax) && (*psz != '\0'); n++ )
            putc(*psz++, f);

                if ( *psz != '\0' ) {
            
            for ( n = 0; n < nStart; n++ )
                putc(' ', f);

                        while ( isspace(*psz) )
                psz++;
        }
    }

    putc('\n', f);
}
#endif  

unsigned long wxSysErrorCode()
{
#if defined(__WINDOWS__)
    return ::GetLastError();
#else       return errno;
#endif  }

const wxChar *wxSysErrorMsg(unsigned long nErrCode)
{
    if ( nErrCode == 0 )
        nErrCode = wxSysErrorCode();

#if defined(__WINDOWS__)
    static wxChar s_szBuf[1024];

        LPVOID lpMsgBuf;
    if ( ::FormatMessage
         (
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            nErrCode,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0,
            NULL
         ) == 0 )
    {
                        wxSprintf(s_szBuf, wxS("unknown error %lx"), nErrCode);
        return s_szBuf;
    }


            if( lpMsgBuf != 0 )
    {
        wxStrlcpy(s_szBuf, (const wxChar *)lpMsgBuf, WXSIZEOF(s_szBuf));

        LocalFree(lpMsgBuf);

                s_szBuf[0] = (wxChar)wxTolower(s_szBuf[0]);
        size_t len = wxStrlen(s_szBuf);
        if ( len > 0 ) {
                        if ( s_szBuf[len - 2] == wxS('\r') )
                s_szBuf[len - 2] = wxS('\0');
        }
    }
    else
    {
        s_szBuf[0] = wxS('\0');
    }

    return s_szBuf;
#else     #if wxUSE_UNICODE
        static wchar_t s_wzBuf[1024];
        wxConvCurrent->MB2WC(s_wzBuf, strerror((int)nErrCode),
                             WXSIZEOF(s_wzBuf) - 1);
        return s_wzBuf;
    #else
        return strerror((int)nErrCode);
    #endif
#endif  }

#endif 