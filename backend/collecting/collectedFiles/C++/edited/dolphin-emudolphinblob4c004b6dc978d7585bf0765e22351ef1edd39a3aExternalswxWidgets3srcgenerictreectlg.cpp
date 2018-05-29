


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_TREECTRL

#include "wx/treectrl.h"

#ifndef WX_PRECOMP
    #include "wx/dcclient.h"
    #include "wx/timer.h"
    #include "wx/settings.h"
    #include "wx/listbox.h"
    #include "wx/textctrl.h"
#endif

#include "wx/generic/treectlg.h"
#include "wx/imaglist.h"

#include "wx/renderer.h"

#ifdef __WXMAC__
    #include "wx/osx/private.h"
#endif


class WXDLLIMPEXP_FWD_CORE wxGenericTreeItem;

WX_DEFINE_ARRAY_PTR(wxGenericTreeItem *, wxArrayGenericTreeItems);


static const int NO_IMAGE = -1;

static const int PIXELS_PER_UNIT = 10;

static const int MARGIN_BETWEEN_STATE_AND_IMAGE = 2;

static const int MARGIN_BETWEEN_IMAGE_AND_TEXT = 4;


class WXDLLEXPORT wxTreeRenameTimer: public wxTimer
{
public:
            enum { DELAY = 500 };

    wxTreeRenameTimer( wxGenericTreeCtrl *owner );

    virtual void Notify() wxOVERRIDE;

private:
    wxGenericTreeCtrl *m_owner;

    wxDECLARE_NO_COPY_CLASS(wxTreeRenameTimer);
};

class WXDLLEXPORT wxTreeTextCtrl: public wxTextCtrl
{
public:
    wxTreeTextCtrl(wxGenericTreeCtrl *owner, wxGenericTreeItem *item);

    void EndEdit( bool discardChanges );

    const wxGenericTreeItem* item() const { return m_itemEdited; }

protected:
    void OnChar( wxKeyEvent &event );
    void OnKeyUp( wxKeyEvent &event );
    void OnKillFocus( wxFocusEvent &event );

    bool AcceptChanges();
    void Finish( bool setfocus );

private:
    wxGenericTreeCtrl  *m_owner;
    wxGenericTreeItem  *m_itemEdited;
    wxString            m_startValue;
    bool                m_aboutToFinish;

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(wxTreeTextCtrl);
};

class WXDLLEXPORT wxTreeFindTimer : public wxTimer
{
public:
        enum { DELAY = 500 };

    wxTreeFindTimer( wxGenericTreeCtrl *owner ) { m_owner = owner; }

    virtual void Notify() wxOVERRIDE { m_owner->ResetFindState(); }

private:
    wxGenericTreeCtrl *m_owner;

    wxDECLARE_NO_COPY_CLASS(wxTreeFindTimer);
};

class WXDLLEXPORT wxGenericTreeItem
{
public:
        wxGenericTreeItem()
    {
        m_data = NULL;
        m_widthText =
        m_heightText = -1;
    }

    wxGenericTreeItem( wxGenericTreeItem *parent,
                       const wxString& text,
                       int image,
                       int selImage,
                       wxTreeItemData *data );

    ~wxGenericTreeItem();

        wxArrayGenericTreeItems& GetChildren() { return m_children; }

    const wxString& GetText() const { return m_text; }
    int GetImage(wxTreeItemIcon which = wxTreeItemIcon_Normal) const
        { return m_images[which]; }
    wxTreeItemData *GetData() const { return m_data; }
    int GetState() const { return m_state; }

            int GetCurrentImage() const;

    void SetText(const wxString& text)
    {
        m_text = text;

        ResetTextSize();
    }

    void SetImage(int image, wxTreeItemIcon which)
    {
        m_images[which] = image;
        m_width = 0;
    }

    void SetData(wxTreeItemData *data) { m_data = data; }
    void SetState(int state) { m_state = state; m_width = 0; }

    void SetHasPlus(bool has = true) { m_hasPlus = has; }

    void SetBold(bool bold)
    {
        m_isBold = bold;

        ResetTextSize();
    }

    int GetX() const { return m_x; }
    int GetY() const { return m_y; }

    void SetX(int x) { m_x = x; }
    void SetY(int y) { m_y = y; }

    int GetHeight() const { return m_height; }
    int GetWidth() const { return m_width; }

    int GetTextHeight() const
    {
        wxASSERT_MSG( m_heightText != -1, "must call CalculateSize() first" );

        return m_heightText;
    }

    int GetTextWidth() const
    {
        wxASSERT_MSG( m_widthText != -1, "must call CalculateSize() first" );

        return m_widthText;
    }

    wxGenericTreeItem *GetParent() const { return m_parent; }

            bool SetFont(wxGenericTreeCtrl *control, wxDC& dc) const
    {
        wxFont font;

        wxTreeItemAttr * const attr = GetAttributes();
        if ( attr && attr->HasFont() )
            font = attr->GetFont();
        else if ( IsBold() )
            font = control->m_boldFont;
        else
            return false;

        dc.SetFont(font);
        return true;
    }

    
        void DeleteChildren(wxGenericTreeCtrl *tree);

        size_t GetChildrenCount(bool recursively = true) const;

    void Insert(wxGenericTreeItem *child, size_t index)
        { m_children.Insert(child, index); }

                void CalculateSize(wxGenericTreeCtrl *control, wxDC& dc)
        { DoCalculateSize(control, dc, true ); }
    void CalculateSize(wxGenericTreeCtrl *control);

    void GetSize( int &x, int &y, const wxGenericTreeCtrl* );

    void ResetSize() { m_width = 0; }
    void ResetTextSize() { m_width = 0; m_widthText = -1; }
    void RecursiveResetSize();
    void RecursiveResetTextSize();

                            wxGenericTreeItem *HitTest( const wxPoint& point,
                                const wxGenericTreeCtrl *,
                                int &flags,
                                int level );

    void Expand() { m_isCollapsed = false; }
    void Collapse() { m_isCollapsed = true; }

    void SetHilight( bool set = true ) { m_hasHilight = set; }

        bool HasChildren() const { return !m_children.IsEmpty(); }
    bool IsSelected()  const { return m_hasHilight != 0; }
    bool IsExpanded()  const { return !m_isCollapsed; }
    bool HasPlus()     const { return m_hasPlus || HasChildren(); }
    bool IsBold()      const { return m_isBold != 0; }

                wxTreeItemAttr *GetAttributes() const { return m_attr; }
            wxTreeItemAttr& Attr()
    {
        if ( !m_attr )
        {
            m_attr = new wxTreeItemAttr;
            m_ownsAttr = true;
        }
        return *m_attr;
    }
            void SetAttributes(wxTreeItemAttr *attr)
    {
        if ( m_ownsAttr ) delete m_attr;
        m_attr = attr;
        m_ownsAttr = false;
        m_width = 0;
        m_widthText = -1;
    }
            void AssignAttributes(wxTreeItemAttr *attr)
    {
        SetAttributes(attr);
        m_ownsAttr = true;
        m_width = 0;
        m_widthText = -1;
    }

private:
                        void DoCalculateSize(wxGenericTreeCtrl *control,
                         wxDC& dc,
                         bool dcUsesNormalFont);

                wxString            m_text;             int                 m_widthText;
    int                 m_heightText;

    wxTreeItemData     *m_data;         
    int                 m_state;        
    wxArrayGenericTreeItems m_children;     wxGenericTreeItem  *m_parent;       
    wxTreeItemAttr     *m_attr;         
            int                 m_images[wxTreeItemIcon_Max];

    wxCoord             m_x;                wxCoord             m_y;                int                 m_width;            int                 m_height;       
        unsigned int        m_isCollapsed :1;
    unsigned int        m_hasHilight  :1;     unsigned int        m_hasPlus     :1;                                               unsigned int        m_isBold      :1;     unsigned int        m_ownsAttr    :1; 
    wxDECLARE_NO_COPY_CLASS(wxGenericTreeItem);
};



static void EventFlagsToSelType(long style,
                                bool shiftDown,
                                bool ctrlDown,
                                bool &is_multiple,
                                bool &extended_select,
                                bool &unselect_others)
{
    is_multiple = (style & wxTR_MULTIPLE) != 0;
    extended_select = shiftDown && is_multiple;
    unselect_others = !(extended_select || (ctrlDown && is_multiple));
}

static bool
IsDescendantOf(const wxGenericTreeItem *parent, const wxGenericTreeItem *item)
{
    while ( item )
    {
        if ( item == parent )
        {
                        return true;
        }

        item = item->GetParent();
    }

    return false;
}


wxTreeRenameTimer::wxTreeRenameTimer( wxGenericTreeCtrl *owner )
{
    m_owner = owner;
}

void wxTreeRenameTimer::Notify()
{
    m_owner->OnRenameTimer();
}


wxBEGIN_EVENT_TABLE(wxTreeTextCtrl,wxTextCtrl)
    EVT_CHAR           (wxTreeTextCtrl::OnChar)
    EVT_KEY_UP         (wxTreeTextCtrl::OnKeyUp)
    EVT_KILL_FOCUS     (wxTreeTextCtrl::OnKillFocus)
wxEND_EVENT_TABLE()

wxTreeTextCtrl::wxTreeTextCtrl(wxGenericTreeCtrl *owner,
                               wxGenericTreeItem *itm)
              : m_itemEdited(itm), m_startValue(itm->GetText())
{
    m_owner = owner;
    m_aboutToFinish = false;

    wxRect rect;
    m_owner->GetBoundingRect(m_itemEdited, rect, true);

    #ifdef __WXMSW__
    rect.x -= 5;
    rect.width += 10;
#elif defined(__WXGTK__)
    rect.x -= 5;
    rect.y -= 2;
    rect.width  += 8;
    rect.height += 4;
#endif 
    (void)Create(m_owner, wxID_ANY, m_startValue,
                 rect.GetPosition(), rect.GetSize());

    SelectAll();
}

void wxTreeTextCtrl::EndEdit(bool discardChanges)
{
    if ( m_aboutToFinish )
    {
                        return;
    }

    m_aboutToFinish = true;

    if ( discardChanges )
    {
        m_owner->OnRenameCancelled(m_itemEdited);

        Finish( true );
    }
    else
    {
                AcceptChanges();

                Finish( true );
    }
}

bool wxTreeTextCtrl::AcceptChanges()
{
    const wxString value = GetValue();

    if ( value == m_startValue )
    {
                                        
        m_owner->OnRenameCancelled(m_itemEdited);
        return true;
    }

    if ( !m_owner->OnRenameAccept(m_itemEdited, value) )
    {
                return false;
    }

        m_owner->SetItemText(m_itemEdited, value);

    return true;
}

void wxTreeTextCtrl::Finish( bool setfocus )
{
    m_owner->ResetTextControl();

#ifdef __WXMAC__
                Hide();
#endif

    wxPendingDelete.Append(this);

    if (setfocus)
        m_owner->SetFocus();
}

void wxTreeTextCtrl::OnChar( wxKeyEvent &event )
{
    switch ( event.m_keyCode )
    {
        case WXK_RETURN:
            EndEdit( false );
            break;

        case WXK_ESCAPE:
            EndEdit( true );
            break;

        default:
            event.Skip();
    }
}

void wxTreeTextCtrl::OnKeyUp( wxKeyEvent &event )
{
    if ( !m_aboutToFinish )
    {
                wxSize parentSize = m_owner->GetSize();
        wxPoint myPos = GetPosition();
        wxSize mySize = GetSize();
        int sx, sy;
        GetTextExtent(GetValue() + wxT("M"), &sx, &sy);
        if (myPos.x + sx > parentSize.x)
            sx = parentSize.x - myPos.x;
        if (mySize.x > sx)
            sx = mySize.x;
        SetSize(sx, wxDefaultCoord);
    }

    event.Skip();
}

void wxTreeTextCtrl::OnKillFocus( wxFocusEvent &event )
{
    if ( !m_aboutToFinish )
    {
        m_aboutToFinish = true;
        if ( !AcceptChanges() )
            m_owner->OnRenameCancelled( m_itemEdited );

        Finish( false );
    }

        event.Skip();
}


