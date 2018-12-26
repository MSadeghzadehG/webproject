


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_TIMEPICKCTRL

#ifndef WX_PRECOMP
    #include "wx/textctrl.h"
    #include "wx/utils.h"           #endif 
#include "wx/timectrl.h"

#if !defined(wxHAS_NATIVE_TIMEPICKERCTRL) || wxUSE_TIMEPICKCTRL_GENERIC

#include "wx/generic/timectrl.h"

#include "wx/dateevt.h"
#include "wx/spinbutt.h"

#ifndef wxHAS_NATIVE_TIMEPICKERCTRL
wxIMPLEMENT_DYNAMIC_CLASS(wxTimePickerCtrl, wxControl);
#endif


enum
{
        HMARGIN_TEXT_SPIN = 2
};


class wxTimePickerGenericImpl : public wxEvtHandler
{
public:
    wxTimePickerGenericImpl(wxTimePickerCtrlGeneric* ctrl)
    {
        m_text = new wxTextCtrl(ctrl, wxID_ANY, wxString());

                                m_text->SetCursor(wxCURSOR_ARROW);

        m_btn = new wxSpinButton(ctrl, wxID_ANY,
                                 wxDefaultPosition, wxDefaultSize,
                                 wxSP_VERTICAL | wxSP_WRAP);

        m_currentField = Field_Hour;
        m_isFirstDigit = true;

                                                        #if wxUSE_XLOCALE
        m_useAMPM = wxLocale::GetInfo(wxLOCALE_TIME_FMT).Contains("%p");
#else
        m_useAMPM = false;
#endif

        m_text->Connect
                (
                    wxEVT_SET_FOCUS,
                    wxFocusEventHandler(wxTimePickerGenericImpl::OnTextSetFocus),
                    NULL,
                    this
                );
        m_text->Connect
                (
                    wxEVT_KEY_DOWN,
                    wxKeyEventHandler(wxTimePickerGenericImpl::OnTextKeyDown),
                    NULL,
                    this
                );
        m_text->Connect
                (
                    wxEVT_LEFT_DOWN,
                    wxMouseEventHandler(wxTimePickerGenericImpl::OnTextClick),
                    NULL,
                    this
                );

        m_btn->Connect
               (
                    wxEVT_SPIN_UP,
                    wxSpinEventHandler(wxTimePickerGenericImpl::OnArrowUp),
                    NULL,
                    this
               );
        m_btn->Connect
               (
                    wxEVT_SPIN_DOWN,
                    wxSpinEventHandler(wxTimePickerGenericImpl::OnArrowDown),
                    NULL,
                    this
               );
    }

        void SetValue(const wxDateTime& time)
    {
        m_time = time.IsValid() ? time : wxDateTime::Now();

                                                wxDateTime::Tm tm = m_time.GetTm();
        tm.mday =
        tm.yday = 1;
        tm.mon = wxDateTime::Jan;
        m_time.Set(tm);

        UpdateTextWithoutEvent();
    }


        wxTextCtrl* m_text;

        wxSpinButton* m_btn;

        wxDateTime m_time;

private:
        enum Field
    {
        Field_Hour,
        Field_Min,
        Field_Sec,
        Field_AMPM,
        Field_Max
    };

        enum Direction
    {
                Dir_Down = -1,
        Dir_Up   = +1
    };

        struct CharRange
    {
        int from,
            to;
    };

        void OnTextSetFocus(wxFocusEvent& event)
    {
        HighlightCurrentField();

        event.Skip();
    }

            void OnTextKeyDown(wxKeyEvent& event)
    {
        const int key = event.GetKeyCode();

        switch ( key )
        {
            case WXK_DOWN:
                ChangeCurrentFieldBy1(Dir_Down);
                break;

            case WXK_UP:
                ChangeCurrentFieldBy1(Dir_Up);
                break;

            case WXK_LEFT:
                CycleCurrentField(Dir_Down);
                break;

            case WXK_RIGHT:
                CycleCurrentField(Dir_Up);
                break;

            case WXK_HOME:
                ResetCurrentField(Dir_Down);
                break;

            case WXK_END:
                ResetCurrentField(Dir_Up);
                break;

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                                if ( m_currentField != Field_AMPM )
                {
                    AppendDigitToCurrentField(key - '0');
                }
                break;

            case 'A':
            case 'P':
                                if ( m_currentField == Field_AMPM )
                {
                    unsigned hour = m_time.GetHour();
                    if ( key == 'A' )
                    {
                        if ( hour >= 12 )
                            hour -= 12;
                    }
                    else                     {
                        if ( hour < 12 )
                            hour += 12;
                    }

                    if ( hour != m_time.GetHour() )
                    {
                        m_time.SetHour(hour);
                        UpdateText();
                    }
                }
                break;

                                }
    }

