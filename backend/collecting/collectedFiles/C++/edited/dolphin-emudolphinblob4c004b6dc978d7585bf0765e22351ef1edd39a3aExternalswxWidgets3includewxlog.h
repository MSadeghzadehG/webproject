
#ifndef _WX_LOG_H_
#define _WX_LOG_H_

#include "wx/defs.h"
#include "wx/cpp.h"


typedef unsigned long wxLogLevel;

#if WXWIN_COMPATIBILITY_2_8
    #define wxTraceMemAlloc 0x0001      #define wxTraceMessages 0x0002      #define wxTraceResAlloc 0x0004      #define wxTraceRefCount 0x0008  
    #ifdef  __WINDOWS__
        #define wxTraceOleCalls 0x0100      #endif

    typedef unsigned long wxTraceMask;
#endif 

#include "wx/string.h"
#include "wx/strvararg.h"


class WXDLLIMPEXP_FWD_BASE wxObject;

#if wxUSE_GUI
    class WXDLLIMPEXP_FWD_CORE wxFrame;
#endif 
#if wxUSE_LOG

#include "wx/arrstr.h"

#include <time.h>   
#include "wx/dynarray.h"
#include "wx/hashmap.h"

#if wxUSE_THREADS
    #include "wx/thread.h"
#endif 
#ifndef wxUSE_LOG_DEBUG
    #if wxDEBUG_LEVEL
        #define wxUSE_LOG_DEBUG 1
    #else         #define wxUSE_LOG_DEBUG 0
    #endif
#endif

#ifndef wxUSE_LOG_TRACE
    #if wxDEBUG_LEVEL
        #define wxUSE_LOG_TRACE 1
    #else         #define wxUSE_LOG_TRACE 0
    #endif
#endif 
#ifndef wxLOG_COMPONENT
            extern WXDLLIMPEXP_DATA_BASE(const char *) wxLOG_COMPONENT;

    #ifdef WXBUILDING
        #define wxLOG_COMPONENT "wx"
    #endif
#endif


enum wxLogLevelValues
{
    wxLOG_FatalError,     wxLOG_Error,          wxLOG_Warning,        wxLOG_Message,        wxLOG_Status,         wxLOG_Info,           wxLOG_Debug,          wxLOG_Trace,          wxLOG_Progress,       wxLOG_User = 100,     wxLOG_Max = 10000
};


#define wxTRACE_MemAlloc wxT("memalloc") #define wxTRACE_Messages wxT("messages") #define wxTRACE_ResAlloc wxT("resalloc") #define wxTRACE_RefCount wxT("refcount") 
#ifdef  __WINDOWS__
    #define wxTRACE_OleCalls wxT("ole")  #endif

#include "wx/iosfwrap.h"


class wxLogRecordInfo
{
public:
        wxLogRecordInfo()
    {
        memset(this, 0, sizeof(*this));
    }

            wxLogRecordInfo(const char *filename_,
                    int line_,
                    const char *func_,
                    const char *component_)
    {
        filename = filename_;
        func = func_;
        line = line_;
        component = component_;

        timestamp = time(NULL);

#if wxUSE_THREADS
        threadId = wxThread::GetCurrentId();
#endif 
        m_data = NULL;
    }

        wxLogRecordInfo(const wxLogRecordInfo& other)
    {
        Copy(other);
    }

    wxLogRecordInfo& operator=(const wxLogRecordInfo& other)
    {
        if ( &other != this )
        {
            delete m_data;
            Copy(other);
        }

        return *this;
    }

        ~wxLogRecordInfo()
    {
        delete m_data;
    }


            const char *filename;
    int line;

            const char *func;

            const char *component;

        time_t timestamp;

#if wxUSE_THREADS
        wxThreadIdType threadId;
#endif 

                void StoreValue(const wxString& key, wxUIntPtr val)
    {
        if ( !m_data )
            m_data = new ExtraData;

        m_data->numValues[key] = val;
    }

