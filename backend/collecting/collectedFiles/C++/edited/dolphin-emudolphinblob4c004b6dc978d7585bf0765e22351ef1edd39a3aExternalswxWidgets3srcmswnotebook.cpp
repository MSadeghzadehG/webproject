
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_NOTEBOOK

#include "wx/notebook.h"

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcctl.h"     #include "wx/string.h"
    #include "wx/dc.h"
    #include "wx/log.h"
    #include "wx/event.h"
    #include "wx/app.h"
    #include "wx/dcclient.h"
    #include "wx/dcmemory.h"
    #include "wx/control.h"
    #include "wx/panel.h"
#endif  
#include "wx/imaglist.h"
#include "wx/sysopt.h"

#include "wx/msw/private.h"
#include "wx/msw/dc.h"

#include <windowsx.h>
#include "wx/msw/winundef.h"

#if wxUSE_UXTHEME
    #include "wx/msw/uxtheme.h"
#endif


#define IS_VALID_PAGE(nPage) ((nPage) < GetPageCount())

#define USE_NOTEBOOK_ANTIFLICKER    1


#ifndef TCS_RIGHT
    #define TCS_RIGHT       0x0002
#endif

#ifndef TCS_VERTICAL
    #define TCS_VERTICAL    0x0080
#endif

#ifndef TCS_BOTTOM
    #define TCS_BOTTOM      TCS_RIGHT
#endif


#if USE_NOTEBOOK_ANTIFLICKER

static WXFARPROC gs_wndprocNotebookSpinBtn = (WXFARPROC)NULL;

static WXFARPROC gs_wndprocNotebook = (WXFARPROC)NULL;

LRESULT APIENTRY
wxNotebookWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

#endif 

static bool HasTroubleWithNonTopTabs()
{
    const int verComCtl32 = wxApp::GetComCtl32Version();

                return verComCtl32 >= 600 && verComCtl32 <= 616;
}


wxBEGIN_EVENT_TABLE(wxNotebook, wxBookCtrlBase)
    EVT_SIZE(wxNotebook::OnSize)
    EVT_NAVIGATION_KEY(wxNotebook::OnNavigationKey)

#if USE_NOTEBOOK_ANTIFLICKER
    EVT_ERASE_BACKGROUND(wxNotebook::OnEraseBackground)
    EVT_PAINT(wxNotebook::OnPaint)
#endif wxEND_EVENT_TABLE()



void wxNotebook::Init()
{
#if wxUSE_UXTHEME
    m_hbrBackground = NULL;
#endif 
#if USE_NOTEBOOK_ANTIFLICKER
    m_hasSubclassedUpdown = false;
#endif }

wxNotebook::wxNotebook()
{
  Init();
}

wxNotebook::wxNotebook(wxWindow *parent,
                       wxWindowID id,
                       const wxPoint& pos,
                       const wxSize& size,
                       long style,
                       const wxString& name)
{
  Init();

  Create(parent, id, pos, size, style, name);
}

bool wxNotebook::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        long style,
                        const wxString& name)
{
    if ( (style & wxBK_ALIGN_MASK) == wxBK_DEFAULT )
    {
        style |= wxBK_TOP;
    }

#if !wxUSE_UXTHEME
                    if ( HasTroubleWithNonTopTabs() )
    {
        style &= ~(wxBK_BOTTOM | wxBK_LEFT | wxBK_RIGHT);
    }
#endif 
#if defined(__WINE__) && wxUSE_UNICODE
    LPCTSTR className = L"SysTabControl32";
#else
    LPCTSTR className = WC_TABCONTROL;
#endif

#if USE_NOTEBOOK_ANTIFLICKER
                if ( !HasFlag(wxFULL_REPAINT_ON_RESIZE) )
    {
        static ClassRegistrar s_clsNotebook;
        if ( !s_clsNotebook.IsInitialized() )
        {
                        WNDCLASS wc;

            if ( ::GetClassInfo(NULL, WC_TABCONTROL, &wc) )
            {
                gs_wndprocNotebook =
                    reinterpret_cast<WXFARPROC>(wc.lpfnWndProc);
                wc.lpszClassName = wxT("_wx_SysTabCtl32");
                wc.style &= ~(CS_HREDRAW | CS_VREDRAW);
                wc.hInstance = wxGetInstance();
                wc.lpfnWndProc = wxNotebookWndProc;
                s_clsNotebook.Register(wc);
            }
            else
            {
                wxLogLastError(wxT("GetClassInfoEx(SysTabCtl32)"));
            }
        }

                        if ( s_clsNotebook.IsRegistered() )
        {
                                    className = s_clsNotebook.GetName().c_str();
        }
    }
#endif 
    if ( !CreateControl(parent, id, pos, size, style | wxTAB_TRAVERSAL,
                        wxDefaultValidator, name) )
        return false;

    if ( !MSWCreateControl(className, wxEmptyString, pos, size) )
        return false;

                InheritAttributes();
    if ( parent->InheritsBackgroundColour() && !UseBgCol() )
        SetBackgroundColour(parent->GetBackgroundColour());

#if wxUSE_UXTHEME
    if ( HasFlag(wxNB_NOPAGETHEME) ||
            wxSystemOptions::IsFalse(wxT("msw.notebook.themed-background")) )
    {
        SetBackgroundColour(GetThemeBackgroundColour());
    }
    else     {
                UpdateBgBrush();
    }

                if ( HasTroubleWithNonTopTabs() &&
            (style & (wxBK_BOTTOM | wxBK_LEFT | wxBK_RIGHT)) )
    {
                if ( wxUxThemeEngine::GetIfActive() )
        {
            wxUxThemeEngine::GetIfActive()->SetWindowTheme(GetHwnd(), L"", L"");

                        SetBackgroundColour(GetThemeBackgroundColour());
        }
    }
#endif 
    return true;
}

