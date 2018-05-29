
#include "wx/wxprec.h"

#include "wx/window.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/utils.h"
    #include "wx/panel.h"
    #include "wx/frame.h"
    #include "wx/dc.h"
    #include "wx/dcclient.h"
    #include "wx/button.h"
    #include "wx/menu.h"
    #include "wx/dialog.h"
    #include "wx/settings.h"
    #include "wx/msgdlg.h"
    #include "wx/scrolbar.h"
    #include "wx/statbox.h"
    #include "wx/textctrl.h"
    #include "wx/toolbar.h"
    #include "wx/layout.h"
    #include "wx/statusbr.h"
    #include "wx/menuitem.h"
    #include "wx/treectrl.h"
    #include "wx/listctrl.h"
    #include "wx/platinfo.h"
#endif

#include "wx/tooltip.h"
#include "wx/spinctrl.h"
#include "wx/geometry.h"
#include "wx/weakref.h"

#if wxUSE_LISTCTRL
    #include "wx/listctrl.h"
#endif

#if wxUSE_TREECTRL
    #include "wx/treectrl.h"
#endif

#if wxUSE_CARET
    #include "wx/caret.h"
#endif

#if wxUSE_POPUPWIN
    #include "wx/popupwin.h"
#endif

#if wxUSE_DRAG_AND_DROP
    #include "wx/dnd.h"
#endif

#include "wx/graphics.h"

#include "wx/osx/private.h"

#define MAC_SCROLLBAR_SIZE 15
#define MAC_SMALL_SCROLLBAR_SIZE 11

#include <string.h>

#ifdef __WXUNIVERSAL__
    wxIMPLEMENT_ABSTRACT_CLASS(wxWindowMac, wxWindowBase);
#endif

wxBEGIN_EVENT_TABLE(wxWindowMac, wxWindowBase)
    EVT_MOUSE_EVENTS(wxWindowMac::OnMouseEvent)
wxEND_EVENT_TABLE()

#define wxMAC_DEBUG_REDRAW 0
#ifndef wxMAC_DEBUG_REDRAW
#define wxMAC_DEBUG_REDRAW 0
#endif

wxWidgetImplType* kOSXNoWidgetImpl = (wxWidgetImplType*) -1L;

#if wxUSE_HOTKEY && wxOSX_USE_COCOA_OR_CARBON

typedef struct  {
    EventHotKeyRef ref;
    int keyId;
    wxWindow* window;
} wxHotKeyRec;

wxVector<wxHotKeyRec> s_hotkeys;

#endif



class WXDLLIMPEXP_CORE wxBlindPlateWindow : public wxWindow
{
public:
    wxBlindPlateWindow() { Init(); }

            wxBlindPlateWindow(wxWindow *parent,
            int x, int y, int width, int height,
            long style = wxTAB_TRAVERSAL | wxNO_BORDER,
            const wxString& name = wxPanelNameStr)
    {
        Init();

        Create(parent, wxID_ANY, wxPoint(x, y), wxSize(width, height), style, name);
    }

        wxBlindPlateWindow(wxWindow *parent,
            wxWindowID winid = wxID_ANY,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxTAB_TRAVERSAL | wxNO_BORDER,
            const wxString& name = wxPanelNameStr)
    {
        Init();

        Create(parent, winid, pos, size, style, name);
    }

        bool Create(wxWindow *parent,
                wxWindowID winid = wxID_ANY,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                const wxString& name = wxPanelNameStr)
    {
        if ( !wxWindow::Create(parent, winid, pos, size, style, name) )
            return false;

                SetThemeEnabled(true);
        return true;
    }

    virtual ~wxBlindPlateWindow();

    virtual bool AcceptsFocus() const wxOVERRIDE
    {
        return false;
    }

protected:
        void Init()
    {
    }

    wxDECLARE_DYNAMIC_CLASS_NO_COPY(wxBlindPlateWindow);
    wxDECLARE_EVENT_TABLE();
};

wxBlindPlateWindow::~wxBlindPlateWindow()
{
}

wxIMPLEMENT_DYNAMIC_CLASS(wxBlindPlateWindow, wxWindow);

wxBEGIN_EVENT_TABLE(wxBlindPlateWindow, wxWindow)
wxEND_EVENT_TABLE()


 
wxWindowMac::wxWindowMac()
{
    Init();
}

wxWindowMac::wxWindowMac(wxWindowMac *parent,
            wxWindowID id,
            const wxPoint& pos ,
            const wxSize& size ,
            long style ,
            const wxString& name )
{
    Init();
    Create(parent, id, pos, size, style, name);
}

void wxWindowMac::Init()
{
    m_peer = NULL ;
    m_macAlpha = 255 ;
    m_cgContextRef = NULL ;

        m_isShown = true;

    m_hScrollBar = NULL ;
    m_vScrollBar = NULL ;
    m_hScrollBarAlwaysShown = false;
    m_vScrollBarAlwaysShown = false;
    m_growBox = NULL ;

    m_clipChildren = false ;
    m_cachedClippedRectValid = false ;
    m_isNativeWindowWrapper = false;
}

wxWindowMac::~wxWindowMac()
{
    SendDestroyEvent();
    
#if wxUSE_HOTKEY && wxOSX_USE_COCOA_OR_CARBON
    for ( int i = s_hotkeys.size()-1; i>=0; -- i )
    {
        if ( s_hotkeys[i].window == this )
        {
            EventHotKeyRef ref = s_hotkeys[i].ref;
            s_hotkeys.erase(s_hotkeys.begin() + i);
            if ( UnregisterEventHotKey(ref) != noErr )
            {
                wxLogLastError(wxT("UnregisterHotKey"));
            }
        }
    }    
#endif

    MacInvalidateBorders() ;

#ifndef __WXUNIVERSAL__
        for ( wxWindow *win = GetParent(); win; win = win->GetParent() )
    {
        wxFrame *frame = wxDynamicCast(win, wxFrame);
        if ( frame )
        {
            if ( frame->GetLastFocus() == this )
                frame->SetLastFocus(NULL);
            break;
        }
    }
#endif

        DestroyChildren();

                wxTopLevelWindow *tlw = wxDynamicCast(wxGetTopLevelParent((wxWindow*)this), wxTopLevelWindow);
    if ( tlw )
    {
        if ( tlw->GetDefaultItem() == (wxButton*) this)
            tlw->SetDefaultItem(NULL);
    }

    if ( g_MacLastWindow == this )
        g_MacLastWindow = NULL ;

#ifndef __WXUNIVERSAL__
    wxFrame* frame = wxDynamicCast( wxGetTopLevelParent( (wxWindow*)this ) , wxFrame ) ;
    if ( frame )
    {
        if ( frame->GetLastFocus() == this )
            frame->SetLastFocus( NULL ) ;
    }
#endif

    #if wxUSE_DRAG_AND_DROP
    wxDELETE(m_dropTarget);
#endif

    delete GetPeer() ;
}

WXWidget wxWindowMac::GetHandle() const
{
    if ( GetPeer() )
        return (WXWidget) GetPeer()->GetWXWidget() ;
    return NULL;
}

wxOSXWidgetImpl* wxWindowMac::GetPeer() const 
{ 
    return m_peer == kOSXNoWidgetImpl ? NULL : m_peer ; 
}

bool wxWindowMac::ShouldCreatePeer() const
{
    return m_peer != kOSXNoWidgetImpl;
}

void wxWindowMac::DontCreatePeer()
{
    m_peer = kOSXNoWidgetImpl;
}

void wxWindowMac::SetWrappingPeer(wxOSXWidgetImpl* wrapper)
{ 
    wxOSXWidgetImpl* inner = GetPeer();
    wxASSERT_MSG( inner != NULL && inner->IsOk(), "missing or incomplete inner peer" );
    wxASSERT_MSG( wrapper != NULL && wrapper->IsOk(), "missing or incomplete wrapper" );
    
    if ( !(inner != NULL && inner->IsOk() && wrapper != NULL && wrapper->IsOk()) )
        return;
    
    inner->RemoveFromParent();
    wrapper->InstallEventHandler();
    wrapper->Embed(inner);
    m_peer = wrapper;
}

void wxWindowMac::SetPeer(wxOSXWidgetImpl* peer)
{
    if ( GetPeer() )
    {
        if ( !GetPeer()->IsRootControl() )
            GetPeer()->RemoveFromParent();
        wxDELETE(m_peer);
    }

    m_peer = peer;

    if ( GetPeer() && !GetPeer()->IsRootControl())
    {
        wxASSERT_MSG( GetPeer()->IsOk() , wxT("The native control must exist already") ) ;
        
        if (!GetParent()->GetChildren().Find((wxWindow*)this))
            GetParent()->AddChild( this );
        
        GetPeer()->InstallEventHandler();
        GetPeer()->Embed(GetParent()->GetPeer());
        
        GetParent()->MacChildAdded() ;
        
                GetPeer()->SetControlSize( m_windowVariant );
        InheritAttributes();
                if ( !m_hasFont )
            DoSetWindowVariant( m_windowVariant );
        
        if ( !m_label.empty() )
            GetPeer()->SetLabel( wxStripMenuCodes(m_label, wxStrip_Mnemonics), GetFont().GetEncoding() ) ;
        
                if ( !GetPeer()->IsUserPane() )
            SetInitialSize(GetMinSize());
        
        SetCursor( *wxSTANDARD_CURSOR ) ;
    }
}

