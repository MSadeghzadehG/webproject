
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_GRID

#include "wx/grid.h"

#ifndef WX_PRECOMP
    #include "wx/utils.h"
    #include "wx/dcclient.h"
    #include "wx/settings.h"
    #include "wx/log.h"
    #include "wx/textctrl.h"
    #include "wx/checkbox.h"
    #include "wx/combobox.h"
    #include "wx/intl.h"
    #include "wx/math.h"
    #include "wx/listbox.h"
#endif

#include "wx/valnum.h"
#include "wx/textfile.h"
#include "wx/spinctrl.h"
#include "wx/tokenzr.h"
#include "wx/renderer.h"
#include "wx/headerctrl.h"

#include "wx/generic/gridsel.h"
#include "wx/generic/grideditors.h"
#include "wx/generic/private/grid.h"

#if defined(__WXMOTIF__)
    #define WXUNUSED_MOTIF(identifier)  WXUNUSED(identifier)
#else
    #define WXUNUSED_MOTIF(identifier)  identifier
#endif

#if defined(__WXGTK__)
    #define WXUNUSED_GTK(identifier)    WXUNUSED(identifier)
#else
    #define WXUNUSED_GTK(identifier)    identifier
#endif

#ifdef __WXOSX__
#include "wx/osx/private.h"
#endif

#include <ctype.h>


wxDEFINE_EVENT( wxEVT_GRID_HIDE_EDITOR, wxCommandEvent );


void wxGridCellEditorEvtHandler::OnKillFocus(wxFocusEvent& event)
{
            event.Skip();

        if (m_inSetFocus)
        return;

                
                m_grid->GetEventHandler()->
        AddPendingEvent(wxCommandEvent(wxEVT_GRID_HIDE_EDITOR));
}

void wxGridCellEditorEvtHandler::OnKeyDown(wxKeyEvent& event)
{
    switch ( event.GetKeyCode() )
    {
        case WXK_ESCAPE:
            m_editor->Reset();
            m_grid->DisableCellEditControl();
            break;

        case WXK_TAB:
            m_grid->GetEventHandler()->ProcessEvent( event );
            break;

        case WXK_RETURN:
        case WXK_NUMPAD_ENTER:
            if (!m_grid->GetEventHandler()->ProcessEvent(event))
                m_editor->HandleReturn(event);
            break;

        default:
            event.Skip();
            break;
    }
}

void wxGridCellEditorEvtHandler::OnChar(wxKeyEvent& event)
{
    int row = m_grid->GetGridCursorRow();
    int col = m_grid->GetGridCursorCol();
    wxRect rect = m_grid->CellToRect( row, col );
    int cw, ch;
    m_grid->GetGridWindow()->GetClientSize( &cw, &ch );

        bool wholeCellVisible = (rect.GetWidth() < cw);

    switch ( event.GetKeyCode() )
    {
        case WXK_ESCAPE:
        case WXK_TAB:
        case WXK_RETURN:
        case WXK_NUMPAD_ENTER:
            break;

        case WXK_HOME:
        {
            if ( wholeCellVisible )
            {
                                event.Skip();
                break;
            }

            
                        int colXPos = 0;
            for ( int i = 0; i < col; i++ )
            {
                colXPos += m_grid->GetColSize(i);
            }

            int xUnit = 1, yUnit = 1;
            m_grid->GetScrollPixelsPerUnit(&xUnit, &yUnit);
            if (col != 0)
            {
                m_grid->Scroll(colXPos / xUnit - 1, m_grid->GetScrollPos(wxVERTICAL));
            }
            else
            {
                m_grid->Scroll(colXPos / xUnit, m_grid->GetScrollPos(wxVERTICAL));
            }
            event.Skip();
            break;
        }

        case WXK_END:
        {
            if ( wholeCellVisible )
            {
                                event.Skip();
                break;
            }

            
            int textWidth = 0;
            wxString value = m_grid->GetCellValue(row, col);
            if ( wxEmptyString != value )
            {
                                int y;
                wxFont font = m_grid->GetCellFont(row, col);
                m_grid->GetTextExtent(value, &textWidth, &y, NULL, NULL, &font);

                                int client_right = m_grid->GetGridWindow()->GetClientSize().GetWidth();

                                                                textWidth -= (client_right - (m_grid->GetScrollLineX() * 2));
                if ( textWidth < 0 )
                {
                    textWidth = 0;
                }
            }

                        int colXPos = 0;
            for ( int i = 0; i < col; i++ )
            {
                colXPos += m_grid->GetColSize(i);
            }

                                    colXPos += textWidth;

            int xUnit = 1, yUnit = 1;
            m_grid->GetScrollPixelsPerUnit(&xUnit, &yUnit);
            m_grid->Scroll(colXPos / xUnit - 1, m_grid->GetScrollPos(wxVERTICAL));
            event.Skip();
            break;
        }

        default:
            event.Skip();
            break;
    }
}


