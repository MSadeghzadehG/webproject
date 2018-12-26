
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_GRID

#include "wx/generic/gridctrl.h"
#include "wx/generic/grideditors.h"

#ifndef WX_PRECOMP
    #include "wx/textctrl.h"
    #include "wx/dc.h"
    #include "wx/combobox.h"
    #include "wx/settings.h"
    #include "wx/log.h"
    #include "wx/checkbox.h"
#endif 
#include "wx/tokenzr.h"
#include "wx/renderer.h"



void wxGridCellRenderer::Draw(wxGrid& grid,
                              wxGridCellAttr& attr,
                              wxDC& dc,
                              const wxRect& rect,
                              int WXUNUSED(row), int WXUNUSED(col),
                              bool isSelected)
{
    dc.SetBackgroundMode( wxBRUSHSTYLE_SOLID );

    wxColour clr;
    if ( grid.IsThisEnabled() )
    {
        if ( isSelected )
        {
            if ( grid.HasFocus() )
                clr = grid.GetSelectionBackground();
            else
                clr = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW);
        }
        else
        {
            clr = attr.GetBackgroundColour();
        }
    }
    else     {
        clr = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
    }

    dc.SetBrush(clr);
    dc.SetPen( *wxTRANSPARENT_PEN );
    dc.DrawRectangle(rect);
}



#if wxUSE_DATETIME


wxGridCellDateTimeRenderer::wxGridCellDateTimeRenderer(const wxString& outformat, const wxString& informat)
{
    m_iformat = informat;
    m_oformat = outformat;
    m_tz = wxDateTime::Local;
    m_dateDef = wxDefaultDateTime;
}

wxGridCellRenderer *wxGridCellDateTimeRenderer::Clone() const
{
    wxGridCellDateTimeRenderer *renderer = new wxGridCellDateTimeRenderer;
    renderer->m_iformat = m_iformat;
    renderer->m_oformat = m_oformat;
    renderer->m_dateDef = m_dateDef;
    renderer->m_tz = m_tz;

    return renderer;
}

wxString wxGridCellDateTimeRenderer::GetString(const wxGrid& grid, int row, int col)
{
    wxGridTableBase *table = grid.GetTable();

    bool hasDatetime = false;
    wxDateTime val;
    wxString text;
    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_DATETIME) )
    {
        void * tempval = table->GetValueAsCustom(row, col,wxGRID_VALUE_DATETIME);

        if (tempval)
        {
            val = *((wxDateTime *)tempval);
            hasDatetime = true;
            delete (wxDateTime *)tempval;
        }

    }

    if (!hasDatetime )
    {
        text = table->GetValue(row, col);
        const char * const end = val.ParseFormat(text, m_iformat, m_dateDef);
        hasDatetime = end && !*end;
    }

    if ( hasDatetime )
        text = val.Format(m_oformat, m_tz );

        return text;
}

void wxGridCellDateTimeRenderer::Draw(wxGrid& grid,
                                   wxGridCellAttr& attr,
                                   wxDC& dc,
                                   const wxRect& rectCell,
                                   int row, int col,
                                   bool isSelected)
{
    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    SetTextColoursAndFont(grid, attr, dc, isSelected);

        int hAlign = wxALIGN_RIGHT,
        vAlign = wxALIGN_INVALID;
    attr.GetNonDefaultAlignment(&hAlign, &vAlign);

    wxRect rect = rectCell;
    rect.Inflate(-1);

    grid.DrawTextRectangle(dc, GetString(grid, row, col), rect, hAlign, vAlign);
}

wxSize wxGridCellDateTimeRenderer::GetBestSize(wxGrid& grid,
                                            wxGridCellAttr& attr,
                                            wxDC& dc,
                                            int row, int col)
{
    return DoGetBestSize(attr, dc, GetString(grid, row, col));
}

void wxGridCellDateTimeRenderer::SetParameters(const wxString& params)
{
    if (!params.empty())
        m_oformat=params;
}

#endif 


wxGridCellEnumRenderer::wxGridCellEnumRenderer(const wxString& choices)
{
    if (!choices.empty())
        SetParameters(choices);
}

