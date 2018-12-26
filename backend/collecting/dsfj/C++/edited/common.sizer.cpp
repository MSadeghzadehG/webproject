
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/sizer.h"
#include "wx/private/flagscheck.h"

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/intl.h"
    #include "wx/math.h"
    #include "wx/utils.h"
    #include "wx/settings.h"
    #include "wx/button.h"
    #include "wx/statbox.h"
    #include "wx/toplevel.h"
#endif 
#include "wx/display.h"
#include "wx/vector.h"
#include "wx/listimpl.cpp"



wxIMPLEMENT_CLASS(wxSizerItem, wxObject);
wxIMPLEMENT_CLASS(wxSizer, wxObject);
wxIMPLEMENT_CLASS(wxGridSizer, wxSizer);
wxIMPLEMENT_CLASS(wxFlexGridSizer, wxGridSizer);
wxIMPLEMENT_CLASS(wxBoxSizer, wxSizer);
#if wxUSE_STATBOX
wxIMPLEMENT_CLASS(wxStaticBoxSizer, wxBoxSizer);
#endif
#if wxUSE_BUTTON
wxIMPLEMENT_CLASS(wxStdDialogButtonSizer, wxBoxSizer);
#endif

WX_DEFINE_EXPORTED_LIST( wxSizerItemList )




#ifdef wxNEEDS_BORDER_IN_PX

int wxSizerFlags::ms_defaultBorderInPx = 0;


int wxSizerFlags::DoGetDefaultBorderInPx()
{
                                                    return wxWindow::FromDIP(5, NULL);
}

#endif 

#if wxDEBUG_LEVEL

static const int SIZER_FLAGS_MASK =
    wxADD_FLAG(wxCENTRE,
    wxADD_FLAG(wxHORIZONTAL,
    wxADD_FLAG(wxVERTICAL,
    wxADD_FLAG(wxLEFT,
    wxADD_FLAG(wxRIGHT,
    wxADD_FLAG(wxUP,
    wxADD_FLAG(wxDOWN,
    wxADD_FLAG(wxALIGN_NOT,
    wxADD_FLAG(wxALIGN_CENTER_HORIZONTAL,
    wxADD_FLAG(wxALIGN_RIGHT,
    wxADD_FLAG(wxALIGN_BOTTOM,
    wxADD_FLAG(wxALIGN_CENTER_VERTICAL,
    wxADD_FLAG(wxFIXED_MINSIZE,
    wxADD_FLAG(wxRESERVE_SPACE_EVEN_IF_HIDDEN,
    wxADD_FLAG(wxSTRETCH_NOT,
    wxADD_FLAG(wxSHRINK,
    wxADD_FLAG(wxGROW,
    wxADD_FLAG(wxSHAPED,
    0))))))))))))))))));

#endif 
#define ASSERT_INCOMPATIBLE_NOT_USED_IMPL(f, f1, n1, f2, n2) \
    wxASSERT_MSG(((f) & (f1 | f2)) != (f1 | f2), \
                 n1 " and " n2 " can't be used together")