wxGenericTreeItem::wxGenericTreeItem(wxGenericTreeItem *parent,
                                     const wxString& text,
                                     int image, int selImage,
                                     wxTreeItemData *data)
                 : m_text(text)
{
    m_images[wxTreeItemIcon_Normal] = image;
    m_images[wxTreeItemIcon_Selected] = selImage;
    m_images[wxTreeItemIcon_Expanded] = NO_IMAGE;
    m_images[wxTreeItemIcon_SelectedExpanded] = NO_IMAGE;

    m_data = data;
    m_state = wxTREE_ITEMSTATE_NONE;
    m_x = m_y = 0;

    m_isCollapsed = true;
    m_hasHilight = false;
    m_hasPlus = false;
    m_isBold = false;

    m_parent = parent;

    m_attr = NULL;
    m_ownsAttr = false;

        m_width = 0;
    m_height = 0;

    m_widthText = -1;
    m_heightText = -1;
}

wxGenericTreeItem::~wxGenericTreeItem()
{
    delete m_data;

    if (m_ownsAttr) delete m_attr;

    wxASSERT_MSG( m_children.IsEmpty(),
                  "must call DeleteChildren() before deleting the item" );
}

void wxGenericTreeItem::DeleteChildren(wxGenericTreeCtrl *tree)
{
    size_t count = m_children.GetCount();
    for ( size_t n = 0; n < count; n++ )
    {
        wxGenericTreeItem *child = m_children[n];
        tree->SendDeleteEvent(child);

        child->DeleteChildren(tree);
        if ( child == tree->m_select_me )
            tree->m_select_me = NULL;
        delete child;
    }

    m_children.Empty();
}

size_t wxGenericTreeItem::GetChildrenCount(bool recursively) const
{
    size_t count = m_children.GetCount();
    if ( !recursively )
        return count;

    size_t total = count;
    for (size_t n = 0; n < count; ++n)
    {
        total += m_children[n]->GetChildrenCount();
    }

    return total;
}

void wxGenericTreeItem::GetSize( int &x, int &y,
                                 const wxGenericTreeCtrl *theButton )
{
    int bottomY=m_y+theButton->GetLineHeight(this);
    if ( y < bottomY ) y = bottomY;
    int width = m_x +  m_width;
    if ( x < width ) x = width;

    if (IsExpanded())
    {
        size_t count = m_children.GetCount();
        for ( size_t n = 0; n < count; ++n )
        {
            m_children[n]->GetSize( x, y, theButton );
        }
    }
}

wxGenericTreeItem *wxGenericTreeItem::HitTest(const wxPoint& point,
                                              const wxGenericTreeCtrl *theCtrl,
                                              int &flags,
                                              int level)
{
        if ( !(level == 0 && theCtrl->HasFlag(wxTR_HIDE_ROOT)) )
    {
                int h = theCtrl->GetLineHeight(this);
        if ((point.y > m_y) && (point.y < m_y + h))
        {
            int y_mid = m_y + h/2;
            if (point.y < y_mid )
                flags |= wxTREE_HITTEST_ONITEMUPPERPART;
            else
                flags |= wxTREE_HITTEST_ONITEMLOWERPART;

            int xCross = m_x - theCtrl->GetSpacing();
#ifdef __WXMAC__
                                    if ((point.x > xCross-4) && (point.x < xCross+10) &&
                (point.y > y_mid-4) && (point.y < y_mid+10) &&
                HasPlus() && theCtrl->HasButtons() )
#else
                        if ((point.x > xCross-6) && (point.x < xCross+6) &&
                (point.y > y_mid-6) && (point.y < y_mid+6) &&
                HasPlus() && theCtrl->HasButtons() )
#endif
            {
                flags |= wxTREE_HITTEST_ONITEMBUTTON;
                return this;
            }

            if ((point.x >= m_x) && (point.x <= m_x+m_width))
            {
                int image_w = -1;
                int image_h;

                                if ( (GetImage() != NO_IMAGE) && theCtrl->m_imageListNormal )
                {
                    theCtrl->m_imageListNormal->GetSize(GetImage(),
                                                        image_w, image_h);
                }

                int state_w = -1;
                int state_h;

                if ( (GetState() != wxTREE_ITEMSTATE_NONE) &&
                        theCtrl->m_imageListState )
                {
                    theCtrl->m_imageListState->GetSize(GetState(),
                                                       state_w, state_h);
                }

                if ((state_w != -1) && (point.x <= m_x + state_w + 1))
                    flags |= wxTREE_HITTEST_ONITEMSTATEICON;
                else if ((image_w != -1) &&
                         (point.x <= m_x +
                            (state_w != -1 ? state_w +
                                                MARGIN_BETWEEN_STATE_AND_IMAGE
                                           : 0)
                                            + image_w + 1))
                    flags |= wxTREE_HITTEST_ONITEMICON;
                else
                    flags |= wxTREE_HITTEST_ONITEMLABEL;

                return this;
            }

            if (point.x < m_x)
                flags |= wxTREE_HITTEST_ONITEMINDENT;
            if (point.x > m_x+m_width)
                flags |= wxTREE_HITTEST_ONITEMRIGHT;

            return this;
        }

                if (m_isCollapsed) return NULL;
    }

        size_t count = m_children.GetCount();
    for ( size_t n = 0; n < count; n++ )
    {
        wxGenericTreeItem *res = m_children[n]->HitTest( point,
                                                         theCtrl,
                                                         flags,
                                                         level + 1 );
        if ( res != NULL )
            return res;
    }

    return NULL;
}

int wxGenericTreeItem::GetCurrentImage() const
{
    int image = NO_IMAGE;
    if ( IsExpanded() )
    {
        if ( IsSelected() )
        {
            image = GetImage(wxTreeItemIcon_SelectedExpanded);
        }

        if ( image == NO_IMAGE )
        {
                                    image = GetImage(wxTreeItemIcon_Expanded);
        }
    }
    else     {
        if ( IsSelected() )
            image = GetImage(wxTreeItemIcon_Selected);
    }

            if ( image == NO_IMAGE ) image = GetImage();

    return image;
}

void wxGenericTreeItem::CalculateSize(wxGenericTreeCtrl* control)
{
        if ( m_width != 0 )
        return;

    wxClientDC dc(control);
    DoCalculateSize(control, dc, false );
}

void
wxGenericTreeItem::DoCalculateSize(wxGenericTreeCtrl* control,
                                   wxDC& dc,
                                   bool dcUsesNormalFont)
{
    if ( m_width != 0 )         return;

    if ( m_widthText == -1 )
    {
        bool fontChanged;
        if ( SetFont(control, dc) )
        {
            fontChanged = true;
        }
        else         {
           if ( !dcUsesNormalFont )
           {
                                                            dc.SetFont(control->m_normalFont);
           }

           fontChanged = false;
        }

        dc.GetTextExtent( GetText(), &m_widthText, &m_heightText );

                if ( fontChanged )
             dc.SetFont(control->m_normalFont);
    }

    int text_h = m_heightText + 2;

    int image_h = 0, image_w = 0;
    int image = GetCurrentImage();
    if ( image != NO_IMAGE && control->m_imageListNormal )
    {
        control->m_imageListNormal->GetSize(image, image_w, image_h);
        image_w += MARGIN_BETWEEN_IMAGE_AND_TEXT;
    }

    int state_h = 0, state_w = 0;
    int state = GetState();
    if ( state != wxTREE_ITEMSTATE_NONE && control->m_imageListState )
    {
        control->m_imageListState->GetSize(state, state_w, state_h);
        if ( image_w != 0 )
            state_w += MARGIN_BETWEEN_STATE_AND_IMAGE;
        else
            state_w += MARGIN_BETWEEN_IMAGE_AND_TEXT;
    }

    int img_h = wxMax(state_h, image_h);
    m_height = wxMax(img_h, text_h);

    if (m_height < 30)
        m_height += 2;                else
        m_height += m_height / 10;   
    if (m_height > control->m_lineHeight)
        control->m_lineHeight = m_height;

    m_width = state_w + image_w + m_widthText + 2;
}

void wxGenericTreeItem::RecursiveResetSize()
{
    m_width = 0;

    const size_t count = m_children.Count();
    for (size_t i = 0; i < count; i++ )
        m_children[i]->RecursiveResetSize();
}

void wxGenericTreeItem::RecursiveResetTextSize()
{
    m_width = 0;
    m_widthText = -1;

    const size_t count = m_children.Count();
    for (size_t i = 0; i < count; i++ )
        m_children[i]->RecursiveResetTextSize();
}


wxIMPLEMENT_DYNAMIC_CLASS(wxGenericTreeCtrl, wxControl);

wxBEGIN_EVENT_TABLE(wxGenericTreeCtrl, wxTreeCtrlBase)
    EVT_PAINT          (wxGenericTreeCtrl::OnPaint)
    EVT_SIZE           (wxGenericTreeCtrl::OnSize)
    EVT_MOUSE_EVENTS   (wxGenericTreeCtrl::OnMouse)
    EVT_KEY_DOWN       (wxGenericTreeCtrl::OnKeyDown)
    EVT_CHAR           (wxGenericTreeCtrl::OnChar)
    EVT_SET_FOCUS      (wxGenericTreeCtrl::OnSetFocus)
    EVT_KILL_FOCUS     (wxGenericTreeCtrl::OnKillFocus)
    EVT_TREE_ITEM_GETTOOLTIP(wxID_ANY, wxGenericTreeCtrl::OnGetToolTip)
wxEND_EVENT_TABLE()


void wxGenericTreeCtrl::Init()
{
    m_current =
    m_key_current =
    m_anchor =
    m_select_me = NULL;
    m_hasFocus = false;
    m_dirty = false;

    m_lineHeight = 10;
    m_indent = 15;
    m_spacing = 18;

    m_hilightBrush = new wxBrush
                         (
                            wxSystemSettings::GetColour
                            (
                                wxSYS_COLOUR_HIGHLIGHT
                            ),
                            wxBRUSHSTYLE_SOLID
                         );

    m_hilightUnfocusedBrush = new wxBrush
                              (
                                 wxSystemSettings::GetColour
                                 (
                                     wxSYS_COLOUR_BTNSHADOW
                                 ),
                                 wxBRUSHSTYLE_SOLID
                              );

    m_imageListButtons = NULL;
    m_ownsImageListButtons = false;

    m_dragCount = 0;
    m_isDragging = false;
    m_dropTarget = m_oldSelection = NULL;
    m_underMouse = NULL;
    m_textCtrl = NULL;

    m_renameTimer = NULL;

    m_findTimer = NULL;
    m_findBell = 0;  
    m_dropEffectAboveItem = false;

    m_dndEffect = NoEffect;
    m_dndEffectItem = NULL;

    m_lastOnSame = false;

#if defined( __WXMAC__ )
    m_normalFont = wxFont(wxOSX_SYSTEM_FONT_VIEWS);
#else
    m_normalFont = wxSystemSettings::GetFont( wxSYS_DEFAULT_GUI_FONT );
#endif
    m_boldFont = m_normalFont.Bold();
}

bool wxGenericTreeCtrl::Create(wxWindow *parent,
                               wxWindowID id,
                               const wxPoint& pos,
                               const wxSize& size,
                               long style,
                               const wxValidator& validator,
                               const wxString& name )
{
#ifdef __WXMAC__
    if (style & wxTR_HAS_BUTTONS)
        style |= wxTR_NO_LINES;
#endif 
#ifdef __WXGTK20__
    if (style & wxTR_HAS_BUTTONS)
        style |= wxTR_NO_LINES;
#endif

    if ( !wxControl::Create( parent, id, pos, size,
                             style|wxHSCROLL|wxVSCROLL|wxWANTS_CHARS,
                             validator,
                             name ) )
        return false;

                if (!HasButtons() && !HasFlag(wxTR_NO_LINES))
    {
        m_indent= 10;
        m_spacing = 10;
    }

    wxVisualAttributes attr = GetDefaultAttributes();
    SetOwnForegroundColour( attr.colFg );
    SetOwnBackgroundColour( attr.colBg );
    if (!m_hasFont)
        SetOwnFont(attr.font);

            #ifdef __WXMSW__
    m_dottedPen = wxPen(*wxLIGHT_GREY, 0, wxPENSTYLE_DOT);
#else
    m_dottedPen = *wxGREY_PEN;
#endif

    SetInitialSize(size);

    return true;
}

wxGenericTreeCtrl::~wxGenericTreeCtrl()
{
    delete m_hilightBrush;
    delete m_hilightUnfocusedBrush;

    DeleteAllItems();

    delete m_renameTimer;
    delete m_findTimer;

    if (m_ownsImageListButtons)
        delete m_imageListButtons;
}

void wxGenericTreeCtrl::EnableBellOnNoMatch( bool on )
{
    m_findBell = on;
}