WXDWORD wxNotebook::MSWGetStyle(long style, WXDWORD *exstyle) const
{
    WXDWORD tabStyle = wxControl::MSWGetStyle(style, exstyle);

    tabStyle |= WS_TABSTOP | TCS_TABS;

    if ( style & wxNB_MULTILINE )
        tabStyle |= TCS_MULTILINE;
    if ( style & wxNB_FIXEDWIDTH )
        tabStyle |= TCS_FIXEDWIDTH;

    if ( style & wxBK_BOTTOM )
        tabStyle |= TCS_RIGHT;
    else if ( style & wxBK_LEFT )
        tabStyle |= TCS_VERTICAL;
    else if ( style & wxBK_RIGHT )
        tabStyle |= TCS_VERTICAL | TCS_RIGHT;

    return tabStyle;
}

wxNotebook::~wxNotebook()
{
#if wxUSE_UXTHEME
    if ( m_hbrBackground )
        ::DeleteObject((HBRUSH)m_hbrBackground);
#endif }


size_t wxNotebook::GetPageCount() const
{
        wxASSERT( (int)m_pages.Count() == TabCtrl_GetItemCount(GetHwnd()) );

    return m_pages.Count();
}

int wxNotebook::GetRowCount() const
{
    return TabCtrl_GetRowCount(GetHwnd());
}

int wxNotebook::SetSelection(size_t nPage)
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), wxNOT_FOUND, wxT("notebook page out of range") );

    if ( m_selection == wxNOT_FOUND || nPage != (size_t)m_selection )
    {
        if ( SendPageChangingEvent(nPage) )
        {
                        const int selectionOld = m_selection;

            UpdateSelection(nPage);

            (void)TabCtrl_SetCurSel(GetHwnd(), nPage);

            SendPageChangedEvent(selectionOld, nPage);
        }
    }

    return m_selection;
}

void wxNotebook::UpdateSelection(int selNew)
{
    if ( m_selection != wxNOT_FOUND )
        m_pages[m_selection]->Show(false);

    if ( selNew != wxNOT_FOUND )
    {
        wxNotebookPage *pPage = m_pages[selNew];
        pPage->Show(true);

                                        if ( ::IsWindowVisible(GetHwnd()) )
        {
                                                                        if ( !HasFocus() )
                pPage->SetFocus();
        }
    }

    m_selection = selNew;
}

int wxNotebook::ChangeSelection(size_t nPage)
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), wxNOT_FOUND, wxT("notebook page out of range") );

    const int selOld = m_selection;

    if ( m_selection == wxNOT_FOUND || nPage != (size_t)m_selection )
    {
        (void)TabCtrl_SetCurSel(GetHwnd(), nPage);

        UpdateSelection(nPage);
    }

    return selOld;
}

