


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_STARTUP_TIPS

#ifndef WX_PRECOMP
    #include "wx/button.h"
    #include "wx/checkbox.h"
    #include "wx/statbox.h"
    #include "wx/dialog.h"
    #include "wx/icon.h"
    #include "wx/intl.h"
    #include "wx/textctrl.h"
    #include "wx/statbmp.h"
    #include "wx/stattext.h"
    #include "wx/sizer.h"
    #include "wx/settings.h"
#endif 
#include "wx/statline.h"
#include "wx/artprov.h"

#include "wx/tipdlg.h"


static const int wxID_NEXT_TIP = 32000;  

#if wxUSE_TEXTFILE
class WXDLLIMPEXP_ADV wxFileTipProvider : public wxTipProvider
{
public:
    wxFileTipProvider(const wxString& filename, size_t currentTip);

    virtual wxString GetTip() wxOVERRIDE;

private:
    wxTextFile m_textfile;

    wxDECLARE_NO_COPY_CLASS(wxFileTipProvider);
};
#endif 
#ifdef __WIN32__
class WXDLLIMPEXP_ADV wxRegTipProvider : public wxTipProvider
{
public:
    wxRegTipProvider(const wxString& keyname);

    virtual wxString GetTip();
};

wxString wxRegTipProvider::GetTip()
{
    return wxEmptyString;
}

#endif 
class WXDLLIMPEXP_ADV wxTipDialog : public wxDialog
{
public:
    wxTipDialog(wxWindow *parent,
                wxTipProvider *tipProvider,
                bool showAtStartup);

            bool ShowTipsOnStartup() const { return m_checkbox->GetValue(); }

        void SetTipText() { m_text->SetValue(m_tipProvider->GetTip()); }

        void OnNextTip(wxCommandEvent& WXUNUSED(event)) { SetTipText(); }

private:
    wxTipProvider *m_tipProvider;

    wxTextCtrl *m_text;
    wxCheckBox *m_checkbox;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(wxTipDialog);
};


#if wxUSE_TEXTFILE
wxFileTipProvider::wxFileTipProvider(const wxString& filename,
                                     size_t currentTip)
                 : wxTipProvider(currentTip), m_textfile(filename)
{
    m_textfile.Open();
}

wxString wxFileTipProvider::GetTip()
{
    size_t count = m_textfile.GetLineCount();
    if ( !count )
    {
        return _("Tips not available, sorry!");
    }

    wxString tip;

                        for ( size_t i=0; i < count; i++ )
    {
                                        if ( m_currentTip >= count )
        {
                        m_currentTip = 0;
        }

                tip = m_textfile.GetLine(m_currentTip++);

                        if ( !tip.StartsWith(wxT("#")) && (tip.Trim() != wxEmptyString) )
        {
            break;
        }
    }

            if ( tip.StartsWith(wxT("_(\"" ), &tip))
    {
                tip = tip.BeforeLast(wxT('\"'));
                tip.Replace(wxT("\\\""), wxT("\""));

                tip = wxGetTranslation(tip);
    }

    return tip;
}
#endif 

wxBEGIN_EVENT_TABLE(wxTipDialog, wxDialog)
    EVT_BUTTON(wxID_NEXT_TIP, wxTipDialog::OnNextTip)
wxEND_EVENT_TABLE()

wxTipDialog::wxTipDialog(wxWindow *parent,
                         wxTipProvider *tipProvider,
                         bool showAtStartup)
           : wxDialog(GetParentForModalDialog(parent, 0), wxID_ANY, _("Tip of the Day"),
                      wxDefaultPosition, wxDefaultSize,
                      wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
                      )
{
    m_tipProvider = tipProvider;
    bool isPda = (wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA);

    
    wxStaticText *text = new wxStaticText(this, wxID_ANY, _("Did you know..."));

    if (!isPda)
    {
        wxFont font = text->GetFont();
        font.SetPointSize(int(1.6 * font.GetPointSize()));
        font.SetWeight(wxFONTWEIGHT_BOLD);
        text->SetFont(font);
    }

    m_text = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
                            wxDefaultPosition, wxSize(200, 160),
                            wxTE_MULTILINE |
                            wxTE_READONLY |
                            wxTE_NO_VSCROLL |
                            wxTE_RICH2 |                             wxDEFAULT_CONTROL_BORDER
                            );
#if defined(__WXMSW__)
    m_text->SetFont(wxFont(12, wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
#endif

    wxIcon icon = wxArtProvider::GetIcon(wxART_TIP, wxART_CMN_DIALOG);
    wxStaticBitmap *bmp = new wxStaticBitmap(this, wxID_ANY, icon);

    m_checkbox = new wxCheckBox(this, wxID_ANY, _("&Show tips at startup"));
    m_checkbox->SetValue(showAtStartup);
    m_checkbox->SetFocus();

    wxButton *btnNext = new wxButton(this, wxID_NEXT_TIP, _("&Next Tip"));

    wxButton *btnClose = new wxButton(this, wxID_CLOSE);
    SetAffirmativeId(wxID_CLOSE);


    
    wxBoxSizer *topsizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *icon_text = new wxBoxSizer( wxHORIZONTAL );
    icon_text->Add( bmp, 0, wxCENTER );
    icon_text->Add( text, 1, wxCENTER | wxLEFT, 20 );
    topsizer->Add( icon_text, 0, wxEXPAND | wxALL, 10 );

    topsizer->Add( m_text, 1, wxEXPAND | wxLEFT|wxRIGHT, 10 );

    wxBoxSizer *bottom = new wxBoxSizer( wxHORIZONTAL );
    if (isPda)
        topsizer->Add( m_checkbox, 0, wxCENTER|wxTOP );
    else
        bottom->Add( m_checkbox, 0, wxCENTER );

    if (!isPda)
        bottom->Add( 10,10,1 );
    bottom->Add( btnNext, 0, wxCENTER | wxLEFT, 10 );
    bottom->Add( btnClose, 0, wxCENTER | wxLEFT, 10 );

    if (isPda)
        topsizer->Add( bottom, 0, wxCENTER | wxALL, 5 );
    else
        topsizer->Add( bottom, 0, wxEXPAND | wxALL, 10 );

    SetTipText();

    SetSizer( topsizer );

    topsizer->SetSizeHints( this );
    topsizer->Fit( this );

    Centre(wxBOTH | wxCENTER_FRAME);
}


#if wxUSE_TEXTFILE
wxTipProvider *wxCreateFileTipProvider(const wxString& filename,
                                       size_t currentTip)
{
    return new wxFileTipProvider(filename, currentTip);
}
#endif 
bool wxShowTip(wxWindow *parent,
               wxTipProvider *tipProvider,
               bool showAtStartup)
{
    wxTipDialog dlg(parent, tipProvider, showAtStartup);
    dlg.ShowModal();

    return dlg.ShowTipsOnStartup();
}

#endif 