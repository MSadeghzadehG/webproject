
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_DATAVIEWCTRL

#include "wx/dataview.h"

#ifdef wxUSE_GENERICDATAVIEWCTRL

#ifndef WX_PRECOMP
    #ifdef __WXMSW__
        #include "wx/msw/private.h"
        #include "wx/msw/wrapwin.h"
        #include "wx/msw/wrapcctl.h"     #endif
    #include "wx/sizer.h"
    #include "wx/log.h"
    #include "wx/dcclient.h"
    #include "wx/timer.h"
    #include "wx/settings.h"
    #include "wx/msgdlg.h"
    #include "wx/dcscreen.h"
    #include "wx/frame.h"
#endif

#include "wx/stockitem.h"
#include "wx/popupwin.h"
#include "wx/renderer.h"
#include "wx/dcbuffer.h"
#include "wx/icon.h"
#include "wx/list.h"
#include "wx/listimpl.cpp"
#include "wx/imaglist.h"
#include "wx/headerctrl.h"
#include "wx/dnd.h"
#include "wx/selstore.h"
#include "wx/stopwatch.h"
#include "wx/weakref.h"
#include "wx/generic/private/widthcalc.h"


class wxDataViewColumn;
class wxDataViewHeaderWindow;
class wxDataViewCtrl;


static const int SCROLL_UNIT_X = 15;

static const int PADDING_RIGHTLEFT = 3;

static const int EXPANDER_MARGIN = 4;

#ifdef __WXMSW__
static const int EXPANDER_OFFSET = 4;
#else
static const int EXPANDER_OFFSET = 1;
#endif


enum
{
        SortColumn_OnThaw = -3,

        SortColumn_None = -2,

        SortColumn_Default = -1
};


namespace
{

wxDataViewColumn* GetExpanderColumnOrFirstOne(wxDataViewCtrl* dataview)
{
    wxDataViewColumn* expander = dataview->GetExpanderColumn();
    if (!expander)
    {
                expander = dataview->GetColumnAt( 0 );
        dataview->SetExpanderColumn(expander);
    }

    return expander;
}

wxTextCtrl *CreateEditorTextCtrl(wxWindow *parent, const wxRect& labelRect, const wxString& value)
{
    wxTextCtrl* ctrl = new wxTextCtrl(parent, wxID_ANY, value,
                                      labelRect.GetPosition(),
                                      labelRect.GetSize(),
                                      wxTE_PROCESS_ENTER);

            const int fitting = ctrl->GetSizeFromTextSize(ctrl->GetTextExtent(ctrl->GetValue())).x;
    const int current = ctrl->GetSize().x;
    const int maxwidth = ctrl->GetParent()->GetSize().x - ctrl->GetPosition().x;

                int width = wxMin(wxMax(current, fitting), maxwidth);

    if ( width != current )
        ctrl->SetSize(wxSize(width, -1));

        ctrl->SetInsertionPointEnd();
    ctrl->SelectAll();

    return ctrl;
}

} 

void wxDataViewColumn::Init(int width, wxAlignment align, int flags)
{
    m_width = width;
    m_minWidth = 0;
    m_align = align;
    m_flags = flags;
    m_sort = false;
    m_sortAscending = true;
}

int wxDataViewColumn::GetWidth() const
{
    switch ( m_width )
    {
        case wxCOL_WIDTH_DEFAULT:
            return wxDVC_DEFAULT_WIDTH;

        case wxCOL_WIDTH_AUTOSIZE:
            wxCHECK_MSG( m_owner, wxDVC_DEFAULT_WIDTH, "no owner control" );
            return m_owner->GetBestColumnWidth(m_owner->GetColumnIndex(this));

        default:
            return m_width;
    }
}

void wxDataViewColumn::UpdateDisplay()
{
    if (m_owner)
    {
        int idx = m_owner->GetColumnIndex( this );
        m_owner->OnColumnChange( idx );
    }
}

void wxDataViewColumn::UnsetAsSortKey()
{
    m_sort = false;

    if ( m_owner )
        m_owner->DontUseColumnForSorting(m_owner->GetColumnIndex(this));

    UpdateDisplay();
}

void wxDataViewColumn::SetSortOrder(bool ascending)
{
    if ( !m_owner )
        return;

    const int idx = m_owner->GetColumnIndex(this);

        if ( !m_sort )
    {
        wxASSERT(!m_owner->IsColumnSorted(idx));

                m_owner->UseColumnForSorting(idx);
        m_sort = true;
    }

   m_sortAscending = ascending;

            m_owner->OnColumnChange(idx);
}


class wxDataViewHeaderWindow : public wxHeaderCtrl
{
public:
    wxDataViewHeaderWindow(wxDataViewCtrl *parent)
        : wxHeaderCtrl(parent)
    {
    }

    wxDataViewCtrl *GetOwner() const
        { return static_cast<wxDataViewCtrl *>(GetParent()); }

        void ToggleSortByColumn(int column)
    {
        wxDataViewCtrl * const owner = GetOwner();

        if ( !owner->IsMultiColumnSortAllowed() )
            return;

        wxDataViewColumn * const col = owner->GetColumn(column);
        if ( !col->IsSortable() )
            return;

        if ( owner->IsColumnSorted(column) )
        {
            col->UnsetAsSortKey();
            SendEvent(wxEVT_DATAVIEW_COLUMN_SORTED, column);
        }
        else         {
            col->SetSortOrder(true);
            SendEvent(wxEVT_DATAVIEW_COLUMN_SORTED, column);
        }
    }

protected:
            virtual const wxHeaderColumn& GetColumn(unsigned int idx) const
    {
        return *(GetOwner()->GetColumn(idx));
    }

    virtual bool UpdateColumnWidthToFit(unsigned int idx, int widthTitle)
    {
        wxDataViewCtrl * const owner = GetOwner();

        int widthContents = owner->GetBestColumnWidth(idx);
        owner->GetColumn(idx)->SetWidth(wxMax(widthTitle, widthContents));
        owner->OnColumnChange(idx);

        return true;
    }

private:
    void FinishEditing();

    bool SendEvent(wxEventType type, unsigned int n)
    {
        wxDataViewCtrl * const owner = GetOwner();
        wxDataViewEvent event(type, owner->GetId());

        event.SetEventObject(owner);
        event.SetColumn(n);
        event.SetDataViewColumn(owner->GetColumn(n));
        event.SetModel(owner->GetModel());

                        return owner->ProcessWindowEvent(event);
    }

    void OnClick(wxHeaderCtrlEvent& event)
    {
        FinishEditing();

        const unsigned idx = event.GetColumn();

        if ( SendEvent(wxEVT_DATAVIEW_COLUMN_HEADER_CLICK, idx) )
            return;

                        wxDataViewCtrl * const owner = GetOwner();
        wxDataViewColumn * const col = owner->GetColumn(idx);
        if ( !col->IsSortable() )
        {
                        event.Skip();
            return;
        }

        if ( col->IsSortKey() )
        {
                        col->ToggleSortOrder();
        }
        else         {
                                    owner->ResetAllSortColumns();

                        col->SetSortOrder(true);
        }

        wxDataViewModel * const model = owner->GetModel();
        if ( model )
            model->Resort();

        owner->OnColumnChange(idx);

        SendEvent(wxEVT_DATAVIEW_COLUMN_SORTED, idx);
    }

    void OnRClick(wxHeaderCtrlEvent& event)
    {
                if ( !SendEvent(wxEVT_DATAVIEW_COLUMN_HEADER_RIGHT_CLICK,
                        event.GetColumn()) )
        {
            event.Skip();
            ToggleSortByColumn(event.GetColumn());
        }
    }

    void OnResize(wxHeaderCtrlEvent& event)
    {
        FinishEditing();

        wxDataViewCtrl * const owner = GetOwner();

        const unsigned col = event.GetColumn();
        owner->GetColumn(col)->SetWidth(event.GetWidth());
        GetOwner()->OnColumnChange(col);
    }

    void OnEndReorder(wxHeaderCtrlEvent& event)
    {
        FinishEditing();

        wxDataViewCtrl * const owner = GetOwner();
        owner->ColumnMoved(owner->GetColumn(event.GetColumn()),
                        event.GetNewOrder());
    }

    wxDECLARE_EVENT_TABLE();
    wxDECLARE_NO_COPY_CLASS(wxDataViewHeaderWindow);
};

wxBEGIN_EVENT_TABLE(wxDataViewHeaderWindow, wxHeaderCtrl)
    EVT_HEADER_CLICK(wxID_ANY, wxDataViewHeaderWindow::OnClick)
    EVT_HEADER_RIGHT_CLICK(wxID_ANY, wxDataViewHeaderWindow::OnRClick)

    EVT_HEADER_RESIZING(wxID_ANY, wxDataViewHeaderWindow::OnResize)
    EVT_HEADER_END_RESIZE(wxID_ANY, wxDataViewHeaderWindow::OnResize)

    EVT_HEADER_END_REORDER(wxID_ANY, wxDataViewHeaderWindow::OnEndReorder)
wxEND_EVENT_TABLE()


class wxDataViewRenameTimer: public wxTimer
{
private:
    wxDataViewMainWindow *m_owner;

public:
    wxDataViewRenameTimer( wxDataViewMainWindow *owner );
    void Notify();
};


class wxDataViewMainWindow;
class wxDataViewTreeNode;
WX_DEFINE_ARRAY( wxDataViewTreeNode *, wxDataViewTreeNodes );

class wxDataViewTreeNode
{
public:
    wxDataViewTreeNode(wxDataViewMainWindow *window,
                       wxDataViewTreeNode *parent,
                       const wxDataViewItem& item)
        : m_window(window),
          m_parent(parent),
          m_item(item),
          m_branchData(NULL)
    {
    }

    ~wxDataViewTreeNode()
    {
        if ( m_branchData )
        {
            wxDataViewTreeNodes& nodes = m_branchData->children;
            for ( wxDataViewTreeNodes::iterator i = nodes.begin();
                  i != nodes.end();
                  ++i )
            {
                delete *i;
            }

            delete m_branchData;
        }
    }

    static wxDataViewTreeNode* CreateRootNode(wxDataViewMainWindow *w)
    {
        wxDataViewTreeNode *n = new wxDataViewTreeNode(w, NULL, wxDataViewItem());
        n->m_branchData = new BranchNodeData;
        n->m_branchData->open = true;
        return n;
    }

    wxDataViewTreeNode * GetParent() const { return m_parent; }

    const wxDataViewTreeNodes& GetChildNodes() const
    {
        wxASSERT( m_branchData != NULL );
        return m_branchData->children;
    }

    void InsertChild(wxDataViewTreeNode *node, unsigned index);

    void RemoveChild(wxDataViewTreeNode *node)
    {
        wxCHECK_RET( m_branchData != NULL, "leaf node doesn't have children" );
        m_branchData->children.Remove(node);
    }

        int FindChildByItem(const wxDataViewItem& item) const
    {
        if ( !m_branchData )
            return wxNOT_FOUND;

        const wxDataViewTreeNodes& nodes = m_branchData->children;
        const int len = nodes.size();
        for ( int i = 0; i < len; i++ )
        {
            if ( nodes[i]->m_item == item )
                return i;
        }
        return wxNOT_FOUND;
    }

    const wxDataViewItem & GetItem() const { return m_item; }
    void SetItem( const wxDataViewItem & item ) { m_item = item; }

    int GetIndentLevel() const
    {
        int ret = 0;
        const wxDataViewTreeNode * node = this;
        while( node->GetParent()->GetParent() != NULL )
        {
            node = node->GetParent();
            ret ++;
        }
        return ret;
    }

    bool IsOpen() const
    {
        return m_branchData && m_branchData->open;
    }

    void ToggleOpen()
    {
                        if ( !m_parent )
            return;

        wxCHECK_RET( m_branchData != NULL, "can't open leaf node" );

        int sum = 0;

        const wxDataViewTreeNodes& nodes = m_branchData->children;
        const int len = nodes.GetCount();
        for ( int i = 0;i < len; i ++)
            sum += 1 + nodes[i]->GetSubTreeCount();

        if (m_branchData->open)
        {
            ChangeSubTreeCount(-sum);
            m_branchData->open = !m_branchData->open;
        }
        else
        {
            m_branchData->open = !m_branchData->open;
            ChangeSubTreeCount(+sum);
        }
    }

            bool HasChildren() const
    {
        return m_branchData != NULL;
    }

    void SetHasChildren(bool has)
    {
                        if ( !m_parent )
            return;

        if ( !has )
        {
            wxDELETE(m_branchData);
        }
        else if ( m_branchData == NULL )
        {
            m_branchData = new BranchNodeData;
        }
    }

    int GetSubTreeCount() const
    {
        return m_branchData ? m_branchData->subTreeCount : 0;
    }

    void ChangeSubTreeCount( int num )
    {
        wxASSERT( m_branchData != NULL );

        if( !m_branchData->open )
            return;

        m_branchData->subTreeCount += num;
        wxASSERT( m_branchData->subTreeCount >= 0 );

        if( m_parent )
            m_parent->ChangeSubTreeCount(num);
    }

    void Resort();

private:
    wxDataViewMainWindow * const m_window;
    wxDataViewTreeNode  *m_parent;

        wxDataViewItem       m_item;

            struct BranchNodeData
    {
        BranchNodeData()
            : open(false),
              subTreeCount(0)
        {
        }

                        wxDataViewTreeNodes  children;

                bool                 open;

                                        int                  subTreeCount;
    };

    BranchNodeData *m_branchData;
};




WX_DEFINE_SORTED_ARRAY_SIZE_T(unsigned int, wxDataViewSelection);

class wxDataViewMainWindow: public wxWindow
{
public:
    wxDataViewMainWindow( wxDataViewCtrl *parent,
                            wxWindowID id,
                            const wxPoint &pos = wxDefaultPosition,
                            const wxSize &size = wxDefaultSize,
                            const wxString &name = wxT("wxdataviewctrlmainwindow") );
    virtual ~wxDataViewMainWindow();

    bool IsList() const { return GetModel()->IsListModel(); }
    bool IsVirtualList() const { return m_root == NULL; }

        bool ItemAdded( const wxDataViewItem &parent, const wxDataViewItem &item );
    bool ItemDeleted( const wxDataViewItem &parent, const wxDataViewItem &item );
    bool ItemChanged( const wxDataViewItem &item );
    bool ValueChanged( const wxDataViewItem &item, unsigned int model_column );
    bool Cleared();
    void Resort()
    {
        if (!IsVirtualList())
        {
            SortPrepare();
            m_root->Resort();
        }
        UpdateDisplay();
    }

            virtual void DoThaw()
    {
        if ( m_sortColumn == SortColumn_OnThaw )
        {
            Resort();
            m_sortColumn = SortColumn_None;
        }

        wxWindow::DoThaw();
    }

    void SortPrepare()
    {
        wxDataViewModel* model = GetModel();

        wxDataViewColumn* col = GetOwner()->GetSortingColumn();
        if( !col )
        {
            if (model->HasDefaultCompare())
            {
                                if ( IsFrozen() )
                    m_sortColumn = SortColumn_OnThaw;
                else
                    m_sortColumn = SortColumn_Default;
            }
            else
                m_sortColumn = SortColumn_None;

            m_sortAscending = true;
            return;
        }

                                if ( IsFrozen() )
        {
            m_sortColumn = SortColumn_OnThaw;
            return;
        }

        m_sortColumn = col->GetModelColumn();
        m_sortAscending = col->IsSortOrderAscending();
    }

    void SetOwner( wxDataViewCtrl* owner ) { m_owner = owner; }
    wxDataViewCtrl *GetOwner() { return m_owner; }
    const wxDataViewCtrl *GetOwner() const { return m_owner; }

    wxDataViewModel* GetModel() { return GetOwner()->GetModel(); }
    const wxDataViewModel* GetModel() const { return GetOwner()->GetModel(); }

#if wxUSE_DRAG_AND_DROP
    wxBitmap CreateItemBitmap( unsigned int row, int &indent );
#endif     void OnPaint( wxPaintEvent &event );
    void OnCharHook( wxKeyEvent &event );
    void OnChar( wxKeyEvent &event );
    void OnVerticalNavigation(const wxKeyEvent& event, int delta);
    void OnLeftKey(wxKeyEvent& event);
    void OnRightKey(wxKeyEvent& event);
    void OnMouse( wxMouseEvent &event );
    void OnSetFocus( wxFocusEvent &event );
    void OnKillFocus( wxFocusEvent &event );

    void UpdateDisplay();
    void RecalculateDisplay();
    void OnInternalIdle();

    void OnRenameTimer();

    void ScrollWindow( int dx, int dy, const wxRect *rect = NULL );
    void ScrollTo( int rows, int column );

    unsigned GetCurrentRow() const { return m_currentRow; }
    bool HasCurrentRow() { return m_currentRow != (unsigned int)-1; }
    void ChangeCurrentRow( unsigned int row );
    bool TryAdvanceCurrentColumn(wxDataViewTreeNode *node, wxKeyEvent& event, bool forward);