#if WXWIN_COMPATIBILITY_2_8

bool wxWindowMac::MacIsUserPane() 
{ 
    return GetPeer() == NULL || GetPeer()->IsUserPane(); 
}

#endif

bool wxWindowMac::MacIsUserPane() const 
{ 
    return GetPeer() == NULL || GetPeer()->IsUserPane(); 
}





bool wxWindowMac::Create(wxWindowMac *parent,
    wxWindowID id,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name)
{
    wxCHECK_MSG( parent, false, wxT("can't create wxWindowMac without parent") );

    if ( !CreateBase(parent, id, pos, size, style, wxDefaultValidator, name) )
        return false;

    m_windowVariant = parent->GetWindowVariant() ;

    m_hScrollBarAlwaysShown =
    m_vScrollBarAlwaysShown = HasFlag(wxALWAYS_SHOW_SB);

    if ( m_peer != kOSXNoWidgetImpl )
    {
        SetPeer(wxWidgetImpl::CreateUserPane( this, parent, id, pos, size , style, GetExtraStyle() ));
        MacPostControlCreate(pos, size) ;
    }

#ifndef __WXUNIVERSAL__
        if ( (! IsKindOf(CLASSINFO(wxControl))
#if wxUSE_STATUSBAR
        && ! IsKindOf(CLASSINFO(wxStatusBar))
#endif
        )
         || (IsKindOf(CLASSINFO(wxControl)) && ((style & wxHSCROLL) || (style & wxVSCROLL))))
    {
        MacCreateScrollBars( style ) ;
    }
#endif

    wxWindowCreateEvent event((wxWindow*)this);
    GetEventHandler()->AddPendingEvent(event);

    return true;
}

void wxWindowMac::MacChildAdded()
{
#if wxUSE_SCROLLBAR
    if ( m_vScrollBar )
        m_vScrollBar->Raise() ;
    if ( m_hScrollBar )
        m_hScrollBar->Raise() ;
    if ( m_growBox )
        m_growBox->Raise() ;
#endif
}

void wxWindowMac::MacPostControlCreate(const wxPoint& pos,
                                       const wxSize& WXUNUSED(size))
{
            if ( pos.IsFullySpecified() && GetPosition() != pos )
    {
        SetPosition(pos);
    }
}

void wxWindowMac::DoSetWindowVariant( wxWindowVariant variant )
{
            
    m_windowVariant = variant ;

    if (GetPeer() == NULL || !GetPeer()->IsOk())
        return;

    GetPeer()->SetControlSize( variant );

    switch ( variant )
    {
        case wxWINDOW_VARIANT_NORMAL :
            static wxFont sysNormal(wxOSX_SYSTEM_FONT_NORMAL);
            SetFont(sysNormal) ;
            break ;

        case wxWINDOW_VARIANT_SMALL :
            static wxFont sysSmall(wxOSX_SYSTEM_FONT_SMALL);
            SetFont(sysSmall) ;
            break ;

        case wxWINDOW_VARIANT_MINI :
            static wxFont sysMini(wxOSX_SYSTEM_FONT_MINI);
            SetFont(sysMini) ;
            break ;

        case wxWINDOW_VARIANT_LARGE :
            static wxFont sysLarge(wxOSX_SYSTEM_FONT_NORMAL);
            SetFont(sysLarge) ;
            break ;

        default:
            wxFAIL_MSG(wxT("unexpected window variant"));
            break ;
    }
}

void wxWindowMac::MacUpdateControlFont()
{
    if ( GetPeer() )
        GetPeer()->SetFont( GetFont() , GetForegroundColour() , GetWindowStyle() ) ;

        if ( IsShownOnScreen() )
        Refresh() ;
}

bool wxWindowMac::SetFont(const wxFont& font)
{
    bool retval = wxWindowBase::SetFont( font );

    MacUpdateControlFont() ;

    return retval;
}

bool wxWindowMac::SetForegroundColour(const wxColour& col )
{
    bool retval = wxWindowBase::SetForegroundColour( col );

    if (retval)
        MacUpdateControlFont();

    return retval;
}

bool wxWindowMac::SetBackgroundStyle(wxBackgroundStyle style)
{
    if ( !wxWindowBase::SetBackgroundStyle(style) )
        return false;

    if ( GetPeer() )
        GetPeer()->SetBackgroundStyle(style);
    return true;
}

bool wxWindowMac::SetBackgroundColour(const wxColour& col )
{
    if (m_growBox)
    {
        if ( m_backgroundColour.IsOk() )
            m_growBox->SetBackgroundColour(m_backgroundColour);
        else
            m_growBox->SetBackgroundColour(*wxWHITE);
    }

    if ( !wxWindowBase::SetBackgroundColour(col) && m_hasBgCol )
        return false ;

    if ( GetPeer() )
        GetPeer()->SetBackgroundColour( col ) ;

    return true ;
}

static bool wxIsWindowOrParentDisabled(wxWindow* w)
{
    while (w && !w->IsTopLevel())
    {
        if (!w->IsEnabled())
            return true;
        w = w->GetParent();
    }
    return false;
}

void wxWindowMac::SetFocus()
{
    if ( !AcceptsFocus() )
            return ;

    if (wxIsWindowOrParentDisabled((wxWindow*) this))
        return;

    wxWindow* former = FindFocus() ;
    if ( former == this )
        return ;

    GetPeer()->SetFocus() ;
}

void wxWindowMac::OSXSimulateFocusEvents()
{
    wxWeakRef<wxWindow> former = FindFocus() ;
    if ( former != NULL && former != this )
    {
        {
            wxFocusEvent event( wxEVT_KILL_FOCUS, former->GetId());
            event.SetEventObject(former);
            event.SetWindow(this);
            former->HandleWindowEvent(event) ;
        }

                        if ( former )
        {
            wxFocusEvent event(wxEVT_SET_FOCUS, former->GetId());
            event.SetEventObject(former);
            event.SetWindow(this);
            former->HandleWindowEvent(event);
        }
    }
}

void wxWindowMac::DoCaptureMouse()
{
    wxApp::s_captureWindow = (wxWindow*) this ;
    GetPeer()->CaptureMouse() ;
}

wxWindow * wxWindowBase::GetCapture()
{
    return wxApp::s_captureWindow ;
}

void wxWindowMac::DoReleaseMouse()
{
    wxApp::s_captureWindow = NULL ;

    GetPeer()->ReleaseMouse() ;
}

#if wxUSE_DRAG_AND_DROP

void wxWindowMac::SetDropTarget(wxDropTarget *pDropTarget)
{
    delete m_dropTarget;

    m_dropTarget = pDropTarget;

    GetPeer()->SetDropTarget(m_dropTarget) ;
}

#endif

void wxWindowMac::DragAcceptFiles(bool WXUNUSED(accept))
{
    }


bool wxWindowMac::MacGetBoundsForControl(
    const wxPoint& pos,
    const wxSize& size,
    int& x, int& y,
    int& w, int& h , bool adjustOrigin ) const
{
        x = (int)pos.x;
    y = (int)pos.y;

    w = WidthDefault( size.x );
    h = HeightDefault( size.y );

    x += MacGetLeftBorderSize() ;
    y += MacGetTopBorderSize() ;
    w -= MacGetLeftBorderSize() + MacGetRightBorderSize() ;
    h -= MacGetTopBorderSize() + MacGetBottomBorderSize() ;

    if ( adjustOrigin )
        AdjustForParentClientOrigin( x , y ) ;

        if ( GetParent() && !GetParent()->IsTopLevel() )
    {
        x -= GetParent()->MacGetLeftBorderSize() ;
        y -= GetParent()->MacGetTopBorderSize() ;
    }

    return true ;
}

void wxWindowMac::DoGetSize(int *x, int *y) const
{
    int width, height;
    GetPeer()->GetSize( width, height );

    if (x)
       *x = width + MacGetLeftBorderSize() + MacGetRightBorderSize() ;
    if (y)
       *y = height + MacGetTopBorderSize() + MacGetBottomBorderSize() ;
}

void wxWindowMac::DoGetPosition(int *x, int *y) const
{
    int x1, y1;

    GetPeer()->GetPosition( x1, y1 ) ;

        x1 -= MacGetLeftBorderSize() ;
    y1 -= MacGetTopBorderSize() ;

    if ( !IsTopLevel() )
    {
        wxWindow *parent = GetParent();
        if ( parent )
        {
                                    x1 += parent->MacGetLeftBorderSize() ;
            y1 += parent->MacGetTopBorderSize() ;

                        wxPoint pt(parent->GetClientAreaOrigin());
            x1 -= pt.x ;
            y1 -= pt.y ;
        }
    }

    if (x)
       *x = x1 ;
    if (y)
       *y = y1 ;
}