wxGridCellEditor::wxGridCellEditor()
{
    m_control = NULL;
    m_attr = NULL;
}

wxGridCellEditor::~wxGridCellEditor()
{
    Destroy();
}

void wxGridCellEditor::Create(wxWindow* WXUNUSED(parent),
                              wxWindowID WXUNUSED(id),
                              wxEvtHandler* evtHandler)
{
    if ( evtHandler )
        m_control->PushEventHandler(evtHandler);
}

void wxGridCellEditor::PaintBackground(wxDC& dc,
                                       const wxRect& rectCell,
                                       const wxGridCellAttr& attr)
{
        dc.SetPen(*wxTRANSPARENT_PEN);
    dc.SetBrush(wxBrush(attr.GetBackgroundColour()));
    dc.DrawRectangle(rectCell);
}

void wxGridCellEditor::Destroy()
{
    if (m_control)
    {
        m_control->PopEventHandler( true  );

        m_control->Destroy();
        m_control = NULL;
    }
}

void wxGridCellEditor::Show(bool show, wxGridCellAttr *attr)
{
    wxASSERT_MSG(m_control, wxT("The wxGridCellEditor must be created first!"));

    m_control->Show(show);

    if ( show )
    {
                if ( attr )
        {
            m_colFgOld = m_control->GetForegroundColour();
            m_control->SetForegroundColour(attr->GetTextColour());

            m_colBgOld = m_control->GetBackgroundColour();
            m_control->SetBackgroundColour(attr->GetBackgroundColour());

#if !defined(__WXGTK__) || defined(__WXGTK20__)
            m_fontOld = m_control->GetFont();
            m_control->SetFont(attr->GetFont());
#endif

                                }
    }
    else
    {
                if ( m_colFgOld.IsOk() )
        {
            m_control->SetForegroundColour(m_colFgOld);
            m_colFgOld = wxNullColour;
        }

        if ( m_colBgOld.IsOk() )
        {
            m_control->SetBackgroundColour(m_colBgOld);
            m_colBgOld = wxNullColour;
        }

#if !defined(__WXGTK__) || defined(__WXGTK20__)
        if ( m_fontOld.IsOk() )
        {
            m_control->SetFont(m_fontOld);
            m_fontOld = wxNullFont;
        }
#endif
    }
}

void wxGridCellEditor::SetSize(const wxRect& rect)
{
    wxASSERT_MSG(m_control, wxT("The wxGridCellEditor must be created first!"));

    m_control->SetSize(rect, wxSIZE_ALLOW_MINUS_ONE);
}

void wxGridCellEditor::HandleReturn(wxKeyEvent& event)
{
    event.Skip();
}

bool wxGridCellEditor::IsAcceptedKey(wxKeyEvent& event)
{
    bool ctrl = event.ControlDown();
    bool alt;

#ifdef __WXMAC__
            alt = event.MetaDown();
#else     alt = event.AltDown();
#endif 
                if ((ctrl || alt) && !(ctrl && alt))
        return false;

#if wxUSE_UNICODE
    if ( static_cast<int>(event.GetUnicodeKey()) == WXK_NONE )
        return false;
#else
    if ( event.GetKeyCode() > WXK_START )
        return false;
#endif

    return true;
}

void wxGridCellEditor::StartingKey(wxKeyEvent& event)
{
    event.Skip();
}

void wxGridCellEditor::StartingClick()
{
}

#if wxUSE_TEXTCTRL


wxGridCellTextEditor::wxGridCellTextEditor(size_t maxChars)
{
    m_maxChars = maxChars;
}

void wxGridCellTextEditor::Create(wxWindow* parent,
                                  wxWindowID id,
                                  wxEvtHandler* evtHandler)
{
    DoCreate(parent, id, evtHandler);
}

void wxGridCellTextEditor::DoCreate(wxWindow* parent,
                                    wxWindowID id,
                                    wxEvtHandler* evtHandler,
                                    long style)
{
    style |= wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB | wxNO_BORDER;

    wxTextCtrl* const text = new wxTextCtrl(parent, id, wxEmptyString,
                                            wxDefaultPosition, wxDefaultSize,
                                            style);
    text->SetMargins(0, 0);
    m_control = text;

#ifdef __WXOSX__
    wxWidgetImpl* impl = m_control->GetPeer();
    impl->SetNeedsFocusRect(false);
#endif
        if ( m_maxChars != 0 )
    {
        Text()->SetMaxLength(m_maxChars);
    }
#if wxUSE_VALIDATORS
        if ( m_validator )
    {
        Text()->SetValidator(*m_validator);
    }
#endif

    wxGridCellEditor::Create(parent, id, evtHandler);
}

void wxGridCellTextEditor::PaintBackground(wxDC& WXUNUSED(dc),
                                           const wxRect& WXUNUSED(rectCell),
                                           const wxGridCellAttr& WXUNUSED(attr))
{
        }

