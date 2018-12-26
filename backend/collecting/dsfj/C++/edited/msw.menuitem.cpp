


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_MENUS

#include "wx/menuitem.h"
#include "wx/stockitem.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/dcmemory.h"
    #include "wx/font.h"
    #include "wx/bitmap.h"
    #include "wx/settings.h"
    #include "wx/window.h"
    #include "wx/accel.h"
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/menu.h"
#endif

#if wxUSE_ACCEL
    #include "wx/accel.h"
#endif 
#include "wx/msw/private.h"
#include "wx/msw/dc.h"
#include "wx/msw/uxtheme.h"


#define GetHMenuOf(menu)    ((HMENU)menu->GetHMenu())


namespace
{

class HDCHandler
{
protected:
    HDCHandler(HDC hdc) : m_hdc(hdc) { }

    const HDC m_hdc;
};

class HDCTextColChanger : HDCHandler
{
public:
    HDCTextColChanger(HDC hdc, COLORREF col)
        : HDCHandler(hdc),
          m_colOld(::SetTextColor(hdc, col))
    {
    }

    ~HDCTextColChanger()
    {
        ::SetTextColor(m_hdc, m_colOld);
    }

private:
    COLORREF m_colOld;
};

class HDCBgColChanger : HDCHandler
{
public:
    HDCBgColChanger(HDC hdc, COLORREF col)
        : HDCHandler(hdc),
          m_colOld(::SetBkColor(hdc, col))
    {
    }

    ~HDCBgColChanger()
    {
        ::SetBkColor(m_hdc, m_colOld);
    }

private:
    COLORREF m_colOld;
};

class HDCBgModeChanger : HDCHandler
{
public:
    HDCBgModeChanger(HDC hdc, int mode)
        : HDCHandler(hdc),
          m_modeOld(::SetBkMode(hdc, mode))
    {
    }

    ~HDCBgModeChanger()
    {
        ::SetBkMode(m_hdc, m_modeOld);
    }

private:
    int m_modeOld;
};

inline bool IsGreaterThanStdSize(const wxBitmap& bmp)
{
    return bmp.GetWidth() > ::GetSystemMetrics(SM_CXMENUCHECK) ||
            bmp.GetHeight() > ::GetSystemMetrics(SM_CYMENUCHECK);
}

} 

#if wxUSE_OWNER_DRAWN

#include "wx/fontutil.h"
#include "wx/msw/private/metrics.h"

#ifndef SPI_GETKEYBOARDCUES
#define SPI_GETKEYBOARDCUES 0x100A
#endif

#if wxUSE_UXTHEME

enum MENUPARTS
{
    MENU_MENUITEM_TMSCHEMA = 1,
    MENU_SEPARATOR_TMSCHEMA = 6,
    MENU_POPUPBACKGROUND = 9,
    MENU_POPUPBORDERS = 10,
    MENU_POPUPCHECK = 11,
    MENU_POPUPCHECKBACKGROUND = 12,
    MENU_POPUPGUTTER = 13,
    MENU_POPUPITEM = 14,
    MENU_POPUPSEPARATOR = 15,
    MENU_POPUPSUBMENU = 16,
};


enum POPUPITEMSTATES
{
    MPI_NORMAL = 1,
    MPI_HOT = 2,
    MPI_DISABLED = 3,
    MPI_DISABLEDHOT = 4,
};

enum POPUPCHECKBACKGROUNDSTATES
{
    MCB_DISABLED = 1,
    MCB_NORMAL = 2,
    MCB_BITMAP = 3,
};

enum POPUPCHECKSTATES
{
    MC_CHECKMARKNORMAL = 1,
    MC_CHECKMARKDISABLED = 2,
    MC_BULLETNORMAL = 3,
    MC_BULLETDISABLED = 4,
};

const int TMT_MENUFONT       = 803;
const int TMT_BORDERSIZE     = 2403;
const int TMT_CONTENTMARGINS = 3602;
const int TMT_SIZINGMARGINS  = 3601;

#endif 
#endif 


#if wxUSE_OWNER_DRAWN

namespace
{

class MenuDrawData
{
public:
            struct Margins : MARGINS
    {
        Margins()
        {
            cxLeftWidth =
            cxRightWidth =
            cyTopHeight =
            cyBottomHeight = 0;
        }

        int GetTotalX() const { return cxLeftWidth + cxRightWidth; }
        int GetTotalY() const { return cyTopHeight + cyBottomHeight; }

        void ApplyTo(RECT& rect) const
        {
            rect.top += cyTopHeight;
            rect.left += cxLeftWidth;
            rect.right -= cyTopHeight;
            rect.bottom -= cyBottomHeight;
        }