#define ASSERT_INCOMPATIBLE_NOT_USED(f, f1, f2) \
    ASSERT_INCOMPATIBLE_NOT_USED_IMPL(f, f1, #f1, f2, #f2)

#define ASSERT_VALID_SIZER_FLAGS(f) \
    wxASSERT_VALID_FLAGS(f, SIZER_FLAGS_MASK); \
    ASSERT_INCOMPATIBLE_NOT_USED(f, wxALIGN_CENTRE_HORIZONTAL, wxALIGN_RIGHT); \
    ASSERT_INCOMPATIBLE_NOT_USED(f, wxALIGN_CENTRE_VERTICAL, wxALIGN_BOTTOM)


void wxSizerItem::Init(const wxSizerFlags& flags)
{
    Init();

    m_proportion = flags.GetProportion();
    m_flag = flags.GetFlags();
    m_border = flags.GetBorderInPixels();

    ASSERT_VALID_SIZER_FLAGS( m_flag );
}

wxSizerItem::wxSizerItem()
{
    Init();

    m_proportion = 0;
    m_border = 0;
    m_flag = 0;
    m_id = wxID_NONE;
}

void wxSizerItem::DoSetWindow(wxWindow *window)
{
    wxCHECK_RET( window, wxT("NULL window in wxSizerItem::SetWindow()") );

    m_kind = Item_Window;
    m_window = window;

        m_minSize = window->GetSize();

    if ( m_flag & wxFIXED_MINSIZE )
        window->SetMinSize(m_minSize);

        SetRatio(m_minSize);
}

wxSizerItem::wxSizerItem(wxWindow *window,
                         int proportion,
                         int flag,
                         int border,
                         wxObject* userData)
           : m_kind(Item_None),
             m_proportion(proportion),
             m_border(border),
             m_flag(flag),
             m_id(wxID_NONE),
             m_userData(userData)
{
    ASSERT_VALID_SIZER_FLAGS( m_flag );

    DoSetWindow(window);
}

void wxSizerItem::DoSetSizer(wxSizer *sizer)
{
    m_kind = Item_Sizer;
    m_sizer = sizer;
}

wxSizerItem::wxSizerItem(wxSizer *sizer,
                         int proportion,
                         int flag,
                         int border,
                         wxObject* userData)
           : m_kind(Item_None),
             m_sizer(NULL),
             m_proportion(proportion),
             m_border(border),
             m_flag(flag),
             m_id(wxID_NONE),
             m_ratio(0.0),
             m_userData(userData)
{
    ASSERT_VALID_SIZER_FLAGS( m_flag );

    DoSetSizer(sizer);

    }

void wxSizerItem::DoSetSpacer(const wxSize& size)
{
    m_kind = Item_Spacer;
    m_spacer = new wxSizerSpacer(size);
    m_minSize = size;
    SetRatio(size);
}

wxSize wxSizerItem::AddBorderToSize(const wxSize& size) const
{
    wxSize result = size;

            
    if ( result.x != wxDefaultCoord )
    {
        if (m_flag & wxWEST)
            result.x += m_border;
        if (m_flag & wxEAST)
            result.x += m_border;
    }

    if ( result.y != wxDefaultCoord )
    {
        if (m_flag & wxNORTH)
            result.y += m_border;
        if (m_flag & wxSOUTH)
            result.y += m_border;
    }

    return result;
}

wxSizerItem::wxSizerItem(int width,
                         int height,
                         int proportion,
                         int flag,
                         int border,
                         wxObject* userData)
           : m_kind(Item_None),
             m_sizer(NULL),
             m_minSize(width, height),              m_proportion(proportion),
             m_border(border),
             m_flag(flag),
             m_id(wxID_NONE),
             m_userData(userData)
{
    ASSERT_VALID_SIZER_FLAGS( m_flag );

    DoSetSpacer(wxSize(width, height));
}

wxSizerItem::~wxSizerItem()
{
    delete m_userData;
    Free();
}

void wxSizerItem::Free()
{
    switch ( m_kind )
    {
        case Item_None:
            break;

        case Item_Window:
            m_window->SetContainingSizer(NULL);
            break;

        case Item_Sizer:
            delete m_sizer;
            break;

        case Item_Spacer:
            delete m_spacer;
            break;

        case Item_Max:
        default:
            wxFAIL_MSG( wxT("unexpected wxSizerItem::m_kind") );
    }

    m_kind = Item_None;
}

wxSize wxSizerItem::GetSpacer() const
{
    wxSize size;
    if ( m_kind == Item_Spacer )
        size = m_spacer->GetSize();

    return size;
}


wxSize wxSizerItem::GetSize() const
{
    wxSize ret;
    switch ( m_kind )
    {
        case Item_None:
            break;

        case Item_Window:
            ret = m_window->GetSize();
            break;

        case Item_Sizer:
            ret = m_sizer->GetSize();
            break;

        case Item_Spacer:
            ret = m_spacer->GetSize();
            break;

        case Item_Max:
        default:
            wxFAIL_MSG( wxT("unexpected wxSizerItem::m_kind") );
    }

    if (m_flag & wxWEST)
        ret.x += m_border;
    if (m_flag & wxEAST)
        ret.x += m_border;
    if (m_flag & wxNORTH)
        ret.y += m_border;
    if (m_flag & wxSOUTH)
        ret.y += m_border;

    return ret;
}

bool wxSizerItem::InformFirstDirection(int direction, int size, int availableOtherDir)
{
            if( size>0 )
    {
        if( direction==wxHORIZONTAL )
        {
            if (m_flag & wxWEST)
                size -= m_border;
            if (m_flag & wxEAST)
                size -= m_border;
        }
        else if( direction==wxVERTICAL )
        {
            if (m_flag & wxNORTH)
                size -= m_border;
            if (m_flag & wxSOUTH)
                size -= m_border;
        }
    }

    bool didUse = false;
        if (IsSizer())
    {
        didUse = GetSizer()->InformFirstDirection(direction,size,availableOtherDir);
        if (didUse)
            m_minSize = GetSizer()->CalcMin();
    }
    else if (IsWindow())
    {
        didUse =  GetWindow()->InformFirstDirection(direction,size,availableOtherDir);
        if (didUse)
            m_minSize = m_window->GetEffectiveMinSize();

                                        if( (m_flag & wxSHAPED) && (m_flag & wxEXPAND) && direction )
        {
            if( !wxIsNullDouble(m_ratio) )
            {
                wxCHECK_MSG( m_proportion==0, false, wxT("Shaped item, non-zero proportion in wxSizerItem::InformFirstDirection()") );
                if( direction==wxHORIZONTAL && !wxIsNullDouble(m_ratio) )
                {
                                        if( availableOtherDir>=0 && int(size/m_ratio)-m_minSize.y>availableOtherDir )
                        size = int((availableOtherDir+m_minSize.y)*m_ratio);
                    m_minSize = wxSize(size,int(size/m_ratio));
                }
                else if( direction==wxVERTICAL )
                {
                                        if( availableOtherDir>=0 && int(size*m_ratio)-m_minSize.x>availableOtherDir )
                        size = int((availableOtherDir+m_minSize.x)/m_ratio);
                    m_minSize = wxSize(int(size*m_ratio),size);
                }
                didUse = true;
            }
        }
    }

    return didUse;
}

wxSize wxSizerItem::CalcMin()
{
    if (IsSizer())
    {
        m_minSize = m_sizer->GetMinSize();

                        if ( (m_flag & wxSHAPED) && wxIsNullDouble(m_ratio) )
            SetRatio(m_minSize);
    }
    else if ( IsWindow() )
    {
                        m_minSize = m_window->GetEffectiveMinSize();
    }

    return GetMinSizeWithBorder();
}

wxSize wxSizerItem::GetMinSizeWithBorder() const
{
    return AddBorderToSize(m_minSize);
}

wxSize wxSizerItem::GetMaxSizeWithBorder() const
{
    return AddBorderToSize(GetMaxSize());
}

void wxSizerItem::SetDimension( const wxPoint& pos_, const wxSize& size_ )
{
    wxPoint pos = pos_;
    wxSize size = size_;
    if (m_flag & wxSHAPED)
    {
                int rwidth = (int) (size.y * m_ratio);
        if (rwidth > size.x)
        {
                        int rheight = (int) (size.x / m_ratio);
                        if (m_flag & wxALIGN_CENTER_VERTICAL)
                pos.y += (size.y - rheight) / 2;
            else if (m_flag & wxALIGN_BOTTOM)
                pos.y += (size.y - rheight);
                        size.y =rheight;
        }
        else if (rwidth < size.x)
        {
                        if (m_flag & wxALIGN_CENTER_HORIZONTAL)
                pos.x += (size.x - rwidth) / 2;
            else if (m_flag & wxALIGN_RIGHT)
                pos.x += (size.x - rwidth);
            size.x = rwidth;
        }
    }

                m_pos = pos;

    if (m_flag & wxWEST)
    {
        pos.x += m_border;
        size.x -= m_border;
    }
    if (m_flag & wxEAST)
    {
        size.x -= m_border;
    }
    if (m_flag & wxNORTH)
    {
        pos.y += m_border;
        size.y -= m_border;
    }
    if (m_flag & wxSOUTH)
    {
        size.y -= m_border;
    }

    if (size.x < 0)
        size.x = 0;
    if (size.y < 0)
        size.y = 0;

    m_rect = wxRect(pos, size);

    switch ( m_kind )
    {
        case Item_None:
            wxFAIL_MSG( wxT("can't set size of uninitialized sizer item") );
            break;

        case Item_Window:
        {
                                                            #if 1
            m_window->SetSize(pos.x, pos.y, size.x, size.y,
                              wxSIZE_ALLOW_MINUS_ONE|wxSIZE_FORCE_EVENT );
#else
            m_window->SetSize(pos.x, pos.y, size.x, size.y,
                              wxSIZE_ALLOW_MINUS_ONE );
#endif
            break;
        }
        case Item_Sizer:
            m_sizer->SetDimension(pos, size);
            break;

        case Item_Spacer:
            m_spacer->SetSize(size);
            break;

        case Item_Max:
        default:
            wxFAIL_MSG( wxT("unexpected wxSizerItem::m_kind") );
    }
}

void wxSizerItem::DeleteWindows()
{
    switch ( m_kind )
    {
        case Item_None:
        case Item_Spacer:
            break;

        case Item_Window:
                                                m_window->SetContainingSizer(NULL);
            m_window->Destroy();
                                    m_kind = Item_None;
            break;

        case Item_Sizer:
            m_sizer->DeleteWindows();
            break;

        case Item_Max:
        default:
            wxFAIL_MSG( wxT("unexpected wxSizerItem::m_kind") );
    }

}

void wxSizerItem::Show( bool show )
{
    switch ( m_kind )
    {
        case Item_None:
            wxFAIL_MSG( wxT("can't show uninitialized sizer item") );
            break;

        case Item_Window:
            m_window->Show(show);
            break;

        case Item_Sizer:
            m_sizer->Show(show);
            break;

        case Item_Spacer:
            m_spacer->Show(show);
            break;

        case Item_Max:
        default:
            wxFAIL_MSG( wxT("unexpected wxSizerItem::m_kind") );
    }
}

bool wxSizerItem::IsShown() const
{
    if ( m_flag & wxRESERVE_SPACE_EVEN_IF_HIDDEN )
        return true;

    switch ( m_kind )
    {
        case Item_None:
                                    break;

        case Item_Window:
            return m_window->IsShown();

        case Item_Sizer:
                                                return m_sizer->AreAnyItemsShown();

        case Item_Spacer:
            return m_spacer->IsShown();

        case Item_Max:
        default:
            wxFAIL_MSG( wxT("unexpected wxSizerItem::m_kind") );
    }

    return false;
}


wxSizer::~wxSizer()
{
    WX_CLEAR_LIST(wxSizerItemList, m_children);
}

wxSizerItem* wxSizer::DoInsert( size_t index, wxSizerItem *item )
{
    m_children.Insert( index, item );

    if ( item->GetWindow() )
        item->GetWindow()->SetContainingSizer( this );

    if ( item->GetSizer() )
        item->GetSizer()->SetContainingWindow( m_containingWindow );

    return item;
}

void wxSizer::SetContainingWindow(wxWindow *win)
{
    if ( win == m_containingWindow )
        return;

    m_containingWindow = win;

            for ( wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
          node;
          node = node->GetNext() )
    {
        wxSizerItem *const item = node->GetData();
        wxSizer *const sizer = item->GetSizer();

        if ( sizer )
        {
            sizer->SetContainingWindow(win);
        }
    }
}

bool wxSizer::Remove( wxSizer *sizer )
{
    wxASSERT_MSG( sizer, wxT("Removing NULL sizer") );

    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->GetSizer() == sizer)
        {
            delete item;
            m_children.Erase( node );
            return true;
        }

        node = node->GetNext();
    }

    return false;
}