void wxGridCellTextEditor::SetSize(const wxRect& rectOrig)
{
    wxRect rect(rectOrig);

                #if defined(__WXGTK__)
    if (rect.x != 0)
    {
        rect.x += 1;
        rect.y += 1;
        rect.width -= 1;
        rect.height -= 1;
    }
#elif defined(__WXMSW__)
    if ( rect.x == 0 )
        rect.x += 2;
    else
        rect.x += 3;

    if ( rect.y == 0 )
        rect.y += 2;
    else
        rect.y += 3;

    rect.width -= 2;
    rect.height -= 2;
#elif defined(__WXOSX__)
    rect.x += 1;
    rect.y += 1;
    
    rect.width -= 1;
    rect.height -= 1;
#else
    int extra_x = ( rect.x > 2 ) ? 2 : 1;
    int extra_y = ( rect.y > 2 ) ? 2 : 1;

    #if defined(__WXMOTIF__)
        extra_x *= 2;
        extra_y *= 2;
    #endif

    rect.SetLeft( wxMax(0, rect.x - extra_x) );
    rect.SetTop( wxMax(0, rect.y - extra_y) );
    rect.SetRight( rect.GetRight() + 2 * extra_x );
    rect.SetBottom( rect.GetBottom() + 2 * extra_y );
#endif

    wxGridCellEditor::SetSize(rect);
}

void wxGridCellTextEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    wxASSERT_MSG(m_control, wxT("The wxGridCellEditor must be created first!"));

    m_value = grid->GetTable()->GetValue(row, col);

    DoBeginEdit(m_value);
}

void wxGridCellTextEditor::DoBeginEdit(const wxString& startValue)
{
    Text()->SetValue(startValue);
    Text()->SetInsertionPointEnd();
    Text()->SelectAll();
    Text()->SetFocus();
}

bool wxGridCellTextEditor::EndEdit(int WXUNUSED(row),
                                   int WXUNUSED(col),
                                   const wxGrid* WXUNUSED(grid),
                                   const wxString& WXUNUSED(oldval),
                                   wxString *newval)
{
    wxCHECK_MSG( m_control, false,
                 "wxGridCellTextEditor must be created first!" );

    const wxString value = Text()->GetValue();
    if ( value == m_value )
        return false;

    m_value = value;

    if ( newval )
        *newval = m_value;

    return true;
}

void wxGridCellTextEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    grid->GetTable()->SetValue(row, col, m_value);
    m_value.clear();
}

void wxGridCellTextEditor::Reset()
{
    wxASSERT_MSG( m_control, "wxGridCellTextEditor must be created first!" );

    DoReset(m_value);
}

void wxGridCellTextEditor::DoReset(const wxString& startValue)
{
    Text()->SetValue(startValue);
    Text()->SetInsertionPointEnd();
}

bool wxGridCellTextEditor::IsAcceptedKey(wxKeyEvent& event)
{
    switch ( event.GetKeyCode() )
    {
        case WXK_DELETE:
        case WXK_BACK:
            return true;

        default:
            return wxGridCellEditor::IsAcceptedKey(event);
    }
}

void wxGridCellTextEditor::StartingKey(wxKeyEvent& event)
{
                
    wxTextCtrl* tc = Text();
    int ch;

    bool isPrintable;

#if wxUSE_UNICODE
    ch = event.GetUnicodeKey();
    if ( ch != WXK_NONE )
        isPrintable = true;
    else
#endif     {
        ch = event.GetKeyCode();
        isPrintable = ch >= WXK_SPACE && ch < WXK_START;
    }

    switch (ch)
    {
        case WXK_DELETE:
                        tc->Remove(0, 1);
            break;

        case WXK_BACK:
                        {
                const long pos = tc->GetLastPosition();
                tc->Remove(pos - 1, pos);
            }
            break;

        default:
            if ( isPrintable )
                tc->WriteText(static_cast<wxChar>(ch));
            break;
    }
}

void wxGridCellTextEditor::HandleReturn( wxKeyEvent&
                                         WXUNUSED_GTK(WXUNUSED_MOTIF(event)) )
{
#if defined(__WXMOTIF__) || defined(__WXGTK__)
        size_t pos = (size_t)( Text()->GetInsertionPoint() );
    wxString s( Text()->GetValue() );
    s = s.Left(pos) + wxT("\n") + s.Mid(pos);
    Text()->SetValue(s);
    Text()->SetInsertionPoint( pos );
#else
            event.Skip();
#endif
}

void wxGridCellTextEditor::SetParameters(const wxString& params)
{
    if ( !params )
    {
                m_maxChars = 0;
    }
    else
    {
        long tmp;
        if ( params.ToLong(&tmp) )
        {
            m_maxChars = (size_t)tmp;
        }
        else
        {
            wxLogDebug( wxT("Invalid wxGridCellTextEditor parameter string '%s' ignored"), params.c_str() );
        }
    }
}

#if wxUSE_VALIDATORS
void wxGridCellTextEditor::SetValidator(const wxValidator& validator)
{
    m_validator.reset(static_cast<wxValidator*>(validator.Clone()));
}
#endif

