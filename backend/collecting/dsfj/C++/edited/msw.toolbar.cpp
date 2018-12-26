


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_TOOLBAR && wxUSE_TOOLBAR_NATIVE

#include "wx/toolbar.h"

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcctl.h"     #include "wx/dynarray.h"
    #include "wx/frame.h"
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/settings.h"
    #include "wx/bitmap.h"
    #include "wx/region.h"
    #include "wx/dcmemory.h"
    #include "wx/control.h"
    #include "wx/app.h"             #include "wx/image.h"
    #include "wx/stattext.h"
#endif

#include "wx/artprov.h"
#include "wx/sysopt.h"
#include "wx/dcclient.h"
#include "wx/scopedarray.h"

#include "wx/msw/private.h"
#include "wx/msw/dc.h"

#if wxUSE_UXTHEME
#include "wx/msw/uxtheme.h"
#endif



#ifndef TBSTYLE_FLAT
    #define TBSTYLE_LIST            0x1000
    #define TBSTYLE_FLAT            0x0800
#endif

#ifndef TBSTYLE_TRANSPARENT
    #define TBSTYLE_TRANSPARENT     0x8000
#endif

#ifndef TBSTYLE_TOOLTIPS
    #define TBSTYLE_TOOLTIPS        0x0100
#endif

#ifndef TB_GETSTYLE
    #define TB_SETSTYLE             (WM_USER + 56)
    #define TB_GETSTYLE             (WM_USER + 57)
#endif

#ifndef TB_HITTEST
    #define TB_HITTEST              (WM_USER + 69)
#endif

#ifndef TB_GETMAXSIZE
    #define TB_GETMAXSIZE           (WM_USER + 83)
#endif


wxIMPLEMENT_DYNAMIC_CLASS(wxToolBar, wxControl);



wxBEGIN_EVENT_TABLE(wxToolBar, wxToolBarBase)
    EVT_MOUSE_EVENTS(wxToolBar::OnMouseEvent)
    EVT_SYS_COLOUR_CHANGED(wxToolBar::OnSysColourChanged)
    EVT_ERASE_BACKGROUND(wxToolBar::OnEraseBackground)
wxEND_EVENT_TABLE()


class wxToolBarTool : public wxToolBarToolBase
{
public:
    wxToolBarTool(wxToolBar *tbar,
                  int id,
                  const wxString& label,
                  const wxBitmap& bmpNormal,
                  const wxBitmap& bmpDisabled,
                  wxItemKind kind,
                  wxObject *clientData,
                  const wxString& shortHelp,
                  const wxString& longHelp)
        : wxToolBarToolBase(tbar, id, label, bmpNormal, bmpDisabled, kind,
                            clientData, shortHelp, longHelp)
    {
        m_staticText = NULL;
        m_toBeDeleted  = false;
    }

    wxToolBarTool(wxToolBar *tbar, wxControl *control, const wxString& label)
        : wxToolBarToolBase(tbar, control, label)
    {
        if ( IsControl() && !m_label.empty() )
        {
                        m_staticText = new wxStaticText
                               (
                                 m_tbar,
                                 wxID_ANY,
                                 m_label,
                                 wxDefaultPosition,
                                 wxDefaultSize,
                                 wxALIGN_CENTRE | wxST_NO_AUTORESIZE
                               );
        }
        else         {
            m_staticText = NULL;
        }

        m_toBeDeleted  = false;
    }

    virtual ~wxToolBarTool()
    {
        delete m_staticText;
    }

    virtual void SetLabel(const wxString& label)
    {
        if ( label == m_label )
            return;

        wxToolBarToolBase::SetLabel(label);

        if ( m_staticText )
            m_staticText->SetLabel(label);

                                    }

    wxStaticText* GetStaticText()
    {
        wxASSERT_MSG( IsControl(),
                      wxT("only makes sense for embedded control tools") );

        return m_staticText;
    }

            void AllocSpacerId()
    {
        if ( m_id == wxID_SEPARATOR )
            m_id = wxWindow::NewControlId();
    }

            void MoveBy(int offset)
    {
        wxControl * const control = GetControl();

        control->Move(control->GetPosition().x + offset, wxDefaultCoord);

        if ( m_staticText )
        {
            m_staticText->Move(m_staticText->GetPosition().x + offset,
                               wxDefaultCoord);
        }
    }

    void ToBeDeleted(bool toBeDeleted = true) { m_toBeDeleted = toBeDeleted; }
    bool IsToBeDeleted() const { return m_toBeDeleted; }

private:
    wxStaticText *m_staticText;
    bool m_toBeDeleted;

    wxDECLARE_NO_COPY_CLASS(wxToolBarTool);
};


static RECT wxGetTBItemRect(HWND hwnd, int index, int id = wxID_NONE)
{
    RECT r;

            if ( !::SendMessage(hwnd, TB_GETITEMRECT, index, (LPARAM)&r) )
    {
                        const DWORD err = ::GetLastError();
        if ( err != ERROR_SUCCESS )
        {
            bool reportError = true;

            if ( id != wxID_NONE )
            {
                const LRESULT state = ::SendMessage(hwnd, TB_GETSTATE, id, 0);
                if ( state != -1 && (state & TBSTATE_HIDDEN) )
                {
                                        reportError = false;
                }
                else                 {
                                                            ::SetLastError(err);
                }
            }

            if ( reportError )
                wxLogLastError(wxT("TB_GETITEMRECT"));
        }

        ::SetRectEmpty(&r);
    }

    return r;
}

inline bool MSWShouldBeChecked(const wxToolBarToolBase *tool)
{
                    if ( wxGetWinVersion() <= wxWinVersion_7 &&
            tool->GetKind() == wxITEM_CHECK &&
                tool->GetDisabledBitmap().IsOk() &&
                    !tool->IsEnabled() )
    {
        return false;
    }

    return tool->IsToggled();
}



wxToolBarToolBase *wxToolBar::CreateTool(int id,
                                         const wxString& label,
                                         const wxBitmap& bmpNormal,
                                         const wxBitmap& bmpDisabled,
                                         wxItemKind kind,
                                         wxObject *clientData,
                                         const wxString& shortHelp,
                                         const wxString& longHelp)
{
    return new wxToolBarTool(this, id, label, bmpNormal, bmpDisabled, kind,
                             clientData, shortHelp, longHelp);
}