bool wxNotebook::SetPageText(size_t nPage, const wxString& strText)
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), false, wxT("notebook page out of range") );

    TC_ITEM tcItem;
    tcItem.mask = TCIF_TEXT;
    tcItem.pszText = wxMSW_CONV_LPTSTR(strText);

    if ( !HasFlag(wxNB_MULTILINE) )
        return TabCtrl_SetItem(GetHwnd(), nPage, &tcItem) != 0;

        int rows = GetRowCount();
    bool ret = TabCtrl_SetItem(GetHwnd(), nPage, &tcItem) != 0;

    if ( ret && rows != GetRowCount() )
    {
        const wxRect r = GetPageSize();
        const size_t count = m_pages.Count();
        for ( size_t page = 0; page < count; page++ )
            m_pages[page]->SetSize(r);
    }

    return ret;
}

wxString wxNotebook::GetPageText(size_t nPage) const
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), wxEmptyString, wxT("notebook page out of range") );

    wxChar buf[256];
    TC_ITEM tcItem;
    tcItem.mask = TCIF_TEXT;
    tcItem.pszText = buf;
    tcItem.cchTextMax = WXSIZEOF(buf);

    wxString str;
    if ( TabCtrl_GetItem(GetHwnd(), nPage, &tcItem) )
        str = tcItem.pszText;

    return str;
}

int wxNotebook::GetPageImage(size_t nPage) const
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), wxNOT_FOUND, wxT("notebook page out of range") );

    TC_ITEM tcItem;
    tcItem.mask = TCIF_IMAGE;

    return TabCtrl_GetItem(GetHwnd(), nPage, &tcItem) ? tcItem.iImage
                                                      : wxNOT_FOUND;
}

bool wxNotebook::SetPageImage(size_t nPage, int nImage)
{
    wxCHECK_MSG( IS_VALID_PAGE(nPage), false, wxT("notebook page out of range") );

    TC_ITEM tcItem;
    tcItem.mask = TCIF_IMAGE;
    tcItem.iImage = nImage;

    return TabCtrl_SetItem(GetHwnd(), nPage, &tcItem) != 0;
}

void wxNotebook::SetImageList(wxImageList* imageList)
{
    wxNotebookBase::SetImageList(imageList);

    if ( imageList )
    {
        (void) TabCtrl_SetImageList(GetHwnd(), GetHimagelistOf(imageList));
    }
}


wxRect wxNotebook::GetPageSize() const
{
    wxRect r;

    RECT rc;
    ::GetClientRect(GetHwnd(), &rc);

                            if ( rc.right > 20 && rc.bottom > 20 )
    {
        (void)TabCtrl_AdjustRect(GetHwnd(), false, &rc);

        wxCopyRECTToRect(rc, r);
    }

    return r;
}

void wxNotebook::SetPageSize(const wxSize& size)
{
        RECT rc;
    rc.left =
    rc.top = 0;
    rc.right = size.x;
    rc.bottom = size.y;

    (void)TabCtrl_AdjustRect(GetHwnd(), true, &rc);

        SetSize(rc.right - rc.left, rc.bottom - rc.top);
}

void wxNotebook::SetPadding(const wxSize& padding)
{
    TabCtrl_SetPadding(GetHwnd(), padding.x, padding.y);
}

void wxNotebook::SetTabSize(const wxSize& sz)
{
    ::SendMessage(GetHwnd(), TCM_SETITEMSIZE, 0, MAKELPARAM(sz.x, sz.y));
}

wxSize wxNotebook::CalcSizeFromPage(const wxSize& sizePage) const
{
        wxSize sizeTotal = sizePage;

    wxSize tabSize;
    if ( GetPageCount() > 0 )
    {
        RECT rect;
        if ( TabCtrl_GetItemRect(GetHwnd(), 0, &rect) )
        {
            tabSize.x = rect.right - rect.left;
            tabSize.y = rect.bottom - rect.top;
        }
    }

    const int rows = GetRowCount();

        const int MARGIN = 8;
    if ( IsVertical() )
    {
        sizeTotal.x += MARGIN;
        sizeTotal.y += tabSize.y * rows + MARGIN;
    }
    else     {
        sizeTotal.x += tabSize.x * rows + MARGIN;
        sizeTotal.y += MARGIN;
    }

    return sizeTotal;
}

