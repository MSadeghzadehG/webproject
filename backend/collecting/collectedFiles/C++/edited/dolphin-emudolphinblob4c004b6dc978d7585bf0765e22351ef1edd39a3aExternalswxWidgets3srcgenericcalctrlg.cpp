


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/dcclient.h"
    #include "wx/settings.h"
    #include "wx/brush.h"
    #include "wx/combobox.h"
    #include "wx/listbox.h"
    #include "wx/stattext.h"
    #include "wx/textctrl.h"
#endif 

#if wxUSE_CALENDARCTRL

#include "wx/spinctrl.h"
#include "wx/calctrl.h"
#include "wx/generic/calctrlg.h"

#define DEBUG_PAINT 0


#ifdef wxHAS_NATIVE_CALENDARCTRL

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxGenericCalendarCtrl, wxControl, "wx/calctrl.h");

#endif

wxBEGIN_EVENT_TABLE(wxGenericCalendarCtrl, wxControl)
    EVT_PAINT(wxGenericCalendarCtrl::OnPaint)

    EVT_CHAR(wxGenericCalendarCtrl::OnChar)

    EVT_LEFT_DOWN(wxGenericCalendarCtrl::OnClick)
    EVT_LEFT_DCLICK(wxGenericCalendarCtrl::OnDClick)
    EVT_MOUSEWHEEL(wxGenericCalendarCtrl::OnWheel)

    EVT_SYS_COLOUR_CHANGED(wxGenericCalendarCtrl::OnSysColourChanged)
wxEND_EVENT_TABLE()



namespace
{

void AddAttr(wxCalendarDateAttr *self, const wxCalendarDateAttr& attr)
{
    if (attr.HasTextColour())
        self->SetTextColour(attr.GetTextColour());
    if (attr.HasBackgroundColour())
        self->SetBackgroundColour(attr.GetBackgroundColour());
    if (attr.HasBorderColour())
        self->SetBorderColour(attr.GetBorderColour());
    if (attr.HasFont())
        self->SetFont(attr.GetFont());
    if (attr.HasBorder())
        self->SetBorder(attr.GetBorder());
    if (attr.IsHoliday())
        self->SetHoliday(true);
}

void DelAttr(wxCalendarDateAttr *self, const wxCalendarDateAttr &attr)
{
    if (attr.HasTextColour())
        self->SetTextColour(wxNullColour);
    if (attr.HasBackgroundColour())
        self->SetBackgroundColour(wxNullColour);
    if (attr.HasBorderColour())
        self->SetBorderColour(wxNullColour);
    if (attr.HasFont())
        self->SetFont(wxNullFont);
    if (attr.HasBorder())
        self->SetBorder(wxCAL_BORDER_NONE);
    if (attr.IsHoliday())
        self->SetHoliday(false);
}

} 

wxGenericCalendarCtrl::wxGenericCalendarCtrl(wxWindow *parent,
                                             wxWindowID id,
                                             const wxDateTime& date,
                                             const wxPoint& pos,
                                             const wxSize& size,
                                             long style,
                                             const wxString& name)
{
    Init();

    (void)Create(parent, id, date, pos, size, style, name);
}

void wxGenericCalendarCtrl::Init()
{
    m_comboMonth = NULL;
    m_spinYear = NULL;
    m_staticYear = NULL;
    m_staticMonth = NULL;

    m_userChangedYear = false;

    m_widthCol =
    m_heightRow =
    m_calendarWeekWidth = 0;

    wxDateTime::WeekDay wd;
    for ( wd = wxDateTime::Sun; wd < wxDateTime::Inv_WeekDay; wxNextWDay(wd) )
    {
        m_weekdays[wd] = wxDateTime::GetWeekDayName(wd, wxDateTime::Name_Abbr);
    }

    for ( size_t n = 0; n < WXSIZEOF(m_attrs); n++ )
    {
        m_attrs[n] = NULL;
    }

    InitColours();
}

void wxGenericCalendarCtrl::InitColours()
{
    m_colHighlightFg = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
    m_colHighlightBg = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
    m_colBackground = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
    m_colSurrounding = wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);

    m_colHolidayFg = *wxRED;
    
    m_colHeaderFg = *wxBLUE;
    m_colHeaderBg = *wxLIGHT_GREY;
}

bool wxGenericCalendarCtrl::Create(wxWindow *parent,
                                   wxWindowID id,
                                   const wxDateTime& date,
                                   const wxPoint& pos,
                                   const wxSize& size,
                                   long style,
                                   const wxString& name)
{
    if ( !wxControl::Create(parent, id, pos, size,
                            style | wxCLIP_CHILDREN | wxWANTS_CHARS | wxFULL_REPAINT_ON_RESIZE,
                            wxDefaultValidator, name) )
    {
        return false;
    }

        SetWindowStyle(style | wxWANTS_CHARS);

    m_date = date.IsValid() ? date : wxDateTime::Today();

    m_lowdate = wxDefaultDateTime;
    m_highdate = wxDefaultDateTime;

    if ( !HasFlag(wxCAL_SEQUENTIAL_MONTH_SELECTION) )
    {
        CreateYearSpinCtrl();
        m_staticYear = new wxStaticText(GetParent(), wxID_ANY, m_date.Format(wxT("%Y")),
                                        wxDefaultPosition, wxDefaultSize,
                                        wxALIGN_CENTRE);
        CreateMonthComboBox();
        m_staticMonth = new wxStaticText(GetParent(), wxID_ANY, m_date.Format(wxT("%B")),
                                         wxDefaultPosition, wxDefaultSize,
                                         wxALIGN_CENTRE);
    }

    ShowCurrentControls();

                SetInitialSize(size);
    SetPosition(pos);

            SetBackgroundColour(m_colBackground);

    SetHolidayAttrs();

    return true;
}

wxGenericCalendarCtrl::~wxGenericCalendarCtrl()
{
    for ( size_t n = 0; n < WXSIZEOF(m_attrs); n++ )
    {
        delete m_attrs[n];
    }

    if ( !HasFlag(wxCAL_SEQUENTIAL_MONTH_SELECTION) )
    {
        delete m_comboMonth;
        delete m_staticMonth;
        delete m_spinYear;
        delete m_staticYear;
    }
}