        void UnapplyFrom(RECT& rect) const
        {
            rect.top -= cyTopHeight;
            rect.left -= cxLeftWidth;
            rect.right += cyTopHeight;
            rect.bottom += cyBottomHeight;
        }
    };

    Margins ItemMargin;         
    Margins CheckMargin;            Margins CheckBgMargin;      
    Margins ArrowMargin;        
    Margins SeparatorMargin;    
    SIZE CheckSize;                 SIZE ArrowSize;                 SIZE SeparatorSize;         
    int TextBorder;                                             
    int AccelBorder;                                            
    int ArrowBorder;                                            
    int Offset;                                                 
    wxFont Font;                
    bool AlwaysShowCues;        
    bool Theme;                 
    static const MenuDrawData* Get()
    {
                                if ( !ms_instance )
        {
            static MenuDrawData s_menuData;
            ms_instance = &s_menuData;
        }

    #if wxUSE_UXTHEME
        bool theme = MenuLayout() == FullTheme;
        if ( ms_instance->Theme != theme )
            ms_instance->Init();
    #endif         return ms_instance;
    }

    MenuDrawData()
    {
        Init();
    }


            static wxUxThemeEngine *GetUxThemeEngine()
    {
    #if wxUSE_UXTHEME
        if ( MenuLayout() == FullTheme )
            return wxUxThemeEngine::GetIfActive();
    #endif         return NULL;
    }


    enum MenuLayoutType
    {
        FullTheme,              PseudoTheme,            Classic
    };

    static MenuLayoutType MenuLayout()
    {
        MenuLayoutType menu = Classic;
    #if wxUSE_UXTHEME
        if ( wxUxThemeEngine::GetIfActive() != NULL )
        {
            static wxWinVersion ver = wxGetWinVersion();
            if ( ver >= wxWinVersion_Vista )
                menu = FullTheme;
            else if ( ver == wxWinVersion_XP )
                menu = PseudoTheme;
        }
    #endif         return menu;
    }

private:
    void Init();

    static MenuDrawData* ms_instance;
};

MenuDrawData* MenuDrawData::ms_instance = NULL;

void MenuDrawData::Init()
{
#if wxUSE_UXTHEME
    wxUxThemeEngine* theme = GetUxThemeEngine();
    if ( theme )
    {
        wxWindow* window = static_cast<wxApp*>(wxApp::GetInstance())->GetTopWindow();
        wxUxThemeHandle hTheme(window, L"MENU");

        theme->GetThemeMargins(hTheme, NULL, MENU_POPUPITEM, 0,
                               TMT_CONTENTMARGINS, NULL,
                               &ItemMargin);

        theme->GetThemeMargins(hTheme, NULL, MENU_POPUPCHECK, 0,
                               TMT_CONTENTMARGINS, NULL,
                               &CheckMargin);
        theme->GetThemeMargins(hTheme, NULL, MENU_POPUPCHECKBACKGROUND, 0,
                               TMT_CONTENTMARGINS, NULL,
                               &CheckBgMargin);

        theme->GetThemeMargins(hTheme, NULL, MENU_POPUPSUBMENU, 0,
                               TMT_CONTENTMARGINS, NULL,
                               &ArrowMargin);

        theme->GetThemeMargins(hTheme, NULL, MENU_POPUPSEPARATOR, 0,
                               TMT_SIZINGMARGINS, NULL,
                               &SeparatorMargin);

        theme->GetThemePartSize(hTheme, NULL, MENU_POPUPCHECK, 0,
                                NULL, TS_TRUE, &CheckSize);

        theme->GetThemePartSize(hTheme, NULL, MENU_POPUPSUBMENU, 0,
                                NULL, TS_TRUE, &ArrowSize);

        theme->GetThemePartSize(hTheme, NULL, MENU_POPUPSEPARATOR, 0,
                                NULL, TS_TRUE, &SeparatorSize);

        theme->GetThemeInt(hTheme, MENU_POPUPBACKGROUND, 0, TMT_BORDERSIZE, &TextBorder);

        AccelBorder = 34;
        ArrowBorder = 0;

        Offset = -14;

        wxUxThemeFont themeFont;
        theme->GetThemeSysFont(hTheme, TMT_MENUFONT, themeFont.GetPtr());
        Font = wxFont(themeFont.GetLOGFONT());

        Theme = true;

                ItemMargin.cyTopHeight =
        ItemMargin.cyBottomHeight = 0;

                if ( SeparatorMargin.cyTopHeight >= 2 )
            SeparatorMargin.cyTopHeight -= 2;
    }
    else
#endif     {
        const NONCLIENTMETRICS& metrics = wxMSWImpl::GetNonClientMetrics();

        CheckMargin.cxLeftWidth =
        CheckMargin.cxRightWidth  = ::GetSystemMetrics(SM_CXEDGE);
        CheckMargin.cyTopHeight =
        CheckMargin.cyBottomHeight = ::GetSystemMetrics(SM_CYEDGE);

        CheckSize.cx = ::GetSystemMetrics(SM_CXMENUCHECK);
        CheckSize.cy = ::GetSystemMetrics(SM_CYMENUCHECK);

        ArrowSize = CheckSize;

                int sepFullSize = metrics.iMenuHeight / 2;

        SeparatorMargin.cxLeftWidth =
        SeparatorMargin.cxRightWidth = 1;
        SeparatorMargin.cyTopHeight =
        SeparatorMargin.cyBottomHeight = sepFullSize / 2 - 1;

        SeparatorSize.cx = 1;
        SeparatorSize.cy = sepFullSize - SeparatorMargin.GetTotalY();

        TextBorder = 0;
        AccelBorder = 8;
        ArrowBorder = 6;

        Offset = -12;

        Font = wxFont(wxNativeFontInfo(metrics.lfMenuFont));

        Theme = false;
    }