wxGridCellRenderer *wxGridCellEnumRenderer::Clone() const
{
    wxGridCellEnumRenderer *renderer = new wxGridCellEnumRenderer;
    renderer->m_choices = m_choices;
    return renderer;
}

wxString wxGridCellEnumRenderer::GetString(const wxGrid& grid, int row, int col)
{
    wxGridTableBase *table = grid.GetTable();
    wxString text;
    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_NUMBER) )
    {
        int choiceno = table->GetValueAsLong(row, col);
        text.Printf(wxT("%s"), m_choices[ choiceno ].c_str() );
    }
    else
    {
        text = table->GetValue(row, col);
    }


        return text;
}

void wxGridCellEnumRenderer::Draw(wxGrid& grid,
                                   wxGridCellAttr& attr,
                                   wxDC& dc,
                                   const wxRect& rectCell,
                                   int row, int col,
                                   bool isSelected)
{
    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    SetTextColoursAndFont(grid, attr, dc, isSelected);

        int hAlign = wxALIGN_RIGHT,
        vAlign = wxALIGN_INVALID;
    attr.GetNonDefaultAlignment(&hAlign, &vAlign);

    wxRect rect = rectCell;
    rect.Inflate(-1);

    grid.DrawTextRectangle(dc, GetString(grid, row, col), rect, hAlign, vAlign);
}

wxSize wxGridCellEnumRenderer::GetBestSize(wxGrid& grid,
                                            wxGridCellAttr& attr,
                                            wxDC& dc,
                                            int row, int col)
{
    return DoGetBestSize(attr, dc, GetString(grid, row, col));
}

void wxGridCellEnumRenderer::SetParameters(const wxString& params)
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




void
wxGridCellAutoWrapStringRenderer::Draw(wxGrid& grid,
                      wxGridCellAttr& attr,
                      wxDC& dc,
                      const wxRect& rectCell,
                      int row, int col,
                      bool isSelected) {


    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

        SetTextColoursAndFont(grid, attr, dc, isSelected);

    int horizAlign, vertAlign;
    attr.GetAlignment(&horizAlign, &vertAlign);

    wxRect rect = rectCell;
    rect.Inflate(-1);

    grid.DrawTextRectangle(dc, GetTextLines(grid,dc,attr,rect,row,col),
                           rect, horizAlign, vertAlign);
}


wxArrayString
wxGridCellAutoWrapStringRenderer::GetTextLines(wxGrid& grid,
                                               wxDC& dc,
                                               const wxGridCellAttr& attr,
                                               const wxRect& rect,
                                               int row, int col)
{
    dc.SetFont(attr.GetFont());
    const wxCoord maxWidth = rect.GetWidth();

        const wxArrayString
        logicalLines = wxSplit(grid.GetCellValue(row, col), '\n', '\0');

            if ( maxWidth <= 0 )
        return logicalLines;

    wxArrayString physicalLines;
    for ( wxArrayString::const_iterator it = logicalLines.begin();
          it != logicalLines.end();
          ++it )
    {
        const wxString& line = *it;

        if ( dc.GetTextExtent(line).x > maxWidth )
        {
                        BreakLine(dc, line, maxWidth, physicalLines);
        }
        else         {
            physicalLines.push_back(line);
        }
    }

    return physicalLines;
}

void
wxGridCellAutoWrapStringRenderer::BreakLine(wxDC& dc,
                                            const wxString& logicalLine,
                                            wxCoord maxWidth,
                                            wxArrayString& lines)
{
    wxCoord lineWidth = 0;
    wxString line;

        wxStringTokenizer wordTokenizer(logicalLine, wxS(" \t"), wxTOKEN_RET_DELIMS);
    while ( wordTokenizer.HasMoreTokens() )
    {
        const wxString word = wordTokenizer.GetNextToken();
        const wxCoord wordWidth = dc.GetTextExtent(word).x;
        if ( lineWidth + wordWidth < maxWidth )
        {
                        line += word;
            lineWidth += wordWidth;
        }
        else
        {
                                    if ( wordWidth < maxWidth )
            {
                                                lines.push_back(line);
                line = word;
                lineWidth = wordWidth;
            }
            else             {
                if ( !line.empty() )
                {
                    lines.push_back(line);
                    line.clear();
                    lineWidth = 0;
                }

                                lineWidth = BreakWord(dc, word, maxWidth, lines, line);
            }
        }
    }

    if ( !line.empty() )
        lines.push_back(line);
}