unsigned int wxGenericTreeCtrl::GetCount() const
{
    if ( !m_anchor )
    {
                return 0;
    }

    unsigned int count = m_anchor->GetChildrenCount();
    if ( !HasFlag(wxTR_HIDE_ROOT) )
    {
                count++;
    }

    return count;
}

void wxGenericTreeCtrl::SetIndent(unsigned int indent)
{
    m_indent = (unsigned short) indent;
    m_dirty = true;
}

size_t
wxGenericTreeCtrl::GetChildrenCount(const wxTreeItemId& item,
                                    bool recursively) const
{
    wxCHECK_MSG( item.IsOk(), 0u, wxT("invalid tree item") );

    return ((wxGenericTreeItem*) item.m_pItem)->GetChildrenCount(recursively);
}

void wxGenericTreeCtrl::SetWindowStyleFlag(long styles)
{
        if (m_anchor && !HasFlag(wxTR_HIDE_ROOT) && (styles & wxTR_HIDE_ROOT))
    {
                m_anchor->SetHasPlus();
        m_anchor->Expand();
        CalculatePositions();
    }

                m_windowStyle = styles;
    m_dirty = true;
}


wxString wxGenericTreeCtrl::GetItemText(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxEmptyString, wxT("invalid tree item") );

    return ((wxGenericTreeItem*) item.m_pItem)->GetText();
}

int wxGenericTreeCtrl::GetItemImage(const wxTreeItemId& item,
                             wxTreeItemIcon which) const
{
    wxCHECK_MSG( item.IsOk(), -1, wxT("invalid tree item") );

    return ((wxGenericTreeItem*) item.m_pItem)->GetImage(which);
}

wxTreeItemData *wxGenericTreeCtrl::GetItemData(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), NULL, wxT("invalid tree item") );

    return ((wxGenericTreeItem*) item.m_pItem)->GetData();
}

int wxGenericTreeCtrl::DoGetItemState(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTREE_ITEMSTATE_NONE, wxT("invalid tree item") );

    wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    return pItem->GetState();
}

wxColour wxGenericTreeCtrl::GetItemTextColour(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxNullColour, wxT("invalid tree item") );

    wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    return pItem->Attr().GetTextColour();
}

wxColour
wxGenericTreeCtrl::GetItemBackgroundColour(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxNullColour, wxT("invalid tree item") );

    wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    return pItem->Attr().GetBackgroundColour();
}

wxFont wxGenericTreeCtrl::GetItemFont(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxNullFont, wxT("invalid tree item") );

    wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    return pItem->Attr().GetFont();
}

void
wxGenericTreeCtrl::SetItemText(const wxTreeItemId& item, const wxString& text)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    pItem->SetText(text);
    pItem->CalculateSize(this);
    RefreshLine(pItem);
}

void wxGenericTreeCtrl::SetItemImage(const wxTreeItemId& item,
                              int image,
                              wxTreeItemIcon which)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    pItem->SetImage(image, which);
    pItem->CalculateSize(this);
    RefreshLine(pItem);
}

void
wxGenericTreeCtrl::SetItemData(const wxTreeItemId& item, wxTreeItemData *data)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    if (data)
        data->SetId( item );

    ((wxGenericTreeItem*) item.m_pItem)->SetData(data);
}

void wxGenericTreeCtrl::DoSetItemState(const wxTreeItemId& item, int state)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    pItem->SetState(state);
    pItem->CalculateSize(this);
    RefreshLine(pItem);
}

void wxGenericTreeCtrl::SetItemHasChildren(const wxTreeItemId& item, bool has)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    pItem->SetHasPlus(has);
    RefreshLine(pItem);
}

void wxGenericTreeCtrl::SetItemBold(const wxTreeItemId& item, bool bold)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

        wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    if ( pItem->IsBold() != bold )
    {
        pItem->SetBold(bold);

                        pItem->CalculateSize(this);
        RefreshLine(pItem);
    }
}

void wxGenericTreeCtrl::SetItemDropHighlight(const wxTreeItemId& item,
                                             bool highlight)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    wxColour fg, bg;

    if (highlight)
    {
        bg = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
        fg = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
    }

    wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    pItem->Attr().SetTextColour(fg);
    pItem->Attr().SetBackgroundColour(bg);
    RefreshLine(pItem);
}

void wxGenericTreeCtrl::SetItemTextColour(const wxTreeItemId& item,
                                   const wxColour& col)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    pItem->Attr().SetTextColour(col);
    RefreshLine(pItem);
}

void wxGenericTreeCtrl::SetItemBackgroundColour(const wxTreeItemId& item,
                                         const wxColour& col)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    pItem->Attr().SetBackgroundColour(col);
    RefreshLine(pItem);
}

void
wxGenericTreeCtrl::SetItemFont(const wxTreeItemId& item, const wxFont& font)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    pItem->Attr().SetFont(font);
    pItem->ResetTextSize();
    pItem->CalculateSize(this);
    RefreshLine(pItem);
}

bool wxGenericTreeCtrl::SetFont( const wxFont &font )
{
    wxTreeCtrlBase::SetFont(font);

    m_normalFont = font;
    m_boldFont = m_normalFont.Bold();

    if (m_anchor)
        m_anchor->RecursiveResetTextSize();

    return true;
}



bool wxGenericTreeCtrl::IsVisible(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), false, wxT("invalid tree item") );

        if ( item == GetRootItem() && HasFlag(wxTR_HIDE_ROOT) )
        return false;

        wxGenericTreeItem *pItem = (wxGenericTreeItem*) item.m_pItem;
    wxGenericTreeItem* parent = pItem->GetParent();
    while (parent)
    {
        if (!parent->IsExpanded())
            return false;
        parent = parent->GetParent();
    }

    int startX, startY;
    GetViewStart(& startX, & startY);

    wxSize clientSize = GetClientSize();

    wxRect rect;
    if (!GetBoundingRect(item, rect))
        return false;
    if (rect.GetWidth() == 0 || rect.GetHeight() == 0)
        return false;
    if (rect.GetBottom() < 0 || rect.GetTop() > clientSize.y)
        return false;
    if (rect.GetRight() < 0 || rect.GetLeft() > clientSize.x)
        return false;

    return true;
}

bool wxGenericTreeCtrl::ItemHasChildren(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), false, wxT("invalid tree item") );

                        return ((wxGenericTreeItem*) item.m_pItem)->HasPlus();
}

bool wxGenericTreeCtrl::IsExpanded(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), false, wxT("invalid tree item") );

    return ((wxGenericTreeItem*) item.m_pItem)->IsExpanded();
}

bool wxGenericTreeCtrl::IsSelected(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), false, wxT("invalid tree item") );

    return ((wxGenericTreeItem*) item.m_pItem)->IsSelected();
}

bool wxGenericTreeCtrl::IsBold(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), false, wxT("invalid tree item") );

    return ((wxGenericTreeItem*) item.m_pItem)->IsBold();
}


wxTreeItemId wxGenericTreeCtrl::GetItemParent(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );

    return ((wxGenericTreeItem*) item.m_pItem)->GetParent();
}

wxTreeItemId wxGenericTreeCtrl::GetFirstChild(const wxTreeItemId& item,
                                              wxTreeItemIdValue& cookie) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );

    cookie = 0;
    return GetNextChild(item, cookie);
}

wxTreeItemId wxGenericTreeCtrl::GetNextChild(const wxTreeItemId& item,
                                             wxTreeItemIdValue& cookie) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );

    wxArrayGenericTreeItems&
        children = ((wxGenericTreeItem*) item.m_pItem)->GetChildren();

            size_t *pIndex = (size_t *)&cookie;
    if ( *pIndex < children.GetCount() )
    {
        return children.Item((*pIndex)++);
    }
    else
    {
                return wxTreeItemId();
    }
}

wxTreeItemId wxGenericTreeCtrl::GetLastChild(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );

    wxArrayGenericTreeItems&
        children = ((wxGenericTreeItem*) item.m_pItem)->GetChildren();
    return children.IsEmpty() ? wxTreeItemId() : wxTreeItemId(children.Last());
}

wxTreeItemId wxGenericTreeCtrl::GetNextSibling(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );

    wxGenericTreeItem *i = (wxGenericTreeItem*) item.m_pItem;
    wxGenericTreeItem *parent = i->GetParent();
    if ( parent == NULL )
    {
                return wxTreeItemId();
    }

    wxArrayGenericTreeItems& siblings = parent->GetChildren();
    int index = siblings.Index(i);
    wxASSERT( index != wxNOT_FOUND ); 
    size_t n = (size_t)(index + 1);
    return n == siblings.GetCount() ? wxTreeItemId()
                                    : wxTreeItemId(siblings[n]);
}

wxTreeItemId wxGenericTreeCtrl::GetPrevSibling(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );

    wxGenericTreeItem *i = (wxGenericTreeItem*) item.m_pItem;
    wxGenericTreeItem *parent = i->GetParent();
    if ( parent == NULL )
    {
                return wxTreeItemId();
    }

    wxArrayGenericTreeItems& siblings = parent->GetChildren();
    int index = siblings.Index(i);
    wxASSERT( index != wxNOT_FOUND ); 
    return index == 0 ? wxTreeItemId()
                      : wxTreeItemId(siblings[(size_t)(index - 1)]);
}

wxTreeItemId wxGenericTreeCtrl::GetNext(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );

    wxGenericTreeItem *i = (wxGenericTreeItem*) item.m_pItem;

        wxArrayGenericTreeItems& children = i->GetChildren();
    if (children.GetCount() > 0)
    {
         return children.Item(0);
    }
    else
    {
                  wxTreeItemId p = item;
         wxTreeItemId toFind;
         do
         {
              toFind = GetNextSibling(p);
              p = GetItemParent(p);
         } while (p.IsOk() && !toFind.IsOk());
         return toFind;
    }
}

wxTreeItemId wxGenericTreeCtrl::GetFirstVisibleItem() const
{
    wxTreeItemId itemid = GetRootItem();
    if (!itemid.IsOk())
        return itemid;

    do
    {
        if (IsVisible(itemid))
              return itemid;
        itemid = GetNext(itemid);
    } while (itemid.IsOk());

    return wxTreeItemId();
}

wxTreeItemId wxGenericTreeCtrl::GetNextVisible(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );
    wxASSERT_MSG( IsVisible(item), wxT("this item itself should be visible") );

    wxTreeItemId id = item;
    if (id.IsOk())
    {
        while (id = GetNext(id), id.IsOk())
        {
            if (IsVisible(id))
                return id;
        }
    }
    return wxTreeItemId();
}

wxTreeItemId wxGenericTreeCtrl::GetPrevVisible(const wxTreeItemId& item) const
{
    wxCHECK_MSG( item.IsOk(), wxTreeItemId(), wxT("invalid tree item") );
    wxASSERT_MSG( IsVisible(item), wxT("this item itself should be visible") );

        wxTreeItemId prevItem = GetPrevSibling(item);
    if ( !prevItem.IsOk() )
    {
        prevItem = GetItemParent(item);
    }

        while ( prevItem.IsOk() && !IsVisible(prevItem) )
    {
        prevItem = GetNext(prevItem);
        if ( !prevItem.IsOk() || prevItem == item )
        {
                        return wxTreeItemId();
        }
    }

        while ( prevItem.IsOk() )
    {
        const wxTreeItemId nextItem = GetNextVisible(prevItem);
        if ( !nextItem.IsOk() || nextItem == item )
            break;

        prevItem = nextItem;
    }

    return prevItem;
}

void wxGenericTreeCtrl::ResetTextControl()
{
    m_textCtrl = NULL;
}

void wxGenericTreeCtrl::ResetFindState()
{
    m_findPrefix.clear();
    if ( m_findBell )
        m_findBell = 1;
}

wxTreeItemId wxGenericTreeCtrl::FindItem(const wxTreeItemId& idParent,
                                         const wxString& prefixOrig) const
{
                wxString prefix = prefixOrig.Lower();

                    wxTreeItemId itemid = idParent;
    if ( prefix.length() == 1 )
    {
        itemid = GetNext(itemid);
    }

        while ( itemid.IsOk() && !GetItemText(itemid).Lower().StartsWith(prefix) )
    {
        itemid = GetNext(itemid);
    }

        if ( !itemid.IsOk() )
    {
                itemid = GetRootItem();
        if ( HasFlag(wxTR_HIDE_ROOT) )
        {
                        itemid = GetNext(itemid);
        }

                while ( itemid.IsOk() && itemid != idParent &&
                    !GetItemText(itemid).Lower().StartsWith(prefix) )
        {
            itemid = GetNext(itemid);
        }
                        if ( itemid == idParent )
        {
            itemid = wxTreeItemId();
        }
    }

    return itemid;
}


