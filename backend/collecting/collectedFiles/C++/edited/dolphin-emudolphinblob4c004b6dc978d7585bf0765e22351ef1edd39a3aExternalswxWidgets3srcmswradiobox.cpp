


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_RADIOBOX

#include "wx/radiobox.h"

#ifndef WX_PRECOMP
    #include "wx/hashmap.h"
    #include "wx/bitmap.h"
    #include "wx/brush.h"
    #include "wx/settings.h"
    #include "wx/log.h"
#endif

#include "wx/msw/subwin.h"

#if wxUSE_TOOLTIPS
    #include "wx/tooltip.h"
#endif 
#if 0 WX_DEFINE_FLAGS( wxRadioBoxStyle )

wxBEGIN_FLAGS( wxRadioBoxStyle )
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

    wxFLAGS_MEMBER(wxRA_SPECIFY_COLS)
    wxFLAGS_MEMBER(wxRA_SPECIFY_ROWS)
wxEND_FLAGS( wxRadioBoxStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxRadioBox, wxControl, "wx/radiobox.h");

wxBEGIN_PROPERTIES_TABLE(wxRadioBox)
    wxEVENT_PROPERTY( Select , wxEVT_RADIOBOX , wxCommandEvent )
    wxPROPERTY_FLAGS( WindowStyle , wxRadioBoxStyle , long , SetWindowStyleFlag , GetWindowStyleFlag , , 0  , wxT("Helpstring") , wxT("group")) wxEND_PROPERTIES_TABLE()

#else
wxIMPLEMENT_DYNAMIC_CLASS(wxRadioBox, wxControl);
#endif




LRESULT APIENTRY
wxRadioBtnWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


namespace
{

WXFARPROC s_wndprocRadioBtn = (WXFARPROC)NULL;

WX_DECLARE_HASH_MAP(HWND, wxRadioBox *,
                    wxPointerHash, wxPointerEqual,
                    RadioBoxFromButton);

RadioBoxFromButton gs_boxFromButton;

} 


wxRadioBox* wxRadioBox::GetFromRadioButtonHWND(WXHWND hwnd)
{
    const RadioBoxFromButton::const_iterator it = gs_boxFromButton.find(hwnd);
    return it == gs_boxFromButton.end() ? NULL : it->second;
}


void wxRadioBox::Init()
{
    m_selectedButton = wxNOT_FOUND;
    m_radioButtons = NULL;
    m_dummyHwnd = NULL;
    m_radioWidth = NULL;
    m_radioHeight = NULL;
}

bool wxRadioBox::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxString& title,
                        const wxPoint& pos,
                        const wxSize& size,
                        int n,
                        const wxString choices[],
                        int majorDim,
                        long style,
                        const wxValidator& val,
                        const wxString& name)
{
        if ( !wxStaticBox::Create(parent, id, title, pos, size, style, name) )
        return false;

            if ( !(style & (wxRA_SPECIFY_ROWS | wxRA_SPECIFY_COLS)) )
        style |= wxRA_SPECIFY_COLS;

#if wxUSE_VALIDATORS
    SetValidator(val);
#else
    wxUnusedVar(val);
#endif 
                                m_radioButtons = new wxSubwindows(n);

    m_radioWidth = new int[n];
    m_radioHeight = new int[n];

    for ( int i = 0; i < n; i++ )
    {
        m_radioWidth[i] =
        m_radioHeight[i] = wxDefaultCoord;
        long styleBtn = BS_AUTORADIOBUTTON | WS_TABSTOP | WS_CHILD | WS_VISIBLE;
        if ( i == 0 )
            styleBtn |= WS_GROUP;

        wxWindowIDRef subid = NewControlId();

        HWND hwndBtn = ::CreateWindow(wxT("BUTTON"),
                                      choices[i].t_str(),
                                      styleBtn,
                                      0, 0, 0, 0,                                         GetHwndOf(parent),
                                      (HMENU)wxUIntToPtr(subid.GetValue()),
                                      wxGetInstance(),
                                      NULL);

        if ( !hwndBtn )
        {
            wxLogLastError(wxT("CreateWindow(radio btn)"));

            return false;
        }

                m_radioButtons->Set(i, hwndBtn, subid);

        SubclassRadioButton((WXHWND)hwndBtn);

                m_subControls.Add(subid);
    }

        m_dummyId = NewControlId();

    m_dummyHwnd = (WXHWND)::CreateWindow(wxT("BUTTON"),
                         wxEmptyString,
                         WS_GROUP | BS_AUTORADIOBUTTON | WS_CHILD,
                         0, 0, 0, 0, GetHwndOf(parent),
                         (HMENU)wxUIntToPtr(m_dummyId.GetValue()),
                         wxGetInstance(), NULL);


    m_radioButtons->SetFont(GetFont());

    SetMajorDim(majorDim == 0 ? n : majorDim, style);
        if ( n > 0 )
        SetSelection(0);
    SetSize(pos.x, pos.y, size.x, size.y);

        SetInitialSize(size);

        const wxSize actualSize = GetSize();
    PositionAllButtons(pos.x, pos.y, actualSize.x, actualSize.y);

                    m_container.EnableSelfFocus();

    return true;
}