wxGridCellEditor *wxGridCellTextEditor::Clone() const
{
    wxGridCellTextEditor* editor = new wxGridCellTextEditor(m_maxChars);
#if wxUSE_VALIDATORS
    if ( m_validator )
    {
        editor->SetValidator(*m_validator);
    }
#endif
    return editor;
}

wxString wxGridCellTextEditor::GetValue() const
{
    return Text()->GetValue();
}


wxGridCellNumberEditor::wxGridCellNumberEditor(int min, int max)
{
    m_min = min;
    m_max = max;
}

void wxGridCellNumberEditor::Create(wxWindow* parent,
                                    wxWindowID id,
                                    wxEvtHandler* evtHandler)
{
#if wxUSE_SPINCTRL
    if ( HasRange() )
    {
                m_control = new wxSpinCtrl(parent, wxID_ANY, wxEmptyString,
                                   wxDefaultPosition, wxDefaultSize,
                                   wxSP_ARROW_KEYS,
                                   m_min, m_max);

        wxGridCellEditor::Create(parent, id, evtHandler);
    }
    else
#endif
    {
                wxGridCellTextEditor::Create(parent, id, evtHandler);

#if wxUSE_VALIDATORS
        Text()->SetValidator(wxIntegerValidator<int>());
#endif
    }
}

void wxGridCellNumberEditor::BeginEdit(int row, int col, wxGrid* grid)
{
        wxGridTableBase *table = grid->GetTable();
    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_NUMBER) )
    {
        m_value = table->GetValueAsLong(row, col);
    }
    else
    {
        m_value = 0;
        wxString sValue = table->GetValue(row, col);
        if (! sValue.ToLong(&m_value) && ! sValue.empty())
        {
            wxFAIL_MSG( wxT("this cell doesn't have numeric value") );
            return;
        }
    }

#if wxUSE_SPINCTRL
    if ( HasRange() )
    {
        Spin()->SetValue((int)m_value);
        Spin()->SetFocus();
    }
    else
#endif
    {
        DoBeginEdit(GetString());
    }
}

bool wxGridCellNumberEditor::EndEdit(int WXUNUSED(row),
                                     int WXUNUSED(col),
                                     const wxGrid* WXUNUSED(grid),
                                     const wxString& oldval, wxString *newval)
{
    long value = 0;
    wxString text;

#if wxUSE_SPINCTRL
    if ( HasRange() )
    {
        value = Spin()->GetValue();
        if ( value == m_value )
            return false;

        text.Printf(wxT("%ld"), value);
    }
    else #endif     {
        text = Text()->GetValue();
        if ( text.empty() )
        {
            if ( oldval.empty() )
                return false;
        }
        else         {
            if ( !text.ToLong(&value) )
                return false;

                                    if ( value == m_value && (value || !oldval.empty()) )
                return false;
        }
    }

    m_value = value;

    if ( newval )
        *newval = text;

    return true;
}

void wxGridCellNumberEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    wxGridTableBase * const table = grid->GetTable();
    if ( table->CanSetValueAs(row, col, wxGRID_VALUE_NUMBER) )
        table->SetValueAsLong(row, col, m_value);
    else
        table->SetValue(row, col, wxString::Format("%ld", m_value));
}

void wxGridCellNumberEditor::Reset()
{
#if wxUSE_SPINCTRL
    if ( HasRange() )
    {
        Spin()->SetValue((int)m_value);
    }
    else
#endif
    {
        DoReset(GetString());
    }
}

bool wxGridCellNumberEditor::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxGridCellEditor::IsAcceptedKey(event) )
    {
        int keycode = event.GetKeyCode();
        if ( (keycode < 128) &&
             (wxIsdigit(keycode) || keycode == '+' || keycode == '-'))
        {
            return true;
        }
    }

    return false;
}

void wxGridCellNumberEditor::StartingKey(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    if ( !HasRange() )
    {
        if ( wxIsdigit(keycode) || keycode == '+' || keycode == '-')
        {
            wxGridCellTextEditor::StartingKey(event);

                        return;
        }
    }
#if wxUSE_SPINCTRL
    else
    {
        if ( wxIsdigit(keycode) )
        {
            wxSpinCtrl* spin = (wxSpinCtrl*)m_control;
            spin->SetValue(keycode - '0');
            spin->SetSelection(1,1);
            return;
        }
    }
#endif

    event.Skip();
}

void wxGridCellNumberEditor::SetParameters(const wxString& params)
{
    if ( !params )
    {
                m_min =
        m_max = -1;
    }
    else
    {
        long tmp;
        if ( params.BeforeFirst(wxT(',')).ToLong(&tmp) )
        {
            m_min = (int)tmp;

            if ( params.AfterFirst(wxT(',')).ToLong(&tmp) )
            {
                m_max = (int)tmp;

                                return;
            }
        }

        wxLogDebug(wxT("Invalid wxGridCellNumberEditor parameter string '%s' ignored"), params.c_str());
    }
}