    void OnTextClick(wxMouseEvent& event)
    {
        Field field = Field_Max;         long pos;
        switch ( m_text->HitTest(event.GetPosition(), &pos) )
        {
            case wxTE_HT_UNKNOWN:
                                return;

            case wxTE_HT_BEFORE:
                                field = Field_Hour;
                break;

            case wxTE_HT_ON_TEXT:
                                for ( field = Field_Hour; field <= GetLastField(); )
                {
                    const CharRange range = GetFieldRange(field);

                                                                                if ( range.from <= pos && pos <= range.to )
                        break;

                    field = static_cast<Field>(field + 1);
                }
                break;

            case wxTE_HT_BELOW:
                                wxFAIL_MSG( "Unreachable" );
                
            case wxTE_HT_BEYOND:
                                field = GetLastField();
                break;
        }

        ChangeCurrentField(field);
    }

    void OnArrowUp(wxSpinEvent& WXUNUSED(event))
    {
        ChangeCurrentFieldBy1(Dir_Up);
    }

    void OnArrowDown(wxSpinEvent& WXUNUSED(event))
    {
        ChangeCurrentFieldBy1(Dir_Down);
    }


            static CharRange GetFieldRange(Field field)
    {
                                        static const CharRange ranges[] =
        {
            { 0, 2 },
            { 3, 5 },
            { 6, 8 },
            { 9, 11},
        };

        wxCOMPILE_TIME_ASSERT( WXSIZEOF(ranges) == Field_Max,
                               FieldRangesMismatch );

        return ranges[field];
    }

        Field GetLastField() const
    {
        return m_useAMPM ? Field_AMPM : Field_Sec;
    }

            void ChangeCurrentField(int field)
    {
        if ( field == m_currentField )
            return;

        wxCHECK_RET( field <= GetLastField(), "Invalid field" );

        m_currentField = static_cast<Field>(field);
        m_isFirstDigit = true;

        HighlightCurrentField();
    }

            void CycleCurrentField(Direction dir)
    {
        const unsigned numFields = GetLastField() + 1;

        ChangeCurrentField((m_currentField + numFields + dir) % numFields);
    }

        void HighlightCurrentField()
    {
        m_text->SetFocus();

        const CharRange range = GetFieldRange(m_currentField);

        m_text->SetSelection(range.from, range.to);
    }

            void ChangeCurrentFieldBy1(Direction dir)
    {
        switch ( m_currentField )
        {
            case Field_Hour:
                m_time.SetHour((m_time.GetHour() + 24 + dir) % 24);
                break;

            case Field_Min:
                m_time.SetMinute((m_time.GetMinute() + 60 + dir) % 60);
                break;

            case Field_Sec:
                m_time.SetSecond((m_time.GetSecond() + 60 + dir) % 60);
                break;

            case Field_AMPM:
                m_time.SetHour((m_time.GetHour() + 12) % 24);
                break;

            case Field_Max:
                wxFAIL_MSG( "Invalid field" );
                return;
        }

        UpdateText();
    }

        void ResetCurrentField(Direction dir)
    {
        switch ( m_currentField )
        {
            case Field_Hour:
            case Field_AMPM:
                                                                                                                m_time.SetHour(dir == Dir_Down ? 0 : 23);
                break;

            case Field_Min:
                m_time.SetMinute(dir == Dir_Down ? 0 : 59);
                break;

            case Field_Sec:
                m_time.SetSecond(dir == Dir_Down ? 0 : 59);
                break;

            case Field_Max:
                wxFAIL_MSG( "Invalid field" );
        }

        UpdateText();
    }