void wxGenericCalendarCtrl::SetWindowStyleFlag(long style)
{
            wxASSERT_MSG( (style & wxCAL_SEQUENTIAL_MONTH_SELECTION) ==
                    (m_windowStyle & wxCAL_SEQUENTIAL_MONTH_SELECTION),
                  wxT("wxCAL_SEQUENTIAL_MONTH_SELECTION can't be changed after creation") );

    wxControl::SetWindowStyleFlag(style);
}


void wxGenericCalendarCtrl::CreateMonthComboBox()
{
    m_comboMonth = new wxComboBox(GetParent(), wxID_ANY,
                                  wxEmptyString,
                                  wxDefaultPosition,
                                  wxDefaultSize,
                                  0, NULL,
                                  wxCB_READONLY | wxCLIP_SIBLINGS);

    wxDateTime::Month m;
    for ( m = wxDateTime::Jan; m < wxDateTime::Inv_Month; wxNextMonth(m) )
    {
        m_comboMonth->Append(wxDateTime::GetMonthName(m));
    }

    m_comboMonth->SetSelection(GetDate().GetMonth());
    m_comboMonth->SetSize(wxDefaultCoord,
                          wxDefaultCoord,
                          wxDefaultCoord,
                          wxDefaultCoord,
                          wxSIZE_AUTO_WIDTH|wxSIZE_AUTO_HEIGHT);

    m_comboMonth->Connect(m_comboMonth->GetId(), wxEVT_COMBOBOX,
                          wxCommandEventHandler(wxGenericCalendarCtrl::OnMonthChange),
                          NULL, this);
}

void wxGenericCalendarCtrl::CreateYearSpinCtrl()
{
    m_spinYear = new wxSpinCtrl(GetParent(), wxID_ANY,
                                GetDate().Format(wxT("%Y")),
                                wxDefaultPosition,
                                wxDefaultSize,
                                wxSP_ARROW_KEYS | wxCLIP_SIBLINGS,
                                -4300, 10000, GetDate().GetYear());

    m_spinYear->Connect(m_spinYear->GetId(), wxEVT_TEXT,
                        wxCommandEventHandler(wxGenericCalendarCtrl::OnYearTextChange),
                        NULL, this);

    m_spinYear->Connect(m_spinYear->GetId(), wxEVT_SPINCTRL,
                        wxSpinEventHandler(wxGenericCalendarCtrl::OnYearChange),
                        NULL, this);
}


bool wxGenericCalendarCtrl::Destroy()
{
    if ( m_staticYear )
        m_staticYear->Destroy();
    if ( m_spinYear )
        m_spinYear->Destroy();
    if ( m_comboMonth )
        m_comboMonth->Destroy();
    if ( m_staticMonth )
        m_staticMonth->Destroy();

    m_staticYear = NULL;
    m_spinYear = NULL;
    m_comboMonth = NULL;
    m_staticMonth = NULL;

    return wxControl::Destroy();
}

bool wxGenericCalendarCtrl::Show(bool show)
{
    if ( !wxControl::Show(show) )
    {
        return false;
    }

    if ( !(GetWindowStyle() & wxCAL_SEQUENTIAL_MONTH_SELECTION) )
    {
        if ( GetMonthControl() )
        {
            GetMonthControl()->Show(show);
            GetYearControl()->Show(show);
        }
    }

    return true;
}

bool wxGenericCalendarCtrl::Enable(bool enable)
{
    if ( !wxControl::Enable(enable) )
    {
        return false;
    }

    if ( !(GetWindowStyle() & wxCAL_SEQUENTIAL_MONTH_SELECTION) )
    {
        GetMonthControl()->Enable(enable);
        GetYearControl()->Enable(enable);
    }

    return true;
}


void wxGenericCalendarCtrl::ShowCurrentControls()
{
    if ( !HasFlag(wxCAL_SEQUENTIAL_MONTH_SELECTION) )
    {
        if ( AllowMonthChange() )
        {
            m_comboMonth->Show();
            m_staticMonth->Hide();

            if ( AllowYearChange() )
            {
                m_spinYear->Show();
                m_staticYear->Hide();

                                return;
            }
        }
        else
        {
            m_comboMonth->Hide();
            m_staticMonth->Show();
        }

                m_spinYear->Hide();
        m_staticYear->Show();
    }
    }

wxControl *wxGenericCalendarCtrl::GetMonthControl() const
{
    return AllowMonthChange() ? (wxControl *)m_comboMonth : (wxControl *)m_staticMonth;
}

wxControl *wxGenericCalendarCtrl::GetYearControl() const
{
    return AllowYearChange() ? (wxControl *)m_spinYear : (wxControl *)m_staticYear;
}

void wxGenericCalendarCtrl::EnableYearChange(bool enable)
{
    if ( enable != AllowYearChange() )
    {
        long style = GetWindowStyle();
        if ( enable )
            style &= ~wxCAL_NO_YEAR_CHANGE;
        else
            style |= wxCAL_NO_YEAR_CHANGE;
        SetWindowStyle(style);

        ShowCurrentControls();
        if ( GetWindowStyle() & wxCAL_SEQUENTIAL_MONTH_SELECTION )
        {
            Refresh();
        }
    }
}

bool wxGenericCalendarCtrl::EnableMonthChange(bool enable)
{
    if ( !wxCalendarCtrlBase::EnableMonthChange(enable) )
        return false;

    ShowCurrentControls();
    if ( GetWindowStyle() & wxCAL_SEQUENTIAL_MONTH_SELECTION )
        Refresh();

    return true;
}


bool wxGenericCalendarCtrl::SetDate(const wxDateTime& date)
{
    bool retval = true;

    bool sameMonth = m_date.GetMonth() == date.GetMonth(),
         sameYear = m_date.GetYear() == date.GetYear();

    if ( IsDateInRange(date) )
    {
        if ( sameMonth && sameYear )
        {
                        ChangeDay(date);
        }
        else
        {
            if ( AllowMonthChange() && (AllowYearChange() || sameYear) )
            {
                                m_date = date;

                if ( !(GetWindowStyle() & wxCAL_SEQUENTIAL_MONTH_SELECTION) )
                {
                                        m_comboMonth->SetSelection(m_date.GetMonth());

                    if ( AllowYearChange() )
                    {
                        if ( !m_userChangedYear )
                            m_spinYear->SetValue(m_date.Format(wxT("%Y")));
                    }
                }

                                SetHolidayAttrs();

                                Refresh();
            }
            else
            {
                                retval = false;
            }
        }
    }

    m_userChangedYear = false;

    return retval;
}

