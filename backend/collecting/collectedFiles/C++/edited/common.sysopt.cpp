


#include "wx/wxprec.h"

#if defined(__BORLANDC__)
    #pragma hdrstop
#endif

#if wxUSE_SYSTEM_OPTIONS

#include "wx/sysopt.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/list.h"
    #include "wx/string.h"
    #include "wx/arrstr.h"
#endif


static wxArrayString gs_optionNames,
                     gs_optionValues;


void wxSystemOptions::SetOption(const wxString& name, const wxString& value)
{
    int idx = gs_optionNames.Index(name, false);
    if (idx == wxNOT_FOUND)
    {
        gs_optionNames.Add(name);
        gs_optionValues.Add(value);
    }
    else
    {
        gs_optionNames[idx] = name;
        gs_optionValues[idx] = value;
    }
}

void wxSystemOptions::SetOption(const wxString& name, int value)
{
    SetOption(name, wxString::Format(wxT("%d"), value));
}

wxString wxSystemOptions::GetOption(const wxString& name)
{
    wxString val;

    int idx = gs_optionNames.Index(name, false);
    if ( idx != wxNOT_FOUND )
    {
        val = gs_optionValues[idx];
    }
    else     {
                                wxString var(name);
        var.Replace(wxT("."), wxT("_"));          var.Replace(wxT("-"), wxT("_"));  
        wxString appname;
        if ( wxTheApp )
            appname = wxTheApp->GetAppName();

        if ( !appname.empty() )
            val = wxGetenv(wxT("wx_") + appname + wxT('_') + var);

        if ( val.empty() )
            val = wxGetenv(wxT("wx_") + var);
    }

    return val;
}

int wxSystemOptions::GetOptionInt(const wxString& name)
{
    return wxAtoi (GetOption(name));
}

bool wxSystemOptions::HasOption(const wxString& name)
{
    return !GetOption(name).empty();
}

#endif 