    int value;
    if ( ::SystemParametersInfo(SPI_GETKEYBOARDCUES, 0, &value, 0) == 0 )
    {
                        value = 1;
    }

    AlwaysShowCues = value == 1;

}

} 
#endif 


wxMenuItem::wxMenuItem(wxMenu *pParentMenu,
                       int id,
                       const wxString& text,
                       const wxString& strHelp,
                       wxItemKind kind,
                       wxMenu *pSubMenu)
          : wxMenuItemBase(pParentMenu, id, text, strHelp, kind, pSubMenu)
{
    Init();
}

#if WXWIN_COMPATIBILITY_2_8
wxMenuItem::wxMenuItem(wxMenu *parentMenu,
                       int id,
                       const wxString& text,
                       const wxString& help,
                       bool isCheckable,
                       wxMenu *subMenu)
          : wxMenuItemBase(parentMenu, id, text, help,
                           isCheckable ? wxITEM_CHECK : wxITEM_NORMAL, subMenu)
{
    Init();
}
#endif

void wxMenuItem::Init()
{
#if  wxUSE_OWNER_DRAWN

                    SetTextColour(wxNullColour);
    SetBackgroundColour(wxNullColour);

        SetOwnerDrawn(false);

        if ( !IsSeparator() )
        SetMarginWidth(GetMarginWidth());

#endif }

wxMenuItem::~wxMenuItem()
{
}


WXWPARAM wxMenuItem::GetMSWId() const
{
                    return m_subMenu ? wxPtrToUInt(m_subMenu->GetHMenu())
                     : static_cast<unsigned short>(GetId());
}


bool wxMenuItem::IsChecked() const
{
            if ( IsSeparator() )
        return false;

                    if ( !m_parentMenu )
        return wxMenuItemBase::IsChecked();

    HMENU hmenu = GetHMenuOf(m_parentMenu);
    int flag = ::GetMenuState(hmenu, GetMSWId(), MF_BYCOMMAND);

    return (flag & MF_CHECKED) != 0;
}


void wxMenuItem::Enable(bool enable)
{
    if ( m_isEnabled == enable )
        return;

    const int itemPos = MSGetMenuItemPos();
    if ( itemPos != -1 )
    {
        long rc = EnableMenuItem(GetHMenuOf(m_parentMenu),
                                 itemPos,
                                 MF_BYPOSITION |
                                 (enable ? MF_ENABLED : MF_GRAYED));

        if ( rc == -1 )
        {
            wxLogLastError(wxT("EnableMenuItem"));
        }
    }

    wxMenuItemBase::Enable(enable);
}

void wxMenuItem::Check(bool check)
{
    wxCHECK_RET( IsCheckable(), wxT("only checkable items may be checked") );

    if ( m_isChecked == check )
        return;

    if ( m_parentMenu )
    {
        int flags = check ? MF_CHECKED : MF_UNCHECKED;
        HMENU hmenu = GetHMenuOf(m_parentMenu);

        if ( GetKind() == wxITEM_RADIO )
        {
                                    if ( !check )
                return;

                        const wxMenuItemList& items = m_parentMenu->GetMenuItems();
            int pos = items.IndexOf(this);
            wxCHECK_RET( pos != wxNOT_FOUND,
                         wxT("menuitem not found in the menu items list?") );

                        int start,
                end;

            if ( !m_parentMenu->MSWGetRadioGroupRange(pos, &start, &end) )
            {
                wxFAIL_MSG( wxT("Menu radio item not part of radio group?") );
                return;
            }

                                                wxCHECK_RET( start != -1 && end != -1,
                         wxT("invalid ::CheckMenuRadioItem() parameter(s)") );

            if ( !::CheckMenuRadioItem(hmenu,
                                       start,                                          end,                                            pos,                                            MF_BYPOSITION) )
            {
                wxLogLastError(wxT("CheckMenuRadioItem"));
            }

                        wxMenuItemList::compatibility_iterator node = items.Item(start);
            for ( int n = start; n <= end && node; n++ )
            {
                if ( n != pos )
                {
                    node->GetData()->m_isChecked = false;
                }

                node = node->GetNext();
            }
        }
        else         {
            if ( ::CheckMenuItem(hmenu,
                                 GetMSWId(),
                                 MF_BYCOMMAND | flags) == (DWORD)-1 )
            {
                wxFAIL_MSG(wxT("CheckMenuItem() failed, item not in the menu?"));
            }
        }
    }

    wxMenuItemBase::Check(check);
}

void wxMenuItem::SetItemLabel(const wxString& txt)
{
    wxString text = txt;

        if ( m_text == txt )
        return;

        wxMenuItemBase::SetItemLabel(text);

            if ( !m_parentMenu )
        return;

#if wxUSE_ACCEL
    m_parentMenu->UpdateAccel(this);
#endif 
    const int itemPos = MSGetMenuItemPos();
    if ( itemPos == -1 )
        return;

    HMENU hMenu = GetHMenuOf(m_parentMenu);

        WinStruct<MENUITEMINFO> info;

                info.fMask = MIIM_STATE |
                 MIIM_ID |
                 MIIM_SUBMENU |
                 MIIM_CHECKMARKS |
                 MIIM_DATA |
                 MIIM_BITMAP |
                 MIIM_FTYPE;
    if ( !::GetMenuItemInfo(hMenu, itemPos, TRUE, &info) )
    {
        wxLogLastError(wxT("GetMenuItemInfo"));
        return;
    }

#if wxUSE_OWNER_DRAWN
                                                        if ( !(info.fState & MF_OWNERDRAW) )
#endif     {
        info.fMask |= MIIM_STRING;
        info.dwTypeData = wxMSW_CONV_LPTSTR(m_text);
        info.cch = m_text.length();
    }

    if ( !::SetMenuItemInfo(hMenu, itemPos, TRUE, &info) )
    {
        wxLogLastError(wxT("SetMenuItemInfo"));
    }
}