void wxGenericCalendarCtrl::ChangeDay(const wxDateTime& date)
{
    if ( m_date != date )
    {
                        wxDateTime dateOld = m_date;
        m_date = date;

        RefreshDate(dateOld);

                if ( GetWeek(m_date) != GetWeek(dateOld) )
        {
            RefreshDate(m_date);
        }
    }
}

void wxGenericCalendarCtrl::SetDateAndNotify(const wxDateTime& date)
{
    const wxDateTime dateOld = GetDate();
    if ( date != dateOld && SetDate(date) )
    {
        GenerateAllChangeEvents(dateOld);
    }
}


bool wxGenericCalendarCtrl::SetLowerDateLimit(const wxDateTime& date )
{
    bool retval = true;

    if ( !(date.IsValid()) || ( ( m_highdate.IsValid() ) ? ( date <= m_highdate ) : true ) )
    {
        m_lowdate = date;
    }
    else
    {
        retval = false;
    }

    return retval;
}

bool wxGenericCalendarCtrl::SetUpperDateLimit(const wxDateTime& date )
{
    bool retval = true;

    if ( !(date.IsValid()) || ( ( m_lowdate.IsValid() ) ? ( date >= m_lowdate ) : true ) )
    {
        m_highdate = date;
    }
    else
    {
        retval = false;
    }

    return retval;
}

bool wxGenericCalendarCtrl::SetDateRange(const wxDateTime& lowerdate , const wxDateTime& upperdate )
{
    bool retval = true;

    if (
        ( !( lowerdate.IsValid() ) || ( ( upperdate.IsValid() ) ? ( lowerdate <= upperdate ) : true ) ) &&
        ( !( upperdate.IsValid() ) || ( ( lowerdate.IsValid() ) ? ( upperdate >= lowerdate ) : true ) ) )
    {
        m_lowdate = lowerdate;
        m_highdate = upperdate;
    }
    else
    {
        retval = false;
    }

    return retval;
}

bool wxGenericCalendarCtrl::GetDateRange(wxDateTime *lowerdate,
                                         wxDateTime *upperdate) const
{
    if ( lowerdate )
        *lowerdate = m_lowdate;
    if ( upperdate )
        *upperdate = m_highdate;

    return m_lowdate.IsValid() || m_highdate.IsValid();
}


wxDateTime wxGenericCalendarCtrl::GetStartDate() const
{
    wxDateTime::Tm tm = m_date.GetTm();

    wxDateTime date = wxDateTime(1, tm.mon, tm.year);

        date.SetToPrevWeekDay(GetWeekStart());

    if ( GetWindowStyle() & wxCAL_SHOW_SURROUNDING_WEEKS )
    {
                if ( date.GetDay() == 1 )
        {
            date -= wxDateSpan::Week();
        }
    }

    return date;
}

bool wxGenericCalendarCtrl::IsDateShown(const wxDateTime& date) const
{
    if ( !(GetWindowStyle() & wxCAL_SHOW_SURROUNDING_WEEKS) )
    {
        return date.GetMonth() == m_date.GetMonth();
    }
    else
    {
        return true;
    }
}

bool wxGenericCalendarCtrl::IsDateInRange(const wxDateTime& date) const
{
        return ( ( ( m_lowdate.IsValid() ) ? ( date >= m_lowdate ) : true )
        && ( ( m_highdate.IsValid() ) ? ( date <= m_highdate ) : true ) );
}

bool wxGenericCalendarCtrl::AdjustDateToRange(wxDateTime *date) const
{
    if ( m_lowdate.IsValid() && *date < m_lowdate )
    {
        *date = m_lowdate;
        return true;
    }

    if ( m_highdate.IsValid() && *date > m_highdate )
    {
        *date = m_highdate;
        return true;
    }

    return false;
}

size_t wxGenericCalendarCtrl::GetWeek(const wxDateTime& date) const
{
    size_t retval = date.GetWeekOfMonth(HasFlag(wxCAL_MONDAY_FIRST)
                                   ? wxDateTime::Monday_First
                                   : wxDateTime::Sunday_First);

    if ( (GetWindowStyle() & wxCAL_SHOW_SURROUNDING_WEEKS) )
    {
                wxDateTime::Tm tm = date.GetTm();

        wxDateTime datetest = wxDateTime(1, tm.mon, tm.year);

                datetest.SetToPrevWeekDay(GetWeekStart());

        if ( datetest.GetDay() == 1 )
        {
            retval += 1;
        }
    }

    return retval;
}




#define VERT_MARGIN    5           #define HORZ_MARGIN    5           
wxSize wxGenericCalendarCtrl::DoGetBestSize() const
{
        const_cast<wxGenericCalendarCtrl *>(this)->RecalcGeometry();

    wxCoord width = 7*m_widthCol + m_calendarWeekWidth,
            height = 7*m_heightRow + m_rowOffset + VERT_MARGIN;

    if ( !HasFlag(wxCAL_SEQUENTIAL_MONTH_SELECTION) )
    {
        const wxSize bestSizeCombo = m_comboMonth->GetBestSize();

        height += wxMax(bestSizeCombo.y, m_spinYear->GetBestSize().y)
                    + VERT_MARGIN;

        wxCoord w2 = bestSizeCombo.x + HORZ_MARGIN + GetCharWidth()*8;
        if ( width < w2 )
            width = w2;
    }

    wxSize best(width, height);
    if ( !HasFlag(wxBORDER_NONE) )
    {
        best += GetWindowBorderSize();
    }

    CacheBestSize(best);

    return best;
}