void wxNotebook::AdjustPageSize(wxNotebookPage *page)
{
    wxCHECK_RET( page, wxT("NULL page in wxNotebook::AdjustPageSize") );

    const wxRect r = GetPageSize();
    if ( !r.IsEmpty() )
    {
        page->SetSize(r);
    }
}


wxNotebookPage *wxNotebook::DoRemovePage(size_t nPage)
{
    wxNotebookPage *pageRemoved = wxNotebookBase::DoRemovePage(nPage);
    if ( !pageRemoved )
        return NULL;

            pageRemoved->Show(false);

    if ( !TabCtrl_DeleteItem(GetHwnd(), nPage) )
        wxLogLastError(wxS("TabCtrl_DeleteItem()"));

    if ( m_pages.IsEmpty() )
    {
                m_selection = wxNOT_FOUND;
    }
    else     {
        int selNew = TabCtrl_GetCurSel(GetHwnd());
        if ( selNew != wxNOT_FOUND )
        {
                                                            m_selection = selNew;
            m_pages[m_selection]->Refresh();
        }
        else if (int(nPage) == m_selection)
        {
            
                        if (m_selection == int(GetPageCount()))
                selNew = m_selection - 1;
            else
                selNew = m_selection;

                                    m_selection = wxNOT_FOUND;
            SetSelection(selNew);
        }
        else
        {
            wxFAIL;         }
    }

    return pageRemoved;
}

bool wxNotebook::DeleteAllPages()
{
    size_t nPageCount = GetPageCount();
    size_t nPage;
    for ( nPage = 0; nPage < nPageCount; nPage++ )
        delete m_pages[nPage];

    m_pages.Clear();

    if ( !TabCtrl_DeleteAllItems(GetHwnd()) )
        wxLogLastError(wxS("TabCtrl_DeleteAllItems()"));

    m_selection = wxNOT_FOUND;

    InvalidateBestSize();
    return true;
}

bool wxNotebook::InsertPage(size_t nPage,
                            wxNotebookPage *pPage,
                            const wxString& strText,
                            bool bSelect,
                            int imageId)
{
    wxCHECK_MSG( pPage != NULL, false, wxT("NULL page in wxNotebook::InsertPage") );
    wxCHECK_MSG( IS_VALID_PAGE(nPage) || nPage == GetPageCount(), false,
                 wxT("invalid index in wxNotebook::InsertPage") );

    wxASSERT_MSG( pPage->GetParent() == this,
                    wxT("notebook pages must have notebook as parent") );

        
        TC_ITEM tcItem;
    wxZeroMemory(tcItem);

        if ( imageId != -1 )
    {
        tcItem.mask |= TCIF_IMAGE;
        tcItem.iImage  = imageId;
    }

        if ( !strText.empty() )
    {
        tcItem.mask |= TCIF_TEXT;
        tcItem.pszText = wxMSW_CONV_LPTSTR(strText);
    }

            pPage->Show(false);


                AdjustPageSize(pPage);

        if ( TabCtrl_InsertItem(GetHwnd(), nPage, &tcItem) == -1 )
    {
        wxLogError(wxT("Can't create the notebook page '%s'."), strText.c_str());

        return false;
    }

            if ( m_pages.empty() )
    {
#if wxUSE_UXTHEME
        UpdateBgBrush();
#endif     }

        m_pages.Insert(pPage, nPage);

                    if ( m_pages.GetCount() == 1 || HasFlag(wxNB_MULTILINE) )
    {
        AdjustPageSize(pPage);

                                                const wxSize s = GetSize();
        ::PostMessage(GetHwnd(), WM_SIZE, SIZE_RESTORED, MAKELPARAM(s.x, s.y));
    }

        
            if ( int(nPage) <= m_selection )
    {
                m_selection++;
    }

    DoSetSelectionAfterInsertion(nPage, bSelect);

    InvalidateBestSize();

    return true;
}

int wxNotebook::HitTest(const wxPoint& pt, long *flags) const
{
    TC_HITTESTINFO hitTestInfo;
    hitTestInfo.pt.x = pt.x;
    hitTestInfo.pt.y = pt.y;
    int item = TabCtrl_HitTest(GetHwnd(), &hitTestInfo);

    if ( flags )
    {
        *flags = 0;

        if ((hitTestInfo.flags & TCHT_NOWHERE) == TCHT_NOWHERE)
        {
            wxASSERT( item == wxNOT_FOUND );
            *flags |= wxBK_HITTEST_NOWHERE;
            if ( GetPageSize().Contains(pt) )
                *flags |= wxBK_HITTEST_ONPAGE;
        }
        else if ((hitTestInfo.flags & TCHT_ONITEM) == TCHT_ONITEM)
            *flags |= wxBK_HITTEST_ONITEM;
        else if ((hitTestInfo.flags & TCHT_ONITEMICON) == TCHT_ONITEMICON)
            *flags |= wxBK_HITTEST_ONICON;
        else if ((hitTestInfo.flags & TCHT_ONITEMLABEL) == TCHT_ONITEMLABEL)
            *flags |= wxBK_HITTEST_ONLABEL;
    }

    return item;
}


#if USE_NOTEBOOK_ANTIFLICKER

LRESULT APIENTRY
wxNotebookSpinBtnWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if ( message == WM_ERASEBKGND )
        return 0;

    return ::CallWindowProc(CASTWNDPROC gs_wndprocNotebookSpinBtn,
                            hwnd, message, wParam, lParam);
}

