
#include "wx/wxprec.h"

#if wxUSE_RADIOBOX

#include "wx/radiobox.h"

#ifndef WX_PRECOMP
    #include "wx/radiobut.h"
    #include "wx/arrstr.h"
#endif

#include "wx/osx/private.h"


wxIMPLEMENT_DYNAMIC_CLASS(wxRadioBox, wxControl);


wxBEGIN_EVENT_TABLE(wxRadioBox, wxControl)
    EVT_RADIOBUTTON( wxID_ANY , wxRadioBox::OnRadioButton )
wxEND_EVENT_TABLE()


void wxRadioBox::OnRadioButton( wxCommandEvent &outer )
{
    if ( outer.IsChecked() )
    {
        wxCommandEvent event( wxEVT_RADIOBOX, m_windowId );
        int i = GetSelection() ;
        event.SetInt(i);
        event.SetString(GetString(i));
        event.SetEventObject( this );
        ProcessCommand(event);
    }
}

wxRadioBox::wxRadioBox()
{
    m_noItems = 0;
    m_noRowsOrCols = 0;
    m_radioButtonCycle = NULL;
}

wxRadioBox::~wxRadioBox()
{
    SendDestroyEvent();

    wxRadioButton *next, *current;

    current = m_radioButtonCycle->NextInCycle();
    if (current != NULL)
    {
        while (current != m_radioButtonCycle)
        {
            next = current->NextInCycle();
            delete current;

            current = next;
        }

        delete current;
    }
}


bool wxRadioBox::Create( wxWindow *parent,
    wxWindowID id, const wxString& label,
    const wxPoint& pos, const wxSize& size,
    const wxArrayString& choices,
    int majorDim, long style,
    const wxValidator& val, const wxString& name )
{
    wxCArrayString chs(choices);

    return Create(
        parent, id, label, pos, size, chs.GetCount(),
        chs.GetStrings(), majorDim, style, val, name);
}

bool wxRadioBox::Create( wxWindow *parent,
    wxWindowID id, const wxString& label,
    const wxPoint& pos, const wxSize& size,
    int n, const wxString choices[],
    int majorDim, long style,
    const wxValidator& val, const wxString& name )
{    
    DontCreatePeer();
    
    if ( !wxControl::Create( parent, id, pos, size, style, val, name ) )
        return false;

        m_noItems = 0;
    m_noRowsOrCols = majorDim;
    m_radioButtonCycle = NULL;

    SetMajorDim( majorDim == 0 ? n : majorDim, style );

    m_labelOrig = m_label = label;

    SetPeer(wxWidgetImpl::CreateGroupBox( this, parent, id, label, pos, size, style, GetExtraStyle() ));

    for (int i = 0; i < n; i++)
    {
        wxRadioButton *radBtn = new wxRadioButton(
            this,
            wxID_ANY,
            GetLabelText(choices[i]),
            wxPoint( 5, 20 * i + 10 ),
            wxDefaultSize,
            i == 0 ? wxRB_GROUP : 0 );

        if ( i == 0 )
            m_radioButtonCycle = radBtn;
    }

        m_noItems = (unsigned int)n;
    SetMajorDim( majorDim == 0 ? n : majorDim, style );

    SetSelection( 0 );
    InvalidateBestSize();
    SetInitialSize( size );
    MacPostControlCreate( pos, size );

    return true;
}

bool wxRadioBox::Enable(bool enable)
{
    wxRadioButton *current;

    if (!wxControl::Enable( enable ))
        return false;

    current = m_radioButtonCycle;
    for (unsigned int i = 0; i < m_noItems; i++)
    {
        current->Enable( enable );
        current = current->NextInCycle();
    }

    return true;
}

bool wxRadioBox::Enable(unsigned int item, bool enable)
{
    if (!IsValid( item ))
        return false;

    unsigned int i = 0;
    wxRadioButton *current = m_radioButtonCycle;
    while (i != item)
    {
        i++;
        current = current->NextInCycle();
    }

    return current->Enable( enable );
}

bool wxRadioBox::IsItemEnabled(unsigned int item) const
{
    if (!IsValid( item ))
        return false;

    unsigned int i = 0;
    wxRadioButton *current = m_radioButtonCycle;
    while (i != item)
    {
        i++;
        current = current->NextInCycle();
    }

    return current->IsEnabled();
}