void wxWindowMac::DoScreenToClient(int *x, int *y) const
{
    wxNonOwnedWindow* tlw = MacGetTopLevelWindow() ;
    wxCHECK_RET( tlw , wxT("TopLevel Window missing") ) ;
    tlw->GetNonOwnedPeer()->ScreenToWindow( x, y);
    MacRootWindowToWindow( x , y ) ;

    wxPoint origin = GetClientAreaOrigin() ;
    if (x)
       *x -= origin.x ;
    if (y)
       *y -= origin.y ;
}

void wxWindowMac::DoClientToScreen(int *x, int *y) const
{
    wxNonOwnedWindow* tlw = MacGetTopLevelWindow() ;
    wxCHECK_RET( tlw , wxT("TopLevel window missing") ) ;

    wxPoint origin = GetClientAreaOrigin() ;
    if (x)
       *x += origin.x ;
    if (y)
       *y += origin.y ;

    MacWindowToRootWindow( x , y ) ;
    tlw->GetNonOwnedPeer()->WindowToScreen( x , y );
}

void wxWindowMac::MacClientToRootWindow( int *x , int *y ) const
{
    wxPoint origin = GetClientAreaOrigin() ;
    if (x)
       *x += origin.x ;
    if (y)
       *y += origin.y ;

    MacWindowToRootWindow( x , y ) ;
}

void wxWindowMac::MacWindowToRootWindow( int *x , int *y ) const
{
    wxPoint pt ;

    if (x)
        pt.x = *x ;
    if (y)
        pt.y = *y ;

    if ( !IsTopLevel() )
    {
        wxNonOwnedWindow* top = MacGetTopLevelWindow();
        if (top)
        {
            pt.x -= MacGetLeftBorderSize() ;
            pt.y -= MacGetTopBorderSize() ;
            wxWidgetImpl::Convert( &pt , GetPeer() , top->GetPeer() ) ;
        }
    }

    if (x)
        *x = (int) pt.x ;
    if (y)
        *y = (int) pt.y ;
}

void wxWindowMac::MacRootWindowToWindow( int *x , int *y ) const
{
    wxPoint pt ;

    if (x)
        pt.x = *x ;
    if (y)
        pt.y = *y ;

    if ( !IsTopLevel() )
    {
        wxNonOwnedWindow* top = MacGetTopLevelWindow();
        if (top)
        {
            wxWidgetImpl::Convert( &pt , top->GetPeer() , GetPeer() ) ;
            pt.x += MacGetLeftBorderSize() ;
            pt.y += MacGetTopBorderSize() ;
        }
    }

    if (x)
        *x = (int) pt.x ;
    if (y)
        *y = (int) pt.y ;
}

wxSize wxWindowMac::DoGetSizeFromClientSize( const wxSize & size )  const
{
    wxSize sizeTotal = size;

    int innerwidth, innerheight;
    int left, top;
    int outerwidth, outerheight;

    GetPeer()->GetContentArea( left, top, innerwidth, innerheight );
    GetPeer()->GetSize( outerwidth, outerheight );

    sizeTotal.x += outerwidth-innerwidth;
    sizeTotal.y += outerheight-innerheight;

    sizeTotal.x += MacGetLeftBorderSize() + MacGetRightBorderSize() ;
    sizeTotal.y += MacGetTopBorderSize() + MacGetBottomBorderSize() ;

    return sizeTotal;
}

void wxWindowMac::DoGetClientSize( int *x, int *y ) const
{
    int ww, hh;

    int left, top;

    GetPeer()->GetContentArea( left, top, ww, hh );
#if wxUSE_SCROLLBAR
    if (m_hScrollBar  && m_hScrollBar->IsShown() )
        hh -= m_hScrollBar->GetSize().y ;

    if (m_vScrollBar  && m_vScrollBar->IsShown() )
        ww -= m_vScrollBar->GetSize().x ;

#endif
    if (x)
    {
                if ( ww < 0 )
            ww = 0;
        
        *x = ww;
    }
    
    if (y)
    {
                if ( hh < 0 )
            hh = 0;
        
        *y = hh;
    }
}

bool wxWindowMac::SetCursor(const wxCursor& cursor)
{
    if (m_cursor.IsSameAs(cursor))
        return false;

    if (!cursor.IsOk())
    {
        if ( ! wxWindowBase::SetCursor( *wxSTANDARD_CURSOR ) )
            return false ;
    }
    else
    {
        if ( ! wxWindowBase::SetCursor( cursor ) )
            return false ;
    }

    wxASSERT_MSG( m_cursor.IsOk(),
        wxT("cursor must be valid after call to the base version"));

    if ( GetPeer() != NULL )
        GetPeer()->SetCursor( m_cursor );

    return true ;
}

#if wxUSE_MENUS
bool wxWindowMac::DoPopupMenu(wxMenu *menu, int x, int y)
{
#ifndef __WXUNIVERSAL__
    if ( x == wxDefaultCoord && y == wxDefaultCoord )
    {
        wxPoint mouse = wxGetMousePosition();
        x = mouse.x;
        y = mouse.y;
    }
    else
    {
        ClientToScreen( &x , &y ) ;
    }
    menu->GetPeer()->PopUp(this, x, y);
    return true;
#else
        return false;
#endif
}
#endif


#if wxUSE_TOOLTIPS

void wxWindowMac::DoSetToolTip(wxToolTip *tooltip)
{
    wxWindowBase::DoSetToolTip(tooltip);

    if ( m_tooltip )
        m_tooltip->SetWindow(this);

    if (GetPeer())
        GetPeer()->SetToolTip(tooltip);
}

#endif

void wxWindowMac::MacInvalidateBorders()
{
    if ( GetPeer() == NULL )
        return ;

    bool vis = IsShownOnScreen() ;
    if ( !vis )
        return ;

    int outerBorder = MacGetLeftBorderSize() ;

    if ( GetPeer()->NeedsFocusRect() )
        outerBorder += 4 ;

    if ( outerBorder == 0 )
        return ;

    
    int tx,ty,tw,th;

    GetPeer()->GetSize( tw, th );
    GetPeer()->GetPosition( tx, ty );

    wxRect leftupdate( tx-outerBorder,ty,outerBorder,th );
    wxRect rightupdate( tx+tw, ty, outerBorder, th );
    wxRect topupdate( tx-outerBorder, ty-outerBorder, tw + 2 * outerBorder, outerBorder );
    wxRect bottomupdate( tx-outerBorder, ty + th, tw + 2 * outerBorder, outerBorder );

    if (GetParent()) {
        GetParent()->GetPeer()->SetNeedsDisplay(&leftupdate);
        GetParent()->GetPeer()->SetNeedsDisplay(&rightupdate);
        GetParent()->GetPeer()->SetNeedsDisplay(&topupdate);
        GetParent()->GetPeer()->SetNeedsDisplay(&bottomupdate);
    }
}

void wxWindowMac::DoMoveWindow(int x, int y, int width, int height)
{
        int former_x , former_y , former_w, former_h ;

        DoGetPosition( &former_x , &former_y ) ;
    DoGetSize( &former_w , &former_h ) ;

    wxWindow *parent = GetParent();
    if ( parent )
    {
        wxPoint pt(parent->GetClientAreaOrigin());
        former_x += pt.x ;
        former_y += pt.y ;
    }

    int actualWidth = width ;
    int actualHeight = height ;
    int actualX = x;
    int actualY = y;

#if 0
        if ((m_minWidth != -1) && (actualWidth < m_minWidth))
        actualWidth = m_minWidth;
    if ((m_minHeight != -1) && (actualHeight < m_minHeight))
        actualHeight = m_minHeight;
    if ((m_maxWidth != -1) && (actualWidth > m_maxWidth))
        actualWidth = m_maxWidth;
    if ((m_maxHeight != -1) && (actualHeight > m_maxHeight))
        actualHeight = m_maxHeight;
#endif

    bool doMove = false, doResize = false ;

    if ( actualX != former_x || actualY != former_y )
        doMove = true ;

    if ( actualWidth != former_w || actualHeight != former_h )
        doResize = true ;

    if ( doMove || doResize )
    {
        
        wxRect bounds( wxPoint( actualX + MacGetLeftBorderSize() ,actualY + MacGetTopBorderSize() ),
            wxSize( actualWidth - (MacGetLeftBorderSize() + MacGetRightBorderSize()) ,
                actualHeight - (MacGetTopBorderSize() + MacGetBottomBorderSize()) ) ) ;

        if ( parent && !parent->IsTopLevel() )
        {
            bounds.Offset( -parent->MacGetLeftBorderSize(), -parent->MacGetTopBorderSize() );
        }

        MacInvalidateBorders() ;

        m_cachedClippedRectValid = false ;

        GetPeer()->Move( bounds.x, bounds.y, bounds.width, bounds.height);

        wxWindowMac::MacSuperChangedPosition() ; 
        MacInvalidateBorders() ;

        MacRepositionScrollBars() ;
        if ( doMove )
        {
            wxPoint point(actualX, actualY);
            wxMoveEvent event(point, m_windowId);
            event.SetEventObject(this);
            HandleWindowEvent(event) ;
        }

        if ( doResize )
        {
            MacRepositionScrollBars() ;
            SendSizeEvent();
        }
    }
}

