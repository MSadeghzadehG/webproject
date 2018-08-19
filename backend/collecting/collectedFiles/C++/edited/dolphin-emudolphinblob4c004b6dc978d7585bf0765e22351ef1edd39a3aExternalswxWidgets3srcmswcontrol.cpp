


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_CONTROLS

#include "wx/control.h"

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcctl.h"     #include "wx/event.h"
    #include "wx/app.h"
    #include "wx/dcclient.h"
    #include "wx/log.h"
    #include "wx/settings.h"
    #include "wx/ctrlsub.h"
#endif

#if wxUSE_LISTCTRL
    #include "wx/listctrl.h"
#endif 
#if wxUSE_TREECTRL
    #include "wx/treectrl.h"
#endif 
#include "wx/renderer.h"
#include "wx/msw/private.h"
#include "wx/msw/uxtheme.h"
#include "wx/msw/dc.h"          #include "wx/msw/ownerdrawnbutton.h"

#ifndef BS_TYPEMASK
#define BS_TYPEMASK 0xf
#endif


wxIMPLEMENT_ABSTRACT_CLASS(wxControl, wxWindow);



bool wxControl::Create(wxWindow *parent,
                       wxWindowID id,
                       const wxPoint& pos,
                       const wxSize& size,
                       long style,
                       const wxValidator& wxVALIDATOR_PARAM(validator),
                       const wxString& name)
{
    if ( !wxWindow::Create(parent, id, pos, size, style, name) )
        return false;

#if wxUSE_VALIDATORS
    SetValidator(validator);
#endif

    return true;
}

bool wxControl::MSWCreateControl(const wxChar *classname,
                                 const wxString& label,
                                 const wxPoint& pos,
                                 const wxSize& size)
{
    WXDWORD exstyle;
    WXDWORD msStyle = MSWGetStyle(GetWindowStyle(), &exstyle);

    return MSWCreateControl(classname, msStyle, pos, size, label, exstyle);
}

bool wxControl::MSWCreateControl(const wxChar *classname,
                                 WXDWORD style,
                                 const wxPoint& pos,
                                 const wxSize& size,
                                 const wxString& label,
                                 WXDWORD exstyle)
{
        if ( exstyle == (WXDWORD)-1 )
    {
        exstyle = 0;
        (void) MSWGetStyle(GetWindowStyle(), &exstyle);
    }

        style |= WS_CHILD;

        if ( m_isShown )
    {
        style |= WS_VISIBLE;
    }

                        int x = pos.x == wxDefaultCoord ? 0 : pos.x,
        y = pos.y == wxDefaultCoord ? 0 : pos.y,
        w = size.x == wxDefaultCoord ? 1 : size.x,
        h = size.y == wxDefaultCoord ? 1 : size.y;

        AdjustForParentClientOrigin(x, y);

    m_hWnd = (WXHWND)::CreateWindowEx
                       (
                        exstyle,                                    classname,                                  label.t_str(),                              style,                                      x, y, w, h,                                 GetHwndOf(GetParent()),                                 (HMENU)wxUIntToPtr(GetId()),                            wxGetInstance(),                            NULL                                       );

    if ( !m_hWnd )
    {
        wxLogLastError(wxString::Format
                       (
                        wxT("CreateWindowEx(\"%s\", flags=%08lx, ex=%08lx)"),
                        classname, style, exstyle
                       ));

        return false;
    }

#if !wxUSE_UNICODE
                                        if ( !label.empty() && label[0] == -1 )
        ::SetWindowText(GetHwnd(), label.t_str());
#endif 
            m_labelOrig = label;

        SubclassWin(m_hWnd);

        InheritAttributes();
    if ( !m_hasFont )
    {
        bool setFont = true;

        wxFont font = GetDefaultAttributes().font;

                        
        #if wxUSE_LISTCTRL || wxUSE_TREECTRL
        bool testFont = false;
#if wxUSE_LISTCTRL
        if ( wxDynamicCastThis(wxListCtrl) )
            testFont = true;
#endif #if wxUSE_TREECTRL
        if ( wxDynamicCastThis(wxTreeCtrl) )
            testFont = true;
#endif 
        if ( testFont )
        {
                                                
                                    if ( font != wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT) )
            {
                setFont = false;
            }
        }
#endif 
        if ( setFont )
        {
            SetFont(GetDefaultAttributes().font);
        }
    }

        SetInitialSize(size);

    return true;
}


WXDWORD wxControl::MSWGetStyle(long style, WXDWORD *exstyle) const
{
    long msStyle = wxWindow::MSWGetStyle(style, exstyle);

    if ( AcceptsFocusFromKeyboard() )
    {
        msStyle |= WS_TABSTOP;
    }

    return msStyle;
}