void wxMenuItem::DoSetBitmap(const wxBitmap& bmpNew, bool bChecked)
{
    wxBitmap& bmp = bChecked ? m_bmpChecked : m_bmpUnchecked;
    if ( bmp.IsSameAs(bmpNew) )
        return;

#if wxUSE_IMAGE
    if ( !bmpNew.HasAlpha() && wxGetWinVersion() >= wxWinVersion_Vista)
    {
                wxImage img(bmpNew.ConvertToImage());
        img.InitAlpha();
        bmp = wxBitmap(img);
    }
    else
#endif     {
        bmp = bmpNew;
    }

#if wxUSE_OWNER_DRAWN
        if ( IsOwnerDrawn() )
        return;

    if ( MSWMustUseOwnerDrawn() )
    {
        SetOwnerDrawn(true);

                        if ( m_parentMenu )
        {
            size_t pos;
            wxMenuItem *item = m_parentMenu->FindChildItem(GetId(), &pos);
            if ( item )
            {
                wxCHECK_RET( item == this, wxS("Non unique menu item ID?") );

                                                wxMenu *menu = m_parentMenu;
                menu->Remove(this);
                menu->Insert(pos, this);
            }
                    }
        return;
    }
#endif 
    const int itemPos = MSGetMenuItemPos();
    if ( itemPos == -1 )
    {
                        return;
    }

                WinStruct<MENUITEMINFO> mii;
    if ( IsCheckable() )
    {
        mii.fMask = MIIM_CHECKMARKS;
        mii.hbmpChecked = GetHBitmapForMenu(Checked);
        mii.hbmpUnchecked = GetHBitmapForMenu(Unchecked);
    }
    else
    {
        mii.fMask = MIIM_BITMAP;
        mii.hbmpItem = GetHBitmapForMenu(Normal);
    }

    if ( !::SetMenuItemInfo(GetHMenuOf(m_parentMenu), itemPos, TRUE, &mii) )
    {
        wxLogLastError(wxT("SetMenuItemInfo"));
    }
}

#if wxUSE_OWNER_DRAWN