wxCoord
wxGridCellAutoWrapStringRenderer::BreakWord(wxDC& dc,
                                            const wxString& word,
                                            wxCoord maxWidth,
                                            wxArrayString& lines,
                                            wxString& line)
{
    wxArrayInt widths;
    dc.GetPartialTextExtents(word, widths);

        const unsigned count = widths.size();
    unsigned n;
    for ( n = 0; n < count; n++ )
    {
        if ( widths[n] > maxWidth )
            break;
    }

    if ( n == 0 )
    {
                                n = 1;
    }

    lines.push_back(word.substr(0, n));

                            const wxString rest = word.substr(n);
    const wxCoord restWidth = dc.GetTextExtent(rest).x;
    if ( restWidth <= maxWidth )
    {
        line = rest;
        return restWidth;
    }

                    return BreakWord(dc, rest, maxWidth, lines, line);
}

wxSize
wxGridCellAutoWrapStringRenderer::GetBestSize(wxGrid& grid,
                                              wxGridCellAttr& attr,
                                              wxDC& dc,
                                              int row, int col)
{
                        const int height = dc.GetCharHeight();

    return wxSize(GetBestWidth(grid, attr, dc, row, col, height), height);
}

static const int AUTOWRAP_Y_MARGIN = 4;

int
wxGridCellAutoWrapStringRenderer::GetBestHeight(wxGrid& grid,
                                                wxGridCellAttr& attr,
                                                wxDC& dc,
                                                int row, int col,
                                                int width)
{
    const int lineHeight = dc.GetCharHeight();

            return GetTextLines(grid, dc, attr, wxSize(width, lineHeight),
                        row, col).size() * lineHeight + AUTOWRAP_Y_MARGIN;
}

int
wxGridCellAutoWrapStringRenderer::GetBestWidth(wxGrid& grid,
                                               wxGridCellAttr& attr,
                                               wxDC& dc,
                                               int row, int col,
                                               int height)
{
    const int lineHeight = dc.GetCharHeight();

        const size_t maxLines = height - AUTOWRAP_Y_MARGIN < lineHeight
                                ? 1
                                : (height - AUTOWRAP_Y_MARGIN)/lineHeight;

                const int charWidth = dc.GetCharWidth();
    int width = 2*charWidth;
    while ( GetTextLines(grid, dc, attr, wxSize(width, height),
                         row, col).size() > maxLines )
        width += charWidth;

    return width;
}


void wxGridCellStringRenderer::SetTextColoursAndFont(const wxGrid& grid,
                                                     const wxGridCellAttr& attr,
                                                     wxDC& dc,
                                                     bool isSelected)
{
    dc.SetBackgroundMode( wxBRUSHSTYLE_TRANSPARENT );

    
        if ( grid.IsThisEnabled() )
    {
        if ( isSelected )
        {
            wxColour clr;
            if ( grid.HasFocus() )
                clr = grid.GetSelectionBackground();
            else
                clr = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNSHADOW);
            dc.SetTextBackground( clr );
            dc.SetTextForeground( grid.GetSelectionForeground() );
        }
        else
        {
            dc.SetTextBackground( attr.GetBackgroundColour() );
            dc.SetTextForeground( attr.GetTextColour() );
        }
    }
    else
    {
        dc.SetTextBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
        dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    }

    dc.SetFont( attr.GetFont() );
}