wxTreeItemId wxGenericTreeCtrl::DoInsertItem(const wxTreeItemId& parentId,
                                             size_t previous,
                                             const wxString& text,
                                             int image,
                                             int selImage,
                                             wxTreeItemData *data)
{
    wxGenericTreeItem *parent = (wxGenericTreeItem*) parentId.m_pItem;
    if ( !parent )
    {
                return AddRoot(text, image, selImage, data);
    }

    m_dirty = true;     
    wxGenericTreeItem *item =
        new wxGenericTreeItem( parent, text, image, selImage, data );

    if ( data != NULL )
    {
        data->m_pItem = item;
    }

    parent->Insert( item, previous == (size_t)-1 ? parent->GetChildren().size()
                                                 : previous );

    InvalidateBestSize();
    return item;
}

wxTreeItemId wxGenericTreeCtrl::AddRoot(const wxString& text,
                                        int image,
                                        int selImage,
                                        wxTreeItemData *data)
{
    wxCHECK_MSG( !m_anchor, wxTreeItemId(), "tree can have only one root" );

    m_dirty = true;     
    m_anchor = new wxGenericTreeItem(NULL, text,
                                   image, selImage, data);
    if ( data != NULL )
    {
        data->m_pItem = m_anchor;
    }

    if (HasFlag(wxTR_HIDE_ROOT))
    {
                        m_anchor->SetHasPlus();
        m_anchor->Expand();
        CalculatePositions();
    }

    if (!HasFlag(wxTR_MULTIPLE))
    {
        m_current = m_key_current = m_anchor;
        m_current->SetHilight( true );
    }

    InvalidateBestSize();
    return m_anchor;
}

wxTreeItemId wxGenericTreeCtrl::DoInsertAfter(const wxTreeItemId& parentId,
                                              const wxTreeItemId& idPrevious,
                                              const wxString& text,
                                              int image, int selImage,
                                              wxTreeItemData *data)
{
    wxGenericTreeItem *parent = (wxGenericTreeItem*) parentId.m_pItem;
    if ( !parent )
    {
                return AddRoot(text, image, selImage, data);
    }

    int index = -1;
    if (idPrevious.IsOk())
    {
        index = parent->GetChildren().Index(
                    (wxGenericTreeItem*) idPrevious.m_pItem);
        wxASSERT_MSG( index != wxNOT_FOUND,
                      "previous item in wxGenericTreeCtrl::InsertItem() "
                      "is not a sibling" );
    }

    return DoInsertItem(parentId, (size_t)++index, text, image, selImage, data);
}


void wxGenericTreeCtrl::SendDeleteEvent(wxGenericTreeItem *item)
{
    wxTreeEvent event(wxEVT_TREE_DELETE_ITEM, this, item);
    GetEventHandler()->ProcessEvent( event );
}

void wxGenericTreeCtrl::ChildrenClosing(wxGenericTreeItem* item)
{
    if ( m_textCtrl && item != m_textCtrl->item() &&
            IsDescendantOf(item, m_textCtrl->item()) )
    {
        m_textCtrl->EndEdit( true );
    }

    if ( item != m_key_current && IsDescendantOf(item, m_key_current) )
    {
        m_key_current = NULL;
    }

    if ( IsDescendantOf(item, m_select_me) )
    {
        m_select_me = item;
    }

    if ( item != m_current && IsDescendantOf(item, m_current) )
    {
        m_current->SetHilight( false );
        m_current = NULL;
        m_select_me = item;
    }
}

void wxGenericTreeCtrl::DeleteChildren(const wxTreeItemId& itemId)
{
    m_dirty = true;     
    wxGenericTreeItem *item = (wxGenericTreeItem*) itemId.m_pItem;
    ChildrenClosing(item);
    item->DeleteChildren(this);
    InvalidateBestSize();
}

void wxGenericTreeCtrl::Delete(const wxTreeItemId& itemId)
{
    m_dirty = true;     
    wxGenericTreeItem *item = (wxGenericTreeItem*) itemId.m_pItem;

    if (m_textCtrl != NULL && IsDescendantOf(item, m_textCtrl->item()))
    {
                m_textCtrl->EndEdit( true );
    }

    wxGenericTreeItem *parent = item->GetParent();

        wxGenericTreeItem *to_be_selected = parent;
    if (parent)
    {
                int pos = parent->GetChildren().Index( item );
        if ((int)(parent->GetChildren().GetCount()) > pos+1)
            to_be_selected = parent->GetChildren().Item( pos+1 );
    }

        if ( IsDescendantOf(item, m_key_current) )
    {
                        
                m_key_current = NULL;
    }

            if ( m_select_me && IsDescendantOf(item, m_select_me) )
    {
        m_select_me = to_be_selected;
    }

    if ( IsDescendantOf(item, m_current) )
    {
                        
                m_current = NULL;
        m_select_me = to_be_selected;
    }

        if ( parent )
    {
        parent->GetChildren().Remove( item );      }
    else     {
                m_anchor = NULL;
    }

        item->DeleteChildren(this);
    SendDeleteEvent(item);

    if (item == m_select_me)
        m_select_me = NULL;

    delete item;

    InvalidateBestSize();
}

void wxGenericTreeCtrl::DeleteAllItems()
{
    if ( m_anchor )
    {
        Delete(m_anchor);
    }
}

void wxGenericTreeCtrl::Expand(const wxTreeItemId& itemId)
{
    wxGenericTreeItem *item = (wxGenericTreeItem*) itemId.m_pItem;

    wxCHECK_RET( item, wxT("invalid item in wxGenericTreeCtrl::Expand") );
    wxCHECK_RET( !HasFlag(wxTR_HIDE_ROOT) || itemId != GetRootItem(),
                 wxT("can't expand hidden root") );

    if ( !item->HasPlus() )
        return;

    if ( item->IsExpanded() )
        return;

    wxTreeEvent event(wxEVT_TREE_ITEM_EXPANDING, this, item);

    if ( GetEventHandler()->ProcessEvent( event ) && !event.IsAllowed() )
    {
                return;
    }

    item->Expand();
    if ( !IsFrozen() )
    {
        CalculatePositions();

        RefreshSubtree(item);
    }
    else     {
        m_dirty = true;
    }

    event.SetEventType(wxEVT_TREE_ITEM_EXPANDED);
    GetEventHandler()->ProcessEvent( event );
}

void wxGenericTreeCtrl::Collapse(const wxTreeItemId& itemId)
{
    wxCHECK_RET( !HasFlag(wxTR_HIDE_ROOT) || itemId != GetRootItem(),
                 wxT("can't collapse hidden root") );

    wxGenericTreeItem *item = (wxGenericTreeItem*) itemId.m_pItem;

    if ( !item->IsExpanded() )
        return;

    wxTreeEvent event(wxEVT_TREE_ITEM_COLLAPSING, this, item);
    if ( GetEventHandler()->ProcessEvent( event ) && !event.IsAllowed() )
    {
                return;
    }

    ChildrenClosing(item);
    item->Collapse();

#if 0      wxArrayGenericTreeItems& children = item->GetChildren();
    size_t count = children.GetCount();
    for ( size_t n = 0; n < count; n++ )
    {
        Collapse(children[n]);
    }
#endif

    CalculatePositions();

    RefreshSubtree(item);

    event.SetEventType(wxEVT_TREE_ITEM_COLLAPSED);
    GetEventHandler()->ProcessEvent( event );
}

void wxGenericTreeCtrl::CollapseAndReset(const wxTreeItemId& item)
{
    Collapse(item);
    DeleteChildren(item);
}

void wxGenericTreeCtrl::Toggle(const wxTreeItemId& itemId)
{
    wxGenericTreeItem *item = (wxGenericTreeItem*) itemId.m_pItem;

    if (item->IsExpanded())
        Collapse(itemId);
    else
        Expand(itemId);
}

void wxGenericTreeCtrl::Unselect()
{
    if (m_current)
    {
        m_current->SetHilight( false );
        RefreshLine( m_current );

        m_current = NULL;
        m_select_me = NULL;
    }
}

void wxGenericTreeCtrl::ClearFocusedItem()
{
    wxTreeItemId item = GetFocusedItem();
    if ( item.IsOk() )
        SelectItem(item, false);

    m_current = NULL;
}

void wxGenericTreeCtrl::SetFocusedItem(const wxTreeItemId& item)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    SelectItem(item, true);
}

void wxGenericTreeCtrl::UnselectAllChildren(wxGenericTreeItem *item)
{
    if (item->IsSelected())
    {
        item->SetHilight(false);
        RefreshLine(item);
    }

    if (item->HasChildren())
    {
        wxArrayGenericTreeItems& children = item->GetChildren();
        size_t count = children.GetCount();
        for ( size_t n = 0; n < count; ++n )
        {
            UnselectAllChildren(children[n]);
        }
    }
}

void wxGenericTreeCtrl::UnselectAll()
{
    wxTreeItemId rootItem = GetRootItem();

        if ( rootItem )
    {
        UnselectAllChildren((wxGenericTreeItem*) rootItem.m_pItem);
    }
}

void wxGenericTreeCtrl::SelectChildren(const wxTreeItemId& parent)
{
    wxCHECK_RET( HasFlag(wxTR_MULTIPLE),
                 "this only works with multiple selection controls" );

    UnselectAll();

    if ( !HasChildren(parent) )
        return;


    wxArrayGenericTreeItems&
        children = ((wxGenericTreeItem*) parent.m_pItem)->GetChildren();
    size_t count = children.GetCount();

    wxGenericTreeItem *
        item = (wxGenericTreeItem*) ((wxTreeItemId)children[0]).m_pItem;
    wxTreeEvent event(wxEVT_TREE_SEL_CHANGING, this, item);
    event.m_itemOld = m_current;

    if ( GetEventHandler()->ProcessEvent( event ) && !event.IsAllowed() )
        return;

    for ( size_t n = 0; n < count; ++n )
    {
        m_current = m_key_current = children[n];
        m_current->SetHilight(true);
        RefreshSelected();
    }


    event.SetEventType(wxEVT_TREE_SEL_CHANGED);
    GetEventHandler()->ProcessEvent( event );
}


bool
wxGenericTreeCtrl::TagNextChildren(wxGenericTreeItem *crt_item,
                                   wxGenericTreeItem *last_item,
                                   bool select)
{
    wxGenericTreeItem *parent = crt_item->GetParent();

    if (parent == NULL)         return TagAllChildrenUntilLast(crt_item, last_item, select);

    wxArrayGenericTreeItems& children = parent->GetChildren();
    int index = children.Index(crt_item);
    wxASSERT( index != wxNOT_FOUND ); 
    size_t count = children.GetCount();
    for (size_t n=(size_t)(index+1); n<count; ++n)
    {
        if ( TagAllChildrenUntilLast(children[n], last_item, select) )
            return true;
    }

    return TagNextChildren(parent, last_item, select);
}

bool
wxGenericTreeCtrl::TagAllChildrenUntilLast(wxGenericTreeItem *crt_item,
                                           wxGenericTreeItem *last_item,
                                           bool select)
{
    crt_item->SetHilight(select);
    RefreshLine(crt_item);

    if (crt_item==last_item)
        return true;

        if (crt_item->HasChildren() && crt_item->IsExpanded())
    {
        wxArrayGenericTreeItems& children = crt_item->GetChildren();
        size_t count = children.GetCount();
        for ( size_t n = 0; n < count; ++n )
        {
            if (TagAllChildrenUntilLast(children[n], last_item, select))
                return true;
        }
    }

  return false;
}

void
wxGenericTreeCtrl::SelectItemRange(wxGenericTreeItem *item1,
                                   wxGenericTreeItem *item2)
{
    m_select_me = NULL;

            wxGenericTreeItem *first= (item1->GetY()<item2->GetY()) ? item1 : item2;
    wxGenericTreeItem *last = (item1->GetY()<item2->GetY()) ? item2 : item1;

    bool select = m_current->IsSelected();

    if ( TagAllChildrenUntilLast(first,last,select) )
        return;

    TagNextChildren(first,last,select);
}