    wxDataViewColumn *GetCurrentColumn() const { return m_currentCol; }
    void ClearCurrentColumn() { m_currentCol = NULL; }

    bool IsSingleSel() const { return !GetParent()->HasFlag(wxDV_MULTIPLE); }
    bool IsEmpty() { return GetRowCount() == 0; }

    int GetCountPerPage() const;
    int GetEndOfLastCol() const;
    unsigned int GetFirstVisibleRow() const;

                unsigned int GetLastVisibleRow();
    unsigned int GetRowCount() const;

    const wxSelectionStore& GetSelections() const { return m_selection; }
    void ClearSelection() { m_selection.SelectRange(0, GetRowCount() - 1, false); }
    void Select( const wxArrayInt& aSelections );

    void SelectAllRows()
    {
        m_selection.SelectRange(0, GetRowCount() - 1);
        Refresh();
    }

                bool UnselectAllRows(unsigned int except = (unsigned int)-1);

    void SelectRow( unsigned int row, bool on );
    void SelectRows( unsigned int from, unsigned int to );
    void ReverseRowSelection( unsigned int row );
    bool IsRowSelected( unsigned int row );
    void SendSelectionChangedEvent( const wxDataViewItem& item);

    void RefreshRow( unsigned int row );
    void RefreshRows( unsigned int from, unsigned int to );
    void RefreshRowsAfter( unsigned int firstRow );

        wxColour GetRuleColour() const
    {
        return wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT);
    }

    wxRect GetLineRect( unsigned int row ) const;

    int GetLineStart( unsigned int row ) const;      int GetLineHeight( unsigned int row ) const;     int GetLineAt( unsigned int y ) const;       
    void SetRowHeight( int lineHeight ) { m_lineHeight = lineHeight; }
    int GetRowHeight() const { return m_lineHeight; }
    int GetDefaultRowHeight() const;

        wxDataViewItem GetItemByRow( unsigned int row ) const;
    int GetRowByItem( const wxDataViewItem & item ) const;

    wxDataViewTreeNode * GetTreeNodeByRow( unsigned int row ) const;
        
        void BuildTree( wxDataViewModel  * model );
    void DestroyTree();
    void HitTest( const wxPoint & point, wxDataViewItem & item, wxDataViewColumn* &column );
    wxRect GetItemRect( const wxDataViewItem & item, const wxDataViewColumn* column );

    void Expand( unsigned int row );
    void Collapse( unsigned int row );
    bool IsExpanded( unsigned int row ) const;
    bool HasChildren( unsigned int row ) const;

#if wxUSE_DRAG_AND_DROP
    bool EnableDragSource( const wxDataFormat &format );
    bool EnableDropTarget( const wxDataFormat &format );

    void RemoveDropHint();
    wxDragResult OnDragOver( wxDataFormat format, wxCoord x, wxCoord y, wxDragResult def );
    bool OnDrop( wxDataFormat format, wxCoord x, wxCoord y );
    wxDragResult OnData( wxDataFormat format, wxCoord x, wxCoord y, wxDragResult def );
    void OnLeave();
#endif 
    void OnColumnsCountChanged();

        void UpdateColumnSizes();

            void StartEditing(const wxDataViewItem& item, const wxDataViewColumn* col);
    void FinishEditing();

    int GetSortColumn() const { return m_sortColumn; }
    bool IsAscendingSort() const { return m_sortAscending; }

private:
    void InvalidateCount() { m_count = -1; }
    void UpdateCount(int count)
    {
        m_count = count;
        m_selection.SetItemCount(count);
    }

    int RecalculateCount() const;

        bool SendExpanderEvent(wxEventType type, const wxDataViewItem& item);

    wxDataViewTreeNode * FindNode( const wxDataViewItem & item );

    wxDataViewColumn *FindColumnForEditing(const wxDataViewItem& item, wxDataViewCellMode mode);

    bool IsCellEditableInMode(const wxDataViewItem& item, const wxDataViewColumn *col, wxDataViewCellMode mode) const;

    void DrawCellBackground( wxDataViewRenderer* cell, wxDC& dc, const wxRect& rect );

private:
    wxDataViewCtrl             *m_owner;
    int                         m_lineHeight;
    bool                        m_dirty;

    wxDataViewColumn           *m_currentCol;
    unsigned int                m_currentRow;
    wxSelectionStore            m_selection;

    wxDataViewRenameTimer      *m_renameTimer;
    bool                        m_lastOnSame;

    bool                        m_hasFocus;
    bool                        m_useCellFocus;
    bool                        m_currentColSetByKeyboard;

#if wxUSE_DRAG_AND_DROP
    int                         m_dragCount;
    wxPoint                     m_dragStart;

    bool                        m_dragEnabled;
    wxDataFormat                m_dragFormat;

    bool                        m_dropEnabled;
    wxDataFormat                m_dropFormat;
    bool                        m_dropHint;
    unsigned int                m_dropHintLine;
#endif 
        unsigned int m_lineLastClicked,
        m_lineBeforeLastClicked,
        m_lineSelectSingleOnUp;

        wxPen m_penRule;

        wxPen m_penExpander;

        wxDataViewTreeNode * m_root;
    int m_count;

        wxDataViewTreeNode * m_underMouse;

        int m_sortColumn;
    bool m_sortAscending;

        wxWeakRef<wxWindow> m_editorCtrl;

        wxDataViewRenderer* m_editorRenderer;

private:
    wxDECLARE_DYNAMIC_CLASS(wxDataViewMainWindow);
    wxDECLARE_EVENT_TABLE();
};


class wxGenericDataViewModelNotifier: public wxDataViewModelNotifier
{
public:
    wxGenericDataViewModelNotifier( wxDataViewMainWindow *mainWindow )
        { m_mainWindow = mainWindow; }

    virtual bool ItemAdded( const wxDataViewItem & parent, const wxDataViewItem & item )
        { return m_mainWindow->ItemAdded( parent , item ); }
    virtual bool ItemDeleted( const wxDataViewItem &parent, const wxDataViewItem &item )
        { return m_mainWindow->ItemDeleted( parent, item ); }
    virtual bool ItemChanged( const wxDataViewItem & item )
        { return m_mainWindow->ItemChanged(item);  }
    virtual bool ValueChanged( const wxDataViewItem & item , unsigned int col )
        { return m_mainWindow->ValueChanged( item, col ); }
    virtual bool Cleared()
        { return m_mainWindow->Cleared(); }
    virtual void Resort()
        { m_mainWindow->Resort(); }

    wxDataViewMainWindow    *m_mainWindow;
};


wxIMPLEMENT_ABSTRACT_CLASS(wxDataViewRenderer, wxDataViewRendererBase);

wxDataViewRenderer::wxDataViewRenderer( const wxString &varianttype,
                                        wxDataViewCellMode mode,
                                        int align) :
    wxDataViewCustomRendererBase( varianttype, mode, align )
{
    m_align = align;
    m_mode = mode;
    m_ellipsizeMode = wxELLIPSIZE_MIDDLE;
    m_dc = NULL;
}

wxDataViewRenderer::~wxDataViewRenderer()
{
    delete m_dc;
}

wxDC *wxDataViewRenderer::GetDC()
{
    if (m_dc == NULL)
    {
        if (GetOwner() == NULL)
            return NULL;
        if (GetOwner()->GetOwner() == NULL)
            return NULL;
        m_dc = new wxClientDC( GetOwner()->GetOwner() );
    }

    return m_dc;
}

void wxDataViewRenderer::SetAlignment( int align )
{
    m_align=align;
}

int wxDataViewRenderer::GetAlignment() const
{
    return m_align;
}


wxIMPLEMENT_ABSTRACT_CLASS(wxDataViewCustomRenderer, wxDataViewRenderer);

wxDataViewCustomRenderer::wxDataViewCustomRenderer( const wxString &varianttype,
                        wxDataViewCellMode mode, int align ) :
    wxDataViewRenderer( varianttype, mode, align )
{
}


wxIMPLEMENT_CLASS(wxDataViewTextRenderer, wxDataViewRenderer);

wxDataViewTextRenderer::wxDataViewTextRenderer( const wxString &varianttype,
                                                wxDataViewCellMode mode, int align ) :
    wxDataViewRenderer( varianttype, mode, align )
{
}

bool wxDataViewTextRenderer::SetValue( const wxVariant &value )
{
    m_text = value.GetString();

    return true;
}

bool wxDataViewTextRenderer::GetValue( wxVariant& WXUNUSED(value) ) const
{
    return false;
}

bool wxDataViewTextRenderer::HasEditorCtrl() const
{
    return true;
}

wxWindow* wxDataViewTextRenderer::CreateEditorCtrl( wxWindow *parent,
        wxRect labelRect, const wxVariant &value )
{
    return CreateEditorTextCtrl(parent, labelRect, value);
}

bool wxDataViewTextRenderer::GetValueFromEditorCtrl( wxWindow *editor, wxVariant &value )
{
    wxTextCtrl *text = (wxTextCtrl*) editor;
    value = text->GetValue();
    return true;
}

bool wxDataViewTextRenderer::Render(wxRect rect, wxDC *dc, int state)
{
    RenderText(m_text, 0, rect, dc, state);
    return true;
}

wxSize wxDataViewTextRenderer::GetSize() const
{
    if (!m_text.empty())
        return GetTextExtent(m_text);
    else
        return wxSize(wxDVC_DEFAULT_RENDERER_SIZE,wxDVC_DEFAULT_RENDERER_SIZE);
}


wxIMPLEMENT_CLASS(wxDataViewBitmapRenderer, wxDataViewRenderer);

wxDataViewBitmapRenderer::wxDataViewBitmapRenderer( const wxString &varianttype,
                                                    wxDataViewCellMode mode, int align ) :
    wxDataViewRenderer( varianttype, mode, align )
{
}

bool wxDataViewBitmapRenderer::SetValue( const wxVariant &value )
{
    if (value.GetType() == wxT("wxBitmap"))
        m_bitmap << value;
    if (value.GetType() == wxT("wxIcon"))
        m_icon << value;

    return true;
}

bool wxDataViewBitmapRenderer::GetValue( wxVariant& WXUNUSED(value) ) const
{
    return false;
}

bool wxDataViewBitmapRenderer::Render( wxRect cell, wxDC *dc, int WXUNUSED(state) )
{
    if (m_bitmap.IsOk())
        dc->DrawBitmap( m_bitmap, cell.x, cell.y, true  );
    else if (m_icon.IsOk())
        dc->DrawIcon( m_icon, cell.x, cell.y );

    return true;
}

wxSize wxDataViewBitmapRenderer::GetSize() const
{
    if (m_bitmap.IsOk())
        return wxSize( m_bitmap.GetWidth(), m_bitmap.GetHeight() );
    else if (m_icon.IsOk())
        return wxSize( m_icon.GetWidth(), m_icon.GetHeight() );

    return wxSize(wxDVC_DEFAULT_RENDERER_SIZE,wxDVC_DEFAULT_RENDERER_SIZE);
}


wxIMPLEMENT_ABSTRACT_CLASS(wxDataViewToggleRenderer, wxDataViewRenderer);

wxDataViewToggleRenderer::wxDataViewToggleRenderer( const wxString &varianttype,
                        wxDataViewCellMode mode, int align ) :
    wxDataViewRenderer( varianttype, mode, align )
{
    m_toggle = false;
}

bool wxDataViewToggleRenderer::SetValue( const wxVariant &value )
{
    m_toggle = value.GetBool();

    return true;
}

bool wxDataViewToggleRenderer::GetValue( wxVariant &WXUNUSED(value) ) const
{
    return false;
}

bool wxDataViewToggleRenderer::Render( wxRect cell, wxDC *dc, int WXUNUSED(state) )
{
    int flags = 0;
    if (m_toggle)
        flags |= wxCONTROL_CHECKED;
    if (GetMode() != wxDATAVIEW_CELL_ACTIVATABLE ||
        GetEnabled() == false)
        flags |= wxCONTROL_DISABLED;

                    wxSize size = cell.GetSize();
    size.IncTo(GetSize());
    cell.SetSize(size);

    wxRendererNative::Get().DrawCheckBox(
            GetOwner()->GetOwner(),
            *dc,
            cell,
            flags );

    return true;
}

bool wxDataViewToggleRenderer::WXActivateCell(const wxRect& WXUNUSED(cellRect),
                                              wxDataViewModel *model,
                                              const wxDataViewItem& item,
                                              unsigned int col,
                                              const wxMouseEvent *mouseEvent)
{
    if ( mouseEvent )
    {
                        if ( !wxRect(GetSize()).Contains(mouseEvent->GetPosition()) )
            return false;
    }

    model->ChangeValue(!m_toggle, item, col);
    return true;
}

wxSize wxDataViewToggleRenderer::GetSize() const
{
    return wxRendererNative::Get().GetCheckBoxSize(GetView());
}


wxIMPLEMENT_ABSTRACT_CLASS(wxDataViewProgressRenderer, wxDataViewRenderer);

wxDataViewProgressRenderer::wxDataViewProgressRenderer( const wxString &label,
    const wxString &varianttype, wxDataViewCellMode mode, int align ) :
    wxDataViewRenderer( varianttype, mode, align )
{
    m_label = label;
    m_value = 0;
}

bool wxDataViewProgressRenderer::SetValue( const wxVariant &value )
{
    m_value = (long) value;

    if (m_value < 0) m_value = 0;
    if (m_value > 100) m_value = 100;

    return true;
}

bool wxDataViewProgressRenderer::GetValue( wxVariant &value ) const
{
    value = (long) m_value;
    return true;
}

bool
wxDataViewProgressRenderer::Render(wxRect rect, wxDC *dc, int WXUNUSED(state))
{
    wxRendererNative::Get().DrawGauge(
        GetOwner()->GetOwner(),
        *dc,
        rect,
        m_value,
        100);

    return true;
}

wxSize wxDataViewProgressRenderer::GetSize() const
{
                    return wxSize(-1, 12);
}


wxIMPLEMENT_CLASS(wxDataViewIconTextRenderer, wxDataViewRenderer);

wxDataViewIconTextRenderer::wxDataViewIconTextRenderer(
const wxString &varianttype, wxDataViewCellMode mode, int align ) :
    wxDataViewRenderer( varianttype, mode, align )
{
    SetMode(mode);
    SetAlignment(align);
}

bool wxDataViewIconTextRenderer::SetValue( const wxVariant &value )
{
    m_value << value;
    return true;
}

bool wxDataViewIconTextRenderer::GetValue( wxVariant& WXUNUSED(value) ) const
{
    return false;
}

bool wxDataViewIconTextRenderer::Render(wxRect rect, wxDC *dc, int state)
{
    int xoffset = 0;

    const wxIcon& icon = m_value.GetIcon();
    if ( icon.IsOk() )
    {
        dc->DrawIcon(icon, rect.x, rect.y + (rect.height - icon.GetHeight())/2);
        xoffset = icon.GetWidth()+4;
    }

    RenderText(m_value.GetText(), xoffset, rect, dc, state);

    return true;
}

wxSize wxDataViewIconTextRenderer::GetSize() const
{
    if (!m_value.GetText().empty())
    {
        wxSize size = GetTextExtent(m_value.GetText());

        if (m_value.GetIcon().IsOk())
            size.x += m_value.GetIcon().GetWidth() + 4;
        return size;
    }
    return wxSize(80,20);
}

wxWindow* wxDataViewIconTextRenderer::CreateEditorCtrl(wxWindow *parent, wxRect labelRect, const wxVariant& value)
{
    wxDataViewIconText iconText;
    iconText << value;

    wxString text = iconText.GetText();

        if (iconText.GetIcon().IsOk())
    {
        int w = iconText.GetIcon().GetWidth() + 4;
        labelRect.x += w;
        labelRect.width -= w;
    }

    return CreateEditorTextCtrl(parent, labelRect, text);
}

bool wxDataViewIconTextRenderer::GetValueFromEditorCtrl( wxWindow *editor, wxVariant& value )
{
    wxTextCtrl *text = (wxTextCtrl*) editor;

        wxVariant valueOld;
    wxDataViewColumn* const col = GetOwner();
    GetView()->GetModel()->GetValue(valueOld, m_item, col->GetModelColumn());

    wxDataViewIconText iconText;
    iconText << valueOld;

        iconText.SetText(text->GetValue());

    value << iconText;
    return true;
}


#if wxUSE_DRAG_AND_DROP

class wxBitmapCanvas: public wxWindow
{
public:
    wxBitmapCanvas( wxWindow *parent, const wxBitmap &bitmap, const wxSize &size ) :
    wxWindow( parent, wxID_ANY, wxPoint(0,0), size )
    {
        m_bitmap = bitmap;
        Connect( wxEVT_PAINT, wxPaintEventHandler(wxBitmapCanvas::OnPaint) );
    }

