


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_TREECTRL

#include "wx/treectrl.h"

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcctl.h"     #include "wx/msw/missing.h"
    #include "wx/dynarray.h"
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/settings.h"
#endif

#include "wx/msw/private.h"

#include "wx/imaglist.h"
#include "wx/msw/dragimag.h"
#include "wx/msw/uxtheme.h"


#define HITEM(item)     ((HTREEITEM)(((item).m_pItem)))


#ifndef TVN_ITEMCHANGINGA

#define TVN_ITEMCHANGINGA (TVN_FIRST-16)
#define TVN_ITEMCHANGINGW (TVN_FIRST-17)

typedef struct tagNMTVITEMCHANGE
{
    NMHDR hdr;
    UINT uChanged;
    HTREEITEM hItem;
    UINT uStateNew;
    UINT uStateOld;
    LPARAM lParam;
} NMTVITEMCHANGE;

#endif




class TreeItemUnlocker
{
public:
        TreeItemUnlocker(HTREEITEM item)
    {
        m_oldUnlockedItem = ms_unlockedItem;
        ms_unlockedItem = item;
    }

        TreeItemUnlocker()
    {
        m_oldUnlockedItem = ms_unlockedItem;
        ms_unlockedItem = (HTREEITEM)-1;
    }

        ~TreeItemUnlocker() { ms_unlockedItem = m_oldUnlockedItem; }


        static bool IsLocked(HTREEITEM item)
        { return ms_unlockedItem != (HTREEITEM)-1 && item != ms_unlockedItem; }

private:
    static HTREEITEM ms_unlockedItem;
    HTREEITEM m_oldUnlockedItem;

    wxDECLARE_NO_COPY_CLASS(TreeItemUnlocker);
};

HTREEITEM TreeItemUnlocker::ms_unlockedItem = NULL;

class TempSetter
{
public:
    TempSetter(bool& var) : m_var(var)
    {
        wxASSERT_MSG( !m_var, "variable shouldn't be already set" );
        m_var = true;
    }

    ~TempSetter()
    {
        m_var = false;
    }

private:
    bool& m_var;

    wxDECLARE_NO_COPY_CLASS(TempSetter);
};


namespace
{

union TVGetItemRectParam
{
    RECT rect;
    HTREEITEM hItem;
};

inline bool
wxTreeView_GetItemRect(HWND hwnd,
                       HTREEITEM hItem,
                       TVGetItemRectParam& param,
                       BOOL fItemRect)
{
    param.hItem = hItem;
    return ::SendMessage(hwnd, TVM_GETITEMRECT, fItemRect,
                        (LPARAM)&param) == TRUE;
}

} 
static bool IsItemSelected(HWND hwndTV, HTREEITEM hItem)
{
    TV_ITEM tvi;
    tvi.mask = TVIF_STATE | TVIF_HANDLE;
    tvi.stateMask = TVIS_SELECTED;
    tvi.hItem = hItem;

    TreeItemUnlocker unlocker(hItem);

    if ( !TreeView_GetItem(hwndTV, &tvi) )
    {
        wxLogLastError(wxT("TreeView_GetItem"));
    }

    return (tvi.state & TVIS_SELECTED) != 0;
}

static bool SelectItem(HWND hwndTV, HTREEITEM hItem, bool select = true)
{
    TV_ITEM tvi;
    tvi.mask = TVIF_STATE | TVIF_HANDLE;
    tvi.stateMask = TVIS_SELECTED;
    tvi.state = select ? TVIS_SELECTED : 0;
    tvi.hItem = hItem;

    TreeItemUnlocker unlocker(hItem);

    if ( TreeView_SetItem(hwndTV, &tvi) == -1 )
    {
        wxLogLastError(wxT("TreeView_SetItem"));
        return false;
    }

    return true;
}

static inline void UnselectItem(HWND hwndTV, HTREEITEM htItem)
{
    SelectItem(hwndTV, htItem, false);
}

static inline void ToggleItemSelection(HWND hwndTV, HTREEITEM htItem)
{
    SelectItem(hwndTV, htItem, !IsItemSelected(hwndTV, htItem));
}


enum
{
    SR_SIMULATE = 1,            SR_UNSELECT_OTHERS = 2  };

static bool SelectRange(HWND hwndTV,
                        HTREEITEM htFirst,
                        HTREEITEM htLast,
                        int flags)
{
        bool changed = false;
    bool cont = true;
    HTREEITEM htItem = (HTREEITEM)TreeView_GetRoot(hwndTV);

    while ( htItem && cont )
    {
        if ( (htItem == htFirst) || (htItem == htLast) )
        {
            if ( !IsItemSelected(hwndTV, htItem) )
            {
                if ( !(flags & SR_SIMULATE) )
                {
                    SelectItem(hwndTV, htItem);
                }

                changed = true;
            }

            cont = false;
        }
        else         {
            if ( flags & SR_UNSELECT_OTHERS )
            {
                if ( IsItemSelected(hwndTV, htItem) )
                {
                    if ( !(flags & SR_SIMULATE) )
                        UnselectItem(hwndTV, htItem);

                    changed = true;
                }
            }
        }

        htItem = (HTREEITEM)TreeView_GetNextVisible(hwndTV, htItem);
    }

        cont = htFirst != htLast;
    while ( htItem && cont )
    {
        if ( !IsItemSelected(hwndTV, htItem) )
        {
            if ( !(flags & SR_SIMULATE) )
            {
                SelectItem(hwndTV, htItem);
            }

            changed = true;
        }

        cont = (htItem != htFirst) && (htItem != htLast);

        htItem = (HTREEITEM)TreeView_GetNextVisible(hwndTV, htItem);
    }

        if ( flags & SR_UNSELECT_OTHERS )
    {
        while ( htItem )
        {
            if ( IsItemSelected(hwndTV, htItem) )
            {
                if ( !(flags & SR_SIMULATE) )
                {
                    UnselectItem(hwndTV, htItem);
                }

                changed = true;
            }

            htItem = (HTREEITEM)TreeView_GetNextVisible(hwndTV, htItem);
        }
    }

            if ( !(flags & SR_SIMULATE) )
    {
        UpdateWindow(hwndTV);
    }

    return changed;
}

static bool SetFocus(HWND hwndTV, HTREEITEM htItem)
{
        HTREEITEM htFocus = (HTREEITEM)TreeView_GetSelection(hwndTV);

    if ( htItem == htFocus )
        return false;

    if ( htItem )
    {
                bool wasSelected = IsItemSelected(hwndTV, htItem);

        if ( htFocus && IsItemSelected(hwndTV, htFocus) )
        {
                                                (void)TreeView_SelectItem(hwndTV, 0);
            SelectItem(hwndTV, htFocus);
        }

        (void)TreeView_SelectItem(hwndTV, htItem);

        if ( !wasSelected )
        {
                                    UnselectItem(hwndTV, htItem);
        }
            }
    else     {
        bool wasFocusSelected = IsItemSelected(hwndTV, htFocus);

                (void)TreeView_SelectItem(hwndTV, 0);

        if ( wasFocusSelected )
        {
                        SelectItem(hwndTV, htFocus);
        }
    }

    return true;
}


#ifdef __VISUALC__
#pragma warning( disable : 4097 ) #endif

struct wxTreeViewItem : public TV_ITEM
{
    wxTreeViewItem(const wxTreeItemId& item,                       UINT mask_,                                     UINT stateMask_ = 0)             {
        wxZeroMemory(*this);

                mask = mask_ | TVIF_HANDLE;
        stateMask = stateMask_;
        hItem = HITEM(item);
    }
};


class wxTreeItemParam
{
public:
    wxTreeItemParam()
    {
        m_data = NULL;

        for ( size_t n = 0; n < WXSIZEOF(m_images); n++ )
        {
            m_images[n] = -1;
        }
    }

        virtual ~wxTreeItemParam() { delete m_data; }

                wxTreeItemData *GetData() const { return m_data; }
            void SetData(wxTreeItemData *data) { m_data = data; }

            bool HasImage(wxTreeItemIcon which) const { return m_images[which] != -1; }
                    int GetImage(wxTreeItemIcon which) const
    {
        int image = m_images[which];
        if ( image == -1 )
        {
            switch ( which )
            {
                case wxTreeItemIcon_SelectedExpanded:
                                                            image = m_images[wxTreeItemIcon_Expanded];
                    if ( image == -1 )
                        image = m_images[wxTreeItemIcon_Selected];
                    if ( image != -1 )
                        break;
                    
                case wxTreeItemIcon_Selected:
                case wxTreeItemIcon_Expanded:
                    image = m_images[wxTreeItemIcon_Normal];
                    break;

                case wxTreeItemIcon_Normal:
                                        break;

                default:
                    wxFAIL_MSG( wxT("unsupported wxTreeItemIcon value") );
            }
        }

        return image;
    }
            void SetImage(int image, wxTreeItemIcon which) { m_images[which] = image; }

            const wxTreeItemId& GetItem() const { return m_item; }
            void SetItem(const wxTreeItemId& item) { m_item = item; }

protected:
        int m_images[wxTreeItemIcon_Max];

        wxTreeItemId m_item;

        wxTreeItemData *m_data;

    wxDECLARE_NO_COPY_CLASS(wxTreeItemParam);
};

class wxVirtualNode : public wxTreeViewItem
{
public:
    wxVirtualNode(wxTreeItemParam *param)
        : wxTreeViewItem(TVI_ROOT, 0)
    {
        m_param = param;
    }

    ~wxVirtualNode()
    {
        delete m_param;
    }

    wxTreeItemParam *GetParam() const { return m_param; }
    void SetParam(wxTreeItemParam *param) { delete m_param; m_param = param; }

private:
    wxTreeItemParam *m_param;

    wxDECLARE_NO_COPY_CLASS(wxVirtualNode);
};

#ifdef __VISUALC__
#pragma warning( default : 4097 )
#endif

#define GET_VIRTUAL_ROOT() ((wxVirtualNode *)m_pVirtualRoot)

#define IS_VIRTUAL_ROOT(item) (HITEM(item) == TVI_ROOT)

class wxTreeTraversal
{
public:
    wxTreeTraversal(const wxTreeCtrl *tree)
    {
        m_tree = tree;
    }

                virtual ~wxTreeTraversal() { }

                bool DoTraverse(const wxTreeItemId& root, bool recursively = true);

            virtual bool OnVisit(const wxTreeItemId& item) = 0;

protected:
    const wxTreeCtrl *GetTree() const { return m_tree; }

private:
    bool Traverse(const wxTreeItemId& root, bool recursively);

    const wxTreeCtrl *m_tree;

    wxDECLARE_NO_COPY_CLASS(wxTreeTraversal);
};

class TraverseSelections : public wxTreeTraversal
{
public:
    TraverseSelections(const wxTreeCtrl *tree,
                       wxArrayTreeItemIds& selections)
        : wxTreeTraversal(tree), m_selections(selections)
        {
            m_selections.Empty();

            if (tree->GetCount() > 0)
                DoTraverse(tree->GetRootItem());
        }

    virtual bool OnVisit(const wxTreeItemId& item)
    {
        const wxTreeCtrl * const tree = GetTree();

                if ( (tree->GetRootItem() == item) && tree->HasFlag(wxTR_HIDE_ROOT) )
        {
            return true;
        }

        if ( ::IsItemSelected(GetHwndOf(tree), HITEM(item)) )
        {
            m_selections.Add(item);
        }

        return true;
    }

    size_t GetCount() const { return m_selections.GetCount(); }

private:
    wxArrayTreeItemIds& m_selections;

    wxDECLARE_NO_COPY_CLASS(TraverseSelections);
};

