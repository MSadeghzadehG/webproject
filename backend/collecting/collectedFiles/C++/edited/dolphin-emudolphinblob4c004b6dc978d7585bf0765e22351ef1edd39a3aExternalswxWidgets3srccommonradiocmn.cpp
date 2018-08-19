


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_RADIOBOX

#ifndef WX_PRECOMP
    #include "wx/radiobox.h"
#endif 
#if wxUSE_TOOLTIPS
    #include "wx/tooltip.h"
#endif 
#if wxUSE_HELP
    #include "wx/cshelp.h"
#endif

extern WXDLLEXPORT_DATA(const char) wxRadioBoxNameStr[] = "radioBox";



#if 0 wxDEFINE_FLAGS( wxRadioBoxStyle )

wxBEGIN_FLAGS( wxRadioBoxStyle )
wxFLAGS_MEMBER(wxBORDER_SIMPLE)
wxFLAGS_MEMBER(wxBORDER_SUNKEN)
wxFLAGS_MEMBER(wxBORDER_DOUBLE)
wxFLAGS_MEMBER(wxBORDER_RAISED)
wxFLAGS_MEMBER(wxBORDER_STATIC)
wxFLAGS_MEMBER(wxBORDER_NONE)

wxFLAGS_MEMBER(wxSIMPLE_BORDER)
wxFLAGS_MEMBER(wxSUNKEN_BORDER)
wxFLAGS_MEMBER(wxDOUBLE_BORDER)
wxFLAGS_MEMBER(wxRAISED_BORDER)
wxFLAGS_MEMBER(wxSTATIC_BORDER)
wxFLAGS_MEMBER(wxBORDER)

wxFLAGS_MEMBER(wxTAB_TRAVERSAL)
wxFLAGS_MEMBER(wxCLIP_CHILDREN)
wxFLAGS_MEMBER(wxTRANSPARENT_WINDOW)
wxFLAGS_MEMBER(wxWANTS_CHARS)
wxFLAGS_MEMBER(wxFULL_REPAINT_ON_RESIZE)
wxFLAGS_MEMBER(wxALWAYS_SHOW_SB )
wxFLAGS_MEMBER(wxVSCROLL)
wxFLAGS_MEMBER(wxHSCROLL)

wxFLAGS_MEMBER(wxRA_SPECIFY_COLS)
wxFLAGS_MEMBER(wxRA_HORIZONTAL)
wxFLAGS_MEMBER(wxRA_SPECIFY_ROWS)
wxFLAGS_MEMBER(wxRA_VERTICAL)

wxEND_FLAGS( wxRadioBoxStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxRadioBox, wxControl, "wx/radiobox.h");

wxBEGIN_PROPERTIES_TABLE(wxRadioBox)
wxEVENT_PROPERTY( Select , wxEVT_RADIOBOX , wxCommandEvent )
wxPROPERTY_FLAGS( WindowStyle , wxRadioBoxStyle , long , SetWindowStyleFlag , GetWindowStyleFlag , , 0  , wxT("Helpstring") , wxT("group")) wxEND_PROPERTIES_TABLE()



#endif



void wxRadioBoxBase::SetMajorDim(unsigned int majorDim, long style)
{
    wxCHECK_RET( majorDim != 0, wxT("major radiobox dimension can't be 0") );

    m_majorDim = majorDim;

    int minorDim = (GetCount() + m_majorDim - 1) / m_majorDim;

    if ( style & wxRA_SPECIFY_COLS )
    {
        m_numCols = majorDim;
        m_numRows = minorDim;
    }
    else     {
        m_numCols = minorDim;
        m_numRows = majorDim;
    }
}