int wxMenuItem::MeasureAccelWidth() const
{
    wxString accel = GetItemLabel().AfterFirst(wxT('\t'));

    wxMemoryDC dc;
    wxFont font;
    GetFontToUse(font);
    dc.SetFont(font);

    wxCoord w;
    dc.GetTextExtent(accel, &w, NULL);

    return w;
}

wxString wxMenuItem::GetName() const
{
    return GetItemLabelText();
}

bool wxMenuItem::OnMeasureItem(size_t *width, size_t *height)
{
    const MenuDrawData* data = MenuDrawData::Get();

    if ( IsOwnerDrawn() )
    {
        *width  = data->ItemMargin.GetTotalX();
        *height = data->ItemMargin.GetTotalY();

        if ( IsSeparator() )
        {
            *width  += data->SeparatorSize.cx
                     + data->SeparatorMargin.GetTotalX();
            *height += data->SeparatorSize.cy
                     + data->SeparatorMargin.GetTotalY();
            return true;
        }

        wxString str = GetName();

        wxMemoryDC dc;
        wxFont font;
        GetFontToUse(font);
        dc.SetFont(font);

        wxCoord w, h;
        dc.GetTextExtent(str, &w, &h);

        *width = data->TextBorder + w + data->AccelBorder;
        *height = h;

        w = m_parentMenu->GetMaxAccelWidth();
        if ( w > 0 )
            *width += w + data->ArrowBorder;

        *width += data->Offset;
        *width += data->ArrowMargin.GetTotalX() + data->ArrowSize.cx;
    }
    else     {
        *width = 0;
        *height = 0;
    }

    
    if ( IsOwnerDrawn() )
    {
                                                        int imgWidth = wxMax(GetMarginWidth(), data->CheckSize.cx)
                     + data->CheckMargin.GetTotalX();

        *width += imgWidth + data->CheckBgMargin.GetTotalX();
    }

    if ( m_bmpChecked.IsOk() || m_bmpUnchecked.IsOk() )
    {
                        size_t heightBmp = wxMax(m_bmpChecked.GetHeight(), m_bmpUnchecked.GetHeight());
        size_t widthBmp = wxMax(m_bmpChecked.GetWidth(),  m_bmpUnchecked.GetWidth());

        if ( IsOwnerDrawn() )
        {
            heightBmp += data->CheckMargin.GetTotalY();
        }
        else
        {
                        *width += widthBmp;
        }

                if ( *height < heightBmp )
            *height = heightBmp;
    }

        const size_t menuHeight = data->CheckMargin.GetTotalY()
                            + data->CheckSize.cy;
    if (*height < menuHeight)
        *height = menuHeight;

    return true;
}