wxToolBarToolBase *
wxToolBar::CreateTool(wxControl *control, const wxString& label)
{
    return new wxToolBarTool(this, control, label);
}


void wxToolBar::Init()
{
    m_hBitmap = 0;
    m_disabledImgList = NULL;

    m_nButtons = 0;
    m_totalFixedSize = 0;

                        m_defaultWidth = 16;
    m_defaultHeight = 15;

    m_pInTool = NULL;
}

bool wxToolBar::Create(wxWindow *parent,
                       wxWindowID id,
                       const wxPoint& pos,
                       const wxSize& size,
                       long style,
                       const wxString& name)
{
        if ( !CreateControl(parent, id, pos, size, style, wxDefaultValidator, name) )
        return false;

    FixupStyle();

        if ( !MSWCreateToolbar(pos, size) )
        return false;

    wxSetCCUnicodeFormat(GetHwnd());

                    if (wxApp::GetComCtl32Version() >= 600)
    {
        SetBackgroundStyle(wxBG_STYLE_PAINT);
    }

    return true;
}

void wxToolBar::MSWSetPadding(WXWORD padding)
{
    DWORD curPadding = ::SendMessage(GetHwnd(), TB_GETPADDING, 0, 0);
        DWORD newPadding = IsVertical() ? MAKELPARAM(LOWORD(curPadding), padding)
                                    : MAKELPARAM(padding, HIWORD(curPadding));
    ::SendMessage(GetHwnd(), TB_SETPADDING, 0, newPadding);
}