bool wxRadioBox::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxString& title,
                        const wxPoint& pos,
                        const wxSize& size,
                        const wxArrayString& choices,
                        int majorDim,
                        long style,
                        const wxValidator& val,
                        const wxString& name)
{
    wxCArrayString chs(choices);
    return Create(parent, id, title, pos, size, chs.GetCount(),
                  chs.GetStrings(), majorDim, style, val, name);
}

wxRadioBox::~wxRadioBox()
{
    SendDestroyEvent();

                    for ( size_t item = 0; item < m_radioButtons->GetCount(); item++ )
    {
        HWND hwnd = m_radioButtons->Get(item);

        wxSetWindowProc(hwnd, reinterpret_cast<WNDPROC>(s_wndprocRadioBtn));
        gs_boxFromButton.erase(hwnd);
    }

    delete m_radioButtons;

    if ( m_dummyHwnd )
        DestroyWindow((HWND)m_dummyHwnd);

    delete[] m_radioWidth;
    delete[] m_radioHeight;
}

void wxRadioBox::SubclassRadioButton(WXHWND hWndBtn)
{
    HWND hwndBtn = (HWND)hWndBtn;

    if ( !s_wndprocRadioBtn )
        s_wndprocRadioBtn = (WXFARPROC)wxGetWindowProc(hwndBtn);

    wxSetWindowProc(hwndBtn, wxRadioBtnWndProc);

    gs_boxFromButton[hwndBtn] = this;
}


bool wxRadioBox::MSWCommand(WXUINT cmd, WXWORD id_)
{
    const int id = (signed short)id_;

    if ( cmd == BN_CLICKED )
    {
        if (id == GetId())
            return true;

        int selectedButton = wxNOT_FOUND;

        const unsigned int count = GetCount();
        for ( unsigned int i = 0; i < count; i++ )
        {
            const HWND hwndBtn = (*m_radioButtons)[i];
            if ( id == wxGetWindowId(hwndBtn) )
            {
                                                                if ( ::SendMessage(hwndBtn, BM_GETCHECK, 0, 0) == BST_CHECKED )
                    selectedButton = i;

                break;
            }
        }

        if ( selectedButton == wxNOT_FOUND )
        {
                                                            return false;
        }

        if ( selectedButton != m_selectedButton )
        {
            m_selectedButton = selectedButton;

            SendNotificationEvent();
        }
        
        return true;
    }
    else
        return false;
}

void wxRadioBox::Command(wxCommandEvent & event)
{
    SetSelection (event.GetInt());
    SetFocus();
    ProcessCommand(event);
}

void wxRadioBox::SendNotificationEvent()
{
    wxCommandEvent event(wxEVT_RADIOBOX, m_windowId);
    event.SetInt( m_selectedButton );
    event.SetString(GetString(m_selectedButton));
    event.SetEventObject( this );
    ProcessCommand(event);
}


unsigned int wxRadioBox::GetCount() const
{
    return m_radioButtons ? m_radioButtons->GetCount() : 0u;
}

void wxRadioBox::SetString(unsigned int item, const wxString& label)
{
    wxCHECK_RET( IsValid(item), wxT("invalid radiobox index") );

    m_radioWidth[item] =
    m_radioHeight[item] = wxDefaultCoord;

    ::SetWindowText((*m_radioButtons)[item], label.c_str());

    InvalidateBestSize();
}