wxSize wxWindowMac::DoGetBestSize() const
{
    if ( GetPeer() == NULL || GetPeer()->IsUserPane() || IsTopLevel() )
    {
        return wxWindowBase::DoGetBestSize() ;
    }
    else
    {
        wxRect r ;

        GetPeer()->GetBestRect(&r);

        if ( r.GetWidth() == 0 && r.GetHeight() == 0 )
        {
            r.x =
            r.y = 0 ;
            r.width =
            r.height = 16 ;

#if wxUSE_SCROLLBAR
            if ( IsKindOf( CLASSINFO( wxScrollBar ) ) )
            {
                r.height = 16 ;
            }
            else
#endif
#if wxUSE_SPINBTN
            if ( IsKindOf( CLASSINFO( wxSpinButton ) ) )
            {
                r.height = 24 ;
            }
            else
#endif
            {
                            }
        }

        int bestWidth = r.width + MacGetLeftBorderSize() +
                    MacGetRightBorderSize();
        int bestHeight = r.height + MacGetTopBorderSize() +
                     MacGetBottomBorderSize();
        if ( bestHeight < 10 )
            bestHeight = 13 ;

        return wxSize(bestWidth, bestHeight);
    }
}

void wxWindowMac::SendSizeEvent(int flags)
{
    MacOnInternalSize();
    wxWindowBase::SendSizeEvent(flags);
}

void wxWindowMac::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
        int currentX, currentY;
    int currentW, currentH;

    GetPosition(&currentX, &currentY);
    GetSize(&currentW, &currentH);

        if ( x == currentX && y == currentY &&
        width == currentW && height == currentH && ( height != -1 && width != -1 ) )
    {
                MacRepositionScrollBars() ; 
        if (sizeFlags & wxSIZE_FORCE_EVENT)
        {
            SendSizeEvent();
        }

        return;
    }

    if ( !(sizeFlags & wxSIZE_ALLOW_MINUS_ONE) )
    {
        if ( x == wxDefaultCoord )
            x = currentX;
        if ( y == wxDefaultCoord )
            y = currentY;
    }

    AdjustForParentClientOrigin( x, y, sizeFlags );

    wxSize size = wxDefaultSize;
    if ( width == wxDefaultCoord )
    {
        if ( sizeFlags & wxSIZE_AUTO_WIDTH )
        {
            size = DoGetBestSize();
            width = size.x;
        }
        else
        {
                        width = currentW;
        }
    }

    if ( height == wxDefaultCoord )
    {
        if ( sizeFlags & wxSIZE_AUTO_HEIGHT )
        {
            if ( size.x == wxDefaultCoord )
                size = DoGetBestSize();
            
            height = size.y;
        }
        else
        {
                        height = currentH;
        }
    }

    DoMoveWindow( x, y, width, height );
}

wxPoint wxWindowMac::GetClientAreaOrigin() const
{
    int left,top,width,height;
    GetPeer()->GetContentArea( left , top , width , height);
    return wxPoint( left + MacGetLeftBorderSize() , top + MacGetTopBorderSize() );
}

void wxWindowMac::DoSetClientSize(int clientwidth, int clientheight)
{
    if ( clientwidth != wxDefaultCoord || clientheight != wxDefaultCoord )
    {
        int currentclientwidth , currentclientheight ;
        int currentwidth , currentheight ;

        GetClientSize( &currentclientwidth , &currentclientheight ) ;
        GetSize( &currentwidth , &currentheight ) ;

        DoSetSize( wxDefaultCoord , wxDefaultCoord , currentwidth + clientwidth - currentclientwidth ,
            currentheight + clientheight - currentclientheight , wxSIZE_USE_EXISTING ) ;
    }
}

double wxWindowMac::GetContentScaleFactor() const 
{
    return GetPeer()->GetContentScaleFactor();
}

void wxWindowMac::SetLabel(const wxString& title)
{
    if ( title == m_label )
        return;

    m_label = title ;

    InvalidateBestSize();

    if ( GetPeer() && GetPeer()->IsOk() )
        GetPeer()->SetLabel( wxStripMenuCodes(m_label, wxStrip_Mnemonics), GetFont().GetEncoding() ) ;

        if ( IsShownOnScreen() )
        Refresh() ;
}

wxString wxWindowMac::GetLabel() const
{
    return m_label ;
}

bool wxWindowMac::Show(bool show)
{
    if ( !show )
        MacInvalidateBorders();
    
    if ( !wxWindowBase::Show(show) )
        return false;

    if ( GetPeer() )
        GetPeer()->SetVisibility( show ) ;

    if ( show )
        MacInvalidateBorders();

#ifdef __WXOSX_IPHONE__
        if ( !IsTopLevel() )
#endif
    {
        wxShowEvent eventShow(GetId(), show);
        eventShow.SetEventObject(this);

        HandleWindowEvent(eventShow);
    }

    return true;
}

bool wxWindowMac::OSXShowWithEffect(bool show,
                                    wxShowEffect effect,
                                    unsigned timeout)
{
    if ( effect == wxSHOW_EFFECT_NONE ||
            !GetPeer() || !GetPeer()->ShowWithEffect(show, effect, timeout) )
        return Show(show);

    return true;
}

void wxWindowMac::DoEnable(bool enable)
{
    GetPeer()->Enable( enable ) ;
    MacInvalidateBorders();
}


void wxWindowMac::MacVisibilityChanged()
{
}

void wxWindowMac::MacHiliteChanged()
{
}

void wxWindowMac::MacEnabledStateChanged()
{
}


bool wxWindowMac::MacIsReallyEnabled()
{
    return GetPeer()->IsEnabled() ;
}

bool wxWindowMac::MacIsReallyHilited()
{
    return true; }

int wxWindowMac::GetCharHeight() const
{
    wxCoord height;
    GetTextExtent( wxT("g") , NULL , &height , NULL , NULL , NULL );

    return height;
}

int wxWindowMac::GetCharWidth() const
{
    wxCoord width;
    GetTextExtent( wxT("g") , &width , NULL , NULL , NULL , NULL );

    return width;
}

void wxWindowMac::DoGetTextExtent(const wxString& str,
                                  int *x, int *y,
                                  int *descent,
                                  int *externalLeading,
                                  const wxFont *theFont) const
{
    const wxFont *fontToUse = theFont;
    wxFont tempFont;
    if ( !fontToUse )
    {
        tempFont = GetFont();
        fontToUse = &tempFont;
    }

    wxGraphicsContext* ctx = wxGraphicsContext::Create();
    ctx->SetFont( *fontToUse, *wxBLACK );

    wxDouble h , d , e , w;
    ctx->GetTextExtent( str, &w, &h, &d, &e );

    delete ctx;

    if ( externalLeading )
        *externalLeading = (wxCoord)(e+0.5);
    if ( descent )
        *descent = (wxCoord)(d+0.5);
    if ( x )
        *x = (wxCoord)(w+0.5);
    if ( y )
        *y = (wxCoord)(h+0.5);
}



void wxWindowMac::Refresh(bool WXUNUSED(eraseBack), const wxRect *rect)
{
    if ( GetPeer() == NULL )
        return ;

    if ( !IsShownOnScreen() )
        return ;
    
    if ( IsFrozen() )
        return;

    GetPeer()->SetNeedsDisplay( rect ) ;
}

void wxWindowMac::DoFreeze()
{
    if ( GetPeer() && GetPeer()->IsOk() )
        GetPeer()->SetDrawingEnabled( false ) ;
}

void wxWindowMac::DoThaw()
{
    if ( GetPeer() && GetPeer()->IsOk() )
        GetPeer()->SetDrawingEnabled( true ) ;
}

wxWindow *wxGetActiveWindow()
{
        return NULL;
}

void wxWindowMac::WarpPointer(int x_pos, int y_pos)
{
#if wxOSX_USE_COCOA_OR_CARBON
    int x = x_pos;
    int y = y_pos;
    DoClientToScreen(&x, &y);
    CGPoint cgpoint = CGPointMake( x, y );
    CGWarpMouseCursorPosition( cgpoint );
    CGAssociateMouseAndMouseCursorPosition(true);

        wxMouseEvent event(wxEVT_MOTION);
    event.m_x = x_pos;
    event.m_y = y_pos;
    wxMouseState mState = ::wxGetMouseState();

    event.m_altDown = mState.AltDown();
    event.m_controlDown = mState.ControlDown();
    event.m_leftDown = mState.LeftIsDown();
    event.m_middleDown = mState.MiddleIsDown();
    event.m_rightDown = mState.RightIsDown();
    event.m_metaDown = mState.MetaDown();
    event.m_shiftDown = mState.ShiftDown();
    event.SetId(GetId());
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(event);
#endif
}

int wxWindowMac::GetScrollPos(int orient) const
{
#if wxUSE_SCROLLBAR
    if ( orient == wxHORIZONTAL )
    {
       if ( m_hScrollBar )
           return m_hScrollBar->GetThumbPosition() ;
    }
    else
    {
       if ( m_vScrollBar )
           return m_vScrollBar->GetThumbPosition() ;
    }
#endif
    return 0;
}