class TraverseCounter : public wxTreeTraversal
{
public:
    TraverseCounter(const wxTreeCtrl *tree,
                    const wxTreeItemId& root,
                    bool recursively)
        : wxTreeTraversal(tree)
        {
            m_count = 0;

            DoTraverse(root, recursively);
        }

    virtual bool OnVisit(const wxTreeItemId& WXUNUSED(item))
    {
        m_count++;

        return true;
    }

    size_t GetCount() const { return m_count; }

private:
    size_t m_count;

    wxDECLARE_NO_COPY_CLASS(TraverseCounter);
};



enum
{
    IDX_COLLAPSE,
    IDX_EXPAND,
    IDX_WHAT_MAX
};

enum
{
    IDX_DONE,
    IDX_DOING,
    IDX_HOW_MAX
};

static  wxEventType gs_expandEvents[IDX_WHAT_MAX][IDX_HOW_MAX];





bool wxTreeTraversal::DoTraverse(const wxTreeItemId& root, bool recursively)
{
    if ( !OnVisit(root) )
        return false;

    return Traverse(root, recursively);
}

bool wxTreeTraversal::Traverse(const wxTreeItemId& root, bool recursively)
{
    wxTreeItemIdValue cookie;
    wxTreeItemId child = m_tree->GetFirstChild(root, cookie);
    while ( child.IsOk() )
    {
                if ( recursively && !Traverse(child, true) )
            return false;

        if ( !OnVisit(child) )
            return false;

        child = m_tree->GetNextChild(root, cookie);
    }

    return true;
}


void wxTreeCtrl::Init()
{
    m_textCtrl = NULL;
    m_hasAnyAttr = false;
#if wxUSE_DRAGIMAGE
    m_dragImage = NULL;
#endif
    m_pVirtualRoot = NULL;
    m_dragStarted = false;
    m_focusLost = true;
    m_changingSelection = false;
    m_triggerStateImageClick = false;
    m_mouseUpDeselect = false;

            gs_expandEvents[IDX_COLLAPSE][IDX_DONE] = wxEVT_TREE_ITEM_COLLAPSED;
    gs_expandEvents[IDX_COLLAPSE][IDX_DOING] = wxEVT_TREE_ITEM_COLLAPSING;
    gs_expandEvents[IDX_EXPAND][IDX_DONE] = wxEVT_TREE_ITEM_EXPANDED;
    gs_expandEvents[IDX_EXPAND][IDX_DOING] = wxEVT_TREE_ITEM_EXPANDING;
}

bool wxTreeCtrl::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        long style,
                        const wxValidator& validator,
                        const wxString& name)
{
    Init();

    if ( (style & wxBORDER_MASK) == wxBORDER_DEFAULT )
        style |= wxBORDER_SUNKEN;

    if ( !CreateControl(parent, id, pos, size, style, validator, name) )
        return false;

    WXDWORD exStyle = 0;
    DWORD wstyle = MSWGetStyle(m_windowStyle, & exStyle);
    wstyle |= WS_TABSTOP | TVS_SHOWSELALWAYS;

    if ( !(m_windowStyle & wxTR_NO_LINES) )
        wstyle |= TVS_HASLINES;
    if ( m_windowStyle & wxTR_HAS_BUTTONS )
        wstyle |= TVS_HASBUTTONS;

    if ( m_windowStyle & wxTR_EDIT_LABELS )
        wstyle |= TVS_EDITLABELS;

    if ( m_windowStyle & wxTR_LINES_AT_ROOT )
        wstyle |= TVS_LINESATROOT;

    if ( m_windowStyle & wxTR_FULL_ROW_HIGHLIGHT )
    {
        wstyle |= TVS_FULLROWSELECT;
    }

#if defined(TVS_INFOTIP)
        wstyle |= TVS_INFOTIP;
#endif

        if ( !MSWCreateControl(WC_TREEVIEW, wstyle, pos, size) )
        return false;

    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
    SetForegroundColour(wxWindow::GetParent()->GetForegroundColour());

    wxSetCCUnicodeFormat(GetHwnd());

    if ( m_windowStyle & wxTR_TWIST_BUTTONS )
    {
                        EnableSystemTheme();
    }

    return true;
}

wxTreeCtrl::~wxTreeCtrl()
{
    m_isBeingDeleted = true;

        if ( m_hasAnyAttr )
    {
        WX_CLEAR_HASH_MAP(wxMapTreeAttr, m_attrs);

                m_hasAnyAttr = false;
    }

    DeleteTextCtrl();

            DeleteAllItems();
}


 wxVisualAttributes
wxTreeCtrl::GetClassDefaultAttributes(wxWindowVariant variant)
{
    wxVisualAttributes attrs = GetCompositeControlsDefaultAttributes(variant);

        attrs.font = wxGetCCDefaultFont();

    return attrs;
}



bool wxTreeCtrl::DoGetItem(wxTreeViewItem *tvItem) const
{
    wxCHECK_MSG( tvItem->hItem != TVI_ROOT, false,
                 wxT("can't retrieve virtual root item") );

    if ( !TreeView_GetItem(GetHwnd(), tvItem) )
    {
        wxLogLastError(wxT("TreeView_GetItem"));

        return false;
    }

    return true;
}

void wxTreeCtrl::DoSetItem(wxTreeViewItem *tvItem)
{
    TreeItemUnlocker unlocker(tvItem->hItem);

    if ( TreeView_SetItem(GetHwnd(), tvItem) == -1 )
    {
        wxLogLastError(wxT("TreeView_SetItem"));
    }
}

unsigned int wxTreeCtrl::GetCount() const
{
    return (unsigned int)TreeView_GetCount(GetHwnd());
}

unsigned int wxTreeCtrl::GetIndent() const
{
    return TreeView_GetIndent(GetHwnd());
}

void wxTreeCtrl::SetIndent(unsigned int indent)
{
    (void)TreeView_SetIndent(GetHwnd(), indent);
}

void wxTreeCtrl::SetAnyImageList(wxImageList *imageList, int which)
{
        (void) TreeView_SetImageList(GetHwnd(),
                                 imageList ? imageList->GetHIMAGELIST() : 0,
                                 which);
}

void wxTreeCtrl::SetImageList(wxImageList *imageList)
{
    if (m_ownsImageListNormal)
        delete m_imageListNormal;

    SetAnyImageList(m_imageListNormal = imageList, TVSIL_NORMAL);
    m_ownsImageListNormal = false;
}

void wxTreeCtrl::SetStateImageList(wxImageList *imageList)
{
    if (m_ownsImageListState) delete m_imageListState;
    SetAnyImageList(m_imageListState = imageList, TVSIL_STATE);
    m_ownsImageListState = false;
}

size_t wxTreeCtrl::GetChildrenCount(const wxTreeItemId& item,
                                    bool recursively) const
{
    wxCHECK_MSG( item.IsOk(), 0u, wxT("invalid tree item") );

    TraverseCounter counter(this, item, recursively);
    return counter.GetCount() - 1;
}


bool wxTreeCtrl::SetBackgroundColour(const wxColour &colour)
{
    if ( !wxWindowBase::SetBackgroundColour(colour) )
        return false;

    ::SendMessage(GetHwnd(), TVM_SETBKCOLOR, 0, colour.GetPixel());

    return true;
}

bool wxTreeCtrl::SetForegroundColour(const wxColour &colour)
{
    if ( !wxWindowBase::SetForegroundColour(colour) )
        return false;

    ::SendMessage(GetHwnd(), TVM_SETTEXTCOLOR, 0, colour.GetPixel());

    return true;
}


bool wxTreeCtrl::IsHiddenRoot(const wxTreeItemId& item) const
{
    return HITEM(item) == TVI_ROOT && HasFlag(wxTR_HIDE_ROOT);
}

wxString wxTreeCtrl::GetItemText(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxEmptyString, wxT("invalid tree item") );

    wxChar buf[512];  
    wxTreeViewItem tvItem(item, TVIF_TEXT);
    tvItem.pszText = buf;
    tvItem.cchTextMax = WXSIZEOF(buf);
    if ( !DoGetItem(&tvItem) )
    {
                buf[0] = wxT('\0');
    }

    return wxString(buf);
}

void wxTreeCtrl::SetItemText(const wxTreeItemId& item, const wxString& text)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    if ( IS_VIRTUAL_ROOT(item) )
        return;

    wxTreeViewItem tvItem(item, TVIF_TEXT);
    tvItem.pszText = wxMSW_CONV_LPTSTR(text);
    DoSetItem(&tvItem);

                        HWND hwndEdit = TreeView_GetEditControl(GetHwnd());
    if ( hwndEdit )
    {
        if ( item == m_idEdited )
        {
            ::SetWindowText(hwndEdit, text.t_str());
        }
    }
}

int wxTreeCtrl::GetItemImage(const wxTreeItemId& item,
                             wxTreeItemIcon which) const
{
    wxCHECK_MSG( item.IsOk(), -1, wxT("invalid tree item") );

    if ( IsHiddenRoot(item) )
    {
                return -1;
    }

    wxTreeItemParam *param = GetItemParam(item);

    return param && param->HasImage(which) ? param->GetImage(which) : -1;
}

void wxTreeCtrl::SetItemImage(const wxTreeItemId& item, int image,
                              wxTreeItemIcon which)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );
    wxCHECK_RET( which >= 0 &&
                 which < wxTreeItemIcon_Max,
                 wxT("invalid image index"));


    if ( IsHiddenRoot(item) )
    {
                return;
    }

    wxTreeItemParam *data = GetItemParam(item);
    if ( !data )
        return;

    data->SetImage(image, which);

    RefreshItem(item);
}

wxTreeItemParam *wxTreeCtrl::GetItemParam(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), NULL, wxT("invalid tree item") );

    wxTreeViewItem tvItem(item, TVIF_PARAM);

        if ( IS_VIRTUAL_ROOT(item) )
    {
        return GET_VIRTUAL_ROOT()->GetParam();
    }

        if ( !DoGetItem(&tvItem) )
    {
        return NULL;
    }

    return (wxTreeItemParam *)tvItem.lParam;
}

bool wxTreeCtrl::HandleTreeEvent(wxTreeEvent& event) const
{
    if ( event.m_item.IsOk() )
    {
        event.SetClientObject(GetItemData(event.m_item));
    }

    return HandleWindowEvent(event);
}

wxTreeItemData *wxTreeCtrl::GetItemData(const wxTreeItemId& item) const
{
    wxTreeItemParam *data = GetItemParam(item);

    return data ? data->GetData() : NULL;
}

void wxTreeCtrl::SetItemData(const wxTreeItemId& item, wxTreeItemData *data)
{
        if ( data )
    {
        data->SetId(item);
    }

    wxTreeItemParam *param = GetItemParam(item);

    wxCHECK_RET( param, wxT("failed to change tree items data") );

    param->SetData(data);
}

void wxTreeCtrl::SetItemHasChildren(const wxTreeItemId& item, bool has)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    if ( IS_VIRTUAL_ROOT(item) )
        return;

    wxTreeViewItem tvItem(item, TVIF_CHILDREN);
    tvItem.cChildren = (int)has;
    DoSetItem(&tvItem);
}

void wxTreeCtrl::SetItemBold(const wxTreeItemId& item, bool bold)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    if ( IS_VIRTUAL_ROOT(item) )
        return;

    wxTreeViewItem tvItem(item, TVIF_STATE, TVIS_BOLD);
    tvItem.state = bold ? TVIS_BOLD : 0;
    DoSetItem(&tvItem);
}