wxSize wxControl::DoGetBestSize() const
{
    if (m_windowSizer)
       return wxControlBase::DoGetBestSize();

    return FromDIP(wxSize(DEFAULT_ITEM_WIDTH, DEFAULT_ITEM_HEIGHT));
}

wxBorder wxControl::GetDefaultBorder() const
{
    return wxControlBase::GetDefaultBorder();
}

 wxVisualAttributes
wxControl::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    wxVisualAttributes attrs;

            attrs.font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);

                attrs.colFg = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNTEXT);
    attrs.colBg = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);

    return attrs;
}


bool wxControl::ProcessCommand(wxCommandEvent& event)
{
    return HandleWindowEvent(event);
}

bool wxControl::MSWOnNotify(int idCtrl,
                            WXLPARAM lParam,
                            WXLPARAM* result)
{
    wxEventType eventType wxDUMMY_INITIALIZE(wxEVT_NULL);

    NMHDR *hdr = (NMHDR*) lParam;
    switch ( hdr->code )
    {
        case NM_CLICK:
            eventType = wxEVT_COMMAND_LEFT_CLICK;
            break;

        case NM_DBLCLK:
            eventType = wxEVT_COMMAND_LEFT_DCLICK;
            break;

        case NM_RCLICK:
            eventType = wxEVT_COMMAND_RIGHT_CLICK;
            break;

        case NM_RDBLCLK:
            eventType = wxEVT_COMMAND_RIGHT_DCLICK;
            break;

        case NM_SETFOCUS:
            eventType = wxEVT_COMMAND_SET_FOCUS;
            break;

        case NM_KILLFOCUS:
            eventType = wxEVT_COMMAND_KILL_FOCUS;
            break;

        case NM_RETURN:
            eventType = wxEVT_COMMAND_ENTER;
            break;

        default:
            return wxWindow::MSWOnNotify(idCtrl, lParam, result);
    }

    wxCommandEvent event(wxEVT_NULL, m_windowId);
    event.SetEventType(eventType);
    event.SetEventObject(this);

    return HandleWindowEvent(event);
}

WXHBRUSH wxControl::DoMSWControlColor(WXHDC pDC, wxColour colBg, WXHWND hWnd)
{
    HDC hdc = (HDC)pDC;

    WXHBRUSH hbr = 0;
    if ( !colBg.IsOk() )
    {
        wxWindow *win = wxFindWinFromHandle( hWnd );
        if ( !win )
        {
                                                                                                                        if ( ContainsHWND(hWnd) )
            {
                win = this;
            }
            else             {
                HWND parent = ::GetParent(hWnd);
                if ( parent )
                {
                    wxWindow *winParent = wxFindWinFromHandle( parent );
                    if( winParent && winParent->ContainsHWND( hWnd ) )
                        win = winParent;
                 }
            }
        }

        if ( win )
            hbr = win->MSWGetBgBrush(pDC);

                                if ( !hbr && m_hasFgCol )
            colBg = GetBackgroundColour();
    }

                if ( colBg.IsOk() )
    {
        wxBrush *brush = wxTheBrushList->FindOrCreateBrush(colBg);
        hbr = (WXHBRUSH)brush->GetResourceHandle();
    }

                    if ( hbr )
    {
        ::SetTextColor(hdc, wxColourToRGB(GetForegroundColour()));
    }

                if ( colBg.IsOk() || m_hasBgCol )
    {
        if ( !colBg.IsOk() )
            colBg = GetBackgroundColour();

        ::SetBkColor(hdc, wxColourToRGB(colBg));
    }

    return hbr;
}

WXHBRUSH wxControl::MSWControlColor(WXHDC pDC, WXHWND hWnd)
{
    if ( HasTransparentBackground() )
        ::SetBkMode((HDC)pDC, TRANSPARENT);

                return DoMSWControlColor(pDC, wxColour(), hWnd);
}

WXHBRUSH wxControl::MSWControlColorDisabled(WXHDC pDC)
{
    return DoMSWControlColor(pDC,
                             wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE),
                             GetHWND());
}

wxWindow* wxControl::MSWFindItem(long id, WXHWND hWnd) const
{
        if ( id == GetId() || (GetSubcontrols().Index(id) != wxNOT_FOUND) )
        return const_cast<wxControl *>(this);

    return wxControlBase::MSWFindItem(id, hWnd);
}


void
wxMSWOwnerDrawnButtonBase::MSWMakeOwnerDrawnIfNecessary(const wxColour& colFg)
{
            if ( wxUxThemeEngine::GetIfActive() )
        MSWMakeOwnerDrawn(colFg.IsOk());
}