    void StoreValue(const wxString& key, const wxString& val)
    {
        if ( !m_data )
            m_data = new ExtraData;

        m_data->strValues[key] = val;
    }


            bool GetNumValue(const wxString& key, wxUIntPtr *val) const
    {
        if ( !m_data )
            return false;

        wxStringToNumHashMap::const_iterator it = m_data->numValues.find(key);
        if ( it == m_data->numValues.end() )
            return false;

        *val = it->second;

        return true;
    }

    bool GetStrValue(const wxString& key, wxString *val) const
    {
        if ( !m_data )
            return false;

        wxStringToStringHashMap::const_iterator it = m_data->strValues.find(key);
        if ( it == m_data->strValues.end() )
            return false;

        *val = it->second;

        return true;
    }

private:
    void Copy(const wxLogRecordInfo& other)
    {
        memcpy(this, &other, sizeof(*this));
        if ( other.m_data )
           m_data = new ExtraData(*other.m_data);
    }

                struct ExtraData
    {
        wxStringToNumHashMap numValues;
        wxStringToStringHashMap strValues;
    };

        ExtraData *m_data;
};

#define wxLOG_KEY_TRACE_MASK "wx.trace_mask"


struct wxLogRecord
{
    wxLogRecord(wxLogLevel level_,
                const wxString& msg_,
                const wxLogRecordInfo& info_)
        : level(level_),
          msg(msg_),
          info(info_)
    {
    }

    wxLogLevel level;
    wxString msg;
    wxLogRecordInfo info;
};


class WXDLLIMPEXP_BASE wxLogFormatter
{
public:
        wxLogFormatter() { }

        virtual ~wxLogFormatter() { }


                virtual wxString Format(wxLogLevel level,
                            const wxString& msg,
                            const wxLogRecordInfo& info) const;

protected:
            virtual wxString FormatTime(time_t t) const;
};



class WXDLLIMPEXP_BASE wxLog
{
public:
        wxLog() : m_formatter(new wxLogFormatter) { }

        virtual ~wxLog();


        
            
        static bool IsEnabled()
    {
#if wxUSE_THREADS
        if ( !wxThread::IsMain() )
            return IsThreadLoggingEnabled();
#endif 
        return ms_doLog;
    }

        static bool EnableLogging(bool enable = true)
    {
#if wxUSE_THREADS
        if ( !wxThread::IsMain() )
            return EnableThreadLogging(enable);
#endif 
        bool doLogOld = ms_doLog;
        ms_doLog = enable;
        return doLogOld;
    }

        static wxLogLevel GetLogLevel() { return ms_logLevel; }

        static void SetLogLevel(wxLogLevel logLevel) { ms_logLevel = logLevel; }

        static void SetComponentLevel(const wxString& component, wxLogLevel level);

                                static wxLogLevel GetComponentLevel(wxString component);


                static bool IsLevelEnabled(wxLogLevel level, wxString component)
    {
        return IsEnabled() && level <= GetComponentLevel(component);
    }


                        static void SetVerbose(bool bVerbose = true) { ms_bVerbose = bVerbose; }

        static bool GetVerbose() { return ms_bVerbose; }


        
                virtual void Flush();

            static void FlushActive();

            static wxLog *GetActiveTarget();

        static wxLog *SetActiveTarget(wxLog *logger);

#if wxUSE_THREADS
            static wxLog *SetThreadActiveTarget(wxLog *logger);
#endif 
                static void Suspend() { ms_suspendCount++; }

        static void Resume() { ms_suspendCount--; }

            static void DontCreateOnDemand();

        static void DoCreateOnDemand();

            static void SetRepetitionCounting(bool bRepetCounting = true)
        { ms_bRepetCounting = bRepetCounting; }

        static bool GetRepetitionCounting() { return ms_bRepetCounting; }