void wxTreeCtrl::SetItemDropHighlight(const wxTreeItemId& item, bool highlight)
{
    if ( IS_VIRTUAL_ROOT(item) )
        return;

    wxTreeViewItem tvItem(item, TVIF_STATE, TVIS_DROPHILITED);
    tvItem.state = highlight ? TVIS_DROPHILITED : 0;
    DoSetItem(&tvItem);
}

void wxTreeCtrl::RefreshItem(const wxTreeItemId& item)
{
    if ( IS_VIRTUAL_ROOT(item) )
        return;

    wxRect rect;
    if ( GetBoundingRect(item, rect) )
    {
        RefreshRect(rect);
    }
}

wxColour wxTreeCtrl::GetItemTextColour(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxNullColour, wxT("invalid tree item") );

    wxMapTreeAttr::const_iterator it = m_attrs.find(item.m_pItem);
    return it == m_attrs.end() ? wxNullColour : it->second->GetTextColour();
}

wxColour wxTreeCtrl::GetItemBackgroundColour(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxNullColour, wxT("invalid tree item") );

    wxMapTreeAttr::const_iterator it = m_attrs.find(item.m_pItem);
    return it == m_attrs.end() ? wxNullColour : it->second->GetBackgroundColour();
}

wxFont wxTreeCtrl::GetItemFont(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxNullFont, wxT("invalid tree item") );

    wxMapTreeAttr::const_iterator it = m_attrs.find(item.m_pItem);
    return it == m_attrs.end() ? wxNullFont : it->second->GetFont();
}

void wxTreeCtrl::SetItemTextColour(const wxTreeItemId& item,
                                   const wxColour& col)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    wxTreeItemAttr *attr;
    wxMapTreeAttr::iterator it = m_attrs.find(item.m_pItem);
    if ( it == m_attrs.end() )
    {
        m_hasAnyAttr = true;

        m_attrs[item.m_pItem] =
        attr = new wxTreeItemAttr;
    }
    else
    {
        attr = it->second;
    }

    attr->SetTextColour(col);

    RefreshItem(item);
}

void wxTreeCtrl::SetItemBackgroundColour(const wxTreeItemId& item,
                                         const wxColour& col)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    wxTreeItemAttr *attr;
    wxMapTreeAttr::iterator it = m_attrs.find(item.m_pItem);
    if ( it == m_attrs.end() )
    {
        m_hasAnyAttr = true;

        m_attrs[item.m_pItem] =
        attr = new wxTreeItemAttr;
    }
    else     {
        attr = it->second;
    }

    attr->SetBackgroundColour(col);

    RefreshItem(item);
}

void wxTreeCtrl::SetItemFont(const wxTreeItemId& item, const wxFont& font)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    wxTreeItemAttr *attr;
    wxMapTreeAttr::iterator it = m_attrs.find(item.m_pItem);
    if ( it == m_attrs.end() )
    {
        m_hasAnyAttr = true;

        m_attrs[item.m_pItem] =
        attr = new wxTreeItemAttr;
    }
    else     {
        attr = it->second;
    }

    attr->SetFont(font);

            SetItemText(item, GetItemText(item));

    RefreshItem(item);
}


bool wxTreeCtrl::IsVisible(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), false, wxT("invalid tree item") );

    if ( item == wxTreeItemId(TVI_ROOT) )
    {
                return false;
    }

        TVGetItemRectParam param;

        if ( !wxTreeView_GetItemRect(GetHwnd(), HITEM(item), param, TRUE) )
    {
                        return false;
    }

                return param.rect.bottom > 0 && param.rect.top < GetClientSize().y;
}

bool wxTreeCtrl::ItemHasChildren(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), false, wxT("invalid tree item") );

    if ( IS_VIRTUAL_ROOT(item) )
    {
        wxTreeItemIdValue cookie;
        return GetFirstChild(item, cookie).IsOk();
    }

    wxTreeViewItem tvItem(item, TVIF_CHILDREN);
    DoGetItem(&tvItem);

    return tvItem.cChildren != 0;
}

bool wxTreeCtrl::IsExpanded(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), false, wxT("invalid tree item") );

    wxTreeViewItem tvItem(item, TVIF_STATE, TVIS_EXPANDED);
    DoGetItem(&tvItem);

    return (tvItem.state & TVIS_EXPANDED) != 0;
}

bool wxTreeCtrl::IsSelected(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), false, wxT("invalid tree item") );

    wxTreeViewItem tvItem(item, TVIF_STATE, TVIS_SELECTED);
    DoGetItem(&tvItem);

    return (tvItem.state & TVIS_SELECTED) != 0;
}

bool wxTreeCtrl::IsBold(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), false, wxT("invalid tree item") );

    wxTreeViewItem tvItem(item, TVIF_STATE, TVIS_BOLD);
    DoGetItem(&tvItem);

    return (tvItem.state & TVIS_BOLD) != 0;
}


wxTreeItemId wxTreeCtrl::GetRootItem() const
{
        if ( GET_VIRTUAL_ROOT() )
        return TVI_ROOT;

    return wxTreeItemId(TreeView_GetRoot(GetHwnd()));
}

wxTreeItemId wxTreeCtrl::GetSelection() const
{
    wxCHECK_MSG( !HasFlag(wxTR_MULTIPLE), wxTreeItemId(),
                 wxT("this only works with single selection controls") );

    return GetFocusedItem();
}

wxTreeItemId wxTreeCtrl::GetFocusedItem() const
{
    return wxTreeItemId(TreeView_GetSelection(GetHwnd()));
}

wxTreeItemId wxTreeCtrl::GetItemParent(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );

    HTREEITEM hItem;

    if ( IS_VIRTUAL_ROOT(item) )
    {
                hItem = 0;
    }
    else     {
        hItem = TreeView_GetParent(GetHwnd(), HITEM(item));
        if ( !hItem && HasFlag(wxTR_HIDE_ROOT) )
        {
                        hItem = TVI_ROOT;
        }
    }

    return wxTreeItemId(hItem);
}

wxTreeItemId wxTreeCtrl::GetFirstChild(const wxTreeItemId& item,
                                       wxTreeItemIdValue& cookie) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );

        cookie = TreeView_GetChild(GetHwnd(), HITEM(item));

    return wxTreeItemId(cookie);
}

wxTreeItemId wxTreeCtrl::GetNextChild(const wxTreeItemId& WXUNUSED(item),
                                      wxTreeItemIdValue& cookie) const
{
    wxTreeItemId fromCookie(cookie);

    HTREEITEM hitem = HITEM(fromCookie);

    hitem = TreeView_GetNextSibling(GetHwnd(), hitem);

    wxTreeItemId item(hitem);

    cookie = item.m_pItem;

    return item;
}

wxTreeItemId wxTreeCtrl::GetLastChild(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );

        wxTreeItemIdValue cookie;

    wxTreeItemId childLast,
    child = GetFirstChild(item, cookie);
    while ( child.IsOk() )
    {
        childLast = child;
        child = GetNextChild(item, cookie);
    }

    return childLast;
}

wxTreeItemId wxTreeCtrl::GetNextSibling(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );
    return wxTreeItemId(TreeView_GetNextSibling(GetHwnd(), HITEM(item)));
}

wxTreeItemId wxTreeCtrl::GetPrevSibling(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );
    return wxTreeItemId(TreeView_GetPrevSibling(GetHwnd(), HITEM(item)));
}

wxTreeItemId wxTreeCtrl::GetFirstVisibleItem() const
{
    return wxTreeItemId(TreeView_GetFirstVisible(GetHwnd()));
}

wxTreeItemId wxTreeCtrl::GetNextVisible(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );
    wxASSERT_MSG( IsVisible(item), wxT("The item you call GetNextVisible() for must be visible itself!"));

    wxTreeItemId next(TreeView_GetNextVisible(GetHwnd(), HITEM(item)));
    if ( next.IsOk() && !IsVisible(next) )
    {
                        next.Unset();
    }

    return next;
}

wxTreeItemId wxTreeCtrl::GetPrevVisible(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );
    wxASSERT_MSG( IsVisible(item), wxT("The item you call GetPrevVisible() for must be visible itself!"));

    wxTreeItemId prev(TreeView_GetPrevVisible(GetHwnd(), HITEM(item)));
    if ( prev.IsOk() && !IsVisible(prev) )
    {
                        prev.Unset();
    }

    return prev;
}


size_t wxTreeCtrl::GetSelections(wxArrayTreeItemIds& selections) const
{
    TraverseSelections selector(this, selections);

    return selector.GetCount();
}


wxTreeItemId wxTreeCtrl::DoInsertAfter(const wxTreeItemId& parent,
                                       const wxTreeItemId& hInsertAfter,
                                       const wxString& text,
                                       int image, int selectedImage,
                                       wxTreeItemData *data)
{
    wxCHECK_MSG( parent.IsOk() || !TreeView_GetRoot(GetHwnd()),
                 wxTreeItemId(),
                 wxT("can't have more than one root in the tree") );

    TV_INSERTSTRUCT tvIns;
    tvIns.hParent = HITEM(parent);
    tvIns.hInsertAfter = HITEM(hInsertAfter);

            if ( !tvIns.hInsertAfter )
    {
        tvIns.hInsertAfter = TVI_FIRST;
    }

    UINT mask = 0;
    if ( !text.empty() )
    {
        mask |= TVIF_TEXT;
        tvIns.item.pszText = wxMSW_CONV_LPTSTR(text);
    }
    else
    {
        tvIns.item.pszText = NULL;
        tvIns.item.cchTextMax = 0;
    }

        wxTreeItemParam *param = new wxTreeItemParam;

            mask |= TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvIns.item.iImage = I_IMAGECALLBACK;
    tvIns.item.iSelectedImage = I_IMAGECALLBACK;

    param->SetImage(image, wxTreeItemIcon_Normal);
    param->SetImage(selectedImage, wxTreeItemIcon_Selected);

    mask |= TVIF_PARAM;
    tvIns.item.lParam = (LPARAM)param;
    tvIns.item.mask = mask;

            const bool firstChild = !IsHiddenRoot(parent) &&
                                !TreeView_GetChild(GetHwnd(), HITEM(parent));

    HTREEITEM id = TreeView_InsertItem(GetHwnd(), &tvIns);
    if ( id == 0 )
    {
        wxLogLastError(wxT("TreeView_InsertItem"));
    }

                if ( firstChild )
    {
        TVGetItemRectParam param2;

        wxTreeView_GetItemRect(GetHwnd(), HITEM(parent), param2, FALSE);
        ::InvalidateRect(GetHwnd(), &param2.rect, FALSE);
    }

        param->SetItem(id);

        if ( data != NULL )
    {
        param->SetData(data);
        data->SetId(id);
    }

    return wxTreeItemId(id);
}

wxTreeItemId wxTreeCtrl::AddRoot(const wxString& text,
                                 int image, int selectedImage,
                                 wxTreeItemData *data)
{
    if ( HasFlag(wxTR_HIDE_ROOT) )
    {
        wxASSERT_MSG( !m_pVirtualRoot, wxT("tree can have only a single root") );

                wxTreeItemParam *param = new wxTreeItemParam;
        param->SetData(data);

        m_pVirtualRoot = new wxVirtualNode(param);

        return TVI_ROOT;
    }

    return DoInsertAfter(wxTreeItemId(), wxTreeItemId(),
                           text, image, selectedImage, data);
}

