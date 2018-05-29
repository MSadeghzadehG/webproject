


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_MENUS

#include "wx/menu.h"

#ifndef WX_PRECOMP
    #include "wx/frame.h"
    #include "wx/utils.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/image.h"
#endif

#if wxUSE_OWNER_DRAWN
    #include "wx/ownerdrw.h"
#endif

#include "wx/scopedarray.h"
#include "wx/vector.h"

#include "wx/msw/private.h"
#include "wx/msw/wrapcctl.h" 
#include <string.h>

#include "wx/dynlib.h"

#ifndef MNS_CHECKORBMP
    #define MNS_CHECKORBMP 0x04000000
#endif
#ifndef MIM_STYLE
    #define MIM_STYLE 0x00000010
#endif



static const int idMenuTitle = wxID_NONE;


class wxMenuRadioItemsData
{
public:
    wxMenuRadioItemsData() { }

    
            bool GetGroupRange(int pos, int *start, int *end) const
    {
                                        for ( Ranges::const_iterator it = m_ranges.begin();
              it != m_ranges.end();
              ++it )
        {
            const Range& r = *it;

            if ( r.start <= pos && pos <= r.end )
            {
                if ( start )
                    *start = r.start;
                if ( end )
                    *end = r.end;

                return true;
            }
        }

        return false;
    }

                        bool UpdateOnInsert(int pos)
    {
        bool inExistingGroup = false;

        for ( Ranges::iterator it = m_ranges.begin();
              it != m_ranges.end();
              ++it )
        {
            Range& r = *it;

            if ( pos < r.start )
            {
                                r.start++;
                r.end++;
            }
            else if ( pos <= r.end + 1 )
            {
                                                                r.end++;

                inExistingGroup = true;
            }
                    }

        if ( inExistingGroup )
            return false;

                Range r;
        r.start = pos;
        r.end = pos;
        m_ranges.push_back(r);

        return true;
    }

                                bool UpdateOnRemoveItem(int pos)
    {
        bool inExistingGroup = false;

                        Ranges::iterator itEmptyGroup = m_ranges.end();

        for ( Ranges::iterator it = m_ranges.begin();
              it != m_ranges.end();
              ++it )
        {
            Range& r = *it;

            if ( pos < r.start )
            {
                                                r.start--;
                r.end--;
            }
            else if ( pos <= r.end )
            {
                                                r.end--;

                                                if ( r.end < r.start )
                    itEmptyGroup = it;

                inExistingGroup = true;
            }
                    }

                if ( itEmptyGroup != m_ranges.end() )
            m_ranges.erase(itEmptyGroup);

        return inExistingGroup;
    }

private:
        struct Range
    {
        int start;
        int end;
    };

    typedef wxVector<Range> Ranges;
    Ranges m_ranges;
};

namespace
{

void SetDefaultMenuItem(HMENU hmenu, UINT id)
{
    WinStruct<MENUITEMINFO> mii;
    mii.fMask = MIIM_STATE;
    mii.fState = MFS_DEFAULT;

    if ( !::SetMenuItemInfo(hmenu, id, FALSE, &mii) )
    {
        wxLogLastError(wxT("SetMenuItemInfo"));
    }
}

void SetOwnerDrawnMenuItem(HMENU hmenu,
                           UINT id,
                           ULONG_PTR data,
                           BOOL byPositon = FALSE)
{
    WinStruct<MENUITEMINFO> mii;
    mii.fMask = MIIM_FTYPE | MIIM_DATA;
    mii.fType = MFT_OWNERDRAW;
    mii.dwItemData = data;

    if ( reinterpret_cast<wxMenuItem*>(data)->IsSeparator() )
        mii.fType |= MFT_SEPARATOR;

    if ( !::SetMenuItemInfo(hmenu, id, byPositon, &mii) )
    {
        wxLogLastError(wxT("SetMenuItemInfo"));
    }
}

} 