bool wxMSWOwnerDrawnButtonBase::MSWIsOwnerDrawn() const
{
    return
        (::GetWindowLong(GetHwndOf(m_win), GWL_STYLE) & BS_OWNERDRAW) == BS_OWNERDRAW;
}

void wxMSWOwnerDrawnButtonBase::MSWMakeOwnerDrawn(bool ownerDrawn)
{
    long style = ::GetWindowLong(GetHwndOf(m_win), GWL_STYLE);

            if ( ownerDrawn )
    {
        style &= ~BS_TYPEMASK;
        style |= BS_OWNERDRAW;

        m_win->Bind(wxEVT_ENTER_WINDOW,
                    &wxMSWOwnerDrawnButtonBase::OnMouseEnterOrLeave, this);
        m_win->Bind(wxEVT_LEAVE_WINDOW,
                    &wxMSWOwnerDrawnButtonBase::OnMouseEnterOrLeave, this);

        m_win->Bind(wxEVT_LEFT_DOWN,
                    &wxMSWOwnerDrawnButtonBase::OnMouseLeft, this);
        m_win->Bind(wxEVT_LEFT_UP,
                    &wxMSWOwnerDrawnButtonBase::OnMouseLeft, this);

        m_win->Bind(wxEVT_SET_FOCUS,
                    &wxMSWOwnerDrawnButtonBase::OnFocus, this);

        m_win->Bind(wxEVT_KILL_FOCUS,
                    &wxMSWOwnerDrawnButtonBase::OnFocus, this);
    }
    else     {
        style &= ~BS_OWNERDRAW;
        style |= MSWGetButtonStyle();

        m_win->Unbind(wxEVT_ENTER_WINDOW,
                      &wxMSWOwnerDrawnButtonBase::OnMouseEnterOrLeave, this);
        m_win->Unbind(wxEVT_LEAVE_WINDOW,
                      &wxMSWOwnerDrawnButtonBase::OnMouseEnterOrLeave, this);

        m_win->Unbind(wxEVT_LEFT_DOWN,
                      &wxMSWOwnerDrawnButtonBase::OnMouseLeft, this);
        m_win->Unbind(wxEVT_LEFT_UP,
                      &wxMSWOwnerDrawnButtonBase::OnMouseLeft, this);

        m_win->Unbind(wxEVT_SET_FOCUS,
                      &wxMSWOwnerDrawnButtonBase::OnFocus, this);
        m_win->Unbind(wxEVT_KILL_FOCUS,
                      &wxMSWOwnerDrawnButtonBase::OnFocus, this);
    }

    ::SetWindowLong(GetHwndOf(m_win), GWL_STYLE, style);

    if ( !ownerDrawn )
        MSWOnButtonResetOwnerDrawn();
}

void wxMSWOwnerDrawnButtonBase::OnMouseEnterOrLeave(wxMouseEvent& event)
{
    if ( event.GetEventType() == wxEVT_LEAVE_WINDOW )
        m_isPressed = false;

    m_win->Refresh();

    event.Skip();
}

void wxMSWOwnerDrawnButtonBase::OnMouseLeft(wxMouseEvent& event)
{
                m_isPressed = event.GetEventType() == wxEVT_LEFT_DOWN;
    m_win->Refresh();

    event.Skip();
}

void wxMSWOwnerDrawnButtonBase::OnFocus(wxFocusEvent& event)
{
    m_win->Refresh();

    event.Skip();
}