wxTreeItemId wxTreeCtrl::DoInsertItem(const wxTreeItemId& parent,
                                      size_t index,
                                      const wxString& text,
                                      int image, int selectedImage,
                                      wxTreeItemData *data)
{
    wxTreeItemId idPrev;
    if ( index == (size_t)-1 )
    {
                idPrev = TVI_LAST;
    }
    else     {
        wxTreeItemIdValue cookie;
        wxTreeItemId idCur = GetFirstChild(parent, cookie);
        while ( index != 0 && idCur.IsOk() )
        {
            index--;

            idPrev = idCur;
            idCur = GetNextChild(parent, cookie);
        }

                        wxASSERT_MSG( index == 0, wxT("bad index in wxTreeCtrl::InsertItem") );
    }

    return DoInsertAfter(parent, idPrev, text, image, selectedImage, data);
}

bool wxTreeCtrl::MSWDeleteItem(const wxTreeItemId& item)
{
    TempSetter set(m_changingSelection);
    if ( !TreeView_DeleteItem(GetHwnd(), HITEM(item)) )
    {
        wxLogLastError(wxT("TreeView_DeleteItem"));
        return false;
    }

    return true;
}

void wxTreeCtrl::Delete(const wxTreeItemId& item)
{
            TreeItemUnlocker unlock_all;

    if ( HasFlag(wxTR_MULTIPLE) )
    {
        bool selected = IsSelected(item);
        wxTreeItemId next;

        if ( selected )
        {
            next = TreeView_GetNextVisible(GetHwnd(), HITEM(item));

            if ( !next.IsOk() )
            {
                next = TreeView_GetPrevVisible(GetHwnd(), HITEM(item));
            }
        }

        if ( !MSWDeleteItem(item) )
            return;

        if ( !selected )
        {
            return;
        }

        if ( item == m_htSelStart )
            m_htSelStart.Unset();

        if ( item == m_htClickedItem )
            m_htClickedItem.Unset();

        if ( next.IsOk() )
        {
            wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING, this, next);

            if ( IsTreeEventAllowed(changingEvent) )
            {
                wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED, this, next);
                (void)HandleTreeEvent(changedEvent);
            }
            else
            {
                DoUnselectItem(next);
                ClearFocusedItem();
            }
        }
    }
    else
    {
        MSWDeleteItem(item);
    }
}

void wxTreeCtrl::DeleteChildren(const wxTreeItemId& item)
{
        TreeItemUnlocker unlock_all;

    wxTreeItemIdValue cookie;

    wxArrayTreeItemIds children;
    wxTreeItemId child = GetFirstChild(item, cookie);
    while ( child.IsOk() )
    {
        children.Add(child);

        child = GetNextChild(item, cookie);
    }

    size_t nCount = children.Count();
    for ( size_t n = 0; n < nCount; n++ )
    {
        Delete(children[n]);
    }
}

void wxTreeCtrl::DeleteAllItems()
{
        TreeItemUnlocker unlock_all;

        m_htSelStart =
    m_htClickedItem = wxTreeItemId();

        if ( GET_VIRTUAL_ROOT() )
    {
        delete GET_VIRTUAL_ROOT();
        m_pVirtualRoot = NULL;
    }

    
    if ( !TreeView_DeleteAllItems(GetHwnd()) )
    {
        wxLogLastError(wxT("TreeView_DeleteAllItems"));
    }
}

void wxTreeCtrl::DoExpand(const wxTreeItemId& item, int flag)
{
    wxASSERT_MSG( flag == TVE_COLLAPSE ||
                  flag == (TVE_COLLAPSE | TVE_COLLAPSERESET) ||
                  flag == TVE_EXPAND   ||
                  flag == TVE_TOGGLE,
                  wxT("Unknown flag in wxTreeCtrl::DoExpand") );

        wxCHECK_RET( !IsHiddenRoot(item),
                 wxT("Can't expand/collapse hidden root node!") );

                            
    wxTreeViewItem tvItem(item, TVIF_STATE, TVIS_EXPANDEDONCE);
    tvItem.state = 0;
    DoSetItem(&tvItem);

    if ( IsExpanded(item) )
    {
        wxTreeEvent event(wxEVT_TREE_ITEM_COLLAPSING,
                          this, wxTreeItemId(item));

        if ( !IsTreeEventAllowed(event) )
            return;
    }

    if ( TreeView_Expand(GetHwnd(), HITEM(item), flag) )
    {
        if ( IsExpanded(item) )
            return;

        wxTreeEvent event(wxEVT_TREE_ITEM_COLLAPSED, this, item);
        (void)HandleTreeEvent(event);
    }
    }

void wxTreeCtrl::Expand(const wxTreeItemId& item)
{
    DoExpand(item, TVE_EXPAND);
}

void wxTreeCtrl::Collapse(const wxTreeItemId& item)
{
    DoExpand(item, TVE_COLLAPSE);
}

void wxTreeCtrl::CollapseAndReset(const wxTreeItemId& item)
{
    DoExpand(item, TVE_COLLAPSE | TVE_COLLAPSERESET);
}

void wxTreeCtrl::Toggle(const wxTreeItemId& item)
{
    DoExpand(item, TVE_TOGGLE);
}

void wxTreeCtrl::Unselect()
{
    wxASSERT_MSG( !HasFlag(wxTR_MULTIPLE),
                  wxT("doesn't make sense, may be you want UnselectAll()?") );

        HTREEITEM htFocus = (HTREEITEM)TreeView_GetSelection(GetHwnd());

    if ( !htFocus )
    {
        return;
    }

    if ( HasFlag(wxTR_MULTIPLE) )
    {
        wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING,
                                  this, wxTreeItemId());
        changingEvent.m_itemOld = htFocus;

        if ( IsTreeEventAllowed(changingEvent) )
        {
            ClearFocusedItem();

            wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                     this, wxTreeItemId());
            changedEvent.m_itemOld = htFocus;
            (void)HandleTreeEvent(changedEvent);
        }
    }
    else
    {
        ClearFocusedItem();
    }
}

void wxTreeCtrl::DoUnselectAll()
{
    wxArrayTreeItemIds selections;
    size_t count = GetSelections(selections);

    for ( size_t n = 0; n < count; n++ )
    {
        DoUnselectItem(selections[n]);
    }

    m_htSelStart.Unset();
}

void wxTreeCtrl::UnselectAll()
{
    if ( HasFlag(wxTR_MULTIPLE) )
    {
        HTREEITEM htFocus = (HTREEITEM)TreeView_GetSelection(GetHwnd());
        if ( !htFocus ) return;

        wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING, this);
        changingEvent.m_itemOld = htFocus;

        if ( IsTreeEventAllowed(changingEvent) )
        {
            DoUnselectAll();

            wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED, this);
            changedEvent.m_itemOld = htFocus;
            (void)HandleTreeEvent(changedEvent);
        }
    }
    else
    {
        Unselect();
    }
}

void wxTreeCtrl::DoSelectChildren(const wxTreeItemId& parent)
{
    DoUnselectAll();

    wxTreeItemIdValue cookie;
    wxTreeItemId child = GetFirstChild(parent, cookie);
    while ( child.IsOk() )
    {
        DoSelectItem(child, true);
        child = GetNextChild(child, cookie);
    }
}

void wxTreeCtrl::SelectChildren(const wxTreeItemId& parent)
{
    wxCHECK_RET( HasFlag(wxTR_MULTIPLE),
                 "this only works with multiple selection controls" );

    HTREEITEM htFocus = (HTREEITEM)TreeView_GetSelection(GetHwnd());

    wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING, this);
    changingEvent.m_itemOld = htFocus;

    if ( IsTreeEventAllowed(changingEvent) )
    {
        DoSelectChildren(parent);

        wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED, this);
        changedEvent.m_itemOld = htFocus;
        (void)HandleTreeEvent(changedEvent);
    }
}

void wxTreeCtrl::DoSelectItem(const wxTreeItemId& item, bool select)
{
    TempSetter set(m_changingSelection);

    ::SelectItem(GetHwnd(), HITEM(item), select);
}

void wxTreeCtrl::SelectItem(const wxTreeItemId& item, bool select)
{
    wxCHECK_RET( !IsHiddenRoot(item), wxT("can't select hidden root item") );

    if ( select == IsSelected(item) )
    {
                return;
    }

    if ( HasFlag(wxTR_MULTIPLE) )
    {
        wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING, this, item);

        if ( IsTreeEventAllowed(changingEvent) )
        {
            HTREEITEM htFocus = (HTREEITEM)TreeView_GetSelection(GetHwnd());
            DoSelectItem(item, select);

            if ( !htFocus )
            {
                SetFocusedItem(item);
            }

            wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                     this, item);
            (void)HandleTreeEvent(changedEvent);
        }
    }
    else     {
        wxTreeItemId itemOld, itemNew;
        if ( select )
        {
            itemOld = GetSelection();
            itemNew = item;
        }
        else         {
            itemOld = item;
                    }

                                        wxTreeEvent
            changingEvent(wxEVT_TREE_SEL_CHANGING, this, itemNew);
        changingEvent.SetOldItem(itemOld);

        if ( IsTreeEventAllowed(changingEvent) )
        {
            TempSetter set(m_changingSelection);

            if ( !TreeView_SelectItem(GetHwnd(), HITEM(itemNew)) )
            {
                wxLogLastError(wxT("TreeView_SelectItem"));
            }
            else             {
                ::SetFocus(GetHwnd(), HITEM(item));

                wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                         this, itemNew);
                changedEvent.SetOldItem(itemOld);
                (void)HandleTreeEvent(changedEvent);
            }
        }
            }
}

void wxTreeCtrl::EnsureVisible(const wxTreeItemId& item)
{
    wxCHECK_RET( !IsHiddenRoot(item), wxT("can't show hidden root item") );

        (void)TreeView_EnsureVisible(GetHwnd(), HITEM(item));
}

void wxTreeCtrl::ScrollTo(const wxTreeItemId& item)
{
    if ( !TreeView_SelectSetFirstVisible(GetHwnd(), HITEM(item)) )
    {
        wxLogLastError(wxT("TreeView_SelectSetFirstVisible"));
    }
}

wxTextCtrl *wxTreeCtrl::GetEditControl() const
{
    return m_textCtrl;
}

void wxTreeCtrl::DeleteTextCtrl()
{
    if ( m_textCtrl )
    {
                                if ( !::IsWindow(GetHwndOf(m_textCtrl)) )
        {
            m_textCtrl->SetHWND(0);
        }

        m_textCtrl->UnsubclassWin();
        m_textCtrl->SetHWND(0);
        wxDELETE(m_textCtrl);

        m_idEdited.Unset();
    }
}

wxTextCtrl *wxTreeCtrl::EditLabel(const wxTreeItemId& item,
                                  wxClassInfo *textControlClass)
{
    wxASSERT( textControlClass->IsKindOf(wxCLASSINFO(wxTextCtrl)) );

    DeleteTextCtrl();

    m_idEdited = item;
    m_textCtrl = (wxTextCtrl *)textControlClass->CreateObject();
    HWND hWnd = (HWND) TreeView_EditLabel(GetHwnd(), HITEM(item));

            if ( !hWnd )
    {
        wxDELETE(m_textCtrl);
        return NULL;
    }

        return m_textCtrl;
}

void wxTreeCtrl::DoEndEditLabel(bool discardChanges)
{
    if ( !TreeView_EndEditLabelNow(GetHwnd(), discardChanges) )
        wxLogLastError(wxS("TreeView_EndEditLabelNow()"));

    DeleteTextCtrl();
}