LRESULT APIENTRY
wxNotebookWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return ::CallWindowProc(CASTWNDPROC gs_wndprocNotebook,
                            hwnd, message, wParam, lParam);
}

void wxNotebook::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
    }

void wxNotebook::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);
    wxMemoryDC memdc;
    RECT rc;
    ::GetClientRect(GetHwnd(), &rc);
    wxBitmap bmp(rc.right, rc.bottom);
    memdc.SelectObject(bmp);

    const wxLayoutDirection dir = dc.GetLayoutDirection();
    memdc.SetLayoutDirection(dir);

    const HDC hdc = GetHdcOf(memdc);

                                                                    
                AutoHBRUSH hbr(wxColourToRGB(GetBackgroundColour()));

    ::FillRect(hdc, &rc, hbr);

    MSWDefWindowProc(WM_PAINT, (WPARAM)hdc, 0);

                if ( UseBgCol() )
    {
        SelectInHDC selectBrush(hdc, hbr);

                                int x = 0,
            y = 0;

        switch ( GetWindowStyle() & wxBK_ALIGN_MASK )
        {
            case wxBK_TOP:
                x = rc.right - 2;
                y = 2;
                break;

            case wxBK_BOTTOM:
                x = rc.right - 2;
                y = rc.bottom - 2;
                break;

            case wxBK_LEFT:
                x = 2;
                y = rc.bottom - 2;
                break;

            case wxBK_RIGHT:
                x = 2;
                y = rc.bottom - 2;
                break;
        }

        ::ExtFloodFill(hdc, x, y, ::GetSysColor(COLOR_BTNFACE), FLOODFILLSURFACE);
    }

            const wxCoord ofs = dir == wxLayout_RightToLeft ? -1 : 0;
    dc.Blit(ofs, 0, rc.right, rc.bottom, &memdc, ofs, 0);
}

#endif 