void wxGenericCalendarCtrl::DoMoveWindow(int x, int y, int width, int height)
{
    int yDiff;

    if ( !HasFlag(wxCAL_SEQUENTIAL_MONTH_SELECTION) && m_staticMonth )
    {
        wxSize sizeCombo = m_comboMonth->GetEffectiveMinSize();
        wxSize sizeStatic = m_staticMonth->GetSize();
        wxSize sizeSpin = m_spinYear->GetSize();

        int maxHeight = wxMax(sizeSpin.y, sizeCombo.y);
        int dy = (maxHeight - sizeStatic.y) / 2;
        m_comboMonth->Move(x, y + (maxHeight - sizeCombo.y)/2);
        m_staticMonth->SetSize(x, y + dy, sizeCombo.x, -1);

        int xDiff = sizeCombo.x + HORZ_MARGIN;

        m_spinYear->SetSize(x + xDiff, y + (maxHeight - sizeSpin.y)/2, width - xDiff, maxHeight);
        m_staticYear->SetSize(x + xDiff, y + dy, width - xDiff, sizeStatic.y);

        yDiff = maxHeight + VERT_MARGIN;
    }
    else     {
        yDiff = 0;
    }

    wxControl::DoMoveWindow(x, y + yDiff, width, height - yDiff);
}

void wxGenericCalendarCtrl::DoGetSize(int *width, int *height) const
{
    wxControl::DoGetSize( width, height );
}

void wxGenericCalendarCtrl::RecalcGeometry()
{
    wxClientDC dc(this);

    dc.SetFont(GetFont());

            m_widthCol = 0;
    for ( int day = 10; day <= 31; day++)
    {
        wxCoord width;
        dc.GetTextExtent(wxString::Format(wxT("%d"), day), &width, &m_heightRow);
        if ( width > m_widthCol )
        {
                                    m_widthCol = width+width/2;
        }
    }
    wxDateTime::WeekDay wd;
    for ( wd = wxDateTime::Sun; wd < wxDateTime::Inv_WeekDay; wxNextWDay(wd) )
    {
        wxCoord width;
        dc.GetTextExtent(m_weekdays[wd], &width, &m_heightRow);
        if ( width > m_widthCol )
        {
            m_widthCol = width;
        }
    }

    m_calendarWeekWidth = HasFlag( wxCAL_SHOW_WEEK_NUMBERS )
        ? dc.GetTextExtent( wxString::Format( wxT( "%d" ), 42 )).GetWidth() + 4 : 0;

        m_widthCol += 2;
    m_heightRow += 2;

    m_rowOffset = HasFlag(wxCAL_SEQUENTIAL_MONTH_SELECTION) ? m_heightRow : 0; }


void wxGenericCalendarCtrl::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);

    dc.SetFont(GetFont());

    RecalcGeometry();

#if DEBUG_PAINT
    wxLogDebug("--- starting to paint, selection: %s, week %u\n",
           m_date.Format("%a %d-%m-%Y %H:%M:%S").c_str(),
           GetWeek(m_date));
#endif

    wxCoord y = 0;
    wxCoord x0 = m_calendarWeekWidth;

    if ( HasFlag(wxCAL_SEQUENTIAL_MONTH_SELECTION) )
    {
        
        dc.SetBackgroundMode(wxBRUSHSTYLE_TRANSPARENT);
        dc.SetTextForeground(*wxBLACK);
        dc.SetBrush(wxBrush(m_colHeaderBg, wxBRUSHSTYLE_SOLID));
        dc.SetPen(wxPen(m_colHeaderBg, 1, wxPENSTYLE_SOLID));
        dc.DrawRectangle(0, y, GetClientSize().x, m_heightRow);

                wxCoord monthw, monthh;
        wxString headertext = m_date.Format(wxT("%B %Y"));
        dc.GetTextExtent(headertext, &monthw, &monthh);

                wxCoord monthx = ((m_widthCol * 7) - monthw) / 2 + x0;
        wxCoord monthy = ((m_heightRow - monthh) / 2) + y;
        dc.DrawText(headertext, monthx,  monthy);

                wxPoint leftarrow[3];
        wxPoint rightarrow[3];

        int arrowheight = monthh / 2;

        leftarrow[0] = wxPoint(0, arrowheight / 2);
        leftarrow[1] = wxPoint(arrowheight / 2, 0);
        leftarrow[2] = wxPoint(arrowheight / 2, arrowheight - 1);

        rightarrow[0] = wxPoint(0,0);
        rightarrow[1] = wxPoint(arrowheight / 2, arrowheight / 2);
        rightarrow[2] = wxPoint(0, arrowheight - 1);

        
        wxCoord arrowy = (m_heightRow - arrowheight) / 2;
        wxCoord larrowx = (m_widthCol - (arrowheight / 2)) / 2 + x0;
        wxCoord rarrowx = ((m_widthCol - (arrowheight / 2)) / 2) + m_widthCol*6 + x0;
        m_leftArrowRect = m_rightArrowRect = wxRect(0,0,0,0);

        if ( AllowMonthChange() )
        {
            wxDateTime ldpm = wxDateTime(1,m_date.GetMonth(), m_date.GetYear()) - wxDateSpan::Day();                         if ( IsDateInRange(ldpm) && ( ( ldpm.GetYear() == m_date.GetYear() ) ? true : AllowYearChange() ) )
            {
                m_leftArrowRect = wxRect(larrowx - 3, arrowy - 3, (arrowheight / 2) + 8, (arrowheight + 6));
                dc.SetBrush(*wxBLACK_BRUSH);
                dc.SetPen(*wxBLACK_PEN);
                dc.DrawPolygon(3, leftarrow, larrowx , arrowy, wxWINDING_RULE);
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                dc.DrawRectangle(m_leftArrowRect);
            }
            wxDateTime fdnm = wxDateTime(1,m_date.GetMonth(), m_date.GetYear()) + wxDateSpan::Month();             if ( IsDateInRange(fdnm) && ( ( fdnm.GetYear() == m_date.GetYear() ) ? true : AllowYearChange() ) )
            {
                m_rightArrowRect = wxRect(rarrowx - 4, arrowy - 3, (arrowheight / 2) + 8, (arrowheight + 6));
                dc.SetBrush(*wxBLACK_BRUSH);
                dc.SetPen(*wxBLACK_PEN);
                dc.DrawPolygon(3, rightarrow, rarrowx , arrowy, wxWINDING_RULE);
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                dc.DrawRectangle(m_rightArrowRect);
            }
        }

        y += m_heightRow;
    }

        if ( IsExposed(x0, y, x0 + 7*m_widthCol, m_heightRow) )
    {
#if DEBUG_PAINT
        wxLogDebug("painting the header");
#endif

        dc.SetBackgroundMode(wxBRUSHSTYLE_TRANSPARENT);
        dc.SetTextForeground(m_colHeaderFg);
        dc.SetBrush(wxBrush(m_colHeaderBg, wxBRUSHSTYLE_SOLID));
        dc.SetPen(wxPen(m_colHeaderBg, 1, wxPENSTYLE_SOLID));
        dc.DrawRectangle(0, y, GetClientSize().x, m_heightRow);

        bool startOnMonday = HasFlag(wxCAL_MONDAY_FIRST);
        for ( int wd = 0; wd < 7; wd++ )
        {
            size_t n;
            if ( startOnMonday )
                n = wd == 6 ? 0 : wd + 1;
            else
                n = wd;
            wxCoord dayw, dayh;
            dc.GetTextExtent(m_weekdays[n], &dayw, &dayh);
            dc.DrawText(m_weekdays[n], x0 + (wd*m_widthCol) + ((m_widthCol- dayw) / 2), y);         }
    }

        dc.SetTextForeground(*wxBLACK);
    
    y += m_heightRow;

        if ( HasFlag( wxCAL_SHOW_WEEK_NUMBERS ) && IsExposed( 0, y, m_calendarWeekWidth, m_heightRow * 6 ))
    {
        dc.SetBackgroundMode(wxTRANSPARENT);
        dc.SetBrush(wxBrush(m_colHeaderBg, wxBRUSHSTYLE_SOLID));
        dc.SetPen(wxPen(m_colHeaderBg, 1, wxPENSTYLE_SOLID));
        dc.DrawRectangle( 0, y, m_calendarWeekWidth, m_heightRow * 6 );
        wxDateTime date = GetStartDate();
        for ( size_t i = 0; i < 6; ++i )
        {
            const int weekNr = date.GetWeekOfYear();
            wxString text = wxString::Format( wxT( "%d" ), weekNr );
            dc.DrawText( text, m_calendarWeekWidth - dc.GetTextExtent( text ).GetWidth() - 2, y + m_heightRow * i );
            date += wxDateSpan::Week();
        }
    }

    wxDateTime date = GetStartDate();