wxTreeItemId wxTreeCtrl::DoTreeHitTest(const wxPoint& point, int& flags) const
{
    TV_HITTESTINFO hitTestInfo;
    hitTestInfo.pt.x = (int)point.x;
    hitTestInfo.pt.y = (int)point.y;

    (void) TreeView_HitTest(GetHwnd(), &hitTestInfo);

    flags = 0;

        #define TRANSLATE_FLAG(flag) if ( hitTestInfo.flags & TVHT_##flag ) \
                                    flags |= wxTREE_HITTEST_##flag

    TRANSLATE_FLAG(ABOVE);
    TRANSLATE_FLAG(BELOW);
    TRANSLATE_FLAG(NOWHERE);
    TRANSLATE_FLAG(ONITEMBUTTON);
    TRANSLATE_FLAG(ONITEMICON);
    TRANSLATE_FLAG(ONITEMINDENT);
    TRANSLATE_FLAG(ONITEMLABEL);
    TRANSLATE_FLAG(ONITEMRIGHT);
    TRANSLATE_FLAG(ONITEMSTATEICON);
    TRANSLATE_FLAG(TOLEFT);
    TRANSLATE_FLAG(TORIGHT);

    #undef TRANSLATE_FLAG

    return wxTreeItemId(hitTestInfo.hItem);
}

bool wxTreeCtrl::GetBoundingRect(const wxTreeItemId& item,
                                 wxRect& rect,
                                 bool textOnly) const
{
        if ( IS_VIRTUAL_ROOT(item) )
    {
        return false;
    }

    TVGetItemRectParam param;

    if ( wxTreeView_GetItemRect(GetHwnd(), HITEM(item), param, textOnly) )
    {
        rect = wxRect(wxPoint(param.rect.left, param.rect.top),
                      wxPoint(param.rect.right, param.rect.bottom));

        return true;
    }
    else
    {
                return false;
    }
}

void wxTreeCtrl::ClearFocusedItem()
{
    TempSetter set(m_changingSelection);

    if ( !TreeView_SelectItem(GetHwnd(), 0) )
    {
        wxLogLastError(wxT("TreeView_SelectItem"));
    }
}

void wxTreeCtrl::SetFocusedItem(const wxTreeItemId& item)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    TempSetter set(m_changingSelection);

    ::SetFocus(GetHwnd(), HITEM(item));
}

void wxTreeCtrl::DoUnselectItem(const wxTreeItemId& item)
{
    TempSetter set(m_changingSelection);

    ::UnselectItem(GetHwnd(), HITEM(item));
}

void wxTreeCtrl::DoToggleItemSelection(const wxTreeItemId& item)
{
    TempSetter set(m_changingSelection);

    ::ToggleItemSelection(GetHwnd(), HITEM(item));
}


class wxTreeSortHelper
{
public:
    static int CALLBACK Compare(LPARAM data1, LPARAM data2, LPARAM tree);

private:
    static wxTreeItemId GetIdFromData(LPARAM lParam)
    {
        return ((wxTreeItemParam*)lParam)->GetItem();
        }
};

int CALLBACK wxTreeSortHelper::Compare(LPARAM pItem1,
                                       LPARAM pItem2,
                                       LPARAM htree)
{
    wxCHECK_MSG( pItem1 && pItem2, 0,
                 wxT("sorting tree without data doesn't make sense") );

    wxTreeCtrl *tree = (wxTreeCtrl *)htree;

    return tree->OnCompareItems(GetIdFromData(pItem1),
                                GetIdFromData(pItem2));
}

void wxTreeCtrl::SortChildren(const wxTreeItemId& item)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

                                if ( GetClassInfo() == wxCLASSINFO(wxTreeCtrl) )
    {
        if ( !TreeView_SortChildren(GetHwnd(), HITEM(item), 0) )
            wxLogLastError(wxS("TreeView_SortChildren()"));
    }
    else
    {
        TV_SORTCB tvSort;
        tvSort.hParent = HITEM(item);
        tvSort.lpfnCompare = wxTreeSortHelper::Compare;
        tvSort.lParam = (LPARAM)this;
        if ( !TreeView_SortChildrenCB(GetHwnd(), &tvSort, 0 ) )
            wxLogLastError(wxS("TreeView_SortChildrenCB()"));
    }
}


bool wxTreeCtrl::MSWShouldPreProcessMessage(WXMSG* msg)
{
    if ( msg->message == WM_KEYDOWN )
    {
                        if ( (msg->wParam == VK_RETURN) && !wxIsAnyModifierDown() )
        {
                        return false;
        }
    }

    return wxTreeCtrlBase::MSWShouldPreProcessMessage(msg);
}

bool wxTreeCtrl::MSWCommand(WXUINT cmd, WXWORD id_)
{
    const int id = (signed short)id_;

    if ( cmd == EN_UPDATE )
    {
        wxCommandEvent event(wxEVT_TEXT, id);
        event.SetEventObject( this );
        ProcessCommand(event);
    }
    else if ( cmd == EN_KILLFOCUS )
    {
        wxCommandEvent event(wxEVT_KILL_FOCUS, id);
        event.SetEventObject( this );
        ProcessCommand(event);
    }
    else
    {
                return false;
    }

        return true;
}

bool wxTreeCtrl::MSWIsOnItem(unsigned flags) const
{
    unsigned mask = TVHT_ONITEM;
    if ( HasFlag(wxTR_FULL_ROW_HIGHLIGHT) )
        mask |= TVHT_ONITEMINDENT | TVHT_ONITEMRIGHT;

    return (flags & mask) != 0;
}

bool wxTreeCtrl::MSWHandleSelectionKey(unsigned vkey)
{
    const bool bCtrl = wxIsCtrlDown();
    const bool bShift = wxIsShiftDown();
    const HTREEITEM htSel = (HTREEITEM)TreeView_GetSelection(GetHwnd());

    switch ( vkey )
    {
        case VK_RETURN:
        case VK_SPACE:
            if ( !htSel )
                break;

            if ( vkey != VK_RETURN && bCtrl )
            {
                wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING,
                                          this, htSel);
                changingEvent.m_itemOld = htSel;

                if ( IsTreeEventAllowed(changingEvent) )
                {
                    DoToggleItemSelection(wxTreeItemId(htSel));

                    wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                             this, htSel);
                    changedEvent.m_itemOld = htSel;
                    (void)HandleTreeEvent(changedEvent);
                }
            }
            else
            {
                wxArrayTreeItemIds selections;
                size_t count = GetSelections(selections);

                if ( count != 1 || HITEM(selections[0]) != htSel )
                {
                    wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING,
                                              this, htSel);
                    changingEvent.m_itemOld = htSel;

                    if ( IsTreeEventAllowed(changingEvent) )
                    {
                        DoUnselectAll();
                        DoSelectItem(wxTreeItemId(htSel));

                        wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                                 this, htSel);
                        changedEvent.m_itemOld = htSel;
                        (void)HandleTreeEvent(changedEvent);
                    }
                }
            }
            break;

        case VK_UP:
        case VK_DOWN:
            if ( !bCtrl && !bShift )
            {
                wxArrayTreeItemIds selections;
                wxTreeItemId next;

                if ( htSel )
                {
                    next = vkey == VK_UP
                            ? TreeView_GetPrevVisible(GetHwnd(), htSel)
                            : TreeView_GetNextVisible(GetHwnd(), htSel);
                }
                else
                {
                    next = GetRootItem();

                    if ( IsHiddenRoot(next) )
                        next = TreeView_GetChild(GetHwnd(), HITEM(next));
                }

                if ( !next.IsOk() )
                {
                    break;
                }

                wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING,
                                          this, next);
                changingEvent.m_itemOld = htSel;

                if ( IsTreeEventAllowed(changingEvent) )
                {
                    DoUnselectAll();
                    DoSelectItem(next);
                    SetFocusedItem(next);

                    wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                             this, next);
                    changedEvent.m_itemOld = htSel;
                    (void)HandleTreeEvent(changedEvent);
                }
            }
            else if ( htSel )
            {
                wxTreeItemId next = vkey == VK_UP
                    ? TreeView_GetPrevVisible(GetHwnd(), htSel)
                    : TreeView_GetNextVisible(GetHwnd(), htSel);

                if ( !next.IsOk() )
                {
                    break;
                }

                if ( !m_htSelStart )
                {
                    m_htSelStart = htSel;
                }

                if ( bShift && SelectRange(GetHwnd(), HITEM(m_htSelStart), HITEM(next),
                     SR_UNSELECT_OTHERS | SR_SIMULATE) )
                {
                    wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING, this, next);
                    changingEvent.m_itemOld = htSel;

                    if ( IsTreeEventAllowed(changingEvent) )
                    {
                        SelectRange(GetHwnd(), HITEM(m_htSelStart), HITEM(next),
                                    SR_UNSELECT_OTHERS);

                        wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED, this, next);
                        changedEvent.m_itemOld = htSel;
                        (void)HandleTreeEvent(changedEvent);
                    }
                }

                SetFocusedItem(next);
            }
            break;

        case VK_LEFT:
            if ( HasChildren(htSel) && IsExpanded(htSel) )
            {
                Collapse(htSel);
            }
            else
            {
                wxTreeItemId next = GetItemParent(htSel);

                if ( next.IsOk() && !IsHiddenRoot(next) )
                {
                    wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING,
                                              this, next);
                    changingEvent.m_itemOld = htSel;

                    if ( IsTreeEventAllowed(changingEvent) )
                    {
                        DoUnselectAll();
                        DoSelectItem(next);
                        SetFocusedItem(next);

                        wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                                 this, next);
                        changedEvent.m_itemOld = htSel;
                        (void)HandleTreeEvent(changedEvent);
                    }
                }
            }
            break;

        case VK_RIGHT:
            if ( !IsVisible(htSel) )
            {
                EnsureVisible(htSel);
            }

            if ( !HasChildren(htSel) )
                break;

            if ( !IsExpanded(htSel) )
            {
                Expand(htSel);
            }
            else
            {
                wxTreeItemId next = TreeView_GetChild(GetHwnd(), htSel);

                wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING, this, next);
                changingEvent.m_itemOld = htSel;

                if ( IsTreeEventAllowed(changingEvent) )
                {
                    DoUnselectAll();
                    DoSelectItem(next);
                    SetFocusedItem(next);

                    wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED, this, next);
                    changedEvent.m_itemOld = htSel;
                    (void)HandleTreeEvent(changedEvent);
                }
            }
            break;

        case VK_HOME:
        case VK_END:
            {
                wxTreeItemId next = GetRootItem();

                if ( IsHiddenRoot(next) )
                {
                    next = TreeView_GetChild(GetHwnd(), HITEM(next));
                }

                if ( !next.IsOk() )
                    break;

                if ( vkey == VK_END )
                {
                    for ( ;; )
                    {
                        wxTreeItemId nextTemp = TreeView_GetNextVisible(
                                                    GetHwnd(), HITEM(next));

                        if ( !nextTemp.IsOk() )
                            break;

                        next = nextTemp;
                    }
                }

                if ( htSel == HITEM(next) )
                    break;

                if ( bShift )
                {
                    if ( !m_htSelStart )
                    {
                        m_htSelStart = htSel;
                    }

                    if ( SelectRange(GetHwnd(),
                                     HITEM(m_htSelStart), HITEM(next),
                                     SR_UNSELECT_OTHERS | SR_SIMULATE) )
                    {
                        wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING,
                                                  this, next);
                        changingEvent.m_itemOld = htSel;

                        if ( IsTreeEventAllowed(changingEvent) )
                        {
                            SelectRange(GetHwnd(),
                                        HITEM(m_htSelStart), HITEM(next),
                                        SR_UNSELECT_OTHERS);
                            SetFocusedItem(next);

                            wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                                     this, next);
                            changedEvent.m_itemOld = htSel;
                            (void)HandleTreeEvent(changedEvent);
                        }
                    }
                }
                else                 {
                    wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING,
                                              this, next);
                    changingEvent.m_itemOld = htSel;

                    if ( IsTreeEventAllowed(changingEvent) )
                    {
                        DoUnselectAll();
                        DoSelectItem(next);
                        SetFocusedItem(next);

                        wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                                 this, next);
                        changedEvent.m_itemOld = htSel;
                        (void)HandleTreeEvent(changedEvent);
                    }
                }
            }
            break;

        case VK_PRIOR:
        case VK_NEXT:
            if ( bCtrl )
            {
                wxTreeItemId firstVisible = GetFirstVisibleItem();
                size_t visibleCount = TreeView_GetVisibleCount(GetHwnd());
                wxTreeItemId nextAdjacent = (vkey == VK_PRIOR) ?
                    TreeView_GetPrevVisible(GetHwnd(), HITEM(firstVisible)) :
                    TreeView_GetNextVisible(GetHwnd(), HITEM(firstVisible));

                if ( !nextAdjacent )
                {
                    break;
                }

                wxTreeItemId nextStart = firstVisible;

                for ( size_t n = 1; n < visibleCount; n++ )
                {
                    wxTreeItemId nextTemp = (vkey == VK_PRIOR) ?
                        TreeView_GetPrevVisible(GetHwnd(), HITEM(nextStart)) :
                        TreeView_GetNextVisible(GetHwnd(), HITEM(nextStart));

                    if ( nextTemp.IsOk() )
                    {
                        nextStart = nextTemp;
                    }
                    else
                    {
                        break;
                    }
                }

                EnsureVisible(nextStart);

                if ( vkey == VK_NEXT )
                {
                    wxTreeItemId nextEnd = nextStart;

                    for ( size_t n = 1; n < visibleCount; n++ )
                    {
                        wxTreeItemId nextTemp =
                            TreeView_GetNextVisible(GetHwnd(), HITEM(nextEnd));

                        if ( nextTemp.IsOk() )
                        {
                            nextEnd = nextTemp;
                        }
                        else
                        {
                            break;
                        }
                    }

                    EnsureVisible(nextEnd);
                }
            }
            else             {
                size_t visibleCount = TreeView_GetVisibleCount(GetHwnd());
                wxTreeItemId nextAdjacent = (vkey == VK_PRIOR) ?
                    TreeView_GetPrevVisible(GetHwnd(), htSel) :
                    TreeView_GetNextVisible(GetHwnd(), htSel);

                if ( !nextAdjacent )
                {
                    break;
                }

                wxTreeItemId next(htSel);

                for ( size_t n = 1; n < visibleCount; n++ )
                {
                    wxTreeItemId nextTemp = vkey == VK_PRIOR ?
                        TreeView_GetPrevVisible(GetHwnd(), HITEM(next)) :
                        TreeView_GetNextVisible(GetHwnd(), HITEM(next));

                    if ( !nextTemp.IsOk() )
                        break;

                    next = nextTemp;
                }

                wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING,
                                          this, next);
                changingEvent.m_itemOld = htSel;

                if ( IsTreeEventAllowed(changingEvent) )
                {
                    DoUnselectAll();
                    m_htSelStart.Unset();
                    DoSelectItem(next);
                    SetFocusedItem(next);

                    wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                             this, next);
                    changedEvent.m_itemOld = htSel;
                    (void)HandleTreeEvent(changedEvent);
                }
            }
            break;

        default:
            return false;
    }

    return true;
}