        static void AddTraceMask(const wxString& str);

        static void RemoveTraceMask(const wxString& str);

        static void ClearTraceMasks();

            static const wxArrayString& GetTraceMasks();

        static bool IsAllowedTraceMask(const wxString& mask);


        
                    wxLogFormatter* SetFormatter(wxLogFormatter* formatter);


                

                static void SetTimestamp(const wxString& ts) { ms_timestamp = ts; }

        static void DisableTimestamp() { SetTimestamp(wxEmptyString); }


        static const wxString& GetTimestamp() { return ms_timestamp; }



        
            static void TimeStamp(wxString *str);
    static void TimeStamp(wxString *str, time_t t);

                void LogRecord(wxLogLevel level,
                   const wxString& msg,
                   const wxLogRecordInfo& info)
    {
        DoLogRecord(level, msg, info);
    }

    void LogTextAtLevel(wxLogLevel level, const wxString& msg)
    {
        DoLogTextAtLevel(level, msg);
    }

    void LogText(const wxString& msg)
    {
        DoLogText(msg);
    }

            static void OnLog(wxLogLevel level,
                      const wxString& msg,
                      const wxLogRecordInfo& info);

                static void OnLog(wxLogLevel level, const wxString& msg, time_t t);

        static void OnLog(wxLogLevel level, const wxString& msg)
    {
        OnLog(level, msg, time(NULL));
    }


        bool HasPendingMessages() const { return true; }

    #if WXWIN_COMPATIBILITY_2_8
    static wxDEPRECATED_INLINE( void SetTraceMask(wxTraceMask ulMask),
        ms_ulTraceMask = ulMask; )

            static wxTraceMask GetTraceMask() { return ms_ulTraceMask; }
#endif 
protected:
                
            virtual void DoLogRecord(wxLogLevel level,
                             const wxString& msg,
                             const wxLogRecordInfo& info);

                virtual void DoLogTextAtLevel(wxLogLevel level, const wxString& msg);

                    virtual void DoLogText(const wxString& msg);


                #if WXWIN_COMPATIBILITY_2_8
    wxDEPRECATED_BUT_USED_INTERNALLY(
        virtual void DoLog(wxLogLevel level, const char *szString, time_t t)
    );

    wxDEPRECATED_BUT_USED_INTERNALLY(
        virtual void DoLog(wxLogLevel level, const wchar_t *wzString, time_t t)
    );

        wxDEPRECATED_BUT_USED_INTERNALLY_INLINE(
        virtual void DoLogString(const char *WXUNUSED(szString),
                                 time_t WXUNUSED(t)),
        wxEMPTY_PARAMETER_VALUE
    )

    wxDEPRECATED_BUT_USED_INTERNALLY_INLINE(
        virtual void DoLogString(const wchar_t *WXUNUSED(wzString),
                                 time_t WXUNUSED(t)),
        wxEMPTY_PARAMETER_VALUE
    )
#endif 

                unsigned LogLastRepeatIfNeeded();

private:
#if wxUSE_THREADS
            void FlushThreadMessages();

            static bool IsThreadLoggingEnabled();
    static bool EnableThreadLogging(bool enable = true);
#endif 
                        static wxLog *GetMainThreadActiveTarget();

                void CallDoLogNow(wxLogLevel level,
                      const wxString& msg,
                      const wxLogRecordInfo& info);


        
    wxLogFormatter    *m_formatter; 

        
            static bool        ms_bRepetCounting;

    static wxLog      *ms_pLogger;          static bool        ms_doLog;            static bool        ms_bAutoCreate;      static bool        ms_bVerbose;     
    static wxLogLevel  ms_logLevel;     
    static size_t      ms_suspendCount; 
            static wxString    ms_timestamp;

#if WXWIN_COMPATIBILITY_2_8
    static wxTraceMask ms_ulTraceMask;   #endif };


class WXDLLIMPEXP_BASE wxLogBuffer : public wxLog
{
public:
    wxLogBuffer() { }

