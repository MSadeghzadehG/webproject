


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/combo.h"

#ifdef __WXMSW__
#include "wx/msw/private.h"
#endif

#if wxUSE_COMBOBOX
#include "wx/combobox.h"
extern WXDLLEXPORT_DATA(const char) wxComboBoxNameStr[] = "comboBox";


wxDEFINE_FLAGS( wxComboBoxStyle )
wxBEGIN_FLAGS( wxComboBoxStyle )
wxFLAGS_MEMBER(wxBORDER_SIMPLE)
wxFLAGS_MEMBER(wxBORDER_SUNKEN)
wxFLAGS_MEMBER(wxBORDER_DOUBLE)
wxFLAGS_MEMBER(wxBORDER_RAISED)
wxFLAGS_MEMBER(wxBORDER_STATIC)
wxFLAGS_MEMBER(wxBORDER_NONE)

wxFLAGS_MEMBER(wxSIMPLE_BORDER)
wxFLAGS_MEMBER(wxSUNKEN_BORDER)
wxFLAGS_MEMBER(wxDOUBLE_BORDER)
wxFLAGS_MEMBER(wxRAISED_BORDER)
wxFLAGS_MEMBER(wxSTATIC_BORDER)
wxFLAGS_MEMBER(wxBORDER)

wxFLAGS_MEMBER(wxTAB_TRAVERSAL)
wxFLAGS_MEMBER(wxCLIP_CHILDREN)
wxFLAGS_MEMBER(wxTRANSPARENT_WINDOW)
wxFLAGS_MEMBER(wxWANTS_CHARS)
wxFLAGS_MEMBER(wxFULL_REPAINT_ON_RESIZE)
wxFLAGS_MEMBER(wxALWAYS_SHOW_SB )
wxFLAGS_MEMBER(wxVSCROLL)
wxFLAGS_MEMBER(wxHSCROLL)

wxFLAGS_MEMBER(wxCB_SIMPLE)
wxFLAGS_MEMBER(wxCB_SORT)
wxFLAGS_MEMBER(wxCB_READONLY)
wxFLAGS_MEMBER(wxCB_DROPDOWN)

wxEND_FLAGS( wxComboBoxStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxComboBox, wxControl, "wx/combobox.h");

wxBEGIN_PROPERTIES_TABLE(wxComboBox)
wxEVENT_PROPERTY( Select, wxEVT_COMBOBOX, wxCommandEvent )
wxEVENT_PROPERTY( TextEnter, wxEVT_TEXT_ENTER, wxCommandEvent )

wxPROPERTY( Font, wxFont, SetFont, GetFont, wxEMPTY_PARAMETER_VALUE, \
           0 , wxT("Helpstring"), wxT("group"))
wxPROPERTY_COLLECTION( Choices, wxArrayString, wxString, AppendString, \
                      GetStrings, 0 , wxT("Helpstring"), wxT("group"))
wxPROPERTY( Value,wxString, SetValue, GetValue, wxEMPTY_PARAMETER_VALUE, \
           0 , wxT("Helpstring"), wxT("group"))
wxPROPERTY( Selection,int, SetSelection, GetSelection, wxEMPTY_PARAMETER_VALUE, \
           0 , wxT("Helpstring"), wxT("group"))

wxPROPERTY_FLAGS( WindowStyle, wxComboBoxStyle, long, SetWindowStyleFlag, \
                 GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 , \
                 wxT("Helpstring"), wxT("group")) wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxComboBox)

wxCONSTRUCTOR_5( wxComboBox, wxWindow*, Parent, wxWindowID, Id, \
                wxString, Value, wxPoint, Position, wxSize, Size )

#endif 
#if wxUSE_COMBOCTRL

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/log.h"
    #include "wx/dcclient.h"
    #include "wx/settings.h"
    #include "wx/timer.h"
    #include "wx/textctrl.h"
#endif

#include "wx/tooltip.h"


#define DEFAULT_DROPBUTTON_WIDTH                19

#define BMP_BUTTON_MARGIN                       4

#define DEFAULT_POPUP_HEIGHT                    400

#define DEFAULT_TEXT_INDENT                     3

#define COMBO_MARGIN                            2 

#if defined(__WXMSW__)

#include "wx/frame.h"
#define wxCC_GENERIC_TLW_IS_FRAME
#define wxComboCtrlGenericTLW   wxFrame

#define USE_TRANSIENT_POPUP           1 #define TRANSIENT_POPUPWIN_IS_PERFECT 0                                         #define POPUPWIN_IS_PERFECT           0 #define TEXTCTRL_TEXT_CENTERED        0 #define FOCUS_RING                    0 

#elif defined(__WXGTK__)


#include "wx/dialog.h"
#define wxComboCtrlGenericTLW   wxDialog

#if defined(__WXGTK20__)
# include "wx/gtk/private.h"
#else
# include "wx/gtk1/private.h"
#endif


#define USE_TRANSIENT_POPUP           1 #define TRANSIENT_POPUPWIN_IS_PERFECT 1                                         #define POPUPWIN_IS_PERFECT           1 #define TEXTCTRL_TEXT_CENTERED        1 #define FOCUS_RING                    0 
#elif defined(__WXMAC__)

#include "wx/nonownedwnd.h"
#define wxCC_GENERIC_TLW_IS_NONOWNEDWINDOW
#define wxComboCtrlGenericTLW   wxNonOwnedWindow

#define USE_TRANSIENT_POPUP           1 #define TRANSIENT_POPUPWIN_IS_PERFECT 1                                         #define POPUPWIN_IS_PERFECT           1 #define TEXTCTRL_TEXT_CENTERED        1 #define FOCUS_RING                    3 
#undef DEFAULT_DROPBUTTON_WIDTH
#define DEFAULT_DROPBUTTON_WIDTH      22
#undef COMBO_MARGIN
#define COMBO_MARGIN                  FOCUS_RING

#else

#include "wx/dialog.h"
#define wxComboCtrlGenericTLW   wxDialog

#define USE_TRANSIENT_POPUP           0 #define TRANSIENT_POPUPWIN_IS_PERFECT 0                                         #define POPUPWIN_IS_PERFECT           0 #define TEXTCTRL_TEXT_CENTERED        1 #define FOCUS_RING                    0

#endif


#if (!defined(__WXMSW__) && !defined(__WXGTK__) && !defined(__WXMAC__))
#undef wxUSE_POPUPWIN
#define wxUSE_POPUPWIN 0
#endif


#if wxUSE_POPUPWIN
    #include "wx/popupwin.h"
#else
    #undef USE_TRANSIENT_POPUP
    #define USE_TRANSIENT_POPUP 0
#endif


enum
{
    POPUPWIN_NONE                   = 0,
    POPUPWIN_WXPOPUPTRANSIENTWINDOW = 1,
    POPUPWIN_WXPOPUPWINDOW          = 2,
    POPUPWIN_GENERICTLW             = 3
};


#if USE_TRANSIENT_POPUP
    
    #define wxComboPopupWindowBase  wxPopupTransientWindow
    #define PRIMARY_POPUP_TYPE      POPUPWIN_WXPOPUPTRANSIENTWINDOW
    #define USES_WXPOPUPTRANSIENTWINDOW 1

    #if TRANSIENT_POPUPWIN_IS_PERFECT
            #elif POPUPWIN_IS_PERFECT
        #define wxComboPopupWindowBase2     wxPopupWindow
        #define SECONDARY_POPUP_TYPE        POPUPWIN_WXPOPUPWINDOW
        #define USES_WXPOPUPWINDOW          1
    #else
        #define wxComboPopupWindowBase2     wxComboCtrlGenericTLW
        #define SECONDARY_POPUP_TYPE        POPUPWIN_GENERICTLW
        #define USES_GENERICTLW             1
    #endif

#elif wxUSE_POPUPWIN
    
    #define wxComboPopupWindowBase      wxPopupWindow
    #define PRIMARY_POPUP_TYPE          POPUPWIN_WXPOPUPWINDOW
    #define USES_WXPOPUPWINDOW          1

    #if !POPUPWIN_IS_PERFECT
        #define wxComboPopupWindowBase2     wxComboCtrlGenericTLW
        #define SECONDARY_POPUP_TYPE        POPUPWIN_GENERICTLW
        #define USES_GENERICTLW             1
    #endif

#else
    
    #define wxComboPopupWindowBase      wxComboCtrlGenericTLW
    #define PRIMARY_POPUP_TYPE          POPUPWIN_GENERICTLW
    #define USES_GENERICTLW             1

#endif


#ifndef USES_WXPOPUPTRANSIENTWINDOW
    #define USES_WXPOPUPTRANSIENTWINDOW 0
#endif

#ifndef USES_WXPOPUPWINDOW
    #define USES_WXPOPUPWINDOW          0
#endif

#ifndef USES_GENERICTLW
    #define USES_GENERICTLW             0
#endif


#if USES_WXPOPUPWINDOW
    #define INSTALL_TOPLEV_HANDLER      1
#else
    #define INSTALL_TOPLEV_HANDLER      0
#endif