bool wxMenuItem::OnDrawItem(wxDC& dc, const wxRect& rc,
                            wxODAction WXUNUSED(act), wxODStatus stat)
{
    const MenuDrawData* data = MenuDrawData::Get();

    wxMSWDCImpl *impl = (wxMSWDCImpl*) dc.GetImpl();
    HDC hdc = GetHdcOf(*impl);

    RECT rect;
    wxCopyRectToRECT(rc, rect);

    int imgWidth = wxMax(GetMarginWidth(), data->CheckSize.cx);

    if ( IsOwnerDrawn() )
    {
                wxFont font;
        GetFontToUse(font);

        wxColour colText, colBack;
        GetColourToUse(stat, colText, colBack);

                RECT rcSelection = rect;
        data->ItemMargin.ApplyTo(rcSelection);

        RECT rcSeparator = rcSelection;
        data->SeparatorMargin.ApplyTo(rcSeparator);

        RECT rcGutter = rcSelection;
        rcGutter.right = data->ItemMargin.cxLeftWidth
                       + data->CheckBgMargin.cxLeftWidth
                       + data->CheckMargin.cxLeftWidth
                       + imgWidth
                       + data->CheckMargin.cxRightWidth
                       + data->CheckBgMargin.cxRightWidth;

        RECT rcText = rcSelection;
        rcText.left = rcGutter.right + data->TextBorder;

                        if ( data->MenuLayout() != MenuDrawData::FullTheme )
            rcText.top--;

#if wxUSE_UXTHEME
                        wxUxThemeEngine* const theme = GetBackgroundColour().IsOk()
                                        ? NULL
                                        : MenuDrawData::GetUxThemeEngine();
        if ( theme )
        {
            POPUPITEMSTATES state;
            if ( stat & wxODDisabled )
            {
                state = (stat & wxODSelected) ? MPI_DISABLEDHOT
                                              : MPI_DISABLED;
            }
            else if ( stat & wxODSelected )
            {
                state = MPI_HOT;
            }
            else
            {
                state = MPI_NORMAL;
            }

            wxUxThemeHandle hTheme(GetMenu()->GetWindow(), L"MENU");

            if ( theme->IsThemeBackgroundPartiallyTransparent(hTheme,
                    MENU_POPUPITEM, state) )
            {
                theme->DrawThemeBackground(hTheme, hdc,
                                           MENU_POPUPBACKGROUND,
                                           0, &rect, NULL);
            }

            theme->DrawThemeBackground(hTheme, hdc, MENU_POPUPGUTTER,
                                       0, &rcGutter, NULL);

            if ( IsSeparator() )
            {
                rcSeparator.left = rcGutter.right;
                theme->DrawThemeBackground(hTheme, hdc, MENU_POPUPSEPARATOR,
                                           0, &rcSeparator, NULL);
                return true;
            }

            theme->DrawThemeBackground(hTheme, hdc, MENU_POPUPITEM,
                                       state, &rcSelection, NULL);

        }
        else
#endif         {
            if ( IsSeparator() )
            {
                DrawEdge(hdc, &rcSeparator, EDGE_ETCHED, BF_TOP);
                return true;
            }

            AutoHBRUSH hbr(colBack.GetPixel());
            SelectInHDC selBrush(hdc, hbr);
            ::FillRect(hdc, &rcSelection, hbr);
        }


                
        HDCTextColChanger changeTextCol(hdc, colText.GetPixel());
        HDCBgColChanger changeBgCol(hdc, colBack.GetPixel());
        HDCBgModeChanger changeBgMode(hdc, TRANSPARENT);

        SelectInHDC selFont(hdc, GetHfontOf(font));


                wxString text = GetName();

        SIZE textSize;
        ::GetTextExtentPoint32(hdc, text.c_str(), text.length(), &textSize);

                text = GetItemLabel().BeforeFirst('\t');

        int flags = DST_PREFIXTEXT;
                if ( data->MenuLayout() == MenuDrawData::Classic &&
             (stat & wxODDisabled) && !(stat & wxODSelected) )
            flags |= DSS_DISABLED;

        if ( (stat & wxODHidePrefix) && !data->AlwaysShowCues )
            flags |= DSS_HIDEPREFIX;

        int x = rcText.left;
        int y = rcText.top + (rcText.bottom - rcText.top - textSize.cy) / 2;

        ::DrawState(hdc, NULL, NULL, wxMSW_CONV_LPARAM(text),
                    text.length(), x, y, 0, 0, flags);

                                wxString accel = GetItemLabel().AfterFirst(wxT('\t'));
        if ( !accel.empty() )
        {
            SIZE accelSize;
            ::GetTextExtentPoint32(hdc, accel.c_str(), accel.length(), &accelSize);

            flags = DST_TEXT;
                        if ( data->MenuLayout() == MenuDrawData::Classic &&
                 (stat & wxODDisabled) && !(stat & wxODSelected) )
                flags |= DSS_DISABLED;

            x = rcText.right - data->ArrowMargin.GetTotalX()
                                 - data->ArrowSize.cx
                                 - data->ArrowBorder;

                        if ( data->MenuLayout() == MenuDrawData::FullTheme)
                x -= accelSize.cx;
            else
                x -= m_parentMenu->GetMaxAccelWidth();

            y = rcText.top + (rcText.bottom - rcText.top - accelSize.cy) / 2;

            ::DrawState(hdc, NULL, NULL, wxMSW_CONV_LPARAM(accel),
                        accel.length(), x, y, 0, 0, flags);
        }
    }


    
    RECT rcImg;
    SetRect(&rcImg,
            rect.left   + data->ItemMargin.cxLeftWidth
                        + data->CheckBgMargin.cxLeftWidth
                        + data->CheckMargin.cxLeftWidth,
            rect.top    + data->ItemMargin.cyTopHeight
                        + data->CheckBgMargin.cyTopHeight
                        + data->CheckMargin.cyTopHeight,
            rect.left   + data->ItemMargin.cxLeftWidth
                        + data->CheckBgMargin.cxLeftWidth
                        + data->CheckMargin.cxLeftWidth
                        + imgWidth,
            rect.bottom - data->ItemMargin.cyBottomHeight
                        - data->CheckBgMargin.cyBottomHeight
                        - data->CheckMargin.cyBottomHeight);

    if ( IsCheckable() && !m_bmpChecked.IsOk() )
    {
        if ( stat & wxODChecked )
        {
            DrawStdCheckMark((WXHDC)hdc, &rcImg, stat);
        }
    }
    else
    {
        wxBitmap bmp;

        if ( stat & wxODDisabled )
        {
            bmp = GetDisabledBitmap();
        }

        if ( !bmp.IsOk() )
        {
                                    bmp = GetBitmap(!IsCheckable() || (stat & wxODChecked));

#if wxUSE_IMAGE
            if ( bmp.IsOk() && stat & wxODDisabled )
            {
                                                wxImage imgGrey = bmp.ConvertToImage().ConvertToGreyscale();
                if ( imgGrey.IsOk() )
                    bmp = wxBitmap(imgGrey);
            }
#endif         }

        if ( bmp.IsOk() )
        {
            wxMemoryDC dcMem(&dc);
            dcMem.SelectObjectAsSource(bmp);

                        int nBmpWidth  = bmp.GetWidth(),
                nBmpHeight = bmp.GetHeight();

            int x = rcImg.left + (imgWidth - nBmpWidth) / 2;
            int y = rcImg.top  + (rcImg.bottom - rcImg.top - nBmpHeight) / 2;
            dc.Blit(x, y, nBmpWidth, nBmpHeight, &dcMem, 0, 0, wxCOPY, true);
        }
    }

    return true;

}