bool wxSizer::Remove( int index )
{
    wxCHECK_MSG( index >= 0 && (size_t)index < m_children.GetCount(),
                 false,
                 wxT("Remove index is out of range") );

    wxSizerItemList::compatibility_iterator node = m_children.Item( index );

    wxCHECK_MSG( node, false, wxT("Failed to find child node") );

    delete node->GetData();
    m_children.Erase( node );

    return true;
}

bool wxSizer::Detach( wxSizer *sizer )
{
    wxASSERT_MSG( sizer, wxT("Detaching NULL sizer") );

    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->GetSizer() == sizer)
        {
            item->DetachSizer();
            delete item;
            m_children.Erase( node );
            return true;
        }
        node = node->GetNext();
    }

    return false;
}

bool wxSizer::Detach( wxWindow *window )
{
    wxASSERT_MSG( window, wxT("Detaching NULL window") );

    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->GetWindow() == window)
        {
            delete item;
            m_children.Erase( node );
            return true;
        }
        node = node->GetNext();
    }

    return false;
}

bool wxSizer::Detach( int index )
{
    wxCHECK_MSG( index >= 0 && (size_t)index < m_children.GetCount(),
                 false,
                 wxT("Detach index is out of range") );

    wxSizerItemList::compatibility_iterator node = m_children.Item( index );

    wxCHECK_MSG( node, false, wxT("Failed to find child node") );

    wxSizerItem *item = node->GetData();

    if ( item->IsSizer() )
        item->DetachSizer();

    delete item;
    m_children.Erase( node );
    return true;
}

bool wxSizer::Replace( wxWindow *oldwin, wxWindow *newwin, bool recursive )
{
    wxASSERT_MSG( oldwin, wxT("Replacing NULL window") );
    wxASSERT_MSG( newwin, wxT("Replacing with NULL window") );

    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->GetWindow() == oldwin)
        {
            item->AssignWindow(newwin);
            newwin->SetContainingSizer( this );
            return true;
        }
        else if (recursive && item->IsSizer())
        {
            if (item->GetSizer()->Replace( oldwin, newwin, true ))
                return true;
        }

        node = node->GetNext();
    }

    return false;
}

bool wxSizer::Replace( wxSizer *oldsz, wxSizer *newsz, bool recursive )
{
    wxASSERT_MSG( oldsz, wxT("Replacing NULL sizer") );
    wxASSERT_MSG( newsz, wxT("Replacing with NULL sizer") );

    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->GetSizer() == oldsz)
        {
            item->AssignSizer(newsz);
            return true;
        }
        else if (recursive && item->IsSizer())
        {
            if (item->GetSizer()->Replace( oldsz, newsz, true ))
                return true;
        }

        node = node->GetNext();
    }

    return false;
}

bool wxSizer::Replace( size_t old, wxSizerItem *newitem )
{
    wxCHECK_MSG( old < m_children.GetCount(), false, wxT("Replace index is out of range") );
    wxASSERT_MSG( newitem, wxT("Replacing with NULL item") );

    wxSizerItemList::compatibility_iterator node = m_children.Item( old );

    wxCHECK_MSG( node, false, wxT("Failed to find child node") );

    wxSizerItem *item = node->GetData();
    node->SetData(newitem);

    if (item->IsWindow() && item->GetWindow())
        item->GetWindow()->SetContainingSizer(NULL);

    delete item;

    return true;
}

void wxSizer::Clear( bool delete_windows )
{
        wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->IsWindow())
            item->GetWindow()->SetContainingSizer( NULL );
        node = node->GetNext();
    }

        if (delete_windows)
        DeleteWindows();

        WX_CLEAR_LIST(wxSizerItemList, m_children);
}

void wxSizer::DeleteWindows()
{
    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        item->DeleteWindows();
        node = node->GetNext();
    }
}

wxSize wxSizer::ComputeFittingClientSize(wxWindow *window)
{
    wxCHECK_MSG( window, wxDefaultSize, "window can't be NULL" );

        wxSize size = GetMinClientSize(window);
    wxSize sizeMax;

    wxTopLevelWindow *tlw = wxDynamicCast(window, wxTopLevelWindow);
    if ( tlw )
    {
                if ( tlw->IsAlwaysMaximized() )
        {
            return tlw->GetClientSize();
        }

                int disp = wxDisplay::GetFromWindow(window);
        if ( disp == wxNOT_FOUND )
        {
                        disp = 0;
        }

        sizeMax = wxDisplay(disp).GetClientArea().GetSize();

                        if ( !sizeMax.x || !sizeMax.y )
            return size;

                sizeMax = tlw->WindowToClientSize(sizeMax);
    }
    else
    {
        sizeMax = GetMaxClientSize(window);
    }

    if ( sizeMax.x != wxDefaultCoord && size.x > sizeMax.x )
            size.x = sizeMax.x;
    if ( sizeMax.y != wxDefaultCoord && size.y > sizeMax.y )
            size.y = sizeMax.y;

    return size;
}

wxSize wxSizer::ComputeFittingWindowSize(wxWindow *window)
{
    wxCHECK_MSG( window, wxDefaultSize, "window can't be NULL" );

    return window->ClientToWindowSize(ComputeFittingClientSize(window));
}

wxSize wxSizer::Fit( wxWindow *window )
{
    wxCHECK_MSG( window, wxDefaultSize, "window can't be NULL" );

        window->SetClientSize(ComputeFittingClientSize(window));

        return window->GetSize();
}

void wxSizer::FitInside( wxWindow *window )
{
    wxSize size;
    if (window->IsTopLevel())
        size = VirtualFitSize( window );
    else
        size = GetMinClientSize( window );

    window->SetVirtualSize( size );
}

void wxSizer::Layout()
{
            CalcMin();

        wxWindow::ChildrenRepositioningGuard repositionGuard(m_containingWindow);

    RecalcSizes();
}

void wxSizer::SetSizeHints( wxWindow *window )
{
        
                    
    const wxSize clientSize = ComputeFittingClientSize(window);

    window->SetMinClientSize(clientSize);
    window->SetClientSize(clientSize);
}

#if WXWIN_COMPATIBILITY_2_8
void wxSizer::SetVirtualSizeHints( wxWindow *window )
{
    FitInside( window );
}
#endif 
wxSize wxSizer::GetMaxClientSize( wxWindow *window ) const
{
    return window->WindowToClientSize(window->GetMaxSize());
}

wxSize wxSizer::GetMinClientSize( wxWindow *WXUNUSED(window) )
{
    return GetMinSize();  }

wxSize wxSizer::VirtualFitSize( wxWindow *window )
{
    wxSize size     = GetMinClientSize( window );
    wxSize sizeMax  = GetMaxClientSize( window );

    
    if ( size.x > sizeMax.x && sizeMax.x != wxDefaultCoord )
        size.x = sizeMax.x;
    if ( size.y > sizeMax.y && sizeMax.y != wxDefaultCoord )
        size.y = sizeMax.y;

    return size;
}

wxSize wxSizer::GetMinSize()
{
    wxSize ret( CalcMin() );
    if (ret.x < m_minSize.x) ret.x = m_minSize.x;
    if (ret.y < m_minSize.y) ret.y = m_minSize.y;
    return ret;
}

void wxSizer::DoSetMinSize( int width, int height )
{
    m_minSize.x = width;
    m_minSize.y = height;
}