static inline bool IsPopupWinTypePerfect( wxByte popupWinType )
{
#if POPUPWIN_IS_PERFECT && TRANSIENT_POPUPWIN_IS_PERFECT
    wxUnusedVar(popupWinType);
    return true;
#else
    return ( popupWinType == POPUPWIN_GENERICTLW
        #if POPUPWIN_IS_PERFECT
             || popupWinType == POPUPWIN_WXPOPUPWINDOW
        #endif
        #if TRANSIENT_POPUPWIN_IS_PERFECT
             || popupWinType == POPUPWIN_WXPOPUPTRANSIENTWINDOW
        #endif
            );
#endif
}





#if INSTALL_TOPLEV_HANDLER


class wxComboFrameEventHandler : public wxEvtHandler
{
public:
    wxComboFrameEventHandler( wxComboCtrlBase* pCb );
    virtual ~wxComboFrameEventHandler();

    void OnPopup();

    void OnIdle( wxIdleEvent& event );
    void OnMouseEvent( wxMouseEvent& event );
    void OnActivate( wxActivateEvent& event );
    void OnResize( wxSizeEvent& event );
    void OnMove( wxMoveEvent& event );
    void OnMenuEvent( wxMenuEvent& event );
    void OnClose( wxCloseEvent& event );

protected:
    wxWindow*                       m_focusStart;
    wxComboCtrlBase*     m_combo;

private:
    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(wxComboFrameEventHandler, wxEvtHandler)
    EVT_IDLE(wxComboFrameEventHandler::OnIdle)
    EVT_LEFT_DOWN(wxComboFrameEventHandler::OnMouseEvent)
    EVT_RIGHT_DOWN(wxComboFrameEventHandler::OnMouseEvent)
    EVT_SIZE(wxComboFrameEventHandler::OnResize)
    EVT_MOVE(wxComboFrameEventHandler::OnMove)
    EVT_MENU_HIGHLIGHT(wxID_ANY,wxComboFrameEventHandler::OnMenuEvent)
    EVT_MENU_OPEN(wxComboFrameEventHandler::OnMenuEvent)
    EVT_ACTIVATE(wxComboFrameEventHandler::OnActivate)
    EVT_CLOSE(wxComboFrameEventHandler::OnClose)
wxEND_EVENT_TABLE()

wxComboFrameEventHandler::wxComboFrameEventHandler( wxComboCtrlBase* combo )
    : wxEvtHandler()
{
    m_combo = combo;
}

wxComboFrameEventHandler::~wxComboFrameEventHandler()
{
}

void wxComboFrameEventHandler::OnPopup()
{
    m_focusStart = ::wxWindow::FindFocus();
}

void wxComboFrameEventHandler::OnIdle( wxIdleEvent& event )
{
    wxWindow* winFocused = ::wxWindow::FindFocus();

    wxWindow* popup = m_combo->GetPopupControl()->GetControl();
    wxWindow* winpopup = m_combo->GetPopupWindow();

    if ( !winFocused || (
         winFocused != m_focusStart &&
         winFocused != popup &&
         winFocused->GetParent() != popup &&
         winFocused != winpopup &&
         winFocused->GetParent() != winpopup &&
         winFocused != m_combo &&
         winFocused != m_combo->GetButton()         )
       )
    {
        m_combo->HidePopup(true);
    }

    event.Skip();
}

void wxComboFrameEventHandler::OnMenuEvent( wxMenuEvent& event )
{
    m_combo->HidePopup(true);
    event.Skip();
}

void wxComboFrameEventHandler::OnMouseEvent( wxMouseEvent& event )
{
    m_combo->HidePopup(true);
    event.Skip();
}

void wxComboFrameEventHandler::OnClose( wxCloseEvent& event )
{
    m_combo->HidePopup(true);
    event.Skip();
}

void wxComboFrameEventHandler::OnActivate( wxActivateEvent& event )
{
    m_combo->HidePopup(true);
    event.Skip();
}

void wxComboFrameEventHandler::OnResize( wxSizeEvent& event )
{
    m_combo->HidePopup(true);
    event.Skip();
}

void wxComboFrameEventHandler::OnMove( wxMoveEvent& event )
{
    m_combo->HidePopup(true);
    event.Skip();
}

#endif 

class wxComboPopupWindow : public wxComboPopupWindowBase
{
public:

    wxComboPopupWindow( wxComboCtrlBase *parent,
                        int style )
    #if USES_WXPOPUPWINDOW || USES_WXPOPUPTRANSIENTWINDOW
                       : wxComboPopupWindowBase(parent,style)
    #else
                       : wxComboPopupWindowBase(parent,
                                                wxID_ANY,
                                                wxEmptyString,
                                                wxPoint(-21,-21),
                                                wxSize(20,20),
                                                style)
    #endif
    {
        m_inShow = 0;
    }

#if USES_WXPOPUPTRANSIENTWINDOW
    virtual bool Show( bool show ) wxOVERRIDE;
    virtual bool ProcessLeftDown(wxMouseEvent& event) wxOVERRIDE;
protected:
    virtual void OnDismiss() wxOVERRIDE;
#endif

private:
    wxByte      m_inShow;
};


#if USES_WXPOPUPTRANSIENTWINDOW
bool wxComboPopupWindow::Show( bool show )
{
        if ( m_inShow )
        return wxComboPopupWindowBase::Show(show);

    m_inShow++;

    wxPopupTransientWindow* const
        ptw = static_cast<wxPopupTransientWindow*>(this);

    if ( show != ptw->IsShown() )
    {
        if ( show )
                                                ptw->Show();
        else
            ptw->Dismiss();
    }

    m_inShow--;

    return true;
}

bool wxComboPopupWindow::ProcessLeftDown(wxMouseEvent& event)
{
    return wxPopupTransientWindow::ProcessLeftDown(event);
}

void wxComboPopupWindow::OnDismiss()
{
    wxComboCtrlBase* combo = (wxComboCtrlBase*) GetParent();
    wxASSERT_MSG( wxDynamicCast(combo, wxComboCtrlBase),
                  wxT("parent might not be wxComboCtrl, but check wxIMPLEMENT_DYNAMIC_CLASS2() macro for correctness") );

    combo->OnPopupDismiss(true);
}
#endif 


class wxComboPopupWindowEvtHandler : public wxEvtHandler
{
public:

    wxComboPopupWindowEvtHandler( wxComboCtrlBase *parent )
    {
        m_combo = parent;
    }

    void OnSizeEvent( wxSizeEvent& event );
    void OnKeyEvent(wxKeyEvent& event);
#if USES_GENERICTLW
    void OnActivate( wxActivateEvent& event );
#endif

private:
    wxComboCtrlBase*    m_combo;

    wxDECLARE_EVENT_TABLE();
};


wxBEGIN_EVENT_TABLE(wxComboPopupWindowEvtHandler, wxEvtHandler)
    EVT_KEY_DOWN(wxComboPopupWindowEvtHandler::OnKeyEvent)
    EVT_KEY_UP(wxComboPopupWindowEvtHandler::OnKeyEvent)
    EVT_CHAR(wxComboPopupWindowEvtHandler::OnKeyEvent)
#if USES_GENERICTLW
    EVT_ACTIVATE(wxComboPopupWindowEvtHandler::OnActivate)
#endif
    EVT_SIZE(wxComboPopupWindowEvtHandler::OnSizeEvent)
wxEND_EVENT_TABLE()


void wxComboPopupWindowEvtHandler::OnSizeEvent( wxSizeEvent& WXUNUSED(event) )
{
    }

void wxComboPopupWindowEvtHandler::OnKeyEvent( wxKeyEvent& event )
{
        wxWindowList children = m_combo->GetPopupWindow()->GetChildren();
    wxWindowList::iterator node = children.begin();
    wxWindow* child = (wxWindow*)*node;
    child->GetEventHandler()->ProcessEvent(event);
}

#if USES_GENERICTLW
void wxComboPopupWindowEvtHandler::OnActivate( wxActivateEvent& event )
{
    if ( !event.GetActive() )
    {
                m_combo->HidePopup(true);

        event.Skip();
    }
}
#endif



wxComboPopup::~wxComboPopup()
{
}

void wxComboPopup::OnPopup()
{
}

void wxComboPopup::OnDismiss()
{
}

wxComboCtrl* wxComboPopup::GetComboCtrl() const
{
    return wxStaticCast(m_combo, wxComboCtrl);
}

wxSize wxComboPopup::GetAdjustedSize( int minWidth,
                                      int prefHeight,
                                      int WXUNUSED(maxHeight) )
{
    return wxSize(minWidth,prefHeight);
}

void wxComboPopup::DefaultPaintComboControl( wxComboCtrlBase* combo,
                                             wxDC& dc, const wxRect& rect )
{
    if ( combo->GetWindowStyle() & wxCB_READONLY )     {
        combo->PrepareBackground(dc,rect,0);

        dc.DrawText( combo->GetValue(),
                     rect.x + combo->m_marginLeft,
                     (rect.height-dc.GetCharHeight())/2 + rect.y );
    }
}

void wxComboPopup::PaintComboControl( wxDC& dc, const wxRect& rect )
{
    DefaultPaintComboControl(m_combo,dc,rect);
}

void wxComboPopup::OnComboKeyEvent( wxKeyEvent& event )
{
    event.Skip();
}

void wxComboPopup::OnComboCharEvent( wxKeyEvent& event )
{
    event.Skip();
}

void wxComboPopup::OnComboDoubleClick()
{
}

void wxComboPopup::SetStringValue( const wxString& WXUNUSED(value) )
{
}

bool wxComboPopup::FindItem(const wxString& WXUNUSED(item),
                            wxString* WXUNUSED(trueItem))
{
    return true;
}

bool wxComboPopup::LazyCreate()
{
    return false;
}

void wxComboPopup::Dismiss()
{
    m_combo->HidePopup(true);
}

void wxComboPopup::DestroyPopup()
{
            wxWindow* popupCtrl = GetControl();
    if ( popupCtrl )
    {
                                                      #ifndef wxNO_RTTI
                        if ( dynamic_cast<void*>(this) !=
             dynamic_cast<void*>(popupCtrl) )
            delete this;
      #endif
        popupCtrl->Destroy();
    }
    else
    {
        delete this;
    }
}


class wxComboBoxExtraInputHandler : public wxEvtHandler
{
public:

    wxComboBoxExtraInputHandler( wxComboCtrlBase* combo )
        : wxEvtHandler()
    {
        m_combo = combo;
    }
    virtual ~wxComboBoxExtraInputHandler() { }
    void OnKey(wxKeyEvent& event);
    void OnFocus(wxFocusEvent& event);

protected:
    wxComboCtrlBase*   m_combo;

private:
    wxDECLARE_EVENT_TABLE();
};


wxBEGIN_EVENT_TABLE(wxComboBoxExtraInputHandler, wxEvtHandler)
    EVT_KEY_DOWN(wxComboBoxExtraInputHandler::OnKey)
    EVT_KEY_UP(wxComboBoxExtraInputHandler::OnKey)
    EVT_CHAR(wxComboBoxExtraInputHandler::OnKey)
    EVT_SET_FOCUS(wxComboBoxExtraInputHandler::OnFocus)
    EVT_KILL_FOCUS(wxComboBoxExtraInputHandler::OnFocus)
wxEND_EVENT_TABLE()


void wxComboBoxExtraInputHandler::OnKey(wxKeyEvent& event)
{
        wxComboCtrlBase* combo = m_combo;

    wxKeyEvent redirectedEvent(event);
    redirectedEvent.SetId(combo->GetId());
    redirectedEvent.SetEventObject(combo);

    if ( !combo->GetEventHandler()->ProcessEvent(redirectedEvent) )
    {
                if ( event.GetKeyCode() != WXK_TAB )
            event.Skip();
    }
}

void wxComboBoxExtraInputHandler::OnFocus(wxFocusEvent& event)
{
            if ( event.GetEventType() == wxEVT_SET_FOCUS &&
        !(m_combo->GetInternalFlags() & wxCC_NO_TEXT_AUTO_SELECT) )
    {
        if ( m_combo->GetTextCtrl() )
            m_combo->GetTextCtrl()->SelectAll();
        else
            m_combo->SelectAll();
    }

                            wxFocusEvent evt2(event);
    evt2.SetId(m_combo->GetId());
    evt2.SetEventObject(m_combo);
    m_combo->GetEventHandler()->ProcessEvent(evt2);

    event.Skip();
}



class wxComboPopupEvtHandler : public wxEvtHandler
{
public:

    wxComboPopupEvtHandler( wxComboCtrlBase* combo )
        : wxEvtHandler()
    {
        m_combo = combo;
        m_beenInside = false;

                        m_blockEventsToPopup = true;
    }
    virtual ~wxComboPopupEvtHandler() { }

    void OnMouseEvent( wxMouseEvent& event );

        void OnPopupDismiss()
    {
        m_beenInside = false;
        m_blockEventsToPopup = true;
    }

protected:
    wxComboCtrlBase*     m_combo;

    bool                m_beenInside;
    bool                m_blockEventsToPopup;

private:
    wxDECLARE_EVENT_TABLE();
};


wxBEGIN_EVENT_TABLE(wxComboPopupEvtHandler, wxEvtHandler)
    EVT_MOUSE_EVENTS(wxComboPopupEvtHandler::OnMouseEvent)
wxEND_EVENT_TABLE()


void wxComboPopupEvtHandler::OnMouseEvent( wxMouseEvent& event )
{
    wxPoint pt = event.GetPosition();
    wxSize sz = m_combo->GetPopupControl()->GetControl()->GetClientSize();
    int evtType = event.GetEventType();
    bool isInside = pt.x >= 0 && pt.y >= 0 && pt.x < sz.x && pt.y < sz.y;
    bool relayToButton = false;

    event.Skip();

    if ( !isInside || !m_combo->IsPopupShown() )
    {
        
        if ( evtType == wxEVT_MOTION ||
             evtType == wxEVT_LEFT_DOWN ||
             evtType == wxEVT_LEFT_UP ||
             evtType == wxEVT_RIGHT_DOWN )
        {
                        event.Skip(false);
        }
    }
    else
    {
        
        m_beenInside = true;

                                                        if ( evtType == wxEVT_MOTION )
        {
            if ( m_blockEventsToPopup )
            {
                if ( event.LeftIsDown() )
                    event.Skip(false);
                else
                    m_blockEventsToPopup = false;
            }
        }
        else if ( evtType == wxEVT_LEFT_DOWN )
        {
            if ( m_blockEventsToPopup )
                m_blockEventsToPopup = false;
        }
        else if ( evtType == wxEVT_LEFT_UP )
        {
            if ( m_blockEventsToPopup )
            {
                                                m_blockEventsToPopup = false;
                event.Skip(false);

                                                                                relayToButton = true;
            }
        }
        else if ( m_blockEventsToPopup )
        {
            event.Skip(false);
        }
    }

                        if ( evtType == wxEVT_LEFT_UP )
    {
        if ( !m_combo->IsPopupShown() )
        {
            event.Skip(false);
            relayToButton = true;
        }
        else if ( !isInside && !m_beenInside )
        {
                        relayToButton = true;
        }
    }

    if ( relayToButton )
    {
        wxWindow* btn = m_combo->GetButton();
        if ( btn )
            btn->GetEventHandler()->ProcessEvent(event);
        else
                                                m_combo->HandleButtonMouseEvent(event, 0);
    }
}


class wxComboCtrlTextCtrl : public wxTextCtrl
{
public:
    wxComboCtrlTextCtrl() : wxTextCtrl() { }
    virtual ~wxComboCtrlTextCtrl() { }