wxString wxRadioBox::GetLabel() const
{
    return wxControl::GetLabel();
}

wxString wxRadioBox::GetString(unsigned int item) const
{
    wxRadioButton *current;

    if (!IsValid( item ))
        return wxEmptyString;

    unsigned int i = 0;
    current = m_radioButtonCycle;
    while (i != item)
    {
        i++;
        current = current->NextInCycle();
    }

    return current->GetLabel();
}

int wxRadioBox::GetSelection() const
{
    int i;
    wxRadioButton *current;

    i = 0;
    current = m_radioButtonCycle;
    while (!current->GetValue())
    {
        i++;
        current = current->NextInCycle();
    }

    return i;
}

void wxRadioBox::SetLabel(const wxString& label)
{
    return wxControl::SetLabel( label );
}

void wxRadioBox::SetString(unsigned int item,const wxString& label)
{
    if (!IsValid( item ))
        return;

    unsigned int i = 0;
    wxRadioButton *current = m_radioButtonCycle;
    while (i != item)
    {
        i++;
        current = current->NextInCycle();
    }

    return current->SetLabel( label );
}

void wxRadioBox::SetSelection(int item)
{
    int i;
    wxRadioButton *current;

    if (!IsValid( item ))
        return;

    i = 0;
    current = m_radioButtonCycle;
    while (i != item)
    {
        i++;
        current = current->NextInCycle();
    }

    current->SetValue( true );
}

bool wxRadioBox::Show(bool show)
{
    wxRadioButton *current;

    current = m_radioButtonCycle;
    for (unsigned int i=0; i<m_noItems; i++)
    {
        current->Show( show );
        current = current->NextInCycle();
    }

    wxControl::Show( show );

    return true;
}

bool wxRadioBox::Show(unsigned int item, bool show)
{
    if (!IsValid( item ))
        return false;

    unsigned int i = 0;
    wxRadioButton *current = m_radioButtonCycle;
    while (i != item)
    {
        i++;
        current = current->NextInCycle();
    }

    return current->Show( show );
}

bool wxRadioBox::IsItemShown(unsigned int item) const
{
    if (!IsValid( item ))
        return false;

    unsigned int i = 0;
    wxRadioButton *current = m_radioButtonCycle;
    while (i != item)
    {
        i++;
        current = current->NextInCycle();
    }

    return current->IsShown();
}


void wxRadioBox::Command( wxCommandEvent& event )
{
    SetSelection( event.GetInt() );
    ProcessCommand( event );
}

void wxRadioBox::SetFocus()
{
    wxRadioButton *current;

    current = m_radioButtonCycle;
    while (!current->GetValue())
    {
        current = current->NextInCycle();
    }

    current->SetFocus();
}

#define RADIO_SIZE 23

