


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/button.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/menu.h"
    #include "wx/frame.h"
    #include "wx/filedlg.h"
    #include "wx/msgdlg.h"
    #include "wx/textctrl.h"
    #include "wx/sizer.h"
    #include "wx/statbmp.h"
    #include "wx/settings.h"
    #include "wx/wxcrtvararg.h"
#endif 
#if wxUSE_LOGGUI || wxUSE_LOGWINDOW

#include "wx/file.h"
#include "wx/clipbrd.h"
#include "wx/dataobj.h"
#include "wx/textfile.h"
#include "wx/statline.h"
#include "wx/artprov.h"
#include "wx/collpane.h"
#include "wx/arrstr.h"
#include "wx/msgout.h"
#include "wx/scopeguard.h"

#ifdef  __WXMSW__
        #include  "wx/msw/private.h"
#endif 

#if wxUSE_LOG_DIALOG
    #include "wx/listctrl.h"
    #include "wx/imaglist.h"
    #include "wx/image.h"
#endif 
#include "wx/time.h"

#define CAN_SAVE_FILES (wxUSE_FILE && wxUSE_FILEDLG)


#if wxUSE_LOG_DIALOG

static wxString TimeStamp(const wxString& format, time_t t)
{
    wxChar buf[4096];
    struct tm tm;
    if ( !wxStrftime(buf, WXSIZEOF(buf), format, wxLocaltime_r(&t, &tm)) )
    {
                wxFAIL_MSG(wxT("strftime() failed"));
    }
    return wxString(buf);
}


class wxLogDialog : public wxDialog
{
public:
    wxLogDialog(wxWindow *parent,
                const wxArrayString& messages,
                const wxArrayInt& severity,
                const wxArrayLong& timess,
                const wxString& caption,
                long style);
    virtual ~wxLogDialog();

        void OnOk(wxCommandEvent& event);
#if wxUSE_CLIPBOARD
    void OnCopy(wxCommandEvent& event);
#endif #if CAN_SAVE_FILES
    void OnSave(wxCommandEvent& event);
#endif     void OnListItemActivated(wxListEvent& event);

private:
        void CreateDetailsControls(wxWindow *);

        wxString EllipsizeString(const wxString &text)
    {
        if (ms_maxLength > 0 &&
            text.length() > ms_maxLength)
        {
            wxString ret(text);
            ret.Truncate(ms_maxLength);
            ret << "...";
            return ret;
        }

        return text;
    }

#if CAN_SAVE_FILES || wxUSE_CLIPBOARD
        wxString GetLogMessages() const;
#endif 

        wxArrayString m_messages;
    wxArrayInt m_severity;
    wxArrayLong m_times;

            wxListCtrl *m_listctrl;

        static wxString ms_details;

        static size_t ms_maxLength;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(wxLogDialog);
};

wxBEGIN_EVENT_TABLE(wxLogDialog, wxDialog)
    EVT_BUTTON(wxID_OK, wxLogDialog::OnOk)
#if wxUSE_CLIPBOARD
    EVT_BUTTON(wxID_COPY,   wxLogDialog::OnCopy)
#endif #if CAN_SAVE_FILES
    EVT_BUTTON(wxID_SAVE,   wxLogDialog::OnSave)
#endif     EVT_LIST_ITEM_ACTIVATED(wxID_ANY, wxLogDialog::OnListItemActivated)
wxEND_EVENT_TABLE()

#endif 

#if CAN_SAVE_FILES

static int OpenLogFile(wxFile& file, wxString *filename = NULL, wxWindow *parent = NULL);

#endif 


#if wxUSE_LOGGUI

wxLogGui::wxLogGui()
{
    Clear();
}

void wxLogGui::Clear()
{
    m_bErrors =
    m_bWarnings =
    m_bHasMessages = false;

    m_aMessages.Empty();
    m_aSeverity.Empty();
    m_aTimes.Empty();
}