    virtual wxWindow *GetMainWindowOfCompositeControl() wxOVERRIDE
    {
        wxComboCtrl* combo = (wxComboCtrl*) GetParent();

                                return combo->GetMainWindowOfCompositeControl();
    }
};



wxBEGIN_EVENT_TABLE(wxComboCtrlBase, wxControl)
    EVT_SIZE(wxComboCtrlBase::OnSizeEvent)
    EVT_SET_FOCUS(wxComboCtrlBase::OnFocusEvent)
    EVT_KILL_FOCUS(wxComboCtrlBase::OnFocusEvent)
    EVT_IDLE(wxComboCtrlBase::OnIdleEvent)
        EVT_KEY_DOWN(wxComboCtrlBase::OnKeyEvent)
    EVT_CHAR(wxComboCtrlBase::OnCharEvent)
    EVT_SYS_COLOUR_CHANGED(wxComboCtrlBase::OnSysColourChanged)
wxEND_EVENT_TABLE()


wxIMPLEMENT_ABSTRACT_CLASS(wxComboCtrlBase, wxControl);

void wxComboCtrlBase::Init()
{
    m_winPopup = NULL;
    m_popup = NULL;
    m_popupWinState = Hidden;
    m_btn = NULL;
    m_text = NULL;
    m_popupInterface = NULL;

    m_popupEvtHandler = NULL;
    m_textEvtHandler = NULL;

#if INSTALL_TOPLEV_HANDLER
    m_toplevEvtHandler = NULL;
#endif

    m_mainCtrlWnd = this;

    m_heightPopup = -1;
    m_widthMinPopup = -1;
    m_anchorSide = 0;
    m_widthCustomPaint = 0;
    m_widthCustomBorder = 0;

    m_btnState = 0;
    m_btnWidDefault = 0;
    m_blankButtonBg = false;
    m_ignoreEvtText = 0;
    m_popupWinType = POPUPWIN_NONE;
    m_btnWid = m_btnHei = -1;
    m_btnSide = wxRIGHT;
    m_btnSpacingX = 0;

    m_extLeft = 0;
    m_extRight = 0;
    m_marginLeft = -1;
    m_iFlags = 0;
    m_textCtrlStyle = 0;
    m_timeCanAcceptClick = 0;

    m_resetFocus = false;
    m_hasTcBgCol = false;
}

bool wxComboCtrlBase::Create(wxWindow *parent,
                             wxWindowID id,
                             const wxString& value,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxValidator& validator,
                             const wxString& name)
{
    if ( !wxControl::Create(parent,
                            id,
                            pos,
                            size,
                            style | wxWANTS_CHARS,
                            validator,
                            name) )
        return false;

    m_valueString = value;

        OnThemeChange();
    m_marginLeft = GetNativeTextIndent();

    m_iFlags |= wxCC_IFLAG_CREATED;

            if ( size.x > 0 && size.y > 0 )
    {
        wxSizeEvent evt(size,GetId());
        evt.SetEventObject(this);
        GetEventHandler()->AddPendingEvent(evt);
    }

    return true;
}

void wxComboCtrlBase::InstallInputHandlers()
{
    if ( m_text )
    {
        m_textEvtHandler = new wxComboBoxExtraInputHandler(this);
        m_text->PushEventHandler(m_textEvtHandler);
    }
}

void
wxComboCtrlBase::CreateTextCtrl(int style)
{
    if ( !(m_windowStyle & wxCB_READONLY) )
    {
        if ( m_text )
            m_text->Destroy();

                                        style |= wxTE_PROCESS_TAB | m_textCtrlStyle;

        if ( HasFlag(wxTE_PROCESS_ENTER) )
            style |= wxTE_PROCESS_ENTER;

                                if ( m_textEvtHandler )
            m_ignoreEvtText = 1;
        else
            m_ignoreEvtText = 0;

        m_text = new wxComboCtrlTextCtrl();
        m_text->Create(this, wxID_ANY, m_valueString,
                       wxDefaultPosition, wxSize(10,-1),
                       style);

                wxWindowID id = m_text->GetId();
        m_text->Connect(id, wxEVT_TEXT,
                        wxCommandEventHandler(wxComboCtrlBase::OnTextCtrlEvent),
                        NULL, this);
        if ( style & wxTE_PROCESS_ENTER )
        {
            m_text->Connect(id, wxEVT_TEXT_ENTER,
                            wxCommandEventHandler(wxComboCtrlBase::OnTextCtrlEvent),
                            NULL, this);
        }

        m_text->SetHint(m_hintText);
    }
}

void wxComboCtrlBase::OnThemeChange()
{
            
#if defined(__WXMSW__) || defined(__WXGTK__)
    wxVisualAttributes vattrs = wxComboBox::GetClassDefaultAttributes();
#else
    wxVisualAttributes vattrs;
    vattrs.colFg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
    vattrs.colBg = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
#endif

    if ( !m_hasTcBgCol )
        m_tcBgCol = vattrs.colBg;

#ifndef __WXMAC__
            if ( !m_hasFgCol )
    {
        SetOwnForegroundColour(vattrs.colFg);
    }
    if ( !HasTransparentBackground() )
    {
        SetOwnBackgroundColour(GetParent()->GetBackgroundColour());
    }
#endif }

wxComboCtrlBase::~wxComboCtrlBase()
{
    if ( HasCapture() )
        ReleaseMouse();

#if INSTALL_TOPLEV_HANDLER
    delete ((wxComboFrameEventHandler*)m_toplevEvtHandler);
    m_toplevEvtHandler = NULL;
#endif

    DestroyPopup();

    if ( m_text )
        m_text->RemoveEventHandler(m_textEvtHandler);

    delete m_textEvtHandler;
}



void wxComboCtrlBase::CalculateAreas( int btnWidth )
{
    wxSize sz = GetClientSize();
    int customBorder = m_widthCustomBorder;
    int btnBorder; 
                if ( ( (m_iFlags & wxCC_BUTTON_OUTSIDE_BORDER) ||
                (m_bmpNormal.IsOk() && m_blankButtonBg) ) &&
         m_btnSpacingX == 0 &&
         m_btnHei <= 0 )
    {
        m_iFlags |= wxCC_IFLAG_BUTTON_OUTSIDE;
        btnBorder = 0;
    }
    else if ( (m_iFlags & wxCC_BUTTON_COVERS_BORDER) &&
              m_btnSpacingX == 0 && !m_bmpNormal.IsOk() )
    {
        m_iFlags &= ~(wxCC_IFLAG_BUTTON_OUTSIDE);
        btnBorder = 0;
    }
    else
    {
        m_iFlags &= ~(wxCC_IFLAG_BUTTON_OUTSIDE);
        btnBorder = customBorder;
    }

        if ( m_marginLeft < 0 )
        m_marginLeft = GetNativeTextIndent();

    int butWidth = btnWidth;

    if ( butWidth <= 0 )
        butWidth = m_btnWidDefault;
    else
        m_btnWidDefault = butWidth;

    if ( butWidth <= 0 )
        return;

    int butHeight = sz.y - btnBorder*2;

        if ( m_btnWid > 0 )
        butWidth = m_btnWid;
    else
    {
                        int bestHeight = GetBestSize().y;
        int height = GetSize().y;

        if ( height < bestHeight )
        {
                                                if ( height > FromDIP(18) )
                butWidth = (height*butWidth)/bestHeight;
            else
                butWidth = butHeight;
        }
    }

        if ( m_btnHei > 0 )
        butHeight = m_btnHei;

                    if ( m_bmpNormal.IsOk() )
    {
        int bmpReqWidth = m_bmpNormal.GetWidth();
        int bmpReqHeight = m_bmpNormal.GetHeight();

                if ( m_blankButtonBg )
        {
            bmpReqWidth += BMP_BUTTON_MARGIN*2;
            bmpReqHeight += BMP_BUTTON_MARGIN*2;
        }

        if ( butWidth < bmpReqWidth || ( m_btnWid == 0 && !m_blankButtonBg ) )
            butWidth = bmpReqWidth;
        if ( butHeight < bmpReqHeight || ( m_btnHei == 0 && !m_blankButtonBg ) )
            butHeight = bmpReqHeight;

                if ( (sz.y-(customBorder*2)) < butHeight && btnWidth == 0 )
        {
            int newY = butHeight+(customBorder*2);
            SetClientSize(wxDefaultCoord,newY);
            if ( m_bmpNormal.IsOk() || m_btnArea.width != butWidth || m_btnArea.height != butHeight )
                m_iFlags |= wxCC_IFLAG_HAS_NONSTANDARD_BUTTON;
            else
                m_iFlags &= ~wxCC_IFLAG_HAS_NONSTANDARD_BUTTON;

            sz.y = newY;
        }
    }

    int butAreaWid = butWidth + (m_btnSpacingX*2);

    m_btnSize.x = butWidth;
    m_btnSize.y = butHeight;

    m_btnArea.x = ( m_btnSide==wxRIGHT ? sz.x - butAreaWid - btnBorder : btnBorder );
    m_btnArea.y = btnBorder + FOCUS_RING;
    m_btnArea.width = butAreaWid;
    m_btnArea.height = sz.y - ((btnBorder+FOCUS_RING)*2);

    m_tcArea.x = ( m_btnSide==wxRIGHT ? 0 : butAreaWid ) + customBorder;
    m_tcArea.y = customBorder + FOCUS_RING;
    m_tcArea.width = sz.x - butAreaWid - (customBorder*2) - FOCUS_RING;
    m_tcArea.height = sz.y - ((customBorder+FOCUS_RING)*2);


}

void wxComboCtrlBase::PositionTextCtrl( int textCtrlXAdjust, int textCtrlYAdjust )
{
    if ( !m_text )
        return;

    wxSize sz = GetClientSize();

    int customBorder = m_widthCustomBorder;
    if ( (m_text->GetWindowStyleFlag() & wxBORDER_MASK) == wxNO_BORDER )
    {
        int x;

        if ( !m_widthCustomPaint )
        {
                                    if ( m_text->SetMargins(0) )
                textCtrlXAdjust = 0;
            x = m_tcArea.x + m_marginLeft + textCtrlXAdjust;
        }
        else
        {
                                    m_text->SetMargins(m_marginLeft);
            x = m_tcArea.x + m_widthCustomPaint +
                m_marginLeft + textCtrlXAdjust;
        }

        #if !TEXTCTRL_TEXT_CENTERED
        int tcSizeY = m_text->GetBestSize().y;
        int diff0 = sz.y - tcSizeY;
        int y = textCtrlYAdjust + (diff0/2);
#else
        wxUnusedVar(textCtrlYAdjust);
        int y = 0;
#endif

        if ( y < customBorder )
            y = customBorder;

        m_text->SetSize(x,
                        y,
                        m_tcArea.width - m_tcArea.x - x,
                        -1 );

                wxSize tsz = m_text->GetSize();
        int diff1 = (y + tsz.y) - (sz.y - customBorder);
        if ( diff1 >= 0 )
        {
            tsz.y = tsz.y - diff1 - 1;
            m_text->SetSize(tsz);
        }
    }
    else
    {
                int w = m_tcArea.width - m_widthCustomPaint;
        if (w < 0) w = 0;
        m_text->SetSize( m_tcArea.x + m_widthCustomPaint,
                         m_tcArea.y,
                         w,
                         m_tcArea.height );
    }
}

wxSize wxComboCtrlBase::DoGetBestSize() const
{
    int width = m_text ? m_text->GetBestSize().x : 80;

    return GetSizeFromTextSize(width);
}

wxSize wxComboCtrlBase::DoGetSizeFromTextSize(int xlen, int ylen) const
{
    
    int fhei;

#if defined(__WXMSW__) && !defined(__WXUNIVERSAL__)
    fhei = EDIT_HEIGHT_FROM_CHAR_HEIGHT(GetCharHeight());
#elif defined(__WXGTK__) && !defined(__WXUNIVERSAL__)
            static wxString s_last_font;
    static int s_last_fhei = -1;
    wxString fontdesc;
    if ( m_font.IsOk() )
        fontdesc = m_font.GetNativeFontInfoDesc();
    if ( s_last_fhei != -1 && fontdesc == s_last_font )
    {
        fhei = s_last_fhei;
    }
    else
    {
        wxComboBox* cb = new wxComboBox;
#ifndef __WXGTK3__
                cb->Hide();
#endif
        cb->Create(const_cast<wxComboCtrlBase*>(this), wxID_ANY);
        if ( m_font.IsOk() )
            cb->SetFont(m_font);
        s_last_font = fontdesc;
        s_last_fhei = fhei = cb->GetBestSize().y;
        cb->Destroy();
    }
#else
    if ( m_font.IsOk() )
        fhei = (m_font.GetPointSize()*2) + 5;
    else if ( wxNORMAL_FONT->IsOk() )
        fhei = (wxNORMAL_FONT->GetPointSize()*2) + 5;
    else
        fhei = 22;
#endif 
        int btnSizeY = m_btnSize.y;
    if ( m_bmpNormal.IsOk() && fhei < btnSizeY )
        fhei = btnSizeY;

    

#ifdef __WXMAC__
        switch ( m_windowVariant )
    {
        case wxWINDOW_VARIANT_NORMAL:
        default :
            fhei = 22;
            break;
        case wxWINDOW_VARIANT_SMALL:
            fhei = 19;
            break;
        case wxWINDOW_VARIANT_MINI:
            fhei = 15;
            break;
    }
#endif

    fhei += 2 * FOCUS_RING;

        int fwid = xlen + FOCUS_RING + COMBO_MARGIN + DEFAULT_DROPBUTTON_WIDTH;

        wxPoint marg( GetMargins() );
    fwid += wxMax(0, marg.x);
    fhei += wxMax(0, marg.y);

    if ( ylen > 0 )
        fhei += ylen - GetCharHeight();

    return wxSize(fwid, fhei);
}

void wxComboCtrlBase::OnSizeEvent( wxSizeEvent& event )
{
    if ( !IsCreated() )
        return;

        OnResize();

    event.Skip();
}


bool wxComboCtrlBase::Enable(bool enable)
{
    if ( !wxControl::Enable(enable) )
        return false;

    if ( m_btn )
        m_btn->Enable(enable);
    if ( m_text )
        m_text->Enable(enable);

    Refresh();

    return true;
}

bool wxComboCtrlBase::Show(bool show)
{
    if ( !wxControl::Show(show) )
        return false;

    if (m_btn)
        m_btn->Show(show);

    if (m_text)
        m_text->Show(show);

    return true;
}

bool wxComboCtrlBase::SetFont ( const wxFont& font )
{
    if ( !wxControl::SetFont(font) )
        return false;

    if ( m_text )
    {
                        m_text->Hide();
        m_text->SetFont(font);
        OnResize();
        m_text->Show();
    }

    return true;
}

#if wxUSE_TOOLTIPS
void wxComboCtrlBase::DoSetToolTip(wxToolTip *tooltip)
{
    wxControl::DoSetToolTip(tooltip);

        if ( tooltip )
    {
        const wxString &tip = tooltip->GetTip();
        if ( m_text ) m_text->SetToolTip(tip);
        if ( m_btn ) m_btn->SetToolTip(tip);
    }
    else
    {
        if ( m_text ) m_text->SetToolTip( NULL );
        if ( m_btn ) m_btn->SetToolTip( NULL );
    }
}
#endif 
bool wxComboCtrlBase::SetForegroundColour(const wxColour& colour)
{
    if ( wxControl::SetForegroundColour(colour) )
    {
        if ( m_text )
            m_text->SetForegroundColour(colour);
        return true;
    }
    return false;
}

bool wxComboCtrlBase::SetBackgroundColour(const wxColour& colour)
{
    if ( m_text )
        m_text->SetBackgroundColour(colour);
    m_tcBgCol = colour;
    m_hasTcBgCol = true;
    return true;
}

wxColour wxComboCtrlBase::GetBackgroundColour() const
{
    if ( m_text )
        return m_text->GetBackgroundColour();
    return m_tcBgCol;
}


#if (!defined(__WXMSW__)) || defined(__WXUNIVERSAL__)
void wxComboCtrlBase::PrepareBackground( wxDC& dc, const wxRect& rect, int flags ) const
{
    wxSize sz = GetClientSize();
    bool isEnabled;
    bool doDrawFocusRect; 
        int focusSpacingX;
    int focusSpacingY;

    if ( !(flags & wxCONTROL_ISSUBMENU) )
    {
                isEnabled = IsEnabled();
        doDrawFocusRect = ShouldDrawFocus() && !(m_iFlags & wxCC_FULL_BUTTON);

                focusSpacingX = isEnabled ? 2 : 1;
        focusSpacingY = sz.y > (GetCharHeight()+2) && isEnabled ? 2 : 1;
    }
    else
    {
                isEnabled = true;         doDrawFocusRect = (flags & wxCONTROL_SELECTED) != 0;

        focusSpacingX = 0;
        focusSpacingY = 0;
    }

        wxRect selRect(rect);
    selRect.y += focusSpacingY;
    selRect.height -= (focusSpacingY*2);

    int wcp = 0;

    if ( !(flags & wxCONTROL_ISSUBMENU) )
        wcp += m_widthCustomPaint;

    selRect.x += wcp + focusSpacingX;
    selRect.width -= wcp + (focusSpacingX*2);

    wxColour bgCol;
    wxColour fgCol;

    bool doDrawSelRect = true;

        if ( isEnabled )
    {
        if ( doDrawFocusRect )
        {
            fgCol = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
        }
        else if ( m_hasFgCol )
        {
                        fgCol = GetForegroundColour();
        }
        else
        {
            fgCol = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
        }
    }
    else
    {
        fgCol = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);
    }

        if ( isEnabled )
    {
        if ( doDrawFocusRect )
        {
            bgCol = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
        }
        else if ( m_hasTcBgCol )
        {
                        bgCol = m_tcBgCol;
        }
        else
        {
#ifndef __WXMAC__              doDrawSelRect = false;
            bgCol = GetBackgroundColour();
#else
            bgCol = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
#endif
        }
    }
    else
    {
#ifndef __WXMAC__          bgCol = GetBackgroundColour();
#else
        bgCol = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
#endif
    }

    dc.SetTextForeground( fgCol );
    dc.SetBrush( bgCol );
    if ( doDrawSelRect )
    {
        dc.SetPen( bgCol );
        dc.DrawRectangle( selRect );
    }

            wxRect clipRect(rect.x,rect.y,
                    (selRect.x+selRect.width)-rect.x,rect.height);
    dc.SetClippingRegion(clipRect);
}
#else
void wxComboCtrlBase::PrepareBackground( wxDC&, const wxRect&, int ) const
{
}
#endif

void wxComboCtrlBase::DrawButton( wxDC& dc, const wxRect& rect, int flags )
{
    int drawState = m_btnState;

    if ( (m_iFlags & wxCC_BUTTON_STAYS_DOWN) &&
         GetPopupWindowState() >= Animating )
        drawState |= wxCONTROL_PRESSED;

    wxRect drawRect(rect.x+m_btnSpacingX,
                    rect.y+((rect.height-m_btnSize.y)/2),
                    m_btnSize.x,
                    m_btnSize.y);

        if ( drawRect.y < rect.y )
        drawRect.y = rect.y;
    if ( drawRect.height > rect.height )
        drawRect.height = rect.height;

    bool enabled = IsEnabled();

    if ( !enabled )
        drawState |= wxCONTROL_DISABLED;

            if ( (flags & Button_PaintBackground) &&
            (!HasTransparentBackground() ||
             !(m_iFlags & wxCC_IFLAG_BUTTON_OUTSIDE)) )
    {
        wxColour bgCol;

        if ( m_iFlags & wxCC_IFLAG_BUTTON_OUTSIDE )
            bgCol = GetParent()->GetBackgroundColour();
        else
            bgCol = GetBackgroundColour();

        dc.SetBrush(bgCol);
        dc.SetPen(bgCol);
        dc.DrawRectangle(rect);
    }

    if ( !m_bmpNormal.IsOk() )
    {
        if ( flags & Button_BitmapOnly )
            return;

                wxRendererNative::Get().DrawComboBoxDropButton(this,
                                                       dc,
                                                       drawRect,
                                                       drawState);
    }
    else
    {
        
        wxBitmap* pBmp;

        if ( !enabled )
            pBmp = &m_bmpDisabled;
        else if ( m_btnState & wxCONTROL_PRESSED )
            pBmp = &m_bmpPressed;
        else if ( m_btnState & wxCONTROL_CURRENT )
            pBmp = &m_bmpHover;
        else
            pBmp = &m_bmpNormal;

        if ( m_blankButtonBg )
        {
            if ( !(flags & Button_BitmapOnly) )
            {
                wxRendererNative::Get().DrawPushButton(this,
                                                       dc,
                                                       drawRect,
                                                       drawState);
            }
        }

                dc.DrawBitmap(*pBmp,
                      drawRect.x + (drawRect.width-pBmp->GetWidth())/2,
                      drawRect.y + (drawRect.height-pBmp->GetHeight())/2,
                      true);
    }
}

void wxComboCtrlBase::RecalcAndRefresh()
{
    if ( IsCreated() )
    {
        wxSizeEvent evt(GetSize(),GetId());
        evt.SetEventObject(this);
        GetEventHandler()->ProcessEvent(evt);
        Refresh();
    }
}


void wxComboCtrlBase::OnTextCtrlEvent(wxCommandEvent& event)
{
        if ( event.GetEventObject() == this )
    {
        event.Skip();
        return;
    }

    if ( event.GetEventType() == wxEVT_TEXT )
    {
        if ( m_ignoreEvtText > 0 )
        {
            m_ignoreEvtText--;
            return;
        }
    }

        wxCommandEvent evt2(event);
    evt2.SetId(GetId());
    evt2.SetEventObject(this);
    HandleWindowEvent(evt2);

    event.StopPropagation();
}

bool wxComboCtrlBase::HandleButtonMouseEvent( wxMouseEvent& event,
                                              int flags )
{
    int type = event.GetEventType();

    if ( type == wxEVT_MOTION )
    {
        if ( (flags & wxCC_MF_ON_BUTTON) &&
             IsPopupWindowState(Hidden) )
        {
            if ( !(m_btnState & wxCONTROL_CURRENT) )
            {
                                m_btnState |= wxCONTROL_CURRENT;
                if ( HasCapture() )                     m_btnState |= wxCONTROL_PRESSED;
                Refresh();
            }
        }
        else if ( (m_btnState & wxCONTROL_CURRENT) )
        {
                        m_btnState &= ~(wxCONTROL_CURRENT|wxCONTROL_PRESSED);
            Refresh();
        }
    }
    else if ( type == wxEVT_LEFT_DOWN || type == wxEVT_LEFT_DCLICK )
    {
        if ( flags & (wxCC_MF_ON_CLICK_AREA|wxCC_MF_ON_BUTTON) )
        {
            m_btnState |= wxCONTROL_PRESSED;
            Refresh();

            if ( !(m_iFlags & wxCC_POPUP_ON_MOUSE_UP) )
                OnButtonClick();
            else
                                CaptureMouse();
        }
    }
    else if ( type == wxEVT_LEFT_UP )
    {

                if ( HasCapture() )
            ReleaseMouse();

        if ( m_btnState & wxCONTROL_PRESSED )
        {
                        if ( m_iFlags & wxCC_POPUP_ON_MOUSE_UP )
            {
                if ( flags & (wxCC_MF_ON_CLICK_AREA|wxCC_MF_ON_BUTTON) )
                    OnButtonClick();
            }

            m_btnState &= ~(wxCONTROL_PRESSED);
            Refresh();
        }
    }
    else if ( type == wxEVT_LEAVE_WINDOW )
    {
        if ( m_btnState & (wxCONTROL_CURRENT|wxCONTROL_PRESSED) )
        {
            m_btnState &= ~(wxCONTROL_CURRENT);

                        if ( IsPopupWindowState(Hidden) )
            {
                m_btnState &= ~(wxCONTROL_PRESSED);
                Refresh();
            }
        }
    }
    else
        return false;

            if ( !IsPopupWindowState(Hidden) )
        m_btnState &= ~wxCONTROL_CURRENT;

    return true;
}

bool wxComboCtrlBase::PreprocessMouseEvent( wxMouseEvent& event,
                                            int WXUNUSED(flags) )
{
    wxMilliClock_t t = ::wxGetLocalTimeMillis();
    int evtType = event.GetEventType();

#if USES_WXPOPUPWINDOW || USES_GENERICTLW
    if ( m_popupWinType != POPUPWIN_WXPOPUPTRANSIENTWINDOW )
    {
        if ( IsPopupWindowState(Visible) &&
             ( evtType == wxEVT_LEFT_DOWN || evtType == wxEVT_RIGHT_DOWN ) )
        {
            HidePopup(true);
            return true;
        }
    }
#endif

        if ( evtType == wxEVT_LEFT_DOWN && t < m_timeCanAcceptClick )
    {
        event.SetEventType(0);
        return true;
    }

    return false;
}

void wxComboCtrlBase::HandleNormalMouseEvent( wxMouseEvent& event )
{
    int evtType = event.GetEventType();

    if ( (evtType == wxEVT_LEFT_DOWN || evtType == wxEVT_LEFT_DCLICK) &&
         (m_windowStyle & wxCB_READONLY) )
    {
        if ( GetPopupWindowState() >= Animating )
        {
    #if USES_WXPOPUPWINDOW
                        if ( m_popupWinType == POPUPWIN_WXPOPUPWINDOW )
                HidePopup(true);
    #endif
        }
        else
        {
            if ( !(m_windowStyle & wxCC_SPECIAL_DCLICK) )
            {
                                                OnButtonClick();
            }
            else if ( evtType == wxEVT_LEFT_DCLICK )
            {
                                                if ( m_popupInterface )
                    m_popupInterface->OnComboDoubleClick();
            }
        }
    }
    else if ( evtType == wxEVT_MOUSEWHEEL )
    {
        if ( IsPopupShown() )
        {
                        m_popup->GetEventHandler()->ProcessEvent(event);
        }
        else if ( event.GetWheelAxis() == 0 &&
                  event.GetWheelRotation() != 0 &&
                  event.GetModifiers() == 0 )
        {
                                                wxKeyEvent kevent(wxEVT_KEY_DOWN);
            kevent.m_keyCode = event.GetWheelRotation() > 0
                               ? WXK_UP
                               : WXK_DOWN;
            GetEventHandler()->ProcessEvent(kevent);
        }
        else
        {
            event.Skip();
        }
    }
    else if ( evtType )
    {
        event.Skip();
    }
}

void wxComboCtrlBase::OnKeyEvent(wxKeyEvent& event)
{
    if ( IsPopupShown() )
    {
                GetPopupControl()->GetControl()->GetEventHandler()->ProcessEvent(event);
    }
    else     {
        wxWindow* mainCtrl = GetMainWindowOfCompositeControl();

        if ( mainCtrl->GetParent()->HasFlag(wxTAB_TRAVERSAL) )
        {
            if ( mainCtrl->HandleAsNavigationKey(event) )
                return;
        }

        if ( IsKeyPopupToggle(event) )
        {
            OnButtonClick();
            return;
        }

        int comboStyle = GetWindowStyle();
        wxComboPopup* popupInterface = GetPopupControl();

        if ( !popupInterface )
        {
            event.Skip();
            return;
        }

        int keycode = event.GetKeyCode();

        if ( (comboStyle & wxCB_READONLY) ||
             (keycode != WXK_RIGHT && keycode != WXK_LEFT) )
        {
            popupInterface->OnComboKeyEvent(event);
        }
        else
            event.Skip();
    }
}

void wxComboCtrlBase::OnCharEvent(wxKeyEvent& event)
{
    if ( IsPopupShown() )
    {
                GetPopupControl()->GetControl()->GetEventHandler()->ProcessEvent(event);
    }
    else     {
        wxComboPopup* popupInterface = GetPopupControl();
        if ( popupInterface )
        {
            popupInterface->OnComboCharEvent(event);
        }
        else
        {
            event.Skip();
        }
    }
}

void wxComboCtrlBase::OnFocusEvent( wxFocusEvent& event )
{
            
    if ( event.GetEventType() == wxEVT_SET_FOCUS )
    {
        if ( !m_resetFocus && GetTextCtrl() && !GetTextCtrl()->HasFocus() )
        {
            m_resetFocus = true;
            GetTextCtrl()->SetFocus();
            m_resetFocus = false;
        }
    }
    
    Refresh();
}

void wxComboCtrlBase::OnIdleEvent( wxIdleEvent& WXUNUSED(event) )
{
    if ( m_resetFocus )
    {
        m_resetFocus = false;
        if ( GetTextCtrl() )
            GetTextCtrl()->SetFocus();
    }
}

void wxComboCtrlBase::OnSysColourChanged(wxSysColourChangedEvent& WXUNUSED(event))
{
    OnThemeChange();
        if ( !(m_iFlags & wxCC_IFLAG_LEFT_MARGIN_SET) )
        m_marginLeft = GetNativeTextIndent();
    RecalcAndRefresh();
}


void wxComboCtrlBase::CreatePopup()
{
    wxComboPopup* popupInterface = m_popupInterface;
    wxWindow* popup;

    if ( !m_winPopup )
    {
#ifdef wxComboPopupWindowBase2
        if ( m_iFlags & wxCC_IFLAG_USE_ALT_POPUP )
        {
        #if !USES_GENERICTLW
            m_winPopup = new wxComboPopupWindowBase2( this, wxNO_BORDER );
        #else
            int tlwFlags = wxNO_BORDER;
          #ifdef wxCC_GENERIC_TLW_IS_FRAME
            tlwFlags |= wxFRAME_NO_TASKBAR;
          #endif

          #ifdef wxCC_GENERIC_TLW_IS_NONOWNEDWINDOW
            m_winPopup = new wxComboPopupWindowBase2( this, wxID_ANY,
                                                      wxPoint(-21,-21), wxSize(20, 20),
                                                      tlwFlags );
          #else
            m_winPopup = new wxComboPopupWindowBase2( this, wxID_ANY, wxEmptyString,
                                                      wxPoint(-21,-21), wxSize(20, 20),
                                                      tlwFlags );
          #endif
        #endif
            m_popupWinType = SECONDARY_POPUP_TYPE;
        }
        else
#endif         {
            m_winPopup = new wxComboPopupWindow( this, wxNO_BORDER );
            m_popupWinType = PRIMARY_POPUP_TYPE;
        }
        m_popupWinEvtHandler = new wxComboPopupWindowEvtHandler(this);
        m_winPopup->PushEventHandler(m_popupWinEvtHandler);
    }