bool wxToolBar::MSWCreateToolbar(const wxPoint& pos, const wxSize& size)
{
    if ( !MSWCreateControl(TOOLBARCLASSNAME, wxEmptyString, pos, size) )
        return false;

        ::SendMessage(GetHwnd(), TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);

#ifdef TB_SETEXTENDEDSTYLE
    ::SendMessage(GetHwnd(), TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
#endif

        if ( m_toolPacking <= 0 )
    {
                DWORD padding = ::SendMessage(GetHwnd(), TB_GETPADDING, 0, 0);
        m_toolPacking = IsVertical() ? HIWORD(padding) : LOWORD(padding);
    }
    else
    {
                MSWSetPadding(m_toolPacking);
    }

    return true;
}

void wxToolBar::Recreate()
{
    const HWND hwndOld = GetHwnd();
    if ( !hwndOld )
    {
                return;
    }

        const wxPoint pos = GetPosition();
    const wxSize size = GetSize();

    UnsubclassWin();

    if ( !MSWCreateToolbar(pos, size) )
    {
                wxFAIL_MSG( wxT("recreating the toolbar failed") );

        return;
    }

        for ( wxWindowList::compatibility_iterator node = m_children.GetFirst();
          node;
          node = node->GetNext() )
    {
        wxWindow *win = node->GetData();
        if ( !win->IsTopLevel() )
            ::SetParent(GetHwndOf(win), GetHwnd());
    }

            ::DestroyWindow(hwndOld);

        if ( m_hBitmap )
    {
        ::DeleteObject((HBITMAP) m_hBitmap);
        m_hBitmap = 0;
    }

    wxDELETE(m_disabledImgList);

    Realize();
}

wxToolBar::~wxToolBar()
{
            SendSizeEventToParent();

    if ( m_hBitmap )
        ::DeleteObject((HBITMAP) m_hBitmap);

    delete m_disabledImgList;
}

wxSize wxToolBar::DoGetBestSize() const
{
    wxSize sizeBest;

    SIZE size;
    if ( !::SendMessage(GetHwnd(), TB_GETMAXSIZE, 0, (LPARAM)&size) )
    {
                        sizeBest = GetToolSize();
        sizeBest.y += 2 * ::GetSystemMetrics(SM_CYBORDER);         sizeBest.x *= GetToolsCount();

                if ( IsVertical() )
        {
            wxSwap(sizeBest.x, sizeBest.y);
        }
    }
    else     {
                                if ( !IsVertical() && wxApp::GetComCtl32Version() < 600 )
        {
                        const RECT rcFirst = wxGetTBItemRect(GetHwnd(), 0);
            const RECT rcLast = wxGetTBItemRect(GetHwnd(), GetToolsCount() - 1);

            const int widthAlt = rcLast.right - rcFirst.left;
            if ( widthAlt > size.cx )
                size.cx = widthAlt;
        }

        sizeBest.x = size.cx;
        sizeBest.y = size.cy;
    }

    if ( !IsVertical() )
    {
        wxToolBarToolsList::compatibility_iterator node;
        for ( node = m_tools.GetFirst(); node; node = node->GetNext() )
        {
            wxToolBarTool * const
                tool = static_cast<wxToolBarTool *>(node->GetData());
            if (tool->IsControl())
            {
                int y = tool->GetControl()->GetSize().y;
                                if (y > (sizeBest.y - 4))
                    sizeBest.y = y + 4;
            }
        }

                                        if (!(GetWindowStyle() & wxTB_NODIVIDER))
            sizeBest.y += 2;
        sizeBest.y ++;
    }

    CacheBestSize(sizeBest);

    return sizeBest;
}

WXDWORD wxToolBar::MSWGetStyle(long style, WXDWORD *exstyle) const
{
            WXDWORD msStyle = wxControl::MSWGetStyle
                      (
                        (style & ~wxBORDER_MASK) | wxBORDER_NONE, exstyle
                      );

    if ( !(style & wxTB_NO_TOOLTIPS) )
        msStyle |= TBSTYLE_TOOLTIPS;

    if ( style & wxTB_FLAT )
        msStyle |= TBSTYLE_FLAT;

    if ( style & wxTB_HORZ_LAYOUT )
        msStyle |= TBSTYLE_LIST;

    if ( style & wxTB_NODIVIDER )
        msStyle |= CCS_NODIVIDER;

    if ( style & wxTB_NOALIGN )
        msStyle |= CCS_NOPARENTALIGN;

    if ( style & wxTB_VERTICAL )
        msStyle |= CCS_VERT;

    if( style & wxTB_BOTTOM )
        msStyle |= CCS_BOTTOM;

    if ( style & wxTB_RIGHT )
        msStyle |= CCS_RIGHT;

                                msStyle |= TBSTYLE_TRANSPARENT;

    return msStyle;
}


bool wxToolBar::DoInsertTool(size_t WXUNUSED(pos),
                             wxToolBarToolBase *tool)
{
                static_cast<wxToolBarTool*>(tool)->ToBeDeleted(false);

            InvalidateBestSize();
    return true;
}

bool wxToolBar::DoDeleteTool(size_t pos, wxToolBarToolBase *tool)
{
        const RECT r = wxGetTBItemRect(GetHwnd(), pos);

    int delta = IsVertical() ? r.bottom - r.top : r.right - r.left;

    m_totalFixedSize -= delta;

        m_nButtons--;
    if ( !::SendMessage(GetHwnd(), TB_DELETEBUTTON, pos, 0) )
    {
        wxLogLastError(wxT("TB_DELETEBUTTON"));

        return false;
    }

    static_cast<wxToolBarTool*>(tool)->ToBeDeleted();

    
        wxToolBarToolsList::compatibility_iterator node;
    for ( node = m_tools.Find(tool); node; node = node->GetNext() )
    {
        wxToolBarTool * const ctool = static_cast<wxToolBarTool*>(node->GetData());

        if ( ctool->IsToBeDeleted() )
            continue;

        if ( ctool->IsControl() )
        {
            ctool->MoveBy(-delta);
        }
    }

        UpdateStretchableSpacersSize();

    InvalidateBestSize();

    return true;
}

void wxToolBar::CreateDisabledImageList()
{
    wxDELETE(m_disabledImgList);

        for ( wxToolBarToolsList::compatibility_iterator
            node = m_tools.GetFirst(); node; node = node->GetNext() )
    {
        wxToolBarToolBase *tool = node->GetData();
        wxBitmap bmpDisabled = tool->GetDisabledBitmap();
        if ( bmpDisabled.IsOk() )
        {
            const wxSize sizeBitmap = bmpDisabled.GetSize();
            m_disabledImgList = new wxImageList
                                    (
                                        sizeBitmap.x,
                                        sizeBitmap.y,
                                                                                                                                                                !bmpDisabled.HasAlpha(),
                                        GetToolsCount()
                                    );
            break;
        }
    }
}

bool wxToolBar::Realize()
{
    if ( !wxToolBarBase::Realize() )
        return false;

    const size_t nTools = GetToolsCount();

            enum
    {
        Remap_None = -1,
        Remap_Bg,
        Remap_Buttons,
        Remap_TransparentBg
    };

                static const wxChar *remapOption = wxT("msw.remap");
    const int remapValue = wxSystemOptions::HasOption(remapOption)
                                ? wxSystemOptions::GetOptionInt(remapOption)
                                : wxDisplayDepth() <= 8 ? Remap_Buttons
                                                        : Remap_None;


        for ( size_t pos = 0; pos < m_nButtons; pos++ )
    {
        if ( !::SendMessage(GetHwnd(), TB_DELETEBUTTON, 0, 0) )
        {
            wxLogDebug(wxT("TB_DELETEBUTTON failed"));
        }
    }

        
    wxToolBarToolsList::compatibility_iterator node;
    int bitmapId = 0;

    if ( !HasFlag(wxTB_NOICONS) )
    {
                        HBITMAP oldToolBarBitmap = (HBITMAP)m_hBitmap;

        const wxCoord totalBitmapWidth  = m_defaultWidth *
                                          wx_truncate_cast(wxCoord, nTools),
                      totalBitmapHeight = m_defaultHeight;

                wxMemoryDC dcAllButtons;
        wxBitmap bitmap(totalBitmapWidth, totalBitmapHeight);
        dcAllButtons.SelectObject(bitmap);

        if ( remapValue != Remap_TransparentBg )
        {
            dcAllButtons.SetBackground(GetBackgroundColour());
            dcAllButtons.Clear();
        }

        HBITMAP hBitmap = GetHbitmapOf(bitmap);

        if ( remapValue == Remap_Bg )
        {
            dcAllButtons.SelectObject(wxNullBitmap);

                                    hBitmap = (HBITMAP)MapBitmap((WXHBITMAP) hBitmap,
                totalBitmapWidth, totalBitmapHeight);

            dcAllButtons.SelectObject(bitmap);
        }

                wxCoord x = 0;

                int nButtons = 0;

        CreateDisabledImageList();
        for ( node = m_tools.GetFirst(); node; node = node->GetNext() )
        {
            wxToolBarToolBase *tool = node->GetData();
            if ( tool->IsButton() )
            {
                const wxBitmap& bmp = tool->GetNormalBitmap();

                const int w = bmp.GetWidth();
                const int h = bmp.GetHeight();

                if ( bmp.IsOk() )
                {
                                                                                if ( bmp.HasAlpha() )
                        bitmap.UseAlpha();

                    int xOffset = wxMax(0, (m_defaultWidth - w)/2);
                    int yOffset = wxMax(0, (m_defaultHeight - h)/2);

                                        dcAllButtons.DrawBitmap(bmp, x + xOffset, yOffset, true);

                                                                                                    hBitmap = GetHbitmapOf(bitmap);
                }
                else
                {
                    wxFAIL_MSG( wxT("invalid tool button bitmap") );
                }

                                if ( m_disabledImgList )
                {
                    wxBitmap bmpDisabled = tool->GetDisabledBitmap();
#if wxUSE_IMAGE && wxUSE_WXDIB
                    if ( !bmpDisabled.IsOk() )
                    {
                                                                                                wxImage
                          imgGreyed = bmp.ConvertToImage().ConvertToGreyscale();

                        if ( remapValue == Remap_Buttons )
                        {
                                                                                    for ( int y = 0; y < h; y++ )
                            {
                                for ( int xx = 0; xx < w; xx++ )
                                {
                                    if ( imgGreyed.IsTransparent(xx, y) )
                                        imgGreyed.SetRGB(xx, y,
                                                         wxLIGHT_GREY->Red(),
                                                         wxLIGHT_GREY->Green(),
                                                         wxLIGHT_GREY->Blue());
                                }
                            }
                        }

                        bmpDisabled = wxBitmap(imgGreyed);
                    }
#endif 
                    if ( remapValue == Remap_Buttons )
                        MapBitmap(bmpDisabled.GetHBITMAP(), w, h);

                    m_disabledImgList->Add(bmpDisabled);
                }

                                                                x += m_defaultWidth;
                nButtons++;
            }
        }

        dcAllButtons.SelectObject(wxNullBitmap);

                bitmap.SetHBITMAP(0);

        if ( remapValue == Remap_Buttons )
        {
                        hBitmap = (HBITMAP)MapBitmap((WXHBITMAP) hBitmap,
                                         totalBitmapWidth, totalBitmapHeight);
        }

        m_hBitmap = hBitmap;

        bool addBitmap = true;

        if ( oldToolBarBitmap )
        {
#ifdef TB_REPLACEBITMAP
            TBREPLACEBITMAP replaceBitmap;
            replaceBitmap.hInstOld = NULL;
            replaceBitmap.hInstNew = NULL;
            replaceBitmap.nIDOld = (UINT_PTR)oldToolBarBitmap;
            replaceBitmap.nIDNew = (UINT_PTR)hBitmap;
            replaceBitmap.nButtons = nButtons;
            if ( !::SendMessage(GetHwnd(), TB_REPLACEBITMAP,
                                0, (LPARAM) &replaceBitmap) )
            {
                wxFAIL_MSG(wxT("Could not replace the old bitmap"));
            }

            ::DeleteObject(oldToolBarBitmap);

                        addBitmap = false;
#else
                                                addBitmap = true;

            bitmapId = m_nButtons;
#endif         }

        if ( addBitmap )         {
            TBADDBITMAP tbAddBitmap;
            tbAddBitmap.hInst = 0;
            tbAddBitmap.nID = (UINT_PTR)hBitmap;
            if ( ::SendMessage(GetHwnd(), TB_ADDBITMAP,
                               (WPARAM) nButtons, (LPARAM)&tbAddBitmap) == -1 )
            {
                wxFAIL_MSG(wxT("Could not add bitmap to toolbar"));
            }
        }

        HIMAGELIST hil = m_disabledImgList
                            ? GetHimagelistOf(m_disabledImgList)
                            : 0;

                        HIMAGELIST oldImageList = (HIMAGELIST)
          ::SendMessage(GetHwnd(), TB_SETDISABLEDIMAGELIST, 0, (LPARAM)hil);

                if ( oldImageList )
            ::DeleteObject(oldImageList);
    }


        
    wxScopedArray<TBBUTTON> buttons(nTools);

        wxArrayInt controlIds;

    bool lastWasRadio = false;
    int i = 0;
    for ( node = m_tools.GetFirst(); node; node = node->GetNext() )
    {
        wxToolBarTool *tool = static_cast<wxToolBarTool *>(node->GetData());

        TBBUTTON& button = buttons[i];

        wxZeroMemory(button);

        bool isRadio = false;
        switch ( tool->GetStyle() )
        {
            case wxTOOL_STYLE_CONTROL:
                if ( wxStaticText *staticText = tool->GetStaticText() )
                {
                                                                                staticText->
                        Show(HasFlag(wxTB_TEXT) && !HasFlag(wxTB_NOICONS));
                }

                                                                {
                    const wxSize sizeControl = tool->GetControl()->GetSize();
                    button.iBitmap = m_toolPacking + (IsVertical() ? sizeControl.y : sizeControl.x);
                }

                wxFALLTHROUGH;

            case wxTOOL_STYLE_SEPARATOR:
                if ( tool->IsStretchableSpace() )
                {
                                                                                tool->AllocSpacerId();
                }

                button.idCommand = tool->GetId();

                                                                if ( IsVertical() && tool->IsControl() )
                    button.fsState = TBSTATE_HIDDEN;
                else
                    button.fsState = TBSTATE_ENABLED;
                button.fsStyle = TBSTYLE_SEP;
                break;

            case wxTOOL_STYLE_BUTTON:
                if ( !HasFlag(wxTB_NOICONS) )
                    button.iBitmap = bitmapId;

                if ( HasFlag(wxTB_TEXT) )
                {
                    const wxString& label = tool->GetLabel();
                    if ( !label.empty() )
                        button.iString = (INT_PTR) wxMSW_CONV_LPCTSTR(label);
                }

                button.idCommand = tool->GetId();

                if ( tool->IsEnabled() )
                    button.fsState |= TBSTATE_ENABLED;
                if ( MSWShouldBeChecked(tool) )
                    button.fsState |= TBSTATE_CHECKED;

                switch ( tool->GetKind() )
                {
                    case wxITEM_RADIO:
                        button.fsStyle = TBSTYLE_CHECKGROUP;

                        if ( !lastWasRadio )
                        {
                                                                                                                button.fsState |= TBSTATE_CHECKED;

                            if (tool->Toggle(true))
                            {
                                DoToggleTool(tool, true);
                            }
                        }
                        else if ( tool->IsToggled() )
                        {
                            wxToolBarToolsList::compatibility_iterator nodePrev = node->GetPrevious();
                            int prevIndex = i - 1;
                            while ( nodePrev )
                            {
                                TBBUTTON& prevButton = buttons[prevIndex];
                                wxToolBarToolBase *toolPrev = nodePrev->GetData();
                                if ( !toolPrev->IsButton() || toolPrev->GetKind() != wxITEM_RADIO )
                                    break;

                                if ( toolPrev->Toggle(false) )
                                    DoToggleTool(toolPrev, false);

                                prevButton.fsState &= ~TBSTATE_CHECKED;
                                nodePrev = nodePrev->GetPrevious();
                                prevIndex--;
                            }
                        }

                        isRadio = true;
                        break;

                    case wxITEM_CHECK:
                        button.fsStyle = TBSTYLE_CHECK;
                        break;

                    case wxITEM_NORMAL:
                        button.fsStyle = TBSTYLE_BUTTON;
                        break;

                   case wxITEM_DROPDOWN:
                        button.fsStyle = TBSTYLE_DROPDOWN;
                        break;

                    default:
                        wxFAIL_MSG( wxT("unexpected toolbar button kind") );
                        button.fsStyle = TBSTYLE_BUTTON;
                        break;
                }

                                                                                                if ( !IsVertical() )
                {
                    button.fsStyle |= TBSTYLE_AUTOSIZE;
                }

                bitmapId++;
                break;
        }

        lastWasRadio = isRadio;

        i++;
    }

    if ( !::SendMessage(GetHwnd(), TB_ADDBUTTONS, i, (LPARAM)buttons.get()) )
    {
        wxLogLastError(wxT("TB_ADDBUTTONS"));
    }


        
            m_totalFixedSize = 0;
    int toolIndex = 0;
    for ( node = m_tools.GetFirst(); node; node = node->GetNext(), toolIndex++ )
    {
        wxToolBarTool * const tool = (wxToolBarTool*)node->GetData();

        const RECT r = wxGetTBItemRect(GetHwnd(), toolIndex, tool->GetId());

        if ( !tool->IsControl() )
        {
            if ( IsVertical() )
                m_totalFixedSize += r.bottom - r.top;
            else
                m_totalFixedSize += r.right - r.left;

            continue;
        }

        wxControl * const control = tool->GetControl();
        if ( IsVertical() )
        {
                                                control->Hide();
            continue;
        }

        control->Show();
        wxStaticText * const staticText = tool->GetStaticText();

        wxSize size = control->GetSize();
        wxSize staticTextSize;
        if ( staticText && staticText->IsShown() )
        {
            staticTextSize = staticText->GetSize();
            staticTextSize.y += 3;         }

                        int height = r.bottom - r.top - staticTextSize.y;

        int diff = height - size.y;
        if ( diff < 0 || !HasFlag(wxTB_TEXT) )
        {
                        if ( staticText )
                staticText->Hide();

                        height = r.bottom - r.top;
            diff = height - size.y;
            if ( diff < 0 )
            {
                                                                
                diff = 2;
            }
        }
        else         {
            if ( staticText )
                staticText->Show();
        }

                control->Move(r.left + m_toolPacking/2, r.top + (diff + 1) / 2);
        if ( staticText )
        {
            staticText->Move(r.left + m_toolPacking/2 + (size.x - staticTextSize.x)/2,
                             r.bottom - staticTextSize.y);
        }

        m_totalFixedSize += r.right - r.left;
    }

            m_nButtons = toolIndex;

    if ( !IsVertical() )
    {
        if ( m_maxRows == 0 )
                        SetRows(1);
    }
    else if ( m_nButtons > 0 )     {
                m_maxRows = 1;
        SetRows(m_nButtons);
    }

    InvalidateBestSize();
    UpdateSize();

    if ( IsVertical() )
    {
                                m_totalFixedSize = 0;
        toolIndex = 0;
        for ( node = m_tools.GetFirst(); node; node = node->GetNext(), toolIndex++ )
        {
            wxToolBarTool * const tool = (wxToolBarTool*)node->GetData();
            if ( !tool->IsStretchableSpace() )
            {
                const RECT r = wxGetTBItemRect(GetHwnd(), toolIndex);
                if ( !IsVertical() )
                    m_totalFixedSize += r.right - r.left;
                else if ( !tool->IsControl() )
                    m_totalFixedSize += r.bottom - r.top;
            }
        }
                UpdateSize();
    }

    return true;
}

void wxToolBar::UpdateStretchableSpacersSize()
{
        unsigned numSpaces = 0;
    wxToolBarToolsList::compatibility_iterator node;
    int toolIndex = 0;
    for ( node = m_tools.GetFirst(); node; node = node->GetNext() )
    {
        wxToolBarTool * const tool = (wxToolBarTool*)node->GetData();

        if ( tool->IsToBeDeleted() )
            continue;

        if ( tool->IsStretchableSpace() )
        {
                        const RECT rcItem = wxGetTBItemRect(GetHwnd(), toolIndex);
            if ( !::IsRectEmpty(&rcItem) )
                numSpaces++;
        }

        toolIndex++;
    }

    if ( !numSpaces )
        return;

            const int totalSize = IsVertical() ? GetClientSize().y : GetClientSize().x;
    const int extraSize = totalSize - m_totalFixedSize;
    const int sizeSpacer = extraSize > 0 ? extraSize / numSpaces : 1;

            const int sizeLastSpacer = extraSize > 0
                                ? extraSize - (numSpaces - 1)*sizeSpacer
                                : 1;

                    int offset = 0;
    toolIndex = 0;
    for ( node = m_tools.GetFirst(); node; node = node->GetNext() )
    {
        wxToolBarTool * const tool = (wxToolBarTool*)node->GetData();

        if ( tool->IsToBeDeleted() )
            continue;

        if ( tool->IsControl() && offset )
        {
            tool->MoveBy(offset);
            toolIndex++;
            continue;
        }

        if ( !tool->IsStretchableSpace() )
        {
            toolIndex++;
            continue;
        }

        const RECT rcOld = wxGetTBItemRect(GetHwnd(), toolIndex);

        const int oldSize = IsVertical()? (rcOld.bottom - rcOld.top): (rcOld.right - rcOld.left);
        const int newSize = --numSpaces ? sizeSpacer : sizeLastSpacer;
        if ( newSize != oldSize)
        {
            if ( !::SendMessage(GetHwnd(), TB_DELETEBUTTON, toolIndex, 0) )
            {
                wxLogLastError(wxT("TB_DELETEBUTTON (separator)"));
            }
            else
            {
                TBBUTTON button;
                wxZeroMemory(button);

                button.idCommand = tool->GetId();
                button.iBitmap = newSize;                 button.fsState = TBSTATE_ENABLED;
                button.fsStyle = TBSTYLE_SEP;
                if ( IsVertical() )
                    button.fsState |= TBSTATE_WRAP;
                if ( !::SendMessage(GetHwnd(), TB_INSERTBUTTON, toolIndex, (LPARAM)&button) )
                {
                    wxLogLastError(wxT("TB_INSERTBUTTON (separator)"));
                }
                else
                {
                                                            offset += newSize - oldSize;
                }
            }
        }

        toolIndex++;
    }
}


bool wxToolBar::MSWCommand(WXUINT WXUNUSED(cmd), WXWORD id_)
{
                const int id = (signed short)id_;

    wxToolBarToolBase *tool = FindById(id);
    if ( !tool )
        return false;

    bool toggled = false; 
    LRESULT state = ::SendMessage(GetHwnd(), TB_GETSTATE, id, 0);

    if ( tool->CanBeToggled() )
    {
        toggled = (state & TBSTATE_CHECKED) != 0;

                        if ( tool->GetKind() == wxITEM_RADIO && !toggled )
            return true;

        tool->Toggle(toggled);
        UnToggleRadioGroup(tool);
    }

                                                    ::SendMessage(GetHwnd(), TB_SETSTATE, id, MAKELONG(state | TBSTATE_PRESSED, 0));
    Update();

    bool allowLeftClick = OnLeftClick(id, toggled);

                    if ( FindById(id) != tool )
    {
                        return true;
    }

            if (tool->IsEnabled())
        state |= TBSTATE_ENABLED;
    else
        state &= ~TBSTATE_ENABLED;
    if ( MSWShouldBeChecked(tool) )
        state |= TBSTATE_CHECKED;
    else
        state &= ~TBSTATE_CHECKED;
    ::SendMessage(GetHwnd(), TB_SETSTATE, id, MAKELONG(state, 0));

            if ( !allowLeftClick && tool->CanBeToggled() )
    {
                tool->Toggle(!toggled);

        ::SendMessage(GetHwnd(), TB_CHECKBUTTON, id,
                      MAKELONG(MSWShouldBeChecked(tool), 0));
    }

    return true;
}

bool wxToolBar::MSWOnNotify(int WXUNUSED(idCtrl),
                            WXLPARAM lParam,
                            WXLPARAM *WXUNUSED(result))
{
    LPNMHDR hdr = (LPNMHDR)lParam;
    if ( hdr->code == TBN_DROPDOWN )
    {
        LPNMTOOLBAR tbhdr = (LPNMTOOLBAR)lParam;

        wxCommandEvent evt(wxEVT_TOOL_DROPDOWN, tbhdr->iItem);
        if ( HandleWindowEvent(evt) )
        {
                        return false;
        }

        const wxToolBarToolBase * const tool = FindById(tbhdr->iItem);
        wxCHECK_MSG( tool, false, wxT("drop down message for unknown tool") );

        wxMenu * const menu = tool->GetDropdownMenu();
        if ( !menu )
            return false;

                const RECT r = wxGetTBItemRect(GetHwnd(), GetToolPos(tbhdr->iItem));
        if ( r.right )
            PopupMenu(menu, r.left, r.bottom);

        return true;
    }


    if( !HasFlag(wxTB_NO_TOOLTIPS) )
    {
#if wxUSE_TOOLTIPS
        
                        UINT code = hdr->code;
        if ( (code != (UINT) TTN_NEEDTEXTA) && (code != (UINT) TTN_NEEDTEXTW) )
            return false;

        HWND toolTipWnd = (HWND)::SendMessage(GetHwnd(), TB_GETTOOLTIPS, 0, 0);
        if ( toolTipWnd != hdr->hwndFrom )
            return false;

        LPTOOLTIPTEXT ttText = (LPTOOLTIPTEXT)lParam;
        int id = (int)ttText->hdr.idFrom;

        wxToolBarToolBase *tool = FindById(id);
        if ( tool )
            return HandleTooltipNotify(code, lParam, tool->GetShortHelp());
#else
        wxUnusedVar(lParam);
#endif
    }

    return false;
}


void wxToolBar::SetToolBitmapSize(const wxSize& size)
{
        wxSize effectiveSize;

    if ( !HasFlag(wxTB_NOICONS) )
        effectiveSize = size;

    wxToolBarBase::SetToolBitmapSize(size);

    ::SendMessage(GetHwnd(), TB_SETBITMAPSIZE, 0, MAKELONG(size.x, size.y));
}

void wxToolBar::SetRows(int nRows)
{
    if ( nRows == m_maxRows )
    {
                return;
    }

            RECT rect;
    ::SendMessage(GetHwnd(), TB_SETROWS,
                  MAKEWPARAM(nRows, !(GetWindowStyle() & wxTB_VERTICAL)),
                  (LPARAM) &rect);

    m_maxRows = nRows;

                const bool enable = (!IsVertical() && m_maxRows == 1) ||
                           (IsVertical() && (size_t)m_maxRows == m_nButtons);

    const LPARAM state = MAKELONG(enable ? TBSTATE_ENABLED : TBSTATE_HIDDEN, 0);
    wxToolBarToolsList::compatibility_iterator node;
    for ( node = m_tools.GetFirst(); node; node = node->GetNext() )
    {
        wxToolBarTool * const tool = (wxToolBarTool*)node->GetData();
        if ( tool->IsStretchableSpace() )
        {
            if ( !::SendMessage(GetHwnd(), TB_SETSTATE, tool->GetId(), state) )
            {
                wxLogLastError(wxT("TB_SETSTATE (stretchable spacer)"));
            }
        }
    }

    UpdateSize();
}

wxSize wxToolBar::GetToolSize() const
{
    DWORD dw = ::SendMessage(GetHwnd(), TB_GETBUTTONSIZE, 0, 0);

    return wxSize(LOWORD(dw), HIWORD(dw));
}

wxToolBarToolBase *wxToolBar::FindToolForPosition(wxCoord x, wxCoord y) const
{
    POINT pt;
    pt.x = x;
    pt.y = y;
    int index = (int)::SendMessage(GetHwnd(), TB_HITTEST, 0, (LPARAM)&pt);

            if ( index < 0 || (size_t)index >= m_nButtons )
                return NULL;

        return m_tools.Item((size_t)index)->GetData();
}

void wxToolBar::UpdateSize()
{
    wxPoint pos = GetPosition();
    ::SendMessage(GetHwnd(), TB_AUTOSIZE, 0, 0);
    if (pos != GetPosition())
        Move(pos);

                        SendSizeEventToParent();
}


long wxToolBar::GetMSWToolbarStyle() const
{
    return ::SendMessage(GetHwnd(), TB_GETSTYLE, 0, 0L);
}

void wxToolBar::SetWindowStyleFlag(long style)
{
        static const long MASK_NEEDS_RECREATE = wxTB_TEXT | wxTB_NOICONS;

    const long styleOld = GetWindowStyle();

    wxToolBarBase::SetWindowStyleFlag(style);

                if ( GetToolsCount() &&
            (style & MASK_NEEDS_RECREATE) != (styleOld & MASK_NEEDS_RECREATE) )
    {
                                Recreate();
    }
}


void wxToolBar::DoEnableTool(wxToolBarToolBase *tool, bool enable)
{
    if ( tool->IsButton() )
    {
        ::SendMessage(GetHwnd(), TB_ENABLEBUTTON,
                      (WPARAM)tool->GetId(), (LPARAM)MAKELONG(enable, 0));

                        DoToggleTool(tool, tool->IsToggled());
    }
    else if ( tool->IsControl() )
    {
        wxToolBarTool* tbTool = static_cast<wxToolBarTool*>(tool);

        tbTool->GetControl()->Enable(enable);
        wxStaticText* text = tbTool->GetStaticText();
        if ( text )
            text->Enable(enable);
    }
}

void wxToolBar::DoToggleTool(wxToolBarToolBase *tool,
                             bool WXUNUSED_UNLESS_DEBUG(toggle))
{
    wxASSERT_MSG( tool->IsToggled() == toggle, wxT("Inconsistent tool state") );

    ::SendMessage(GetHwnd(), TB_CHECKBUTTON,
                  (WPARAM)tool->GetId(),
                  (LPARAM)MAKELONG(MSWShouldBeChecked(tool), 0));
}

void wxToolBar::DoSetToggle(wxToolBarToolBase *WXUNUSED(tool), bool WXUNUSED(toggle))
{
            wxFAIL_MSG( wxT("not implemented") );
}

void wxToolBar::SetToolNormalBitmap( int id, const wxBitmap& bitmap )
{
    wxToolBarTool* tool = static_cast<wxToolBarTool*>(FindById(id));
    if ( tool )
    {
        wxCHECK_RET( tool->IsButton(), wxT("Can only set bitmap on button tools."));

        tool->SetNormalBitmap(bitmap);
        Realize();
    }
}

void wxToolBar::SetToolDisabledBitmap( int id, const wxBitmap& bitmap )
{
    wxToolBarTool* tool = static_cast<wxToolBarTool*>(FindById(id));
    if ( tool )
    {
        wxCHECK_RET( tool->IsButton(), wxT("Can only set bitmap on button tools."));

        tool->SetDisabledBitmap(bitmap);
        Realize();
    }
}

void wxToolBar::SetToolPacking(int packing)
{
    if ( packing > 0 && packing != m_toolPacking )
    {
        m_toolPacking = packing;
        if ( GetHwnd() )
        {
            MSWSetPadding(packing);
            Realize();
        }
    }
}


void wxToolBar::OnSysColourChanged(wxSysColourChangedEvent& event)
{
    wxRGBToColour(m_backgroundColour, ::GetSysColor(COLOR_BTNFACE));

        Realize();

        int nrows = m_maxRows;
    m_maxRows = 0;          SetRows(nrows);

    Refresh();

        event.Skip();
}

void wxToolBar::OnMouseEvent(wxMouseEvent& event)
{
    if ( event.Leaving() )
    {
        if ( m_pInTool )
        {
            OnMouseEnter(wxID_ANY);
            m_pInTool = NULL;
        }

        event.Skip();
        return;
    }

    if ( event.RightDown() )
    {
                wxCoord x = 0, y = 0;
        event.GetPosition(&x, &y);

        wxToolBarToolBase *tool = FindToolForPosition(x, y);
        OnRightClick(tool ? tool->GetId() : -1, x, y);
    }
    else
    {
        event.Skip();
    }
}

void wxToolBar::OnEraseBackground(wxEraseEvent& event)
{
#ifdef wxHAS_MSW_BACKGROUND_ERASE_HOOK
    MSWDoEraseBackground(event.GetDC()->GetHDC());
#endif }

bool wxToolBar::HandleSize(WXWPARAM WXUNUSED(wParam), WXLPARAM lParam)
{
        const int toolsCount = GetToolsCount();
    if ( toolsCount == 0 )
        return false;

                RECT r;
    ::SetRectEmpty(&r);
        RECT rcRow;
    ::SetRectEmpty(&rcRow);
    int rowPosX = INT_MIN;
    wxToolBarToolsList::compatibility_iterator node;
    int i = 0;
    for ( node = m_tools.GetFirst(); node; node = node->GetNext() )
    {
        wxToolBarTool * const
            tool = static_cast<wxToolBarTool *>(node->GetData());
        if ( tool->IsToBeDeleted() )
            continue;

                const RECT rcItem = wxGetTBItemRect(GetHwnd(), i);
        if ( ::IsRectEmpty(&rcItem) )
        {
            i++;
            continue;
        }

        if ( rcItem.top > rowPosX )
        {
                        rowPosX = rcItem.top;

                        ::OffsetRect(&rcRow, -rcRow.left, -rcRow.top);

                        ::UnionRect(&r, &r, &rcRow);

                        ::SetRectEmpty(&rcRow);
        }

                                        if( !tool->IsSeparator() )
        {
                        ::UnionRect(&rcRow, &rcRow, &rcItem);
        }

        i++;
    }

        ::OffsetRect(&rcRow, -rcRow.left, -rcRow.top);
    ::UnionRect(&r, &r, &rcRow);

    if ( !r.right )
        return false;

    int w, h;

    if ( IsVertical() )
    {
        w = r.right - r.left;
        h = HIWORD(lParam);
    }
    else
    {
        w = LOWORD(lParam);
        if (HasFlag( wxTB_FLAT ))
            h = r.bottom - r.top - 3;
        else
            h = r.bottom - r.top;

                for ( node = m_tools.GetFirst(); node; node = node->GetNext() )
        {
            wxToolBarTool * const
                tool = static_cast<wxToolBarTool *>(node->GetData());
            if (tool->IsControl())
            {
                int y = (tool->GetControl()->GetSize().y - 2);                 if (y > h)
                    h = y;
            }
        }

        if ( m_maxRows )
        {
                        h += HasFlag(wxTB_NODIVIDER) ? 4 : 6;
            h *= m_maxRows;
        }
    }

    if ( MAKELPARAM(w, h) != lParam )
    {
                SetSize(w, h);
    }

    UpdateStretchableSpacersSize();

        return true;
}

#ifdef wxHAS_MSW_BACKGROUND_ERASE_HOOK

bool wxToolBar::HandlePaint(WXWPARAM wParam, WXLPARAM lParam)
{
                
            wxRegion rgnDummySeps;
    const wxRect rectTotal = GetClientRect();
    int toolIndex = 0;
    for ( wxToolBarToolsList::compatibility_iterator node = m_tools.GetFirst();
          node;
          node = node->GetNext(), toolIndex++ )
    {
        wxToolBarTool * const
            tool = static_cast<wxToolBarTool *>(node->GetData());

        if ( tool->IsToBeDeleted() )
            continue;

        if ( tool->IsControl() || tool->IsStretchableSpace() )
        {
                                                RECT rcItem = wxGetTBItemRect(GetHwnd(), toolIndex);

                        if ( ::IsRectEmpty(&rcItem) )
                continue;

            if ( IsVertical() )
            {
                rcItem.left = 0;
                rcItem.right = rectTotal.width;
            }
            else
            {
                rcItem.bottom = rcItem.top + rectTotal.height / m_maxRows;
            }

                                    if ( rcItem.bottom - rcItem.top > 0 && rcItem.bottom - rcItem.top < 3 )
                rcItem.bottom = rcItem.top + 3;

            rgnDummySeps.Union(wxRectFromRECT(rcItem));
        }
    }

    if ( rgnDummySeps.IsOk() )
    {
                                if ( !::ValidateRgn(GetHwnd(), GetHrgnOf(rgnDummySeps)) )
        {
            wxLogLastError(wxT("ValidateRgn()"));
        }
    }

                                            wxWindow * const parent = GetParent();
    const bool hadHook = parent->MSWHasEraseBgHook();
    if ( !hadHook )
        GetParent()->MSWSetEraseBgHook(this);

    MSWDefWindowProc(WM_PAINT, wParam, lParam);

    if ( !hadHook )
        GetParent()->MSWSetEraseBgHook(NULL);


    if ( rgnDummySeps.IsOk() )
    {
                        WindowHDC hdc(GetHwnd());
        ::SelectClipRgn(hdc, GetHrgnOf(rgnDummySeps));
        MSWDoEraseBackground(hdc);
    }

    return true;
}

WXHBRUSH wxToolBar::MSWGetToolbarBgBrush()
{
                                                wxColour const
        colBg = m_hasBgCol ? GetBackgroundColour()
                           : wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
    wxBrush * const
        brush = wxTheBrushList->FindOrCreateBrush(colBg);

    return brush ? static_cast<WXHBRUSH>(brush->GetResourceHandle()) : 0;
}

WXHBRUSH wxToolBar::MSWGetBgBrushForChild(WXHDC hDC, wxWindowMSW *child)
{
    WXHBRUSH hbr = wxToolBarBase::MSWGetBgBrushForChild(hDC, child);
    if ( hbr )
        return hbr;

                    if ( child->GetParent() == this && child->HasTransparentBackground() )
        return MSWGetToolbarBgBrush();

    return 0;
}

void wxToolBar::MSWDoEraseBackground(WXHDC hDC)
{
    wxFillRect(GetHwnd(), (HDC)hDC, (HBRUSH)MSWGetToolbarBgBrush());
}

bool wxToolBar::MSWEraseBgHook(WXHDC hDC)
{
                HDC hdc = (HDC)hDC;
    POINT ptOldOrg;
    if ( !::SetWindowOrgEx(hdc, 0, 0, &ptOldOrg) )
    {
        wxLogLastError(wxT("SetWindowOrgEx(tbar-bg-hdc)"));
        return false;
    }

    MSWDoEraseBackground(hDC);

    ::SetWindowOrgEx(hdc, ptOldOrg.x, ptOldOrg.y, NULL);

    return true;
}

#endif 
void wxToolBar::HandleMouseMove(WXWPARAM WXUNUSED(wParam), WXLPARAM lParam)
{
    wxCoord x = GET_X_LPARAM(lParam),
            y = GET_Y_LPARAM(lParam);
    wxToolBarToolBase* tool = FindToolForPosition( x, y );

        if ( tool != m_pInTool )
    {
        m_pInTool = tool;
        OnMouseEnter(tool ? tool->GetId() : wxID_ANY);
    }
}

WXLRESULT wxToolBar::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
    switch ( nMsg )
    {
        case WM_MOUSEMOVE:
                                    HandleMouseMove(wParam, lParam);
            break;

        case WM_SIZE:
            if ( HandleSize(wParam, lParam) )
                return 0;
            break;

#ifdef wxHAS_MSW_BACKGROUND_ERASE_HOOK
        case WM_PAINT:
            if ( HandlePaint(wParam, lParam) )
                return 0;
            break;
#endif 
        case WM_PRINTCLIENT:
            wxFillRect(GetHwnd(), (HDC)wParam, MSWGetToolbarBgBrush());
            return 1;
    }

    return wxControl::MSWWindowProc(nMsg, wParam, lParam);
}


WXHBITMAP wxToolBar::MapBitmap(WXHBITMAP bitmap, int width, int height)
{
    MemoryHDC hdcMem;

    if ( !hdcMem )
    {
        wxLogLastError(wxT("CreateCompatibleDC"));

        return bitmap;
    }

    SelectInHDC bmpInHDC(hdcMem, (HBITMAP)bitmap);

    if ( !bmpInHDC )
    {
        wxLogLastError(wxT("SelectObject"));

        return bitmap;
    }

    wxCOLORMAP *cmap = wxGetStdColourMap();

    for ( int i = 0; i < width; i++ )
    {
        for ( int j = 0; j < height; j++ )
        {
            COLORREF pixel = ::GetPixel(hdcMem, i, j);

            for ( size_t k = 0; k < wxSTD_COL_MAX; k++ )
            {
                COLORREF col = cmap[k].from;
                if ( abs(GetRValue(pixel) - GetRValue(col)) < 10 &&
                     abs(GetGValue(pixel) - GetGValue(col)) < 10 &&
                     abs(GetBValue(pixel) - GetBValue(col)) < 10 )
                {
                    if ( cmap[k].to != pixel )
                        ::SetPixel(hdcMem, i, j, cmap[k].to);
                    break;
                }
            }
        }
    }

    return bitmap;
}

#endif 