int wxLogGui::GetSeverityIcon() const
{
    return m_bErrors ? wxICON_STOP
                     : m_bWarnings ? wxICON_EXCLAMATION
                                   : wxICON_INFORMATION;
}

wxString wxLogGui::GetTitle() const
{
    wxString titleFormat;
    switch ( GetSeverityIcon() )
    {
        case wxICON_STOP:
            titleFormat = _("%s Error");
            break;

        case wxICON_EXCLAMATION:
            titleFormat = _("%s Warning");
            break;

        default:
            wxFAIL_MSG( "unexpected icon severity" );
            wxFALLTHROUGH;

        case wxICON_INFORMATION:
            titleFormat = _("%s Information");
    }

    return wxString::Format(titleFormat, wxTheApp->GetAppDisplayName());
}

void
wxLogGui::DoShowSingleLogMessage(const wxString& message,
                                 const wxString& title,
                                 int style)
{
    wxMessageBox(message, title, wxOK | style);
}

void
wxLogGui::DoShowMultipleLogMessages(const wxArrayString& messages,
                                    const wxArrayInt& severities,
                                    const wxArrayLong& times,
                                    const wxString& title,
                                    int style)
{
#if wxUSE_LOG_DIALOG
    wxLogDialog dlg(NULL,
                    messages, severities, times,
                    title, style);

            Clear();

    (void)dlg.ShowModal();
#else         wxString message;
    const size_t nMsgCount = messages.size();
    message.reserve(nMsgCount*100);
    for ( size_t n = nMsgCount; n > 0; n-- ) {
        message << m_aMessages[n - 1] << wxT("\n");
    }

    DoShowSingleLogMessage(message, title, style);
#endif }

void wxLogGui::Flush()
{
    wxLog::Flush();

    if ( !m_bHasMessages )
        return;

        m_bHasMessages = false;

            const unsigned repeatCount = LogLastRepeatIfNeeded();

    const size_t nMsgCount = m_aMessages.size();

    if ( repeatCount > 0 )
    {
        m_aMessages[nMsgCount - 1] << " (" << m_aMessages[nMsgCount - 2] << ")";
    }

    const wxString title = GetTitle();
    const int style = GetSeverityIcon();

            Suspend();

            wxON_BLOCK_EXIT0(wxLog::Resume);

    if ( nMsgCount == 1 )
    {
                const wxString message(m_aMessages[0]);
        Clear();

        DoShowSingleLogMessage(message, title, style);
    }
    else     {
        wxArrayString messages;
        wxArrayInt severities;
        wxArrayLong times;

        messages.swap(m_aMessages);
        severities.swap(m_aSeverity);
        times.swap(m_aTimes);

        Clear();

        DoShowMultipleLogMessages(messages, severities, times, title, style);
    }
}

void wxLogGui::DoLogRecord(wxLogLevel level,
                           const wxString& msg,
                           const wxLogRecordInfo& info)
{
    switch ( level )
    {
        case wxLOG_Info:
        case wxLOG_Message:
            {
                m_aMessages.Add(msg);
                m_aSeverity.Add(wxLOG_Message);
                m_aTimes.Add((long)info.timestamp);
                m_bHasMessages = true;
            }
            break;

        case wxLOG_Status:
#if wxUSE_STATUSBAR
            {
                wxFrame *pFrame = NULL;

                                wxUIntPtr ptr = 0;
                if ( info.GetNumValue(wxLOG_KEY_FRAME, &ptr) )
                {
                    pFrame = static_cast<wxFrame *>(wxUIntToPtr(ptr));
                }

                                if ( pFrame == NULL ) {
                    wxWindow *pWin = wxTheApp->GetTopWindow();
                    if ( wxDynamicCast(pWin, wxFrame) ) {
                        pFrame = (wxFrame *)pWin;
                    }
                }

                if ( pFrame && pFrame->GetStatusBar() )
                    pFrame->SetStatusText(msg);
            }
#endif             break;

        case wxLOG_Error:
            if ( !m_bErrors ) {
#if !wxUSE_LOG_DIALOG
                                                                m_aMessages.Empty();
                m_aSeverity.Empty();
                m_aTimes.Empty();
#endif                 m_bErrors = true;
            }
            wxFALLTHROUGH;

        case wxLOG_Warning:
            if ( !m_bErrors ) {
                                m_bWarnings = true;
            }

            m_aMessages.Add(msg);
            m_aSeverity.Add((int)level);
            m_aTimes.Add((long)info.timestamp);
            m_bHasMessages = true;
            break;

        case wxLOG_Debug:
        case wxLOG_Trace:
                        wxLog::DoLogRecord(level, msg, info);
            break;

        case wxLOG_FatalError:
        case wxLOG_Max:
                                    wxFAIL_MSG("unexpected log level");
            break;

        case wxLOG_Progress:
        case wxLOG_User:
                                                break;
    }
}

#endif   

#if wxUSE_LOGWINDOW

class wxLogFrame : public wxFrame
{
public:
        wxLogFrame(wxWindow *pParent, wxLogWindow *log, const wxString& szTitle);
    virtual ~wxLogFrame();

        virtual bool ShouldPreventAppExit() const wxOVERRIDE { return false; }

        void OnClose(wxCommandEvent& event);
    void OnCloseWindow(wxCloseEvent& event);
#if CAN_SAVE_FILES
    void OnSave(wxCommandEvent& event);
#endif     void OnClear(wxCommandEvent& event);

        void ShowLogMessage(const wxString& message)
    {
        m_pTextCtrl->AppendText(message + wxS('\n'));
    }

private:
        enum
    {
        Menu_Close = wxID_CLOSE,
        Menu_Save  = wxID_SAVE,
        Menu_Clear = wxID_CLEAR
    };

        void DoClose();

    wxTextCtrl  *m_pTextCtrl;
    wxLogWindow *m_log;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(wxLogFrame);
};

wxBEGIN_EVENT_TABLE(wxLogFrame, wxFrame)
        EVT_MENU(Menu_Close, wxLogFrame::OnClose)
#if CAN_SAVE_FILES
    EVT_MENU(Menu_Save,  wxLogFrame::OnSave)
#endif     EVT_MENU(Menu_Clear, wxLogFrame::OnClear)

    EVT_CLOSE(wxLogFrame::OnCloseWindow)
wxEND_EVENT_TABLE()

wxLogFrame::wxLogFrame(wxWindow *pParent, wxLogWindow *log, const wxString& szTitle)
          : wxFrame(pParent, wxID_ANY, szTitle)
{
    m_log = log;

    m_pTextCtrl = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
            wxDefaultSize,
            wxTE_MULTILINE  |
            wxHSCROLL       |
                        #if !wxUSE_UNICODE
            wxTE_RICH       |
#endif             wxTE_READONLY);

#if wxUSE_MENUS
        wxMenuBar *pMenuBar = new wxMenuBar;
    wxMenu *pMenu = new wxMenu;
#if CAN_SAVE_FILES
    pMenu->Append(Menu_Save,  _("Save &As..."), _("Save log contents to file"));
#endif     pMenu->Append(Menu_Clear, _("C&lear"), _("Clear the log contents"));
    pMenu->AppendSeparator();
    pMenu->Append(Menu_Close, _("&Close"), _("Close this window"));
    pMenuBar->Append(pMenu, _("&Log"));
    SetMenuBar(pMenuBar);
#endif 
#if wxUSE_STATUSBAR
        CreateStatusBar();
#endif }

void wxLogFrame::DoClose()
{
    if ( m_log->OnFrameClose(this) )
    {
                        Show(false);
    }
}

void wxLogFrame::OnClose(wxCommandEvent& WXUNUSED(event))
{
    DoClose();
}

void wxLogFrame::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
    DoClose();
}

#if CAN_SAVE_FILES
void wxLogFrame::OnSave(wxCommandEvent& WXUNUSED(event))
{
    wxString filename;
    wxFile file;
    int rc = OpenLogFile(file, &filename, this);
    if ( rc == -1 )
    {
                return;
    }

    bool bOk = rc != 0;

            int nLines = m_pTextCtrl->GetNumberOfLines();
    for ( int nLine = 0; bOk && nLine < nLines; nLine++ ) {
        bOk = file.Write(m_pTextCtrl->GetLineText(nLine) +
                         wxTextFile::GetEOL());
    }

    if ( bOk )
        bOk = file.Close();

    if ( !bOk ) {
        wxLogError(_("Can't save log contents to file."));
    }
    else {
        wxLogStatus((wxFrame*)this, _("Log saved to the file '%s'."), filename.c_str());
    }
}
#endif 
void wxLogFrame::OnClear(wxCommandEvent& WXUNUSED(event))
{
    m_pTextCtrl->Clear();
}

wxLogFrame::~wxLogFrame()
{
    m_log->OnFrameDelete(this);
}


wxLogWindow::wxLogWindow(wxWindow *pParent,
                         const wxString& szTitle,
                         bool bShow,
                         bool bDoPass)
{
                m_pLogFrame = NULL;

    PassMessages(bDoPass);

    m_pLogFrame = new wxLogFrame(pParent, this, szTitle);

    if ( bShow )
        m_pLogFrame->Show();
}

void wxLogWindow::Show(bool bShow)
{
    m_pLogFrame->Show(bShow);
}

void wxLogWindow::DoLogTextAtLevel(wxLogLevel level, const wxString& msg)
{
    if ( !m_pLogFrame )
        return;

                        if ( level == wxLOG_Trace )
        return;

    m_pLogFrame->ShowLogMessage(msg);
}

wxFrame *wxLogWindow::GetFrame() const
{
    return m_pLogFrame;
}

bool wxLogWindow::OnFrameClose(wxFrame * WXUNUSED(frame))
{
        return true;
}

void wxLogWindow::OnFrameDelete(wxFrame * WXUNUSED(frame))
{
    m_pLogFrame = NULL;
}

wxLogWindow::~wxLogWindow()
{
        delete m_pLogFrame;
}

#endif 

#if wxUSE_LOG_DIALOG

wxString wxLogDialog::ms_details;
size_t wxLogDialog::ms_maxLength = 0;

wxLogDialog::wxLogDialog(wxWindow *parent,
                         const wxArrayString& messages,
                         const wxArrayInt& severity,
                         const wxArrayLong& times,
                         const wxString& caption,
                         long style)
           : wxDialog(parent, wxID_ANY, caption,
                      wxDefaultPosition, wxDefaultSize,
                      wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    
    if ( ms_details.empty() )
    {
                        ms_details = wxTRANSLATE("&Details");
        ms_details = wxGetTranslation(ms_details);
    }

    if ( ms_maxLength == 0 )
    {
        ms_maxLength = (2 * wxGetDisplaySize().x/3) / GetCharWidth();
    }

    size_t count = messages.GetCount();
    m_messages.Alloc(count);
    m_severity.Alloc(count);
    m_times.Alloc(count);

    for ( size_t n = 0; n < count; n++ )
    {
        m_messages.Add(messages[n]);
        m_severity.Add(severity[n]);
        m_times.Add(times[n]);
    }

    m_listctrl = NULL;

    bool isPda = (wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA);

                wxBoxSizer *sizerTop = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *sizerAll = new wxBoxSizer(isPda ? wxVERTICAL : wxHORIZONTAL);

    if (!isPda)
    {
        wxStaticBitmap *icon = new wxStaticBitmap
                                   (
                                    this,
                                    wxID_ANY,
                                    wxArtProvider::GetMessageBoxIcon(style)
                                   );
        sizerAll->Add(icon, wxSizerFlags().Centre());
    }

        wxString message = EllipsizeString(messages.Last());
    wxSizer *szText = CreateTextSizer(message);
    szText->SetMinSize(wxMin(300, wxGetDisplaySize().x / 3), -1);

    sizerAll->Add(szText, wxSizerFlags(1).Centre().Border(wxLEFT | wxRIGHT));

    wxButton *btnOk = new wxButton(this, wxID_OK);
    sizerAll->Add(btnOk, wxSizerFlags().Centre());

    sizerTop->Add(sizerAll, wxSizerFlags().Expand().Border());


    #if wxUSE_COLLPANE
    wxCollapsiblePane * const
        collpane = new wxCollapsiblePane(this, wxID_ANY, ms_details);
    sizerTop->Add(collpane, wxSizerFlags(1).Expand().Border());

    wxWindow *win = collpane->GetPane();
#else
    wxPanel* win = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                               wxBORDER_NONE);
#endif
    wxSizer * const paneSz = new wxBoxSizer(wxVERTICAL);

    CreateDetailsControls(win);

    paneSz->Add(m_listctrl, wxSizerFlags(1).Expand().Border(wxTOP));

#if wxUSE_CLIPBOARD || CAN_SAVE_FILES
    wxBoxSizer * const btnSizer = new wxBoxSizer(wxHORIZONTAL);

    wxSizerFlags flagsBtn;
    flagsBtn.Border(wxLEFT);

#if wxUSE_CLIPBOARD
    btnSizer->Add(new wxButton(win, wxID_COPY), flagsBtn);
#endif 
#if CAN_SAVE_FILES
    btnSizer->Add(new wxButton(win, wxID_SAVE), flagsBtn);
#endif 
    paneSz->Add(btnSizer, wxSizerFlags().Right().Border(wxTOP|wxBOTTOM));
#endif 
    win->SetSizer(paneSz);
    paneSz->SetSizeHints(win);

    SetSizerAndFit(sizerTop);

    Centre();

    if (isPda)
    {
                        Move(wxPoint(GetPosition().x, GetPosition().y / 2));
    }
}