void wxGenericTreeCtrl::DoSelectItem(const wxTreeItemId& itemId,
                                     bool unselect_others,
                                     bool extended_select)
{
    wxCHECK_RET( itemId.IsOk(), wxT("invalid tree item") );

    m_select_me = NULL;

    bool is_single=!(GetWindowStyleFlag() & wxTR_MULTIPLE);
    wxGenericTreeItem *item = (wxGenericTreeItem*) itemId.m_pItem;

        
        if (is_single)
    {
        if (item->IsSelected())
            return;         unselect_others = true;
        extended_select = false;
    }
    else if ( unselect_others && item->IsSelected() )
    {
                wxArrayTreeItemIds selected_items;
        if ( GetSelections(selected_items) == 1 )
            return;
    }

    wxTreeEvent event(wxEVT_TREE_SEL_CHANGING, this, item);
    event.m_itemOld = m_current;
    
    if ( GetEventHandler()->ProcessEvent( event ) && !event.IsAllowed() )
        return;

    wxTreeItemId parent = GetItemParent( itemId );
    while (parent.IsOk())
    {
        if (!IsExpanded(parent))
            Expand( parent );

        parent = GetItemParent( parent );
    }

        if (unselect_others)
    {
        if (is_single) Unselect();         else UnselectAll();
    }

        if (extended_select)
    {
        if ( !m_current )
        {
            m_current =
            m_key_current = (wxGenericTreeItem*) GetRootItem().m_pItem;
        }

                SelectItemRange(m_current, item);
    }
    else
    {
        bool select = true; 
                if (!unselect_others)
            select=!item->IsSelected();

        m_current = m_key_current = item;
        m_current->SetHilight(select);
        RefreshLine( m_current );
    }

                EnsureVisible( itemId );

    event.SetEventType(wxEVT_TREE_SEL_CHANGED);
    GetEventHandler()->ProcessEvent( event );
}

void wxGenericTreeCtrl::SelectItem(const wxTreeItemId& itemId, bool select)
{
    wxGenericTreeItem * const item = (wxGenericTreeItem*) itemId.m_pItem;
    wxCHECK_RET( item, wxT("SelectItem(): invalid tree item") );

    if ( select )
    {
        if ( !item->IsSelected() )
            DoSelectItem(itemId, !HasFlag(wxTR_MULTIPLE));
    }
    else     {
        wxTreeEvent event(wxEVT_TREE_SEL_CHANGING, this, item);
        if ( GetEventHandler()->ProcessEvent( event ) && !event.IsAllowed() )
            return;

        item->SetHilight(false);
        RefreshLine(item);

        event.SetEventType(wxEVT_TREE_SEL_CHANGED);
        GetEventHandler()->ProcessEvent( event );
    }
}

void wxGenericTreeCtrl::FillArray(wxGenericTreeItem *item,
                                  wxArrayTreeItemIds &array) const
{
    if ( item->IsSelected() )
        array.Add(wxTreeItemId(item));

    if ( item->HasChildren() )
    {
        wxArrayGenericTreeItems& children = item->GetChildren();
        size_t count = children.GetCount();
        for ( size_t n = 0; n < count; ++n )
            FillArray(children[n], array);
    }
}

size_t wxGenericTreeCtrl::GetSelections(wxArrayTreeItemIds &array) const
{
    array.Empty();
    wxTreeItemId idRoot = GetRootItem();
    if ( idRoot.IsOk() )
    {
        FillArray((wxGenericTreeItem*) idRoot.m_pItem, array);
    }
    
    return array.GetCount();
}

void wxGenericTreeCtrl::EnsureVisible(const wxTreeItemId& item)
{
    wxCHECK_RET( item.IsOk(), wxT("invalid tree item") );

    if (!item.IsOk()) return;

    wxGenericTreeItem *gitem = (wxGenericTreeItem*) item.m_pItem;

        wxGenericTreeItem *parent = gitem->GetParent();

    if ( HasFlag(wxTR_HIDE_ROOT) )
    {
        while ( parent && parent != m_anchor )
        {
            Expand(parent);
            parent = parent->GetParent();
        }
    }
    else
    {
        while ( parent )
        {
            Expand(parent);
            parent = parent->GetParent();
        }
    }

    
    ScrollTo(item);
}

void wxGenericTreeCtrl::ScrollTo(const wxTreeItemId &item)
{
    if (!item.IsOk())
        return;

        if (m_dirty)
    {
#if defined( __WXMSW__ ) 
        Update();
#elif defined(__WXMAC__)
        Update();
        DoDirtyProcessing();
#else
        DoDirtyProcessing();
#endif
    }
        
    wxGenericTreeItem *gitem = (wxGenericTreeItem*) item.m_pItem;

    int itemY = gitem->GetY();

    int start_x = 0;
    int start_y = 0;
    GetViewStart( &start_x, &start_y );

    const int clientHeight = GetClientSize().y;

    const int itemHeight = GetLineHeight(gitem) + 2;

    if ( itemY + itemHeight > start_y*PIXELS_PER_UNIT + clientHeight )
    {
                itemY += itemHeight - clientHeight;

                                itemY += PIXELS_PER_UNIT - 1;
    }
    else if ( itemY > start_y*PIXELS_PER_UNIT )
    {
                return;
    }
    
    Scroll(-1, itemY/PIXELS_PER_UNIT);
}

static wxGenericTreeCtrl *s_treeBeingSorted = NULL;

static int LINKAGEMODE tree_ctrl_compare_func(wxGenericTreeItem **item1,
                                  wxGenericTreeItem **item2)
{
    wxCHECK_MSG( s_treeBeingSorted, 0,
                 "bug in wxGenericTreeCtrl::SortChildren()" );

    return s_treeBeingSorted->OnCompareItems(*item1, *item2);
}

void wxGenericTreeCtrl::SortChildren(const wxTreeItemId& itemId)
{
    wxCHECK_RET( itemId.IsOk(), wxT("invalid tree item") );

    wxGenericTreeItem *item = (wxGenericTreeItem*) itemId.m_pItem;

    wxCHECK_RET( !s_treeBeingSorted,
                 wxT("wxGenericTreeCtrl::SortChildren is not reentrant") );

    wxArrayGenericTreeItems& children = item->GetChildren();
    if ( children.GetCount() > 1 )
    {
        m_dirty = true;

        s_treeBeingSorted = this;
        children.Sort(tree_ctrl_compare_func);
        s_treeBeingSorted = NULL;
    }
    }

void wxGenericTreeCtrl::CalculateLineHeight()
{
    wxClientDC dc(this);
    m_lineHeight = (int)(dc.GetCharHeight() + 4);

    if ( m_imageListNormal )
    {
                                int n = m_imageListNormal->GetImageCount();
        for (int i = 0; i < n ; i++)
        {
            int width = 0, height = 0;
            m_imageListNormal->GetSize(i, width, height);
            if (height > m_lineHeight) m_lineHeight = height;
        }
    }

    if ( m_imageListState )
    {
                                int n = m_imageListState->GetImageCount();
        for (int i = 0; i < n ; i++)
        {
            int width = 0, height = 0;
            m_imageListState->GetSize(i, width, height);
            if (height > m_lineHeight) m_lineHeight = height;
        }
    }

    if (m_imageListButtons)
    {
                                int n = m_imageListButtons->GetImageCount();
        for (int i = 0; i < n ; i++)
        {
            int width = 0, height = 0;
            m_imageListButtons->GetSize(i, width, height);
            if (height > m_lineHeight) m_lineHeight = height;
        }
    }

    if (m_lineHeight < 30)
        m_lineHeight += 2;                     else
        m_lineHeight += m_lineHeight/10;   }

void wxGenericTreeCtrl::SetImageList(wxImageList *imageList)
{
    if (m_ownsImageListNormal) delete m_imageListNormal;
    m_imageListNormal = imageList;
    m_ownsImageListNormal = false;
    m_dirty = true;

    if (m_anchor)
        m_anchor->RecursiveResetSize();

            if (imageList)
        CalculateLineHeight();
}

void wxGenericTreeCtrl::SetStateImageList(wxImageList *imageList)
{
    if (m_ownsImageListState) delete m_imageListState;
    m_imageListState = imageList;
    m_ownsImageListState = false;
    m_dirty = true;

    if (m_anchor)
        m_anchor->RecursiveResetSize();

            if (imageList)
        CalculateLineHeight();
}

void wxGenericTreeCtrl::SetButtonsImageList(wxImageList *imageList)
{
    if (m_ownsImageListButtons) delete m_imageListButtons;
    m_imageListButtons = imageList;
    m_ownsImageListButtons = false;
    m_dirty = true;

    if (m_anchor)
        m_anchor->RecursiveResetSize();

    CalculateLineHeight();
}

void wxGenericTreeCtrl::AssignButtonsImageList(wxImageList *imageList)
{
    SetButtonsImageList(imageList);
    m_ownsImageListButtons = true;
}


void wxGenericTreeCtrl::AdjustMyScrollbars()
{
    if (m_anchor)
    {
        int x = 0, y = 0;
        m_anchor->GetSize( x, y, this );
        y += PIXELS_PER_UNIT+2;         x += PIXELS_PER_UNIT+2;         int x_pos = GetScrollPos( wxHORIZONTAL );
        int y_pos = GetScrollPos( wxVERTICAL );
        SetScrollbars( PIXELS_PER_UNIT, PIXELS_PER_UNIT,
                       x/PIXELS_PER_UNIT, y/PIXELS_PER_UNIT,
                       x_pos, y_pos );
    }
    else
    {
        SetScrollbars( 0, 0, 0, 0 );
    }
}

int wxGenericTreeCtrl::GetLineHeight(wxGenericTreeItem *item) const
{
    if (GetWindowStyleFlag() & wxTR_HAS_VARIABLE_ROW_HEIGHT)
        return item->GetHeight();
    else
        return m_lineHeight;
}