    void OnPaint( wxPaintEvent &WXUNUSED(event) )
    {
        wxPaintDC dc(this);
        dc.DrawBitmap( m_bitmap, 0, 0);
    }

    wxBitmap m_bitmap;
};

class wxDataViewDropSource: public wxDropSource
{
public:
    wxDataViewDropSource( wxDataViewMainWindow *win, unsigned int row ) :
        wxDropSource( win )
    {
        m_win = win;
        m_row = row;
        m_hint = NULL;
    }

    ~wxDataViewDropSource()
    {
        delete m_hint;
    }

    virtual bool GiveFeedback( wxDragResult WXUNUSED(effect) )
    {
        wxPoint pos = wxGetMousePosition();

        if (!m_hint)
        {
            int liney = m_win->GetLineStart( m_row );
            int linex = 0;
            m_win->GetOwner()->CalcUnscrolledPosition( 0, liney, NULL, &liney );
            m_win->ClientToScreen( &linex, &liney );
            m_dist_x = pos.x - linex;
            m_dist_y = pos.y - liney;

            int indent = 0;
            wxBitmap ib = m_win->CreateItemBitmap( m_row, indent );
            m_dist_x -= indent;
            m_hint = new wxFrame( m_win->GetParent(), wxID_ANY, wxEmptyString,
                                        wxPoint(pos.x - m_dist_x, pos.y + 5 ),
                                        ib.GetSize(),
                                        wxFRAME_TOOL_WINDOW |
                                        wxFRAME_FLOAT_ON_PARENT |
                                        wxFRAME_NO_TASKBAR |
                                        wxNO_BORDER );
            new wxBitmapCanvas( m_hint, ib, ib.GetSize() );
            m_hint->Show();
        }
        else
        {
            m_hint->Move( pos.x - m_dist_x, pos.y + 5  );
            m_hint->SetTransparent( 128 );
        }

        return false;
    }

    wxDataViewMainWindow   *m_win;
    unsigned int            m_row;
    wxFrame                *m_hint;
    int m_dist_x,m_dist_y;
};


class wxDataViewDropTarget: public wxDropTarget
{
public:
    wxDataViewDropTarget( wxDataObject *obj, wxDataViewMainWindow *win ) :
        wxDropTarget( obj )
    {
        m_win = win;
    }

    virtual wxDragResult OnDragOver( wxCoord x, wxCoord y, wxDragResult def )
    {
        wxDataFormat format = GetMatchingPair();
        if (format == wxDF_INVALID)
            return wxDragNone;
        return m_win->OnDragOver( format, x, y, def);
    }

    virtual bool OnDrop( wxCoord x, wxCoord y )
    {
        wxDataFormat format = GetMatchingPair();
        if (format == wxDF_INVALID)
            return false;
        return m_win->OnDrop( format, x, y );
    }

    virtual wxDragResult OnData( wxCoord x, wxCoord y, wxDragResult def )
    {
        wxDataFormat format = GetMatchingPair();
        if (format == wxDF_INVALID)
            return wxDragNone;
        if (!GetData())
            return wxDragNone;
        return m_win->OnData( format, x, y, def );
    }

    virtual void OnLeave()
        { m_win->OnLeave(); }

    wxDataViewMainWindow   *m_win;
};

#endif 

wxDataViewRenameTimer::wxDataViewRenameTimer( wxDataViewMainWindow *owner )
{
    m_owner = owner;
}

void wxDataViewRenameTimer::Notify()
{
    m_owner->OnRenameTimer();
}




static wxDataViewModel* g_model;
static int g_column;
static bool g_asending;

int LINKAGEMODE wxGenericTreeModelNodeCmp(wxDataViewTreeNode ** node1,
    wxDataViewTreeNode ** node2)
{
    return g_model->Compare((*node1)->GetItem(), (*node2)->GetItem(), g_column, g_asending);
}


void wxDataViewTreeNode::InsertChild(wxDataViewTreeNode *node, unsigned index)
{
    if (!m_branchData)
        m_branchData = new BranchNodeData;

    m_branchData->children.Insert(node, index);

        if ((g_column = m_window->GetSortColumn()) >= -1)
    {
        g_model = m_window->GetModel();
        g_asending = m_window->IsAscendingSort();

        m_branchData->children.Sort(&wxGenericTreeModelNodeCmp);
    }
}

void wxDataViewTreeNode::Resort()
{
    if (!m_branchData)
        return;

    if ((g_column = m_window->GetSortColumn()) >= -1)
    {
        wxDataViewTreeNodes& nodes = m_branchData->children;

        g_model = m_window->GetModel();
        g_asending = m_window->IsAscendingSort();
        nodes.Sort(&wxGenericTreeModelNodeCmp);
        int len = nodes.GetCount();
        for (int i = 0; i < len; i++)
        {
            if (nodes[i]->HasChildren())
                nodes[i]->Resort();
        }
    }
}


static void BuildTreeHelper(wxDataViewMainWindow *window,
                            const wxDataViewModel *model,
                            const wxDataViewItem & item,
                            wxDataViewTreeNode * node);

wxIMPLEMENT_ABSTRACT_CLASS(wxDataViewMainWindow, wxWindow);

wxBEGIN_EVENT_TABLE(wxDataViewMainWindow,wxWindow)
    EVT_PAINT         (wxDataViewMainWindow::OnPaint)
    EVT_MOUSE_EVENTS  (wxDataViewMainWindow::OnMouse)
    EVT_SET_FOCUS     (wxDataViewMainWindow::OnSetFocus)
    EVT_KILL_FOCUS    (wxDataViewMainWindow::OnKillFocus)
    EVT_CHAR_HOOK     (wxDataViewMainWindow::OnCharHook)
    EVT_CHAR          (wxDataViewMainWindow::OnChar)
wxEND_EVENT_TABLE()

wxDataViewMainWindow::wxDataViewMainWindow( wxDataViewCtrl *parent, wxWindowID id,
    const wxPoint &pos, const wxSize &size, const wxString &name ) :
    wxWindow( parent, id, pos, size, wxWANTS_CHARS|wxBORDER_NONE, name )
{
    SetOwner( parent );

    m_editorRenderer = NULL;

    m_lastOnSame = false;
    m_renameTimer = new wxDataViewRenameTimer( this );

        m_currentCol = NULL;
    m_currentColSetByKeyboard = false;
    m_useCellFocus = false;
    m_currentRow = (unsigned)-1;
    m_lineHeight = GetDefaultRowHeight();

#if wxUSE_DRAG_AND_DROP
    m_dragCount = 0;
    m_dragStart = wxPoint(0,0);

    m_dragEnabled = false;
    m_dropEnabled = false;
    m_dropHint = false;
    m_dropHintLine = (unsigned int) -1;
#endif 
    m_lineLastClicked = (unsigned int) -1;
    m_lineBeforeLastClicked = (unsigned int) -1;
    m_lineSelectSingleOnUp = (unsigned int) -1;

    m_hasFocus = false;

    SetBackgroundColour( *wxWHITE );

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    m_penRule = wxPen(GetRuleColour());

            m_penExpander = wxPen(wxColour(0,0,0));

    m_root = wxDataViewTreeNode::CreateRootNode(this);

        m_count = -1;
    m_underMouse = NULL;

    m_sortColumn = SortColumn_None;
    m_sortAscending = true;

    UpdateDisplay();
}

wxDataViewMainWindow::~wxDataViewMainWindow()
{
    DestroyTree();
    delete m_renameTimer;
}


int wxDataViewMainWindow::GetDefaultRowHeight() const
{
#ifdef __WXMSW__
            if ( wxGetWinVersion() >= wxWinVersion_Vista )
        return wxMax(16, GetCharHeight()) + 6;     else
#endif         return wxMax(16, GetCharHeight()) + 1; }



#if wxUSE_DRAG_AND_DROP
bool wxDataViewMainWindow::EnableDragSource( const wxDataFormat &format )
{
    m_dragFormat = format;
    m_dragEnabled = format != wxDF_INVALID;

    return true;
}

bool wxDataViewMainWindow::EnableDropTarget( const wxDataFormat &format )
{
    m_dropFormat = format;
    m_dropEnabled = format != wxDF_INVALID;

    if (m_dropEnabled)
        SetDropTarget( new wxDataViewDropTarget( new wxCustomDataObject( format ), this ) );

    return true;
}

void wxDataViewMainWindow::RemoveDropHint()
{
    if (m_dropHint)
    {
            m_dropHint = false;
            RefreshRow( m_dropHintLine );
            m_dropHintLine = (unsigned int) -1;
    }
}

wxDragResult wxDataViewMainWindow::OnDragOver( wxDataFormat format, wxCoord x,
                                               wxCoord y, wxDragResult def )
{
    int xx = x;
    int yy = y;
    m_owner->CalcUnscrolledPosition( xx, yy, &xx, &yy );
    unsigned int row = GetLineAt( yy );

    wxDataViewItem item;

    if ( row < GetRowCount() && xx <= GetEndOfLastCol() )
        item = GetItemByRow( row );

    wxDataViewModel *model = GetModel();

    wxDataViewEvent event( wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE, m_owner->GetId() );
    event.SetEventObject( m_owner );
    event.SetItem( item );
    event.SetModel( model );
    event.SetDataFormat( format );
    event.SetDropEffect( def );
    if ( !m_owner->HandleWindowEvent( event ) || !event.IsAllowed() )
    {
        RemoveDropHint();
        return wxDragNone;
    }

    if ( item.IsOk() )
    {
        if (m_dropHint && (row != m_dropHintLine))
            RefreshRow( m_dropHintLine );
        m_dropHint = true;
        m_dropHintLine = row;
        RefreshRow( row );
    }
    else
    {
        RemoveDropHint();
    }

    return def;
}

bool wxDataViewMainWindow::OnDrop( wxDataFormat format, wxCoord x, wxCoord y )
{
    RemoveDropHint();

    int xx = x;
    int yy = y;
    m_owner->CalcUnscrolledPosition( xx, yy, &xx, &yy );
    unsigned int row = GetLineAt( yy );

    wxDataViewItem item;

    if ( row < GetRowCount() && xx <= GetEndOfLastCol())
       item = GetItemByRow( row );

    wxDataViewModel *model = GetModel();

    wxDataViewEvent event( wxEVT_DATAVIEW_ITEM_DROP_POSSIBLE, m_owner->GetId() );
    event.SetEventObject( m_owner );
    event.SetItem( item );
    event.SetModel( model );
    event.SetDataFormat( format );
    if (!m_owner->HandleWindowEvent( event ) || !event.IsAllowed())
        return false;

    return true;
}

wxDragResult wxDataViewMainWindow::OnData( wxDataFormat format, wxCoord x, wxCoord y,
                                           wxDragResult def )
{
    int xx = x;
    int yy = y;
    m_owner->CalcUnscrolledPosition( xx, yy, &xx, &yy );
    unsigned int row = GetLineAt( yy );

    wxDataViewItem item;

    if ( row < GetRowCount() && xx <= GetEndOfLastCol() )
        item = GetItemByRow( row );

    wxDataViewModel *model = GetModel();

    wxCustomDataObject *obj = (wxCustomDataObject *) GetDropTarget()->GetDataObject();

    wxDataViewEvent event( wxEVT_DATAVIEW_ITEM_DROP, m_owner->GetId() );
    event.SetEventObject( m_owner );
    event.SetItem( item );
    event.SetModel( model );
    event.SetDataFormat( format );
    event.SetDataSize( obj->GetSize() );
    event.SetDataBuffer( obj->GetData() );
    event.SetDropEffect( def );
    if ( !m_owner->HandleWindowEvent( event ) || !event.IsAllowed() )
        return wxDragNone;

    return def;
}

void wxDataViewMainWindow::OnLeave()
{
    RemoveDropHint();
}

wxBitmap wxDataViewMainWindow::CreateItemBitmap( unsigned int row, int &indent )
{
    int height = GetLineHeight( row );
    int width = 0;
    unsigned int cols = GetOwner()->GetColumnCount();
    unsigned int col;
    for (col = 0; col < cols; col++)
    {
        wxDataViewColumn *column = GetOwner()->GetColumnAt(col);
        if (column->IsHidden())
            continue;              width += column->GetWidth();
    }

    indent = 0;
    if (!IsList())
    {
        wxDataViewTreeNode *node = GetTreeNodeByRow(row);
        indent = GetOwner()->GetIndent() * node->GetIndentLevel();
        indent = indent + m_lineHeight;
                }
    width -= indent;

    wxBitmap bitmap( width, height );
    wxMemoryDC dc( bitmap );
    dc.SetFont( GetFont() );
    dc.SetPen( *wxBLACK_PEN );
    dc.SetBrush( *wxWHITE_BRUSH );
    dc.DrawRectangle( 0,0,width,height );

    wxDataViewModel *model = m_owner->GetModel();

    wxDataViewColumn * const
        expander = GetExpanderColumnOrFirstOne(GetOwner());

    int x = 0;
    for (col = 0; col < cols; col++)
    {
        wxDataViewColumn *column = GetOwner()->GetColumnAt( col );
        wxDataViewRenderer *cell = column->GetRenderer();

        if (column->IsHidden())
            continue;       
        width = column->GetWidth();

        if (column == expander)
            width -= indent;

        wxDataViewItem item = GetItemByRow( row );
        cell->PrepareForItem(model, item, column->GetModelColumn());

        wxRect item_rect(x, 0, width, height);
        item_rect.Deflate(PADDING_RIGHTLEFT, 0);

                cell->WXCallRender(item_rect, &dc, 0);
        
        x += width;
    }

    return bitmap;
}

#endif 
void wxDataViewMainWindow::OnPaint( wxPaintEvent &WXUNUSED(event) )
{
    wxDataViewModel *model = GetModel();
    wxAutoBufferedPaintDC dc( this );

    const wxSize size = GetClientSize();

    dc.SetBrush(GetOwner()->GetBackgroundColour());
    dc.SetPen( *wxTRANSPARENT_PEN );
    dc.DrawRectangle(size);

    if ( IsEmpty() )
    {
                return;
    }

        GetOwner()->PrepareDC( dc );
    dc.SetFont( GetFont() );

    wxRect update = GetUpdateRegion().GetBox();
    m_owner->CalcUnscrolledPosition( update.x, update.y, &update.x, &update.y );

        unsigned int item_start = GetLineAt( wxMax(0,update.y) );
    unsigned int item_count =
        wxMin( (int)(  GetLineAt( wxMax(0,update.y+update.height) ) - item_start + 1),
            (int)(GetRowCount( ) - item_start));
    unsigned int item_last = item_start + item_count;

        wxWindow * const parent = GetParent();
    wxDataViewEvent cache_event(wxEVT_DATAVIEW_CACHE_HINT, parent->GetId());
    cache_event.SetEventObject(parent);
    cache_event.SetCache(item_start, item_last - 1);
    parent->ProcessWindowEvent(cache_event);

        unsigned int cols = GetOwner()->GetColumnCount();
    if ( !cols )
    {
                        return;
    }

    unsigned int col_start = 0;
    unsigned int x_start;
    for (x_start = 0; col_start < cols; col_start++)
    {
        wxDataViewColumn *col = GetOwner()->GetColumnAt(col_start);
        if (col->IsHidden())
            continue;      
        unsigned int w = col->GetWidth();
        if (x_start+w >= (unsigned int)update.x)
            break;

        x_start += w;
    }

    unsigned int col_last = col_start;
    unsigned int x_last = x_start;
    for (; col_last < cols; col_last++)
    {
        wxDataViewColumn *col = GetOwner()->GetColumnAt(col_last);
        if (col->IsHidden())
            continue;      
        if (x_last > (unsigned int)update.GetRight())
            break;

        x_last += col->GetWidth();
    }

        if ( m_owner->HasFlag(wxDV_ROW_LINES) )
    {
        wxColour altRowColour = m_owner->m_alternateRowColour;
        if ( !altRowColour.IsOk() )
        {
                                    const wxColour bgColour = m_owner->GetBackgroundColour();

                                    int alpha = bgColour.GetRGB() > 0x808080 ? 97 : 150;
            altRowColour = bgColour.ChangeLightness(alpha);
        }

        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.SetBrush(wxBrush(altRowColour));

                const int xRect = m_owner->CalcUnscrolledPosition(wxPoint(0, 0)).x;
        const int widthRect = size.x;
        for (unsigned int item = item_start; item < item_last; item++)
        {
            if ( item % 2 )
            {
                dc.DrawRectangle(xRect,
                                 GetLineStart(item),
                                 widthRect,
                                 GetLineHeight(item));
            }
        }
    }

        if ( m_owner->HasFlag(wxDV_HORIZ_RULES) )
    {
        dc.SetPen(m_penRule);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);

        for (unsigned int i = item_start; i <= item_last; i++)
        {
            int y = GetLineStart( i );
            dc.DrawLine(x_start, y, x_last, y);
        }
    }

        if ( m_owner->HasFlag(wxDV_VERT_RULES) )
    {
        dc.SetPen(m_penRule);
        dc.SetBrush(*wxTRANSPARENT_BRUSH);

                                
        int x = x_start - 1;
        for (unsigned int i = col_start; i < col_last; i++)
        {
            wxDataViewColumn *col = GetOwner()->GetColumnAt(i);
            if (col->IsHidden())
                continue;       
            x += col->GetWidth();

            dc.DrawLine(x, GetLineStart( item_start ),
                        x, GetLineStart( item_last ) );
        }
    }

        for (unsigned int item = item_start; item < item_last; item++)
    {
        bool selected = m_selection.IsSelected(item);

        if (selected || item == m_currentRow)
        {
            wxRect rowRect( x_start, GetLineStart( item ),
                         x_last - x_start, GetLineHeight( item ) );

            bool renderColumnFocus = false;

            int flags = wxCONTROL_SELECTED;
            if ( m_hasFocus )
                flags |= wxCONTROL_FOCUSED;

                        if ( item == m_currentRow && m_hasFocus )
            {

                if ( m_useCellFocus && m_currentCol && m_currentColSetByKeyboard )
                {
                    renderColumnFocus = true;

                                        if ( !IsList() )
                    {
                        wxDataViewTreeNode *node = GetTreeNodeByRow(item);
                        if ( node->HasChildren() && !model->HasContainerColumns(node->GetItem()) )
                            renderColumnFocus = false;
                    }
                }

                if ( renderColumnFocus )
                {
                    wxRect colRect(rowRect);

                    for ( unsigned int i = col_start; i < col_last; i++ )
                    {
                        wxDataViewColumn *col = GetOwner()->GetColumnAt(i);
                        if ( col->IsHidden() )
                            continue;

                        colRect.width = col->GetWidth();

                        if ( col == m_currentCol )
                        {
                                                        {
                                wxRect clipRect(rowRect);
                                clipRect.width = colRect.x;

                                wxDCClipper clip(dc, clipRect);
                                wxRendererNative::Get().DrawItemSelectionRect
                                    (
                                    this,
                                    dc,
                                    rowRect,
                                    flags
                                    );
                            }

                                                        {
                                wxRect clipRect(rowRect);
                                clipRect.x = colRect.x + colRect.width;
                                clipRect.width = rowRect.width - clipRect.x;

                                wxDCClipper clip(dc, clipRect);
                                wxRendererNative::Get().DrawItemSelectionRect
                                    (
                                    this,
                                    dc,
                                    rowRect,
                                    flags
                                    );
                            }

                                                        wxRendererNative::Get().DrawItemSelectionRect
                                (
                                this,
                                dc,
                                colRect,
                                flags | wxCONTROL_CURRENT | wxCONTROL_CELL
                                );

                            break;
                        }

                        colRect.x += colRect.width;
                    }
                }
            }

                        if ( selected && !renderColumnFocus )
            {
                wxRendererNative::Get().DrawItemSelectionRect
                    (
                    this,
                    dc,
                    rowRect,
                    flags | wxCONTROL_CURRENT
                    );
            }
        }
    }

#if wxUSE_DRAG_AND_DROP
    if (m_dropHint)
    {
        wxRect rect( x_start, GetLineStart( m_dropHintLine ),
                     x_last - x_start, GetLineHeight( m_dropHintLine ) );
        dc.SetPen( *wxBLACK_PEN );
        dc.SetBrush( *wxTRANSPARENT_BRUSH );
        dc.DrawRectangle( rect );
    }
#endif 
    wxDataViewColumn * const
        expander = GetExpanderColumnOrFirstOne(GetOwner());

        wxRect cell_rect;
    cell_rect.x = x_start;
    for (unsigned int i = col_start; i < col_last; i++)
    {
        wxDataViewColumn *col = GetOwner()->GetColumnAt( i );
        wxDataViewRenderer *cell = col->GetRenderer();
        cell_rect.width = col->GetWidth();

        if ( col->IsHidden() || cell_rect.width <= 0 )
            continue;       
        for (unsigned int item = item_start; item < item_last; item++)
        {
                        wxDataViewTreeNode *node = NULL;
            wxDataViewItem dataitem;

            if (!IsVirtualList())
            {
                node = GetTreeNodeByRow(item);
                if( node == NULL )
                    continue;

                dataitem = node->GetItem();

                                                if ( col != expander &&
                        model->IsContainer(dataitem) &&
                            !model->HasContainerColumns(dataitem) )
                    continue;
            }
            else
            {
                dataitem = wxDataViewItem( wxUIntToPtr(item+1) );
            }

                        cell_rect.y = GetLineStart( item );
            cell_rect.height = GetLineHeight( item );

            cell->PrepareForItem(model, dataitem, col->GetModelColumn());

                        bool selected = m_selection.IsSelected(item);
            if ( !selected )
                DrawCellBackground( cell, dc, cell_rect );

                        int indent = 0;
            if ((!IsList()) && (col == expander))
            {
                                indent = GetOwner()->GetIndent() * node->GetIndentLevel();

                                                int exp_x = cell_rect.x + indent + EXPANDER_MARGIN;

                indent += m_lineHeight;

                                if ( node->HasChildren() && exp_x < cell_rect.GetRight() )
                {
                    dc.SetPen( m_penExpander );
                    dc.SetBrush( wxNullBrush );

                    int exp_size = m_lineHeight - 2*EXPANDER_MARGIN;
                    int exp_y = cell_rect.y + (cell_rect.height - exp_size)/2
                                   + EXPANDER_MARGIN - EXPANDER_OFFSET;

                    const wxRect rect(exp_x, exp_y, exp_size, exp_size);

                    int flag = 0;
                    if ( m_underMouse == node )
                        flag |= wxCONTROL_CURRENT;
                    if ( node->IsOpen() )
                        flag |= wxCONTROL_EXPANDED;

                                                            wxDCClipper clip(dc, cell_rect);

                    wxRendererNative::Get().DrawTreeItemButton( this, dc, rect, flag);
                }

                                cell->SetAlignment( wxALIGN_CENTER_VERTICAL );
            }

            wxRect item_rect = cell_rect;
            item_rect.Deflate(PADDING_RIGHTLEFT, 0);

                        item_rect.x += indent;
            item_rect.width -= indent;

            if ( item_rect.width <= 0 )
                continue;

            int state = 0;
            if (m_hasFocus && selected)
                state |= wxDATAVIEW_CELL_SELECTED;

                                                                                                wxDCClipper clip(dc, item_rect);

            cell->WXCallRender(item_rect, &dc, state);
        }

        cell_rect.x += cell_rect.width;
    }
}