    popupInterface->Create(m_winPopup);
    m_popup = popup = popupInterface->GetControl();

    m_popupEvtHandler = new wxComboPopupEvtHandler(this);
    popup->PushEventHandler( m_popupEvtHandler );

                m_winPopup->Hide();

    popupInterface->m_iFlags |= wxCP_IFLAG_CREATED;
}

void wxComboCtrlBase::DestroyPopup()
{
    HidePopup(true);

    if ( m_popup )
        m_popup->RemoveEventHandler(m_popupEvtHandler);

    wxDELETE(m_popupEvtHandler);

    if ( m_popupInterface )
    {
                m_popupInterface->DestroyPopup();
        m_popupInterface = NULL;
    }

    if ( m_winPopup )
    {
        m_winPopup->RemoveEventHandler(m_popupWinEvtHandler);
        wxDELETE(m_popupWinEvtHandler);
        m_winPopup->Destroy();
        m_winPopup = NULL;
    }

    m_popup = NULL;
}

void wxComboCtrlBase::DoSetPopupControl(wxComboPopup* iface)
{
    wxCHECK_RET( iface, wxT("no popup interface set for wxComboCtrl") );

    DestroyPopup();

    iface->InitBase(this);
    iface->Init();

    m_popupInterface = iface;

    if ( !iface->LazyCreate() )
    {
        CreatePopup();
    }
    else
    {
        m_popup = NULL;
    }

        if ( !m_valueString.empty() )
    {
        iface->SetStringValue(m_valueString);
            }
}