        const wxString& GetBuffer() const { return m_str; }

            virtual void Flush() wxOVERRIDE;

protected:
    virtual void DoLogTextAtLevel(wxLogLevel level, const wxString& msg) wxOVERRIDE;

private:
    wxString m_str;

    wxDECLARE_NO_COPY_CLASS(wxLogBuffer);
};


class WXDLLIMPEXP_BASE wxLogStderr : public wxLog
{
public:
        wxLogStderr(FILE *fp = NULL);

protected:
        virtual void DoLogText(const wxString& msg) wxOVERRIDE;

    FILE *m_fp;

    wxDECLARE_NO_COPY_CLASS(wxLogStderr);
};

#if wxUSE_STD_IOSTREAM

class WXDLLIMPEXP_BASE wxLogStream : public wxLog
{
public:
        wxLogStream(wxSTD ostream *ostr = (wxSTD ostream *) NULL);

protected:
        virtual void DoLogText(const wxString& msg) wxOVERRIDE;

        wxSTD ostream *m_ostr;
};

#endif 


class WXDLLIMPEXP_BASE wxLogNull
{
public:
    wxLogNull() : m_flagOld(wxLog::EnableLogging(false)) { }
    ~wxLogNull() { (void)wxLog::EnableLogging(m_flagOld); }

private:
    bool m_flagOld; };


class WXDLLIMPEXP_BASE wxLogChain : public wxLog
{
public:
    wxLogChain(wxLog *logger);
    virtual ~wxLogChain();

        void SetLog(wxLog *logger);

            void PassMessages(bool bDoPass) { m_bPassMessages = bDoPass; }

        bool IsPassingMessages() const { return m_bPassMessages; }

        wxLog *GetOldLog() const { return m_logOld; }

        virtual void Flush() wxOVERRIDE;

        void DetachOldLog() { m_logOld = NULL; }

protected:
        virtual void DoLogRecord(wxLogLevel level,
                             const wxString& msg,
                             const wxLogRecordInfo& info) wxOVERRIDE;

private:
        wxLog *m_logNew;

        wxLog *m_logOld;

        bool m_bPassMessages;

    wxDECLARE_NO_COPY_CLASS(wxLogChain);
};


#define wxLogPassThrough wxLogInterposer

class WXDLLIMPEXP_BASE wxLogInterposer : public wxLogChain
{
public:
    wxLogInterposer();

private:
    wxDECLARE_NO_COPY_CLASS(wxLogInterposer);
};


class WXDLLIMPEXP_BASE wxLogInterposerTemp : public wxLogChain
{
public:
    wxLogInterposerTemp();

private:
    wxDECLARE_NO_COPY_CLASS(wxLogInterposerTemp);
};

#if wxUSE_GUI
        #include "wx/generic/logg.h"
#endif 


class wxLogger
{
public:
        wxLogger(wxLogLevel level,
             const char *filename,
             int line,
             const char *func,
             const char *component)
        : m_level(level),
          m_info(filename, line, func, component)
    {
    }

            template <typename T>
    wxLogger& Store(const wxString& key, T val)
    {
        m_info.StoreValue(key, val);
        return *this;
    }

                    wxLogger& MaybeStore(const wxString& key, wxUIntPtr value = 0)
    {
        wxASSERT_MSG( m_optKey.empty(), "can only have one optional value" );
        m_optKey = key;

        m_info.StoreValue(key, value);
        return *this;
    }


    
            void LogV(const wxString& format, va_list argptr)
    {
                if ( m_level == wxLOG_FatalError ||
                wxLog::IsLevelEnabled(m_level, m_info.component) )
            DoCallOnLog(format, argptr);
    }

            void LogV(long num, const wxString& format, va_list argptr)
    {
        Store(m_optKey, num);

        LogV(format, argptr);
    }