void wxMenu::InitNoCreate()
{
    m_radioData = NULL;
    m_doBreak = false;

#if wxUSE_OWNER_DRAWN
    m_ownerDrawn = false;
    m_maxBitmapWidth = 0;
    m_maxAccelWidth = -1;
#endif }

void wxMenu::Init()
{
    InitNoCreate();

        m_hMenu = (WXHMENU)CreatePopupMenu();
    if ( !m_hMenu )
    {
        wxLogLastError(wxT("CreatePopupMenu"));
    }

        if ( !m_title.empty() )
    {
        const wxString title = m_title;
        m_title.clear();         SetTitle(title);
    }
}

wxMenu::wxMenu(WXHMENU hMenu)
{
    InitNoCreate();

    m_hMenu = hMenu;

                        const int numExistingItems = ::GetMenuItemCount(m_hMenu);
    for ( int n = 0; n < numExistingItems; n++ )
    {
        wxMenuBase::DoAppend(wxMenuItem::New(this, wxID_SEPARATOR));
    }
}

wxMenu::~wxMenu()
{
                if ( m_hMenu && !IsAttached() && !GetParent() )
    {
        if ( !::DestroyMenu(GetHmenu()) )
        {
            wxLogLastError(wxT("DestroyMenu"));
        }
    }

#if wxUSE_ACCEL
        WX_CLEAR_ARRAY(m_accels);
#endif 
    delete m_radioData;
}

void wxMenu::Break()
{
        m_doBreak = true;
}

#if wxUSE_ACCEL

int wxMenu::FindAccel(int id) const
{
    size_t n, count = m_accels.GetCount();
    for ( n = 0; n < count; n++ )
    {
        if ( m_accels[n]->m_command == id )
            return n;
    }

    return wxNOT_FOUND;
}

void wxMenu::UpdateAccel(wxMenuItem *item)
{
    if ( item->IsSubMenu() )
    {
        wxMenu *submenu = item->GetSubMenu();
        wxMenuItemList::compatibility_iterator node = submenu->GetMenuItems().GetFirst();
        while ( node )
        {
            UpdateAccel(node->GetData());

            node = node->GetNext();
        }
    }
    else if ( !item->IsSeparator() )
    {
                                        if ( GetParent() )
        {
            GetParent()->UpdateAccel(item);
            return;
        }

                wxAcceleratorEntry *accel = wxAcceleratorEntry::Create(item->GetItemLabel());
        if ( accel )
            accel->m_command = item->GetId();

                int n = FindAccel(item->GetId());
        if ( n == wxNOT_FOUND )
        {
                        if ( accel )
                m_accels.Add(accel);
            else
                return;             }
        else
        {
                        delete m_accels[n];
            if ( accel )
                m_accels[n] = accel;
            else
                m_accels.RemoveAt(n);
        }

        if ( IsAttached() )
        {
            GetMenuBar()->RebuildAccelTable();
        }

#if wxUSE_OWNER_DRAWN
        ResetMaxAccelWidth();
#endif
    }
    }

#endif 
namespace
{

} 
bool wxMenu::MSWGetRadioGroupRange(int pos, int *start, int *end) const
{
    return m_radioData && m_radioData->GetGroupRange(pos, start, end);
}

bool wxMenu::DoInsertOrAppend(wxMenuItem *pItem, size_t pos)
{
#if wxUSE_ACCEL
    UpdateAccel(pItem);
#endif 
        UINT flags = pItem->IsEnabled() ? MF_ENABLED : MF_GRAYED;

            if ( m_doBreak ) {
        flags |= MF_MENUBREAK;
        m_doBreak = false;
    }

    if ( pItem->IsSeparator() ) {
        flags |= MF_SEPARATOR;
    }

            UINT_PTR id;
    wxMenu *submenu = pItem->GetSubMenu();
    if ( submenu != NULL ) {
        wxASSERT_MSG( submenu->GetHMenu(), wxT("invalid submenu") );

        submenu->SetParent(this);

        id = (UINT_PTR)submenu->GetHMenu();

        flags |= MF_POPUP;
    }
    else {
        id = pItem->GetMSWId();
    }


        wxString itemText = pItem->GetItemLabel();
    LPCTSTR pData = NULL;
    if ( pos == (size_t)-1 )
    {
                        pos = GetMenuItemCount() - 1;
    }

                        bool checkInitially = false;
    if ( pItem->GetKind() == wxITEM_RADIO )
    {
        if ( !m_radioData )
            m_radioData = new wxMenuRadioItemsData;

        if ( m_radioData->UpdateOnInsert(pos) )
            checkInitially = true;
    }

                            if ( pItem->IsCheck() && pItem->wxMenuItemBase::IsChecked() )
        flags |= MF_CHECKED;

        if ( !GetMenuBar() && !m_title.empty() )
        pos += 2; 
    BOOL ok = false;

#if wxUSE_OWNER_DRAWN
                if ( m_ownerDrawn )
        pItem->SetOwnerDrawn(true);

        bool makeItemOwnerDrawn = false;
#endif 
    if (
#if wxUSE_OWNER_DRAWN
            !pItem->IsOwnerDrawn() &&
#endif
        !pItem->IsSeparator() )
            {
                WinStruct<MENUITEMINFO> mii;
                mii.fMask = MIIM_STRING | MIIM_DATA;

                                                if ( pItem->IsCheckable() )
                {
                    mii.fMask |= MIIM_CHECKMARKS;
                    mii.hbmpChecked = pItem->GetHBitmapForMenu(wxMenuItem::Checked);
                    mii.hbmpUnchecked = pItem->GetHBitmapForMenu(wxMenuItem::Unchecked);
                }
                else if ( pItem->GetBitmap().IsOk() )
                {
                    mii.fMask |= MIIM_BITMAP;
                    mii.hbmpItem = pItem->GetHBitmapForMenu(wxMenuItem::Normal);
                }

                mii.cch = itemText.length();
                mii.dwTypeData = wxMSW_CONV_LPTSTR(itemText);

                if ( flags & MF_POPUP )
                {
                    mii.fMask |= MIIM_SUBMENU;
                    mii.hSubMenu = GetHmenuOf(pItem->GetSubMenu());
                }
                else
                {
                    mii.fMask |= MIIM_ID;
                    mii.wID = id;
                }

                if ( flags & MF_GRAYED )
                {
                    mii.fMask |= MIIM_STATE;
                    mii.fState = MFS_GRAYED;
                }

                if ( flags & MF_CHECKED )
                {
                    mii.fMask |= MIIM_STATE;
                    mii.fState = MFS_CHECKED;
                }

                mii.dwItemData = reinterpret_cast<ULONG_PTR>(pItem);

                ok = ::InsertMenuItem(GetHmenu(), pos, TRUE , &mii);
                if ( !ok )
                {
                    wxLogLastError(wxT("InsertMenuItem()"));
#if wxUSE_OWNER_DRAWN
                        makeItemOwnerDrawn = true;
#endif
                }
                else                 {
                                                                                                    WinStruct<MENUINFO> mi;
                    mi.fMask = MIM_STYLE;
                    mi.dwStyle = MNS_CHECKORBMP;
                    if ( !::SetMenuInfo(GetHmenu(), &mi) )
                    {
                        wxLogLastError(wxT("SetMenuInfo(MNS_NOCHECK)"));
                    }
                }
        }

#if wxUSE_OWNER_DRAWN
    if ( pItem->IsOwnerDrawn() || makeItemOwnerDrawn )
        {
                        flags |= MF_OWNERDRAW;
            pData = (LPCTSTR)pItem;

            bool updateAllMargins = false;

                                    int uncheckedW = pItem->GetBitmap(false).GetWidth();
            int checkedW   = pItem->GetBitmap(true).GetWidth();

            if ( m_maxBitmapWidth < uncheckedW )
            {
                m_maxBitmapWidth = uncheckedW;
                updateAllMargins = true;
            }

            if ( m_maxBitmapWidth < checkedW )
            {
                m_maxBitmapWidth = checkedW;
                updateAllMargins = true;
            }

                        if ( !m_ownerDrawn || updateAllMargins )
            {
                                                int position = 0;
                wxMenuItemList::compatibility_iterator node = GetMenuItems().GetFirst();
                while (node)
                {
                    wxMenuItem* item = node->GetData();

                                                                                                    if ( !item->IsOwnerDrawn() && item != pItem )
                    {
                        item->SetOwnerDrawn(true);
                        SetOwnerDrawnMenuItem(GetHmenu(), position,
                                              reinterpret_cast<ULONG_PTR>(item), TRUE);
                    }

                    item->SetMarginWidth(m_maxBitmapWidth);

                    node = node->GetNext();
                                                                                                    if ( item != pItem )
                        position++;
                }

                                m_ownerDrawn = true;

                ResetMaxAccelWidth();
            }
                        else if ( !updateAllMargins )
            {
                pItem->SetMarginWidth(m_maxBitmapWidth);
            }
        }
#endif 
        if ( !ok )
    {
        if ( !::InsertMenu(GetHmenu(), pos, flags | MF_BYPOSITION, id, pData) )
        {
            wxLogLastError(wxT("InsertMenu[Item]()"));

            return false;
        }

#if wxUSE_OWNER_DRAWN
        if ( makeItemOwnerDrawn )
        {
            pItem->SetOwnerDrawn(true);
            SetOwnerDrawnMenuItem(GetHmenu(), pos,
                                  reinterpret_cast<ULONG_PTR>(pItem), TRUE);
        }
#endif
    }


        if ( checkInitially )
        pItem->Check(true);

        if ( id == (UINT_PTR)idMenuTitle )
    {
                SetDefaultMenuItem(GetHmenu(), id);
    }

        if ( IsAttached() && GetMenuBar()->IsAttached() )
    {
        GetMenuBar()->Refresh();
    }

    return true;
}

wxMenuItem* wxMenu::DoAppend(wxMenuItem *item)
{
    return wxMenuBase::DoAppend(item) && DoInsertOrAppend(item) ? item : NULL;
}

wxMenuItem* wxMenu::DoInsert(size_t pos, wxMenuItem *item)
{
    if (wxMenuBase::DoInsert(pos, item) && DoInsertOrAppend(item, pos))
        return item;
    else
        return NULL;
}

wxMenuItem *wxMenu::DoRemove(wxMenuItem *item)
{
        size_t pos;
    wxMenuItemList::compatibility_iterator node = GetMenuItems().GetFirst();
    for ( pos = 0; node; pos++ )
    {
        if ( node->GetData() == item )
            break;

        node = node->GetNext();
    }

#if wxUSE_ACCEL
        int n = FindAccel(item->GetId());
    if ( n != wxNOT_FOUND )
    {
        delete m_accels[n];

        m_accels.RemoveAt(n);

#if wxUSE_OWNER_DRAWN
        ResetMaxAccelWidth();
#endif
    }
    #endif 
        if ( m_radioData )
    {
        if ( m_radioData->UpdateOnRemoveItem(pos) )
        {
            wxASSERT_MSG( item->GetKind() == wxITEM_RADIO,
                          wxT("Removing non radio button from radio group?") );
        }
            }

        if ( !::RemoveMenu(GetHmenu(), (UINT)pos, MF_BYPOSITION) )
    {
        wxLogLastError(wxT("RemoveMenu"));
    }

    if ( IsAttached() && GetMenuBar()->IsAttached() )
    {
                GetMenuBar()->Refresh();
    }

        return wxMenuBase::DoRemove(item);
}


#if wxUSE_ACCEL

size_t wxMenu::CopyAccels(wxAcceleratorEntry *accels) const
{
    size_t count = GetAccelCount();
    for ( size_t n = 0; n < count; n++ )
    {
        *accels++ = *m_accels[n];
    }

    return count;
}

wxAcceleratorTable *wxMenu::CreateAccelTable() const
{
    const size_t count = m_accels.size();
    wxScopedArray<wxAcceleratorEntry> accels(count);
    CopyAccels(accels.get());

    return new wxAcceleratorTable(count, accels.get());
}

#endif 

#if wxUSE_OWNER_DRAWN

void wxMenu::CalculateMaxAccelWidth()
{
    wxASSERT_MSG( m_maxAccelWidth == -1, wxT("it's really needed?") );

    wxMenuItemList::compatibility_iterator node = GetMenuItems().GetFirst();
    while (node)
    {
        wxMenuItem* item = node->GetData();

        if ( item->IsOwnerDrawn() )
        {
            int width = item->MeasureAccelWidth();
            if (width > m_maxAccelWidth )
                m_maxAccelWidth = width;
        }

        node = node->GetNext();
    }
}

#endif 

void wxMenu::SetTitle(const wxString& label)
{
    bool hasNoTitle = m_title.empty();
    m_title = label;

    HMENU hMenu = GetHmenu();

    if ( hasNoTitle )
    {
        if ( !label.empty() )
        {
            if ( !::InsertMenu(hMenu, 0u, MF_BYPOSITION | MF_STRING,
                               (UINT_PTR)idMenuTitle, m_title.t_str()) ||
                 !::InsertMenu(hMenu, 1u, MF_BYPOSITION, (unsigned)-1, NULL) )
            {
                wxLogLastError(wxT("InsertMenu"));
            }
        }
    }
    else
    {
        if ( label.empty() )
        {
                        if ( !RemoveMenu(hMenu, 0, MF_BYPOSITION) ||
                 !RemoveMenu(hMenu, 0, MF_BYPOSITION) )
            {
                wxLogLastError(wxT("RemoveMenu"));
            }
        }
        else
        {
                        if ( !ModifyMenu(hMenu, 0u,
                             MF_BYPOSITION | MF_STRING,
                             (UINT_PTR)idMenuTitle, m_title.t_str()) )
            {
                wxLogLastError(wxT("ModifyMenu"));
            }
        }
    }

        if ( !m_title.empty() )
    {
        SetDefaultMenuItem(GetHmenu(), (UINT)idMenuTitle);
    }
}


bool wxMenu::MSWCommand(WXUINT WXUNUSED(param), WXWORD id_)
{
    const int id = (signed short)id_;

        if ( id != idMenuTitle )
    {
                int checked = -1;

                wxMenuItem * const item = FindItem(id);
        if ( item )
        {
            if ( (item->GetKind() == wxITEM_RADIO) && item->IsChecked() )
                return true;

            if ( item->IsCheckable() )
            {
                item->Toggle();

                                                                                                UINT menuState = ::GetMenuState(GetHmenu(), id_, MF_BYCOMMAND);
                checked = (menuState & MF_CHECKED) != 0;
            }
        }

        SendEvent(id, checked);
    }

    return true;
}

wxMenu* wxMenu::MSWGetMenu(WXHMENU hMenu)
{
        if ( GetHMenu() == hMenu )
        return this;

        for ( size_t n = 0 ; n < GetMenuItemCount(); ++n )
    {
        wxMenuItem* item = FindItemByPosition(n);
        wxMenu* submenu = item->GetSubMenu();
        if ( submenu )
        {
            submenu = submenu->MSWGetMenu(hMenu);
            if (submenu)
                return submenu;
        }
    }

        return NULL;
}


void wxMenuBar::Init()
{
    m_eventHandler = this;
    m_hMenu = 0;
}

wxMenuBar::wxMenuBar()
{
    Init();
}

wxMenuBar::wxMenuBar( long WXUNUSED(style) )
{
    Init();
}

wxMenuBar::wxMenuBar(size_t count, wxMenu *menus[], const wxString titles[], long WXUNUSED(style))
{
    Init();

    for ( size_t i = 0; i < count; i++ )
    {
                                menus[i]->wxMenuBase::SetTitle(titles[i]);
        m_menus.Append(menus[i]);

        menus[i]->Attach(this);
    }
}

wxMenuBar::~wxMenuBar()
{
            if (m_hMenu && !IsAttached())
    {
        ::DestroyMenu((HMENU)m_hMenu);
        m_hMenu = (WXHMENU)NULL;
    }
}


void wxMenuBar::Refresh()
{
    if ( IsFrozen() )
        return;

    wxCHECK_RET( IsAttached(), wxT("can't refresh unattached menubar") );

    DrawMenuBar(GetHwndOf(GetFrame()));
}

WXHMENU wxMenuBar::Create()
{
    if ( m_hMenu != 0 )
        return m_hMenu;

    m_hMenu = (WXHMENU)::CreateMenu();

    if ( !m_hMenu )
    {
        wxLogLastError(wxT("CreateMenu"));
    }
    else
    {
        for ( wxMenuList::iterator it = m_menus.begin();
              it != m_menus.end();
              ++it )
        {
            if ( !::AppendMenu((HMENU)m_hMenu, MF_POPUP | MF_STRING,
                               (UINT_PTR)(*it)->GetHMenu(),
                               (*it)->GetTitle().t_str()) )
            {
                wxLogLastError(wxT("AppendMenu"));
            }
        }
    }

    return m_hMenu;
}

int wxMenuBar::MSWPositionForWxMenu(wxMenu *menu, int wxpos)
{
    wxASSERT(menu);
    wxASSERT(menu->GetHMenu());
    wxASSERT(m_hMenu);

    int totalMSWItems = GetMenuItemCount((HMENU)m_hMenu);

    int i;     for(i=wxpos; i<totalMSWItems; i++)
    {
        if(GetSubMenu((HMENU)m_hMenu,i)==(HMENU)menu->GetHMenu())
            return i;
    }
    for(i=0; i<wxpos; i++)
    {
        if(GetSubMenu((HMENU)m_hMenu,i)==(HMENU)menu->GetHMenu())
            return i;
    }
    wxFAIL;
    return -1;
}



void wxMenuBar::EnableTop(size_t pos, bool enable)
{
    wxCHECK_RET( IsAttached(), wxT("doesn't work with unattached menubars") );
    wxCHECK_RET( pos < GetMenuCount(), wxT("invalid menu index") );

    int flag = enable ? MF_ENABLED : MF_GRAYED;

    EnableMenuItem((HMENU)m_hMenu, MSWPositionForWxMenu(GetMenu(pos),pos), MF_BYPOSITION | flag);

    Refresh();
}

bool wxMenuBar::IsEnabledTop(size_t pos) const
{
    wxCHECK_MSG( pos < GetMenuCount(), false, wxS("invalid menu index") );
    WinStruct<MENUITEMINFO> mii;
    mii.fMask = MIIM_STATE;
    if ( !::GetMenuItemInfo(GetHmenu(), pos, TRUE, &mii) )
    {
        wxLogLastError(wxS("GetMenuItemInfo(menubar)"));
    }

    return !(mii.fState & MFS_GRAYED);
}

void wxMenuBar::SetMenuLabel(size_t pos, const wxString& label)
{
    wxCHECK_RET( pos < GetMenuCount(), wxT("invalid menu index") );

    m_menus[pos]->wxMenuBase::SetTitle(label);

    if ( !IsAttached() )
    {
        return;
    }
    
    int mswpos = MSWPositionForWxMenu(GetMenu(pos),pos);

    UINT_PTR id;
    UINT flagsOld = ::GetMenuState((HMENU)m_hMenu, mswpos, MF_BYPOSITION);
    if ( flagsOld == 0xFFFFFFFF )
    {
        wxLogLastError(wxT("GetMenuState"));

        return;
    }

    if ( flagsOld & MF_POPUP )
    {
                flagsOld &= 0xff;
        id = (UINT_PTR)::GetSubMenu((HMENU)m_hMenu, mswpos);
    }
    else
    {
        id = pos;
    }

    if ( ::ModifyMenu(GetHmenu(), mswpos, MF_BYPOSITION | MF_STRING | flagsOld,
                      id, label.t_str()) == (int)0xFFFFFFFF )
    {
        wxLogLastError(wxT("ModifyMenu"));
    }

    Refresh();
}

wxString wxMenuBar::GetMenuLabel(size_t pos) const
{
    wxCHECK_MSG( pos < GetMenuCount(), wxEmptyString,
                 wxT("invalid menu index in wxMenuBar::GetMenuLabel") );

    return m_menus[pos]->GetTitle();
}


wxMenu *wxMenuBar::Replace(size_t pos, wxMenu *menu, const wxString& title)
{
    wxMenu *menuOld = wxMenuBarBase::Replace(pos, menu, title);
    if ( !menuOld )
        return NULL;

    menu->wxMenuBase::SetTitle(title);

    if (GetHmenu())
    {
        int mswpos = MSWPositionForWxMenu(menuOld,pos);

                if ( !::RemoveMenu(GetHmenu(), (UINT)mswpos, MF_BYPOSITION) )
        {
            wxLogLastError(wxT("RemoveMenu"));
        }

        if ( !::InsertMenu(GetHmenu(), (UINT)mswpos,
                           MF_BYPOSITION | MF_POPUP | MF_STRING,
                           (UINT_PTR)GetHmenuOf(menu), title.t_str()) )
        {
            wxLogLastError(wxT("InsertMenu"));
        }

#if wxUSE_ACCEL
        if ( menuOld->HasAccels() || menu->HasAccels() )
        {
                        RebuildAccelTable();
        }
#endif 
        if (IsAttached())
            Refresh();
    }

    return menuOld;
}

bool wxMenuBar::Insert(size_t pos, wxMenu *menu, const wxString& title)
{
                bool isAttached =
        (GetHmenu() != 0);

    if ( !wxMenuBarBase::Insert(pos, menu, title) )
        return false;

    menu->wxMenuBase::SetTitle(title);

    if ( isAttached )
    {
                                                        int mswExtra = 0;

                                int mswCount = ::GetMenuItemCount(GetHmenu());
        if ( mswCount != -1 &&
                static_cast<unsigned>(mswCount) != GetMenuCount() - 1 )
        {
            wxMenuList::compatibility_iterator node = m_menus.GetFirst();
            for ( size_t n = 0; n < pos; n++ )
            {
                if ( ::GetSubMenu(GetHmenu(), n) != GetHmenuOf(node->GetData()) )
                    mswExtra++;
                else
                    node = node->GetNext();
            }
        }

        if ( !::InsertMenu(GetHmenu(), pos + mswExtra,
                           MF_BYPOSITION | MF_POPUP | MF_STRING,
                           (UINT_PTR)GetHmenuOf(menu), title.t_str()) )
        {
            wxLogLastError(wxT("InsertMenu"));
        }
#if wxUSE_ACCEL
        if ( menu->HasAccels() )
        {
                        RebuildAccelTable();
        }
#endif 
        if (IsAttached())
            Refresh();
    }

    return true;
}

bool wxMenuBar::Append(wxMenu *menu, const wxString& title)
{
    WXHMENU submenu = menu ? menu->GetHMenu() : 0;
    wxCHECK_MSG( submenu, false, wxT("can't append invalid menu to menubar") );

    if ( !wxMenuBarBase::Append(menu, title) )
        return false;

    menu->wxMenuBase::SetTitle(title);

    if (GetHmenu())
    {
        if ( !::AppendMenu(GetHmenu(), MF_POPUP | MF_STRING,
                           (UINT_PTR)submenu, title.t_str()) )
        {
            wxLogLastError(wxT("AppendMenu"));
        }

#if wxUSE_ACCEL
        if ( menu->HasAccels() )
        {
                        RebuildAccelTable();
        }
#endif 
        if (IsAttached())
            Refresh();
    }

    return true;
}

wxMenu *wxMenuBar::Remove(size_t pos)
{
    wxMenu *menu = wxMenuBarBase::Remove(pos);
    if ( !menu )
        return NULL;

    if (GetHmenu())
    {
        if ( !::RemoveMenu(GetHmenu(), (UINT)MSWPositionForWxMenu(menu,pos), MF_BYPOSITION) )
        {
            wxLogLastError(wxT("RemoveMenu"));
        }

#if wxUSE_ACCEL
        if ( menu->HasAccels() )
        {
                        RebuildAccelTable();
        }
#endif 
        if (IsAttached())
            Refresh();
    }

    return menu;
}

#if wxUSE_ACCEL

void wxMenuBar::RebuildAccelTable()
{
        size_t nAccelCount = 0;
    size_t i, count = GetMenuCount();
    wxMenuList::iterator it;
    for ( i = 0, it = m_menus.begin(); i < count; i++, it++ )
    {
        nAccelCount += (*it)->GetAccelCount();
    }

    if ( nAccelCount )
    {
        wxAcceleratorEntry *accelEntries = new wxAcceleratorEntry[nAccelCount];

        nAccelCount = 0;
        for ( i = 0, it = m_menus.begin(); i < count; i++, it++ )
        {
            nAccelCount += (*it)->CopyAccels(&accelEntries[nAccelCount]);
        }

        SetAcceleratorTable(wxAcceleratorTable(nAccelCount, accelEntries));

        delete [] accelEntries;
    }
    else     {
        SetAcceleratorTable(wxAcceleratorTable());
    }
}

#endif 
void wxMenuBar::Attach(wxFrame *frame)
{
    wxMenuBarBase::Attach(frame);

#if wxUSE_ACCEL
    RebuildAccelTable();
#endif }

void wxMenuBar::Detach()
{
    wxMenuBarBase::Detach();
}

int wxMenuBar::MSWGetTopMenuPos(WXHMENU hMenu) const
{
    for ( size_t n = 0 ; n < GetMenuCount(); ++n )
    {
        wxMenu* menu = GetMenu(n)->MSWGetMenu(hMenu);
        if ( menu )
            return n;
    }

    return wxNOT_FOUND;
}

wxMenu* wxMenuBar::MSWGetMenu(WXHMENU hMenu) const
{
            if ( hMenu == GetHMenu() )
        return NULL;

        for ( size_t n = 0 ; n < GetMenuCount(); ++n )
    {
        wxMenu* menu = GetMenu(n)->MSWGetMenu(hMenu);
        if ( menu )
            return menu;
    }

        return NULL;
}

#endif 