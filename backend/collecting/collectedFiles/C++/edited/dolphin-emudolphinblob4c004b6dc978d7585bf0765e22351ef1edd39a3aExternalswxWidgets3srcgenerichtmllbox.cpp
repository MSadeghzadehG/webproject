


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/dcclient.h"
#endif 
#if wxUSE_HTML

#include "wx/htmllbox.h"

#include "wx/html/htmlcell.h"
#include "wx/html/winpars.h"

#include "wx/html/forcelnk.h"
FORCE_WXHTML_MODULES()


static const wxCoord CELL_BORDER = 2;

const char wxHtmlListBoxNameStr[] = "htmlListBox";
const char wxSimpleHtmlListBoxNameStr[] = "simpleHtmlListBox";



class wxHtmlListBoxCache
{
private:
        void InvalidateItem(size_t n)
    {
        m_items[n] = (size_t)-1;
        wxDELETE(m_cells[n]);
    }

public:
    wxHtmlListBoxCache()
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            m_items[n] = (size_t)-1;
            m_cells[n] = NULL;
        }

        m_next = 0;
    }

    ~wxHtmlListBoxCache()
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            delete m_cells[n];
        }
    }

        void Clear()
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            InvalidateItem(n);
        }
    }

        wxHtmlCell *Get(size_t item) const
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            if ( m_items[n] == item )
                return m_cells[n];
        }

        return NULL;
    }

        bool Has(size_t item) const { return Get(item) != NULL; }

        void Store(size_t item, wxHtmlCell *cell)
    {
        delete m_cells[m_next];
        m_cells[m_next] = cell;
        m_items[m_next] = item;

                if ( ++m_next == SIZE )
            m_next = 0;
    }

        void InvalidateRange(size_t from, size_t to)
    {
        for ( size_t n = 0; n < SIZE; n++ )
        {
            if ( m_items[n] >= from && m_items[n] <= to )
            {
                InvalidateItem(n);
            }
        }
    }

private:
        enum { SIZE = 50 };

        size_t m_next;

        wxHtmlCell *m_cells[SIZE];

        size_t m_items[SIZE];
};


class wxHtmlListBoxStyle : public wxDefaultHtmlRenderingStyle
{
public:
    wxHtmlListBoxStyle(const wxHtmlListBox& hlbox) : m_hlbox(hlbox) { }

    virtual wxColour GetSelectedTextColour(const wxColour& colFg) wxOVERRIDE
    {
                                wxColour col = m_hlbox.GetSelectedTextColour(colFg);
        if ( !col.IsOk() )
        {
            col = wxDefaultHtmlRenderingStyle::GetSelectedTextColour(colFg);
        }

        return col;
    }

    virtual wxColour GetSelectedTextBgColour(const wxColour& colBg) wxOVERRIDE
    {
        wxColour col = m_hlbox.GetSelectedTextBgColour(colBg);
        if ( !col.IsOk() )
        {
            col = wxDefaultHtmlRenderingStyle::GetSelectedTextBgColour(colBg);
        }

        return col;
    }

private:
    const wxHtmlListBox& m_hlbox;

    wxDECLARE_NO_COPY_CLASS(wxHtmlListBoxStyle);
};


wxBEGIN_EVENT_TABLE(wxHtmlListBox, wxVListBox)
    EVT_SIZE(wxHtmlListBox::OnSize)
    EVT_MOTION(wxHtmlListBox::OnMouseMove)
    EVT_LEFT_DOWN(wxHtmlListBox::OnLeftDown)
wxEND_EVENT_TABLE()


wxIMPLEMENT_ABSTRACT_CLASS(wxHtmlListBox, wxVListBox);



wxHtmlListBox::wxHtmlListBox()
    : wxHtmlWindowMouseHelper(this)
{
    Init();
}

wxHtmlListBox::wxHtmlListBox(wxWindow *parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxString& name)
    : wxHtmlWindowMouseHelper(this)
{
    Init();

    (void)Create(parent, id, pos, size, style, name);
}

