
#ifndef _WX_FILECTRL_H_BASE_
#define _WX_FILECTRL_H_BASE_

#include "wx/defs.h"

#if wxUSE_FILECTRL

#include "wx/string.h"
#include "wx/event.h"

enum
{
    wxFC_OPEN              = 0x0001,
    wxFC_SAVE              = 0x0002,
    wxFC_MULTIPLE          = 0x0004,
    wxFC_NOSHOWHIDDEN      = 0x0008
};

#define wxFC_DEFAULT_STYLE wxFC_OPEN
extern WXDLLIMPEXP_DATA_CORE(const char) wxFileCtrlNameStr[]; 
class WXDLLIMPEXP_CORE wxFileCtrlBase
{
public:
    virtual ~wxFileCtrlBase() {}

    virtual void SetWildcard( const wxString& wildCard ) = 0;
    virtual void SetFilterIndex( int filterindex ) = 0;
    virtual bool SetDirectory( const wxString& dir ) = 0;

                virtual bool SetFilename( const wxString& name ) = 0;

                    virtual bool SetPath( const wxString& path ) = 0;

    virtual wxString GetFilename() const = 0;
    virtual wxString GetDirectory() const = 0;
    virtual wxString GetWildcard() const = 0;
    virtual wxString GetPath() const = 0;
    virtual void GetPaths( wxArrayString& paths ) const = 0;
    virtual void GetFilenames( wxArrayString& files ) const = 0;
    virtual int GetFilterIndex() const = 0;

    virtual bool HasMultipleFileSelection() const = 0;
    virtual void ShowHidden(bool show) = 0;
};

void GenerateFilterChangedEvent( wxFileCtrlBase *fileCtrl, wxWindow *wnd );
void GenerateFolderChangedEvent( wxFileCtrlBase *fileCtrl, wxWindow *wnd );
void GenerateSelectionChangedEvent( wxFileCtrlBase *fileCtrl, wxWindow *wnd );
void GenerateFileActivatedEvent( wxFileCtrlBase *fileCtrl, wxWindow *wnd, const wxString& filename = wxEmptyString );

#if defined(__WXGTK20__) && !defined(__WXUNIVERSAL__)
    #define wxFileCtrl wxGtkFileCtrl
    #include "wx/gtk/filectrl.h"
#else
    #define wxFileCtrl wxGenericFileCtrl
    #include "wx/generic/filectrlg.h"
#endif

class WXDLLIMPEXP_CORE wxFileCtrlEvent : public wxCommandEvent
{
public:
    wxFileCtrlEvent() {}
    wxFileCtrlEvent( wxEventType type, wxObject *evtObject, int id )
            : wxCommandEvent( type, id )
    {
        SetEventObject( evtObject );
    }

        virtual wxEvent *Clone() const wxOVERRIDE { return new wxFileCtrlEvent( *this ); }

    void SetFiles( const wxArrayString &files ) { m_files = files; }
    void SetDirectory( const wxString &directory ) { m_directory = directory; }
    void SetFilterIndex( int filterIndex ) { m_filterIndex = filterIndex; }

    wxArrayString GetFiles() const { return m_files; }
    wxString GetDirectory() const { return m_directory; }
    int GetFilterIndex() const { return m_filterIndex; }

    wxString GetFile() const;

protected:
    int m_filterIndex;
    wxString m_directory;
    wxArrayString m_files;

    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxFileCtrlEvent);
};

typedef void ( wxEvtHandler::*wxFileCtrlEventFunction )( wxFileCtrlEvent& );

wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_FILECTRL_SELECTIONCHANGED, wxFileCtrlEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_FILECTRL_FILEACTIVATED, wxFileCtrlEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_FILECTRL_FOLDERCHANGED, wxFileCtrlEvent );
wxDECLARE_EXPORTED_EVENT( WXDLLIMPEXP_CORE, wxEVT_FILECTRL_FILTERCHANGED, wxFileCtrlEvent );

#define wxFileCtrlEventHandler(func) \
    wxEVENT_HANDLER_CAST( wxFileCtrlEventFunction, func )

#define EVT_FILECTRL_FILEACTIVATED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FILECTRL_FILEACTIVATED, id, wxFileCtrlEventHandler(fn))

#define EVT_FILECTRL_SELECTIONCHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FILECTRL_SELECTIONCHANGED, id, wxFileCtrlEventHandler(fn))

#define EVT_FILECTRL_FOLDERCHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FILECTRL_FOLDERCHANGED, id, wxFileCtrlEventHandler(fn))

#define EVT_FILECTRL_FILTERCHANGED(id, fn) \
    wx__DECLARE_EVT1(wxEVT_FILECTRL_FILTERCHANGED, id, wxFileCtrlEventHandler(fn))

#endif 
#endif 