void wxGenericTreeCtrl::PaintItem(wxGenericTreeItem *item, wxDC& dc)
{
    item->SetFont(this, dc);
    item->CalculateSize(this, dc);

    wxCoord text_h = item->GetTextHeight();

    int image_h = 0, image_w = 0;
    int image = item->GetCurrentImage();
    if ( image != NO_IMAGE )
    {
        if ( m_imageListNormal )
        {
            m_imageListNormal->GetSize(image, image_w, image_h);
            image_w += MARGIN_BETWEEN_IMAGE_AND_TEXT;
        }
        else
        {
            image = NO_IMAGE;
        }
    }

    int state_h = 0, state_w = 0;
    int state = item->GetState();
    if ( state != wxTREE_ITEMSTATE_NONE )
    {
        if ( m_imageListState )
        {
            m_imageListState->GetSize(state, state_w, state_h);
            if ( image_w != 0 )
                state_w += MARGIN_BETWEEN_STATE_AND_IMAGE;
            else
                state_w += MARGIN_BETWEEN_IMAGE_AND_TEXT;
        }
        else
        {
            state = wxTREE_ITEMSTATE_NONE;
        }
    }

    int total_h = GetLineHeight(item);
    bool drawItemBackground = false,
         hasBgColour = false;

    if ( item->IsSelected() )
    {
        dc.SetBrush(*(m_hasFocus ? m_hilightBrush : m_hilightUnfocusedBrush));
        drawItemBackground = true;
    }
    else
    {
        wxColour colBg;
        wxTreeItemAttr * const attr = item->GetAttributes();
        if ( attr && attr->HasBackgroundColour() )
        {
            drawItemBackground =
            hasBgColour = true;
            colBg = attr->GetBackgroundColour();
        }
        else
        {
            colBg = GetBackgroundColour();
        }
        dc.SetBrush(wxBrush(colBg, wxBRUSHSTYLE_SOLID));
    }

    int offset = HasFlag(wxTR_ROW_LINES) ? 1 : 0;

    if ( HasFlag(wxTR_FULL_ROW_HIGHLIGHT) )
    {
        int x, w, h;
        x=0;
        GetVirtualSize(&w, &h);
        wxRect rect( x, item->GetY()+offset, w, total_h-offset);
        if (!item->IsSelected())
        {
            dc.DrawRectangle(rect);
        }
        else
        {
            int flags = wxCONTROL_SELECTED;
            if (m_hasFocus)
                flags |= wxCONTROL_FOCUSED;
            if ((item == m_current) && (m_hasFocus))
                flags |= wxCONTROL_CURRENT;

            wxRendererNative::Get().
                DrawItemSelectionRect(this, dc, rect, flags);
        }
    }
    else     {
        if ( item->IsSelected() &&
                (state != wxTREE_ITEMSTATE_NONE || image != NO_IMAGE) )
        {
                                                wxRect rect( item->GetX() + state_w + image_w - 2,
                         item->GetY() + offset,
                         item->GetWidth() - state_w - image_w + 2,
                         total_h - offset );
#if !defined(__WXGTK20__) && !defined(__WXMAC__)
            dc.DrawRectangle( rect );
#else
            rect.x -= 1;
            rect.width += 2;

            int flags = wxCONTROL_SELECTED;
            if (m_hasFocus)
                flags |= wxCONTROL_FOCUSED;
            if ((item == m_current) && (m_hasFocus))
                flags |= wxCONTROL_CURRENT;
            wxRendererNative::Get().
                DrawItemSelectionRect(this, dc, rect, flags);
#endif
        }
                                else if (drawItemBackground)
        {
            wxRect rect( item->GetX() + state_w + image_w - 2,
                         item->GetY() + offset,
                         item->GetWidth() - state_w - image_w + 2,
                         total_h - offset );
            if ( hasBgColour )
            {
                dc.DrawRectangle( rect );
            }
            else             {
                rect.x -= 1;
                rect.width += 2;

                int flags = wxCONTROL_SELECTED;
                if (m_hasFocus)
                    flags |= wxCONTROL_FOCUSED;
                if ((item == m_current) && (m_hasFocus))
                    flags |= wxCONTROL_CURRENT;
                wxRendererNative::Get().
                    DrawItemSelectionRect(this, dc, rect, flags);
            }
        }
    }

    if ( state != wxTREE_ITEMSTATE_NONE )
    {
        dc.SetClippingRegion( item->GetX(), item->GetY(), state_w, total_h );
        m_imageListState->Draw( state, dc,
                                item->GetX(),
                                item->GetY() +
                                    (total_h > state_h ? (total_h-state_h)/2
                                                       : 0),
                                wxIMAGELIST_DRAW_TRANSPARENT );
        dc.DestroyClippingRegion();
    }

    if ( image != NO_IMAGE )
    {
        dc.SetClippingRegion(item->GetX() + state_w, item->GetY(),
                             image_w, total_h);
        m_imageListNormal->Draw( image, dc,
                                 item->GetX() + state_w,
                                 item->GetY() +
                                    (total_h > image_h ? (total_h-image_h)/2
                                                       : 0),
                                 wxIMAGELIST_DRAW_TRANSPARENT );
        dc.DestroyClippingRegion();
    }

    dc.SetBackgroundMode(wxBRUSHSTYLE_TRANSPARENT);
    int extraH = (total_h > text_h) ? (total_h - text_h)/2 : 0;
    dc.DrawText( item->GetText(),
                 (wxCoord)(state_w + image_w + item->GetX()),
                 (wxCoord)(item->GetY() + extraH));

        dc.SetFont( m_normalFont );

    if (item == m_dndEffectItem)
    {
        dc.SetPen( *wxBLACK_PEN );
                switch (m_dndEffect)
        {
            case BorderEffect:
            {
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                int w = item->GetWidth() + 2;
                int h = total_h + 2;
                dc.DrawRectangle( item->GetX() - 1, item->GetY() - 1, w, h);
                break;
            }
            case AboveEffect:
            {
                int x = item->GetX(),
                    y = item->GetY();
                dc.DrawLine( x, y, x + item->GetWidth(), y);
                break;
            }
            case BelowEffect:
            {
                int x = item->GetX(),
                    y = item->GetY();
                y += total_h - 1;
                dc.DrawLine( x, y, x + item->GetWidth(), y);
                break;
            }
            case NoEffect:
                break;
        }
    }
}

void
wxGenericTreeCtrl::PaintLevel(wxGenericTreeItem *item,
                              wxDC &dc,
                              int level,
                              int &y)
{
    int x = level*m_indent;
    if (!HasFlag(wxTR_HIDE_ROOT))
    {
        x += m_indent;
    }
    else if (level == 0)
    {
                int origY = y;
        wxArrayGenericTreeItems& children = item->GetChildren();
        int count = children.GetCount();
        if (count > 0)
        {
            int n = 0, oldY;
            do {
                oldY = y;
                PaintLevel(children[n], dc, 1, y);
            } while (++n < count);

            if ( !HasFlag(wxTR_NO_LINES) && HasFlag(wxTR_LINES_AT_ROOT)
                    && count > 0 )
            {
                                origY += GetLineHeight(children[0])>>1;
                oldY += GetLineHeight(children[n-1])>>1;
                dc.DrawLine(3, origY, 3, oldY);
            }
        }
        return;
    }

    item->SetX(x+m_spacing);
    item->SetY(y);

    int h = GetLineHeight(item);
    int y_top = y;
    int y_mid = y_top + (h>>1);
    y += h;

    int exposed_x = dc.LogicalToDeviceX(0);
    int exposed_y = dc.LogicalToDeviceY(y_top);

    if (IsExposed(exposed_x, exposed_y, 10000, h))      {
        const wxPen *pen =
#ifndef __WXMAC__
                                    (item->IsSelected() && m_hasFocus) ? wxBLACK_PEN :
#endif             wxTRANSPARENT_PEN;

        wxColour colText;
        if ( item->IsSelected() )
        {
#ifdef __WXMAC__
            colText = *wxWHITE;
#else
            if (m_hasFocus)
                colText = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
            else
                colText = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT);
#endif
        }
        else
        {
            wxTreeItemAttr *attr = item->GetAttributes();
            if (attr && attr->HasTextColour())
                colText = attr->GetTextColour();
            else
                colText = GetForegroundColour();
        }

                dc.SetTextForeground(colText);
        dc.SetPen(*pen);

                PaintItem(item, dc);

        if (HasFlag(wxTR_ROW_LINES))
        {
                                    dc.SetPen(*((GetBackgroundColour() == *wxWHITE)
                         ? wxMEDIUM_GREY_PEN : wxWHITE_PEN));
            dc.DrawLine(0, y_top, 10000, y_top);
            dc.DrawLine(0, y, 10000, y);
        }

                dc.SetBrush(*wxWHITE_BRUSH);
        dc.SetPen(m_dottedPen);
        dc.SetTextForeground(*wxBLACK);

        if ( !HasFlag(wxTR_NO_LINES) )
        {
                        int x_start = x;
            if (x > (signed)m_indent)
                x_start -= m_indent;
            else if (HasFlag(wxTR_LINES_AT_ROOT))
                x_start = 3;
            dc.DrawLine(x_start, y_mid, x + m_spacing, y_mid);
        }

                if ( item->HasPlus() && HasButtons() )
        {
            if ( m_imageListButtons )
            {
                                int image_h = 0,
                    image_w = 0;
                int image = item->IsExpanded() ? wxTreeItemIcon_Expanded
                                               : wxTreeItemIcon_Normal;
                if ( item->IsSelected() )
                    image += wxTreeItemIcon_Selected - wxTreeItemIcon_Normal;

                m_imageListButtons->GetSize(image, image_w, image_h);
                int xx = x - image_w/2;
                int yy = y_mid - image_h/2;

                wxDCClipper clip(dc, xx, yy, image_w, image_h);
                m_imageListButtons->Draw(image, dc, xx, yy,
                                         wxIMAGELIST_DRAW_TRANSPARENT);
            }
            else             {
                static const int wImage = 9;
                static const int hImage = 9;

                int flag = 0;
                if (item->IsExpanded())
                    flag |= wxCONTROL_EXPANDED;
                if (item == m_underMouse)
                    flag |= wxCONTROL_CURRENT;

                wxRendererNative::Get().DrawTreeItemButton
                                        (
                                            this,
                                            dc,
                                            wxRect(x - wImage/2,
                                                   y_mid - hImage/2,
                                                   wImage, hImage),
                                            flag
                                        );
            }
        }
    }

    if (item->IsExpanded())
    {
        wxArrayGenericTreeItems& children = item->GetChildren();
        int count = children.GetCount();
        if (count > 0)
        {
            int n = 0, oldY;
            ++level;
            do {
                oldY = y;
                PaintLevel(children[n], dc, level, y);
            } while (++n < count);

            if (!HasFlag(wxTR_NO_LINES) && count > 0)
            {
                                oldY += GetLineHeight(children[n-1])>>1;
                if (HasButtons()) y_mid += 5;

                                                wxCoord xOrigin=0, yOrigin=0, width, height;
                dc.GetDeviceOrigin(&xOrigin, &yOrigin);
                yOrigin = abs(yOrigin);
                GetClientSize(&width, &height);

                                if (y_mid < yOrigin)
                    y_mid = yOrigin;
                if (oldY > yOrigin + height)
                    oldY = yOrigin + height;

                                                if (y_mid < oldY)
                    dc.DrawLine(x, y_mid, x, oldY);
            }
        }
    }
}

void wxGenericTreeCtrl::DrawDropEffect(wxGenericTreeItem *item)
{
    if ( item )
    {
        if ( item->HasPlus() )
        {
                        DrawBorder(item);
        }
        else
        {
                                    DrawLine(item, !m_dropEffectAboveItem );
        }

        SetCursor(*wxSTANDARD_CURSOR);
    }
    else
    {
                SetCursor(wxCURSOR_NO_ENTRY);
    }
}

void wxGenericTreeCtrl::DrawBorder(const wxTreeItemId &item)
{
    wxCHECK_RET( item.IsOk(), "invalid item in wxGenericTreeCtrl::DrawLine" );

    wxGenericTreeItem *i = (wxGenericTreeItem*) item.m_pItem;

    if (m_dndEffect == NoEffect)
    {
        m_dndEffect = BorderEffect;
        m_dndEffectItem = i;
    }
    else
    {
        m_dndEffect = NoEffect;
        m_dndEffectItem = NULL;
    }

    wxRect rect( i->GetX()-1, i->GetY()-1, i->GetWidth()+2, GetLineHeight(i)+2 );
    CalcScrolledPosition( rect.x, rect.y, &rect.x, &rect.y );
    RefreshRect( rect );
}

void wxGenericTreeCtrl::DrawLine(const wxTreeItemId &item, bool below)
{
    wxCHECK_RET( item.IsOk(), "invalid item in wxGenericTreeCtrl::DrawLine" );

    wxGenericTreeItem *i = (wxGenericTreeItem*) item.m_pItem;

    if (m_dndEffect == NoEffect)
    {
        if (below)
            m_dndEffect = BelowEffect;
        else
            m_dndEffect = AboveEffect;
        m_dndEffectItem = i;
    }
    else
    {
        m_dndEffect = NoEffect;
        m_dndEffectItem = NULL;
    }

    wxRect rect( i->GetX()-1, i->GetY()-1, i->GetWidth()+2, GetLineHeight(i)+2 );
    CalcScrolledPosition( rect.x, rect.y, &rect.x, &rect.y );
    RefreshRect( rect );
}


void wxGenericTreeCtrl::OnSize( wxSizeEvent &event )
{
#ifdef __WXGTK__
    if (HasFlag( wxTR_FULL_ROW_HIGHLIGHT) && m_current)
        RefreshLine( m_current );
#endif

    event.Skip(true);
}

void wxGenericTreeCtrl::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    wxPaintDC dc(this);
    PrepareDC( dc );

    if ( !m_anchor)
        return;

    dc.SetFont( m_normalFont );
    dc.SetPen( m_dottedPen );

            
    int y = 2;
    PaintLevel( m_anchor, dc, 0, y );
}

void wxGenericTreeCtrl::OnSetFocus( wxFocusEvent &event )
{
    m_hasFocus = true;

    RefreshSelected();

    event.Skip();
}

void wxGenericTreeCtrl::OnKillFocus( wxFocusEvent &event )
{
    m_hasFocus = false;

    RefreshSelected();

    event.Skip();
}

void wxGenericTreeCtrl::OnKeyDown( wxKeyEvent &event )
{
        wxTreeEvent te( wxEVT_TREE_KEY_DOWN, this);
    te.m_evtKey = event;
    if ( GetEventHandler()->ProcessEvent( te ) )
        return;

    event.Skip();
}