int wxWindowMac::GetScrollRange(int orient) const
{
#if wxUSE_SCROLLBAR
    if ( orient == wxHORIZONTAL )
    {
       if ( m_hScrollBar )
           return m_hScrollBar->GetRange() ;
    }
    else
    {
       if ( m_vScrollBar )
           return m_vScrollBar->GetRange() ;
    }
#endif
    return 0;
}

int wxWindowMac::GetScrollThumb(int orient) const
{
#if wxUSE_SCROLLBAR
    if ( orient == wxHORIZONTAL )
    {
       if ( m_hScrollBar )
           return m_hScrollBar->GetThumbSize() ;
    }
    else
    {
       if ( m_vScrollBar )
           return m_vScrollBar->GetThumbSize() ;
    }
#endif
    return 0;
}

void wxWindowMac::SetScrollPos(int orient, int pos, bool WXUNUSED(refresh))
{
#if wxUSE_SCROLLBAR
    if ( orient == wxHORIZONTAL )
    {
       if ( m_hScrollBar )
           m_hScrollBar->SetThumbPosition( pos ) ;
    }
    else
    {
       if ( m_vScrollBar )
           m_vScrollBar->SetThumbPosition( pos ) ;
    }
#endif
}

void
wxWindowMac::AlwaysShowScrollbars(bool hflag, bool vflag)
{
    bool needVisibilityUpdate = false;

    if ( m_hScrollBarAlwaysShown != hflag )
    {
        m_hScrollBarAlwaysShown = hflag;
        needVisibilityUpdate = true;
    }

    if ( m_vScrollBarAlwaysShown != vflag )
    {
        m_vScrollBarAlwaysShown = vflag;
        needVisibilityUpdate = true;
    }

    if ( needVisibilityUpdate )
        DoUpdateScrollbarVisibility();
}


void  wxWindowMac::MacPaintGrowBox()
{
    if ( IsTopLevel() )
        return ;

#if wxUSE_SCROLLBAR
    if ( MacHasScrollBarCorner() )
    {
#if 0
        CGContextRef cgContext = (CGContextRef) MacGetCGContextRef() ;
        wxASSERT( cgContext ) ;

        int tx,ty,tw,th;

        GetPeer()->GetSize( tw, th );
        GetPeer()->GetPosition( tx, ty );

        Rect rect  = { ty,tx, ty+th, tx+tw };


        int size = m_hScrollBar ? m_hScrollBar->GetSize().y : ( m_vScrollBar ? m_vScrollBar->GetSize().x : MAC_SCROLLBAR_SIZE ) ;
        CGRect cgrect = CGRectMake( rect.right - size , rect.bottom - size , size , size ) ;
        CGContextSaveGState( cgContext );

        if ( m_backgroundColour.IsOk() )
        {
            CGContextSetFillColorWithColor( cgContext, m_backgroundColour.GetCGColor() );
        }
        else
        {
            CGContextSetRGBFillColor( cgContext, (CGFloat) 1.0, (CGFloat)1.0 ,(CGFloat) 1.0 , (CGFloat)1.0 );
        }
        CGContextFillRect( cgContext, cgrect );
        CGContextRestoreGState( cgContext );
#else
        if (m_growBox)
        {
             if ( m_backgroundColour.IsOk() )
                 m_growBox->SetBackgroundColour(m_backgroundColour);
             else
                 m_growBox->SetBackgroundColour(*wxWHITE);
        }
#endif
    }

#endif
}