wxString wxGridCellNumberEditor::GetValue() const
{
    wxString s;

#if wxUSE_SPINCTRL
    if ( HasRange() )
    {
        long value = Spin()->GetValue();
        s.Printf(wxT("%ld"), value);
    }
    else
#endif
    {
        s = Text()->GetValue();
    }

    return s;
}


wxGridCellFloatEditor::wxGridCellFloatEditor(int width,
                                             int precision,
                                             int format)
{
    m_width = width;
    m_precision = precision;
    m_style = format;
}

void wxGridCellFloatEditor::Create(wxWindow* parent,
                                   wxWindowID id,
                                   wxEvtHandler* evtHandler)
{
    wxGridCellTextEditor::Create(parent, id, evtHandler);

#if wxUSE_VALIDATORS
    Text()->SetValidator(wxFloatingPointValidator<double>(m_precision));
#endif
}

void wxGridCellFloatEditor::BeginEdit(int row, int col, wxGrid* grid)
{
        wxGridTableBase * const table = grid->GetTable();
    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_FLOAT) )
    {
        m_value = table->GetValueAsDouble(row, col);
    }
    else
    {
        m_value = 0.0;

        const wxString value = table->GetValue(row, col);
        if ( !value.empty() )
        {
            if ( !value.ToDouble(&m_value) )
            {
                wxFAIL_MSG( wxT("this cell doesn't have float value") );
                return;
            }
        }
    }

    DoBeginEdit(GetString());
}

bool wxGridCellFloatEditor::EndEdit(int WXUNUSED(row),
                                    int WXUNUSED(col),
                                    const wxGrid* WXUNUSED(grid),
                                    const wxString& oldval, wxString *newval)
{
    const wxString text(Text()->GetValue());

    double value;
    if ( !text.empty() )
    {
        if ( !text.ToDouble(&value) )
            return false;
    }
    else     {
        if ( oldval.empty() )
            return false;           
        value = 0.;
    }

            if ( wxIsSameDouble(value, m_value) && !text.empty() && !oldval.empty() )
        return false;           
    m_value = value;

    if ( newval )
        *newval = text;

    return true;
}

void wxGridCellFloatEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    wxGridTableBase * const table = grid->GetTable();

    if ( table->CanSetValueAs(row, col, wxGRID_VALUE_FLOAT) )
        table->SetValueAsDouble(row, col, m_value);
    else
        table->SetValue(row, col, Text()->GetValue());
}

void wxGridCellFloatEditor::Reset()
{
    DoReset(GetString());
}

void wxGridCellFloatEditor::StartingKey(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    char tmpbuf[2];
    tmpbuf[0] = (char) keycode;
    tmpbuf[1] = '\0';
    wxString strbuf(tmpbuf, *wxConvCurrent);

#if wxUSE_INTL
    bool is_decimal_point = ( strbuf ==
       wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT, wxLOCALE_CAT_NUMBER) );
#else
    bool is_decimal_point = ( strbuf == wxT(".") );
#endif

    if ( wxIsdigit(keycode) || keycode == '+' || keycode == '-'
         || is_decimal_point )
    {
        wxGridCellTextEditor::StartingKey(event);

                return;
    }

    event.Skip();
}

void wxGridCellFloatEditor::SetParameters(const wxString& params)
{
    if ( !params )
    {
                m_width =
        m_precision = -1;
        m_style = wxGRID_FLOAT_FORMAT_DEFAULT;
        m_format.clear();
    }
    else
    {
        wxString rest;
        wxString tmp = params.BeforeFirst(wxT(','), &rest);
        if ( !tmp.empty() )
        {
            long width;
            if ( tmp.ToLong(&width) )
            {
                m_width = (int)width;
            }
            else
            {
                wxLogDebug(wxT("Invalid wxGridCellFloatRenderer width parameter string '%s ignored"), params.c_str());
            }
        }

        tmp = rest.BeforeFirst(wxT(','));
        if ( !tmp.empty() )
        {
            long precision;
            if ( tmp.ToLong(&precision) )
            {
                m_precision = (int)precision;
            }
            else
            {
                wxLogDebug(wxT("Invalid wxGridCellFloatRenderer precision parameter string '%s ignored"), params.c_str());
            }
        }

        tmp = rest.AfterFirst(wxT(','));
        if ( !tmp.empty() )
        {
            if ( tmp[0] == wxT('f') )
            {
                m_style = wxGRID_FLOAT_FORMAT_FIXED;
            }
            else if ( tmp[0] == wxT('e') )
            {
                m_style = wxGRID_FLOAT_FORMAT_SCIENTIFIC;
            }
            else if ( tmp[0] == wxT('g') )
            {
                m_style = wxGRID_FLOAT_FORMAT_COMPACT;
            }
            else if ( tmp[0] == wxT('E') )
            {
                m_style = wxGRID_FLOAT_FORMAT_SCIENTIFIC |
                          wxGRID_FLOAT_FORMAT_UPPER;
            }
            else if ( tmp[0] == wxT('F') )
            {
                m_style = wxGRID_FLOAT_FORMAT_FIXED |
                          wxGRID_FLOAT_FORMAT_UPPER;
            }
            else if ( tmp[0] == wxT('G') )
            {
                m_style = wxGRID_FLOAT_FORMAT_COMPACT |
                          wxGRID_FLOAT_FORMAT_UPPER;
            }
            else
            {
                wxLogDebug("Invalid wxGridCellFloatRenderer format "
                           "parameter string '%s ignored", params);
            }
        }
    }
}