void wxLogDialog::CreateDetailsControls(wxWindow *parent)
{
    wxString fmt = wxLog::GetTimestamp();
    bool hasTimeStamp = !fmt.IsEmpty();

        m_listctrl = new wxListCtrl(parent, wxID_ANY,
                                wxDefaultPosition, wxDefaultSize,
                                wxBORDER_SIMPLE |
                                wxLC_REPORT |
                                wxLC_NO_HEADER |
                                wxLC_SINGLE_SEL);

            m_listctrl->InsertColumn(0, wxT("Message"));

    if (hasTimeStamp)
        m_listctrl->InsertColumn(1, wxT("Time"));

        static const int ICON_SIZE = 16;
    wxImageList *imageList = new wxImageList(ICON_SIZE, ICON_SIZE);

        static const char* const icons[] =
    {
        wxART_ERROR,
        wxART_WARNING,
        wxART_INFORMATION
    };

    bool loadedIcons = true;

    for ( size_t icon = 0; icon < WXSIZEOF(icons); icon++ )
    {
        wxBitmap bmp = wxArtProvider::GetBitmap(icons[icon], wxART_MESSAGE_BOX,
                                                wxSize(ICON_SIZE, ICON_SIZE));

                        if ( !bmp.IsOk() )
        {
            loadedIcons = false;

            break;
        }

        imageList->Add(bmp);
    }

    m_listctrl->SetImageList(imageList, wxIMAGE_LIST_SMALL);

        size_t count = m_messages.GetCount();
    for ( size_t n = 0; n < count; n++ )
    {
        int image;

        if ( loadedIcons )
        {
            switch ( m_severity[n] )
            {
                case wxLOG_Error:
                    image = 0;
                    break;

                case wxLOG_Warning:
                    image = 1;
                    break;

                default:
                    image = 2;
            }
        }
        else         {
            image = -1;
        }

        wxString msg = m_messages[n];
        msg.Replace(wxT("\n"), wxT(" "));
        msg = EllipsizeString(msg);

        m_listctrl->InsertItem(n, msg, image);

        if (hasTimeStamp)
            m_listctrl->SetItem(n, 1, TimeStamp(fmt, (time_t)m_times[n]));
    }

        m_listctrl->SetColumnWidth(0, wxLIST_AUTOSIZE);
    if (hasTimeStamp)
        m_listctrl->SetColumnWidth(1, wxLIST_AUTOSIZE);

        int height = GetCharHeight()*(count + 4);

                        int heightMax = wxGetDisplaySize().y - GetPosition().y - 2*GetMinHeight();

        heightMax *= 9;
    heightMax /= 10;

    m_listctrl->SetSize(wxDefaultCoord, wxMin(height, heightMax));
}