void wxWindowMac::MacPaintBorders( int WXUNUSED(leftOrigin) , int WXUNUSED(rightOrigin) )
{
    if ( IsTopLevel() )
        return ;

        int tx,ty,tw,th;

    GetPeer()->GetSize( tw, th );
    GetPeer()->GetPosition( tx, ty );

#if wxOSX_USE_COCOA_OR_CARBON
    {
        const bool hasFocus = GetPeer()->NeedsFocusRect() && HasFocus();

        CGRect cgrect = CGRectMake( tx-1 , ty-1 , tw+2 ,
            th+2 ) ;

        CGContextRef cgContext = (CGContextRef) GetParent()->MacGetCGContextRef() ;
        wxASSERT( cgContext ) ;

        if ( GetPeer()->NeedsFrame() )
        {
            HIThemeFrameDrawInfo info ;
            memset( &info, 0 , sizeof(info) ) ;

            info.version = 0 ;
            info.kind = 0 ;
            info.state = IsEnabled() ? kThemeStateActive : kThemeStateInactive ;
            info.isFocused = hasFocus ;

            if ( HasFlag(wxRAISED_BORDER) || HasFlag(wxSUNKEN_BORDER) || HasFlag(wxDOUBLE_BORDER) )
            {
                info.kind = kHIThemeFrameTextFieldSquare ;
                HIThemeDrawFrame( &cgrect , &info , cgContext , kHIThemeOrientationNormal ) ;
            }
            else if ( HasFlag(wxSIMPLE_BORDER) )
            {
                info.kind = kHIThemeFrameListBox ;
                HIThemeDrawFrame( &cgrect , &info , cgContext , kHIThemeOrientationNormal ) ;
            }
        }

        if ( hasFocus )
        {
            HIThemeDrawFocusRect( &cgrect , true , cgContext , kHIThemeOrientationNormal ) ;
        }
    }
#endif }

void wxWindowMac::RemoveChild( wxWindowBase *child )
{
#if wxUSE_SCROLLBAR
    if ( child == m_hScrollBar )
        m_hScrollBar = NULL ;
    if ( child == m_vScrollBar )
        m_vScrollBar = NULL ;
    if ( child == m_growBox )
        m_growBox = NULL ;
#endif
    wxWindowBase::RemoveChild( child ) ;
}

void wxWindowMac::DoUpdateScrollbarVisibility()
{
#if wxUSE_SCROLLBAR
    bool triggerSizeEvent = false;

    if ( m_hScrollBar )
    {
        bool showHScrollBar = m_hScrollBarAlwaysShown || m_hScrollBar->IsNeeded();

        if ( m_hScrollBar->IsShown() != showHScrollBar )
        {
            m_hScrollBar->Show( showHScrollBar );
            triggerSizeEvent = true;
        }
    }

    if ( m_vScrollBar)
    {
        bool showVScrollBar = m_vScrollBarAlwaysShown || m_vScrollBar->IsNeeded();

        if ( m_vScrollBar->IsShown() != showVScrollBar )
        {
            m_vScrollBar->Show( showVScrollBar ) ;
            triggerSizeEvent = true;
        }
    }

    MacRepositionScrollBars() ;
    if ( triggerSizeEvent )
    {
        SendSizeEvent();
    }
#endif
}

void wxWindowMac::SetScrollbar(int orient, int pos, int thumb,
                               int range, bool refresh)
{
#if wxUSE_SCROLLBAR
                if ( m_isBeingDeleted )
        return;

    if ( orient == wxHORIZONTAL && m_hScrollBar )
        m_hScrollBar->SetScrollbar(pos, thumb, range, thumb, refresh);
    else if ( orient == wxVERTICAL && m_vScrollBar )
        m_vScrollBar->SetScrollbar(pos, thumb, range, thumb, refresh);

    DoUpdateScrollbarVisibility();
#endif
}

void wxWindowMac::ScrollWindow(int dx, int dy, const wxRect *rect)
{
    if ( dx == 0 && dy == 0 )
        return ;

    int width , height ;
    GetClientSize( &width , &height ) ;

    {
        wxRect scrollrect( MacGetLeftBorderSize() , MacGetTopBorderSize() , width , height ) ;
        if ( rect )
            scrollrect.Intersect( *rect ) ;
                scrollrect.Offset( -MacGetLeftBorderSize() , -MacGetTopBorderSize() ) ;

        GetPeer()->ScrollRect( &scrollrect, dx, dy );
    }

    wxWindowMac *child;
    int x, y, w, h;
    for (wxWindowList::compatibility_iterator node = GetChildren().GetFirst(); node; node = node->GetNext())
    {
        child = node->GetData();
        if (child == NULL)
            continue;

        if (child->IsTopLevel())
            continue;

        if ( !IsClientAreaChild(child) )
            continue;

        child->GetPosition( &x, &y );
        child->GetSize( &w, &h );
        if (rect)
        {
            wxRect rc( x, y, w, h );
            if (rect->Intersects( rc ))
                child->SetSize( x + dx, y + dy, w, h, wxSIZE_AUTO|wxSIZE_ALLOW_MINUS_ONE );
        }
        else
        {
            child->SetSize( x + dx, y + dy, w, h, wxSIZE_AUTO|wxSIZE_ALLOW_MINUS_ONE );
        }
    }
}

void wxWindowMac::MacOnScroll( wxScrollEvent &event )
{
#if wxUSE_SCROLLBAR
    if ( event.GetEventObject() == m_vScrollBar || event.GetEventObject() == m_hScrollBar )
    {
        wxScrollWinEvent wevent;
        wevent.SetPosition(event.GetPosition());
        wevent.SetOrientation(event.GetOrientation());
        wevent.SetEventObject(this);

        if (event.GetEventType() == wxEVT_SCROLL_TOP)
            wevent.SetEventType( wxEVT_SCROLLWIN_TOP );
        else if (event.GetEventType() == wxEVT_SCROLL_BOTTOM)
            wevent.SetEventType( wxEVT_SCROLLWIN_BOTTOM );
        else if (event.GetEventType() == wxEVT_SCROLL_LINEUP)
            wevent.SetEventType( wxEVT_SCROLLWIN_LINEUP );
        else if (event.GetEventType() == wxEVT_SCROLL_LINEDOWN)
            wevent.SetEventType( wxEVT_SCROLLWIN_LINEDOWN );
        else if (event.GetEventType() == wxEVT_SCROLL_PAGEUP)
            wevent.SetEventType( wxEVT_SCROLLWIN_PAGEUP );
        else if (event.GetEventType() == wxEVT_SCROLL_PAGEDOWN)
            wevent.SetEventType( wxEVT_SCROLLWIN_PAGEDOWN );
        else if (event.GetEventType() == wxEVT_SCROLL_THUMBTRACK)
            wevent.SetEventType( wxEVT_SCROLLWIN_THUMBTRACK );
        else if (event.GetEventType() == wxEVT_SCROLL_THUMBRELEASE)
            wevent.SetEventType( wxEVT_SCROLLWIN_THUMBRELEASE );

        HandleWindowEvent(wevent);
    }
#endif
}

wxWindow *wxWindowBase::DoFindFocus()
{
    return wxFindWindowFromWXWidget(wxWidgetImpl::FindFocus());
}

void wxWindowMac::Raise()
{
    GetPeer()->Raise();
}

void wxWindowMac::Lower()
{
    GetPeer()->Lower();
}


bool wxWindowMac::MacSetupCursor( const wxPoint& pt )
{
    
    wxPoint clientorigin = GetClientAreaOrigin() ;
    wxSize clientsize = GetClientSize() ;
    wxCursor cursor ;
    if ( wxRect2DInt( clientorigin.x , clientorigin.y , clientsize.x , clientsize.y ).Contains( wxPoint2DInt( pt ) ) )
    {
        wxSetCursorEvent event( pt.x , pt.y );
        event.SetId(GetId());
        event.SetEventObject(this);

        bool processedEvtSetCursor = HandleWindowEvent(event);
        if ( processedEvtSetCursor && event.HasCursor() )
        {
            cursor = event.GetCursor() ;
        }
        else
        {
                                                            if ( !processedEvtSetCursor && m_cursor.IsOk() )
                cursor = m_cursor ;

            if ( !wxIsBusy() && !GetParent() )
                cursor = *wxSTANDARD_CURSOR ;
        }

        if ( cursor.IsOk() )
            cursor.MacInstall() ;
    }

    return cursor.IsOk() ;
}

wxString wxWindowMac::MacGetToolTipString( wxPoint &WXUNUSED(pt) )
{
#if wxUSE_TOOLTIPS
    if ( m_tooltip )
        return m_tooltip->GetTip() ;
#endif

    return wxEmptyString ;
}

void wxWindowMac::ClearBackground()
{
    Refresh() ;
    Update() ;
}

void wxWindowMac::Update()
{
    wxNonOwnedWindow* top = MacGetTopLevelWindow();
    if (top)
        top->Update() ;
}

wxNonOwnedWindow* wxWindowMac::MacGetTopLevelWindow() const
{
    wxWindowMac *iter = (wxWindowMac*)this ;

    while ( iter )
    {
        if ( iter->IsTopLevel() )
        {
            wxTopLevelWindow* toplevel = wxDynamicCast(iter,wxTopLevelWindow);
            if ( toplevel )
                return toplevel;
#if wxUSE_POPUPWIN
            wxPopupWindow* popupwin = wxDynamicCast(iter,wxPopupWindow);
            if ( popupwin )
                return popupwin;
#endif
        }
        iter = iter->GetParent() ;
    }

    return NULL ;
}

const wxRect& wxWindowMac::MacGetClippedClientRect() const
{
    MacUpdateClippedRects() ;

    return m_cachedClippedClientRect ;
}

const wxRect& wxWindowMac::MacGetClippedRect() const
{
    MacUpdateClippedRects() ;

    return m_cachedClippedRect ;
}

const wxRect&wxWindowMac:: MacGetClippedRectWithOuterStructure() const
{
    MacUpdateClippedRects() ;

    return m_cachedClippedRectWithOuterStructure ;
}

const wxRegion& wxWindowMac::MacGetVisibleRegion( bool includeOuterStructures )
{
    static wxRegion emptyrgn ;

    if ( !m_isBeingDeleted && IsShownOnScreen() )
    {
        MacUpdateClippedRects() ;
        if ( includeOuterStructures )
            return m_cachedClippedRegionWithOuterStructure ;
        else
            return m_cachedClippedRegion ;
    }
    else
    {
        return emptyrgn ;
    }
}

void wxWindowMac::MacUpdateClippedRects() const
{
}


bool wxWindowMac::MacDoRedraw( long time )
{
    bool handled = false ;

    wxRegion formerUpdateRgn = m_updateRegion;
    wxRegion clientUpdateRgn = formerUpdateRgn;

    const wxRect clientRect = GetClientRect();

    clientUpdateRgn.Intersect(clientRect);

        const wxBackgroundStyle bgStyle = GetBackgroundStyle();
    switch ( bgStyle )
    {
        case wxBG_STYLE_ERASE:
        case wxBG_STYLE_SYSTEM:
        case wxBG_STYLE_COLOUR:
            {
                                                                                wxWindowDC dc(this);
                if ( IsTopLevel() )
                    dc.SetDeviceClippingRegion(formerUpdateRgn);
                else
                    dc.SetDeviceClippingRegion(clientUpdateRgn);

                if ( bgStyle == wxBG_STYLE_ERASE )
                {
                    wxEraseEvent eevent( GetId(), &dc );
                    eevent.SetEventObject( this );
                    if ( ProcessWindowEvent( eevent ) )
                        break;
                }

                if ( UseBgCol() )
                {
                    dc.SetBackground(GetBackgroundColour());
                    dc.Clear();
                }
            }
            break;

        case wxBG_STYLE_PAINT:
        case wxBG_STYLE_TRANSPARENT:
                                    break;

        default:
            wxFAIL_MSG( "unsupported background style" );
    }

        
            clientUpdateRgn.Offset(-clientRect.GetPosition());
    m_updateRegion = clientUpdateRgn ;

    if ( !m_updateRegion.Empty() )
    {
        
        wxPaintEvent event(GetId());
        event.SetTimestamp(time);
        event.SetEventObject(this);
        handled = HandleWindowEvent(event);
    }

    m_updateRegion = formerUpdateRgn;

    wxNonOwnedWindow* top = MacGetTopLevelWindow();
    if (top)
        top->WindowWasPainted() ;
    
    return handled;
}

void wxWindowMac::MacPaintChildrenBorders()
{
                    wxPoint clientOrigin = GetClientAreaOrigin() ;

    wxWindowMac *child;
    int x, y, w, h;
    for (wxWindowList::compatibility_iterator node = GetChildren().GetFirst(); node; node = node->GetNext())
    {
        child = node->GetData();
        if (child == NULL)
            continue;
#if wxUSE_SCROLLBAR
        if (child == m_vScrollBar)
            continue;
        if (child == m_hScrollBar)
            continue;
         if (child == m_growBox)
             continue;
#endif
        if (child->IsTopLevel())
            continue;
        if (!child->IsShown())
            continue;

        
        child->GetPosition( &x, &y );
        child->GetSize( &w, &h );

        if ( m_updateRegion.Contains(clientOrigin.x+x-10, clientOrigin.y+y-10, w+20, h+20) )
        {
                        wxNcPaintEvent eventNc( child->GetId() );
            eventNc.SetEventObject( child );
            if ( !child->HandleWindowEvent( eventNc ) )
            {
                child->MacPaintBorders(0, 0) ;
            }
        }
    }
}


WXWindow wxWindowMac::MacGetTopLevelWindowRef() const
{
    wxNonOwnedWindow* tlw = MacGetTopLevelWindow();
    return tlw ? tlw->GetWXWindow() : NULL ;
}

bool wxWindowMac::MacHasScrollBarCorner() const
{
#if wxUSE_SCROLLBAR && !wxOSX_USE_IPHONE
    

    if ( m_hScrollBar == NULL && m_vScrollBar == NULL )
        return false;

    if ( ( m_hScrollBar && m_hScrollBar->IsShown() )
         && ( m_vScrollBar && m_vScrollBar->IsShown() ) )
    {
                return true;
    }
    else
    {
        wxPoint thisWindowBottomRight = GetScreenRect().GetBottomRight();

        for ( const wxWindow *win = (wxWindow*)this; win; win = win->GetParent() )
        {
            const wxFrame *frame = wxDynamicCast( win, wxFrame ) ;
            if ( frame )
            {
                                if ( (frame->GetWindowStyleFlag() & wxRESIZE_BORDER) && !wxPlatformInfo::Get().CheckOSVersion(10, 7) )
                {
                                        wxPoint frameBottomRight = frame->GetScreenRect().GetBottomRight();

                                                            if ( abs( thisWindowBottomRight.x - frameBottomRight.x ) <= 2
                        && abs( thisWindowBottomRight.y - frameBottomRight.y ) <= 2 )
                    {
                                                                        return true ;
                    }
                    else
                    {
                                                                        return false ;
                    }
                }
                else
                {
                                        return false ;
                }
            }
        }

                return false ;
    }
#else
    return false;
#endif
}

void wxWindowMac::MacCreateScrollBars( long style )
{
#if wxUSE_SCROLLBAR
    wxASSERT_MSG( m_vScrollBar == NULL && m_hScrollBar == NULL , wxT("attempt to create window twice") ) ;

    if ( style & ( wxVSCROLL | wxHSCROLL ) )
    {
        int scrlsize = MAC_SCROLLBAR_SIZE ;
        if ( GetWindowVariant() == wxWINDOW_VARIANT_SMALL || GetWindowVariant() == wxWINDOW_VARIANT_MINI )
        {
            scrlsize = MAC_SMALL_SCROLLBAR_SIZE ;
        }

        int adjust = MacHasScrollBarCorner() ? scrlsize - 1: 0 ;
        int width, height ;
        GetClientSize( &width , &height ) ;

        wxPoint vPoint(width - scrlsize, 0) ;
        wxSize vSize(scrlsize, height - adjust) ;
        wxPoint hPoint(0, height - scrlsize) ;
        wxSize hSize(width - adjust, scrlsize) ;

                if ( style & wxVSCROLL )
        {
            m_vScrollBar = new wxScrollBar((wxWindow*)this, wxID_ANY, vPoint, vSize , wxVERTICAL);
            m_vScrollBar->SetMinSize( wxDefaultSize );
        }

        if ( style  & wxHSCROLL )
        {
            m_hScrollBar = new wxScrollBar((wxWindow*)this, wxID_ANY, hPoint, hSize , wxHORIZONTAL);
            m_hScrollBar->SetMinSize( wxDefaultSize );
        }

        wxPoint gPoint(width - scrlsize, height - scrlsize);
        wxSize gSize(scrlsize, scrlsize);
        m_growBox = new wxBlindPlateWindow((wxWindow *)this, wxID_ANY, gPoint, gSize, 0);
    }

            MacRepositionScrollBars() ;
#endif
}

bool wxWindowMac::MacIsChildOfClientArea( const wxWindow* child ) const
{
    bool result = ((child == NULL)
#if wxUSE_SCROLLBAR
      || ((child != m_hScrollBar) && (child != m_vScrollBar) && (child != m_growBox))
#endif
     );

    return result ;
}

void wxWindowMac::MacRepositionScrollBars()
{
#if wxUSE_SCROLLBAR
    if ( !m_hScrollBar && !m_vScrollBar )
        return ;

    int scrlsize = m_hScrollBar ? m_hScrollBar->GetSize().y : ( m_vScrollBar ? m_vScrollBar->GetSize().x : MAC_SCROLLBAR_SIZE ) ;
    int adjust = MacHasScrollBarCorner() ? scrlsize - 1 : 0 ;

        int width, height ;
    GetSize( &width , &height );

    width -= MacGetLeftBorderSize() + MacGetRightBorderSize();
    height -= MacGetTopBorderSize() + MacGetBottomBorderSize();

    wxPoint vPoint( width - scrlsize, 0 ) ;
    wxSize vSize( scrlsize, height - adjust ) ;
    wxPoint hPoint( 0 , height - scrlsize ) ;
    wxSize hSize( width - adjust, scrlsize ) ;

    if ( m_vScrollBar )
        m_vScrollBar->SetSize( vPoint.x , vPoint.y, vSize.x, vSize.y , wxSIZE_ALLOW_MINUS_ONE );
    if ( m_hScrollBar )
        m_hScrollBar->SetSize( hPoint.x , hPoint.y, hSize.x, hSize.y, wxSIZE_ALLOW_MINUS_ONE );
    if ( m_growBox )
    {
        if ( MacHasScrollBarCorner() )
        {
            m_growBox->SetSize( width - scrlsize, height - scrlsize, wxDefaultCoord, wxDefaultCoord, wxSIZE_USE_EXISTING );
            if ( !m_growBox->IsShown() )
                m_growBox->Show();
        }
        else
        {
            if ( m_growBox->IsShown() )
                m_growBox->Hide();
        }
    }
#endif
}

bool wxWindowMac::AcceptsFocus() const
{
    if ( GetPeer() == NULL || GetPeer()->IsUserPane() )
        return wxWindowBase::AcceptsFocus();
    else
        return GetPeer()->CanFocus();
}

void wxWindowMac::MacSuperChangedPosition()
{
    
    m_cachedClippedRectValid = false ;

    wxWindowMac *child;
    wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
    while ( node )
    {
        child = node->GetData();
        child->MacSuperChangedPosition() ;

        node = node->GetNext();
    }
}

void wxWindowMac::MacTopLevelWindowChangedPosition()
{
    
    wxWindowMac *child;
    wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
    while ( node )
    {
        child = node->GetData();
        child->MacTopLevelWindowChangedPosition() ;

        node = node->GetNext();
    }
}

long wxWindowMac::MacGetWXBorderSize() const
{
    if ( IsTopLevel() )
        return 0 ;

    SInt32 border = 0 ;

    if ( GetPeer() && GetPeer()->NeedsFrame() )
    {
        if (HasFlag(wxRAISED_BORDER) || HasFlag( wxSUNKEN_BORDER) || HasFlag(wxDOUBLE_BORDER))
        {
#if wxOSX_USE_COCOA_OR_CARBON
                        GetThemeMetric( kThemeMetricEditTextFrameOutset , &border ) ;
            border += 1;
#else
            border += 2;
#endif
        }
        else if (HasFlag(wxSIMPLE_BORDER))
        {
#if wxOSX_USE_COCOA_OR_CARBON
                        GetThemeMetric( kThemeMetricListBoxFrameOutset , &border ) ;
            border += 1;
#else
            border += 1;
#endif
        }
    }

    return border ;
}

long wxWindowMac::MacGetLeftBorderSize() const
{
        long border = MacGetWXBorderSize() ;
 
    if ( GetPeer() )
    {
        int left, top, right, bottom;
        GetPeer()->GetLayoutInset( left, top, right, bottom );
        border -= left;
    }
    
    return border;
}


long wxWindowMac::MacGetRightBorderSize() const
{
        long border = MacGetWXBorderSize() ;
    
    if ( GetPeer() )
    {
        int left, top, right, bottom;
        GetPeer()->GetLayoutInset( left, top, right, bottom );
        border -= right;
    }
    
    return border;
}

long wxWindowMac::MacGetTopBorderSize() const
{
        long border = MacGetWXBorderSize() ;
    
    if ( GetPeer() )
    {
        int left, top, right, bottom;
        GetPeer()->GetLayoutInset( left, top, right, bottom );
        border -= top;
    }
    
    return border;
}

long wxWindowMac::MacGetBottomBorderSize() const
{
        long border = MacGetWXBorderSize() ;
    
    if ( GetPeer() )
    {
        int left, top, right, bottom;
        GetPeer()->GetLayoutInset( left, top, right, bottom );
        border -= bottom;
    }
    
    return border;
}

long wxWindowMac::MacRemoveBordersFromStyle( long style )
{
    return style & ~wxBORDER_MASK ;
}

wxWindow * wxFindWindowAtPointer( wxPoint& pt )
{
    pt = wxGetMousePosition();
    wxWindowMac* found = wxFindWindowAtPoint(pt);

    return (wxWindow*) found;
}

wxPoint wxGetMousePosition()
{
    int x, y;

    wxGetMousePosition( &x, &y );

    return wxPoint(x, y);
}

void wxWindowMac::OnMouseEvent( wxMouseEvent &event )
{
    if ( event.GetEventType() == wxEVT_RIGHT_DOWN )
    {
                
                                                        wxContextMenuEvent evtCtx(wxEVT_CONTEXT_MENU,
                                  this->GetId(),
                                  this->ClientToScreen(event.GetPosition()));
        evtCtx.SetEventObject(this);
        if ( ! HandleWindowEvent(evtCtx) )
            event.Skip() ;
    }
    else
    {
        event.Skip() ;
    }
}

void wxWindowMac::TriggerScrollEvent( wxEventType WXUNUSED(scrollEvent) )
{
}

Rect wxMacGetBoundsForControl( wxWindowMac* window , const wxPoint& pos , const wxSize &size , bool adjustForOrigin )
{
    int x, y, w, h ;

    window->MacGetBoundsForControl( pos , size , x , y, w, h , adjustForOrigin ) ;
    Rect bounds = { static_cast<short>(y), static_cast<short>(x), static_cast<short>(y + h), static_cast<short>(x + w) };

    return bounds ;
}

bool wxWindowMac::OSXHandleClicked( double WXUNUSED(timestampsec) )
{
    return false;
}

#if wxOSX_USE_COCOA_OR_IPHONE
void *wxWindowMac::OSXGetViewOrWindow() const
{
    return GetHandle();
}
#endif

wxInt32 wxWindowMac::MacControlHit(WXEVENTHANDLERREF WXUNUSED(handler) , WXEVENTREF event )
{
#if wxOSX_USE_COCOA_OR_CARBON
    if ( OSXHandleClicked( GetEventTime((EventRef)event) ) )
        return noErr;

    return eventNotHandledErr ;
#else
    return 0;
#endif
}

bool wxWindowMac::Reparent(wxWindowBase *newParentBase)
{
    wxWindowMac *newParent = (wxWindowMac *)newParentBase;
    if ( !wxWindowBase::Reparent(newParent) )
        return false;

    GetPeer()->RemoveFromParent();
    GetPeer()->Embed( GetParent()->GetPeer() );

    MacChildAdded();
    return true;
}

bool wxWindowMac::SetTransparent(wxByte alpha)
{
    SetBackgroundStyle(wxBG_STYLE_TRANSPARENT);

    if ( alpha != m_macAlpha )
    {
        m_macAlpha = alpha ;
        Refresh() ;
    }
    return true ;
}


bool wxWindowMac::CanSetTransparent()
{
    return true ;
}

wxByte wxWindowMac::GetTransparent() const
{
    return m_macAlpha ;
}

bool wxWindowMac::IsShownOnScreen() const
{
    if ( GetPeer() && GetPeer()->IsOk() )
    {
        bool peerVis = GetPeer()->IsVisible();
        bool wxVis = wxWindowBase::IsShownOnScreen();
        if( peerVis != wxVis )
        {
                                                                                    wxVis = wxWindowBase::IsShownOnScreen();
            return wxVis;
        }

        return GetPeer()->IsVisible();
    }
    return wxWindowBase::IsShownOnScreen();
}

#if wxUSE_HOTKEY && wxOSX_USE_COCOA_OR_CARBON

OSStatus
wxHotKeyHandler(EventHandlerCallRef WXUNUSED(nextHandler),
                EventRef event,
                void* WXUNUSED(userData))
{
    EventHotKeyID hotKeyId;

    GetEventParameter( event, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(hotKeyId), NULL, &hotKeyId);

    for ( unsigned i = 0; i < s_hotkeys.size(); ++i )
    {
        if ( s_hotkeys[i].keyId == static_cast<int>(hotKeyId.id) )
        {
            unsigned char charCode ;
            UInt32 keyCode ;
            UInt32 modifiers ;
            UInt32 when = EventTimeToTicks( GetEventTime( event ) ) ;

            GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, 1, NULL, &charCode );
            GetEventParameter( event, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &keyCode );
            GetEventParameter( event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &modifiers );
            
            UInt32 keymessage = (keyCode << 8) + charCode;
            
            wxKeyEvent wxevent(wxEVT_HOTKEY);
            wxevent.SetId(hotKeyId.id);
            wxTheApp->MacCreateKeyEvent( wxevent, s_hotkeys[i].window , keymessage , 
                                        modifiers , when , 0 ) ;
            
            s_hotkeys[i].window->HandleWindowEvent(wxevent);
        }
    }
    
    return noErr;
}