wxString wxGridCellFloatEditor::GetString()
{
    if ( !m_format )
    {
        if ( m_precision == -1 && m_width != -1)
        {
                        m_format.Printf(wxT("%%%d."), m_width);
        }
        else if ( m_precision != -1 && m_width == -1)
        {
                        m_format.Printf(wxT("%%.%d"), m_precision);
        }
        else if ( m_precision != -1 && m_width != -1 )
        {
            m_format.Printf(wxT("%%%d.%d"), m_width, m_precision);
        }
        else
        {
                        m_format = wxT("%");
        }

        bool isUpper = (m_style & wxGRID_FLOAT_FORMAT_UPPER) != 0;
        if ( m_style & wxGRID_FLOAT_FORMAT_SCIENTIFIC )
            m_format += isUpper ? wxT('E') : wxT('e');
        else if ( m_style & wxGRID_FLOAT_FORMAT_COMPACT )
            m_format += isUpper ? wxT('G') : wxT('g');
        else
            m_format += wxT('f');
    }

    return wxString::Format(m_format, m_value);
}

bool wxGridCellFloatEditor::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxGridCellEditor::IsAcceptedKey(event) )
    {
        const int keycode = event.GetKeyCode();
        if ( wxIsascii(keycode) )
        {
#if wxUSE_INTL
            const wxString decimalPoint =
                wxLocale::GetInfo(wxLOCALE_DECIMAL_POINT, wxLOCALE_CAT_NUMBER);
#else
            const wxString decimalPoint(wxT('.'));
#endif

                        if ( wxIsdigit(keycode) ||
                    tolower(keycode) == 'e' ||
                        keycode == decimalPoint ||
                            keycode == '+' ||
                                keycode == '-' )
            {
                return true;
            }
        }
    }

    return false;
}

#endif 
#if wxUSE_CHECKBOX


wxString wxGridCellBoolEditor::ms_stringValues[2] = { wxT(""), wxT("1") };

void wxGridCellBoolEditor::Create(wxWindow* parent,
                                  wxWindowID id,
                                  wxEvtHandler* evtHandler)
{
    m_control = new wxCheckBox(parent, id, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize,
                               wxNO_BORDER);

    wxGridCellEditor::Create(parent, id, evtHandler);
}

void wxGridCellBoolEditor::SetSize(const wxRect& r)
{
    bool resize = false;
    wxSize size = m_control->GetSize();
    wxCoord minSize = wxMin(r.width, r.height);

        wxSize sizeBest = m_control->GetBestSize();
    if ( !(size == sizeBest) )
    {
                size = sizeBest;

        resize = true;
    }

    if ( size.x >= minSize || size.y >= minSize )
    {
                size.x = size.y = minSize - 2;

        resize = true;
    }

    if ( resize )
    {
        m_control->SetSize(size);
    }

    
#if defined(__WXGTK__) || defined (__WXMOTIF__)
            size.x -= 8;
#elif defined(__WXMSW__)
        size.x += 1;
    size.y -= 2;
#endif

    int hAlign = wxALIGN_CENTRE;
    int vAlign = wxALIGN_CENTRE;
    if (GetCellAttr())
        GetCellAttr()->GetAlignment(& hAlign, & vAlign);

    int x = 0, y = 0;
    if (hAlign == wxALIGN_LEFT)
    {
        x = r.x + 2;

#ifdef __WXMSW__
        x += 2;
#endif

        y = r.y + r.height / 2 - size.y / 2;
    }
    else if (hAlign == wxALIGN_RIGHT)
    {
        x = r.x + r.width - size.x - 2;
        y = r.y + r.height / 2 - size.y / 2;
    }
    else if (hAlign == wxALIGN_CENTRE)
    {
        x = r.x + r.width / 2 - size.x / 2;
        y = r.y + r.height / 2 - size.y / 2;
    }

    m_control->Move(x, y);
}

void wxGridCellBoolEditor::Show(bool show, wxGridCellAttr *attr)
{
    m_control->Show(show);

    if ( show )
    {
        wxColour colBg = attr ? attr->GetBackgroundColour() : *wxLIGHT_GREY;
        CBox()->SetBackgroundColour(colBg);
    }
}

void wxGridCellBoolEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    wxASSERT_MSG(m_control,
                 wxT("The wxGridCellEditor must be created first!"));

    if (grid->GetTable()->CanGetValueAs(row, col, wxGRID_VALUE_BOOL))
    {
        m_value = grid->GetTable()->GetValueAsBool(row, col);
    }
    else
    {
        wxString cellval( grid->GetTable()->GetValue(row, col) );

        if ( cellval == ms_stringValues[false] )
            m_value = false;
        else if ( cellval == ms_stringValues[true] )
            m_value = true;
        else
        {
                                                            wxFAIL_MSG( wxT("invalid value for a cell with bool editor!") );
        }
    }

    CBox()->SetValue(m_value);
    CBox()->SetFocus();
}

bool wxGridCellBoolEditor::EndEdit(int WXUNUSED(row),
                                   int WXUNUSED(col),
                                   const wxGrid* WXUNUSED(grid),
                                   const wxString& WXUNUSED(oldval),
                                   wxString *newval)
{
    bool value = CBox()->GetValue();
    if ( value == m_value )
        return false;

    m_value = value;

    if ( newval )
        *newval = GetValue();

    return true;
}

void wxGridCellBoolEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    wxGridTableBase * const table = grid->GetTable();
    if ( table->CanSetValueAs(row, col, wxGRID_VALUE_BOOL) )
        table->SetValueAsBool(row, col, m_value);
    else
        table->SetValue(row, col, GetValue());
}

void wxGridCellBoolEditor::Reset()
{
    wxASSERT_MSG(m_control,
                 wxT("The wxGridCellEditor must be created first!"));

    CBox()->SetValue(m_value);
}

void wxGridCellBoolEditor::StartingClick()
{
    CBox()->SetValue(!CBox()->GetValue());
}

bool wxGridCellBoolEditor::IsAcceptedKey(wxKeyEvent& event)
{
    if ( wxGridCellEditor::IsAcceptedKey(event) )
    {
        int keycode = event.GetKeyCode();
        switch ( keycode )
        {
            case WXK_SPACE:
            case '+':
            case '-':
                return true;
        }
    }

    return false;
}

void wxGridCellBoolEditor::StartingKey(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    switch ( keycode )
    {
        case WXK_SPACE:
            CBox()->SetValue(!CBox()->GetValue());
            break;

        case '+':
            CBox()->SetValue(true);
            break;

        case '-':
            CBox()->SetValue(false);
            break;
    }
}

wxString wxGridCellBoolEditor::GetValue() const
{
  return ms_stringValues[CBox()->GetValue()];
}

 void
wxGridCellBoolEditor::UseStringValues(const wxString& valueTrue,
                                      const wxString& valueFalse)
{
    ms_stringValues[false] = valueFalse;
    ms_stringValues[true] = valueTrue;
}

 bool
wxGridCellBoolEditor::IsTrueValue(const wxString& value)
{
    return value == ms_stringValues[true];
}

#endif 
#if wxUSE_COMBOBOX


wxGridCellChoiceEditor::wxGridCellChoiceEditor(const wxArrayString& choices,
                                               bool allowOthers)
    : m_choices(choices),
      m_allowOthers(allowOthers) { }

wxGridCellChoiceEditor::wxGridCellChoiceEditor(size_t count,
                                               const wxString choices[],
                                               bool allowOthers)
                      : m_allowOthers(allowOthers)
{
    if ( count )
    {
        m_choices.Alloc(count);
        for ( size_t n = 0; n < count; n++ )
        {
            m_choices.Add(choices[n]);
        }
    }
}

wxGridCellEditor *wxGridCellChoiceEditor::Clone() const
{
    wxGridCellChoiceEditor *editor = new wxGridCellChoiceEditor;
    editor->m_allowOthers = m_allowOthers;
    editor->m_choices = m_choices;

    return editor;
}

void wxGridCellChoiceEditor::Create(wxWindow* parent,
                                    wxWindowID id,
                                    wxEvtHandler* evtHandler)
{
    int style = wxTE_PROCESS_ENTER |
                wxTE_PROCESS_TAB |
                wxBORDER_NONE;

    if ( !m_allowOthers )
        style |= wxCB_READONLY;
    m_control = new wxComboBox(parent, id, wxEmptyString,
                               wxDefaultPosition, wxDefaultSize,
                               m_choices,
                               style);

    wxGridCellEditor::Create(parent, id, evtHandler);
}

void wxGridCellChoiceEditor::SetSize(const wxRect& rect)
{
    wxASSERT_MSG(m_control,
                 wxT("The wxGridCellChoiceEditor must be created first!"));

        wxRect rectTallEnough = rect;
    const wxSize bestSize = m_control->GetBestSize();
    const wxCoord diffY = bestSize.GetHeight() - rectTallEnough.GetHeight();
    if ( diffY > 0 )
    {
                rectTallEnough.height += diffY;

                        rectTallEnough.y -= diffY/2;
    }
    
    wxGridCellEditor::SetSize(rectTallEnough);
}

void wxGridCellChoiceEditor::PaintBackground(wxDC& dc,
                                             const wxRect& rectCell,
                                             const wxGridCellAttr& attr)
{
        
                wxGridCellEditor::PaintBackground(dc, rectCell, attr);
}

void wxGridCellChoiceEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    wxASSERT_MSG(m_control,
                 wxT("The wxGridCellEditor must be created first!"));

    wxGridCellEditorEvtHandler* evtHandler = NULL;
    if (m_control)
        evtHandler = wxDynamicCast(m_control->GetEventHandler(), wxGridCellEditorEvtHandler);

        if (evtHandler)
        evtHandler->SetInSetFocus(true);

    m_value = grid->GetTable()->GetValue(row, col);

    Reset(); 
    Combo()->SetFocus();

#ifdef __WXOSX_COCOA__
                    Combo()->Popup();
#endif

    if (evtHandler)
    {
                #if !defined(__WXGTK20__)
        evtHandler->SetInSetFocus(false);
#endif
    }
}

bool wxGridCellChoiceEditor::EndEdit(int WXUNUSED(row),
                                     int WXUNUSED(col),
                                     const wxGrid* WXUNUSED(grid),
                                     const wxString& WXUNUSED(oldval),
                                     wxString *newval)
{
    const wxString value = Combo()->GetValue();
    if ( value == m_value )
        return false;

    m_value = value;

    if ( newval )
        *newval = value;

    return true;
}

void wxGridCellChoiceEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    grid->GetTable()->SetValue(row, col, m_value);
}

void wxGridCellChoiceEditor::Reset()
{
    if (m_allowOthers)
    {
        Combo()->SetValue(m_value);
        Combo()->SetInsertionPointEnd();
    }
    else     {
                int pos = Combo()->FindString(m_value);
        if (pos == wxNOT_FOUND)
            pos = 0;
        Combo()->SetSelection(pos);
    }
}

void wxGridCellChoiceEditor::SetParameters(const wxString& params)
{
    if ( !params )
    {
                return;
    }

    m_choices.Empty();

    wxStringTokenizer tk(params, wxT(','));
    while ( tk.HasMoreTokens() )
    {
        m_choices.Add(tk.GetNextToken());
    }
}

wxString wxGridCellChoiceEditor::GetValue() const
{
  return Combo()->GetValue();
}

#endif 
#if wxUSE_COMBOBOX



wxGridCellEnumEditor::wxGridCellEnumEditor(const wxString& choices)
                     :wxGridCellChoiceEditor()
{
    m_index = -1;

    if (!choices.empty())
        SetParameters(choices);
}

wxGridCellEditor *wxGridCellEnumEditor::Clone() const
{
    wxGridCellEnumEditor *editor = new wxGridCellEnumEditor();
    editor->m_index = m_index;
    return editor;
}

void wxGridCellEnumEditor::BeginEdit(int row, int col, wxGrid* grid)
{
    wxASSERT_MSG(m_control,
                 wxT("The wxGridCellEnumEditor must be Created first!"));

    wxGridCellEditorEvtHandler* evtHandler = NULL;
    if (m_control)
        evtHandler = wxDynamicCast(m_control->GetEventHandler(), wxGridCellEditorEvtHandler);

        if (evtHandler)
        evtHandler->SetInSetFocus(true);

    wxGridTableBase *table = grid->GetTable();

    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_NUMBER) )
    {
        m_index = table->GetValueAsLong(row, col);
    }
    else
    {
        wxString startValue = table->GetValue(row, col);
        if (startValue.IsNumber() && !startValue.empty())
        {
            startValue.ToLong(&m_index);
        }
        else
        {
            m_index = -1;
        }
    }

    Combo()->SetSelection(m_index);
    Combo()->SetFocus();

#ifdef __WXOSX_COCOA__
                    Combo()->Popup();
#endif

    if (evtHandler)
    {
                #if !defined(__WXGTK20__)
        evtHandler->SetInSetFocus(false);
#endif
    }
}

bool wxGridCellEnumEditor::EndEdit(int WXUNUSED(row),
                                   int WXUNUSED(col),
                                   const wxGrid* WXUNUSED(grid),
                                   const wxString& WXUNUSED(oldval),
                                   wxString *newval)
{
    long idx = Combo()->GetSelection();
    if ( idx == m_index )
        return false;

    m_index = idx;

    if ( newval )
        newval->Printf("%ld", m_index);

    return true;
}

void wxGridCellEnumEditor::ApplyEdit(int row, int col, wxGrid* grid)
{
    wxGridTableBase * const table = grid->GetTable();
    if ( table->CanSetValueAs(row, col, wxGRID_VALUE_NUMBER) )
        table->SetValueAsLong(row, col, m_index);
    else
        table->SetValue(row, col, wxString::Format("%ld", m_index));
}

#endif 

void
wxGridCellAutoWrapStringEditor::Create(wxWindow* parent,
                                       wxWindowID id,
                                       wxEvtHandler* evtHandler)
{
    wxGridCellTextEditor::DoCreate(parent, id, evtHandler,
                                    wxTE_MULTILINE | wxTE_RICH);
}


#endif 