void wxDataViewMainWindow::DrawCellBackground( wxDataViewRenderer* cell, wxDC& dc, const wxRect& rect )
{
    wxRect rectBg( rect );

        if ( m_owner->HasFlag(wxDV_HORIZ_RULES) )
    {
        rectBg.x++;
        rectBg.width--;
    }

        if ( m_owner->HasFlag(wxDV_VERT_RULES) )
    {
        rectBg.y++;
        rectBg.height--;
    }

    cell->RenderBackground(&dc, rectBg);
}

void wxDataViewMainWindow::OnRenameTimer()
{
            if ( m_dirty )
    {
                        wxSafeYield();
    }

    wxDataViewItem item = GetItemByRow( m_currentRow );

    StartEditing( item, m_currentCol );
}

void
wxDataViewMainWindow::StartEditing(const wxDataViewItem& item,
                                   const wxDataViewColumn* col)
{
    wxDataViewRenderer* renderer = col->GetRenderer();
    if ( !IsCellEditableInMode(item, col, wxDATAVIEW_CELL_EDITABLE) )
        return;

    const wxRect itemRect = GetItemRect(item, col);
    if ( renderer->StartEditing(item, itemRect) )
    {
        renderer->NotifyEditingStarted(item);

                        m_editorRenderer = renderer;
        m_editorCtrl = renderer->GetEditorCtrl();
    }
}

void wxDataViewMainWindow::FinishEditing()
{
    if ( m_editorCtrl )
    {
        m_editorRenderer->FinishEditing();
    }
}

void wxDataViewHeaderWindow::FinishEditing()
{
    wxDataViewMainWindow *win = static_cast<wxDataViewMainWindow*>(GetOwner()->GetMainWindow());
    win->FinishEditing();
}

class DoJob
{
public:
    DoJob() { }
    virtual ~DoJob() { }

        enum
    {
        DONE,                  SKIP_SUBTREE,          CONTINUE           };

    virtual int operator() ( wxDataViewTreeNode * node ) = 0;
};

bool Walker( wxDataViewTreeNode * node, DoJob & func )
{
    wxCHECK_MSG( node, false, "can't walk NULL node" );

    switch( func( node ) )
    {
        case DoJob::DONE:
            return true;
        case DoJob::SKIP_SUBTREE:
            return false;
        case DoJob::CONTINUE:
            break;
    }

    if ( node->HasChildren() )
    {
        const wxDataViewTreeNodes& nodes = node->GetChildNodes();

        for ( wxDataViewTreeNodes::const_iterator i = nodes.begin();
              i != nodes.end();
              ++i )
        {
            if ( Walker(*i, func) )
                return true;
        }
    }

    return false;
}

bool wxDataViewMainWindow::ItemAdded(const wxDataViewItem & parent, const wxDataViewItem & item)
{
    if (IsVirtualList())
    {
        wxDataViewVirtualListModel *list_model =
            (wxDataViewVirtualListModel*) GetModel();
        m_count = list_model->GetCount();

        m_selection.OnItemsInserted(GetRowByItem(item), 1);
    }
    else
    {
        SortPrepare();

        wxDataViewTreeNode *parentNode = FindNode(parent);

        if ( !parentNode )
            return false;

        wxDataViewItemArray modelSiblings;
        GetModel()->GetChildren(parent, modelSiblings);
        const int modelSiblingsSize = modelSiblings.size();

        int posInModel = modelSiblings.Index(item, true);
        wxCHECK_MSG( posInModel != wxNOT_FOUND, false, "adding non-existent item?" );

        wxDataViewTreeNode *itemNode = new wxDataViewTreeNode(this, parentNode, item);
        itemNode->SetHasChildren(GetModel()->IsContainer(item));

        parentNode->SetHasChildren(true);

        const wxDataViewTreeNodes& nodeSiblings = parentNode->GetChildNodes();
        const int nodeSiblingsSize = nodeSiblings.size();

        int nodePos = 0;

        if ( posInModel == modelSiblingsSize - 1 )
        {
            nodePos = nodeSiblingsSize;
        }
        else if ( modelSiblingsSize == nodeSiblingsSize + 1 )
        {
                                    nodePos = posInModel;
        }
        else
        {
                                                            
                        nodePos = nodeSiblingsSize;

            for ( int nextItemPos = posInModel + 1;
                  nextItemPos < modelSiblingsSize;
                  nextItemPos++ )
            {
                int nextNodePos = parentNode->FindChildByItem(modelSiblings[nextItemPos]);
                if ( nextNodePos != wxNOT_FOUND )
                {
                    nodePos = nextNodePos;
                    break;
                }
            }
        }

        parentNode->ChangeSubTreeCount(+1);
        parentNode->InsertChild(itemNode, nodePos);

        InvalidateCount();
    }

    GetOwner()->InvalidateColBestWidths();
    UpdateDisplay();

    return true;
}

bool wxDataViewMainWindow::ItemDeleted(const wxDataViewItem& parent,
                                       const wxDataViewItem& item)
{
    if (IsVirtualList())
    {
        wxDataViewVirtualListModel *list_model =
            (wxDataViewVirtualListModel*) GetModel();
        m_count = list_model->GetCount();

        m_selection.OnItemDelete(GetRowByItem(item));
    }
    else     {
        wxDataViewTreeNode *parentNode = FindNode(parent);

                                        if ( !parentNode )
            return true;

        wxCHECK_MSG( parentNode->HasChildren(), false, "parent node doesn't have children?" );
        const wxDataViewTreeNodes& parentsChildren = parentNode->GetChildNodes();

                                        int itemPosInNode = 0;
        wxDataViewTreeNode *itemNode = NULL;
        for ( wxDataViewTreeNodes::const_iterator i = parentsChildren.begin();
              i != parentsChildren.end();
              ++i, ++itemPosInNode )
        {
            if( (*i)->GetItem() == item )
            {
                itemNode = *i;
                break;
            }
        }

                        if ( !itemNode )
        {
                                    if ( parentNode->GetChildNodes().empty() )
                parentNode->SetHasChildren(GetModel()->IsContainer(parent));

            return true;
        }

                const int itemsDeleted = 1 + itemNode->GetSubTreeCount();

        parentNode->RemoveChild(itemNode);
        delete itemNode;
        parentNode->ChangeSubTreeCount(-itemsDeleted);

                InvalidateCount();

                        if ( parentNode->GetChildNodes().empty() )
        {
            bool isContainer = GetModel()->IsContainer(parent);
            parentNode->SetHasChildren(isContainer);
            if ( isContainer )
            {
                                                if ( parentNode->IsOpen() )
                    parentNode->ToggleOpen();
            }
        }

                if ( !m_selection.IsEmpty() )
        {
                                    int itemRow;
            if ( itemPosInNode == 0 )
            {
                                itemRow = GetRowByItem(parentNode->GetItem()) + 1;
            }
            else
            {
                                const wxDataViewTreeNode *siblingNode = parentNode->GetChildNodes()[itemPosInNode - 1];

                itemRow = GetRowByItem(siblingNode->GetItem()) +
                          siblingNode->GetSubTreeCount() +
                          1;
            }

            m_selection.OnItemsDeleted(itemRow, itemsDeleted);
        }
    }

        if ( m_currentRow >= GetRowCount() )
        ChangeCurrentRow(m_count - 1);

    GetOwner()->InvalidateColBestWidths();
    UpdateDisplay();

    return true;
}

bool wxDataViewMainWindow::ItemChanged(const wxDataViewItem & item)
{
    SortPrepare();
    GetModel()->Resort();

    GetOwner()->InvalidateColBestWidths();

        wxWindow *parent = GetParent();
    wxDataViewEvent le(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, parent->GetId());
    le.SetEventObject(parent);
    le.SetModel(GetModel());
    le.SetItem(item);
    parent->ProcessWindowEvent(le);

    return true;
}

bool wxDataViewMainWindow::ValueChanged( const wxDataViewItem & item, unsigned int model_column )
{
    int view_column = m_owner->GetModelColumnIndex(model_column);
    if ( view_column == wxNOT_FOUND )
        return false;

    
    SortPrepare();
    GetModel()->Resort();

    GetOwner()->InvalidateColBestWidth(view_column);

        wxWindow *parent = GetParent();
    wxDataViewEvent le(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED, parent->GetId());
    le.SetEventObject(parent);
    le.SetModel(GetModel());
    le.SetItem(item);
    le.SetColumn(view_column);
    le.SetDataViewColumn(GetOwner()->GetColumn(view_column));
    parent->ProcessWindowEvent(le);

    return true;
}

bool wxDataViewMainWindow::Cleared()
{
    DestroyTree();
    m_selection.Clear();
    m_currentRow = (unsigned)-1;

    if (GetModel())
    {
        SortPrepare();
        BuildTree( GetModel() );
    }
    else
    {
        m_count = 0;
    }

    GetOwner()->InvalidateColBestWidths();
    UpdateDisplay();

    return true;
}

void wxDataViewMainWindow::UpdateDisplay()
{
    m_dirty = true;
    m_underMouse = NULL;
}

void wxDataViewMainWindow::OnInternalIdle()
{
    wxWindow::OnInternalIdle();

    if (m_dirty)
    {
        UpdateColumnSizes();
        RecalculateDisplay();
        m_dirty = false;
    }
}

void wxDataViewMainWindow::RecalculateDisplay()
{
    wxDataViewModel *model = GetModel();
    if (!model)
    {
        Refresh();
        return;
    }

    int width = GetEndOfLastCol();
    int height = GetLineStart( GetRowCount() );

    SetVirtualSize( width, height );
    GetOwner()->SetScrollRate( 10, m_lineHeight );

    Refresh();
}

void wxDataViewMainWindow::ScrollWindow( int dx, int dy, const wxRect *rect )
{
    m_underMouse = NULL;

    wxWindow::ScrollWindow( dx, dy, rect );

    if (GetOwner()->m_headerArea)
        GetOwner()->m_headerArea->ScrollWindow( dx, 0 );
}

void wxDataViewMainWindow::ScrollTo( int rows, int column )
{
    m_underMouse = NULL;

    int x, y;
    m_owner->GetScrollPixelsPerUnit( &x, &y );
    int sy = GetLineStart( rows )/y;
    int sx = -1;
    if( column != -1 )
    {
        wxRect rect = GetClientRect();
        int colnum = 0;
        int x_start, w = 0;
        int xx, yy, xe;
        m_owner->CalcUnscrolledPosition( rect.x, rect.y, &xx, &yy );
        for (x_start = 0; colnum < column; colnum++)
        {
            wxDataViewColumn *col = GetOwner()->GetColumnAt(colnum);
            if (col->IsHidden())
                continue;      
            w = col->GetWidth();
            x_start += w;
        }

        int x_end = x_start + w;
        xe = xx + rect.width;
        if( x_end > xe )
        {
            sx = ( xx + x_end - xe )/x;
        }
        if( x_start < xx )
        {
            sx = x_start/x;
        }
    }
    m_owner->Scroll( sx, sy );
}