void wxLogDialog::OnListItemActivated(wxListEvent& event)
{
                
                wxString str = m_messages[event.GetIndex()];

            wxMessageBox(str, wxT("Log message"), wxOK, this);
}

void wxLogDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    EndModal(wxID_OK);
}

#if CAN_SAVE_FILES || wxUSE_CLIPBOARD

wxString wxLogDialog::GetLogMessages() const
{
    wxString fmt = wxLog::GetTimestamp();
    if ( fmt.empty() )
    {
                fmt = "%c";
    }

    const size_t count = m_messages.GetCount();

    wxString text;
    text.reserve(count*m_messages[0].length());
    for ( size_t n = 0; n < count; n++ )
    {
        text << TimeStamp(fmt, (time_t)m_times[n])
             << ": "
             << m_messages[n]
             << wxTextFile::GetEOL();
    }

    return text;
}

#endif 
#if wxUSE_CLIPBOARD

void wxLogDialog::OnCopy(wxCommandEvent& WXUNUSED(event))
{
    wxClipboardLocker clip;
    if ( !clip ||
            !wxTheClipboard->AddData(new wxTextDataObject(GetLogMessages())) )
    {
        wxLogError(_("Failed to copy dialog contents to the clipboard."));
    }
}

#endif 
#if CAN_SAVE_FILES