bool wxSizer::DoSetItemMinSize( wxWindow *window, int width, int height )
{
    wxASSERT_MSG( window, wxT("SetMinSize for NULL window") );

    
    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->GetWindow() == window)
        {
            item->SetMinSize( width, height );
            return true;
        }
        node = node->GetNext();
    }

    
    node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if ( item->GetSizer() &&
             item->GetSizer()->DoSetItemMinSize( window, width, height ) )
        {
                        return true;
        }
        node = node->GetNext();
    }

    return false;
}

bool wxSizer::DoSetItemMinSize( wxSizer *sizer, int width, int height )
{
    wxASSERT_MSG( sizer, wxT("SetMinSize for NULL sizer") );

    
    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->GetSizer() == sizer)
        {
            item->GetSizer()->DoSetMinSize( width, height );
            return true;
        }
        node = node->GetNext();
    }

    
    node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if ( item->GetSizer() &&
             item->GetSizer()->DoSetItemMinSize( sizer, width, height ) )
        {
                        return true;
        }
        node = node->GetNext();
    }

    return false;
}

bool wxSizer::DoSetItemMinSize( size_t index, int width, int height )
{
    wxSizerItemList::compatibility_iterator node = m_children.Item( index );

    wxCHECK_MSG( node, false, wxT("Failed to find child node") );

    wxSizerItem     *item = node->GetData();

    if (item->GetSizer())
    {
                item->GetSizer()->DoSetMinSize( width, height );
    }
    else
    {
                item->SetMinSize( width, height );
    }

    return true;
}

wxSizerItem* wxSizer::GetItem( wxWindow *window, bool recursive )
{
    wxASSERT_MSG( window, wxT("GetItem for NULL window") );

    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->GetWindow() == window)
        {
            return item;
        }
        else if (recursive && item->IsSizer())
        {
            wxSizerItem *subitem = item->GetSizer()->GetItem( window, true );
            if (subitem)
                return subitem;
        }

        node = node->GetNext();
    }

    return NULL;
}

wxSizerItem* wxSizer::GetItem( wxSizer *sizer, bool recursive )
{
    wxASSERT_MSG( sizer, wxT("GetItem for NULL sizer") );

    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem *item = node->GetData();

        if (item->GetSizer() == sizer)
        {
            return item;
        }
        else if (recursive && item->IsSizer())
        {
            wxSizerItem *subitem = item->GetSizer()->GetItem( sizer, true );
            if (subitem)
                return subitem;
        }

        node = node->GetNext();
    }

    return NULL;
}

wxSizerItem* wxSizer::GetItem( size_t index )
{
    wxCHECK_MSG( index < m_children.GetCount(),
                 NULL,
                 wxT("GetItem index is out of range") );

    return m_children.Item( index )->GetData();
}

wxSizerItem* wxSizer::GetItemById( int id, bool recursive )
{
        
    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->GetId() == id)
        {
            return item;
        }
        else if (recursive && item->IsSizer())
        {
            wxSizerItem *subitem = item->GetSizer()->GetItemById( id, true );
            if (subitem)
                return subitem;
        }

        node = node->GetNext();
    }

    return NULL;
}

bool wxSizer::Show( wxWindow *window, bool show, bool recursive )
{
    wxSizerItem *item = GetItem( window, recursive );

    if ( item )
    {
         item->Show( show );
         return true;
    }

    return false;
}

bool wxSizer::Show( wxSizer *sizer, bool show, bool recursive )
{
    wxSizerItem *item = GetItem( sizer, recursive );

    if ( item )
    {
         item->Show( show );
         return true;
    }

    return false;
}

bool wxSizer::Show( size_t index, bool show)
{
    wxSizerItem *item = GetItem( index );

    if ( item )
    {
         item->Show( show );
         return true;
    }

    return false;
}

void wxSizer::ShowItems( bool show )
{
    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        node->GetData()->Show( show );
        node = node->GetNext();
    }
}

bool wxSizer::AreAnyItemsShown() const
{
    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        if ( node->GetData()->IsShown() )
            return true;
        node = node->GetNext();
    }

    return false;
}

bool wxSizer::IsShown( wxWindow *window ) const
{
    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->GetWindow() == window)
        {
            return item->IsShown();
        }
        node = node->GetNext();
    }

    wxFAIL_MSG( wxT("IsShown failed to find sizer item") );

    return false;
}

bool wxSizer::IsShown( wxSizer *sizer ) const
{
    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();

        if (item->GetSizer() == sizer)
        {
            return item->IsShown();
        }
        node = node->GetNext();
    }

    wxFAIL_MSG( wxT("IsShown failed to find sizer item") );

    return false;
}

bool wxSizer::IsShown( size_t index ) const
{
    wxCHECK_MSG( index < m_children.GetCount(),
                 false,
                 wxT("IsShown index is out of range") );

    return m_children.Item( index )->GetData()->IsShown();
}



wxGridSizer::wxGridSizer( int cols, int vgap, int hgap )
    : m_rows( cols == 0 ? 1 : 0 ),
      m_cols( cols ),
      m_vgap( vgap ),
      m_hgap( hgap )
{
    wxASSERT(cols >= 0);
}

wxGridSizer::wxGridSizer( int cols, const wxSize& gap )
    : m_rows( cols == 0 ? 1 : 0 ),
      m_cols( cols ),
      m_vgap( gap.GetHeight() ),
      m_hgap( gap.GetWidth() )
{
    wxASSERT(cols >= 0);
}

wxGridSizer::wxGridSizer( int rows, int cols, int vgap, int hgap )
    : m_rows( rows || cols ? rows : 1 ),
      m_cols( cols ),
      m_vgap( vgap ),
      m_hgap( hgap )
{
    wxASSERT(rows >= 0 && cols >= 0);
}

wxGridSizer::wxGridSizer( int rows, int cols, const wxSize& gap )
    : m_rows( rows || cols ? rows : 1 ),
      m_cols( cols ),
      m_vgap( gap.GetHeight() ),
      m_hgap( gap.GetWidth() )
{
    wxASSERT(rows >= 0 && cols >= 0);
}

wxSizerItem *wxGridSizer::DoInsert(size_t index, wxSizerItem *item)
{
                    if ( m_cols && m_rows )
    {
        const int nitems = m_children.GetCount();
        if ( nitems == m_cols*m_rows )
        {
            wxFAIL_MSG(
                wxString::Format(
                    "too many items (%d > %d*%d) in grid sizer (maybe you "
                    "should omit the number of either rows or columns?)",
                nitems + 1, m_cols, m_rows)
            );

                                                                                                            m_rows = 0;
        }
    }

    const int flags = item->GetFlag();
    if ( flags & wxEXPAND )
    {
                wxASSERT_MSG
        (
            !(flags & (wxALIGN_BOTTOM | wxALIGN_CENTRE_VERTICAL)) ||
                !(flags & (wxALIGN_RIGHT | wxALIGN_CENTRE_HORIZONTAL)),
            wxS("wxEXPAND flag will be overridden by alignment flags")
        );
    }

    return wxSizer::DoInsert(index, item);
}

int wxGridSizer::CalcRowsCols(int& nrows, int& ncols) const
{
    const int nitems = m_children.GetCount();

    ncols = GetEffectiveColsCount();
    nrows = GetEffectiveRowsCount();

            wxASSERT_MSG( nitems <= ncols*nrows, "logic error in wxGridSizer" );

    return nitems;
}

void wxGridSizer::RecalcSizes()
{
    int nitems, nrows, ncols;
    if ( (nitems = CalcRowsCols(nrows, ncols)) == 0 )
        return;

    wxSize sz( GetSize() );
    wxPoint pt( GetPosition() );

    int w = (sz.x - (ncols - 1) * m_hgap) / ncols;
    int h = (sz.y - (nrows - 1) * m_vgap) / nrows;

    int x = pt.x;
    for (int c = 0; c < ncols; c++)
    {
        int y = pt.y;
        for (int r = 0; r < nrows; r++)
        {
            int i = r * ncols + c;
            if (i < nitems)
            {
                wxSizerItemList::compatibility_iterator node = m_children.Item( i );

                wxASSERT_MSG( node, wxT("Failed to find SizerItemList node") );

                SetItemBounds( node->GetData(), x, y, w, h);
            }
            y = y + h + m_vgap;
        }
        x = x + w + m_hgap;
    }
}