void wxComboCtrlBase::EnsurePopupControl()
{
    if ( !m_popupInterface )
        SetPopupControl(NULL);
}

void wxComboCtrlBase::OnButtonClick()
{
            switch ( GetPopupWindowState() )
    {
        case Hidden:
        {
            Popup();
            break;
        }

        case Animating:
        case Visible:
        {
            HidePopup(true);
            break;
        }
    }
}

void wxComboCtrlBase::Popup()
{
    wxCommandEvent event(wxEVT_COMBOBOX_DROPDOWN, GetId());
    event.SetEventObject(this);
    HandleWindowEvent(event);

    ShowPopup();
}

void wxComboCtrlBase::ShowPopup()
{
    EnsurePopupControl();
    wxCHECK_RET( !IsPopupWindowState(Visible), wxT("popup window already shown") );

    if ( IsPopupWindowState(Animating) )
        return;

    SetFocus();

        int screenHeight;
    wxPoint scrPos;
    int spaceAbove;
    int spaceBelow;
    int maxHeightPopup;
    wxSize ctrlSz = GetSize();

    screenHeight = wxSystemSettings::GetMetric( wxSYS_SCREEN_Y );
    scrPos = GetParent()->ClientToScreen(GetPosition());

    spaceAbove = scrPos.y;
    spaceBelow = screenHeight - spaceAbove - ctrlSz.y;

    maxHeightPopup = spaceBelow;
    if ( spaceAbove > spaceBelow )
        maxHeightPopup = spaceAbove;

        int widthPopup = ctrlSz.x + m_extLeft + m_extRight;

    if ( widthPopup < m_widthMinPopup )
        widthPopup = m_widthMinPopup;

    wxWindow* winPopup = m_winPopup;
    wxWindow* popup;

                                wxWindow* mainCtrl = GetMainWindowOfCompositeControl();
    wxWindow* parent = mainCtrl->GetParent();
    int parentFlags = parent->GetWindowStyle();
    if ( parentFlags & wxTAB_TRAVERSAL )
    {
        parent->SetWindowStyle( parentFlags & ~(wxTAB_TRAVERSAL) );
        m_iFlags |= wxCC_IFLAG_PARENT_TAB_TRAVERSAL;
    }

    if ( !winPopup )
    {
        CreatePopup();
        winPopup = m_winPopup;
        popup = m_popup;
    }
    else
    {
        popup = m_popup;
    }

    winPopup->Enable();

    wxASSERT( !m_popup || m_popup == popup ); 
    wxSize adjustedSize = m_popupInterface->GetAdjustedSize(widthPopup,
                                                            m_heightPopup<=0?DEFAULT_POPUP_HEIGHT:m_heightPopup,
                                                            maxHeightPopup);

    popup->SetSize(adjustedSize);
    popup->Move(0,0);
    m_popupInterface->OnPopup();

            
    wxSize szp = popup->GetSize();

    int popupX;
    int popupY = scrPos.y + ctrlSz.y;

        int anchorSide = m_anchorSide;
    if ( !anchorSide )
        anchorSide = wxLEFT;

    int rightX = scrPos.x + ctrlSz.x + m_extRight - szp.x;
    int leftX = scrPos.x - m_extLeft;

    if ( wxTheApp->GetLayoutDirection() == wxLayout_RightToLeft )
        leftX -= ctrlSz.x;

    int screenWidth = wxSystemSettings::GetMetric( wxSYS_SCREEN_X );

            if ( anchorSide == wxRIGHT )
    {
        if ( rightX < 0 )
        {
            if ( (leftX+szp.x) < screenWidth )
                anchorSide = wxLEFT;
            else
                anchorSide = 0;
        }
    }
    else
    {
        if ( (leftX+szp.x) >= screenWidth )
        {
            if ( rightX >= 0 )
                anchorSide = wxRIGHT;
            else
                anchorSide = 0;
        }
    }

        if ( anchorSide == wxRIGHT )
        popupX = rightX;
    else if ( anchorSide == wxLEFT )
        popupX = leftX;
    else
        popupX = 0;

    int showFlags = CanDeferShow;

    if ( spaceBelow < szp.y )
    {
        popupY = scrPos.y - szp.y;
        showFlags |= ShowAbove;
    }

#if INSTALL_TOPLEV_HANDLER
        if ( m_popupWinType == POPUPWIN_WXPOPUPWINDOW )
    {
        if ( !m_toplevEvtHandler )
            m_toplevEvtHandler = new wxComboFrameEventHandler(this);

        wxWindow* toplev = ::wxGetTopLevelParent( this );
        wxASSERT( toplev );
        ((wxComboFrameEventHandler*)m_toplevEvtHandler)->OnPopup();
        toplev->PushEventHandler( m_toplevEvtHandler );
    }
#endif

        if ( m_text )
    {
        if ( !(m_iFlags & wxCC_NO_TEXT_AUTO_SELECT) )
            m_text->SelectAll();

        m_popupInterface->SetStringValue( m_text->GetValue() );
    }
    else
    {
                Refresh();
    }

        m_popupWinState = Animating;

    wxRect popupWinRect( popupX, popupY, szp.x, szp.y );

    m_popup = popup;
    if ( (m_iFlags & wxCC_IFLAG_DISABLE_POPUP_ANIM) ||
         AnimateShow( popupWinRect, showFlags ) )
    {
        DoShowPopup( popupWinRect, showFlags );
    }
}