int wxDataViewMainWindow::GetCountPerPage() const
{
    wxSize size = GetClientSize();
    return size.y / m_lineHeight;
}

int wxDataViewMainWindow::GetEndOfLastCol() const
{
    int width = 0;
    unsigned int i;
    for (i = 0; i < GetOwner()->GetColumnCount(); i++)
    {
        const wxDataViewColumn *c =
            const_cast<wxDataViewCtrl*>(GetOwner())->GetColumnAt( i );

        if (!c->IsHidden())
            width += c->GetWidth();
    }
    return width;
}

unsigned int wxDataViewMainWindow::GetFirstVisibleRow() const
{
    int x = 0;
    int y = 0;
    m_owner->CalcUnscrolledPosition( x, y, &x, &y );

    return GetLineAt( y );
}

unsigned int wxDataViewMainWindow::GetLastVisibleRow()
{
    wxSize client_size = GetClientSize();
    m_owner->CalcUnscrolledPosition( client_size.x, client_size.y,
                                    &client_size.x, &client_size.y );

        unsigned int row = GetLineAt(client_size.y) - 1;

    return wxMin( GetRowCount()-1, row );
}

unsigned int wxDataViewMainWindow::GetRowCount() const
{
    if ( m_count == -1 )
    {
        wxDataViewMainWindow* const
            self = const_cast<wxDataViewMainWindow*>(this);
        self->UpdateCount(RecalculateCount());
        self->UpdateDisplay();
    }
    return m_count;
}

void wxDataViewMainWindow::ChangeCurrentRow( unsigned int row )
{
    m_currentRow = row;

    }

bool wxDataViewMainWindow::UnselectAllRows(unsigned int except)
{
    if (!m_selection.IsEmpty())
    {
        for (unsigned i = GetFirstVisibleRow(); i <= GetLastVisibleRow(); i++)
        {
            if (m_selection.IsSelected(i) && i != except)
                RefreshRow(i);
        }

        if (except != (unsigned int)-1)
        {
            const bool wasSelected = m_selection.IsSelected(except);
            ClearSelection();
            if (wasSelected)
            {
                m_selection.SelectItem(except);

                                return false;
            }
        }
        else
        {
            ClearSelection();
        }
    }

        return true;
}

void wxDataViewMainWindow::SelectRow( unsigned int row, bool on )
{
    if ( m_selection.SelectItem(row, on) )
        RefreshRow(row);
}

void wxDataViewMainWindow::SelectRows( unsigned int from, unsigned int to )
{
    wxArrayInt changed;
    if ( m_selection.SelectRange(from, to, true, &changed) )
    {
        for (unsigned i = 0; i < changed.size(); i++)
            RefreshRow(changed[i]);
    }
    else     {
        RefreshRows( from, to );
    }
}

void wxDataViewMainWindow::Select( const wxArrayInt& aSelections )
{
    for (size_t i=0; i < aSelections.GetCount(); i++)
    {
        int n = aSelections[i];

        if ( m_selection.SelectItem(n) )
            RefreshRow( n );
    }
}

void wxDataViewMainWindow::ReverseRowSelection( unsigned int row )
{
    m_selection.SelectItem(row, !m_selection.IsSelected(row));
    RefreshRow( row );
}

bool wxDataViewMainWindow::IsRowSelected( unsigned int row )
{
    return m_selection.IsSelected(row);
}

void wxDataViewMainWindow::SendSelectionChangedEvent( const wxDataViewItem& item)
{
    wxWindow *parent = GetParent();
    wxDataViewEvent le(wxEVT_DATAVIEW_SELECTION_CHANGED, parent->GetId());

    le.SetEventObject(parent);
    le.SetModel(GetModel());
    le.SetItem( item );

    parent->ProcessWindowEvent(le);
}

void wxDataViewMainWindow::RefreshRow( unsigned int row )
{
    wxRect rect( 0, GetLineStart( row ), GetEndOfLastCol(), GetLineHeight( row ) );
    m_owner->CalcScrolledPosition( rect.x, rect.y, &rect.x, &rect.y );

    wxSize client_size = GetClientSize();
    wxRect client_rect( 0, 0, client_size.x, client_size.y );
    wxRect intersect_rect = client_rect.Intersect( rect );
    if (intersect_rect.width > 0)
        Refresh( true, &intersect_rect );
}

void wxDataViewMainWindow::RefreshRows( unsigned int from, unsigned int to )
{
    if (from > to)
    {
        unsigned int tmp = to;
        to = from;
        from = tmp;
    }

    wxRect rect( 0, GetLineStart( from ), GetEndOfLastCol(), GetLineStart( (to-from+1) ) );
    m_owner->CalcScrolledPosition( rect.x, rect.y, &rect.x, &rect.y );

    wxSize client_size = GetClientSize();
    wxRect client_rect( 0, 0, client_size.x, client_size.y );
    wxRect intersect_rect = client_rect.Intersect( rect );
    if (intersect_rect.width > 0)
        Refresh( true, &intersect_rect );
}

void wxDataViewMainWindow::RefreshRowsAfter( unsigned int firstRow )
{
    wxSize client_size = GetClientSize();
    int start = GetLineStart( firstRow );
    m_owner->CalcScrolledPosition( start, 0, &start, NULL );
    if (start > client_size.y) return;

    wxRect rect( 0, start, client_size.x, client_size.y - start );

    Refresh( true, &rect );
}

wxRect wxDataViewMainWindow::GetLineRect( unsigned int row ) const
{
    wxRect rect;
    rect.x = 0;
    rect.y = GetLineStart( row );
    rect.width = GetEndOfLastCol();
    rect.height = GetLineHeight( row );

    return rect;
}

int wxDataViewMainWindow::GetLineStart( unsigned int row ) const
{
    const wxDataViewModel *model = GetModel();

    if (GetOwner()->GetWindowStyle() & wxDV_VARIABLE_LINE_HEIGHT)
    {
        
        int start = 0;

        unsigned int r;
        for (r = 0; r < row; r++)
        {
            const wxDataViewTreeNode* node = GetTreeNodeByRow(r);
            if (!node) return start;

            wxDataViewItem item = node->GetItem();

            unsigned int cols = GetOwner()->GetColumnCount();
            unsigned int col;
            int height = m_lineHeight;
            for (col = 0; col < cols; col++)
            {
                const wxDataViewColumn *column = GetOwner()->GetColumn(col);
                if (column->IsHidden())
                    continue;      
                if ((col != 0) &&
                    model->IsContainer(item) &&
                    !model->HasContainerColumns(item))
                    continue;      
                wxDataViewRenderer *renderer =
                    const_cast<wxDataViewRenderer*>(column->GetRenderer());
                renderer->PrepareForItem(model, item, column->GetModelColumn());

                height = wxMax( height, renderer->GetSize().y );
            }

            start += height;
        }

        return start;
    }
    else
    {
        return row * m_lineHeight;
    }
}

int wxDataViewMainWindow::GetLineAt( unsigned int y ) const
{
    const wxDataViewModel *model = GetModel();

        if ( !GetOwner()->HasFlag(wxDV_VARIABLE_LINE_HEIGHT) )
        return y / m_lineHeight;

        unsigned int row = 0;
    unsigned int yy = 0;
    for (;;)
    {
        const wxDataViewTreeNode* node = GetTreeNodeByRow(row);
        if (!node)
        {
                        return row + ((y-yy) / m_lineHeight);
        }

        wxDataViewItem item = node->GetItem();

        unsigned int cols = GetOwner()->GetColumnCount();
        unsigned int col;
        int height = m_lineHeight;
        for (col = 0; col < cols; col++)
        {
            const wxDataViewColumn *column = GetOwner()->GetColumn(col);
            if (column->IsHidden())
                continue;      
            if ((col != 0) &&
                model->IsContainer(item) &&
                !model->HasContainerColumns(item))
                continue;      
            wxDataViewRenderer *renderer =
                const_cast<wxDataViewRenderer*>(column->GetRenderer());
            renderer->PrepareForItem(model, item, column->GetModelColumn());

            height = wxMax( height, renderer->GetSize().y );
        }

        yy += height;
        if (y < yy)
            return row;

        row++;
    }
}

int wxDataViewMainWindow::GetLineHeight( unsigned int row ) const
{
    const wxDataViewModel *model = GetModel();

    if (GetOwner()->GetWindowStyle() & wxDV_VARIABLE_LINE_HEIGHT)
    {
        wxASSERT( !IsVirtualList() );

        const wxDataViewTreeNode* node = GetTreeNodeByRow(row);
                if (!node) return m_lineHeight;

        wxDataViewItem item = node->GetItem();

        int height = m_lineHeight;

        unsigned int cols = GetOwner()->GetColumnCount();
        unsigned int col;
        for (col = 0; col < cols; col++)
        {
            const wxDataViewColumn *column = GetOwner()->GetColumn(col);
            if (column->IsHidden())
                continue;      
            if ((col != 0) &&
                model->IsContainer(item) &&
                !model->HasContainerColumns(item))
                continue;      
            wxDataViewRenderer *renderer =
                const_cast<wxDataViewRenderer*>(column->GetRenderer());
            renderer->PrepareForItem(model, item, column->GetModelColumn());

            height = wxMax( height, renderer->GetSize().y );
        }

        return height;
    }
    else
    {
        return m_lineHeight;
    }
}


class RowToTreeNodeJob: public DoJob
{
public:
    RowToTreeNodeJob( unsigned int row_ , int current_, wxDataViewTreeNode * node )
    {
        this->row = row_;
        this->current = current_;
        ret = NULL;
        parent = node;
    }

    virtual int operator() ( wxDataViewTreeNode * node )
    {
        current ++;
        if( current == static_cast<int>(row))
        {
            ret = node;
            return DoJob::DONE;
        }

        if( node->GetSubTreeCount() + current < static_cast<int>(row) )
        {
            current += node->GetSubTreeCount();
            return  DoJob::SKIP_SUBTREE;
        }
        else
        {
            parent = node;

                                                if ( node->HasChildren() &&
                 (int)node->GetChildNodes().size() == node->GetSubTreeCount() )
            {
                const int index = static_cast<int>(row) - current - 1;
                ret = node->GetChildNodes()[index];
                return DoJob::DONE;
            }

            return DoJob::CONTINUE;
        }
    }

    wxDataViewTreeNode * GetResult() const
        { return ret; }

private:
    unsigned int row;
    int current;
    wxDataViewTreeNode * ret;
    wxDataViewTreeNode * parent;
};

wxDataViewTreeNode * wxDataViewMainWindow::GetTreeNodeByRow(unsigned int row) const
{
    wxASSERT( !IsVirtualList() );

    if ( row == (unsigned)-1 )
        return NULL;

    RowToTreeNodeJob job( row , -2, m_root );
    Walker( m_root , job );
    return job.GetResult();
}

wxDataViewItem wxDataViewMainWindow::GetItemByRow(unsigned int row) const
{
    wxDataViewItem item;
    if (IsVirtualList())
    {
        if ( row < GetRowCount() )
            item = wxDataViewItem(wxUIntToPtr(row+1));
    }
    else
    {
        wxDataViewTreeNode *node = GetTreeNodeByRow(row);
        if ( node )
            item = node->GetItem();
    }

    return item;
}

bool
wxDataViewMainWindow::SendExpanderEvent(wxEventType type,
                                        const wxDataViewItem& item)
{
    wxWindow *parent = GetParent();
    wxDataViewEvent le(type, parent->GetId());

    le.SetEventObject(parent);
    le.SetModel(GetModel());
    le.SetItem( item );

    return !parent->ProcessWindowEvent(le) || le.IsAllowed();
}

bool wxDataViewMainWindow::IsExpanded( unsigned int row ) const
{
    if (IsList())
        return false;

    wxDataViewTreeNode * node = GetTreeNodeByRow(row);
    if (!node)
        return false;

    if (!node->HasChildren())
        return false;

    return node->IsOpen();
}

bool wxDataViewMainWindow::HasChildren( unsigned int row ) const
{
    if (IsList())
        return false;

    wxDataViewTreeNode * node = GetTreeNodeByRow(row);
    if (!node)
        return false;

    if (!node->HasChildren())
        return false;

    return true;
}

void wxDataViewMainWindow::Expand( unsigned int row )
{
    if (IsList())
        return;

    wxDataViewTreeNode * node = GetTreeNodeByRow(row);
    if (!node)
        return;

    if (!node->HasChildren())
        return;

    if (!node->IsOpen())
    {
        if ( !SendExpanderEvent(wxEVT_DATAVIEW_ITEM_EXPANDING, node->GetItem()) )
        {
                        return;
        }

        node->ToggleOpen();

                if( node->GetChildNodes().empty() )
        {
            SortPrepare();
            ::BuildTreeHelper(this, GetModel(), node->GetItem(), node);
        }

        const unsigned countNewRows = node->GetSubTreeCount();

                        m_selection.OnItemsInserted(row + 1, countNewRows);
        if ( m_currentRow > row )
            ChangeCurrentRow(m_currentRow + countNewRows);

        if ( m_count != -1 )
            m_count += countNewRows;

                        GetOwner()->InvalidateColBestWidths();

        UpdateDisplay();
                SendExpanderEvent(wxEVT_DATAVIEW_ITEM_EXPANDED,node->GetItem());
    }
}

void wxDataViewMainWindow::Collapse(unsigned int row)
{
    if (IsList())
        return;

    wxDataViewTreeNode *node = GetTreeNodeByRow(row);
    if (!node)
        return;

    if (!node->HasChildren())
        return;

        if (node->IsOpen())
        {
            if ( !SendExpanderEvent(wxEVT_DATAVIEW_ITEM_COLLAPSING,node->GetItem()) )
            {
                                return;
            }

            const unsigned countDeletedRows = node->GetSubTreeCount();

            if ( m_selection.OnItemsDeleted(row + 1, countDeletedRows) )
            {
                SendSelectionChangedEvent(GetItemByRow(row));
            }

            node->ToggleOpen();

                        if ( m_currentRow > row )
            {
                                                if ( m_currentRow <= row + countDeletedRows )
                    ChangeCurrentRow(row);
                else                     ChangeCurrentRow(m_currentRow - countDeletedRows);
            }

            if ( m_count != -1 )
                m_count -= countDeletedRows;

            GetOwner()->InvalidateColBestWidths();

            UpdateDisplay();
            SendExpanderEvent(wxEVT_DATAVIEW_ITEM_COLLAPSED,node->GetItem());
        }
}

wxDataViewTreeNode * wxDataViewMainWindow::FindNode( const wxDataViewItem & item )
{
    const wxDataViewModel * model = GetModel();
    if( model == NULL )
        return NULL;

    if (!item.IsOk())
        return m_root;

        wxVector<wxDataViewItem> parentChain;
    wxDataViewItem it( item );
    while( it.IsOk() )
    {
        parentChain.push_back(it);
        it = model->GetParent(it);
    }

            wxDataViewTreeNode* node = m_root;
    for( unsigned iter = parentChain.size()-1; ; --iter )
    {
        if( node->HasChildren() )
        {
            if( node->GetChildNodes().empty() )
            {
                                                                SortPrepare();
                ::BuildTreeHelper(this, model, node->GetItem(), node);
            }

            const wxDataViewTreeNodes& nodes = node->GetChildNodes();
            bool found = false;

            for (unsigned i = 0; i < nodes.GetCount(); ++i)
            {
                wxDataViewTreeNode* currentNode = nodes[i];
                if (currentNode->GetItem() == parentChain[iter])
                {
                    if (currentNode->GetItem() == item)
                        return currentNode;

                    node = currentNode;
                    found = true;
                    break;
                }
            }
            if (!found)
                return NULL;
        }
        else
            return NULL;

        if ( !iter )
            break;
    }
    return NULL;
}

void wxDataViewMainWindow::HitTest( const wxPoint & point, wxDataViewItem & item,
                                    wxDataViewColumn* &column )
{
    wxDataViewColumn *col = NULL;
    unsigned int cols = GetOwner()->GetColumnCount();
    unsigned int colnum = 0;
    int x, y;
    m_owner->CalcUnscrolledPosition( point.x, point.y, &x, &y );
    for (unsigned x_start = 0; colnum < cols; colnum++)
    {
        col = GetOwner()->GetColumnAt(colnum);
        if (col->IsHidden())
            continue;      
        unsigned int w = col->GetWidth();
        if (x_start+w >= (unsigned int)x)
            break;

        x_start += w;
    }

    column = col;
    item = GetItemByRow( GetLineAt( y ) );
}

