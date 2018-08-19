
#ifndef   _WX_LOGG_H_
#define   _WX_LOGG_H_

#if wxUSE_GUI

class WXDLLIMPEXP_FWD_CORE wxTextCtrl;
class WXDLLIMPEXP_FWD_CORE wxLogFrame;
class WXDLLIMPEXP_FWD_CORE wxWindow;


#if wxUSE_TEXTCTRL

class WXDLLIMPEXP_CORE wxLogTextCtrl : public wxLog
{
public:
    wxLogTextCtrl(wxTextCtrl *pTextCtrl);

protected:
        virtual void DoLogText(const wxString& msg) wxOVERRIDE;

private:
        wxTextCtrl *m_pTextCtrl;

    wxDECLARE_NO_COPY_CLASS(wxLogTextCtrl);
};

#endif 

#if wxUSE_LOGGUI

class WXDLLIMPEXP_CORE wxLogGui : public wxLog
{
public:
        wxLogGui();

        virtual void Flush() wxOVERRIDE;

protected:
    virtual void DoLogRecord(wxLogLevel level,
                             const wxString& msg,
                             const wxLogRecordInfo& info) wxOVERRIDE;

            wxString GetTitle() const;

            int GetSeverityIcon() const;

        void Clear();


    wxArrayString m_aMessages;          wxArrayInt    m_aSeverity;          wxArrayLong   m_aTimes;             bool          m_bErrors,                          m_bWarnings,                        m_bHasMessages;   
private:
            virtual void DoShowSingleLogMessage(const wxString& message,
                                        const wxString& title,
                                        int style);

        virtual void DoShowMultipleLogMessages(const wxArrayString& messages,
                                           const wxArrayInt& severities,
                                           const wxArrayLong& times,
                                           const wxString& title,
                                           int style);
};

#endif 

#if wxUSE_LOGWINDOW

class WXDLLIMPEXP_CORE wxLogWindow : public wxLogPassThrough
{
public:
    wxLogWindow(wxWindow *pParent,                        const wxString& szTitle,                  bool bShow = true,                        bool bPassToOld = true);  
    virtual ~wxLogWindow();

                void Show(bool bShow = true);
            wxFrame *GetFrame() const;

                                        virtual bool OnFrameClose(wxFrame *frame);
                    virtual void OnFrameDelete(wxFrame *frame);

protected:
    virtual void DoLogTextAtLevel(wxLogLevel level, const wxString& msg) wxOVERRIDE;

private:
    wxLogFrame *m_pLogFrame;      
    wxDECLARE_NO_COPY_CLASS(wxLogWindow);
};

#endif 
#endif 
#endif  