wxSize wxGridCellStringRenderer::DoGetBestSize(const wxGridCellAttr& attr,
                                               wxDC& dc,
                                               const wxString& text)
{
    wxCoord x = 0, y = 0, max_x = 0;
    dc.SetFont(attr.GetFont());
    wxStringTokenizer tk(text, wxT('\n'));
    while ( tk.HasMoreTokens() )
    {
        dc.GetTextExtent(tk.GetNextToken(), &x, &y);
        max_x = wxMax(max_x, x);
    }

    y *= 1 + text.Freq(wxT('\n')); 
    return wxSize(max_x, y);
}

wxSize wxGridCellStringRenderer::GetBestSize(wxGrid& grid,
                                             wxGridCellAttr& attr,
                                             wxDC& dc,
                                             int row, int col)
{
    return DoGetBestSize(attr, dc, grid.GetCellValue(row, col));
}

void wxGridCellStringRenderer::Draw(wxGrid& grid,
                                    wxGridCellAttr& attr,
                                    wxDC& dc,
                                    const wxRect& rectCell,
                                    int row, int col,
                                    bool isSelected)
{
    wxRect rect = rectCell;
    rect.Inflate(-1);

        wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    int hAlign, vAlign;
    attr.GetAlignment(&hAlign, &vAlign);

    int overflowCols = 0;

    if (attr.GetOverflow())
    {
        int cols = grid.GetNumberCols();
        int best_width = GetBestSize(grid,attr,dc,row,col).GetWidth();
        int cell_rows, cell_cols;
        attr.GetSize( &cell_rows, &cell_cols );         if ((best_width > rectCell.width) && (col < cols) && grid.GetTable())
        {
            int i, c_cols, c_rows;
            for (i = col+cell_cols; i < cols; i++)
            {
                bool is_empty = true;
                for (int j=row; j < row + cell_rows; j++)
                {
                                        grid.GetCellSize(j, i, &c_rows, &c_cols);
                    if (c_rows > 0)
                        c_rows = 0;
                    if (!grid.GetTable()->IsEmptyCell(j + c_rows, i))
                    {
                        is_empty = false;
                        break;
                    }
                }

                if (is_empty)
                {
                    rect.width += grid.GetColSize(i);
                }
                else
                {
                    i--;
                    break;
                }

                if (rect.width >= best_width)
                    break;
            }

            overflowCols = i - col - cell_cols + 1;
            if (overflowCols >= cols)
                overflowCols = cols - 1;
        }

        if (overflowCols > 0)         {
            hAlign = wxALIGN_LEFT;             wxRect clip = rect;
            clip.x += rectCell.width;
                        int col_end = col + cell_cols + overflowCols;
            if (col_end >= grid.GetNumberCols())
                col_end = grid.GetNumberCols() - 1;
            for (int i = col + cell_cols; i <= col_end; i++)
            {
                clip.width = grid.GetColSize(i) - 1;
                dc.DestroyClippingRegion();
                dc.SetClippingRegion(clip);

                SetTextColoursAndFont(grid, attr, dc,
                        grid.IsInSelection(row,i));

                grid.DrawTextRectangle(dc, grid.GetCellValue(row, col),
                        rect, hAlign, vAlign);
                clip.x += grid.GetColSize(i) - 1;
            }

            rect = rectCell;
            rect.Inflate(-1);
            rect.width++;
            dc.DestroyClippingRegion();
        }
    }

        SetTextColoursAndFont(grid, attr, dc, isSelected);

    grid.DrawTextRectangle(dc, grid.GetCellValue(row, col),
                           rect, hAlign, vAlign);
}


wxString wxGridCellNumberRenderer::GetString(const wxGrid& grid, int row, int col)
{
    wxGridTableBase *table = grid.GetTable();
    wxString text;
    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_NUMBER) )
    {
        text.Printf(wxT("%ld"), table->GetValueAsLong(row, col));
    }
    else
    {
        text = table->GetValue(row, col);
    }

    return text;
}