bool wxWindowMac::RegisterHotKey(int hotkeyId, int modifiers, int keycode)
{
    for ( unsigned i = 0; i < s_hotkeys.size(); ++i )
    {
        if ( s_hotkeys[i].keyId == hotkeyId )
        {
            wxLogLastError(wxT("hotkeyId already registered"));
                
            return false;
        }
    }
    
    static bool installed = false;
    if ( !installed )
    {
        EventTypeSpec eventType;
        eventType.eventClass=kEventClassKeyboard;
        eventType.eventKind=kEventHotKeyPressed;

        InstallApplicationEventHandler(&wxHotKeyHandler, 1, &eventType, NULL, NULL);
        installed = true;
    }
    
    UInt32 mac_modifiers=0;
    if ( modifiers & wxMOD_ALT )
        mac_modifiers |= optionKey;
    if ( modifiers & wxMOD_SHIFT )
        mac_modifiers |= shiftKey;
    if ( modifiers & wxMOD_RAW_CONTROL )
        mac_modifiers |= controlKey;
    if ( modifiers & wxMOD_CONTROL )
        mac_modifiers |= cmdKey;
    
    EventHotKeyRef hotKeyRef;
    EventHotKeyID hotKeyIDmac;
    
    hotKeyIDmac.signature = 'WXMC';
    hotKeyIDmac.id = hotkeyId;
    
    if ( RegisterEventHotKey(wxCharCodeWXToOSX((wxKeyCode)keycode), mac_modifiers, hotKeyIDmac,
                        GetApplicationEventTarget(), 0, &hotKeyRef) != noErr )
    {
        wxLogLastError(wxT("RegisterHotKey"));
        
        return false;
    }
    else
    {
        wxHotKeyRec v;
        v.ref = hotKeyRef;
        v.keyId = hotkeyId;
        v.window = this;
        
        s_hotkeys.push_back(v);
    }
    
    return true;
}