void wxRadioBox::SetSelection(int N)
{
    wxCHECK_RET( IsValid(N), wxT("invalid radiobox index") );

        if ( m_selectedButton != wxNOT_FOUND )
        ::SendMessage((*m_radioButtons)[m_selectedButton], BM_SETCHECK, 0, 0L);

        ::SendMessage((*m_radioButtons)[N], BM_SETCHECK, 1, 0L);

    m_selectedButton = N;
}

wxString wxRadioBox::GetString(unsigned int item) const
{
    wxCHECK_MSG( IsValid(item), wxEmptyString,
                 wxT("invalid radiobox index") );

    return wxGetWindowText((*m_radioButtons)[item]);
}

void wxRadioBox::SetFocus()
{
    if ( GetCount() > 0 )
    {
        ::SetFocus((*m_radioButtons)[m_selectedButton == wxNOT_FOUND
                                        ? 0
                                        : m_selectedButton]);
    }
}

bool wxRadioBox::CanBeFocused() const
{
            if ( !wxStaticBox::CanBeFocused() )
        return false;

        for ( size_t item = 0; item < m_radioButtons->GetCount(); item++ )
    {
        if ( IsItemEnabled(item) && IsItemShown(item) )
            return true;
    }

            return false;
}

bool wxRadioBox::Enable(unsigned int item, bool enable)
{
    wxCHECK_MSG( IsValid(item), false,
                 wxT("invalid item in wxRadioBox::Enable()") );

    BOOL ret = MSWEnableHWND((*m_radioButtons)[item], enable);

    return (ret == 0) != enable;
}

bool wxRadioBox::IsItemEnabled(unsigned int item) const
{
    wxCHECK_MSG( IsValid(item), false,
                 wxT("invalid item in wxRadioBox::IsItemEnabled()") );

    return ::IsWindowEnabled((*m_radioButtons)[item]) != 0;
}

bool wxRadioBox::Show(unsigned int item, bool show)
{
    wxCHECK_MSG( IsValid(item), false,
                 wxT("invalid item in wxRadioBox::Show()") );

    BOOL ret = ::ShowWindow((*m_radioButtons)[item], show ? SW_SHOW : SW_HIDE);

    bool changed = (ret != 0) != show;
    if ( changed )
    {
        InvalidateBestSize();
    }

    return changed;
}

bool wxRadioBox::IsItemShown(unsigned int item) const
{
    wxCHECK_MSG( IsValid(item), false,
                 wxT("invalid item in wxRadioBox::IsItemShown()") );

                return (::GetWindowLong((*m_radioButtons)[item],
                            GWL_STYLE) & WS_VISIBLE) != 0;
}

#if wxUSE_TOOLTIPS

bool wxRadioBox::HasToolTips() const
{
    return wxStaticBox::HasToolTips() || wxRadioBoxBase::HasItemToolTips();
}

void wxRadioBox::DoSetItemToolTip(unsigned int item, wxToolTip *tooltip)
{
        const HWND hwndRbtn = (*m_radioButtons)[item];
    if ( tooltip != NULL )
        tooltip->AddOtherWindow(hwndRbtn);
    else         wxToolTip::Remove(hwndRbtn, 0, wxRect(0,0,0,0));
                }

#endif 
bool wxRadioBox::Reparent(wxWindowBase *newParent)
{
    if ( !wxStaticBox::Reparent(newParent) )
    {
        return false;
    }

    HWND hwndParent = GetHwndOf(GetParent());
    for ( size_t item = 0; item < m_radioButtons->GetCount(); item++ )
    {
        ::SetParent((*m_radioButtons)[item], hwndParent);
    }
    return true;
}

WX_FORWARD_STD_METHODS_TO_SUBWINDOWS(wxRadioBox, wxStaticBox, m_radioButtons)


wxSize wxRadioBox::GetMaxButtonSize() const
{
        int widthMax = 0,
        heightMax = 0;
    const unsigned int count = GetCount();
    for ( unsigned int i = 0 ; i < count; i++ )
    {
        int width, height;
        if ( m_radioWidth[i] < 0 )
        {
            GetTextExtent(wxGetWindowText((*m_radioButtons)[i]), &width, &height);

                                    width += FromDIP(RADIO_SIZE);
            height *= 3;
            height /= 2;
        }
        else
        {
            width = m_radioWidth[i];
            height = m_radioHeight[i];
        }

        if ( widthMax < width )
            widthMax = width;
        if ( heightMax < height )
            heightMax = height;
    }

    return wxSize(widthMax, heightMax);
}