#if DEBUG_PAINT
    wxLogDebug("starting calendar from %s\n",
            date.Format("%a %d-%m-%Y %H:%M:%S").c_str());
#endif

    dc.SetBackgroundMode(wxBRUSHSTYLE_SOLID);
    for ( size_t nWeek = 1; nWeek <= 6; nWeek++, y += m_heightRow )
    {
                if ( !IsExposed(x0, y, x0 + 7*m_widthCol, m_heightRow - 1) )
        {
            date += wxDateSpan::Week();

            continue;
        }

#if DEBUG_PAINT
        wxLogDebug("painting week %d at y = %d\n", nWeek, y);
#endif

        for ( int wd = 0; wd < 7; wd++ )
        {
            dc.SetTextBackground(m_colBackground);
            if ( IsDateShown(date) )
            {
                                unsigned int day = date.GetDay();
                wxString dayStr = wxString::Format(wxT("%u"), day);
                wxCoord width;
                dc.GetTextExtent(dayStr, &width, NULL);

                bool changedColours = false,
                     changedFont = false;

                bool isSel = false;
                wxCalendarDateAttr *attr = NULL;

                if ( date.GetMonth() != m_date.GetMonth() || !IsDateInRange(date) )
                {
                                        dc.SetTextForeground(m_colSurrounding);
                    changedColours = true;
                }
                else
                {
                    isSel = date.IsSameDate(m_date);
                    attr = m_attrs[day - 1];

                    if ( isSel )
                    {
                        dc.SetTextForeground(m_colHighlightFg);
                        dc.SetTextBackground(m_colHighlightBg);

                        changedColours = true;
                    }
                    else if ( attr )
                    {
                        wxColour colFg, colBg;

                        if ( attr->IsHoliday() )
                        {
                            colFg = m_colHolidayFg;
                            colBg = m_colHolidayBg;
                        }
                        else
                        {
                            colFg = attr->GetTextColour();
                            colBg = attr->GetBackgroundColour();
                        }

                        if ( colFg.IsOk() )
                        {
                            dc.SetTextForeground(colFg);
                            changedColours = true;
                        }

                        if ( colBg.IsOk() )
                        {
                            dc.SetTextBackground(colBg);
                            changedColours = true;
                        }

                        if ( attr->HasFont() )
                        {
                            dc.SetFont(attr->GetFont());
                            changedFont = true;
                        }
                    }
                }

                wxCoord x = wd*m_widthCol + (m_widthCol - width) / 2 + x0;
                dc.DrawText(dayStr, x, y + 1);

                if ( !isSel && attr && attr->HasBorder() )
                {
                    wxColour colBorder;
                    if ( attr->HasBorderColour() )
                    {
                        colBorder = attr->GetBorderColour();
                    }
                    else
                    {
                        colBorder = GetForegroundColour();
                    }

                    wxPen pen(colBorder, 1, wxPENSTYLE_SOLID);
                    dc.SetPen(pen);
                    dc.SetBrush(*wxTRANSPARENT_BRUSH);

                    switch ( attr->GetBorder() )
                    {
                        case wxCAL_BORDER_SQUARE:
                            dc.DrawRectangle(x - 2, y,
                                             width + 4, m_heightRow);
                            break;

                        case wxCAL_BORDER_ROUND:
                            dc.DrawEllipse(x - 2, y,
                                           width + 4, m_heightRow);
                            break;

                        default:
                            wxFAIL_MSG(wxT("unknown border type"));
                    }
                }

                if ( changedColours )
                {
                    dc.SetTextForeground(GetForegroundColour());
                    dc.SetTextBackground(GetBackgroundColour());
                }

                if ( changedFont )
                {
                    dc.SetFont(GetFont());
                }
            }
            
            date += wxDateSpan::Day();
        }
    }

        bool showSurrounding = (GetWindowStyle() & wxCAL_SHOW_SURROUNDING_WEEKS) != 0;

    date = ( showSurrounding ) ? GetStartDate() : wxDateTime(1, m_date.GetMonth(), m_date.GetYear());
    if ( !IsDateInRange(date) )
    {
        wxDateTime firstOOR = GetLowerDateLimit() - wxDateSpan::Day(); 
        wxBrush oorbrush = *wxLIGHT_GREY_BRUSH;
        oorbrush.SetStyle(wxBRUSHSTYLE_FDIAGONAL_HATCH);

        HighlightRange(&dc, date, firstOOR, wxTRANSPARENT_PEN, &oorbrush);
    }

    date = ( showSurrounding ) ? GetStartDate() + wxDateSpan::Weeks(6) - wxDateSpan::Day() : wxDateTime().SetToLastMonthDay(m_date.GetMonth(), m_date.GetYear());
    if ( !IsDateInRange(date) )
    {
        wxDateTime firstOOR = GetUpperDateLimit() + wxDateSpan::Day(); 
        wxBrush oorbrush = *wxLIGHT_GREY_BRUSH;
        oorbrush.SetStyle(wxBRUSHSTYLE_FDIAGONAL_HATCH);

        HighlightRange(&dc, firstOOR, date, wxTRANSPARENT_PEN, &oorbrush);
    }