wxRect wxDataViewMainWindow::GetItemRect( const wxDataViewItem & item,
                                          const wxDataViewColumn* column )
{
    int xpos = 0;
    int width = 0;

    unsigned int cols = GetOwner()->GetColumnCount();
            for (unsigned int i = 0; i < cols; i++)
    {
        wxDataViewColumn* col = GetOwner()->GetColumnAt( i );

        if (col == column)
            break;

        if (col->IsHidden())
            continue;      
        xpos += col->GetWidth();
        width += col->GetWidth();
    }

    if(column != 0)
    {
                if(column->IsHidden())
            width = 0;
        else
            width = column->GetWidth();

    }
    else
    {
                xpos = 0;
    }

            int indent = 0;
    int row = GetRowByItem(item);
    if (!IsList() &&
            (column == 0 || GetExpanderColumnOrFirstOne(GetOwner()) == column) )
    {
        wxDataViewTreeNode* node = GetTreeNodeByRow(row);
        indent = GetOwner()->GetIndent() * node->GetIndentLevel();
        indent = indent + m_lineHeight;     }

    wxRect itemRect( xpos + indent,
                     GetLineStart( row ),
                     width - indent,
                     GetLineHeight( row ) );

    GetOwner()->CalcScrolledPosition(  itemRect.x,  itemRect.y,
                                      &itemRect.x, &itemRect.y );

    return itemRect;
}

int wxDataViewMainWindow::RecalculateCount() const
{
    if (IsVirtualList())
    {
        wxDataViewVirtualListModel *list_model =
            (wxDataViewVirtualListModel*) GetModel();

        return list_model->GetCount();
    }
    else
    {
        return m_root->GetSubTreeCount();
    }
}

class ItemToRowJob : public DoJob
{
public:
    ItemToRowJob(const wxDataViewItem& item_, wxVector<wxDataViewItem>::reverse_iterator iter)
        : m_iter(iter),
        item(item_)
    {
        ret = -1;
    }

        virtual int operator() ( wxDataViewTreeNode * node)
    {
        ret ++;
        if( node->GetItem() == item )
        {
            return DoJob::DONE;
        }

        if( node->GetItem() == *m_iter )
        {
            m_iter++;
            return DoJob::CONTINUE;
        }
        else
        {
            ret += node->GetSubTreeCount();
            return DoJob::SKIP_SUBTREE;
        }

    }

        int GetResult() const
        { return ret -1; }

private:
    wxVector<wxDataViewItem>::reverse_iterator m_iter;
    wxDataViewItem item;
    int ret;

};

int wxDataViewMainWindow::GetRowByItem(const wxDataViewItem & item) const
{
    const wxDataViewModel * model = GetModel();
    if( model == NULL )
        return -1;

    if (IsVirtualList())
    {
        return wxPtrToUInt( item.GetID() ) -1;
    }
    else
    {
        if( !item.IsOk() )
            return -1;

                wxVector<wxDataViewItem> parentChain;
        wxDataViewItem it( item );
        while( it.IsOk() )
        {
            parentChain.push_back(it);
            it = model->GetParent(it);
        }

                parentChain.push_back(wxDataViewItem());

                        ItemToRowJob job( item, parentChain.rbegin() );
        Walker( m_root, job );
        return job.GetResult();
    }
}

static void BuildTreeHelper( wxDataViewMainWindow *window, const wxDataViewModel * model,
                             const wxDataViewItem & item, wxDataViewTreeNode * node)
{
    if( !model->IsContainer( item ) )
        return;

    wxDataViewItemArray children;
    unsigned int num = model->GetChildren( item, children);

    for ( unsigned int index = 0; index < num; index++ )
    {
        wxDataViewTreeNode *n = new wxDataViewTreeNode(window, node, children[index]);

        if( model->IsContainer(children[index]) )
            n->SetHasChildren( true );

        node->InsertChild(n, index);
    }

    wxASSERT( node->IsOpen() );
    node->ChangeSubTreeCount(+num);
}

void wxDataViewMainWindow::BuildTree(wxDataViewModel * model)
{
    DestroyTree();

    if (GetModel()->IsVirtualListModel())
    {
        InvalidateCount();
        return;
    }

    m_root = wxDataViewTreeNode::CreateRootNode(this);

        wxDataViewItem item;
    SortPrepare();
    BuildTreeHelper(this, model, item, m_root);
    InvalidateCount();
}

void wxDataViewMainWindow::DestroyTree()
{
    if (!IsVirtualList())
    {
        wxDELETE(m_root);
        m_count = 0;
    }
}

wxDataViewColumn*
wxDataViewMainWindow::FindColumnForEditing(const wxDataViewItem& item, wxDataViewCellMode mode)
{
                    
    wxDataViewColumn *candidate = m_currentCol;

    if ( candidate &&
         !IsCellEditableInMode(item, candidate, mode) &&
         !m_currentColSetByKeyboard )
    {
                                                                                candidate = NULL;
    }

    if ( !candidate )
    {
        const unsigned cols = GetOwner()->GetColumnCount();
        for ( unsigned i = 0; i < cols; i++ )
        {
            wxDataViewColumn *c = GetOwner()->GetColumnAt(i);
            if ( c->IsHidden() )
                continue;

            if ( IsCellEditableInMode(item, c, mode) )
            {
                candidate = c;
                break;
            }
        }
    }

            if ( candidate &&
         GetOwner()->GetExpanderColumn() != candidate &&
         GetModel()->IsContainer(item) &&
         !GetModel()->HasContainerColumns(item) )
    {
        candidate = GetOwner()->GetExpanderColumn();
    }

    if ( !candidate )
       return NULL;

   if ( !IsCellEditableInMode(item, candidate, mode) )
       return NULL;

   return candidate;
}

bool wxDataViewMainWindow::IsCellEditableInMode(const wxDataViewItem& item,
                                                const wxDataViewColumn *col,
                                                wxDataViewCellMode mode) const
{
    if ( col->GetRenderer()->GetMode() != mode )
        return false;

    if ( !GetModel()->IsEnabled(item, col->GetModelColumn()) )
        return false;

    return true;
}

void wxDataViewMainWindow::OnCharHook(wxKeyEvent& event)
{
    if ( m_editorCtrl )
    {
                        switch ( event.GetKeyCode() )
        {
            case WXK_ESCAPE:
                m_editorRenderer->CancelEditing();
                return;

            case WXK_RETURN:
                                if ( event.ShiftDown() )
                    break;
                wxFALLTHROUGH;

            case WXK_TAB:
                                                if ( event.HasModifiers() )
                    break;

                m_editorRenderer->FinishEditing();
                return;
        }
    }
    else if ( m_useCellFocus )
    {
        if ( event.GetKeyCode() == WXK_TAB && !event.HasModifiers() )
        {
            if ( event.ShiftDown() )
                OnLeftKey(event);
            else
                OnRightKey(event);
            return;
        }
    }

    event.Skip();
}

void wxDataViewMainWindow::OnChar( wxKeyEvent &event )
{
    wxWindow * const parent = GetParent();

        wxKeyEvent eventForParent(event);
    eventForParent.SetEventObject(parent);
    if ( parent->ProcessWindowEvent(eventForParent) )
        return;

    if ( parent->HandleAsNavigationKey(event) )
        return;

        if (!HasCurrentRow())
    {
        event.Skip();
        return;
    }

    switch ( event.GetKeyCode() )
    {
        case WXK_RETURN:
            if ( event.HasModifiers() )
            {
                event.Skip();
                break;
            }
            else
            {
                                                
                const wxDataViewItem item = GetItemByRow(m_currentRow);

                wxDataViewEvent le(wxEVT_DATAVIEW_ITEM_ACTIVATED,
                                   parent->GetId());
                le.SetItem(item);
                le.SetEventObject(parent);
                le.SetModel(GetModel());

                if ( parent->ProcessWindowEvent(le) )
                    break;
                            }

        case WXK_SPACE:
            if ( event.HasModifiers() )
            {
                event.Skip();
                break;
            }
            else
            {
                                                                
                const wxDataViewItem item = GetItemByRow(m_currentRow);

                                                                                wxDataViewColumn *activatableCol = FindColumnForEditing(item, wxDATAVIEW_CELL_ACTIVATABLE);

                if ( activatableCol )
                {
                    const unsigned colIdx = activatableCol->GetModelColumn();
                    const wxRect cell_rect = GetOwner()->GetItemRect(item, activatableCol);

                    wxDataViewRenderer *cell = activatableCol->GetRenderer();
                    cell->PrepareForItem(GetModel(), item, colIdx);
                    cell->WXActivateCell(cell_rect, GetModel(), item, colIdx, NULL);

                    break;
                }
                            }

        case WXK_F2:
            if ( event.HasModifiers() )
            {
                event.Skip();
                break;
            }
            else
            {
                if ( !m_selection.IsEmpty() )
                {
                                                                                unsigned sel;
                    if ( m_selection.IsSelected(m_currentRow) )
                    {
                        sel = m_currentRow;
                    }
                    else                     {
                        wxSelectionStore::IterationState cookie;
                        sel = m_selection.GetFirstSelectedItem(cookie);
                    }


                    const wxDataViewItem item = GetItemByRow(sel);

                                                                                wxDataViewColumn *editableCol = FindColumnForEditing(item, wxDATAVIEW_CELL_EDITABLE);

                    if ( editableCol )
                        GetOwner()->EditItem(item, editableCol);
                }
            }
            break;

        case WXK_UP:
            OnVerticalNavigation(event, -1);
            break;

        case WXK_DOWN:
            OnVerticalNavigation(event, +1);
            break;
                case WXK_LEFT:
            OnLeftKey(event);
            break;

        case WXK_RIGHT:
            OnRightKey(event);
            break;

        case WXK_END:
            OnVerticalNavigation(event, +(int)GetRowCount());
            break;

        case WXK_HOME:
            OnVerticalNavigation(event, -(int)GetRowCount());
            break;

        case WXK_PAGEUP:
            OnVerticalNavigation(event, -(GetCountPerPage() - 1));
            break;

        case WXK_PAGEDOWN:
            OnVerticalNavigation(event, +(GetCountPerPage() - 1));
            break;

        default:
            event.Skip();
    }
}

void wxDataViewMainWindow::OnVerticalNavigation(const wxKeyEvent& event, int delta)
{
        if (!HasCurrentRow() || IsEmpty())
        return;

    int newRow = (int)m_currentRow + delta;

        if ( newRow < 0 )
        newRow = 0;

    const int rowCount = (int)GetRowCount();
    if ( newRow >= rowCount )
        newRow = rowCount - 1;

    unsigned int oldCurrent = m_currentRow;
    unsigned int newCurrent = (unsigned int)newRow;

    if ( newCurrent == oldCurrent )
        return;

            if ( event.ShiftDown() && !IsSingleSel() )
    {
        RefreshRow( oldCurrent );

        ChangeCurrentRow( newCurrent );

                if ( oldCurrent > newCurrent )
        {
            newCurrent = oldCurrent;
            oldCurrent = m_currentRow;
        }

        SelectRows(oldCurrent, newCurrent);

        wxSelectionStore::IterationState cookie;
        const unsigned firstSel = m_selection.GetFirstSelectedItem(cookie);
        if ( firstSel != wxSelectionStore::NO_SELECTION )
            SendSelectionChangedEvent(GetItemByRow(firstSel));
    }
    else     {
        RefreshRow( oldCurrent );

                if ( !event.ControlDown() )
            UnselectAllRows();

        ChangeCurrentRow( newCurrent );

        if ( !event.ControlDown() )
        {
            SelectRow( m_currentRow, true );
            SendSelectionChangedEvent(GetItemByRow(m_currentRow));
        }
        else
            RefreshRow( m_currentRow );
    }

    GetOwner()->EnsureVisible( m_currentRow, -1 );
}

void wxDataViewMainWindow::OnLeftKey(wxKeyEvent& event)
{
    if ( IsList() )
    {
        TryAdvanceCurrentColumn(NULL, event, false);
    }
    else
    {
        wxDataViewTreeNode* node = GetTreeNodeByRow(m_currentRow);
        if ( !node )
            return;

        if ( TryAdvanceCurrentColumn(node, event, false) )
            return;

        const bool dontCollapseNodes = event.GetKeyCode() == WXK_TAB;
        if ( dontCollapseNodes )
        {
            m_currentCol = NULL;
                        event.Skip();
            return;
        }

                                if (node->HasChildren() && node->IsOpen())
        {
            Collapse(m_currentRow);
        }
        else
        {
                        wxDataViewTreeNode *parent_node = node->GetParent();

            if (parent_node)
            {
                int parent = GetRowByItem( parent_node->GetItem() );
                if ( parent >= 0 )
                {
                    unsigned int row = m_currentRow;
                    SelectRow( row, false);
                    SelectRow( parent, true );
                    ChangeCurrentRow( parent );
                    GetOwner()->EnsureVisible( parent, -1 );
                    SendSelectionChangedEvent( parent_node->GetItem() );
                }
            }
        }
    }
}

void wxDataViewMainWindow::OnRightKey(wxKeyEvent& event)
{
    if ( IsList() )
    {
        TryAdvanceCurrentColumn(NULL, event, true);
    }
    else
    {
        wxDataViewTreeNode* node = GetTreeNodeByRow(m_currentRow);
        if ( !node )
            return;

        if ( node->HasChildren() )
        {
            if ( !node->IsOpen() )
            {
                Expand( m_currentRow );
            }
            else
            {
                                unsigned int row = m_currentRow;
                SelectRow( row, false );
                SelectRow( row + 1, true );
                ChangeCurrentRow( row + 1 );
                GetOwner()->EnsureVisible( row + 1, -1 );
                SendSelectionChangedEvent( GetItemByRow(row+1) );
            }
        }
        else
        {
            TryAdvanceCurrentColumn(node, event, true);
        }
    }
}

bool wxDataViewMainWindow::TryAdvanceCurrentColumn(wxDataViewTreeNode *node, wxKeyEvent& event, bool forward)
{
    if ( GetOwner()->GetColumnCount() == 0 )
        return false;

    if ( !m_useCellFocus )
        return false;

    const bool wrapAround = event.GetKeyCode() == WXK_TAB;

    if ( node )
    {
                if ( node->HasChildren() && !GetModel()->HasContainerColumns(node->GetItem()) )
            return false;
    }

    if ( m_currentCol == NULL || !m_currentColSetByKeyboard )
    {
        if ( forward )
        {
            m_currentCol = GetOwner()->GetColumnAt(1);
            m_currentColSetByKeyboard = true;
            RefreshRow(m_currentRow);
            return true;
        }
        else
        {
            if ( !wrapAround )
                return false;
        }
    }

    int idx = GetOwner()->GetColumnIndex(m_currentCol) + (forward ? +1 : -1);

    if ( idx >= (int)GetOwner()->GetColumnCount() )
    {
        if ( !wrapAround )
            return false;

        if ( GetCurrentRow() < GetRowCount() - 1 )
        {
                        idx = 0;
            OnVerticalNavigation(wxKeyEvent(), +1);
        }
        else
        {
                        event.Skip();
            return false;
        }
    }

    if ( idx < 0 && wrapAround )
    {
        if ( GetCurrentRow() > 0 )
        {
                        idx = (int)GetOwner()->GetColumnCount() - 1;
            OnVerticalNavigation(wxKeyEvent(), -1);
        }
        else
        {
                        event.Skip();
            return false;
        }
    }

    GetOwner()->EnsureVisible(m_currentRow, idx);

    if ( idx < 1 )
    {
                        m_currentCol = NULL;
        RefreshRow(m_currentRow);
        return true;
    }

    m_currentCol = GetOwner()->GetColumnAt(idx);
    m_currentColSetByKeyboard = true;
    RefreshRow(m_currentRow);
    return true;
}