void wxLogDialog::OnSave(wxCommandEvent& WXUNUSED(event))
{
    wxFile file;
    int rc = OpenLogFile(file, NULL, this);
    if ( rc == -1 )
    {
                return;
    }

    if ( !rc || !file.Write(GetLogMessages()) || !file.Close() )
    {
        wxLogError(_("Can't save log contents to file."));
    }
}

#endif 
wxLogDialog::~wxLogDialog()
{
    if ( m_listctrl )
    {
        delete m_listctrl->GetImageList(wxIMAGE_LIST_SMALL);
    }
}

#endif 
#if CAN_SAVE_FILES

static int OpenLogFile(wxFile& file, wxString *pFilename, wxWindow *parent)
{
            wxString filename = wxSaveFileSelector(wxT("log"), wxT("txt"), wxT("log.txt"), parent);
    if ( !filename ) {
                return -1;
    }

            bool bOk = true;     if ( wxFile::Exists(filename) ) {
        bool bAppend = false;
        wxString strMsg;
        strMsg.Printf(_("Append log to file '%s' (choosing [No] will overwrite it)?"),
                      filename.c_str());
        switch ( wxMessageBox(strMsg, _("Question"),
                              wxICON_QUESTION | wxYES_NO | wxCANCEL) ) {
            case wxYES:
                bAppend = true;
                break;

            case wxNO:
                bAppend = false;
                break;

            case wxCANCEL:
                return -1;

            default:
                wxFAIL_MSG(_("invalid message box return value"));
        }

        if ( bAppend ) {
            bOk = file.Open(filename, wxFile::write_append);
        }
        else {
            bOk = file.Create(filename, true );
        }
    }
    else {
        bOk = file.Create(filename);
    }

    if ( pFilename )
        *pFilename = filename;

    return bOk;
}

#endif 
#endif 
#if wxUSE_LOG && wxUSE_TEXTCTRL


wxLogTextCtrl::wxLogTextCtrl(wxTextCtrl *pTextCtrl)
{
    m_pTextCtrl = pTextCtrl;
}

void wxLogTextCtrl::DoLogText(const wxString& msg)
{
    m_pTextCtrl->AppendText(msg + wxS('\n'));
}

#endif 