namespace
{

void DrawColorCheckMark(HDC hdc, int x, int y, int cx, int cy, HDC hdcCheckMask, int idxColor)
{
    const COLORREF colBlack = RGB(0, 0, 0);
    const COLORREF colWhite = RGB(255, 255, 255);

    HDCTextColChanger changeTextCol(hdc, colBlack);
    HDCBgColChanger changeBgCol(hdc, colWhite);
    HDCBgModeChanger changeBgMode(hdc, TRANSPARENT);

        MemoryHDC hdcMem(hdc);
    CompatibleBitmap hbmpMem(hdc, cx, cy);
    SelectInHDC selMem(hdcMem, hbmpMem);

    RECT rect = { 0, 0, cx, cy };
    ::FillRect(hdcMem, &rect, ::GetSysColorBrush(idxColor));

    const COLORREF colCheck = ::GetSysColor(idxColor);
    if ( colCheck == colWhite )
    {
        ::BitBlt(hdc, x, y, cx, cy, hdcCheckMask, 0, 0, MERGEPAINT);
        ::BitBlt(hdc, x, y, cx, cy, hdcMem, 0, 0, SRCAND);
    }
    else
    {
        if ( colCheck != colBlack )
        {
            const DWORD ROP_DSna = 0x00220326;              ::BitBlt(hdcMem, 0, 0, cx, cy, hdcCheckMask, 0, 0, ROP_DSna);
        }

        ::BitBlt(hdc, x, y, cx, cy, hdcCheckMask, 0, 0, SRCAND);
        ::BitBlt(hdc, x, y, cx, cy, hdcMem, 0, 0, SRCPAINT);
    }
}

} 
void wxMenuItem::DrawStdCheckMark(WXHDC hdc_, const RECT* rc, wxODStatus stat)
{
    HDC hdc = (HDC)hdc_;

#if wxUSE_UXTHEME
    wxUxThemeEngine* theme = MenuDrawData::GetUxThemeEngine();
    if ( theme )
    {
        wxUxThemeHandle hTheme(GetMenu()->GetWindow(), L"MENU");

        const MenuDrawData* data = MenuDrawData::Get();

                RECT rcBg = *rc;
        data->CheckMargin.UnapplyFrom(rcBg);

        POPUPCHECKBACKGROUNDSTATES stateCheckBg = (stat & wxODDisabled)
                                                    ? MCB_DISABLED
                                                    : MCB_NORMAL;

        theme->DrawThemeBackground(hTheme, hdc, MENU_POPUPCHECKBACKGROUND,
                                   stateCheckBg, &rcBg, NULL);

        POPUPCHECKSTATES stateCheck;
        if ( GetKind() == wxITEM_CHECK )
        {
            stateCheck = (stat & wxODDisabled) ? MC_CHECKMARKDISABLED
                                               : MC_CHECKMARKNORMAL;
        }
        else
        {
            stateCheck = (stat & wxODDisabled) ? MC_BULLETDISABLED
                                               : MC_BULLETNORMAL;
        }

        theme->DrawThemeBackground(hTheme, hdc, MENU_POPUPCHECK,
                                   stateCheck, rc, NULL);
    }
    else
#endif     {
        int cx = rc->right - rc->left;
        int cy = rc->bottom - rc->top;

                MemoryHDC hdcMask(hdc);
        MonoBitmap hbmpMask(cx, cy);
        SelectInHDC selMask(hdcMask,hbmpMask);

                UINT stateCheck = (GetKind() == wxITEM_CHECK) ? DFCS_MENUCHECK
                                                      : DFCS_MENUBULLET;
        RECT rect = { 0, 0, cx, cy };
        ::DrawFrameControl(hdcMask, &rect, DFC_MENU, stateCheck);

                if ( (stat & wxODDisabled) && !(stat & wxODSelected) )
        {
            DrawColorCheckMark(hdc, rc->left + 1, rc->top + 1,
                               cx, cy, hdcMask, COLOR_3DHILIGHT);
        }

                int color = COLOR_MENUTEXT;
        if ( stat & wxODDisabled )
            color = COLOR_BTNSHADOW;
        else if ( stat & wxODSelected )
            color = COLOR_HIGHLIGHTTEXT;

        DrawColorCheckMark(hdc, rc->left, rc->top, cx, cy, hdcMask, color);
    }
}