bool wxTreeCtrl::MSWHandleTreeKeyDownEvent(WXWPARAM wParam, WXLPARAM lParam)
{
    wxTreeEvent keyEvent(wxEVT_TREE_KEY_DOWN, this);
    keyEvent.m_evtKey = CreateKeyEvent(wxEVT_KEY_DOWN, wParam, lParam);

    bool processed = HandleTreeEvent(keyEvent);

        if ( !wxIsCtrlDown() && !wxIsShiftDown() && !wxIsAltDown() &&
         ((wParam == VK_SPACE) || (wParam == VK_RETURN)) )
    {
        const HTREEITEM htSel = (HTREEITEM)TreeView_GetSelection(GetHwnd());
        if ( htSel )
        {
            wxTreeEvent activatedEvent(wxEVT_TREE_ITEM_ACTIVATED,
                                       this, htSel);
            (void)HandleTreeEvent(activatedEvent);
        }
    }

    return processed;
}

WXLRESULT
wxTreeCtrl::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
    bool processed = false;
    WXLRESULT rc = 0;
    bool isMultiple = HasFlag(wxTR_MULTIPLE);

    if ( nMsg == WM_CONTEXTMENU )
    {
        int x = GET_X_LPARAM(lParam),
            y = GET_Y_LPARAM(lParam);

                wxTreeItemId item;

                        wxPoint pt;

        if ( x == -1 || y == -1 )
        {
                                                                        item = wxTreeItemId(TreeView_GetSelection(GetHwnd()));
            if ( item.IsOk() )
            {
                                wxRect rect;
                GetBoundingRect(item, rect, true);
                pt = wxPoint(rect.GetX(), rect.GetY() + rect.GetHeight() / 2);
            }
        }
        else         {
            pt = ScreenToClient(wxPoint(x, y));

            TV_HITTESTINFO tvhti;
            tvhti.pt.x = pt.x;
            tvhti.pt.y = pt.y;

            if ( TreeView_HitTest(GetHwnd(), &tvhti) )
                item = wxTreeItemId(tvhti.hItem);
        }

                if ( item.IsOk() )
        {
            wxTreeEvent event(wxEVT_TREE_ITEM_MENU, this, item);

            event.m_pointDrag = pt;

            if ( HandleTreeEvent(event) )
                processed = true;
                    }
    }
    else if ( (nMsg >= WM_MOUSEFIRST) && (nMsg <= WM_MOUSELAST) )
    {
                        int x = GET_X_LPARAM(lParam),
            y = GET_Y_LPARAM(lParam);

        TV_HITTESTINFO tvht;
        tvht.pt.x = x;
        tvht.pt.y = y;

        HTREEITEM htOldItem = TreeView_GetSelection(GetHwnd());
        HTREEITEM htItem = TreeView_HitTest(GetHwnd(), &tvht);

        switch ( nMsg )
        {
            case WM_LBUTTONDOWN:
                if ( !isMultiple )
                    break;

                m_htClickedItem.Unset();

                if ( !MSWIsOnItem(tvht.flags) )
                {
                    if ( tvht.flags & TVHT_ONITEMBUTTON )
                    {
                                                                                                                                                processed = true;
                        SetFocus();

                        if ( !HandleMouseEvent(nMsg, x, y, wParam) )
                        {
                            if ( !IsExpanded(htItem) )
                            {
                                Expand(htItem);
                            }
                            else
                            {
                                Collapse(htItem);
                            }
                        }
                    }

                    m_focusLost = false;
                    break;
                }

                processed = true;
                SetFocus();
                m_htClickedItem = (WXHTREEITEM) htItem;
                m_ptClick = wxPoint(x, y);

                if ( wParam & MK_CONTROL )
                {
                    if ( HandleMouseEvent(nMsg, x, y, wParam) )
                    {
                        m_htClickedItem.Unset();
                        break;
                    }

                    wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING,
                                              this, htItem);
                    changingEvent.m_itemOld = htOldItem;

                    if ( IsTreeEventAllowed(changingEvent) )
                    {
                                                DoToggleItemSelection(wxTreeItemId(htItem));

                        SetFocusedItem(wxTreeItemId(htItem));

                                                m_htSelStart.Unset();

                        wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                                 this, htItem);
                        changedEvent.m_itemOld = htOldItem;
                        (void)HandleTreeEvent(changedEvent);
                    }
                }
                else if ( wParam & MK_SHIFT )
                {
                    if ( HandleMouseEvent(nMsg, x, y, wParam) )
                    {
                        m_htClickedItem.Unset();
                        break;
                    }

                    int srFlags = 0;
                    bool willChange = true;

                    if ( !(wParam & MK_CONTROL) )
                    {
                        srFlags |= SR_UNSELECT_OTHERS;
                    }

                    if ( !m_htSelStart )
                    {
                                                m_htSelStart = htOldItem;
                    }
                    else
                    {
                        willChange = SelectRange(GetHwnd(), HITEM(m_htSelStart),
                                                 htItem, srFlags | SR_SIMULATE);
                    }

                    if ( willChange )
                    {
                        wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING,
                                                  this, htItem);
                        changingEvent.m_itemOld = htOldItem;

                        if ( IsTreeEventAllowed(changingEvent) )
                        {
                                                                                    if ( m_htSelStart )
                            {
                                SelectRange(GetHwnd(), HITEM(m_htSelStart),
                                            htItem, srFlags);
                            }
                            else
                            {
                                DoSelectItem(wxTreeItemId(htItem));
                            }

                            SetFocusedItem(wxTreeItemId(htItem));

                            wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                                     this, htItem);
                            changedEvent.m_itemOld = htOldItem;
                            (void)HandleTreeEvent(changedEvent);
                        }
                    }
                }
                else                 {
                                        
                    wxArrayTreeItemIds selections;
                    size_t count = GetSelections(selections);

                    if ( count == 0 ||
                         count > 1 ||
                         HITEM(selections[0]) != htItem )
                    {
                        if ( HandleMouseEvent(nMsg, x, y, wParam) )
                        {
                            m_htClickedItem.Unset();
                            break;
                        }

                                                                                                                        if ( !IsItemSelected(GetHwnd(), htItem))
                        {
                            wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING,
                                                      this, htItem);
                            changingEvent.m_itemOld = htOldItem;

                            if ( IsTreeEventAllowed(changingEvent) )
                            {
                                DoUnselectAll();
                                DoSelectItem(wxTreeItemId(htItem));
                                SetFocusedItem(wxTreeItemId(htItem));

                                wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                                         this, htItem);
                                changedEvent.m_itemOld = htOldItem;
                                (void)HandleTreeEvent(changedEvent);
                            }
                        }
                        else
                        {
                            SetFocusedItem(wxTreeItemId(htItem));
                            m_mouseUpDeselect = true;
                        }
                    }
                    else                     {
                                                                                                                        m_htClickedItem.Unset();

                                                                        if ( m_focusLost )
                        {
                            ClearFocusedItem();
                            DoSelectItem(wxTreeItemId(htItem));
                            SetFocusedItem(wxTreeItemId(htItem));
                        }
                        else
                        {
                            processed = false;
                        }
                    }

                                        m_htSelStart.Unset();
                }

                m_focusLost = false;

                                                if ( processed )
                {
                    if ( tvht.flags & TVHT_ONITEMSTATEICON )
                    {
                        m_triggerStateImageClick = true;
                    }
                }
                break;

            case WM_RBUTTONDOWN:
                if ( !isMultiple )
                    break;

                processed = true;
                SetFocus();

                if ( HandleMouseEvent(nMsg, x, y, wParam) || !htItem )
                {
                    break;
                }

                                                                                if ( !IsItemSelected(GetHwnd(), htItem) )
                {
                    wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING,
                                              this, htItem);
                    changingEvent.m_itemOld = htOldItem;

                    if ( IsTreeEventAllowed(changingEvent) )
                    {
                        DoUnselectAll();
                        DoSelectItem(wxTreeItemId(htItem));
                        SetFocusedItem(wxTreeItemId(htItem));

                        wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                                 this, htItem);
                        changedEvent.m_itemOld = htOldItem;
                        (void)HandleTreeEvent(changedEvent);
                    }
                }

                break;

            case WM_MOUSEMOVE:
                if ( m_htClickedItem )
                {
                    int cx = abs(m_ptClick.x - x);
                    int cy = abs(m_ptClick.y - y);

                    if ( cx > ::GetSystemMetrics(SM_CXDRAG) ||
                            cy > ::GetSystemMetrics(SM_CYDRAG) )
                    {
                        NM_TREEVIEW tv;
                        wxZeroMemory(tv);

                        tv.hdr.hwndFrom = GetHwnd();
                        tv.hdr.idFrom = ::GetWindowLong(GetHwnd(), GWL_ID);
                        tv.hdr.code = TVN_BEGINDRAG;

                        tv.itemNew.hItem = HITEM(m_htClickedItem);


                        TVITEM tviAux;
                        wxZeroMemory(tviAux);

                        tviAux.hItem = HITEM(m_htClickedItem);
                        tviAux.mask = TVIF_STATE | TVIF_PARAM;
                        tviAux.stateMask = 0xffffffff;
                        if ( TreeView_GetItem(GetHwnd(), &tviAux) )
                        {
                            tv.itemNew.state = tviAux.state;
                            tv.itemNew.lParam = tviAux.lParam;

                            tv.ptDrag.x = x;
                            tv.ptDrag.y = y;

                                                                                                                m_htClickedItem.Unset();

                            ::SendMessage(GetHwndOf(GetParent()), WM_NOTIFY,
                                          tv.hdr.idFrom, (LPARAM)&tv );

                                                                                    processed = true;
                        }
                    }
                }