bool wxWindowMac::UnregisterHotKey(int hotkeyId)
{
    for ( int i = ((int)s_hotkeys.size())-1; i>=0; -- i )
    {
        if ( s_hotkeys[i].keyId == hotkeyId )
        {
            EventHotKeyRef ref = s_hotkeys[i].ref;
            s_hotkeys.erase(s_hotkeys.begin() + i);
            if ( UnregisterEventHotKey(ref) != noErr )
            {
                wxLogLastError(wxT("UnregisterHotKey"));
                
                return false;
            }
            else 
                return true;
        }
    }
    
    return false;
}

#endif 
bool wxWindowMac::OSXHandleKeyEvent( wxKeyEvent& event )
{
    bool handled = false;
    
    
#if wxUSE_ACCEL
    if ( !handled && event.GetEventType() == wxEVT_KEY_DOWN)
    {
        wxWindow *ancestor = this;
        while (ancestor)
        {
            int command = ancestor->GetAcceleratorTable()->GetCommand( event );
            if (command != -1)
            {
                wxEvtHandler * const handler = ancestor->GetEventHandler();

                wxCommandEvent command_event( wxEVT_MENU, command );
                handled = handler->ProcessEvent( command_event );

                if ( !handled )
                {
                                        command_event.SetEventType(wxEVT_BUTTON);
                    handled = handler->ProcessEvent( command_event );
                }

                break;
            }

            if (ancestor->IsTopNavigationDomain(wxWindow::Navigation_Accel))
                break;

            ancestor = ancestor->GetParent();
        }
    }
#endif     
    if ( !handled )
    {
        handled = HandleWindowEvent( event ) ;
        if ( handled && event.GetSkipped() )
            handled = false ;
    }

    return handled ;
}



WX_DECLARE_HASH_MAP(WXWidget, wxWidgetImpl*, wxPointerHash, wxPointerEqual, MacControlMap);

static MacControlMap wxWinMacControlList;

wxWindowMac *wxFindWindowFromWXWidget(WXWidget inControl )
{
    wxWidgetImpl* impl = wxWidgetImpl::FindFromWXWidget( inControl );
    if ( impl )
        return impl->GetWXPeer();

    return NULL;
}

wxWidgetImpl *wxWidgetImpl::FindFromWXWidget(WXWidget inControl )
{
    MacControlMap::iterator node = wxWinMacControlList.find(inControl);

    return (node == wxWinMacControlList.end()) ? NULL : node->second;
}

void wxWidgetImpl::Associate(WXWidget inControl, wxWidgetImpl *impl)
{
            wxCHECK_RET( inControl != (WXWidget) NULL, wxT("attempt to add a NULL WXWidget to control map") );

    wxWinMacControlList[inControl] = impl;
}

void wxWidgetImpl::RemoveAssociations(wxWidgetImpl* impl)
{
           
    bool found = true ;
    while ( found )
    {
        found = false ;
        MacControlMap::iterator it;
        for ( it = wxWinMacControlList.begin(); it != wxWinMacControlList.end(); ++it )
        {
            if ( it->second == impl )
            {
                wxWinMacControlList.erase(it);
                found = true ;
                break;
            }
        }
    }
}

wxIMPLEMENT_ABSTRACT_CLASS(wxWidgetImpl, wxObject);

wxWidgetImpl::wxWidgetImpl( wxWindowMac* peer , bool isRootControl, bool isUserPane )
{
    Init();
    m_isRootControl = isRootControl;
    m_isUserPane = isUserPane;
    m_wxPeer = peer;
    m_shouldSendEvents = true;
}

wxWidgetImpl::wxWidgetImpl()
{
    Init();
}

wxWidgetImpl::~wxWidgetImpl()
{
    m_wxPeer = NULL;
}

void wxWidgetImpl::Init()
{
    m_isRootControl = false;
    m_wxPeer = NULL;
    m_needsFocusRect = false;
    m_needsFrame = true;
}

void wxWidgetImpl::SetNeedsFocusRect( bool needs )
{
    m_needsFocusRect = needs;
}

bool wxWidgetImpl::NeedsFocusRect() const
{
    return m_needsFocusRect;
}

void wxWidgetImpl::SetNeedsFrame( bool needs )
{
    m_needsFrame = needs;
}

bool wxWidgetImpl::NeedsFrame() const
{
    return m_needsFrame;
}

void wxWidgetImpl::SetDrawingEnabled(bool WXUNUSED(enabled))
{
}