void wxGridCellNumberRenderer::Draw(wxGrid& grid,
                                    wxGridCellAttr& attr,
                                    wxDC& dc,
                                    const wxRect& rectCell,
                                    int row, int col,
                                    bool isSelected)
{
    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    SetTextColoursAndFont(grid, attr, dc, isSelected);

        int hAlign = wxALIGN_RIGHT,
        vAlign = wxALIGN_INVALID;
    attr.GetNonDefaultAlignment(&hAlign, &vAlign);

    wxRect rect = rectCell;
    rect.Inflate(-1);

    grid.DrawTextRectangle(dc, GetString(grid, row, col), rect, hAlign, vAlign);
}

wxSize wxGridCellNumberRenderer::GetBestSize(wxGrid& grid,
                                             wxGridCellAttr& attr,
                                             wxDC& dc,
                                             int row, int col)
{
    return DoGetBestSize(attr, dc, GetString(grid, row, col));
}


wxGridCellFloatRenderer::wxGridCellFloatRenderer(int width,
                                                 int precision,
                                                 int format)
{
    SetWidth(width);
    SetPrecision(precision);
    SetFormat(format);
}

wxGridCellRenderer *wxGridCellFloatRenderer::Clone() const
{
    wxGridCellFloatRenderer *renderer = new wxGridCellFloatRenderer;
    renderer->m_width = m_width;
    renderer->m_precision = m_precision;
    renderer->m_style = m_style;
    renderer->m_format = m_format;

    return renderer;
}

wxString wxGridCellFloatRenderer::GetString(const wxGrid& grid, int row, int col)
{
    wxGridTableBase *table = grid.GetTable();

    bool hasDouble;
    double val;
    wxString text;
    if ( table->CanGetValueAs(row, col, wxGRID_VALUE_FLOAT) )
    {
        val = table->GetValueAsDouble(row, col);
        hasDouble = true;
    }
    else
    {
        text = table->GetValue(row, col);
        hasDouble = text.ToDouble(&val);
    }

    if ( hasDouble )
    {
        if ( !m_format )
        {
            if ( m_width == -1 )
            {
                if ( m_precision == -1 )
                {
                                        m_format = wxT("%");
                }
                else
                {
                    m_format.Printf(wxT("%%.%d"), m_precision);
                }
            }
            else if ( m_precision == -1 )
            {
                                m_format.Printf(wxT("%%%d."), m_width);
            }
            else
            {
                m_format.Printf(wxT("%%%d.%d"), m_width, m_precision);
            }

            bool isUpper = ( ( m_style & wxGRID_FLOAT_FORMAT_UPPER ) == wxGRID_FLOAT_FORMAT_UPPER);
            if ( ( m_style & wxGRID_FLOAT_FORMAT_SCIENTIFIC ) == wxGRID_FLOAT_FORMAT_SCIENTIFIC)
                m_format += isUpper ? wxT('E') : wxT('e');
            else if ( ( m_style & wxGRID_FLOAT_FORMAT_COMPACT ) == wxGRID_FLOAT_FORMAT_COMPACT)
                m_format += isUpper ? wxT('G') : wxT('g');
            else
                m_format += wxT('f');
        }

        text.Printf(m_format, val);

    }
    
    return text;
}

void wxGridCellFloatRenderer::Draw(wxGrid& grid,
                                   wxGridCellAttr& attr,
                                   wxDC& dc,
                                   const wxRect& rectCell,
                                   int row, int col,
                                   bool isSelected)
{
    wxGridCellRenderer::Draw(grid, attr, dc, rectCell, row, col, isSelected);

    SetTextColoursAndFont(grid, attr, dc, isSelected);

        int hAlign = wxALIGN_RIGHT,
        vAlign = wxALIGN_INVALID;
    attr.GetNonDefaultAlignment(&hAlign, &vAlign);

    wxRect rect = rectCell;
    rect.Inflate(-1);

    grid.DrawTextRectangle(dc, GetString(grid, row, col), rect, hAlign, vAlign);
}

wxSize wxGridCellFloatRenderer::GetBestSize(wxGrid& grid,
                                            wxGridCellAttr& attr,
                                            wxDC& dc,
                                            int row, int col)
{
    return DoGetBestSize(attr, dc, GetString(grid, row, col));
}