void wxNotebook::OnSize(wxSizeEvent& event)
{
    if ( GetPageCount() == 0 )
    {
                        Refresh();
        event.Skip();
        return;
    }
    else
    {
                                        
        wxSize sz = GetClientSize();

                wxRect rect(sz.x-4, 0, 4, sz.y);
        RefreshRect(rect);

                rect = wxRect(0, sz.y-4, sz.x, 4);
        RefreshRect(rect);

                rect = wxRect(0, 0, 4, sz.y);
        RefreshRect(rect);
    }

    
    RECT rc;
    rc.left = rc.top = 0;
    GetSize((int *)&rc.right, (int *)&rc.bottom);

        int widthNbook = rc.right - rc.left,
        heightNbook = rc.bottom - rc.top;

                                        if ( HasFlag(wxNB_MULTILINE) )
    {
                static bool s_isInOnSize = false;

        if ( !s_isInOnSize )
        {
            s_isInOnSize = true;
            SendMessage(GetHwnd(), WM_SIZE, SIZE_RESTORED,
                    MAKELPARAM(rc.right, rc.bottom));
            s_isInOnSize = false;
        }

                        InvalidateBestSize();
    }

#if wxUSE_UXTHEME
        UpdateBgBrush();
#endif 
    (void)TabCtrl_AdjustRect(GetHwnd(), false, &rc);

    int width = rc.right - rc.left,
        height = rc.bottom - rc.top;
    size_t nCount = m_pages.Count();
    for ( size_t nPage = 0; nPage < nCount; nPage++ ) {
        wxNotebookPage *pPage = m_pages[nPage];
        pPage->SetSize(rc.left, rc.top, width, height);
    }


        if ( !HasFlag(wxFULL_REPAINT_ON_RESIZE) )
    {
                RefreshRect(wxRect(0, 0, widthNbook, rc.top), false);
        RefreshRect(wxRect(0, rc.top, rc.left, height), false);
        RefreshRect(wxRect(0, rc.bottom, widthNbook, heightNbook - rc.bottom),
                    false);
        RefreshRect(wxRect(rc.right, rc.top, widthNbook - rc.right, height),
                    false);
    }

#if USE_NOTEBOOK_ANTIFLICKER
            if ( !m_hasSubclassedUpdown )
    {
                for ( HWND child = ::GetWindow(GetHwnd(), GW_CHILD);
              child;
              child = ::GetWindow(child, GW_HWNDNEXT) )
        {
            wxWindow *childWindow = wxFindWinFromHandle((WXHWND)child);

                                    if ( !childWindow )
            {
                                if ( !gs_wndprocNotebookSpinBtn )
                    gs_wndprocNotebookSpinBtn = (WXFARPROC)wxGetWindowProc(child);

                wxSetWindowProc(child, wxNotebookSpinBtnWndProc);
                m_hasSubclassedUpdown = true;
                break;
            }
        }
    }
#endif 
    event.Skip();
}

void wxNotebook::OnNavigationKey(wxNavigationKeyEvent& event)
{
    if ( event.IsWindowChange() ) {
                AdvanceSelection(event.GetDirection());
    }
    else {
                                                                                                                                                        wxWindow * const parent = GetParent();

                const bool isFromParent = event.GetEventObject() == (wxObject*) parent;
        const bool isFromSelf = event.GetEventObject() == (wxObject*) this;
        const bool isForward = event.GetDirection();

        if ( isFromSelf && !isForward )
        {
                                    event.SetCurrentFocus(this);
            parent->HandleWindowEvent(event);
        }
        else if ( isFromParent || isFromSelf )
        {
                                                            if ( m_selection != wxNOT_FOUND &&
                    (!event.GetDirection() || isFromSelf) )
            {
                                                event.SetEventObject(this);

                wxWindow *page = m_pages[m_selection];
                if ( !page->HandleWindowEvent(event) )
                {
                    page->SetFocus();
                }
                            }
            else             {
                SetFocus();
            }
        }
        else
        {
                                                            if ( !isForward )
            {
                SetFocus();
            }
            else if ( parent )
            {
                event.SetCurrentFocus(this);
                parent->HandleWindowEvent(event);
            }
        }
    }
}

#if wxUSE_UXTHEME

bool wxNotebook::DoDrawBackground(WXHDC hDC, wxWindow *child)
{
    wxUxThemeHandle theme(child ? child : this, L"TAB");
    if ( !theme )
        return false;

            wxRect r = GetPageSize();
    if ( r.IsEmpty() )
        return false;

    RECT rc;
    wxCopyRectToRECT(r, rc);

        if ( child )
        ::MapWindowPoints(GetHwnd(), GetHwndOf(child), (POINT *)&rc, 2);

            wxUxThemeEngine::Get()->GetThemeBackgroundExtent
                            (
                                theme,
                                (HDC) hDC,
                                9 ,
                                0,
                                &rc,
                                &rc
                            );
    wxUxThemeEngine::Get()->DrawThemeBackground
                            (
                                theme,
                                (HDC) hDC,
                                9 ,
                                0,
                                &rc,
                                NULL
                            );

    return true;
}

WXHBRUSH wxNotebook::QueryBgBitmap()
{
    wxRect r = GetPageSize();
    if ( r.IsEmpty() )
        return 0;

    WindowHDC hDC(GetHwnd());
    MemoryHDC hDCMem(hDC);
    CompatibleBitmap hBmp(hDC, r.x + r.width, r.y + r.height);

    SelectInHDC selectBmp(hDCMem, hBmp);

    if ( !DoDrawBackground((WXHDC)(HDC)hDCMem) )
        return 0;

    return (WXHBRUSH)::CreatePatternBrush(hBmp);
}