wxSize wxRadioBox::GetTotalButtonSize(const wxSize& sizeBtn) const
{
        int cx1, cy1;
    wxGetCharSize(m_hWnd, &cx1, &cy1, GetFont());

    int extraHeight = cy1;

    int height = GetRowCount() * sizeBtn.y + cy1/2 + extraHeight;
    int width  = GetColumnCount() * (sizeBtn.x + cx1) + cx1;

        if (!wxControl::GetLabel().empty())
        height += cy1/2;

        int widthLabel;
    GetTextExtent(GetLabelText(), &widthLabel, NULL);
    widthLabel += FromDIP(RADIO_SIZE);     if ( widthLabel > width )
        width = widthLabel;

    return wxSize(width, height);
}

wxSize wxRadioBox::DoGetBestSize() const
{
    if ( !m_radioButtons )
    {
                        return wxSize(1, 1);
    }

    wxSize best = GetTotalButtonSize(GetMaxButtonSize());
    CacheBestSize(best);
    return best;
}

void wxRadioBox::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
    if ( (width == wxDefaultCoord && (sizeFlags & wxSIZE_AUTO_WIDTH)) ||
            (height == wxDefaultCoord && (sizeFlags & wxSIZE_AUTO_HEIGHT)) )
    {
                        const wxSize totSize = GetTotalButtonSize(GetMaxButtonSize());

                if ( width == wxDefaultCoord && (sizeFlags & wxSIZE_AUTO_WIDTH) )
            width = totSize.x;

        if ( height == wxDefaultCoord && (sizeFlags & wxSIZE_AUTO_HEIGHT) )
            height = totSize.y;
    }

    wxStaticBox::DoSetSize(x, y, width, height);
}

void wxRadioBox::DoMoveWindow(int x, int y, int width, int height)
{
    wxStaticBox::DoMoveWindow(x, y, width, height);

    PositionAllButtons(x, y, width, height);
}

void
wxRadioBox::PositionAllButtons(int x, int y, int width, int WXUNUSED(height))
{
    wxSize maxSize = GetMaxButtonSize();
    int maxWidth = maxSize.x,
        maxHeight = maxSize.y;

                    
                
    int cx1, cy1;
    wxGetCharSize(m_hWnd, &cx1, &cy1, GetFont());

    int x_offset = x + cx1;
    int y_offset = y + cy1;

        if (!wxControl::GetLabel().empty())
        y_offset += cy1/2;

    int startX = x_offset;
    int startY = y_offset;

    const unsigned int count = GetCount();
    for (unsigned int i = 0; i < count; i++)
    {
                                bool isLastInTheRow;
        if ( m_windowStyle & wxRA_SPECIFY_COLS )
        {
                                    unsigned int n = i + 1;
            isLastInTheRow = ((n % GetMajorDim()) == 0) || (n == count);
        }
        else         {
                        isLastInTheRow = i >= (count/GetMajorDim())*GetMajorDim();
        }

                if ( i && (i % GetMajorDim() == 0) )
        {
            if ( m_windowStyle & wxRA_SPECIFY_ROWS )
            {
                                y_offset = startY;
                x_offset += maxWidth + cx1;
            }
            else             {
                x_offset = startX;
                y_offset += maxHeight;
                if (m_radioWidth[0]>0)
                    y_offset += cy1/2;
            }
        }

        int widthBtn;
        if ( isLastInTheRow )
        {
                        widthBtn = startX + width - x_offset - 2*cx1;
            if ( widthBtn < maxWidth )
                widthBtn = maxWidth;
        }
        else
        {
                        widthBtn = maxWidth;
        }

                                        DoMoveSibling((*m_radioButtons)[i], x_offset, y_offset, widthBtn, maxHeight);

                if ( m_windowStyle & wxRA_SPECIFY_ROWS )
        {
                        y_offset += maxHeight;
            if (m_radioWidth[0]>0)
                y_offset += cy1/2;
        }
        else
        {
                        x_offset += widthBtn + cx1;
        }
    }
}