wxSize wxGridSizer::CalcMin()
{
    int nrows, ncols;
    if ( CalcRowsCols(nrows, ncols) == 0 )
        return wxSize();

        int w = 0;
    int h = 0;

    wxSizerItemList::compatibility_iterator node = m_children.GetFirst();
    while (node)
    {
        wxSizerItem     *item = node->GetData();
        wxSize           sz( item->CalcMin() );

        w = wxMax( w, sz.x );
        h = wxMax( h, sz.y );

        node = node->GetNext();
    }

            node = m_children.GetFirst();
    bool didChangeMinSize = false;
    while (node)
    {
        wxSizerItem     *item = node->GetData();
        didChangeMinSize |= item->InformFirstDirection( wxHORIZONTAL, w, -1 );

        node = node->GetNext();
    }

        if( didChangeMinSize )
    {
        node = m_children.GetFirst();
        w = h = 0;
        while (node)
        {
            wxSizerItem     *item = node->GetData();
            wxSize           sz( item->GetMinSizeWithBorder() );

            w = wxMax( w, sz.x );
            h = wxMax( h, sz.y );

            node = node->GetNext();
        }
    }

    return wxSize( ncols * w + (ncols-1) * m_hgap,
                   nrows * h + (nrows-1) * m_vgap );
}

void wxGridSizer::SetItemBounds( wxSizerItem *item, int x, int y, int w, int h )
{
    wxPoint pt( x,y );
    wxSize sz( item->GetMinSizeWithBorder() );
    int flag = item->GetFlag();

            if ( flag & wxSHAPED )
    {
        sz = wxSize(w, h);
    }
    else     {
        if (flag & wxALIGN_CENTER_HORIZONTAL)
        {
            pt.x = x + (w - sz.x) / 2;
        }
        else if (flag & wxALIGN_RIGHT)
        {
            pt.x = x + (w - sz.x);
        }
        else if (flag & wxEXPAND)
        {
            sz.x = w;
        }

        if (flag & wxALIGN_CENTER_VERTICAL)
        {
            pt.y = y + (h - sz.y) / 2;
        }
        else if (flag & wxALIGN_BOTTOM)
        {
            pt.y = y + (h - sz.y);
        }
        else if ( flag & wxEXPAND )
        {
            sz.y = h;
        }
    }

    item->SetDimension(pt, sz);
}


wxFlexGridSizer::wxFlexGridSizer( int cols, int vgap, int hgap )
               : wxGridSizer( cols, vgap, hgap ),
                 m_flexDirection(wxBOTH),
                 m_growMode(wxFLEX_GROWMODE_SPECIFIED)
{
}

wxFlexGridSizer::wxFlexGridSizer( int cols, const wxSize& gap )
               : wxGridSizer( cols, gap ),
                 m_flexDirection(wxBOTH),
                 m_growMode(wxFLEX_GROWMODE_SPECIFIED)
{
}

wxFlexGridSizer::wxFlexGridSizer( int rows, int cols, int vgap, int hgap )
               : wxGridSizer( rows, cols, vgap, hgap ),
                 m_flexDirection(wxBOTH),
                 m_growMode(wxFLEX_GROWMODE_SPECIFIED)
{
}

wxFlexGridSizer::wxFlexGridSizer( int rows, int cols, const wxSize& gap )
               : wxGridSizer( rows, cols, gap ),
                 m_flexDirection(wxBOTH),
                 m_growMode(wxFLEX_GROWMODE_SPECIFIED)
{
}

wxFlexGridSizer::~wxFlexGridSizer()
{
}

void wxFlexGridSizer::RecalcSizes()
{
    int nrows, ncols;
    if ( !CalcRowsCols(nrows, ncols) )
        return;

    const wxPoint pt(GetPosition());
    const wxSize sz(GetSize());

    AdjustForGrowables(sz);

    wxSizerItemList::const_iterator i = m_children.begin();
    const wxSizerItemList::const_iterator end = m_children.end();

    int y = 0;
    for ( int r = 0; r < nrows; r++ )
    {
        if ( m_rowHeights[r] == -1 )
        {
                        for ( int c = 0; c < ncols; c++ )
            {
                if ( i == end )
                    return;

                ++i;
            }

            continue;
        }

        const int hrow = m_rowHeights[r];
        int h = sz.y - y;         if ( hrow < h )
            h = hrow;

        int x = 0;
        for ( int c = 0; c < ncols && i != end; c++, ++i )
        {
            const int wcol = m_colWidths[c];

            if ( wcol == -1 )
                continue;

            int w = sz.x - x;             if ( wcol < w )
                w = wcol;

            SetItemBounds(*i, pt.x + x, pt.y + y, w, h);

            x += wcol + m_hgap;
        }

        if ( i == end )
            return;

        y += hrow + m_vgap;
    }
}

static int SumArraySizes(const wxArrayInt& sizes, int gap)
{
            int total = 0;

    const size_t count = sizes.size();
    for ( size_t n = 0; n < count; n++ )
    {
        if ( sizes[n] != -1 )
        {
            if ( total )
                total += gap; 
            total += sizes[n];
        }
    }

    return total;
}

void wxFlexGridSizer::FindWidthsAndHeights(int nrows, int ncols)
{
                        m_rowHeights.assign(nrows, -1);
    m_colWidths.assign(ncols, -1);

        size_t n = 0;
    for ( wxSizerItemList::iterator i = m_children.begin();
          i != m_children.end();
          ++i, ++n )
    {
        wxSizerItem * const item = *i;
        if ( item->IsShown() )
        {
                                    const wxSize sz(item->GetMinSizeWithBorder());

            const int row = n / ncols;
            const int col = n % ncols;

            if ( sz.y > m_rowHeights[row] )
                m_rowHeights[row] = sz.y;
            if ( sz.x > m_colWidths[col] )
                m_colWidths[col] = sz.x;
        }
    }

    AdjustForFlexDirection();

    m_calculatedMinSize = wxSize(SumArraySizes(m_colWidths, m_hgap),
                                 SumArraySizes(m_rowHeights, m_vgap));
}

wxSize wxFlexGridSizer::CalcMin()
{
    int nrows,
        ncols;

        if ( !CalcRowsCols(nrows, ncols) )
        return wxSize();


                        m_rowHeights.assign(nrows, -1);
    m_colWidths.assign(ncols, -1);

    for ( wxSizerItemList::iterator i = m_children.begin();
          i != m_children.end();
          ++i)
    {
        wxSizerItem * const item = *i;
        if ( item->IsShown() )
        {
            item->CalcMin();
        }
    }

            FindWidthsAndHeights(nrows,ncols);

    return m_calculatedMinSize;
}

void wxFlexGridSizer::AdjustForFlexDirection()
{
            if ( m_flexDirection != wxBOTH )
    {
                        wxArrayInt& array = m_flexDirection == wxVERTICAL ? m_colWidths
                                                          : m_rowHeights;

        const size_t count = array.GetCount();

                size_t n;
        int largest = 0;

        for ( n = 0; n < count; ++n )
        {
            if ( array[n] > largest )
                largest = array[n];
        }

                for ( n = 0; n < count; ++n )
        {
                        if ( array[n] != -1 )
                array[n] = largest;
        }
    }
}