void wxGenericTreeCtrl::OnChar( wxKeyEvent &event )
{
    if ( (m_current == 0) || (m_key_current == 0) )
    {
        event.Skip();
        return;
    }

        bool is_multiple, extended_select, unselect_others;
    EventFlagsToSelType(GetWindowStyleFlag(),
                        event.ShiftDown(),
                        event.CmdDown(),
                        is_multiple, extended_select, unselect_others);

    if (GetLayoutDirection() == wxLayout_RightToLeft)
    {
        if (event.GetKeyCode() == WXK_RIGHT)
            event.m_keyCode = WXK_LEFT;
        else if (event.GetKeyCode() == WXK_LEFT)
            event.m_keyCode = WXK_RIGHT;
    }

                                                int keyCode = event.GetKeyCode();

#ifdef __WXOSX__
                if (keyCode == WXK_RIGHT)
    {
        keyCode = '+';
    }
    else if (keyCode == WXK_LEFT && IsExpanded(m_current))
    {
        keyCode = '-';
    }
#endif 
    switch ( keyCode )
    {
        case '+':
        case WXK_ADD:
            if (m_current->HasPlus() && !IsExpanded(m_current))
            {
                Expand(m_current);
            }
            break;

        case '*':
        case WXK_MULTIPLY:
            if ( !IsExpanded(m_current) )
            {
                                ExpandAllChildren(m_current);
                break;
            }
            wxFALLTHROUGH;
        case '-':
        case WXK_SUBTRACT:
            if (IsExpanded(m_current))
            {
                Collapse(m_current);
            }
            break;

        case WXK_MENU:
            {
                                                wxRect ItemRect;
                GetBoundingRect(m_current, ItemRect, true);

                wxTreeEvent
                    eventMenu(wxEVT_TREE_ITEM_MENU, this, m_current);
                                eventMenu.m_pointDrag = wxPoint(ItemRect.GetX(),
                                                ItemRect.GetY() +
                                                    ItemRect.GetHeight() / 2);
                GetEventHandler()->ProcessEvent( eventMenu );
            }
            break;

        case ' ':
        case WXK_RETURN:
            if ( !event.HasModifiers() )
            {
                wxTreeEvent
                   eventAct(wxEVT_TREE_ITEM_ACTIVATED, this, m_current);
                GetEventHandler()->ProcessEvent( eventAct );
            }

                                                                        event.Skip();
            break;

                                case WXK_UP:
            {
                wxTreeItemId prev = GetPrevSibling( m_key_current );
                if (!prev)
                {
                    prev = GetItemParent( m_key_current );
                    if ((prev == GetRootItem()) && HasFlag(wxTR_HIDE_ROOT))
                    {
                        break;                      }
                    if (prev)
                    {
                        wxTreeItemIdValue cookie;
                        wxTreeItemId current = m_key_current;
                                                                        if (current == GetFirstChild( prev, cookie ))
                        {
                                                        DoSelectItem(prev,
                                         unselect_others,
                                         extended_select);
                            m_key_current = (wxGenericTreeItem*) prev.m_pItem;
                            break;
                        }
                    }
                }
                if (prev)
                {
                    while ( IsExpanded(prev) && HasChildren(prev) )
                    {
                        wxTreeItemId child = GetLastChild(prev);
                        if ( child )
                        {
                            prev = child;
                        }
                    }

                    DoSelectItem( prev, unselect_others, extended_select );
                    m_key_current=(wxGenericTreeItem*) prev.m_pItem;
                }
            }
            break;

                    case WXK_LEFT:
            {
                wxTreeItemId prev = GetItemParent( m_current );
                if ((prev == GetRootItem()) && HasFlag(wxTR_HIDE_ROOT))
                {
                                        prev = GetPrevSibling( m_current );
                }
                if (prev)
                {
                    DoSelectItem( prev, unselect_others, extended_select );
                }
            }
            break;

        case WXK_RIGHT:
                        if (m_current != GetRootItem().m_pItem || !HasFlag(wxTR_HIDE_ROOT))
                Expand(m_current);
                                    break;

        case WXK_DOWN:
            {
                if (IsExpanded(m_key_current) && HasChildren(m_key_current))
                {
                    wxTreeItemIdValue cookie;
                    wxTreeItemId child = GetFirstChild( m_key_current, cookie );
                    if ( !child )
                        break;

                    DoSelectItem( child, unselect_others, extended_select );
                    m_key_current=(wxGenericTreeItem*) child.m_pItem;
                }
                else
                {
                    wxTreeItemId next = GetNextSibling( m_key_current );
                    if (!next)
                    {
                        wxTreeItemId current = m_key_current;
                        while (current.IsOk() && !next)
                        {
                            current = GetItemParent( current );
                            if (current) next = GetNextSibling( current );
                        }
                    }
                    if (next)
                    {
                        DoSelectItem( next, unselect_others, extended_select );
                        m_key_current=(wxGenericTreeItem*) next.m_pItem;
                    }
                }
            }
            break;

                    case WXK_END:
            {
                wxTreeItemId last = GetRootItem();

                while ( last.IsOk() && IsExpanded(last) )
                {
                    wxTreeItemId lastChild = GetLastChild(last);

                                                                                if ( !lastChild )
                        break;

                    last = lastChild;
                }

                if ( last.IsOk() )
                {
                    DoSelectItem( last, unselect_others, extended_select );
                }
            }
            break;

                    case WXK_HOME:
            {
                wxTreeItemId prev = GetRootItem();
                if (!prev)
                    break;

                if ( HasFlag(wxTR_HIDE_ROOT) )
                {
                    wxTreeItemIdValue cookie;
                    prev = GetFirstChild(prev, cookie);
                    if (!prev)
                        break;
                }

                DoSelectItem( prev, unselect_others, extended_select );
            }
            break;

        default:
                        if ( !event.HasModifiers() &&
                 ((keyCode >= '0' && keyCode <= '9') ||
                  (keyCode >= 'a' && keyCode <= 'z') ||
                  (keyCode >= 'A' && keyCode <= 'Z') ||
                  (keyCode == '_')))
            {
                                wxChar ch = (wxChar)keyCode;
                wxTreeItemId id;

                                                                                if ( m_findPrefix.length() == 1 && m_findPrefix[0] == ch )
                {
                    id = FindItem(m_current, ch);
                }
                else
                {
                    const wxString newPrefix(m_findPrefix + ch);
                    id = FindItem(m_current, newPrefix);
                    if ( id.IsOk() )
                        m_findPrefix = newPrefix;
                }

                                                                if ( !m_findTimer )
                {
                    m_findTimer = new wxTreeFindTimer(this);
                }

                                                m_findTimer->Start(wxTreeFindTimer::DELAY, wxTIMER_ONE_SHOT);

                if ( id.IsOk() )
                {
                    SelectItem(id);

                                                            if ( m_findBell )
                        m_findBell = 1;
                }
                else                 {
                                        if ( m_findBell == 1 )
                    {
                        ::wxBell();

                                                                                                m_findBell = -1;
                    }
                }
            }
            else
            {
                event.Skip();
            }
    }
}

wxTreeItemId
wxGenericTreeCtrl::DoTreeHitTest(const wxPoint& point, int& flags) const
{
    int w, h;
    GetSize(&w, &h);
    flags=0;
    if (point.x<0) flags |= wxTREE_HITTEST_TOLEFT;
    if (point.x>w) flags |= wxTREE_HITTEST_TORIGHT;
    if (point.y<0) flags |= wxTREE_HITTEST_ABOVE;
    if (point.y>h) flags |= wxTREE_HITTEST_BELOW;
    if (flags) return wxTreeItemId();

    if (m_anchor == NULL)
    {
        flags = wxTREE_HITTEST_NOWHERE;
        return wxTreeItemId();
    }

    wxGenericTreeItem *hit =  m_anchor->HitTest(CalcUnscrolledPosition(point),
                                                this, flags, 0);
    if (hit == NULL)
    {
        flags = wxTREE_HITTEST_NOWHERE;
        return wxTreeItemId();
    }
    return hit;
}

bool wxGenericTreeCtrl::GetBoundingRect(const wxTreeItemId& item,
                                        wxRect& rect,
                                        bool textOnly) const
{
    wxCHECK_MSG( item.IsOk(), false,
                 "invalid item in wxGenericTreeCtrl::GetBoundingRect" );

    wxGenericTreeItem *i = (wxGenericTreeItem*) item.m_pItem;

    if ( textOnly )
    {
        int image_h = 0, image_w = 0;
        int image = ((wxGenericTreeItem*) item.m_pItem)->GetCurrentImage();
        if ( image != NO_IMAGE && m_imageListNormal )
        {
            m_imageListNormal->GetSize( image, image_w, image_h );
            image_w += MARGIN_BETWEEN_IMAGE_AND_TEXT;
        }

        int state_h = 0, state_w = 0;
        int state = ((wxGenericTreeItem*) item.m_pItem)->GetState();
        if ( state != wxTREE_ITEMSTATE_NONE && m_imageListState )
        {
            m_imageListState->GetSize( state, state_w, state_h );
            if ( image_w != 0 )
                state_w += MARGIN_BETWEEN_STATE_AND_IMAGE;
            else
                state_w += MARGIN_BETWEEN_IMAGE_AND_TEXT;
        }

        rect.x = i->GetX() + state_w + image_w;
        rect.width = i->GetWidth() - state_w - image_w;

    }
    else     {
        rect.x = 0;
        rect.width = GetClientSize().x;
    }

    rect.y = i->GetY();
    rect.height = GetLineHeight(i);

        rect.SetTopLeft(CalcScrolledPosition(rect.GetTopLeft()));

    return true;
}

wxTextCtrl *wxGenericTreeCtrl::EditLabel(const wxTreeItemId& item,
                                  wxClassInfo * WXUNUSED(textCtrlClass))
{
    wxCHECK_MSG( item.IsOk(), NULL, wxT("can't edit an invalid item") );

    wxGenericTreeItem *itemEdit = (wxGenericTreeItem *)item.m_pItem;

    wxTreeEvent te(wxEVT_TREE_BEGIN_LABEL_EDIT, this, itemEdit);
    if ( GetEventHandler()->ProcessEvent( te ) && !te.IsAllowed() )
    {
                return NULL;
    }

                if ( m_dirty )
        DoDirtyProcessing();

        m_textCtrl = new wxTreeTextCtrl(this, itemEdit);

    m_textCtrl->SetFocus();

    return m_textCtrl;
}

wxTextCtrl* wxGenericTreeCtrl::GetEditControl() const
{
    return m_textCtrl;
}

void wxGenericTreeCtrl::EndEditLabel(const wxTreeItemId& WXUNUSED(item),
                                     bool discardChanges)
{
    if (m_textCtrl)
    {
        m_textCtrl->EndEdit(discardChanges);
    }
}

bool wxGenericTreeCtrl::OnRenameAccept(wxGenericTreeItem *item,
                                       const wxString& value)
{
    wxTreeEvent le(wxEVT_TREE_END_LABEL_EDIT, this, item);
    le.m_label = value;
    le.m_editCancelled = false;

    return !GetEventHandler()->ProcessEvent( le ) || le.IsAllowed();
}

void wxGenericTreeCtrl::OnRenameCancelled(wxGenericTreeItem *item)
{
        wxTreeEvent le(wxEVT_TREE_END_LABEL_EDIT, this, item);
    le.m_label = wxEmptyString;
    le.m_editCancelled = true;

    GetEventHandler()->ProcessEvent( le );
}

void wxGenericTreeCtrl::OnRenameTimer()
{
    EditLabel( m_current );
}