void wxMenuItem::GetFontToUse(wxFont& font) const
{
    font = GetFont();
    if ( !font.IsOk() )
        font = MenuDrawData::Get()->Font;
}

void wxMenuItem::GetColourToUse(wxODStatus stat, wxColour& colText, wxColour& colBack) const
{
#if wxUSE_UXTHEME
    wxUxThemeEngine* theme = MenuDrawData::GetUxThemeEngine();
    if ( theme )
    {
        wxUxThemeHandle hTheme(GetMenu()->GetWindow(), L"MENU");

        if ( stat & wxODDisabled)
        {
            wxRGBToColour(colText, theme->GetThemeSysColor(hTheme, COLOR_GRAYTEXT));
        }
        else
        {
            colText = GetTextColour();
            if ( !colText.IsOk() )
                wxRGBToColour(colText, theme->GetThemeSysColor(hTheme, COLOR_MENUTEXT));
        }

        if ( stat & wxODSelected )
        {
            wxRGBToColour(colBack, theme->GetThemeSysColor(hTheme, COLOR_HIGHLIGHT));
        }
        else
        {
            colBack = GetBackgroundColour();
            if ( !colBack.IsOk() )
                wxRGBToColour(colBack, theme->GetThemeSysColor(hTheme, COLOR_MENU));
        }
    }
    else
#endif     {
        wxOwnerDrawn::GetColourToUse(stat, colText, colBack);
    }
}

bool wxMenuItem::MSWMustUseOwnerDrawn()
{
                static const wxWinVersion winver = wxGetWinVersion();
    bool mustUseOwnerDrawn = winver < wxWinVersion_98 ||
                                GetTextColour().IsOk() ||
                                GetBackgroundColour().IsOk() ||
                                GetFont().IsOk();

                                if ( !mustUseOwnerDrawn && winver < wxWinVersion_Vista )
    {
        const wxBitmap& bmpUnchecked = GetBitmap(false),
                        bmpChecked   = GetBitmap(true);

        if ( (bmpUnchecked.IsOk() && IsGreaterThanStdSize(bmpUnchecked)) ||
                (bmpChecked.IsOk()   && IsGreaterThanStdSize(bmpChecked)) ||
                (bmpChecked.IsOk() && IsCheckable()) )
        {
            mustUseOwnerDrawn = true;
        }
    }

    return mustUseOwnerDrawn;
}

#endif 
HBITMAP wxMenuItem::GetHBitmapForMenu(BitmapKind kind) const
{
                                                            #if wxUSE_IMAGE
    if ( wxGetWinVersion() >= wxWinVersion_Vista )
    {
        bool checked = (kind != Unchecked);
        wxBitmap bmp = GetBitmap(checked);
        if ( bmp.IsOk() )
        {
            return GetHbitmapOf(bmp);
        }
                return NULL;
    }
#endif 
    return (kind == Normal) ? HBMMENU_CALLBACK : NULL;
}

int wxMenuItem::MSGetMenuItemPos() const
{
    if ( !m_parentMenu )
        return -1;

    const HMENU hMenu = GetHMenuOf(m_parentMenu);
    if ( !hMenu )
        return -1;

    const WXWPARAM id = GetMSWId();
    const int menuItems = ::GetMenuItemCount(hMenu);
    for ( int i = 0; i < menuItems; i++ )
    {
        const UINT state = ::GetMenuState(hMenu, i, MF_BYPOSITION);
        if ( state == (UINT)-1 )
        {
                                    continue;
        }

        if ( state & MF_POPUP )
        {
            if ( ::GetSubMenu(hMenu, i) == (HMENU)id )
                return i;
        }
        else if ( !(state & MF_SEPARATOR) )
        {
            if ( ::GetMenuItemID(hMenu, i) == id )
                return i;
        }
    }

    return -1;
}


wxMenuItem *wxMenuItemBase::New(wxMenu *parentMenu,
                                int id,
                                const wxString& name,
                                const wxString& help,
                                wxItemKind kind,
                                wxMenu *subMenu)
{
    return new wxMenuItem(parentMenu, id, name, help, kind, subMenu);
}

#endif 