static void
DoAdjustForGrowables(int delta,
                     const wxArrayInt& growable,
                     wxArrayInt& sizes,
                     const wxArrayInt *proportions)
{
    if ( delta <= 0 )
        return;

        int sum_proportions = 0;

        int num = 0;

    const int max_idx = sizes.size();

    const size_t count = growable.size();
    size_t idx;
    for ( idx = 0; idx < count; idx++ )
    {
                                if ( growable[idx] >= max_idx )
            continue;

                                if ( sizes[growable[idx]] == -1 )
            continue;

        if ( proportions )
            sum_proportions += (*proportions)[idx];

        num++;
    }

    if ( !num )
        return;

        for ( idx = 0; idx < count; idx++ )
    {
        if ( growable[idx] >= max_idx )
            continue;

        if ( sizes[ growable[idx] ] == -1 )
            continue;

        int cur_delta;
        if ( sum_proportions == 0 )
        {
                        cur_delta = delta/num;
            num--;
        }
        else         {
            const int cur_prop = (*proportions)[idx];
            cur_delta = (delta*cur_prop)/sum_proportions;
            sum_proportions -= cur_prop;
        }

        sizes[growable[idx]] += cur_delta;
        delta -= cur_delta;
    }
}

void wxFlexGridSizer::AdjustForGrowables(const wxSize& sz)
{
#if wxDEBUG_LEVEL
                    if ( !m_rows || !m_cols )
    {
        if ( !m_rows )
        {
            int nrows = CalcRows();

            for ( size_t n = 0; n < m_growableRows.size(); n++ )
            {
                wxASSERT_MSG( m_growableRows[n] < nrows,
                              "invalid growable row index" );
            }
        }

        if ( !m_cols )
        {
            int ncols = CalcCols();

            for ( size_t n = 0; n < m_growableCols.size(); n++ )
            {
                wxASSERT_MSG( m_growableCols[n] < ncols,
                              "invalid growable column index" );
            }
        }
    }
#endif 

    if ( (m_flexDirection & wxHORIZONTAL) || (m_growMode != wxFLEX_GROWMODE_NONE) )
    {
        DoAdjustForGrowables
        (
            sz.x - m_calculatedMinSize.x,
            m_growableCols,
            m_colWidths,
            m_growMode == wxFLEX_GROWMODE_SPECIFIED ? &m_growableColsProportions
                                                    : NULL
        );

                        bool didAdjustMinSize = false;

                const int ncols = GetEffectiveColsCount();
        int col = 0;
        for ( wxSizerItemList::iterator i = m_children.begin();
              i != m_children.end();
              ++i )
        {
            didAdjustMinSize |= (*i)->InformFirstDirection(wxHORIZONTAL, m_colWidths[col], sz.y - m_calculatedMinSize.y);
            if ( ++col == ncols )
                col = 0;
        }

                if( didAdjustMinSize )
        {
            DoAdjustForGrowables
            (
                sz.x - m_calculatedMinSize.x,
                m_growableCols,
                m_colWidths,
                m_growMode == wxFLEX_GROWMODE_SPECIFIED ? &m_growableColsProportions
                                                        : NULL
            );
        }
    }

    if ( (m_flexDirection & wxVERTICAL) || (m_growMode != wxFLEX_GROWMODE_NONE) )
    {
                        DoAdjustForGrowables
        (
            sz.y - m_calculatedMinSize.y,
            m_growableRows,
            m_rowHeights,
            m_growMode == wxFLEX_GROWMODE_SPECIFIED ? &m_growableRowsProportions
                                                    : NULL
        );
    }
}

bool wxFlexGridSizer::IsRowGrowable( size_t idx )
{
    return m_growableRows.Index( idx ) != wxNOT_FOUND;
}

bool wxFlexGridSizer::IsColGrowable( size_t idx )
{
    return m_growableCols.Index( idx ) != wxNOT_FOUND;
}

void wxFlexGridSizer::AddGrowableRow( size_t idx, int proportion )
{
    wxASSERT_MSG( !IsRowGrowable( idx ),
                  "AddGrowableRow() called for growable row" );

                    wxCHECK_RET( !m_rows || idx < (size_t)m_rows, "invalid row index" );

    m_growableRows.Add( idx );
    m_growableRowsProportions.Add( proportion );
}

void wxFlexGridSizer::AddGrowableCol( size_t idx, int proportion )
{
    wxASSERT_MSG( !IsColGrowable( idx ),
                  "AddGrowableCol() called for growable column" );

            wxCHECK_RET( !m_cols || idx < (size_t)m_cols, "invalid column index" );

    m_growableCols.Add( idx );
    m_growableColsProportions.Add( proportion );
}

static void
DoRemoveFromArrays(size_t idx, wxArrayInt& items, wxArrayInt& proportions)
{
    const size_t count = items.size();
    for ( size_t n = 0; n < count; n++ )
    {
        if ( (size_t)items[n] == idx )
        {
            items.RemoveAt(n);
            proportions.RemoveAt(n);
            return;
        }
    }

    wxFAIL_MSG( wxT("column/row is already not growable") );
}

void wxFlexGridSizer::RemoveGrowableCol( size_t idx )
{
    DoRemoveFromArrays(idx, m_growableCols, m_growableColsProportions);
}

void wxFlexGridSizer::RemoveGrowableRow( size_t idx )
{
    DoRemoveFromArrays(idx, m_growableRows, m_growableRowsProportions);
}


wxSizerItem *wxBoxSizer::DoInsert(size_t index, wxSizerItem *item)
{
    const int flags = item->GetFlag();
    if ( IsVertical() )
    {
        wxASSERT_MSG
        (
            !(flags & wxALIGN_BOTTOM),
            wxS("Vertical alignment flags are ignored in vertical sizers")
        );

                                if ( !(flags & wxALIGN_CENTRE_HORIZONTAL) )
        {
            wxASSERT_MSG
            (
                !(flags & wxALIGN_CENTRE_VERTICAL),
                wxS("Vertical alignment flags are ignored in vertical sizers")
            );
        }

        if ( flags & wxEXPAND )
        {
            wxASSERT_MSG
            (
                !(flags & (wxALIGN_RIGHT | wxALIGN_CENTRE_HORIZONTAL)),
                wxS("Horizontal alignment flags are ignored with wxEXPAND")
            );
        }
    }
    else     {
        wxASSERT_MSG
        (
            !(flags & wxALIGN_RIGHT),
            wxS("Horizontal alignment flags are ignored in horizontal sizers")
        );

        if ( !(flags & wxALIGN_CENTRE_VERTICAL) )
        {
            wxASSERT_MSG
            (
                !(flags & wxALIGN_CENTRE_HORIZONTAL),
                wxS("Horizontal alignment flags are ignored in horizontal sizers")
            );
        }

        if ( flags & wxEXPAND )
        {
            wxASSERT_MSG(
                !(flags & (wxALIGN_BOTTOM | wxALIGN_CENTRE_VERTICAL)),
                wxS("Vertical alignment flags are ignored with wxEXPAND")
            );
        }
    }

    return wxSizer::DoInsert(index, item);
}

wxSizerItem *wxBoxSizer::AddSpacer(int size)
{
    return IsVertical() ? Add(0, size) : Add(size, 0);
}