    void LogV(void *ptr, const wxString& format, va_list argptr)
    {
        Store(m_optKey, wxPtrToUInt(ptr));

        LogV(format, argptr);
    }

    void LogVTrace(const wxString& mask, const wxString& format, va_list argptr)
    {
        if ( !wxLog::IsAllowedTraceMask(mask) )
            return;

        Store(wxLOG_KEY_TRACE_MASK, mask);

        LogV(format, argptr);
    }


    
                    WX_DEFINE_VARARG_FUNC_VOID
    (
        Log,
        1, (const wxFormatString&),
        DoLog, DoLogUtf8
    )

                    WX_DEFINE_VARARG_FUNC_VOID
    (
        Log,
        2, (long, const wxFormatString&),
        DoLogWithNum, DoLogWithNumUtf8
    )

                        WX_DEFINE_VARARG_FUNC_VOID
    (
        Log,
        2, (wxObject *, const wxFormatString&),
        DoLogWithPtr, DoLogWithPtrUtf8
    )

                    WX_DEFINE_VARARG_FUNC_VOID
    (
        LogAtLevel,
        2, (wxLogLevel, const wxFormatString&),
        DoLogAtLevel, DoLogAtLevelUtf8
    )

                WX_DEFINE_VARARG_FUNC_VOID
    (
        LogTrace,
        2, (const wxString&, const wxFormatString&),
        DoLogTrace, DoLogTraceUtf8
    )

#if WXWIN_COMPATIBILITY_2_8
    WX_DEFINE_VARARG_FUNC_VOID
    (
        LogTrace,
        2, (wxTraceMask, const wxFormatString&),
        DoLogTraceMask, DoLogTraceMaskUtf8
    )
#endif 
private:
#if !wxUSE_UTF8_LOCALE_ONLY
    void DoLog(const wxChar *format, ...)
    {
        va_list argptr;
        va_start(argptr, format);
        DoCallOnLog(format, argptr);
        va_end(argptr);
    }

    void DoLogWithNum(long num, const wxChar *format, ...)
    {
        Store(m_optKey, num);

        va_list argptr;
        va_start(argptr, format);
        DoCallOnLog(format, argptr);
        va_end(argptr);
    }

    void DoLogWithPtr(void *ptr, const wxChar *format, ...)
    {
        Store(m_optKey, wxPtrToUInt(ptr));

        va_list argptr;
        va_start(argptr, format);
        DoCallOnLog(format, argptr);
        va_end(argptr);
    }

    void DoLogAtLevel(wxLogLevel level, const wxChar *format, ...)
    {
        if ( !wxLog::IsLevelEnabled(level, m_info.component) )
            return;

        va_list argptr;
        va_start(argptr, format);
        DoCallOnLog(level, format, argptr);
        va_end(argptr);
    }

    void DoLogTrace(const wxString& mask, const wxChar *format, ...)
    {
        if ( !wxLog::IsAllowedTraceMask(mask) )
            return;

        Store(wxLOG_KEY_TRACE_MASK, mask);

        va_list argptr;
        va_start(argptr, format);
        DoCallOnLog(format, argptr);
        va_end(argptr);
    }

#if WXWIN_COMPATIBILITY_2_8
    void DoLogTraceMask(wxTraceMask mask, const wxChar *format, ...)
    {
        if ( (wxLog::GetTraceMask() & mask) != mask )
            return;

        Store(wxLOG_KEY_TRACE_MASK, mask);

        va_list argptr;
        va_start(argptr, format);
        DoCallOnLog(format, argptr);
        va_end(argptr);
    }
#endif #endif 
#if wxUSE_UNICODE_UTF8
    void DoLogUtf8(const char *format, ...)
    {
        va_list argptr;
        va_start(argptr, format);
        DoCallOnLog(format, argptr);
        va_end(argptr);
    }