void wxGenericTreeCtrl::OnMouse( wxMouseEvent &event )
{
    if ( !m_anchor )return;

    wxPoint pt = CalcUnscrolledPosition(event.GetPosition());

        int flags = 0;
    wxGenericTreeItem *thisItem = m_anchor->HitTest(pt, this, flags, 0);
    wxGenericTreeItem *underMouse = thisItem;
#if wxUSE_TOOLTIPS
    bool underMouseChanged = (underMouse != m_underMouse) ;
#endif 
    if ((underMouse) &&
        (flags & wxTREE_HITTEST_ONITEMBUTTON) &&
        (!event.LeftIsDown()) &&
        (!m_isDragging) &&
        (!m_renameTimer || !m_renameTimer->IsRunning()))
    {
    }
    else
    {
        underMouse = NULL;
    }

    if (underMouse != m_underMouse)
    {
         if (m_underMouse)
         {
                        wxGenericTreeItem *tmp = m_underMouse;
            m_underMouse = NULL;
            RefreshLine( tmp );
         }

         m_underMouse = underMouse;
         if (m_underMouse)
            RefreshLine( m_underMouse );
    }

#if wxUSE_TOOLTIPS
        wxTreeItemId hoverItem = thisItem;

            if ( underMouseChanged &&
            hoverItem.IsOk() &&
              !m_isDragging &&
                (!m_renameTimer || !m_renameTimer->IsRunning()) )
    {
                wxTreeEvent
            hevent(wxEVT_TREE_ITEM_GETTOOLTIP,  this, hoverItem);

                #ifdef __WXOSX__
        if ( event.Leaving() )
            SetToolTip(NULL);
        else
#endif
        if ( GetEventHandler()->ProcessEvent(hevent) )
        {
                                    if ( hevent.IsAllowed() )
                SetToolTip(hevent.m_label);
            else
                SetToolTip(NULL);
        }
    }
#endif

                if ( !(event.LeftDown() ||
           event.LeftUp() ||
           event.MiddleDown() ||
           event.RightDown() ||
           event.LeftDClick() ||
           event.Dragging() ||
           ((event.Moving() || event.RightUp()) && m_isDragging)) )
    {
        event.Skip();

        return;
    }


    flags = 0;
    wxGenericTreeItem *item = m_anchor->HitTest(pt, this, flags, 0);

    if ( event.Dragging() && !m_isDragging )
    {
        if (m_dragCount == 0)
            m_dragStart = pt;

        m_dragCount++;

        if (m_dragCount != 3)
        {
                        return;
        }

        wxEventType command = event.RightIsDown()
                              ? wxEVT_TREE_BEGIN_RDRAG
                              : wxEVT_TREE_BEGIN_DRAG;

        wxTreeEvent nevent(command,  this, m_current);
        nevent.SetPoint(CalcScrolledPosition(pt));

                        nevent.Veto();

        if ( GetEventHandler()->ProcessEvent(nevent) && nevent.IsAllowed() )
        {
                        m_isDragging = true;

                                    m_oldCursor = m_cursor;

                        if ( !(GetWindowStyleFlag() & wxTR_MULTIPLE) )
            {
                m_oldSelection = (wxGenericTreeItem*) GetSelection().m_pItem;

                if ( m_oldSelection )
                {
                    m_oldSelection->SetHilight(false);
                    RefreshLine(m_oldSelection);
                }
            }

            CaptureMouse();
        }
    }
    else if ( event.Dragging() )
    {
        if ( item != m_dropTarget )
        {
                        DrawDropEffect(m_dropTarget);

            m_dropTarget = item;

                        DrawDropEffect(m_dropTarget);

#if defined(__WXMSW__) || defined(__WXMAC__) || defined(__WXGTK20__)
            Update();
#else
                                    wxYieldIfNeeded();
#endif
        }
    }
    else if ( (event.LeftUp() || event.RightUp()) && m_isDragging )
    {
        ReleaseMouse();

                DrawDropEffect(m_dropTarget);

        if ( m_oldSelection )
        {
            m_oldSelection->SetHilight(true);
            RefreshLine(m_oldSelection);
            m_oldSelection = NULL;
        }

                wxTreeEvent eventEndDrag(wxEVT_TREE_END_DRAG,  this, item);

        eventEndDrag.m_pointDrag = CalcScrolledPosition(pt);

        (void)GetEventHandler()->ProcessEvent(eventEndDrag);

        m_isDragging = false;
        m_dropTarget = NULL;

        SetCursor(m_oldCursor);

#if defined( __WXMSW__ ) || defined(__WXMAC__) || defined(__WXGTK20__)
        Update();
#else
                        wxYieldIfNeeded();
#endif
    }
    else
    {
                                                                if ( event.LeftDown() )
        {
            event.Skip();
        }

        
        m_dragCount = 0;

        if (item == NULL) return;  

        if ( event.RightDown() )
        {
                                                if (!IsSelected(item))
            {
                DoSelectItem(item, true, false);
            }

            wxTreeEvent
                nevent(wxEVT_TREE_ITEM_RIGHT_CLICK,  this, item);
            nevent.m_pointDrag = CalcScrolledPosition(pt);
            event.Skip(!GetEventHandler()->ProcessEvent(nevent));

                                    wxTreeEvent nevent2(wxEVT_TREE_ITEM_MENU,  this, item);
            nevent2.m_pointDrag = CalcScrolledPosition(pt);
            GetEventHandler()->ProcessEvent(nevent2);
        }
        else if ( event.MiddleDown() )
        {
            wxTreeEvent
                nevent(wxEVT_TREE_ITEM_MIDDLE_CLICK,  this, item);
            nevent.m_pointDrag = CalcScrolledPosition(pt);
            event.Skip(!GetEventHandler()->ProcessEvent(nevent));
        }
        else if ( event.LeftUp() )
        {
            if (flags & wxTREE_HITTEST_ONITEMSTATEICON)
            {
                wxTreeEvent
                    nevent(wxEVT_TREE_STATE_IMAGE_CLICK, this, item);
                GetEventHandler()->ProcessEvent(nevent);
            }

            
            if (  HasFlag(wxTR_MULTIPLE))
            {
                wxArrayTreeItemIds selections;
                size_t count = GetSelections(selections);

                if (count > 1 &&
                    !event.CmdDown() &&
                    !event.ShiftDown())
                {
                    DoSelectItem(item, true, false);
                }
            }

            if ( m_lastOnSame )
            {
                if ( (item == m_current) &&
                     (flags & wxTREE_HITTEST_ONITEMLABEL) &&
                     HasFlag(wxTR_EDIT_LABELS) )
                {
                    if ( m_renameTimer )
                    {
                        if ( m_renameTimer->IsRunning() )
                            m_renameTimer->Stop();
                    }
                    else
                    {
                        m_renameTimer = new wxTreeRenameTimer( this );
                    }

                    m_renameTimer->Start( wxTreeRenameTimer::DELAY, true );
                }

                m_lastOnSame = false;
            }
        }
        else         {
                        if ( event.LeftDown() )
            {
                                                                                                                                m_lastOnSame = item == m_current && HasFocus();
            }

            if ( flags & wxTREE_HITTEST_ONITEMBUTTON )
            {
                                                if ( event.LeftDown() )
                {
                    Toggle( item );
                }

                                return;
            }


                                                                        if (!IsSelected(item) || event.CmdDown())
            {
                                bool is_multiple, extended_select, unselect_others;
                EventFlagsToSelType(GetWindowStyleFlag(),
                                    event.ShiftDown(),
                                    event.CmdDown(),
                                    is_multiple,
                                    extended_select,
                                    unselect_others);

                DoSelectItem(item, unselect_others, extended_select);
            }


                                    if ( event.LeftDClick() )
            {
                                if ( m_renameTimer )
                    m_renameTimer->Stop();

                m_lastOnSame = false;

                                wxTreeEvent
                    nevent(wxEVT_TREE_ITEM_ACTIVATED,  this, item);
                nevent.m_pointDrag = CalcScrolledPosition(pt);
                if ( !GetEventHandler()->ProcessEvent( nevent ) )
                {
                                                                                if ( item->HasPlus() )
                    {
                        Toggle(item);
                    }
                }
            }
        }
    }
}

void wxGenericTreeCtrl::OnInternalIdle()
{
    wxWindow::OnInternalIdle();

                    if (!HasFlag(wxTR_MULTIPLE) && !GetSelection().IsOk())
    {
        if (m_select_me)
            SelectItem(m_select_me);
        else if (GetRootItem().IsOk())
            SelectItem(GetRootItem());
    }

            if (m_dirty)
        DoDirtyProcessing();
}

void
wxGenericTreeCtrl::CalculateLevel(wxGenericTreeItem *item,
                                  wxDC &dc,
                                  int level,
                                  int &y )
{
    int x = level*m_indent;
    if (!HasFlag(wxTR_HIDE_ROOT))
    {
        x += m_indent;
    }
    else if (level == 0)
    {
                        goto Recurse;
    }

    item->CalculateSize(this, dc);

        item->SetX( x+m_spacing );
    item->SetY( y );
    y += GetLineHeight(item);

    if ( !item->IsExpanded() )
    {
                return;
    }

  Recurse:
    wxArrayGenericTreeItems& children = item->GetChildren();
    size_t n, count = children.GetCount();
    ++level;
    for (n = 0; n < count; ++n )
        CalculateLevel( children[n], dc, level, y );  }

void wxGenericTreeCtrl::CalculatePositions()
{
    if ( !m_anchor ) return;

    wxClientDC dc(this);
    PrepareDC( dc );

    dc.SetFont( m_normalFont );

    dc.SetPen( m_dottedPen );
        
    int y = 2;
    CalculateLevel( m_anchor, dc, 0, y ); }

void wxGenericTreeCtrl::Refresh(bool eraseBackground, const wxRect *rect)
{
    if ( !IsFrozen() )
        wxTreeCtrlBase::Refresh(eraseBackground, rect);
}

void wxGenericTreeCtrl::RefreshSubtree(wxGenericTreeItem *item)
{
    if (m_dirty || IsFrozen() )
        return;

    wxSize client = GetClientSize();

    wxRect rect;
    CalcScrolledPosition(0, item->GetY(), NULL, &rect.y);
    rect.width = client.x;
    rect.height = client.y;

    Refresh(true, &rect);

    AdjustMyScrollbars();
}

void wxGenericTreeCtrl::RefreshLine( wxGenericTreeItem *item )
{
    if (m_dirty || IsFrozen() )
        return;

    wxRect rect;
    CalcScrolledPosition(0, item->GetY(), NULL, &rect.y);
    rect.width = GetClientSize().x;
    rect.height = GetLineHeight(item); 
    Refresh(true, &rect);
}

void wxGenericTreeCtrl::RefreshSelected()
{
    if (IsFrozen())
        return;

            if ( m_anchor )
        RefreshSelectedUnder(m_anchor);
}

void wxGenericTreeCtrl::RefreshSelectedUnder(wxGenericTreeItem *item)
{
    if (IsFrozen())
        return;

    if ( item->IsSelected() )
        RefreshLine(item);

    const wxArrayGenericTreeItems& children = item->GetChildren();
    size_t count = children.GetCount();
    for ( size_t n = 0; n < count; n++ )
    {
        RefreshSelectedUnder(children[n]);
    }
}

void wxGenericTreeCtrl::DoThaw()
{
    wxTreeCtrlBase::DoThaw();

    if ( m_dirty )
        DoDirtyProcessing();
    else
        Refresh();
}


bool wxGenericTreeCtrl::SetBackgroundColour(const wxColour& colour)
{
    if ( !wxWindow::SetBackgroundColour(colour) )
        return false;

    Refresh();

    return true;
}

bool wxGenericTreeCtrl::SetForegroundColour(const wxColour& colour)
{
    if ( !wxWindow::SetForegroundColour(colour) )
        return false;

    Refresh();

    return true;
}

void wxGenericTreeCtrl::OnGetToolTip( wxTreeEvent &event )
{
#if wxUSE_TOOLTIPS
    wxTreeItemId itemId = event.GetItem();
    const wxGenericTreeItem* const pItem = (wxGenericTreeItem*)itemId.m_pItem;

        if ( pItem->GetX() + pItem->GetWidth() > GetClientSize().x )
    {
                event.SetLabel(pItem->GetText());
    }
    else
#endif     {
                event.Veto();
    }
}


#define _USE_VISATTR 0

wxVisualAttributes
#if _USE_VISATTR
wxGenericTreeCtrl::GetClassDefaultAttributes(wxWindowVariant variant)
#else
wxGenericTreeCtrl::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
#endif
{
#if _USE_VISATTR
        return wxListBox::GetClassDefaultAttributes(variant);
#else
    wxVisualAttributes attr;
    attr.colFg = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT);
    attr.colBg = wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX);
    attr.font  = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    return attr;
#endif
}

void wxGenericTreeCtrl::DoDirtyProcessing()
{
    if (IsFrozen())
        return;

    m_dirty = false;

    CalculatePositions();
    Refresh();
    AdjustMyScrollbars();
}

wxSize wxGenericTreeCtrl::DoGetBestSize() const
{
                wxConstCast(this, wxGenericTreeCtrl)->CalculatePositions();

    wxSize size = wxTreeCtrlBase::DoGetBestSize();

                size.IncBy(4, 4);

            const wxSize& borderSize = GetWindowBorderSize();

    int dx = (size.x - borderSize.x) % PIXELS_PER_UNIT;
    if ( dx )
        size.x += PIXELS_PER_UNIT - dx;
    int dy = (size.y - borderSize.y) % PIXELS_PER_UNIT;
    if ( dy )
        size.y += PIXELS_PER_UNIT - dy;

        CacheBestSize(size);

    return size;
}

#endif 