void wxGridCellFloatRenderer::SetParameters(const wxString& params)
{
    if ( !params )
    {
                SetWidth(-1);
        SetPrecision(-1);
        SetFormat(wxGRID_FLOAT_FORMAT_DEFAULT);
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
                SetWidth((int)width);
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
                SetPrecision((int)precision);
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
                SetFormat(wxGRID_FLOAT_FORMAT_FIXED);
            }
            else if ( tmp[0] == wxT('e') )
            {
                SetFormat(wxGRID_FLOAT_FORMAT_SCIENTIFIC);
            }
            else if ( tmp[0] == wxT('g') )
            {
                SetFormat(wxGRID_FLOAT_FORMAT_COMPACT);
            }
            else if ( tmp[0] == wxT('E') )
            {
                SetFormat(wxGRID_FLOAT_FORMAT_SCIENTIFIC |
                          wxGRID_FLOAT_FORMAT_UPPER);
            }
            else if ( tmp[0] == wxT('F') )
            {
                SetFormat(wxGRID_FLOAT_FORMAT_FIXED |
                          wxGRID_FLOAT_FORMAT_UPPER);
            }
            else if ( tmp[0] == wxT('G') )
            {
                SetFormat(wxGRID_FLOAT_FORMAT_COMPACT |
                          wxGRID_FLOAT_FORMAT_UPPER);
            }
            else
            {
                wxLogDebug("Invalid wxGridCellFloatRenderer format "
                           "parameter string '%s ignored", params);
            }
        }
    }
}


wxSize wxGridCellBoolRenderer::ms_sizeCheckMark;

wxSize wxGridCellBoolRenderer::GetBestSize(wxGrid& grid,
                                           wxGridCellAttr& WXUNUSED(attr),
                                           wxDC& WXUNUSED(dc),
                                           int WXUNUSED(row),
                                           int WXUNUSED(col))
{
        if ( !ms_sizeCheckMark.x )
    {
        ms_sizeCheckMark = wxRendererNative::Get().GetCheckBoxSize(&grid);
    }

    return ms_sizeCheckMark;
}

void wxGridCellBoolRenderer::Draw(wxGrid& grid,
                                  wxGridCellAttr& attr,
                                  wxDC& dc,
                                  const wxRect& rect,
                                  int row, int col,
                                  bool isSelected)
{
    wxGridCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);

        wxSize size = GetBestSize(grid, attr, dc, row, col);

        wxCoord minSize = wxMin(rect.width, rect.height);
    if ( size.x >= minSize || size.y >= minSize )
    {
                size.x = size.y = minSize;
    }

        int vAlign, hAlign;
    attr.GetAlignment(&hAlign, &vAlign);

    wxRect rectBorder;
    if (hAlign == wxALIGN_CENTRE)
    {
        rectBorder.x = rect.x + rect.width / 2 - size.x / 2;
        rectBorder.y = rect.y + rect.height / 2 - size.y / 2;
        rectBorder.width = size.x;
        rectBorder.height = size.y;
    }
    else if (hAlign == wxALIGN_LEFT)
    {
        rectBorder.x = rect.x + 2;
        rectBorder.y = rect.y + rect.height / 2 - size.y / 2;
        rectBorder.width = size.x;
        rectBorder.height = size.y;
    }
    else if (hAlign == wxALIGN_RIGHT)
    {
        rectBorder.x = rect.x + rect.width - size.x - 2;
        rectBorder.y = rect.y + rect.height / 2 - size.y / 2;
        rectBorder.width = size.x;
        rectBorder.height = size.y;
    }

    bool value;
    if ( grid.GetTable()->CanGetValueAs(row, col, wxGRID_VALUE_BOOL) )
    {
        value = grid.GetTable()->GetValueAsBool(row, col);
    }
    else
    {
        wxString cellval( grid.GetTable()->GetValue(row, col) );
        value = wxGridCellBoolEditor::IsTrueValue(cellval);
    }

    int flags = 0;
    if (value)
        flags |= wxCONTROL_CHECKED;

    wxRendererNative::Get().DrawCheckBox( &grid, dc, rectBorder, flags );
}

#endif 