#if DEBUG_PAINT
    wxLogDebug("+++ finished painting");
#endif
}

void wxGenericCalendarCtrl::RefreshDate(const wxDateTime& date)
{
    RecalcGeometry();

    wxRect rect;

                rect.x = m_calendarWeekWidth;

    rect.y = (m_heightRow * GetWeek(date)) + m_rowOffset;

    rect.width = 7*m_widthCol;
    rect.height = m_heightRow;

#ifdef __WXMSW__
                        rect.Inflate(0, 1);
#endif 
#if DEBUG_PAINT
    wxLogDebug("*** refreshing week %d at (%d, %d)-(%d, %d)\n",
           GetWeek(date),
           rect.x, rect.y,
           rect.x + rect.width, rect.y + rect.height);
#endif

    Refresh(true, &rect);
}

void wxGenericCalendarCtrl::HighlightRange(wxPaintDC* pDC, const wxDateTime& fromdate, const wxDateTime& todate, const wxPen* pPen, const wxBrush* pBrush)
{
        

#if DEBUG_PAINT
    wxLogDebug("+++ HighlightRange: (%s) - (%s) +++", fromdate.Format("%d %m %Y"), todate.Format("%d %m %Y"));
#endif

    if ( todate >= fromdate )
    {
                        int fd, fw;
        int td, tw;

                if ( GetDateCoord(fromdate, &fd, &fw) && GetDateCoord(todate, &td, &tw) )
        {
#if DEBUG_PAINT
            wxLogDebug("Highlight range: (%i, %i) - (%i, %i)", fd, fw, td, tw);
#endif
            if ( ( (tw - fw) == 1 ) && ( td < fd ) )
            {
                                                wxDateTime tfd = fromdate + wxDateSpan::Days(7-fd);
                wxDateTime ftd = tfd + wxDateSpan::Day();
#if DEBUG_PAINT
                wxLogDebug("Highlight: Separate segments");
#endif
                                HighlightRange(pDC, fromdate, tfd, pPen, pBrush);
                HighlightRange(pDC, ftd, todate, pPen, pBrush);
            }
            else
            {
                int numpoints;
                wxPoint corners[8];                 wxCoord x0 = m_calendarWeekWidth;

                if ( fw == tw )
                {
                                        numpoints = 4;
                    corners[0] = wxPoint(x0 + (fd - 1) * m_widthCol, (fw * m_heightRow) + m_rowOffset);
                    corners[1] = wxPoint(x0 + (fd - 1) * m_widthCol, ((fw + 1 ) * m_heightRow) + m_rowOffset);
                    corners[2] = wxPoint(x0 + td * m_widthCol, ((tw + 1) * m_heightRow) + m_rowOffset);
                    corners[3] = wxPoint(x0 + td * m_widthCol, (tw * m_heightRow) + m_rowOffset);
                }
                else
                {
                    int cidx = 0;
                                        corners[cidx] = wxPoint(x0 + (fd - 1) * m_widthCol, (fw * m_heightRow) + m_rowOffset); cidx++;

                    if ( fd > 1 )
                    {
                        corners[cidx] = wxPoint(x0 + (fd - 1) * m_widthCol, ((fw + 1) * m_heightRow) + m_rowOffset); cidx++;
                        corners[cidx] = wxPoint(x0, ((fw + 1) * m_heightRow) + m_rowOffset); cidx++;
                    }

                    corners[cidx] = wxPoint(x0, ((tw + 1) * m_heightRow) + m_rowOffset); cidx++;
                    corners[cidx] = wxPoint(x0 + td * m_widthCol, ((tw + 1) * m_heightRow) + m_rowOffset); cidx++;

                    if ( td < 7 )
                    {
                        corners[cidx] = wxPoint(x0 + td * m_widthCol, (tw * m_heightRow) + m_rowOffset); cidx++;
                        corners[cidx] = wxPoint(x0 + 7 * m_widthCol, (tw * m_heightRow) + m_rowOffset); cidx++;
                    }

                    corners[cidx] = wxPoint(x0 + 7 * m_widthCol, (fw * m_heightRow) + m_rowOffset); cidx++;

                    numpoints = cidx;
                }

                                pDC->SetBrush(*pBrush);
                pDC->SetPen(*pPen);
                pDC->DrawPolygon(numpoints, corners);
            }
        }
    }
    #if DEBUG_PAINT
    wxLogDebug("--- HighlightRange ---");
#endif
}