int wxRadioBoxBase::GetNextItem(int item, wxDirection dir, long style) const
{
    const int itemStart = item;

    int count = GetCount(),
        numCols = GetColumnCount(),
        numRows = GetRowCount();

    bool horz = (style & wxRA_SPECIFY_COLS) != 0;

    do
    {
        switch ( dir )
        {
            case wxUP:
                if ( horz )
                {
                    item -= numCols;
                }
                else                 {
                    if ( !item-- )
                        item = count - 1;
                }
                break;

            case wxLEFT:
                if ( horz )
                {
                    if ( !item-- )
                        item = count - 1;
                }
                else                 {
                    item -= numRows;
                }
                break;

            case wxDOWN:
                if ( horz )
                {
                    item += numCols;
                }
                else                 {
                    if ( ++item == count )
                        item = 0;
                }
                break;

            case wxRIGHT:
                if ( horz )
                {
                    if ( ++item == count )
                        item = 0;
                }
                else                 {
                    item += numRows;
                }
                break;

            default:
                wxFAIL_MSG( wxT("unexpected wxDirection value") );
                return wxNOT_FOUND;
        }

                if ( item < 0 )
        {
                                    item += count;

                                                if ( item % (horz ? numCols : numRows) )
                item--;
            else
                item = count - 1;
        }
        else if ( item >= count )
        {
                        item -= count;

                                    if ( (item + 1) % (horz ? numCols : numRows) )
                item++;
            else
                item = 0;
        }

        wxASSERT_MSG( item < count && item >= 0,
                      wxT("logic error in wxRadioBox::GetNextItem()") );
    }
                while ( !(IsItemShown(item) && IsItemEnabled(item)) && item != itemStart );

    return item;
}

#if wxUSE_TOOLTIPS

void wxRadioBoxBase::SetItemToolTip(unsigned int item, const wxString& text)
{
    wxASSERT_MSG( item < GetCount(), wxT("Invalid item index") );

        if ( !m_itemsTooltips )
    {
        m_itemsTooltips = new wxToolTipArray;
        m_itemsTooltips->resize(GetCount());
    }

    wxToolTip *tooltip = (*m_itemsTooltips)[item];

    bool changed = true;
    if ( text.empty() )
    {
        if ( tooltip )
        {
                        wxDELETE(tooltip);
        }
        else         {
            changed = false;
        }
    }
    else     {
        if ( tooltip )
        {
                        tooltip->SetTip(text);
            changed = false;
        }
        else         {
                        tooltip = new wxToolTip(text);
        }
    }

    if ( changed )
    {
        (*m_itemsTooltips)[item] = tooltip;
        DoSetItemToolTip(item, tooltip);
    }
}

void
wxRadioBoxBase::DoSetItemToolTip(unsigned int WXUNUSED(item),
                                 wxToolTip * WXUNUSED(tooltip))
{
    }

#endif 
wxRadioBoxBase::~wxRadioBoxBase()
{
#if wxUSE_TOOLTIPS
    if ( m_itemsTooltips )
    {
        const size_t n = m_itemsTooltips->size();
        for ( size_t i = 0; i < n; i++ )
            delete (*m_itemsTooltips)[i];

        delete m_itemsTooltips;
    }
#endif }

#if wxUSE_HELP

void wxRadioBoxBase::SetItemHelpText(unsigned int n, const wxString& helpText)
{
    wxCHECK_RET( n < GetCount(), wxT("Invalid item index") );

    if ( m_itemsHelpTexts.empty() )
    {
                m_itemsHelpTexts.Add(wxEmptyString, GetCount());
    }

    m_itemsHelpTexts[n] = helpText;
}

wxString wxRadioBoxBase::GetItemHelpText( unsigned int n ) const
{
    wxCHECK_MSG( n < GetCount(), wxEmptyString, wxT("Invalid item index") );

    return m_itemsHelpTexts.empty() ? wxString() : m_itemsHelpTexts[n];
}

wxString wxRadioBoxBase::DoGetHelpTextAtPoint(const wxWindow *derived,
                                              const wxPoint& pt,
                                              wxHelpEvent::Origin origin) const
{
    int item;
    switch ( origin )
    {
        case wxHelpEvent::Origin_HelpButton:
            item = GetItemFromPoint(pt);
            break;

        case wxHelpEvent::Origin_Keyboard:
            item = GetSelection();
            break;

        default:
            wxFAIL_MSG( "unknown help even origin" );
            wxFALLTHROUGH;

        case wxHelpEvent::Origin_Unknown:
                                    item = wxNOT_FOUND;
    }

    if ( item != wxNOT_FOUND )
    {
        wxString text = GetItemHelpText(static_cast<unsigned int>(item));
        if( !text.empty() )
            return text;
    }

    return derived->wxWindowBase::GetHelpTextAtPoint(pt, origin);
}

#endif 
#endif 