void wxNotebook::UpdateBgBrush()
{
    if ( m_hbrBackground )
        ::DeleteObject((HBRUSH)m_hbrBackground);

    if ( !m_hasBgCol && wxUxThemeEngine::GetIfActive() )
    {
        m_hbrBackground = QueryBgBitmap();
    }
    else     {
        m_hbrBackground = NULL;
    }
}

bool wxNotebook::MSWPrintChild(WXHDC hDC, wxWindow *child)
{
        if ( !UseBgCol() && DoDrawBackground(hDC, child) )
        return true;

            if (UseBgCol())
    {
        wxRect r = GetPageSize();
        if ( r.IsEmpty() )
            return false;

        RECT rc;
        wxCopyRectToRECT(r, rc);

                if ( child )
            ::MapWindowPoints(GetHwnd(), GetHwndOf(child), (POINT *)&rc, 2);

        wxBrush brush(GetBackgroundColour());
        HBRUSH hbr = GetHbrushOf(brush);

        ::FillRect((HDC) hDC, &rc, hbr);

        return true;
    }

    return wxNotebookBase::MSWPrintChild(hDC, child);
}

#endif 
wxColour wxNotebook::GetThemeBackgroundColour() const
{
#if wxUSE_UXTHEME
    if (wxUxThemeEngine::Get())
    {
        wxUxThemeHandle hTheme((wxNotebook*) this, L"TAB");
        if (hTheme)
        {
                                                COLORREF themeColor;
            bool success = (S_OK == wxUxThemeEngine::Get()->GetThemeColor(
                                        hTheme,
                                        10 ,
                                        1 ,
                                        3821 ,
                                        &themeColor));
            if (!success)
                return GetBackgroundColour();

            
            if (themeColor == 1)
            {
                wxUxThemeEngine::Get()->GetThemeColor(
                                            hTheme,
                                            10 ,
                                            1 ,
                                            3802 ,
                                            &themeColor);
            }

            wxColour colour = wxRGBToColour(themeColor);

                                    
            static int s_AeroStatus = -1;
            if (s_AeroStatus == -1)
            {
                WCHAR szwThemeFile[1024];
                WCHAR szwThemeColor[256];
                if (S_OK == wxUxThemeEngine::Get()->GetCurrentThemeName(szwThemeFile, 1024, szwThemeColor, 256, NULL, 0))
                {
                    wxString themeFile(szwThemeFile);
                    if (themeFile.Find(wxT("Aero")) != -1 && wxString(szwThemeColor) == wxT("NormalColor"))
                        s_AeroStatus = 1;
                    else
                        s_AeroStatus = 0;
                }
                else
                    s_AeroStatus = 0;
            }

            if (s_AeroStatus == 1)
                colour = wxColour(255, 255, 255);

            return colour;
        }
    }
#endif 
    return GetBackgroundColour();
}


#if wxUSE_CONSTRAINTS


void wxNotebook::SetConstraintSizes(bool WXUNUSED(recurse))
{
    wxControl::SetConstraintSizes(false);
}

bool wxNotebook::DoPhase(int WXUNUSED(nPhase))
{
  return true;
}

#endif 

bool wxNotebook::MSWOnScroll(int orientation, WXWORD nSBCode,
                             WXWORD pos, WXHWND control)
{
            if ( control )
        return false;

    return wxNotebookBase::MSWOnScroll(orientation, nSBCode, pos, control);
}

bool wxNotebook::MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM* result)
{
  wxBookCtrlEvent event(wxEVT_NULL, m_windowId);

  NMHDR* hdr = (NMHDR *)lParam;
  switch ( hdr->code ) {
    case TCN_SELCHANGE:
      event.SetEventType(wxEVT_NOTEBOOK_PAGE_CHANGED);
      break;

    case TCN_SELCHANGING:
      event.SetEventType(wxEVT_NOTEBOOK_PAGE_CHANGING);
      break;

    default:
      return wxControl::MSWOnNotify(idCtrl, lParam, result);
  }

  event.SetSelection(TabCtrl_GetCurSel(GetHwnd()));
  event.SetOldSelection(m_selection);
  event.SetEventObject(this);
  event.SetInt(idCtrl);

      if ( hdr->code == TCN_SELCHANGE )
      UpdateSelection(event.GetSelection());

  bool processed = HandleWindowEvent(event);
  *result = !event.IsAllowed();
  return processed;
}

#endif 