    void DoLogWithNumUtf8(long num, const char *format, ...)
    {
        Store(m_optKey, num);

        va_list argptr;
        va_start(argptr, format);
        DoCallOnLog(format, argptr);
        va_end(argptr);
    }

    void DoLogWithPtrUtf8(void *ptr, const char *format, ...)
    {
        Store(m_optKey, wxPtrToUInt(ptr));

        va_list argptr;
        va_start(argptr, format);
        DoCallOnLog(format, argptr);
        va_end(argptr);
    }

    void DoLogAtLevelUtf8(wxLogLevel level, const char *format, ...)
    {
        if ( !wxLog::IsLevelEnabled(level, m_info.component) )
            return;

        va_list argptr;
        va_start(argptr, format);
        DoCallOnLog(level, format, argptr);
        va_end(argptr);
    }

    void DoLogTraceUtf8(const wxString& mask, const char *format, ...)
    {
        if ( !wxLog::IsAllowedTraceMask(mask) )
            return;

        Store(wxLOG_KEY_TRACE_MASK, mask);

        va_list argptr;
        va_start(argptr, format);
        DoCallOnLog(format, argptr);
        va_end(argptr);
    }

#if WXWIN_COMPATIBILITY_2_8
    void DoLogTraceMaskUtf8(wxTraceMask mask, const char *format, ...)
    {
        if ( (wxLog::GetTraceMask() & mask) != mask )
            return;

        Store(wxLOG_KEY_TRACE_MASK, mask);

        va_list argptr;
        va_start(argptr, format);
        DoCallOnLog(format, argptr);
        va_end(argptr);
    }
#endif #endif 
    void DoCallOnLog(wxLogLevel level, const wxString& format, va_list argptr)
    {
        wxLog::OnLog(level, wxString::FormatV(format, argptr), m_info);
    }

    void DoCallOnLog(const wxString& format, va_list argptr)
    {
        DoCallOnLog(m_level, format, argptr);
    }


    const wxLogLevel m_level;
    wxLogRecordInfo m_info;

    wxString m_optKey;