int wxRadioBox::GetItemFromPoint(const wxPoint& pt) const
{
    const unsigned int count = GetCount();
    for ( unsigned int i = 0; i < count; i++ )
    {
        RECT rect = wxGetWindowRect((*m_radioButtons)[i]);

        if ( rect.left <= pt.x && pt.x < rect.right &&
                rect.top  <= pt.y && pt.y < rect.bottom )
        {
            return i;
        }
    }

    return wxNOT_FOUND;
}


WXHRGN wxRadioBox::MSWGetRegionWithoutChildren()
{
    RECT rc;
    ::GetWindowRect(GetHwnd(), &rc);
    HRGN hrgn = ::CreateRectRgn(rc.left, rc.top, rc.right + 1, rc.bottom + 1);

    const unsigned int count = GetCount();
    for ( unsigned int i = 0; i < count; ++i )
    {
                if ( !IsItemShown(i) )
            continue;

        ::GetWindowRect((*m_radioButtons)[i], &rc);
        AutoHRGN hrgnchild(::CreateRectRgnIndirect(&rc));
        ::CombineRgn(hrgn, hrgn, hrgnchild, RGN_DIFF);
    }

    return (WXHRGN)hrgn;
}


LRESULT APIENTRY
wxRadioBtnWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    wxRadioBox * const radiobox = wxRadioBox::GetFromRadioButtonHWND(hwnd);
    wxCHECK_MSG( radiobox, 0, wxT("Should have the associated radio box") );

    switch ( message )
    {
        case WM_GETDLGCODE:
                                                {
                long lDlgCode = ::CallWindowProc(CASTWNDPROC s_wndprocRadioBtn, hwnd,
                                                 message, wParam, lParam);

                return lDlgCode | DLGC_WANTARROWS;
            }

        case WM_KEYDOWN:
            {
                bool processed = true;

                wxDirection dir;
                switch ( wParam )
                {
                    case VK_UP:
                        dir = wxUP;
                        break;

                    case VK_LEFT:
                        dir = wxLEFT;
                        break;

                    case VK_DOWN:
                        dir = wxDOWN;
                        break;

                    case VK_RIGHT:
                        dir = wxRIGHT;
                        break;

                    default:
                        processed = false;

                                                dir = wxALL;
                }

                if ( processed )
                {
                    int selOld = radiobox->GetSelection();
                    int selNew = radiobox->GetNextItem
                                 (
                                  selOld,
                                  dir,
                                  radiobox->GetWindowStyle()
                                 );

                    if ( selNew != selOld )
                    {
                        radiobox->SetSelection(selNew);
                        radiobox->SetFocus();

                                                radiobox->SendNotificationEvent();

                        return 0;
                    }
                }
            }
            break;

        case WM_SETFOCUS:
        case WM_KILLFOCUS:
            {
                                                                                if ( message == WM_SETFOCUS )
                    radiobox->HandleSetFocus((WXHWND)wParam);
                else
                    radiobox->HandleKillFocus((WXHWND)wParam);
            }
            break;

        case WM_HELP:
            {
                bool processed = false;

                wxEvtHandler * const handler = radiobox->GetEventHandler();

                HELPINFO* info = (HELPINFO*) lParam;
                if ( info->iContextType == HELPINFO_WINDOW )
                {
                    for ( wxWindow* subjectOfHelp = radiobox;
                          subjectOfHelp;
                          subjectOfHelp = subjectOfHelp->GetParent() )
                    {
                        wxHelpEvent helpEvent(wxEVT_HELP,
                                              subjectOfHelp->GetId(),
                                              wxPoint(info->MousePos.x,
                                                      info->MousePos.y));
                        helpEvent.SetEventObject(radiobox);
                        if ( handler->ProcessEvent(helpEvent) )
                        {
                            processed = true;
                            break;
                        }
                    }
                }
                else if (info->iContextType == HELPINFO_MENUITEM)
                {
                    wxHelpEvent helpEvent(wxEVT_HELP, info->iCtrlId);
                    helpEvent.SetEventObject(radiobox);
                    processed = handler->ProcessEvent(helpEvent);
                }

                if ( processed )
                    return 0;
            }
            break;
    }

    return ::CallWindowProc(CASTWNDPROC s_wndprocRadioBtn, hwnd, message, wParam, lParam);
}

#endif 