            void AppendDigitToCurrentField(int n)
    {
        bool moveToNextField = false;

        if ( !m_isFirstDigit )
        {
                                                int currentValue = 0,
                maxValue  = 0;

            switch ( m_currentField )
            {
                case Field_Hour:
                    currentValue = m_time.GetHour();
                    maxValue = 23;
                    break;

                case Field_Min:
                    currentValue = m_time.GetMinute();
                    maxValue = 59;
                    break;

                case Field_Sec:
                    currentValue = m_time.GetSecond();
                    maxValue = 59;
                    break;

                case Field_AMPM:
                case Field_Max:
                    wxFAIL_MSG( "Invalid field" );
                    return;
            }

                                    int newValue = currentValue*10 + n;
            if ( newValue < maxValue )
            {
                n = newValue;

                                                                                                                                moveToNextField = true;

                                m_isFirstDigit = true;
            }
        }
        else         {
                        m_isFirstDigit = false;
        }

        switch ( m_currentField )
        {
            case Field_Hour:
                m_time.SetHour(n);
                break;

            case Field_Min:
                m_time.SetMinute(n);
                break;

            case Field_Sec:
                m_time.SetSecond(n);
                break;

            case Field_AMPM:
            case Field_Max:
                wxFAIL_MSG( "Invalid field" );
        }

        if ( moveToNextField && m_currentField < Field_Sec )
            CycleCurrentField(Dir_Up);

        UpdateText();
    }

                void UpdateText()
    {
        UpdateTextWithoutEvent();

        wxWindow* const ctrl = m_text->GetParent();

        wxDateEvent event(ctrl, m_time, wxEVT_TIME_CHANGED);
        ctrl->HandleWindowEvent(event);
    }

    void UpdateTextWithoutEvent()
    {
        m_text->SetValue(m_time.Format(m_useAMPM ? "%I:%M:%S %p" : "%H:%M:%S"));

        HighlightCurrentField();
    }


            Field m_currentField;

        bool m_useAMPM;

                        bool m_isFirstDigit;

    wxDECLARE_NO_COPY_CLASS(wxTimePickerGenericImpl);
};



void wxTimePickerCtrlGeneric::Init()
{
    m_impl = NULL;
}

bool
wxTimePickerCtrlGeneric::Create(wxWindow *parent,
                                wxWindowID id,
                                const wxDateTime& date,
                                const wxPoint& pos,
                                const wxSize& size,
                                long style,
                                const wxValidator& validator,
                                const wxString& name)
{
            style &= ~wxBORDER_MASK;
    style |= wxBORDER_NONE;

    if ( !Base::Create(parent, id, pos, size, style, validator, name) )
        return false;

    m_impl = new wxTimePickerGenericImpl(this);
    m_impl->SetValue(date);

    InvalidateBestSize();
    SetInitialSize(size);

    return true;
}

wxTimePickerCtrlGeneric::~wxTimePickerCtrlGeneric()
{
    delete m_impl;
}

wxWindowList wxTimePickerCtrlGeneric::GetCompositeWindowParts() const
{
    wxWindowList parts;
    if ( m_impl )
    {
        parts.push_back(m_impl->m_text);
        parts.push_back(m_impl->m_btn);
    }
    return parts;
}


void wxTimePickerCtrlGeneric::SetValue(const wxDateTime& date)
{
    wxCHECK_RET( m_impl, "Must create first" );

    m_impl->SetValue(date);
}

wxDateTime wxTimePickerCtrlGeneric::GetValue() const
{
    wxCHECK_MSG( m_impl, wxDateTime(), "Must create first" );

    return m_impl->m_time;
}


void wxTimePickerCtrlGeneric::DoMoveWindow(int x, int y, int width, int height)
{
    Base::DoMoveWindow(x, y, width, height);

    if ( !m_impl )
        return;

    const int widthBtn = m_impl->m_btn->GetSize().x;
    const int widthText = wxMax(width - widthBtn - HMARGIN_TEXT_SPIN, 0);

    m_impl->m_text->SetSize(0, 0, widthText, height);
    m_impl->m_btn->SetSize(widthText + HMARGIN_TEXT_SPIN, 0, widthBtn, height);
}

wxSize wxTimePickerCtrlGeneric::DoGetBestSize() const
{
    if ( !m_impl )
        return Base::DoGetBestSize();

    wxSize size = m_impl->m_text->GetBestSize();
    size.x += m_impl->m_btn->GetBestSize().x + HMARGIN_TEXT_SPIN;

    return size;
}

#endif 
#endif 