#if wxUSE_DRAGIMAGE
                if ( m_dragImage )
                {
                    m_dragImage->Move(wxPoint(x, y));
                    if ( htItem )
                    {
                                                                        m_dragImage->Hide();
                        if ( !TreeView_SelectDropTarget(GetHwnd(), htItem) )
                            wxLogLastError(wxS("TreeView_SelectDropTarget()"));
                        m_dragImage->Show();
                    }
                }
#endif                 break;

            case WM_LBUTTONUP:
                if ( isMultiple )
                {
                                        if ( htItem )
                    {
                        if ( m_mouseUpDeselect )
                        {
                            m_mouseUpDeselect = false;

                            wxTreeEvent changingEvent(wxEVT_TREE_SEL_CHANGING,
                                                      this, htItem);
                            changingEvent.m_itemOld = htOldItem;

                            if ( IsTreeEventAllowed(changingEvent) )
                            {
                                DoUnselectAll();
                                DoSelectItem(wxTreeItemId(htItem));
                                SetFocusedItem(wxTreeItemId(htItem));

                                wxTreeEvent changedEvent(wxEVT_TREE_SEL_CHANGED,
                                                         this, htItem);
                                changedEvent.m_itemOld = htOldItem;
                                (void)HandleTreeEvent(changedEvent);
                            }
                        }
                    }

                    m_htClickedItem.Unset();

                    if ( m_triggerStateImageClick )
                    {
                        if ( tvht.flags & TVHT_ONITEMSTATEICON )
                        {
                            wxTreeEvent event(wxEVT_TREE_STATE_IMAGE_CLICK,
                                              this, htItem);
                            (void)HandleTreeEvent(event);

                            m_triggerStateImageClick = false;
                            processed = true;
                        }
                    }

                    if ( !m_dragStarted && MSWIsOnItem(tvht.flags) )
                    {
                        processed = true;
                    }
                }

                
            case WM_RBUTTONUP:
#if wxUSE_DRAGIMAGE
                if ( m_dragImage )
                {
                    m_dragImage->EndDrag();
                    wxDELETE(m_dragImage);

                                        wxTreeEvent event(wxEVT_TREE_END_DRAG,
                                      this, htItem);
                    event.m_pointDrag = wxPoint(x, y);
                    (void)HandleTreeEvent(event);

                                                            if ( !TreeView_SelectDropTarget(GetHwnd(), 0) )
                        wxLogLastError(wxS("TreeView_SelectDropTarget(0)"));
                }
#endif 
                if ( isMultiple && nMsg == WM_RBUTTONUP )
                {
                                        NMHDR nmhdr;
                    nmhdr.hwndFrom = GetHwnd();
                    nmhdr.idFrom = ::GetWindowLong(GetHwnd(), GWL_ID);
                    nmhdr.code = NM_RCLICK;
                    ::SendMessage(::GetParent(GetHwnd()), WM_NOTIFY,
                                  nmhdr.idFrom, (LPARAM)&nmhdr);
                    processed = true;
                }

                m_dragStarted = false;

                break;
        }
    }
    else if ( (nMsg == WM_SETFOCUS || nMsg == WM_KILLFOCUS) )
    {
        if ( isMultiple )
        {
                                                wxArrayTreeItemIds selections;
            size_t count = GetSelections(selections);
            TVGetItemRectParam param;

            for ( size_t n = 0; n < count; n++ )
            {
                                                if ( wxTreeView_GetItemRect(GetHwnd(), HITEM(selections[n]),
                                            param, TRUE) )
                {
                    ::InvalidateRect(GetHwnd(), &param.rect, FALSE);
                }
            }
        }

        if ( nMsg == WM_KILLFOCUS )
        {
            m_focusLost = true;
        }
    }
    else if ( (nMsg == WM_KEYDOWN || nMsg == WM_SYSKEYDOWN) && isMultiple )
    {
                                        switch ( wParam )
        {
            case VK_RETURN:
            case VK_SPACE:
            case VK_UP:
            case VK_DOWN:
            case VK_LEFT:
            case VK_RIGHT:
            case VK_HOME:
            case VK_END:
            case VK_PRIOR:
            case VK_NEXT:
                if ( !HandleKeyDown(wParam, lParam) &&
                        !MSWHandleTreeKeyDownEvent(wParam, lParam) )
                {
                                                            MSWHandleSelectionKey(wParam);
                }

                                                processed = true;

                                }

    }
    else if ( nMsg == WM_COMMAND )
    {
                        WORD id, cmd;
        WXHWND hwnd;
        UnpackCommand(wParam, lParam, &id, &hwnd, &cmd);

        if ( cmd == EN_KILLFOCUS )
        {
            if ( m_textCtrl && m_textCtrl->GetHandle() == hwnd )
            {
                DoEndEditLabel();

                processed = true;
            }
        }
    }

    if ( !processed )
        rc = wxControl::MSWWindowProc(nMsg, wParam, lParam);

    return rc;
}

WXLRESULT
wxTreeCtrl::MSWDefWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
    if ( nMsg == WM_CHAR )
    {
                                                if ( wParam == VK_SPACE || wParam == VK_RETURN )
            return 0;
    }
#if wxUSE_DRAGIMAGE
    else if ( nMsg == WM_KEYDOWN )
    {
        if ( wParam == VK_ESCAPE )
        {
            if ( m_dragImage )
            {
                m_dragImage->EndDrag();
                wxDELETE(m_dragImage);

                                                if ( !TreeView_SelectDropTarget(GetHwnd(), 0) )
                    wxLogLastError(wxS("TreeView_SelectDropTarget(0)"));
            }
        }
    }
#endif 
    return wxControl::MSWDefWindowProc(nMsg, wParam, lParam);
}