#ifdef __WXMAC__
bool wxComboCtrlBase::AnimateShow( const wxRect& rect, int WXUNUSED(flags) )
{
                    if ( GetPopupWindow() )
    {
        GetPopupWindow()->SetSize(rect);
        GetPopupWindow()->Raise();
        GetPopupWindow()->Show();
    }
    return true;
}
#else
bool wxComboCtrlBase::AnimateShow( const wxRect& WXUNUSED(rect), int WXUNUSED(flags) )
{
    return true;
}
#endif

void wxComboCtrlBase::DoShowPopup( const wxRect& rect, int WXUNUSED(flags) )
{
    wxWindow* winPopup = m_winPopup;

    if ( IsPopupWindowState(Animating) )
    {
                
                        winPopup->SetSize( rect );

#if USES_WXPOPUPTRANSIENTWINDOW
        if ( m_popupWinType == POPUPWIN_WXPOPUPTRANSIENTWINDOW )
            ((wxPopupTransientWindow*)winPopup)->Popup(m_popup);
        else
#endif
            winPopup->Show();

        m_popupWinState = Visible;

                                                        #if !defined(__WXX11__)
        if ( IsPopupWinTypePerfect(m_popupWinType) )
            m_popup->SetFocus();
#endif
    }
    else if ( IsPopupWindowState(Hidden) )
    {
        
        wxASSERT( !winPopup->IsShown() );

        m_popupWinState = Hidden;
    }

    Refresh();
}