bool wxMSWOwnerDrawnButtonBase::MSWDrawButton(WXDRAWITEMSTRUCT *item)
{
    DRAWITEMSTRUCT *dis = (DRAWITEMSTRUCT *)item;

    if ( !MSWIsOwnerDrawn() || dis->CtlType != ODT_BUTTON )
        return false;

        const bool isFocused = m_isPressed || m_win->HasFocus();

    int flags = MSWGetButtonCheckedFlag();

    if ( dis->itemState & ODS_SELECTED )
        flags |= wxCONTROL_SELECTED | wxCONTROL_PRESSED;

    if ( !m_win->IsEnabled() )
        flags |= wxCONTROL_DISABLED;

    if ( m_isPressed )
        flags |= wxCONTROL_PRESSED;

    if ( wxFindWindowAtPoint(wxGetMousePosition()) == m_win )
        flags |= wxCONTROL_CURRENT;


        HDC hdc = dis->hDC;
    const RECT& rect = dis->rcItem;

        const wxSize bestSize = m_win->GetBestSize();
    RECT rectButton,
         rectLabel;
    rectLabel.top = rect.top + (rect.bottom - rect.top - bestSize.y) / 2;
    rectLabel.bottom = rectLabel.top + bestSize.y;

            static const int MARGIN = 3;
    int CXMENUCHECK = ::GetSystemMetrics(SM_CXMENUCHECK) + 1;

        if ( wxGetWinVersion() < wxWinVersion_6 )
        CXMENUCHECK += 2;

            const int buttonSize = wxMin(CXMENUCHECK - MARGIN, m_win->GetSize().y);
    rectButton.top = rect.top + (rect.bottom - rect.top - buttonSize) / 2;
    rectButton.bottom = rectButton.top + buttonSize;

    const bool isRightAligned = m_win->HasFlag(wxALIGN_RIGHT);
    if ( isRightAligned )
    {
        rectLabel.right = rect.right - CXMENUCHECK;
        rectLabel.left = rect.left;

        rectButton.left = rectLabel.right + ( CXMENUCHECK + MARGIN - buttonSize ) / 2;
        rectButton.right = rectButton.left + buttonSize;
    }
    else     {
        rectButton.left = rect.left + ( CXMENUCHECK - MARGIN - buttonSize ) / 2;
        rectButton.right = rectButton.left + buttonSize;

        rectLabel.left = rect.left + CXMENUCHECK;
        rectLabel.right = rect.right;
    }

        ::FillRect(hdc, &rect, m_win->MSWGetBgBrush(hdc));

        wxDCTemp dc(hdc);

    MSWDrawButtonBitmap(dc, wxRectFromRECT(rectButton), flags);

        const wxString& label = m_win->GetLabel();

        UINT fmt = DT_NOCLIP;

            if ( isFocused )
        fmt |= DT_HIDEPREFIX;
    if ( isRightAligned )
        fmt |= DT_RIGHT;
    
            if ( isFocused )
    {
        RECT oldLabelRect = rectLabel; 
        if ( !::DrawText(hdc, label.t_str(), label.length(), &rectLabel,
                         fmt | DT_CALCRECT) )
        {
            wxLogLastError(wxT("DrawText(DT_CALCRECT)"));
        }

        if ( isRightAligned )
        {
                        const int labelWidth = rectLabel.right - rectLabel.left;
            rectLabel.right = oldLabelRect.right;
            rectLabel.left = rectLabel.right - labelWidth;
        }
    }

    if ( flags & wxCONTROL_DISABLED )
    {
        ::SetTextColor(hdc, ::GetSysColor(COLOR_GRAYTEXT));
    }

    if ( !::DrawText(hdc, label.t_str(), label.length(), &rectLabel, fmt) )
    {
        wxLogLastError(wxT("DrawText()"));
    }

        if ( isFocused )
    {
        rectLabel.left--;
        rectLabel.right++;
        if ( !::DrawFocusRect(hdc, &rectLabel) )
        {
            wxLogLastError(wxT("DrawFocusRect()"));
        }
    }

    return true;
}


void wxControlWithItems::MSWAllocStorage(const wxArrayStringsAdapter& items,
                                         unsigned wm)
{
    const unsigned numItems = items.GetCount();
    unsigned long totalTextLength = numItems;     for ( unsigned i = 0; i < numItems; ++i )
    {
        totalTextLength += items[i].length();
    }

    if ( SendMessage((HWND)MSWGetItemsHWND(), wm, numItems,
                     (LPARAM)totalTextLength*sizeof(wxChar)) == LB_ERRSPACE )
    {
        wxLogLastError(wxT("SendMessage(XX_INITSTORAGE)"));
    }
}

int wxControlWithItems::MSWInsertOrAppendItem(unsigned pos,
                                              const wxString& item,
                                              unsigned wm)
{
    LRESULT n = SendMessage((HWND)MSWGetItemsHWND(), wm, pos,
                            wxMSW_CONV_LPARAM(item));
    if ( n == CB_ERR || n == CB_ERRSPACE )
    {
        wxLogLastError(wxT("SendMessage(XX_ADD/INSERTSTRING)"));
        return wxNOT_FOUND;
    }

    return n;
}


void wxFindMaxSize(WXHWND wnd, RECT *rect)
{
    int left = rect->left;
    int right = rect->right;
    int top = rect->top;
    int bottom = rect->bottom;

    GetWindowRect((HWND) wnd, rect);

    if (left < 0)
        return;

    if (left < rect->left)
        rect->left = left;

    if (right > rect->right)
        rect->right = right;

    if (top < rect->top)
        rect->top = top;

    if (bottom > rect->bottom)
        rect->bottom = bottom;
}

#endif 