bool wxGenericCalendarCtrl::GetDateCoord(const wxDateTime& date, int *day, int *week) const
{
    bool retval = true;

#if DEBUG_PAINT
    wxLogDebug("+++ GetDateCoord: (%s) +++", date.Format("%d %m %Y"));
#endif

    if ( IsDateShown(date) )
    {
        bool startOnMonday = HasFlag(wxCAL_MONDAY_FIRST);

                *day = date.GetWeekDay();

        if ( *day == 0 )         {
            *day = ( startOnMonday ) ? 7 : 1;
        }
        else
        {
            *day += ( startOnMonday ) ? 0 : 1;
        }

        int targetmonth = date.GetMonth() + (12 * date.GetYear());
        int thismonth = m_date.GetMonth() + (12 * m_date.GetYear());

                if ( targetmonth == thismonth )
        {
            *week = GetWeek(date);
        }
        else
        {
            if ( targetmonth < thismonth )
            {
                *week = 1;             }
            else             {
                wxDateTime ldcm;
                int lastweek;
                int lastday;

                #if DEBUG_PAINT
                wxLogDebug("     +++ LDOM +++");
#endif
                GetDateCoord(ldcm.SetToLastMonthDay(m_date.GetMonth(), m_date.GetYear()), &lastday, &lastweek);
#if DEBUG_PAINT
                wxLogDebug("     --- LDOM ---");
#endif

                wxTimeSpan span = date - ldcm;

                int daysfromlast = span.GetDays();
#if DEBUG_PAINT
                wxLogDebug("daysfromlast: %i", daysfromlast);
#endif
                if ( daysfromlast + lastday > 7 )                 {
                    int wholeweeks = (daysfromlast / 7);
                    *week = wholeweeks + lastweek;
                    if ( (daysfromlast - (7 * wholeweeks) + lastday) > 7 )
                    {
                        *week += 1;
                    }
                }
                else
                {
                    *week = lastweek;
                }
            }
        }
    }
    else
    {
        *day = -1;
        *week = -1;
        retval = false;
    }

#if DEBUG_PAINT
    wxLogDebug("--- GetDateCoord: (%s) = (%i, %i) ---", date.Format("%d %m %Y"), *day, *week);
#endif

    return retval;
}


void wxGenericCalendarCtrl::OnDClick(wxMouseEvent& event)
{
    wxDateTime date;
    switch ( HitTest(event.GetPosition(), &date) )
    {
        case wxCAL_HITTEST_DAY:
            GenerateEvent(wxEVT_CALENDAR_DOUBLECLICKED);
            break;

        case wxCAL_HITTEST_DECMONTH:
        case wxCAL_HITTEST_INCMONTH:
                                    SetDateAndNotify(date);
            break;

        case wxCAL_HITTEST_WEEK:
        case wxCAL_HITTEST_HEADER:
        case wxCAL_HITTEST_SURROUNDING_WEEK:
        case wxCAL_HITTEST_NOWHERE:
            event.Skip();
            break;
    }
}

void wxGenericCalendarCtrl::OnClick(wxMouseEvent& event)
{
    wxDateTime date;
    wxDateTime::WeekDay wday;
    switch ( HitTest(event.GetPosition(), &date, &wday) )
    {
        case wxCAL_HITTEST_DAY:
            if ( IsDateInRange(date) )
            {
                ChangeDay(date);

                GenerateEvent(wxEVT_CALENDAR_SEL_CHANGED);

                                                                GenerateEvent(wxEVT_CALENDAR_DAY_CHANGED);
            }
            break;

        case wxCAL_HITTEST_WEEK:
            {
                wxCalendarEvent send( this, date, wxEVT_CALENDAR_WEEK_CLICKED );
                HandleWindowEvent( send );
            }
            break;

        case wxCAL_HITTEST_HEADER:
            {
                wxCalendarEvent eventWd(this, GetDate(),
                                        wxEVT_CALENDAR_WEEKDAY_CLICKED);
                eventWd.SetWeekDay(wday);
                (void)GetEventHandler()->ProcessEvent(eventWd);
            }
            break;

        case wxCAL_HITTEST_DECMONTH:
        case wxCAL_HITTEST_INCMONTH:
        case wxCAL_HITTEST_SURROUNDING_WEEK:
            SetDateAndNotify(date);             break;

        default:
            wxFAIL_MSG(wxT("unknown hittest code"));
            wxFALLTHROUGH;

        case wxCAL_HITTEST_NOWHERE:
            event.Skip();
            break;
    }

            SetFocus();
}

wxCalendarHitTestResult wxGenericCalendarCtrl::HitTest(const wxPoint& pos,
                                                wxDateTime *date,
                                                wxDateTime::WeekDay *wd)
{
    RecalcGeometry();

        wxCoord x0 = m_calendarWeekWidth;

    if ( HasFlag(wxCAL_SEQUENTIAL_MONTH_SELECTION) )
    {
        
                        if ( m_leftArrowRect.Contains(pos) )
        {
            if ( date )
            {
                if ( IsDateInRange(m_date - wxDateSpan::Month()) )
                {
                    *date = m_date - wxDateSpan::Month();
                }
                else
                {
                    *date = GetLowerDateLimit();
                }
            }

            return wxCAL_HITTEST_DECMONTH;
        }

        if ( m_rightArrowRect.Contains(pos) )
        {
            if ( date )
            {
                if ( IsDateInRange(m_date + wxDateSpan::Month()) )
                {
                    *date = m_date + wxDateSpan::Month();
                }
                else
                {
                    *date = GetUpperDateLimit();
                }
            }

            return wxCAL_HITTEST_INCMONTH;
        }
    }

    if ( pos.x - x0 < 0 )
    {
        if ( pos.x >= 0 && pos.y > m_rowOffset + m_heightRow && pos.y <= m_rowOffset + m_heightRow * 7 )
        {
            if ( date )
            {
                *date = GetStartDate();
                *date += wxDateSpan::Week() * (( pos.y - m_rowOffset ) / m_heightRow - 1 );
            }
            if ( wd )
                *wd = GetWeekStart();
            return wxCAL_HITTEST_WEEK;
        }
        else                return wxCAL_HITTEST_NOWHERE;
    }

        int wday = (pos.x - x0) / m_widthCol;
    if ( wday > 6 )
        return wxCAL_HITTEST_NOWHERE;
    if ( pos.y < (m_heightRow + m_rowOffset))
    {
        if ( pos.y > m_rowOffset )
        {
            if ( wd )
            {
                if ( HasFlag(wxCAL_MONDAY_FIRST) )
                {
                    wday = wday == 6 ? 0 : wday + 1;
                }

                *wd = (wxDateTime::WeekDay)wday;
            }

            return wxCAL_HITTEST_HEADER;
        }
        else
        {
            return wxCAL_HITTEST_NOWHERE;
        }
    }

    int week = (pos.y - (m_heightRow + m_rowOffset)) / m_heightRow;
    if ( week >= 6 || wday >= 7 )
    {
        return wxCAL_HITTEST_NOWHERE;
    }

    wxDateTime dt = GetStartDate() + wxDateSpan::Days(7*week + wday);

    if ( IsDateShown(dt) )
    {
        if ( date )
            *date = dt;

        if ( dt.GetMonth() == m_date.GetMonth() )
        {

            return wxCAL_HITTEST_DAY;
        }
        else
        {
            return wxCAL_HITTEST_SURROUNDING_WEEK;
        }
    }
    else
    {
        return wxCAL_HITTEST_NOWHERE;
    }
}