namespace
{


int
GetMinOrRemainingSize(int orient, const wxSizerItem *item, int *remainingSpace_)
{
    int& remainingSpace = *remainingSpace_;

    wxCoord size;
    if ( remainingSpace > 0 )
    {
        const wxSize sizeMin = item->GetMinSizeWithBorder();
        size = orient == wxHORIZONTAL ? sizeMin.x : sizeMin.y;

        if ( size >= remainingSpace )
        {
                                                size = remainingSpace;
        }

        remainingSpace -= size;
    }
    else     {
                        size = 0;
    }

    return size;
}

} 
void wxBoxSizer::RecalcSizes()
{
    if ( m_children.empty() )
        return;

    const wxCoord totalMinorSize = GetSizeInMinorDir(m_size);
    const wxCoord totalMajorSize = GetSizeInMajorDir(m_size);

            int delta = totalMajorSize - GetSizeInMajorDir(m_calculatedMinSize);

        wxSizerItemList::const_iterator i;      unsigned n = 0;                     

            
    int minMajorSize = 0;
    for ( i = m_children.begin(); i != m_children.end(); ++i )
    {
        wxSizerItem * const item = *i;

        if ( !item->IsShown() )
            continue;

        wxSize szMinPrev = item->GetMinSizeWithBorder();
        item->InformFirstDirection(m_orient^wxBOTH,totalMinorSize,delta);
        wxSize szMin = item->GetMinSizeWithBorder();
        int deltaChange = GetSizeInMajorDir(szMin-szMinPrev);
        if( deltaChange )
        {
                                    delta -= deltaChange;
        }
        minMajorSize += GetSizeInMajorDir(item->GetMinSizeWithBorder());
    }


            wxCoord remaining = totalMajorSize;
    int totalProportion = m_totalProportion;

        wxVector<int> majorSizes(GetItemCount(), wxDefaultCoord);


                                if ( totalMajorSize < minMajorSize )
    {
                        for ( i = m_children.begin(), n = 0; i != m_children.end(); ++i, ++n )
        {
            wxSizerItem * const item = *i;

            if ( !item->IsShown() )
                continue;

                        if ( item->GetProportion() )
                continue;

            majorSizes[n] = GetMinOrRemainingSize(m_orient, item, &remaining);
        }


                        for ( i = m_children.begin(), n = 0; i != m_children.end(); ++i, ++n )
        {
            wxSizerItem * const item = *i;

            if ( !item->IsShown() )
                continue;

                        if ( !item->GetProportion() )
                continue;

            majorSizes[n] = GetMinOrRemainingSize(m_orient, item, &remaining);
        }
    }
    else     {
                                                                                                bool nonFixedSpaceChanged = false;
        for ( i = m_children.begin(), n = 0; ; ++i, ++n )
        {
            if ( nonFixedSpaceChanged )
            {
                i = m_children.begin();
                n = 0;
                nonFixedSpaceChanged = false;
            }

                                                if ( i == m_children.end() )
                break;

            wxSizerItem * const item = *i;

            if ( !item->IsShown() )
                continue;

                                                            if ( majorSizes[n] != wxDefaultCoord )
                continue;

            wxCoord minMajor = GetSizeInMajorDir(item->GetMinSizeWithBorder());

                                                                                                            if ( minMajor < 0 )
                minMajor = 0;

            const int propItem = item->GetProportion();
            if ( propItem )
            {
                                if ( (remaining*propItem)/totalProportion >= minMajor )
                {
                                                            continue;
                }

                                                totalProportion -= propItem;
            }

                        majorSizes[n] = minMajor;

                                                            remaining -= minMajor;

            nonFixedSpaceChanged = true;
        }

                                nonFixedSpaceChanged = false;
        for ( i = m_children.begin(), n = 0; ; ++i, ++n )
        {
            if ( nonFixedSpaceChanged )
            {
                i = m_children.begin();
                n = 0;
                nonFixedSpaceChanged = false;
            }

                                                if ( i == m_children.end() )
                break;

            wxSizerItem * const item = *i;

            if ( !item->IsShown() )
                continue;

                                                            if ( majorSizes[n] != wxDefaultCoord )
                continue;

            wxCoord maxMajor = GetSizeInMajorDir(item->GetMaxSizeWithBorder());

                        const int propItem = item->GetProportion();

                        if ( maxMajor < 0 ||
                    (remaining*propItem)/totalProportion <= maxMajor )
            {
                                                continue;
            }

                                    totalProportion -= propItem;

                        majorSizes[n] = maxMajor;

                                                            remaining -= maxMajor;

            nonFixedSpaceChanged = true;
        }

                        for ( i = m_children.begin(), n = 0; i != m_children.end(); ++i, ++n )
        {
            wxSizerItem * const item = *i;

            if ( !item->IsShown() )
                continue;

            if ( majorSizes[n] == wxDefaultCoord )
            {
                const int propItem = item->GetProportion();
                majorSizes[n] = (remaining*propItem)/totalProportion;

                remaining -= majorSizes[n];
                totalProportion -= propItem;
            }
        }
    }


        wxPoint pt(m_position);


            for ( i = m_children.begin(), n = 0; i != m_children.end(); ++i, ++n )
    {
        wxSizerItem * const item = *i;

        if ( !item->IsShown() )
            continue;

        const int majorSize = majorSizes[n];

        const wxSize sizeThis(item->GetMinSizeWithBorder());

                wxPoint posChild(pt);

        wxCoord minorSize = GetSizeInMinorDir(sizeThis);
        const int flag = item->GetFlag();
        if ( (flag & (wxEXPAND | wxSHAPED)) || (minorSize > totalMinorSize) )
        {
                                                            minorSize = totalMinorSize;

                                    const wxCoord maxMinorSize = GetSizeInMinorDir(item->GetMaxSizeWithBorder());
            if ( maxMinorSize >= 0 && minorSize > maxMinorSize )
                minorSize = maxMinorSize;
        }

        if ( flag & (IsVertical() ? wxALIGN_RIGHT : wxALIGN_BOTTOM) )
        {
            PosInMinorDir(posChild) += totalMinorSize - minorSize;
        }
                        else if ( flag & (wxCENTER | (IsVertical() ? wxALIGN_CENTRE_HORIZONTAL
                                                   : wxALIGN_CENTRE_VERTICAL)) )
        {
            PosInMinorDir(posChild) += (totalMinorSize - minorSize) / 2;
        }


                if ( !IsVertical() && m_containingWindow )
        {
            posChild.x = m_containingWindow->AdjustForLayoutDirection
                                             (
                                                posChild.x,
                                                majorSize,
                                                m_size.x
                                             );
        }

                item->SetDimension(posChild, SizeFromMajorMinor(majorSize, minorSize));

        PosInMajorDir(pt) += majorSize;
    }
}

wxSize wxBoxSizer::CalcMin()
{
    m_totalProportion = 0;
    m_calculatedMinSize = wxSize(0, 0);

                        float maxMinSizeToProp = 0.;
    for ( wxSizerItemList::const_iterator i = m_children.begin();
          i != m_children.end();
          ++i )
    {
        wxSizerItem * const item = *i;

        if ( !item->IsShown() )
            continue;

        const wxSize sizeMinThis = item->CalcMin();
        if ( const int propThis = item->GetProportion() )
        {
            float minSizeToProp = GetSizeInMajorDir(sizeMinThis);
            minSizeToProp /= propThis;

            if ( minSizeToProp > maxMinSizeToProp )
                maxMinSizeToProp = minSizeToProp;

            m_totalProportion += item->GetProportion();
        }
        else         {
                        SizeInMajorDir(m_calculatedMinSize) += GetSizeInMajorDir(sizeMinThis);
        }

                if ( GetSizeInMinorDir(sizeMinThis) > GetSizeInMinorDir(m_calculatedMinSize) )
            SizeInMinorDir(m_calculatedMinSize) = GetSizeInMinorDir(sizeMinThis);
    }

            SizeInMajorDir(m_calculatedMinSize) += (int)(maxMinSizeToProp*m_totalProportion);

    return m_calculatedMinSize;
}


#if wxUSE_STATBOX

wxStaticBoxSizer::wxStaticBoxSizer( wxStaticBox *box, int orient )
    : wxBoxSizer( orient ),
      m_staticBox( box )
{
    wxASSERT_MSG( box, wxT("wxStaticBoxSizer needs a static box") );

            m_staticBox->SetContainingSizer(this);
}