void wxHtmlListBox::Init()
{
    m_htmlParser = NULL;
    m_htmlRendStyle = new wxHtmlListBoxStyle(*this);
    m_cache = new wxHtmlListBoxCache;
}

bool wxHtmlListBox::Create(wxWindow *parent,
                           wxWindowID id,
                           const wxPoint& pos,
                           const wxSize& size,
                           long style,
                           const wxString& name)
{
    return wxVListBox::Create(parent, id, pos, size, style, name);
}

wxHtmlListBox::~wxHtmlListBox()
{
    delete m_cache;

    if ( m_htmlParser )
    {
        delete m_htmlParser->GetDC();
        delete m_htmlParser;
    }

    delete m_htmlRendStyle;
}


wxColour
wxHtmlListBox::GetSelectedTextColour(const wxColour& WXUNUSED(colFg)) const
{
    return wxNullColour;
}

wxColour
wxHtmlListBox::GetSelectedTextBgColour(const wxColour& WXUNUSED(colBg)) const
{
    return GetSelectionBackground();
}


wxString wxHtmlListBox::OnGetItemMarkup(size_t n) const
{
                return OnGetItem(n);
}


wxHtmlCell* wxHtmlListBox::CreateCellForItem(size_t n) const
{
    if ( !m_htmlParser )
    {
        wxHtmlListBox *self = wxConstCast(this, wxHtmlListBox);

        self->m_htmlParser = new wxHtmlWinParser(self);
        m_htmlParser->SetDC(new wxClientDC(self));
        m_htmlParser->SetFS(&self->m_filesystem);
#if !wxUSE_UNICODE
        if (GetFont().IsOk())
            m_htmlParser->SetInputEncoding(GetFont().GetEncoding());
#endif
                m_htmlParser->SetStandardFonts();
    }

    wxHtmlContainerCell *cell = (wxHtmlContainerCell *)m_htmlParser->
            Parse(OnGetItemMarkup(n));
    wxCHECK_MSG( cell, NULL, wxT("wxHtmlParser::Parse() returned NULL?") );

            cell->SetId(wxString::Format(wxT("%lu"), (unsigned long)n));

    cell->Layout(GetClientSize().x - 2*GetMargins().x);

    return cell;
}

void wxHtmlListBox::CacheItem(size_t n) const
{
    if ( !m_cache->Has(n) )
        m_cache->Store(n, CreateCellForItem(n));
}

void wxHtmlListBox::OnSize(wxSizeEvent& event)
{
        m_cache->Clear();

    event.Skip();
}

void wxHtmlListBox::RefreshRow(size_t line)
{
    m_cache->InvalidateRange(line, line);

    wxVListBox::RefreshRow(line);
}

void wxHtmlListBox::RefreshRows(size_t from, size_t to)
{
    m_cache->InvalidateRange(from, to);

    wxVListBox::RefreshRows(from, to);
}

void wxHtmlListBox::RefreshAll()
{
    m_cache->Clear();

    wxVListBox::RefreshAll();
}

void wxHtmlListBox::SetItemCount(size_t count)
{
        m_cache->Clear();

    wxVListBox::SetItemCount(count);
}


void
wxHtmlListBox::OnDrawBackground(wxDC& dc, const wxRect& rect, size_t n) const
{
    if ( IsSelected(n) )
    {
        if ( DoDrawSolidBackground
             (
                GetSelectedTextBgColour(GetBackgroundColour()),
                dc,
                rect,
                n
             ) )
        {
            return;
        }
            }

    wxVListBox::OnDrawBackground(dc, rect, n);
}

void wxHtmlListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
    CacheItem(n);

    wxHtmlCell *cell = m_cache->Get(n);
    wxCHECK_RET( cell, wxT("this cell should be cached!") );

    wxHtmlRenderingInfo htmlRendInfo;

                if ( IsSelected(n) &&
            (GetSelectedTextColour(*wxBLACK).IsOk() ||
             GetSelectedTextBgColour(*wxWHITE).IsOk()) )
    {
        wxHtmlSelection htmlSel;
        htmlSel.Set(wxPoint(0,0), cell, wxPoint(INT_MAX, INT_MAX), cell);
        htmlRendInfo.SetSelection(&htmlSel);
        htmlRendInfo.SetStyle(m_htmlRendStyle);
        htmlRendInfo.GetState().SetSelectionState(wxHTML_SEL_IN);
    }
        
                cell->Draw(dc,
               rect.x + CELL_BORDER, rect.y + CELL_BORDER,
               0, INT_MAX, htmlRendInfo);
}

