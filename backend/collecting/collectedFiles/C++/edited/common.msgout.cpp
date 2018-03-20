


#include "wx/wxprec.h"

#if defined(__BORLANDC__)
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/ffile.h"
    #include "wx/app.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #if wxUSE_GUI
        #include "wx/msgdlg.h"
    #endif #endif

#include "wx/msgout.h"
#include "wx/apptrait.h"
#include <stdarg.h>
#include <stdio.h>

#if defined(__WINDOWS__)
    #include "wx/msw/private.h"
#endif


#if wxUSE_BASE


wxMessageOutput* wxMessageOutput::ms_msgOut = 0;

wxMessageOutput* wxMessageOutput::Get()
{
    if ( !ms_msgOut && wxTheApp )
    {
        ms_msgOut = wxTheApp->GetTraits()->CreateMessageOutput();
    }

    return ms_msgOut;
}

wxMessageOutput* wxMessageOutput::Set(wxMessageOutput* msgout)
{
    wxMessageOutput* old = ms_msgOut;
    ms_msgOut = msgout;
    return old;
}

#if !wxUSE_UTF8_LOCALE_ONLY
void wxMessageOutput::DoPrintfWchar(const wxChar *format, ...)
{
    va_list args;
    va_start(args, format);
    wxString out;

    out.PrintfV(format, args);
    va_end(args);

    Output(out);
}
#endif 
#if wxUSE_UNICODE_UTF8
void wxMessageOutput::DoPrintfUtf8(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    wxString out;

    out.PrintfV(format, args);
    va_end(args);

    Output(out);
}
#endif 

void wxMessageOutputBest::Output(const wxString& str)
{
#ifdef __WINDOWS__
        wxAppTraits * const traits = wxTheApp ? wxTheApp->GetTraits() : NULL;
    const bool hasStderr = traits ? traits->CanUseStderr() : false;

    if ( !(m_flags & wxMSGOUT_PREFER_MSGBOX) )
    {
        if ( hasStderr && traits->WriteToStderr(AppendLineFeedIfNeeded(str)) )
            return;
    }

    wxString title;
    if ( wxTheApp )
        title = wxTheApp->GetAppDisplayName();
    else         title = _("Message");

    ::MessageBox(NULL, str.t_str(), title.t_str(), MB_ICONINFORMATION | MB_OK);
#else     wxUnusedVar(m_flags);

        wxMessageOutputStderr::Output(str);
#endif }


wxString wxMessageOutputStderr::AppendLineFeedIfNeeded(const wxString& str)
{
    wxString strLF(str);
    if ( strLF.empty() || *strLF.rbegin() != '\n' )
        strLF += '\n';

    return strLF;
}

void wxMessageOutputStderr::Output(const wxString& str)
{
    const wxString strWithLF = AppendLineFeedIfNeeded(str);

    fprintf(m_fp, "%s", (const char*) strWithLF.mb_str(wxConvWhateverWorks));
    fflush(m_fp);
}


void wxMessageOutputDebug::Output(const wxString& str)
{
#if defined(__WINDOWS__)
    wxString out(AppendLineFeedIfNeeded(str));
    out.Replace(wxT("\t"), wxT("        "));
    out.Replace(wxT("\n"), wxT("\r\n"));
    ::OutputDebugString(out.t_str());
#else
        wxMessageOutputStderr::Output(str);
#endif }


void wxMessageOutputLog::Output(const wxString& str)
{
    wxString out(str);

    out.Replace(wxT("\t"), wxT("        "));

    wxLogMessage(wxT("%s"), out.c_str());
}

#endif 

#if wxUSE_GUI && wxUSE_MSGDLG

extern WXDLLEXPORT_DATA(const char) wxMessageBoxCaptionStr[] = "Message";

void wxMessageOutputMessageBox::Output(const wxString& str)
{
    wxString out(str);

    #ifndef __WINDOWS__
    out.Replace(wxT("\t"), wxT("        "));
#endif

    wxString title = wxT("wxWidgets") ;
    if (wxTheApp) title = wxTheApp->GetAppDisplayName();

    ::wxMessageBox(out, title);
}

#endif 