void wxRadioBox::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
    int i;
    wxRadioButton *current;

    
    int x_current, y_current;
    int x_offset, y_offset;
    int widthOld, heightOld;

    GetSize( &widthOld, &heightOld );
    GetPosition( &x_current, &y_current );

    x_offset = x;
    y_offset = y;
    if (!(sizeFlags & wxSIZE_ALLOW_MINUS_ONE))
    {
        if (x == wxDefaultCoord)
            x_offset = x_current;
        if (y == wxDefaultCoord)
            y_offset = y_current;
    }

        int charWidth, charHeight;
    int maxWidth, maxHeight;
    int eachWidth[128], eachHeight[128];
    int totWidth, totHeight;

    GetTextExtent(
        wxT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"),
        &charWidth, &charHeight );

    charWidth /= 52;

    maxWidth = -1;
    maxHeight = -1;
    wxSize bestSizeRadio ;
    if ( m_radioButtonCycle )
        bestSizeRadio = m_radioButtonCycle->GetBestSize();

    for (unsigned int i = 0 ; i < m_noItems; i++)
    {
        GetTextExtent(GetString(i), &eachWidth[i], &eachHeight[i] );
        eachWidth[i] = eachWidth[i] + RADIO_SIZE;
        eachHeight[i] = wxMax( eachHeight[i], bestSizeRadio.y );

        if (maxWidth < eachWidth[i])
            maxWidth = eachWidth[i];
        if (maxHeight < eachHeight[i])
            maxHeight = eachHeight[i];
    }

        int space = 3;
    if ( GetWindowVariant() == wxWINDOW_VARIANT_MINI )
        space = 2;

    totHeight = GetRowCount() * maxHeight + (GetRowCount() - 1) * space;
    totWidth  = GetColumnCount() * (maxWidth + charWidth);

        wxSize sz;
    if ( (width == wxDefaultCoord && (sizeFlags & wxSIZE_AUTO_WIDTH)) ||
            (height == wxDefaultCoord && (sizeFlags & wxSIZE_AUTO_HEIGHT)) )
    {
        sz = DoGetSizeFromClientSize( wxSize( totWidth, totHeight ) ) ;
    }

        if ( width == wxDefaultCoord )
    {
        if ( sizeFlags & wxSIZE_AUTO_WIDTH )
            width = sz.x;
        else
            width = widthOld;
    }

    if ( height == wxDefaultCoord )
    {
        if ( sizeFlags & wxSIZE_AUTO_HEIGHT )
            height = sz.y;
        else
            height = heightOld;
    }

    wxControl::DoSetSize( x_offset, y_offset, width, height, wxSIZE_AUTO );

                sz = DoGetSizeFromClientSize( wxSize( totWidth, totHeight ) ) ;

        int x_start, y_start;

    x_start = ( width - sz.x ) / 2;
    y_start = ( height - sz.y ) / 2;

    x_offset = x_start;
    y_offset = y_start;

    current = m_radioButtonCycle;
    for (i = 0 ; i < (int)m_noItems; i++)
    {
                if ((i > 0) && ((i % GetMajorDim()) == 0))
        {
            if (m_windowStyle & wxRA_SPECIFY_ROWS)
            {
                x_offset += maxWidth + charWidth;
                y_offset = y_start;
            }
            else
            {
                x_offset = x_start;
                y_offset += maxHeight + space;
            }
        }

        current->SetSize( x_offset, y_offset, eachWidth[i], eachHeight[i] );
        current = current->NextInCycle();

        if (m_windowStyle & wxRA_SPECIFY_ROWS)
            y_offset += maxHeight + space;
        else
            x_offset += maxWidth + charWidth;
    }
}

wxSize wxRadioBox::DoGetBestSize() const
{
    int charWidth, charHeight;
    int maxWidth, maxHeight;
    int eachWidth, eachHeight;
    int totWidth, totHeight;

    wxFont font = GetFont();     GetTextExtent(
        wxT("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"),
        &charWidth, &charHeight, NULL, NULL, &font );

    charWidth /= 52;

    maxWidth = -1;
    maxHeight = -1;

    wxSize bestSizeRadio ;
    if ( m_radioButtonCycle )
        bestSizeRadio = m_radioButtonCycle->GetBestSize();

    for (unsigned int i = 0 ; i < m_noItems; i++)
    {
        GetTextExtent(GetString(i), &eachWidth, &eachHeight, NULL, NULL, &font );
        eachWidth  = (eachWidth + RADIO_SIZE);
        eachHeight = wxMax(eachHeight, bestSizeRadio.y );
        if (maxWidth < eachWidth)
            maxWidth = eachWidth;
        if (maxHeight < eachHeight)
            maxHeight = eachHeight;
    }

        int space = 3;
    if ( GetWindowVariant() == wxWINDOW_VARIANT_MINI )
        space = 2;

    totHeight = GetRowCount() * maxHeight + (GetRowCount() - 1) * space;
    totWidth  = GetColumnCount() * (maxWidth + charWidth);

    wxSize sz = DoGetSizeFromClientSize( wxSize( totWidth, totHeight ) );
    totWidth = sz.x;
    totHeight = sz.y;

        totWidth += 10;
    totHeight += 10;

        GetTextExtent( GetLabel(), &eachWidth, NULL );
    eachWidth  = (int)(eachWidth + RADIO_SIZE) +  3 * charWidth;
    if (totWidth < eachWidth)
        totWidth = eachWidth;

    return wxSize( totWidth, totHeight );
}

bool wxRadioBox::SetFont(const wxFont& font)
{
    bool retval = wxWindowBase::SetFont( font );

    
    
    return retval;
}

#endif 