wxCoord wxHtmlListBox::OnMeasureItem(size_t n) const
{
                    wxHtmlCell * const cell = CreateCellForItem(n);
    if ( !cell )
        return 0;

    const wxCoord h = cell->GetHeight() + cell->GetDescent() + 4;
    delete cell;

    return h;
}


void wxHtmlListBox::SetHTMLWindowTitle(const wxString& WXUNUSED(title))
{
    }

void wxHtmlListBox::OnHTMLLinkClicked(const wxHtmlLinkInfo& link)
{
    OnLinkClicked(GetItemForCell(link.GetHtmlCell()), link);
}

void wxHtmlListBox::OnLinkClicked(size_t WXUNUSED(n),
                                  const wxHtmlLinkInfo& link)
{
    wxHtmlLinkEvent event(GetId(), link);
    GetEventHandler()->ProcessEvent(event);
}

wxHtmlOpeningStatus
wxHtmlListBox::OnHTMLOpeningURL(wxHtmlURLType WXUNUSED(type),
                                const wxString& WXUNUSED(url),
                                wxString *WXUNUSED(redirect)) const
{
    return wxHTML_OPEN;
}

wxPoint wxHtmlListBox::HTMLCoordsToWindow(wxHtmlCell *cell,
                                          const wxPoint& pos) const
{
    return CellCoordsToPhysical(pos, cell);
}

wxWindow* wxHtmlListBox::GetHTMLWindow() { return this; }

wxColour wxHtmlListBox::GetHTMLBackgroundColour() const
{
    return GetBackgroundColour();
}

void wxHtmlListBox::SetHTMLBackgroundColour(const wxColour& WXUNUSED(clr))
{
    }

void wxHtmlListBox::SetHTMLBackgroundImage(const wxBitmap& WXUNUSED(bmpBg))
{
    }

void wxHtmlListBox::SetHTMLStatusText(const wxString& WXUNUSED(text))
{
    }

wxCursor wxHtmlListBox::GetHTMLCursor(HTMLCursor type) const
{
        if (type == HTMLCursor_Text)
        return wxHtmlWindow::GetDefaultHTMLCursor(HTMLCursor_Default);

        return wxHtmlWindow::GetDefaultHTMLCursor(type);
}


wxPoint wxHtmlListBox::GetRootCellCoords(size_t n) const
{
    wxPoint pos(CELL_BORDER, CELL_BORDER);
    pos += GetMargins();
    pos.y += GetRowsHeight(GetVisibleBegin(), n);
    return pos;
}

bool wxHtmlListBox::PhysicalCoordsToCell(wxPoint& pos, wxHtmlCell*& cell) const
{
    int n = VirtualHitTest(pos.y);
    if ( n == wxNOT_FOUND )
        return false;

        pos -= GetRootCellCoords(n);

    CacheItem(n);
    cell = m_cache->Get(n);

    return true;
}

size_t wxHtmlListBox::GetItemForCell(const wxHtmlCell *cell) const
{
    wxCHECK_MSG( cell, 0, wxT("no cell") );

    cell = cell->GetRootCell();

    wxCHECK_MSG( cell, 0, wxT("no root cell") );

        unsigned long n;
    if ( !cell->GetId().ToULong(&n) )
    {
        wxFAIL_MSG( wxT("unexpected root cell's ID") );
        return 0;
    }

    return n;
}

wxPoint
wxHtmlListBox::CellCoordsToPhysical(const wxPoint& pos, wxHtmlCell *cell) const
{
    return pos + GetRootCellCoords(GetItemForCell(cell));
}