void wxGenericCalendarCtrl::OnWheel(wxMouseEvent& event)
{
    wxDateSpan span;
    switch ( event.GetWheelAxis() )
    {
        case wxMOUSE_WHEEL_VERTICAL:
                                                span = -wxDateSpan::Month();
            break;

        case wxMOUSE_WHEEL_HORIZONTAL:
            span = wxDateSpan::Year();
            break;
    }

            if ( event.GetWheelRotation() < 0 )
        span = -span;

    SetDateAndNotify(m_date + span);
}


void wxGenericCalendarCtrl::OnMonthChange(wxCommandEvent& event)
{
    wxDateTime::Tm tm = m_date.GetTm();

    wxDateTime::Month mon = (wxDateTime::Month)event.GetInt();
    if ( tm.mday > wxDateTime::GetNumberOfDays(mon, tm.year) )
    {
        tm.mday = wxDateTime::GetNumberOfDays(mon, tm.year);
    }

    wxDateTime dt(tm.mday, mon, tm.year);
    if ( AdjustDateToRange(&dt) )
    {
                        m_comboMonth->SetSelection(dt.GetMonth());
    }

    SetDateAndNotify(dt);
}

void wxGenericCalendarCtrl::HandleYearChange(wxCommandEvent& event)
{
    int year = (int)event.GetInt();
    if ( year == INT_MIN )
    {
                return;
    }

    wxDateTime::Tm tm = m_date.GetTm();

    if ( tm.mday > wxDateTime::GetNumberOfDays(tm.mon, year) )
    {
        tm.mday = wxDateTime::GetNumberOfDays(tm.mon, year);
    }

    wxDateTime dt(tm.mday, tm.mon, year);
    if ( AdjustDateToRange(&dt) )
    {
                        m_spinYear->SetValue(dt.GetYear());
    }

    SetDateAndNotify(dt);
}

void wxGenericCalendarCtrl::OnYearChange(wxSpinEvent& event)
{
    HandleYearChange( event );
}

void wxGenericCalendarCtrl::OnYearTextChange(wxCommandEvent& event)
{
    SetUserChangedYear();
    HandleYearChange(event);
}

void wxGenericCalendarCtrl::OnSysColourChanged(wxSysColourChangedEvent& event)
{
        InitColours();

        wxControl::OnSysColourChanged(event);

        SetBackgroundColour(m_colBackground);
    Refresh();
}


void wxGenericCalendarCtrl::OnChar(wxKeyEvent& event)
{
    switch ( event.GetKeyCode() )
    {
        case wxT('+'):
        case WXK_ADD:
            SetDateAndNotify(m_date + wxDateSpan::Year());
            break;

        case wxT('-'):
        case WXK_SUBTRACT:
            SetDateAndNotify(m_date - wxDateSpan::Year());
            break;

        case WXK_PAGEUP:
            SetDateAndNotify(m_date - wxDateSpan::Month());
            break;

        case WXK_PAGEDOWN:
            SetDateAndNotify(m_date + wxDateSpan::Month());
            break;

        case WXK_RIGHT:
            if ( event.ControlDown() )
            {
                wxDateTime target = m_date.SetToNextWeekDay(GetWeekEnd());
                AdjustDateToRange(&target);
                SetDateAndNotify(target);
            }
            else
                SetDateAndNotify(m_date + wxDateSpan::Day());
            break;

        case WXK_LEFT:
            if ( event.ControlDown() )
            {
                wxDateTime target = m_date.SetToPrevWeekDay(GetWeekStart());
                AdjustDateToRange(&target);
                SetDateAndNotify(target);
            }
            else
                SetDateAndNotify(m_date - wxDateSpan::Day());
            break;

        case WXK_UP:
            SetDateAndNotify(m_date - wxDateSpan::Week());
            break;

        case WXK_DOWN:
            SetDateAndNotify(m_date + wxDateSpan::Week());
            break;

        case WXK_HOME:
            if ( event.ControlDown() )
                SetDateAndNotify(wxDateTime::Today());
            else
                SetDateAndNotify(wxDateTime(1, m_date.GetMonth(), m_date.GetYear()));
            break;

        case WXK_END:
            SetDateAndNotify(wxDateTime(m_date).SetToLastMonthDay());
            break;

        case WXK_RETURN:
            GenerateEvent(wxEVT_CALENDAR_DOUBLECLICKED);
            break;

        default:
            event.Skip();
    }
}


void wxGenericCalendarCtrl::SetHoliday(size_t day)
{
    wxCHECK_RET( day > 0 && day < 32, wxT("invalid day in SetHoliday") );

    wxCalendarDateAttr *attr = GetAttr(day);
    if ( !attr )
    {
        attr = new wxCalendarDateAttr;
    }

    attr->SetHoliday(true);

        m_attrs[day - 1] = attr;
}

void wxGenericCalendarCtrl::ResetHolidayAttrs()
{
    for ( size_t day = 0; day < 31; day++ )
    {
        if ( m_attrs[day] )
        {
            m_attrs[day]->SetHoliday(false);
        }
    }
}

void wxGenericCalendarCtrl::Mark(size_t day, bool mark)
{
    wxCHECK_RET( day > 0 && day < 32, wxT("invalid day in Mark") );

    const wxCalendarDateAttr& m = wxCalendarDateAttr::GetMark();
    if (mark) {
        if ( m_attrs[day - 1] )
            AddAttr(m_attrs[day - 1], m);
        else
            SetAttr(day, new wxCalendarDateAttr(m));
    }
    else {
        if ( m_attrs[day - 1] )
            DelAttr(m_attrs[day - 1], m);
    }
}

wxVisualAttributes
wxGenericCalendarCtrl::GetClassDefaultAttributes(wxWindowVariant variant)
{
        return wxListBox::GetClassDefaultAttributes(variant);
}

#endif 