wxStaticBoxSizer::wxStaticBoxSizer(int orient, wxWindow *win, const wxString& s)
                : wxBoxSizer(orient),
                  m_staticBox(new wxStaticBox(win, wxID_ANY, s))
{
        m_staticBox->SetContainingSizer(this);
}

wxStaticBoxSizer::~wxStaticBoxSizer()
{
                                    
    if ( m_staticBox )
    {
                        const wxWindowList children = m_staticBox->GetChildren();
        wxWindow* const parent = m_staticBox->GetParent();
        for ( wxWindowList::const_iterator i = children.begin();
              i != children.end();
              ++i )
        {
            (*i)->Reparent(parent);
        }

        delete m_staticBox;
    }
}

void wxStaticBoxSizer::RecalcSizes()
{
    int top_border, other_border;
    m_staticBox->GetBordersForSizer(&top_border, &other_border);

    m_staticBox->SetSize( m_position.x, m_position.y, m_size.x, m_size.y );

    wxSize old_size( m_size );
    m_size.x -= 2*other_border;
    m_size.y -= top_border + other_border;

    wxPoint old_pos( m_position );
    if (m_staticBox->GetChildren().GetCount() > 0)
    {
#if defined( __WXGTK20__ )
                                        m_position.x = m_position.y = 0;
#elif defined(__WXOSX__) && wxOSX_USE_COCOA
                        m_position.x = m_position.y = 10;
#else
                                        m_position.x = other_border;
        m_position.y = top_border;
#endif
    }
    else
    {
                                m_position.x += other_border;
        m_position.y += top_border;
    }

    wxBoxSizer::RecalcSizes();

    m_position = old_pos;
    m_size = old_size;
}

wxSize wxStaticBoxSizer::CalcMin()
{
    int top_border, other_border;
    m_staticBox->GetBordersForSizer(&top_border, &other_border);

    wxSize ret( wxBoxSizer::CalcMin() );
    ret.x += 2*other_border;

            const int boxWidth = m_staticBox->GetBestSize().x;
    if ( ret.x < boxWidth )
        ret.x = boxWidth;

    ret.y += other_border + top_border;

    return ret;
}

void wxStaticBoxSizer::ShowItems( bool show )
{
    m_staticBox->Show( show );
    wxBoxSizer::ShowItems( show );
}

bool wxStaticBoxSizer::AreAnyItemsShown() const
{
                        return m_staticBox->IsShown();
}

bool wxStaticBoxSizer::Detach( wxWindow *window )
{
                if ( window == m_staticBox )
    {
        m_staticBox = NULL;
        return true;
    }

    return wxSizer::Detach( window );
}

#endif 

#if wxUSE_BUTTON

wxStdDialogButtonSizer::wxStdDialogButtonSizer()
    : wxBoxSizer(wxHORIZONTAL)
{
    bool is_pda = (wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA);
            if (is_pda)
        m_orient = wxVERTICAL;

    m_buttonAffirmative = NULL;
    m_buttonApply = NULL;
    m_buttonNegative = NULL;
    m_buttonCancel = NULL;
    m_buttonHelp = NULL;
}

void wxStdDialogButtonSizer::AddButton(wxButton *mybutton)
{
    switch (mybutton->GetId())
    {
        case wxID_OK:
        case wxID_YES:
        case wxID_SAVE:
            m_buttonAffirmative = mybutton;
            break;
        case wxID_APPLY:
            m_buttonApply = mybutton;
            break;
        case wxID_NO:
            m_buttonNegative = mybutton;
            break;
        case wxID_CANCEL:
        case wxID_CLOSE:
            m_buttonCancel = mybutton;
            break;
        case wxID_HELP:
        case wxID_CONTEXT_HELP:
            m_buttonHelp = mybutton;
            break;
        default:
            break;
    }
}

void wxStdDialogButtonSizer::SetAffirmativeButton( wxButton *button )
{
    m_buttonAffirmative = button;
}

void wxStdDialogButtonSizer::SetNegativeButton( wxButton *button )
{
    m_buttonNegative = button;
}

void wxStdDialogButtonSizer::SetCancelButton( wxButton *button )
{
    m_buttonCancel = button;
}

void wxStdDialogButtonSizer::Realize()
{
#ifdef __WXMAC__
        Add(0, 0, 0, wxLEFT, 6);
        if (m_buttonHelp)
            Add((wxWindow*)m_buttonHelp, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, 6);

        if (m_buttonNegative){
                                    Add((wxWindow*)m_buttonNegative, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, 12);
        }

                Add(0, 0, 1, wxEXPAND, 0);

        if (m_buttonCancel){
            Add((wxWindow*)m_buttonCancel, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, 6);
                                }

                        if (m_buttonApply)
            Add((wxWindow*)m_buttonApply, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, 6);

        if (m_buttonAffirmative){
            Add((wxWindow*)m_buttonAffirmative, 0, wxALIGN_CENTRE | wxLEFT, 6);

            if (m_buttonAffirmative->GetId() == wxID_SAVE){
                                m_buttonAffirmative->SetLabel(_("Save"));
                if (m_buttonNegative)
                    m_buttonNegative->SetLabel(_("Don't Save"));
            }
        }

                Add(12, 40);
#elif defined(__WXGTK20__)
                                
                const wxSizerFlags
            flagsBtn = wxSizerFlags().Centre().Border(wxLEFT | wxRIGHT, 3);

                AddSpacer(9);

        if (m_buttonHelp)
            Add(m_buttonHelp, flagsBtn);

                AddStretchSpacer();

        if (m_buttonNegative)
            Add(m_buttonNegative, flagsBtn);

        if (m_buttonApply)
            Add(m_buttonApply, flagsBtn);

        if (m_buttonCancel)
            Add(m_buttonCancel, flagsBtn);

        if (m_buttonAffirmative)
            Add(m_buttonAffirmative, flagsBtn);

                AddSpacer(9);
#elif defined(__WXMSW__)
        
                Add(0, 0, 1, wxEXPAND, 0);

        if (m_buttonAffirmative){
            Add((wxWindow*)m_buttonAffirmative, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, m_buttonAffirmative->ConvertDialogToPixels(wxSize(2, 0)).x);
        }

        if (m_buttonNegative){
            Add((wxWindow*)m_buttonNegative, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, m_buttonNegative->ConvertDialogToPixels(wxSize(2, 0)).x);
        }

        if (m_buttonCancel){
            Add((wxWindow*)m_buttonCancel, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, m_buttonCancel->ConvertDialogToPixels(wxSize(2, 0)).x);
        }
        if (m_buttonApply)
            Add((wxWindow*)m_buttonApply, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, m_buttonApply->ConvertDialogToPixels(wxSize(2, 0)).x);

        if (m_buttonHelp)
            Add((wxWindow*)m_buttonHelp, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, m_buttonHelp->ConvertDialogToPixels(wxSize(2, 0)).x);
#else
        
                if (m_buttonHelp)
            Add((wxWindow*)m_buttonHelp, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, m_buttonHelp->ConvertDialogToPixels(wxSize(4, 0)).x);

                Add(0, 0, 1, wxEXPAND, 0);

        if (m_buttonApply)
            Add((wxWindow*)m_buttonApply, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, m_buttonApply->ConvertDialogToPixels(wxSize(4, 0)).x);

        if (m_buttonAffirmative){
            Add((wxWindow*)m_buttonAffirmative, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, m_buttonAffirmative->ConvertDialogToPixels(wxSize(4, 0)).x);
        }

        if (m_buttonNegative){
            Add((wxWindow*)m_buttonNegative, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, m_buttonNegative->ConvertDialogToPixels(wxSize(4, 0)).x);
        }

        if (m_buttonCancel){
            Add((wxWindow*)m_buttonCancel, 0, wxALIGN_CENTRE | wxLEFT | wxRIGHT, m_buttonCancel->ConvertDialogToPixels(wxSize(4, 0)).x);
                                }

#endif
}

#endif 