void wxHtmlListBox::OnInternalIdle()
{
    wxVListBox::OnInternalIdle();

    if ( wxHtmlWindowMouseHelper::DidMouseMove() )
    {
        wxPoint pos = ScreenToClient(wxGetMousePosition());
        wxHtmlCell *cell;

        if ( !PhysicalCoordsToCell(pos, cell) )
            return;

        wxHtmlWindowMouseHelper::HandleIdle(cell, pos);
    }
}

void wxHtmlListBox::OnMouseMove(wxMouseEvent& event)
{
    wxHtmlWindowMouseHelper::HandleMouseMoved();
    event.Skip();
}

void wxHtmlListBox::OnLeftDown(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();
    wxHtmlCell *cell;

    if ( !PhysicalCoordsToCell(pos, cell) )
    {
        event.Skip();
        return;
    }

    if ( !wxHtmlWindowMouseHelper::HandleMouseClick(cell, pos, event) )
    {
                        event.Skip();
    }
}



wxIMPLEMENT_ABSTRACT_CLASS(wxSimpleHtmlListBox, wxHtmlListBox);


bool wxSimpleHtmlListBox::Create(wxWindow *parent, wxWindowID id,
                                 const wxPoint& pos,
                                 const wxSize& size,
                                 int n, const wxString choices[],
                                 long style,
                                 const wxValidator& wxVALIDATOR_PARAM(validator),
                                 const wxString& name)
{
    if (!wxHtmlListBox::Create(parent, id, pos, size, style, name))
        return false;

#if wxUSE_VALIDATORS
    SetValidator(validator);
#endif

    Append(n, choices);

    return true;
}

bool wxSimpleHtmlListBox::Create(wxWindow *parent, wxWindowID id,
                                 const wxPoint& pos,
                                 const wxSize& size,
                                 const wxArrayString& choices,
                                 long style,
                                 const wxValidator& wxVALIDATOR_PARAM(validator),
                                 const wxString& name)
{
    if (!wxHtmlListBox::Create(parent, id, pos, size, style, name))
        return false;

#if wxUSE_VALIDATORS
    SetValidator(validator);
#endif

    Append(choices);

    return true;
}

wxSimpleHtmlListBox::~wxSimpleHtmlListBox()
{
    wxItemContainer::Clear();
}

void wxSimpleHtmlListBox::DoClear()
{
    wxASSERT(m_items.GetCount() == m_HTMLclientData.GetCount());

    m_items.Clear();
    m_HTMLclientData.Clear();

    UpdateCount();
}

void wxSimpleHtmlListBox::Clear()
{
    DoClear();
}

void wxSimpleHtmlListBox::DoDeleteOneItem(unsigned int n)
{
    m_items.RemoveAt(n);

    m_HTMLclientData.RemoveAt(n);

    UpdateCount();
}

int wxSimpleHtmlListBox::DoInsertItems(const wxArrayStringsAdapter& items,
                                       unsigned int pos,
                                       void **clientData,
                                       wxClientDataType type)
{
    const unsigned int count = items.GetCount();

    m_items.Insert(wxEmptyString, pos, count);
    m_HTMLclientData.Insert(NULL, pos, count);

    for ( unsigned int i = 0; i < count; ++i, ++pos )
    {
        m_items[pos] = items[i];
        AssignNewItemClientData(pos, clientData, i, type);
    }

    UpdateCount();

    return pos - 1;
}

void wxSimpleHtmlListBox::SetString(unsigned int n, const wxString& s)
{
    wxCHECK_RET( IsValid(n),
                 wxT("invalid index in wxSimpleHtmlListBox::SetString") );

    m_items[n]=s;
    RefreshRow(n);
}

wxString wxSimpleHtmlListBox::GetString(unsigned int n) const
{
    wxCHECK_MSG( IsValid(n), wxEmptyString,
                 wxT("invalid index in wxSimpleHtmlListBox::GetString") );

    return m_items[n];
}

void wxSimpleHtmlListBox::UpdateCount()
{
    wxASSERT(m_items.GetCount() == m_HTMLclientData.GetCount());
    wxHtmlListBox::SetItemCount(m_items.GetCount());

                if (!this->IsFrozen())
        RefreshAll();
}

#endif 