void wxComboCtrlBase::OnPopupDismiss(bool generateEvent)
{
        if ( IsPopupWindowState(Hidden) )
        return;

            m_popupWinState = Hidden;

        m_winPopup->Disable();

        m_popupInterface->OnDismiss();

    if ( m_popupEvtHandler )
        ((wxComboPopupEvtHandler*)m_popupEvtHandler)->OnPopupDismiss();

#if INSTALL_TOPLEV_HANDLER
        if ( m_toplevEvtHandler )
    {
        wxWindow* toplev = ::wxGetTopLevelParent( this );
        if ( toplev )
            toplev->RemoveEventHandler( m_toplevEvtHandler );
    }
#endif

    m_timeCanAcceptClick = ::wxGetLocalTimeMillis();

    if ( m_popupWinType == POPUPWIN_WXPOPUPTRANSIENTWINDOW )
        m_timeCanAcceptClick += 150;

            if ( !m_btnArea.Contains(ScreenToClient(::wxGetMousePosition())) )
        m_btnState = 0;

            if ( m_iFlags & wxCC_IFLAG_PARENT_TAB_TRAVERSAL )
    {
        wxWindow* parent = GetParent();
        parent->SetWindowStyle( parent->GetWindowStyle() | wxTAB_TRAVERSAL );
        m_iFlags &= ~(wxCC_IFLAG_PARENT_TAB_TRAVERSAL);
    }

        Refresh();

#if !defined(__WXX11__)
    SetFocus();
#endif

    if ( generateEvent )
    {
        wxCommandEvent event(wxEVT_COMBOBOX_CLOSEUP, GetId());
        event.SetEventObject(this);
        HandleWindowEvent(event);
    }
}

void wxComboCtrlBase::HidePopup(bool generateEvent)
{
        if ( IsPopupWindowState(Hidden) )
        return;

        if ( !IsPopupWindowState(Animating) )
        SetValueByUser( m_popupInterface->GetStringValue() );

    m_winPopup->Hide();

    OnPopupDismiss(generateEvent);
}


void wxComboCtrlBase::SetButtonPosition( int width, int height,
                                         int side, int spacingX )
{
    m_btnWid = width;
    m_btnHei = height;
    m_btnSide = side;
    m_btnSpacingX = spacingX;

    if ( width > 0 || height > 0 || spacingX )
        m_iFlags |= wxCC_IFLAG_HAS_NONSTANDARD_BUTTON;

    RecalcAndRefresh();
}

wxSize wxComboCtrlBase::GetButtonSize()
{
    if ( m_btnSize.x > 0 )
        return m_btnSize;

    wxSize retSize(m_btnWid,m_btnHei);

            if ( retSize.x <= 0 || retSize.y <= 0)
    {
        OnResize();

        retSize = m_btnSize;
    }

    return retSize;
}

void wxComboCtrlBase::SetButtonBitmaps( const wxBitmap& bmpNormal,
                                           bool blankButtonBg,
                                           const wxBitmap& bmpPressed,
                                           const wxBitmap& bmpHover,
                                           const wxBitmap& bmpDisabled )
{
    m_bmpNormal = bmpNormal;
    m_blankButtonBg = blankButtonBg;

    if ( bmpPressed.IsOk() )
        m_bmpPressed = bmpPressed;
    else
        m_bmpPressed = bmpNormal;

    if ( bmpHover.IsOk() )
        m_bmpHover = bmpHover;
    else
        m_bmpHover = bmpNormal;

    if ( bmpDisabled.IsOk() )
        m_bmpDisabled = bmpDisabled;
    else
        m_bmpDisabled = bmpNormal;

    RecalcAndRefresh();
}

void wxComboCtrlBase::SetCustomPaintWidth( int width )
{
    if ( m_text )
    {
                wxRect r = m_text->GetRect();
        int inc = width - m_widthCustomPaint;
        r.x += inc;
        r.width -= inc;
        m_text->SetSize( r );
    }

    m_widthCustomPaint = width;

    RecalcAndRefresh();
}

bool wxComboCtrlBase::DoSetMargins(const wxPoint& margins)
{
            bool res = true;

    if ( margins.x != -1 )
    {
        m_marginLeft = margins.x;
        m_iFlags |= wxCC_IFLAG_LEFT_MARGIN_SET;
    }
    else
    {
        m_marginLeft = GetNativeTextIndent();
        m_iFlags &= ~(wxCC_IFLAG_LEFT_MARGIN_SET);
    }

    if ( margins.y != -1 )
    {
        res = false;
    }

    RecalcAndRefresh();

    return res;
}

wxPoint wxComboCtrlBase::DoGetMargins() const
{
    return wxPoint(m_marginLeft, -1);
}

#if WXWIN_COMPATIBILITY_2_8
void wxComboCtrlBase::SetTextIndent( int indent )
{
    if ( indent < 0 )
    {
        m_marginLeft = GetNativeTextIndent();
        m_iFlags &= ~(wxCC_IFLAG_LEFT_MARGIN_SET);
    }
    else
    {
        m_marginLeft = indent;
        m_iFlags |= wxCC_IFLAG_LEFT_MARGIN_SET;
    }

    RecalcAndRefresh();
}

wxCoord wxComboCtrlBase::GetTextIndent() const
{
    return m_marginLeft;
}
#endif

wxCoord wxComboCtrlBase::GetNativeTextIndent() const
{
    return DEFAULT_TEXT_INDENT;
}

void wxComboCtrlBase::SetTextCtrlStyle( int style )
{
    m_textCtrlStyle = style;

    if ( m_text )
        m_text->SetWindowStyle(style);
}


wxString wxComboCtrlBase::DoGetValue() const
{
    if ( m_text )
        return m_text->GetValue();
    return m_valueString;
}

void wxComboCtrlBase::SetValueWithEvent(const wxString& value,
                                        bool withEvent)
{
    DoSetValue(value, withEvent ? SetValue_SendEvent : 0);
}

void wxComboCtrlBase::OnSetValue(const wxString& value)
{
        
            if ( m_valueString != value )
    {
        bool found = true;
        wxString trueValue = value;

                        if ( m_popupInterface && HasFlag(wxCB_READONLY) && value.length() )
        {
            found = m_popupInterface->FindItem(value,
                                               &trueValue);
        }

        if ( found )
        {
            m_valueString = trueValue;

            EnsurePopupControl();

            if ( m_popupInterface )
                m_popupInterface->SetStringValue(trueValue);
        }
    }

    Refresh();
}

void wxComboCtrlBase::SetValueByUser(const wxString& value)
{
        
    if ( m_text )
    {
        m_text->SetValue(value);

        if ( !(m_iFlags & wxCC_NO_TEXT_AUTO_SELECT) )
            m_text->SelectAll();
    }

    OnSetValue(value);
}

void wxComboCtrlBase::SetText(const wxString& value)
{
            EnsurePopupControl();

    m_valueString = value;

    if ( m_text )
    {
        m_ignoreEvtText++;
        m_text->SetValue( value );
    }

    Refresh();
}

void wxComboCtrlBase::Copy()
{
    if ( m_text )
        m_text->Copy();
}

void wxComboCtrlBase::Cut()
{
    if ( m_text )
        m_text->Cut();
}

void wxComboCtrlBase::Paste()
{
    if ( m_text )
        m_text->Paste();
}

void wxComboCtrlBase::SetInsertionPoint(long pos)
{
    if ( m_text )
        m_text->SetInsertionPoint(pos);
}

long wxComboCtrlBase::GetInsertionPoint() const
{
    if ( m_text )
        return m_text->GetInsertionPoint();

    return 0;
}

long wxComboCtrlBase::GetLastPosition() const
{
    if ( m_text )
        return m_text->GetLastPosition();

    return 0;
}

void wxComboCtrlBase::WriteText(const wxString& text)
{
    if ( m_text )
    {
        m_text->WriteText(text);
        OnSetValue(m_text->GetValue());
    }
    else
    {
        OnSetValue(text);
    }
}

void wxComboCtrlBase::DoSetValue(const wxString& value, int flags)
{
    if ( m_text )
    {
        if ( flags & SetValue_SendEvent )
            m_text->SetValue(value);
        else
            m_text->ChangeValue(value);
    }

    OnSetValue(value);
}

void wxComboCtrlBase::Replace(long from, long to, const wxString& value)
{
    if ( m_text )
    {
        m_text->Replace(from, to, value);
        OnSetValue(m_text->GetValue());
    }
}

void wxComboCtrlBase::Remove(long from, long to)
{
    if ( m_text )
    {
        m_text->Remove(from, to);
        OnSetValue(m_text->GetValue());
    }
}

void wxComboCtrlBase::SetSelection(long from, long to)
{
    if ( m_text )
        m_text->SetSelection(from, to);
}

void wxComboCtrlBase::GetSelection(long *from, long *to) const
{
    if ( m_text )
    {
        m_text->GetSelection(from, to);
    }
    else
    {
        *from = 0;
        *to = 0;
    }
}

bool wxComboCtrlBase::IsEditable() const
{
    if ( m_text )
        return m_text->IsEditable();
    return false;
}

void wxComboCtrlBase::SetEditable(bool editable)
{
    if ( m_text )
        m_text->SetEditable(editable);
}

void wxComboCtrlBase::Undo()
{
    if ( m_text )
        m_text->Undo();
}

void wxComboCtrlBase::Redo()
{
    if ( m_text )
        m_text->Redo();
}

bool wxComboCtrlBase::CanUndo() const
{
    if ( m_text )
        return m_text->CanUndo();

    return false;
}

bool wxComboCtrlBase::CanRedo() const
{
    if ( m_text )
        return m_text->CanRedo();

    return false;
}

bool wxComboCtrlBase::SetHint(const wxString& hint)
{
    m_hintText = hint;
    bool res = true;
    if ( m_text )
        res = m_text->SetHint(hint);
    Refresh();
    return res;
}

wxString wxComboCtrlBase::GetHint() const
{
    return m_hintText;
}

#endif 