bool wxTreeCtrl::MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result)
{
    wxTreeEvent event(wxEVT_NULL, this);
    wxEventType eventType = wxEVT_NULL;
    NMHDR *hdr = (NMHDR *)lParam;

    switch ( hdr->code )
    {
        case TVN_BEGINDRAG:
            eventType = wxEVT_TREE_BEGIN_DRAG;
            
        case TVN_BEGINRDRAG:
            {
                if ( eventType == wxEVT_NULL )
                    eventType = wxEVT_TREE_BEGIN_RDRAG;
                
                NM_TREEVIEW *tv = (NM_TREEVIEW *)lParam;

                event.m_item = tv->itemNew.hItem;
                event.m_pointDrag = wxPoint(tv->ptDrag.x, tv->ptDrag.y);

                                                                event.Veto();
            }
            break;

        case TVN_BEGINLABELEDIT:
            {
                eventType = wxEVT_TREE_BEGIN_LABEL_EDIT;
                NMTVDISPINFO *info = (NMTVDISPINFO *)lParam;

                                                                m_idEdited =
                event.m_item = info->item.hItem;
                event.m_label = info->item.pszText;
                event.m_editCancelled = false;
            }
            break;

        case TVN_DELETEITEM:
            {
                eventType = wxEVT_TREE_DELETE_ITEM;
                NM_TREEVIEW *tv = (NM_TREEVIEW *)lParam;

                event.m_item = tv->itemOld.hItem;

                if ( m_hasAnyAttr )
                {
                    wxMapTreeAttr::iterator it = m_attrs.find(tv->itemOld.hItem);
                    if ( it != m_attrs.end() )
                    {
                        delete it->second;
                        m_attrs.erase(it);
                    }
                }
            }
            break;

        case TVN_ENDLABELEDIT:
            {
                eventType = wxEVT_TREE_END_LABEL_EDIT;
                NMTVDISPINFO *info = (NMTVDISPINFO *)lParam;

                event.m_item = info->item.hItem;
                event.m_label = info->item.pszText;
                event.m_editCancelled = info->item.pszText == NULL;
                break;
            }

                                case TTN_NEEDTEXTA:
        case TTN_NEEDTEXTW:
            {
                *result = 0;

                break;
            }

#ifdef TVN_GETINFOTIP
        case TVN_GETINFOTIP:
            {
                eventType = wxEVT_TREE_ITEM_GETTOOLTIP;
                NMTVGETINFOTIP *info = (NMTVGETINFOTIP*)lParam;

                                event.m_item = info->hItem;

                break;
            }
#endif 
        case TVN_GETDISPINFO:
            eventType = wxEVT_TREE_GET_INFO;
            
        case TVN_SETDISPINFO:
            {
                if ( eventType == wxEVT_NULL )
                    eventType = wxEVT_TREE_SET_INFO;
                
                NMTVDISPINFO *info = (NMTVDISPINFO *)lParam;

                event.m_item = info->item.hItem;
                break;
            }

        case TVN_ITEMEXPANDING:
        case TVN_ITEMEXPANDED:
            {
                NM_TREEVIEW *tv = (NM_TREEVIEW*)lParam;

                int what;
                switch ( tv->action )
                {
                    default:
                        wxLogDebug(wxT("unexpected code %d in TVN_ITEMEXPAND message"), tv->action);
                        
                    case TVE_EXPAND:
                        what = IDX_EXPAND;
                        break;

                    case TVE_COLLAPSE:
                        what = IDX_COLLAPSE;
                        break;
                }

                int how = hdr->code == TVN_ITEMEXPANDING ? IDX_DOING
                                                         : IDX_DONE;

                eventType = gs_expandEvents[what][how];

                event.m_item = tv->itemNew.hItem;
            }
            break;

        case TVN_KEYDOWN:
            {
                TV_KEYDOWN *info = (TV_KEYDOWN *)lParam;

                                                                return MSWHandleTreeKeyDownEvent(
                        info->wVKey, (wxIsAltDown() ? KF_ALTDOWN : 0) << 16);
            }


                        
                                
        case TVN_ITEMCHANGINGA:
        case TVN_ITEMCHANGINGW:
            {
                                if ( HasFlag(wxTR_MULTIPLE) )
                {
                                        NMTVITEMCHANGE* info = (NMTVITEMCHANGE*)lParam;
                    if (TreeItemUnlocker::IsLocked(info->hItem))
                    {
                                                                        *result = 1;
                        return true;
                    }
                }

                            }
            return false;

        case TVN_SELCHANGEDA:
        case TVN_SELCHANGEDW:
            if ( !m_changingSelection )
            {
                eventType = wxEVT_TREE_SEL_CHANGED;
            }
            
        case TVN_SELCHANGINGA:
        case TVN_SELCHANGINGW:
            if ( !m_changingSelection )
            {
                if ( eventType == wxEVT_NULL )
                    eventType = wxEVT_TREE_SEL_CHANGING;
                
                if (hdr->code == TVN_SELCHANGINGW ||
                    hdr->code == TVN_SELCHANGEDW)
                {
                    NM_TREEVIEWW *tv = (NM_TREEVIEWW *)lParam;
                    event.m_item = tv->itemNew.hItem;
                    event.m_itemOld = tv->itemOld.hItem;
                }
                else
                {
                    NM_TREEVIEWA *tv = (NM_TREEVIEWA *)lParam;
                    event.m_item = tv->itemNew.hItem;
                    event.m_itemOld = tv->itemOld.hItem;
                }
            }

                                                                                                                                                if ( !m_changingSelection && !m_isBeingDeleted )
            {
                                                                TempSetter set(m_changingSelection);

                SetFocus();
            }
            break;

                #if defined(CDDS_PREPAINT)
        case NM_CUSTOMDRAW:
            {
                LPNMTVCUSTOMDRAW lptvcd = (LPNMTVCUSTOMDRAW)lParam;
                NMCUSTOMDRAW& nmcd = lptvcd->nmcd;
                switch ( nmcd.dwDrawStage )
                {
                    case CDDS_PREPAINT:
                                                                        *result = m_hasAnyAttr ? CDRF_NOTIFYITEMDRAW
                                               : CDRF_DODEFAULT;

                                                                                                                                                if (m_imageListState && m_imageListState->GetImageCount() > 0)
                        {
                            const HIMAGELIST
                                hImageList = GetHimagelistOf(m_imageListState);

                                                        int width, height;
                            m_imageListState->GetSize(0, width, height);

                            HBITMAP hbmpTemp = ::CreateBitmap(width, height, 1, 1, NULL);
                            int index = ::ImageList_Add(hImageList, hbmpTemp, hbmpTemp);
                            ::DeleteObject(hbmpTemp);

                            if ( index != -1 )
                            {
                                                                for ( int i = index; i > 0; i-- )
                                {
                                    ImageList_Copy(hImageList, i,
                                                   hImageList, i-1,
                                                   ILCF_MOVE);
                                }

                                                                *result |= CDRF_NOTIFYPOSTPAINT;
                            }
                        }
                        break;

                    case CDDS_POSTPAINT:
                                                                        if (m_imageListState && m_imageListState->GetImageCount() > 0)
                            m_imageListState->Remove(0);
                        break;

                    case CDDS_ITEMPREPAINT:
                        {
                            wxMapTreeAttr::iterator
                                it = m_attrs.find((void *)nmcd.dwItemSpec);

                            if ( it == m_attrs.end() )
                            {
                                                                *result = CDRF_DODEFAULT;
                                break;
                            }

                            wxTreeItemAttr * const attr = it->second;

                            wxTreeViewItem tvItem((void *)nmcd.dwItemSpec,
                                                  TVIF_STATE, TVIS_DROPHILITED);
                            DoGetItem(&tvItem);
                            const UINT tvItemState = tvItem.state;

                                                                                    if ( !(nmcd.uItemState & CDIS_SELECTED) &&
                                 !(tvItemState & TVIS_DROPHILITED) )
                            {
                                wxColour colBack;
                                if ( attr->HasBackgroundColour() )
                                {
                                    colBack = attr->GetBackgroundColour();
                                    lptvcd->clrTextBk = wxColourToRGB(colBack);
                                }
                            }

                                                                                                                                            if ( ( !(nmcd.uItemState & CDIS_SELECTED) ||
                                    FindFocus() != this ) &&
                                 !(tvItemState & TVIS_DROPHILITED) )
                            {
                                wxColour colText;
                                if ( attr->HasTextColour() )
                                {
                                    colText = attr->GetTextColour();
                                    lptvcd->clrText = wxColourToRGB(colText);
                                }
                            }

                            if ( attr->HasFont() )
                            {
                                HFONT hFont = GetHfontOf(attr->GetFont());

                                ::SelectObject(nmcd.hdc, hFont);

                                *result = CDRF_NEWFONT;
                            }
                            else                             {
                                *result = CDRF_DODEFAULT;
                            }
                        }
                        break;

                    default:
                        *result = CDRF_DODEFAULT;
                }
            }

                        return true;
#endif 
        case NM_CLICK:
            {
                DWORD pos = GetMessagePos();
                POINT point;
                point.x = GET_X_LPARAM(pos);
                point.y = GET_Y_LPARAM(pos);
                ::MapWindowPoints(HWND_DESKTOP, GetHwnd(), &point, 1);
                int htFlags = 0;
                wxTreeItemId item = HitTest(wxPoint(point.x, point.y), htFlags);

                if ( htFlags & wxTREE_HITTEST_ONITEMSTATEICON )
                {
                    event.m_item = item;
                    eventType = wxEVT_TREE_STATE_IMAGE_CLICK;
                }

                break;
            }

        case NM_DBLCLK:
        case NM_RCLICK:
            {
                TV_HITTESTINFO tvhti;
                wxGetCursorPosMSW(&tvhti.pt);
                ::ScreenToClient(GetHwnd(), &tvhti.pt);
                if ( TreeView_HitTest(GetHwnd(), &tvhti) )
                {
                    if ( MSWIsOnItem(tvhti.flags) )
                    {
                        event.m_item = tvhti.hItem;
                                                                                                eventType = hdr->code == (UINT)NM_DBLCLK
                                    ? wxEVT_TREE_ITEM_ACTIVATED
                                    : wxEVT_TREE_ITEM_RIGHT_CLICK;

                        event.m_pointDrag.x = tvhti.pt.x;
                        event.m_pointDrag.y = tvhti.pt.y;
                    }

                    break;
                }
            }
            
        default:
            return wxControl::MSWOnNotify(idCtrl, lParam, result);
    }

    event.SetEventType(eventType);

    bool processed = HandleTreeEvent(event);

        switch ( hdr->code )
    {
        case NM_DBLCLK:
                                                                        *result = processed;
            break;

        case NM_RCLICK:
                                                *result =
            processed = true;

            ::SendMessage(GetHwnd(), WM_CONTEXTMENU,
                          (WPARAM)GetHwnd(), ::GetMessagePos());
            break;

        case TVN_BEGINDRAG:
        case TVN_BEGINRDRAG:
#if wxUSE_DRAGIMAGE
            if ( event.IsAllowed() )
            {
                                                wxASSERT_MSG( !m_dragImage, wxT("starting to drag once again?") );

                m_dragImage = new wxDragImage(*this, event.m_item);
                m_dragImage->BeginDrag(wxPoint(0,0), this);
                m_dragImage->Show();

                m_dragStarted = true;
            }
#endif             break;

        case TVN_DELETEITEM:
            {
                                                                                NM_TREEVIEW *tv = (NM_TREEVIEW *)lParam;

                wxTreeItemParam *param =
                        (wxTreeItemParam *)tv->itemOld.lParam;
                delete param;

                processed = true;             }
            break;

        case TVN_BEGINLABELEDIT:
                        *result = !event.IsAllowed();

                        if ( event.IsAllowed() )
            {
                HWND hText = TreeView_GetEditControl(GetHwnd());
                if ( hText )
                {
                                                                                                    if ( m_textCtrl && m_textCtrl->GetHWND() )
                        DeleteTextCtrl();
                    if ( !m_textCtrl )
                        m_textCtrl = new wxTextCtrl();
                    m_textCtrl->SetParent(this);
                    m_textCtrl->SetHWND((WXHWND)hText);
                    m_textCtrl->SubclassWin((WXHWND)hText);

                                                                                                    m_textCtrl->SetWindowStyle(m_textCtrl->GetWindowStyle()
                                               | wxTE_PROCESS_ENTER);
                }
            }
            else             {
                m_idEdited.Unset();
            }
            break;

        case TVN_ENDLABELEDIT:
                                                            *result = event.IsAllowed();
            processed = true;

                                    DeleteTextCtrl();
            break;

#ifdef TVN_GETINFOTIP
         case TVN_GETINFOTIP:
            {
                                if (event.IsAllowed())
                {
                    SetToolTip(event.m_label);
                }
            }
            break;
#endif

        case TVN_SELCHANGING:
        case TVN_ITEMEXPANDING:
                        *result = !event.IsAllowed();
            break;

        case TVN_ITEMEXPANDED:
            {
                NM_TREEVIEW *tv = (NM_TREEVIEW *)lParam;
                const wxTreeItemId id(tv->itemNew.hItem);

                if ( tv->action == TVE_COLLAPSE )
                {
                    if ( wxApp::GetComCtl32Version() >= 600 )
                    {
                                                                                                                                                                                                                                                                                                                        RefreshItem(id);
                    }
                }
                else                 {
                                                                                                    int image = GetItemImage(id, wxTreeItemIcon_Expanded);
                    if ( image != -1 )
                    {
                        RefreshItem(id);
                    }
                }
            }
            break;

        case TVN_GETDISPINFO:
                                                {
                wxTreeItemId item = event.m_item;
                NMTVDISPINFO *info = (NMTVDISPINFO *)lParam;

                const wxTreeItemParam * const param = GetItemParam(item);
                if ( !param )
                    break;

                if ( info->item.mask & TVIF_IMAGE )
                {
                    info->item.iImage =
                        param->GetImage
                        (
                         IsExpanded(item) ? wxTreeItemIcon_Expanded
                                          : wxTreeItemIcon_Normal
                        );
                }
                if ( info->item.mask & TVIF_SELECTEDIMAGE )
                {
                    info->item.iSelectedImage =
                        param->GetImage
                        (
                         IsExpanded(item) ? wxTreeItemIcon_SelectedExpanded
                                          : wxTreeItemIcon_Selected
                        );
                }
            }
            break;

                                    }
    return processed;
}


#define STATEIMAGEMASKTOINDEX(state) (((state) & TVIS_STATEIMAGEMASK) >> 12)

int wxTreeCtrl::DoGetItemState(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTREE_ITEMSTATE_NONE, wxT("invalid tree item") );

        wxTreeViewItem tvItem(item, TVIF_STATE, TVIS_STATEIMAGEMASK);
    DoGetItem(&tvItem);

        return STATEIMAGEMASKTOINDEX(tvItem.state) - 1;
}

void wxTreeCtrl::DoSetItemState(const wxTreeItemId& item, int state)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    wxTreeViewItem tvItem(item, TVIF_STATE, TVIS_STATEIMAGEMASK);

            tvItem.state = INDEXTOSTATEIMAGEMASK(state + 1);

    DoSetItem(&tvItem);
}



void wxTreeCtrl::DoFreeze()
{
    if ( IsShown() )
    {
        RECT rc;
        ::GetWindowRect(GetHwnd(), &rc);
        m_thawnSize = wxRectFromRECT(rc).GetSize();

        ::SetWindowPos(GetHwnd(), 0, 0, 0, 1, 1,
                       SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
    }
}

void wxTreeCtrl::DoThaw()
{
    if ( IsShown() )
    {
        if ( m_thawnSize != wxDefaultSize )
        {
            ::SetWindowPos(GetHwnd(), 0, 0, 0, m_thawnSize.x, m_thawnSize.y,
                           SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }
}

void wxTreeCtrl::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
    m_thawnSize = wxDefaultSize;

    wxTreeCtrlBase::DoSetSize(x, y, width, height, sizeFlags);
}

#endif 