void wxDataViewMainWindow::OnMouse( wxMouseEvent &event )
{
    if (event.GetEventType() == wxEVT_MOUSEWHEEL)
    {
                event.Skip();
        return;
    }

    if(event.ButtonDown())
    {
                                event.Skip();
    }

    int x = event.GetX();
    int y = event.GetY();
    m_owner->CalcUnscrolledPosition( x, y, &x, &y );
    wxDataViewColumn *col = NULL;

    int xpos = 0;
    unsigned int cols = GetOwner()->GetColumnCount();
    unsigned int i;
    for (i = 0; i < cols; i++)
    {
        wxDataViewColumn *c = GetOwner()->GetColumnAt( i );
        if (c->IsHidden())
            continue;      
        if (x < xpos + c->GetWidth())
        {
            col = c;
            break;
        }
        xpos += c->GetWidth();
    }

    wxDataViewModel* const model = GetModel();

    const unsigned int current = GetLineAt( y );
    const wxDataViewItem item = GetItemByRow(current);

                if (event.RightUp())
    {
        wxWindow *parent = GetParent();
        wxDataViewEvent le(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, parent->GetId());
        le.SetEventObject(parent);
        le.SetModel(model);

        if ( item.IsOk() && col )
        {
            le.SetItem( item );
            le.SetColumn( col->GetModelColumn() );
            le.SetDataViewColumn( col );
        }

        parent->ProcessWindowEvent(le);
        return;
    }

#if wxUSE_DRAG_AND_DROP
    if (event.Dragging() || ((m_dragCount > 0) && event.Leaving()))
    {
        if (m_dragCount == 0)
        {
                                                m_dragStart = event.GetPosition();
        }

        m_dragCount++;
        if ((m_dragCount < 3) && (event.Leaving()))
            m_dragCount = 3;
        else if (m_dragCount != 3)
            return;

        if (event.LeftIsDown())
        {
            m_owner->CalcUnscrolledPosition( m_dragStart.x, m_dragStart.y,
                                             &m_dragStart.x, &m_dragStart.y );
            unsigned int drag_item_row = GetLineAt( m_dragStart.y );
            wxDataViewItem itemDragged = GetItemByRow( drag_item_row );

                        wxDataViewEvent evt( wxEVT_DATAVIEW_ITEM_BEGIN_DRAG, m_owner->GetId() );
            evt.SetEventObject( m_owner );
            evt.SetItem( itemDragged );
            evt.SetModel( model );
            if (!m_owner->HandleWindowEvent( evt ))
                return;

            if (!evt.IsAllowed())
                return;

            wxDataObject *obj = evt.GetDataObject();
            if (!obj)
                return;

            wxDataViewDropSource drag( this, drag_item_row );
            drag.SetData( *obj );
             drag.DoDragDrop(evt.GetDragFlags());
            delete obj;
        }
        return;
    }
    else
    {
        m_dragCount = 0;
    }
#endif 
        if ((current >= GetRowCount()) || !col)
    {
                        if (m_owner && (event.LeftDown() || event.RightDown()))
        {
            if (!m_selection.IsEmpty())
            {
                m_owner->UnselectAll();
                SendSelectionChangedEvent(wxDataViewItem());
            }
        }
        event.Skip();
        return;
    }

    wxDataViewRenderer *cell = col->GetRenderer();
    wxDataViewColumn* const
        expander = GetExpanderColumnOrFirstOne(GetOwner());

                bool hoverOverExpander = false;
    int itemOffset = 0;
    if ((!IsList()) && (expander == col))
    {
        wxDataViewTreeNode * node = GetTreeNodeByRow(current);

        int indent = node->GetIndentLevel();
        itemOffset = GetOwner()->GetIndent()*indent;

        if ( node->HasChildren() )
        {
                                    wxRect rect(xpos + itemOffset,
                        GetLineStart( current ) + (GetLineHeight(current) - m_lineHeight)/2,
                        m_lineHeight, m_lineHeight);

            if( rect.Contains(x, y) )
            {
                                hoverOverExpander = true;
                if (m_underMouse && m_underMouse != node)
                {
                                        RefreshRow(GetRowByItem(m_underMouse->GetItem()));
                }
                if (m_underMouse != node)
                {
                                        RefreshRow(current);
                }
                m_underMouse = node;
            }
        }

                        itemOffset += m_lineHeight;
    }
    if (!hoverOverExpander)
    {
        if (m_underMouse != NULL)
        {
                        RefreshRow(GetRowByItem(m_underMouse->GetItem()));
            m_underMouse = NULL;
        }
    }

    bool simulateClick = false;

    if (event.ButtonDClick())
    {
        m_renameTimer->Stop();
        m_lastOnSame = false;
    }

    bool ignore_other_columns =
        ((expander != col) &&
        (model->IsContainer(item)) &&
        (!model->HasContainerColumns(item)));

    if (event.LeftDClick())
    {
        if ( !hoverOverExpander && (current == m_lineLastClicked) )
        {
            wxWindow *parent = GetParent();
            wxDataViewEvent le(wxEVT_DATAVIEW_ITEM_ACTIVATED, parent->GetId());
            le.SetItem( item );
            le.SetColumn( col->GetModelColumn() );
            le.SetDataViewColumn( col );
            le.SetEventObject(parent);
            le.SetModel(GetModel());

            if ( parent->ProcessWindowEvent(le) )
            {
                                return;
            }
        }

                                                simulateClick = true;
    }

    if (event.LeftUp() && !hoverOverExpander)
    {
        if (m_lineSelectSingleOnUp != (unsigned int)-1)
        {
                        if ( UnselectAllRows(m_lineSelectSingleOnUp) )
            {
                SelectRow( m_lineSelectSingleOnUp, true );
                SendSelectionChangedEvent( GetItemByRow(m_lineSelectSingleOnUp) );
            }
                    }

                        if (m_lastOnSame && !ignore_other_columns)
        {
            if ((col == m_currentCol) && (current == m_currentRow) &&
                IsCellEditableInMode(item, col, wxDATAVIEW_CELL_EDITABLE) )
            {
                m_renameTimer->Start( 100, true );
            }
        }

        m_lastOnSame = false;
        m_lineSelectSingleOnUp = (unsigned int)-1;
    }
    else if(!event.LeftUp())
    {
                                        m_lineSelectSingleOnUp = (unsigned int)-1;
    }

    if (event.RightDown())
    {
        m_lineBeforeLastClicked = m_lineLastClicked;
        m_lineLastClicked = current;

                        if (!IsRowSelected(current))
        {
            UnselectAllRows();

            const unsigned oldCurrent = m_currentRow;
            ChangeCurrentRow(current);
            SelectRow(m_currentRow,true);
            RefreshRow(oldCurrent);
            SendSelectionChangedEvent(GetItemByRow( m_currentRow ) );
        }
    }
    else if (event.MiddleDown())
    {
    }

    if((event.LeftDown() || simulateClick) && hoverOverExpander)
    {
        wxDataViewTreeNode* node = GetTreeNodeByRow(current);

                                if( node->IsOpen() )
            Collapse(current);
        else
            Expand(current);
    }
    else if ((event.LeftDown() || simulateClick) && !hoverOverExpander)
    {
        m_lineBeforeLastClicked = m_lineLastClicked;
        m_lineLastClicked = current;

        unsigned int oldCurrentRow = m_currentRow;
        bool oldWasSelected = IsRowSelected(m_currentRow);

        bool cmdModifierDown = event.CmdDown();
        if ( IsSingleSel() || !(cmdModifierDown || event.ShiftDown()) )
        {
            if ( IsSingleSel() || !IsRowSelected(current) )
            {
                ChangeCurrentRow(current);
                if ( UnselectAllRows(current) )
                {
                    SelectRow(m_currentRow,true);
                    SendSelectionChangedEvent(GetItemByRow( m_currentRow ) );
                }
            }
            else             {
                m_lineSelectSingleOnUp = current;
                ChangeCurrentRow(current);             }
        }
        else         {
            if (cmdModifierDown)
            {
                ChangeCurrentRow(current);
                ReverseRowSelection(m_currentRow);
                SendSelectionChangedEvent(GetItemByRow(m_currentRow));
            }
            else if (event.ShiftDown())
            {
                ChangeCurrentRow(current);

                unsigned int lineFrom = oldCurrentRow,
                    lineTo = current;

                if ( lineFrom == static_cast<unsigned>(-1) )
                {
                                                            lineFrom = current;
                }

                if ( lineTo < lineFrom )
                {
                    lineTo = lineFrom;
                    lineFrom = m_currentRow;
                }

                SelectRows(lineFrom, lineTo);

                wxSelectionStore::IterationState cookie;
                const unsigned firstSel = m_selection.GetFirstSelectedItem(cookie);
                if ( firstSel != wxSelectionStore::NO_SELECTION )
                    SendSelectionChangedEvent(GetItemByRow(firstSel) );
            }
            else             {
                                wxFAIL_MSG( wxT("how did we get here?") );
            }
        }

        if (m_currentRow != oldCurrentRow)
            RefreshRow( oldCurrentRow );

        wxDataViewColumn *oldCurrentCol = m_currentCol;

                m_currentCol = col;
        m_currentColSetByKeyboard = false;

                                                        m_lastOnSame = !simulateClick && ((col == oldCurrentCol) &&
                        (current == oldCurrentRow)) && oldWasSelected &&
                        HasFocus();

                if ( IsCellEditableInMode(item, col, wxDATAVIEW_CELL_ACTIVATABLE) )
        {
            
            wxRect cell_rect( xpos + itemOffset,
                              GetLineStart( current ),
                              col->GetWidth() - itemOffset,
                              GetLineHeight( current ) );

                                                cell->PrepareForItem(model, item, col->GetModelColumn());

                                                                                                            
                        const int align = cell->GetEffectiveAlignment();

            wxRect rectItem = cell_rect;
            const wxSize size = cell->GetSize();
            if ( size.x >= 0 && size.x < cell_rect.width )
            {
                if ( align & wxALIGN_CENTER_HORIZONTAL )
                    rectItem.x += (cell_rect.width - size.x)/2;
                else if ( align & wxALIGN_RIGHT )
                    rectItem.x += cell_rect.width - size.x;
                            }

            if ( size.y >= 0 && size.y < cell_rect.height )
            {
                if ( align & wxALIGN_CENTER_VERTICAL )
                    rectItem.y += (cell_rect.height - size.y)/2;
                else if ( align & wxALIGN_BOTTOM )
                    rectItem.y += cell_rect.height - size.y;
                            }

            wxMouseEvent event2(event);
            event2.m_x -= rectItem.x;
            event2.m_y -= rectItem.y;
            m_owner->CalcUnscrolledPosition(event2.m_x, event2.m_y, &event2.m_x, &event2.m_y);

              cell->WXActivateCell
                                    (
                                        cell_rect,
                                        model,
                                        item,
                                        col->GetModelColumn(),
                                        &event2
                                    );
        }
    }
}

void wxDataViewMainWindow::OnSetFocus( wxFocusEvent &event )
{
    m_hasFocus = true;

            if ( !HasCurrentRow() && !IsEmpty() )
    {
        ChangeCurrentRow(0);
    }

    if (HasCurrentRow())
        Refresh();

    event.Skip();
}

void wxDataViewMainWindow::OnKillFocus( wxFocusEvent &event )
{
    m_hasFocus = false;

    if (HasCurrentRow())
        Refresh();

    event.Skip();
}

void wxDataViewMainWindow::OnColumnsCountChanged()
{
    int editableCount = 0;

    const unsigned cols = GetOwner()->GetColumnCount();
    for ( unsigned i = 0; i < cols; i++ )
    {
        wxDataViewColumn *c = GetOwner()->GetColumnAt(i);
        if ( c->IsHidden() )
            continue;
        if ( c->GetRenderer()->GetMode() != wxDATAVIEW_CELL_INERT )
            editableCount++;
    }

    m_useCellFocus = (editableCount > 0);

    UpdateDisplay();
}

void wxDataViewMainWindow::UpdateColumnSizes()
{
    int colsCount = GetOwner()->GetColumnCount();
    if ( !colsCount )
        return;

    wxDataViewCtrl *owner = GetOwner();

    int fullWinWidth = GetSize().x;

    wxDataViewColumn *lastCol = owner->GetColumn(colsCount - 1);
    int colswidth = GetEndOfLastCol();
    int lastColX = colswidth - lastCol->GetWidth();
    if ( lastColX < fullWinWidth )
    {
        int desiredWidth = wxMax(fullWinWidth - lastColX, lastCol->GetMinWidth());
        lastCol->SetWidth(desiredWidth);

                                        SetVirtualSize(0, m_virtualSize.y);

        RefreshRect(wxRect(lastColX, 0, fullWinWidth - lastColX, GetSize().y));
    }
    else
    {
                SetVirtualSize(colswidth, m_virtualSize.y);
    }
}


WX_DEFINE_LIST(wxDataViewColumnList)

wxIMPLEMENT_DYNAMIC_CLASS(wxDataViewCtrl, wxDataViewCtrlBase);
wxBEGIN_EVENT_TABLE(wxDataViewCtrl, wxDataViewCtrlBase)
    EVT_SIZE(wxDataViewCtrl::OnSize)
wxEND_EVENT_TABLE()

wxDataViewCtrl::~wxDataViewCtrl()
{
    if (m_notifier)
        GetModel()->RemoveNotifier( m_notifier );

    m_cols.Clear();
    m_colsBestWidths.clear();
}

void wxDataViewCtrl::Init()
{
    m_cols.DeleteContents(true);
    m_notifier = NULL;

    m_headerArea = NULL;
    m_clientArea = NULL;

    m_colsDirty = false;

    m_allowMultiColumnSort = false;
}

bool wxDataViewCtrl::Create(wxWindow *parent,
                            wxWindowID id,
                            const wxPoint& pos,
                            const wxSize& size,
                            long style,
                            const wxValidator& validator,
                            const wxString& name)
{

    Init();

    if (!wxControl::Create( parent, id, pos, size,
                            style | wxScrolledWindowStyle, validator, name))
        return false;

    SetInitialSize(size);

#ifdef __WXMAC__
    MacSetClipChildren( true );
#endif

    m_clientArea = new wxDataViewMainWindow( this, wxID_ANY );

                DisableKeyboardScrolling();

    if (HasFlag(wxDV_NO_HEADER))
        m_headerArea = NULL;
    else
        m_headerArea = new wxDataViewHeaderWindow(this);

    SetTargetWindow( m_clientArea );

    wxBoxSizer *sizer = new wxBoxSizer( wxVERTICAL );
    if (m_headerArea)
        sizer->Add( m_headerArea, 0, wxGROW );
    sizer->Add( m_clientArea, 1, wxGROW );
    SetSizer( sizer );

    EnableSystemTheme();

    return true;
}

wxBorder wxDataViewCtrl::GetDefaultBorder() const
{
    return wxBORDER_THEME;
}

#ifdef __WXMSW__
WXLRESULT wxDataViewCtrl::MSWWindowProc(WXUINT nMsg,
                                        WXWPARAM wParam,
                                        WXLPARAM lParam)
{
    WXLRESULT rc = wxDataViewCtrlBase::MSWWindowProc(nMsg, wParam, lParam);

        if ( nMsg == WM_GETDLGCODE )
    {
        rc |= DLGC_WANTARROWS;
    }

    return rc;
}
#endif

wxSize wxDataViewCtrl::GetSizeAvailableForScrollTarget(const wxSize& size)
{
    wxSize newsize = size;
    if (!HasFlag(wxDV_NO_HEADER) && (m_headerArea))
    newsize.y -= m_headerArea->GetSize().y;

    return newsize;
}

void wxDataViewCtrl::OnSize( wxSizeEvent &WXUNUSED(event) )
{
    if ( m_clientArea && GetColumnCount() )
        m_clientArea->UpdateColumnSizes();

                        
    Layout();

    AdjustScrollbars();

                    if ( !HasFlag(wxDV_NO_HEADER) && m_headerArea &&
            m_headerArea->GetSize().y <= m_headerArea->GetBestSize(). y )
    {
        m_headerArea->Refresh();
    }
}

void wxDataViewCtrl::SetFocus()
{
    if (m_clientArea)
        m_clientArea->SetFocus();
}

bool wxDataViewCtrl::SetFont(const wxFont & font)
{
    if (!wxControl::SetFont(font))
        return false;

    if (m_headerArea)
        m_headerArea->SetFont(font);

    if (m_clientArea)
    {
        m_clientArea->SetFont(font);
        m_clientArea->SetRowHeight(m_clientArea->GetDefaultRowHeight());
    }

    if (m_headerArea || m_clientArea)
    {
        InvalidateColBestWidths();
        Layout();
    }

    return true;
}



bool wxDataViewCtrl::AssociateModel( wxDataViewModel *model )
{
    if (!wxDataViewCtrlBase::AssociateModel( model ))
        return false;

    if (model)
    {
        m_notifier = new wxGenericDataViewModelNotifier( m_clientArea );
        model->AddNotifier( m_notifier );
    }
    else
    {
                                        m_notifier = NULL;
    }

    m_clientArea->DestroyTree();

    if (model)
    {
        m_clientArea->BuildTree(model);
    }

    m_clientArea->UpdateDisplay();

    return true;
}

#if wxUSE_DRAG_AND_DROP

bool wxDataViewCtrl::EnableDragSource( const wxDataFormat &format )
{
    return m_clientArea->EnableDragSource( format );
}

bool wxDataViewCtrl::EnableDropTarget( const wxDataFormat &format )
{
    return m_clientArea->EnableDropTarget( format );
}

#endif 
bool wxDataViewCtrl::AppendColumn( wxDataViewColumn *col )
{
    if (!wxDataViewCtrlBase::AppendColumn(col))
        return false;

    m_cols.Append( col );
    m_colsBestWidths.push_back(CachedColWidthInfo());
    OnColumnsCountChanged();
    return true;
}