    wxDECLARE_NO_COPY_CLASS(wxLogger);
};



WXDLLIMPEXP_BASE unsigned long wxSysErrorCode();

WXDLLIMPEXP_BASE const wxChar* wxSysErrorMsg(unsigned long nErrCode = 0);




#define wxMAKE_LOGGER(level) \
    wxLogger(wxLOG_##level, __FILE__, __LINE__, __WXFUNCTION__, wxLOG_COMPONENT)

#define wxDO_LOG(level) wxMAKE_LOGGER(level).Log

#define wxDO_LOGV(level, format, argptr) \
    wxMAKE_LOGGER(level).LogV(format, argptr)

#define wxDO_LOG_IF_ENABLED_HELPER(level, loopvar)                            \
    for ( bool loopvar = false;                                               \
          !loopvar && wxLog::IsLevelEnabled(wxLOG_##level, wxLOG_COMPONENT);  \
          loopvar = true )                                                    \
        wxDO_LOG(level)

#define wxDO_LOG_IF_ENABLED(level)                                            \
    wxDO_LOG_IF_ENABLED_HELPER(level, wxMAKE_UNIQUE_NAME(wxlogcheck))

#define wxLogFatalError wxDO_LOG(FatalError)
#define wxVLogFatalError(format, argptr) wxDO_LOGV(FatalError, format, argptr)

#define wxLogError wxDO_LOG_IF_ENABLED(Error)
#define wxVLogError(format, argptr) wxDO_LOGV(Error, format, argptr)

#define wxLogWarning wxDO_LOG_IF_ENABLED(Warning)
#define wxVLogWarning(format, argptr) wxDO_LOGV(Warning, format, argptr)

#define wxLogMessage wxDO_LOG_IF_ENABLED(Message)
#define wxVLogMessage(format, argptr) wxDO_LOGV(Message, format, argptr)

#define wxLogInfo wxDO_LOG_IF_ENABLED(Info)
#define wxVLogInfo(format, argptr) wxDO_LOGV(Info, format, argptr)


#define wxLogVerbose                                                          \
    if ( !(wxLog::IsLevelEnabled(wxLOG_Info, wxLOG_COMPONENT) &&              \
            wxLog::GetVerbose()) )                                            \
    {}                                                                        \
    else                                                                      \
        wxDO_LOG(Info)
#define wxVLogVerbose(format, argptr)                                         \
    if ( !(wxLog::IsLevelEnabled(wxLOG_Info, wxLOG_COMPONENT) &&              \
            wxLog::GetVerbose()) )                                            \
    {}                                                                        \
    else                                                                      \
        wxDO_LOGV(Info, format, argptr)

#define wxLogGeneric wxMAKE_LOGGER(Max).LogAtLevel
#define wxVLogGeneric(level, format, argptr) \
    if ( !wxLog::IsLevelEnabled(wxLOG_##level, wxLOG_COMPONENT) )             \
    {}                                                                        \
    else                                                                      \
        wxDO_LOGV(level, format, argptr)


#define wxLOG_KEY_SYS_ERROR_CODE "wx.sys_error"

#define wxLogSysError                                                         \
    if ( !wxLog::IsLevelEnabled(wxLOG_Error, wxLOG_COMPONENT) )               \
    {}                                                                        \
    else                                                                      \
        wxMAKE_LOGGER(Error).MaybeStore(wxLOG_KEY_SYS_ERROR_CODE,             \
                                        wxSysErrorCode()).Log

#define wxVLogSysError \
    wxMAKE_LOGGER(Error).MaybeStore(wxLOG_KEY_SYS_ERROR_CODE, \
                                    wxSysErrorCode()).LogV

#if wxUSE_GUI
            #define wxLOG_KEY_FRAME "wx.frame"

    #define wxLogStatus                                                       \
        if ( !wxLog::IsLevelEnabled(wxLOG_Status, wxLOG_COMPONENT) )          \
        {}                                                                    \
        else                                                                  \
            wxMAKE_LOGGER(Status).MaybeStore(wxLOG_KEY_FRAME).Log

    #define wxVLogStatus \
        wxMAKE_LOGGER(Status).MaybeStore(wxLOG_KEY_FRAME).LogV
#endif 

#else 
#undef wxUSE_LOG_DEBUG
#define wxUSE_LOG_DEBUG 0

#undef wxUSE_LOG_TRACE
#define wxUSE_LOG_TRACE 0

#define wxDEFINE_EMPTY_LOG_FUNCTION(level)                                  \
    WX_DEFINE_VARARG_FUNC_NOP(wxLog##level, 1, (const wxFormatString&))     \
    inline void wxVLog##level(const wxFormatString& WXUNUSED(format),       \
                              va_list WXUNUSED(argptr)) { }                 \

#define wxDEFINE_EMPTY_LOG_FUNCTION2(level, argclass)                       \
    WX_DEFINE_VARARG_FUNC_NOP(wxLog##level, 2, (argclass, const wxFormatString&)) \
    inline void wxVLog##level(argclass WXUNUSED(arg),                       \
                              const wxFormatString& WXUNUSED(format),       \
                              va_list WXUNUSED(argptr)) {}

wxDEFINE_EMPTY_LOG_FUNCTION(FatalError);
wxDEFINE_EMPTY_LOG_FUNCTION(Error);
wxDEFINE_EMPTY_LOG_FUNCTION(SysError);
wxDEFINE_EMPTY_LOG_FUNCTION2(SysError, long);
wxDEFINE_EMPTY_LOG_FUNCTION(Warning);
wxDEFINE_EMPTY_LOG_FUNCTION(Message);
wxDEFINE_EMPTY_LOG_FUNCTION(Info);
wxDEFINE_EMPTY_LOG_FUNCTION(Verbose);

wxDEFINE_EMPTY_LOG_FUNCTION2(Generic, wxLogLevel);

#if wxUSE_GUI
    wxDEFINE_EMPTY_LOG_FUNCTION(Status);
    wxDEFINE_EMPTY_LOG_FUNCTION2(Status, wxFrame *);
#endif 
class WXDLLIMPEXP_BASE wxLogNull
{
public:
    wxLogNull() { }
};

#define wxSysErrorCode() (unsigned long)0
#define wxSysErrorMsg( X ) (const wxChar*)NULL

#define wxTRACE_OleCalls wxEmptyString 
#endif 


#ifdef __BORLANDC__
                #define wxLogNop() { }
#else
    inline void wxLogNop() { }
#endif

#if wxUSE_LOG_DEBUG
    #define wxLogDebug wxDO_LOG_IF_ENABLED(Debug)
    #define wxVLogDebug(format, argptr) wxDO_LOGV(Debug, format, argptr)
#else     #define wxVLogDebug(fmt, valist) wxLogNop()

    #ifdef HAVE_VARIADIC_MACROS
        #define wxLogDebug(fmt, ...) wxLogNop()
    #else         WX_DEFINE_VARARG_FUNC_NOP(wxLogDebug, 1, (const wxFormatString&))
    #endif
#endif 
#if wxUSE_LOG_TRACE
    #define wxLogTrace                                                        \
        if ( !wxLog::IsLevelEnabled(wxLOG_Trace, wxLOG_COMPONENT) )           \
        {}                                                                    \
        else                                                                  \
            wxMAKE_LOGGER(Trace).LogTrace
    #define wxVLogTrace                                                       \
        if ( !wxLog::IsLevelEnabled(wxLOG_Trace, wxLOG_COMPONENT) )           \
        {}                                                                    \
        else                                                                  \
            wxMAKE_LOGGER(Trace).LogVTrace
#else      #define wxVLogTrace(mask, fmt, valist) wxLogNop()

    #ifdef HAVE_VARIADIC_MACROS
        #define wxLogTrace(mask, fmt, ...) wxLogNop()
    #else         #if WXWIN_COMPATIBILITY_2_8
        WX_DEFINE_VARARG_FUNC_NOP(wxLogTrace, 2, (wxTraceMask, const wxFormatString&))
        #endif
        WX_DEFINE_VARARG_FUNC_NOP(wxLogTrace, 2, (const wxString&, const wxFormatString&))
    #endif #endif 
void WXDLLIMPEXP_BASE
wxSafeShowMessage(const wxString& title, const wxString& text);


#if wxUSE_LOG_DEBUG
        #ifdef __VISUALC__
    #define wxLogApiError(api, rc)                                            \
        wxLogDebug(wxT("%s(%d): '%s' failed with error 0x%08lx (%s)."),       \
                   __FILE__, __LINE__, api,                                   \
                   (long)rc, wxSysErrorMsg(rc))
#else     #define wxLogApiError(api, rc)                                            \
        wxLogDebug(wxT("In file %s at line %d: '%s' failed with ")            \
                   wxT("error 0x%08lx (%s)."),                                \
                   __FILE__, __LINE__, api,                                   \
                   (long)rc, wxSysErrorMsg(rc))
#endif 
    #define wxLogLastError(api) wxLogApiError(api, wxSysErrorCode())

#else     #define wxLogApiError(api, err) wxLogNop()
    #define wxLogLastError(api) wxLogNop()
#endif 
#if defined(NDEBUG) && wxUSE_LOG_DEBUG
    #define wxDISABLE_DEBUG_LOGGING_IN_RELEASE_BUILD() \
        wxLog::SetLogLevel(wxLOG_Info)
#else     #define wxDISABLE_DEBUG_LOGGING_IN_RELEASE_BUILD()
#endif 
#endif  