bool wxDataViewCtrl::PrependColumn( wxDataViewColumn *col )
{
    if (!wxDataViewCtrlBase::PrependColumn(col))
        return false;

    m_cols.Insert( col );
    m_colsBestWidths.insert(m_colsBestWidths.begin(), CachedColWidthInfo());
    OnColumnsCountChanged();
    return true;
}

bool wxDataViewCtrl::InsertColumn( unsigned int pos, wxDataViewColumn *col )
{
    if (!wxDataViewCtrlBase::InsertColumn(pos,col))
        return false;

    m_cols.Insert( pos, col );
    m_colsBestWidths.insert(m_colsBestWidths.begin() + pos, CachedColWidthInfo());
    OnColumnsCountChanged();
    return true;
}

void wxDataViewCtrl::OnColumnChange(unsigned int idx)
{
    if ( m_headerArea )
        m_headerArea->UpdateColumn(idx);

    m_clientArea->UpdateDisplay();
}

void wxDataViewCtrl::OnColumnsCountChanged()
{
    if (m_headerArea)
        m_headerArea->SetColumnCount(GetColumnCount());

    m_clientArea->OnColumnsCountChanged();
}

void wxDataViewCtrl::DoSetExpanderColumn()
{
    wxDataViewColumn* column = GetExpanderColumn();
    if ( column )
    {
        int index = GetColumnIndex(column);
        if ( index != wxNOT_FOUND )
            InvalidateColBestWidth(index);
    }

    m_clientArea->UpdateDisplay();
}

void wxDataViewCtrl::DoSetIndent()
{
    m_clientArea->UpdateDisplay();
}

unsigned int wxDataViewCtrl::GetColumnCount() const
{
    return m_cols.GetCount();
}

bool wxDataViewCtrl::SetRowHeight( int lineHeight )
{
    if ( !m_clientArea )
        return false;

    m_clientArea->SetRowHeight(lineHeight);

    return true;
}

wxDataViewColumn* wxDataViewCtrl::GetColumn( unsigned int idx ) const
{
    return m_cols[idx];
}

wxDataViewColumn *wxDataViewCtrl::GetColumnAt(unsigned int pos) const
{
            const unsigned idx = m_headerArea ? m_headerArea->GetColumnsOrder()[pos]
                                    : pos;

    return GetColumn(idx);
}

int wxDataViewCtrl::GetColumnIndex(const wxDataViewColumn *column) const
{
    const unsigned count = m_cols.size();
    for ( unsigned n = 0; n < count; n++ )
    {
        if ( m_cols[n] == column )
            return n;
    }

    return wxNOT_FOUND;
}

int wxDataViewCtrl::GetModelColumnIndex( unsigned int model_column ) const
{
    const int count = GetColumnCount();
    for ( int index = 0; index < count; index++ )
    {
        wxDataViewColumn* column = GetColumn(index);
        if ( column->GetModelColumn() == model_column )
            return index;
    }
    return wxNOT_FOUND;
}

class wxDataViewMaxWidthCalculator : public wxMaxWidthCalculatorBase
{
public:
    wxDataViewMaxWidthCalculator(const wxDataViewCtrl *dvc,
                                 wxDataViewMainWindow *clientArea,
                                 wxDataViewRenderer *renderer,
                                 const wxDataViewModel *model,
                                 size_t model_column,
                                 int expanderSize)
        : wxMaxWidthCalculatorBase(model_column),
          m_dvc(dvc),
          m_clientArea(clientArea),
          m_renderer(renderer),
          m_model(model),
          m_expanderSize(expanderSize)
    {
        int index = dvc->GetModelColumnIndex( model_column );
        wxDataViewColumn* column = index == wxNOT_FOUND ? NULL : dvc->GetColumn(index);
        m_isExpanderCol =
            !clientArea->IsList() &&
            (column == 0 ||
             GetExpanderColumnOrFirstOne(const_cast<wxDataViewCtrl*>(dvc)) == column );
    }

    virtual void UpdateWithRow(int row) wxOVERRIDE
    {
        int indent = 0;
        wxDataViewItem item;

        if ( m_isExpanderCol )
        {
            wxDataViewTreeNode *node = m_clientArea->GetTreeNodeByRow(row);
            item = node->GetItem();
            indent = m_dvc->GetIndent() * node->GetIndentLevel() + m_expanderSize;
        }
        else
        {
            item = m_clientArea->GetItemByRow(row);
        }

        m_renderer->PrepareForItem(m_model, item, GetColumn());
        UpdateWithWidth(m_renderer->GetSize().x + indent);
    }

private:
    const wxDataViewCtrl *m_dvc;
    wxDataViewMainWindow *m_clientArea;
    wxDataViewRenderer *m_renderer;
    const wxDataViewModel *m_model;
    bool m_isExpanderCol;
    int m_expanderSize;
};


unsigned int wxDataViewCtrl::GetBestColumnWidth(int idx) const
{
    if ( m_colsBestWidths[idx].width != 0 )
        return m_colsBestWidths[idx].width;

    const int count = m_clientArea->GetRowCount();
    wxDataViewColumn *column = GetColumn(idx);
    wxDataViewRenderer *renderer =
        const_cast<wxDataViewRenderer*>(column->GetRenderer());

    wxDataViewMaxWidthCalculator calculator(this, m_clientArea, renderer,
                                            GetModel(), column->GetModelColumn(),
                                            m_clientArea->GetRowHeight());

    calculator.UpdateWithWidth(column->GetMinWidth());

    if ( m_headerArea )
        calculator.UpdateWithWidth(m_headerArea->GetColumnTitleWidth(*column));

    const wxPoint origin = CalcUnscrolledPosition(wxPoint(0, 0));
    calculator.ComputeBestColumnWidth(count,
                                      m_clientArea->GetLineAt(origin.y),
                                      m_clientArea->GetLineAt(origin.y + GetClientSize().y));

    int max_width = calculator.GetMaxWidth();
    if ( max_width > 0 )
        max_width += 2 * PADDING_RIGHTLEFT;

    const_cast<wxDataViewCtrl*>(this)->m_colsBestWidths[idx].width = max_width;
    return max_width;
}

void wxDataViewCtrl::ColumnMoved(wxDataViewColumn * WXUNUSED(col),
                                unsigned int WXUNUSED(new_pos))
{
                m_clientArea->UpdateDisplay();
}

bool wxDataViewCtrl::DeleteColumn( wxDataViewColumn *column )
{
    wxDataViewColumnList::compatibility_iterator ret = m_cols.Find( column );
    if (!ret)
        return false;

    m_colsBestWidths.erase(m_colsBestWidths.begin() + GetColumnIndex(column));
    m_cols.Erase(ret);

    if ( m_clientArea->GetCurrentColumn() == column )
        m_clientArea->ClearCurrentColumn();

    OnColumnsCountChanged();

    return true;
}

bool wxDataViewCtrl::ClearColumns()
{
    SetExpanderColumn(NULL);
    m_cols.Clear();
    m_sortingColumnIdxs.clear();
    m_colsBestWidths.clear();

    m_clientArea->ClearCurrentColumn();

    OnColumnsCountChanged();

    return true;
}

void wxDataViewCtrl::InvalidateColBestWidth(int idx)
{
    m_colsBestWidths[idx].width = 0;
    m_colsBestWidths[idx].dirty = true;
    m_colsDirty = true;
}

void wxDataViewCtrl::InvalidateColBestWidths()
{
        m_colsBestWidths.clear();
    m_colsBestWidths.resize(m_cols.size());
    m_colsDirty = true;
}

void wxDataViewCtrl::UpdateColWidths()
{
    m_colsDirty = false;

    if ( !m_headerArea )
        return;

    const unsigned len = m_colsBestWidths.size();
    for ( unsigned i = 0; i < len; i++ )
    {
                                                                        if ( m_colsBestWidths[i].dirty )
        {
            m_headerArea->UpdateColumn(i);
            m_colsBestWidths[i].dirty = false;
        }
    }
}

void wxDataViewCtrl::OnInternalIdle()
{
    wxDataViewCtrlBase::OnInternalIdle();

    if ( m_colsDirty )
        UpdateColWidths();
}

int wxDataViewCtrl::GetColumnPosition( const wxDataViewColumn *column ) const
{
    unsigned int len = GetColumnCount();
    for ( unsigned int i = 0; i < len; i++ )
    {
        wxDataViewColumn * col = GetColumnAt(i);
        if (column==col)
            return i;
    }

    return wxNOT_FOUND;
}

wxDataViewColumn *wxDataViewCtrl::GetSortingColumn() const
{
    if ( m_sortingColumnIdxs.empty() )
        return NULL;

    return GetColumn(m_sortingColumnIdxs.front());
}

wxVector<wxDataViewColumn *> wxDataViewCtrl::GetSortingColumns() const
{
    wxVector<wxDataViewColumn *> out;

    for ( wxVector<int>::const_iterator it = m_sortingColumnIdxs.begin(),
                                       end = m_sortingColumnIdxs.end();
          it != end;
          ++it )
    {
        out.push_back(GetColumn(*it));
    }

    return out;
}

wxDataViewItem wxDataViewCtrl::DoGetCurrentItem() const
{
    return GetItemByRow(m_clientArea->GetCurrentRow());
}

void wxDataViewCtrl::DoSetCurrentItem(const wxDataViewItem& item)
{
    const int row = m_clientArea->GetRowByItem(item);

    const unsigned oldCurrent = m_clientArea->GetCurrentRow();
    if ( static_cast<unsigned>(row) != oldCurrent )
    {
        m_clientArea->ChangeCurrentRow(row);
        m_clientArea->RefreshRow(oldCurrent);
        m_clientArea->RefreshRow(row);
    }
}

wxDataViewColumn *wxDataViewCtrl::GetCurrentColumn() const
{
    return m_clientArea->GetCurrentColumn();
}

int wxDataViewCtrl::GetSelectedItemsCount() const
{
    return m_clientArea->GetSelections().GetSelectedCount();
}

int wxDataViewCtrl::GetSelections( wxDataViewItemArray & sel ) const
{
    sel.Empty();
    const wxSelectionStore& selections = m_clientArea->GetSelections();

    wxSelectionStore::IterationState cookie;
    for ( unsigned row = selections.GetFirstSelectedItem(cookie);
          row != wxSelectionStore::NO_SELECTION;
          row = selections.GetNextSelectedItem(cookie) )
    {
        wxDataViewItem item = m_clientArea->GetItemByRow(row);
        if ( item.IsOk() )
        {
            sel.Add(item);
        }
        else
        {
            wxFAIL_MSG( "invalid item in selection - bad internal state" );
        }
    }

    return sel.size();
}

void wxDataViewCtrl::SetSelections( const wxDataViewItemArray & sel )
{
    m_clientArea->ClearSelection();

    wxDataViewItem last_parent;

    for ( size_t i = 0; i < sel.size(); i++ )
    {
        wxDataViewItem item = sel[i];
        wxDataViewItem parent = GetModel()->GetParent( item );
        if (parent)
        {
            if (parent != last_parent)
                ExpandAncestors(item);
        }

        last_parent = parent;
        int row = m_clientArea->GetRowByItem( item );
        if( row >= 0 )
            m_clientArea->SelectRow(static_cast<unsigned int>(row), true);
    }
}

void wxDataViewCtrl::Select( const wxDataViewItem & item )
{
    ExpandAncestors( item );

    int row = m_clientArea->GetRowByItem( item );
    if( row >= 0 )
    {
                if (m_clientArea->IsSingleSel())
            m_clientArea->UnselectAllRows();

        m_clientArea->SelectRow(row, true);

                m_clientArea->ChangeCurrentRow( row );
    }
}

void wxDataViewCtrl::Unselect( const wxDataViewItem & item )
{
    int row = m_clientArea->GetRowByItem( item );
    if( row >= 0 )
        m_clientArea->SelectRow(row, false);
}

bool wxDataViewCtrl::IsSelected( const wxDataViewItem & item ) const
{
    int row = m_clientArea->GetRowByItem( item );
    if( row >= 0 )
    {
        return m_clientArea->IsRowSelected(row);
    }
    return false;
}

void wxDataViewCtrl::SetAlternateRowColour(const wxColour& colour)
{
    m_alternateRowColour = colour;
}

void wxDataViewCtrl::SelectAll()
{
    m_clientArea->SelectAllRows();
}

void wxDataViewCtrl::UnselectAll()
{
    m_clientArea->UnselectAllRows();
}

void wxDataViewCtrl::EnsureVisible( int row, int column )
{
    if( row < 0 )
        row = 0;
    if( row > (int) m_clientArea->GetRowCount() )
        row = m_clientArea->GetRowCount();

    int first = m_clientArea->GetFirstVisibleRow();
    int last = m_clientArea->GetLastVisibleRow();
    if( row < first )
        m_clientArea->ScrollTo( row, column );
    else if( row > last )
        m_clientArea->ScrollTo( row - last + first, column );
    else
        m_clientArea->ScrollTo( first, column );
}

void wxDataViewCtrl::EnsureVisible( const wxDataViewItem & item, const wxDataViewColumn * column )
{
    ExpandAncestors( item );

    m_clientArea->RecalculateDisplay();

    int row = m_clientArea->GetRowByItem(item);
    if( row >= 0 )
    {
        if( column == NULL )
            EnsureVisible(row, -1);
        else
            EnsureVisible( row, GetColumnIndex(column) );
    }

}

void wxDataViewCtrl::HitTest( const wxPoint & point, wxDataViewItem & item,
                              wxDataViewColumn* &column ) const
{
    m_clientArea->HitTest(point, item, column);
}

wxRect wxDataViewCtrl::GetItemRect( const wxDataViewItem & item,
                                    const wxDataViewColumn* column ) const
{
    return m_clientArea->GetItemRect(item, column);
}

wxDataViewItem wxDataViewCtrl::GetItemByRow( unsigned int row ) const
{
    return m_clientArea->GetItemByRow( row );
}

int wxDataViewCtrl::GetRowByItem( const wxDataViewItem & item ) const
{
    return m_clientArea->GetRowByItem( item );
}

void wxDataViewCtrl::Expand( const wxDataViewItem & item )
{
    ExpandAncestors( item );

    int row = m_clientArea->GetRowByItem( item );
    if (row != -1)
        m_clientArea->Expand(row);
}

void wxDataViewCtrl::Collapse( const wxDataViewItem & item )
{
    int row = m_clientArea->GetRowByItem( item );
    if (row != -1)
        m_clientArea->Collapse(row);
}

bool wxDataViewCtrl::IsExpanded( const wxDataViewItem & item ) const
{
    int row = m_clientArea->GetRowByItem( item );
    if (row != -1)
        return m_clientArea->IsExpanded(row);
    return false;
}

void wxDataViewCtrl::EditItem(const wxDataViewItem& item, const wxDataViewColumn *column)
{
    wxCHECK_RET( item.IsOk(), "invalid item" );
    wxCHECK_RET( column, "no column provided" );

    m_clientArea->StartEditing(item, column);
}

void wxDataViewCtrl::ResetAllSortColumns()
{
        wxVector<int> const copy(m_sortingColumnIdxs);
    for ( wxVector<int>::const_iterator it = copy.begin(),
                                       end = copy.end();
          it != end;
          ++it )
    {
        GetColumn(*it)->UnsetAsSortKey();
    }

    wxASSERT( m_sortingColumnIdxs.empty() );
}

bool wxDataViewCtrl::AllowMultiColumnSort(bool allow)
{
    if ( m_allowMultiColumnSort == allow )
        return true;

    m_allowMultiColumnSort = allow;

        if ( !allow )
    {
        ResetAllSortColumns();

        if ( wxDataViewModel *model = GetModel() )
            model->Resort();
    }

    return true;
}


bool wxDataViewCtrl::IsColumnSorted(int idx) const
{
    for ( wxVector<int>::const_iterator it = m_sortingColumnIdxs.begin(),
                                       end = m_sortingColumnIdxs.end();
          it != end;
          ++it )
    {
        if ( *it == idx )
            return true;
    }

    return false;
}

void wxDataViewCtrl::UseColumnForSorting(int idx )
{
    m_sortingColumnIdxs.push_back(idx);
}

void wxDataViewCtrl::DontUseColumnForSorting(int idx)
{
    for ( wxVector<int>::iterator it = m_sortingColumnIdxs.begin(),
                                 end = m_sortingColumnIdxs.end();
          it != end;
          ++it )
    {
        if ( *it == idx )
        {
            m_sortingColumnIdxs.erase(it);
            return;
        }
    }

    wxFAIL_MSG( "Column is not used for sorting" );
}

void wxDataViewCtrl::ToggleSortByColumn(int column)
{
    m_headerArea->ToggleSortByColumn(column);
}

void wxDataViewCtrl::DoEnableSystemTheme(bool enable, wxWindow* window)
{
    typedef wxSystemThemedControl<wxControl> Base;
    Base::DoEnableSystemTheme(enable, window);
    Base::DoEnableSystemTheme(enable, m_clientArea);
    if ( m_headerArea )
        Base::DoEnableSystemTheme(enable, m_headerArea);
}

#endif 
#endif 