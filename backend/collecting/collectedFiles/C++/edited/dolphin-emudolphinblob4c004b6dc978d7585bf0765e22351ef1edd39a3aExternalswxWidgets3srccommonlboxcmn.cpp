


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_LISTBOX

#include "wx/listbox.h"

#ifndef WX_PRECOMP
    #include "wx/dynarray.h"
    #include "wx/arrstr.h"
    #include "wx/log.h"
    #include "wx/dcclient.h"
#endif

static const int LINE_SPACING = 0;

#ifdef __WXGTK__
static const int EXTRA_HEIGHT = 6;
#else
static const int EXTRA_HEIGHT = 4;
#endif

extern WXDLLEXPORT_DATA(const char) wxListBoxNameStr[] = "listBox";


wxListBoxBase::~wxListBoxBase()
{
    }


wxDEFINE_FLAGS( wxListBoxStyle )
wxBEGIN_FLAGS( wxListBoxStyle )
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

wxFLAGS_MEMBER(wxLB_SINGLE)
wxFLAGS_MEMBER(wxLB_MULTIPLE)
wxFLAGS_MEMBER(wxLB_EXTENDED)
wxFLAGS_MEMBER(wxLB_HSCROLL)
wxFLAGS_MEMBER(wxLB_ALWAYS_SB)
wxFLAGS_MEMBER(wxLB_NEEDED_SB)
wxFLAGS_MEMBER(wxLB_SORT)
wxEND_FLAGS( wxListBoxStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxListBox, wxControl, "wx/listbox.h");

wxBEGIN_PROPERTIES_TABLE(wxListBox)
wxEVENT_PROPERTY( Select, wxEVT_LISTBOX, wxCommandEvent )
wxEVENT_PROPERTY( DoubleClick, wxEVT_LISTBOX_DCLICK, wxCommandEvent )

wxPROPERTY( Font, wxFont, SetFont, GetFont , wxEMPTY_PARAMETER_VALUE, 0 , \
           wxT("Helpstring"), wxT("group"))
wxPROPERTY_COLLECTION( Choices, wxArrayString, wxString, AppendString, \
                      GetStrings, 0 , wxT("Helpstring"), wxT("group") )
wxPROPERTY( Selection, int, SetSelection, GetSelection, wxEMPTY_PARAMETER_VALUE, \
           0 , wxT("Helpstring"), wxT("group") )

wxPROPERTY_FLAGS( WindowStyle, wxListBoxStyle, long, SetWindowStyleFlag, \
                 GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 , \
                 wxT("Helpstring"), wxT("group")) wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxListBox)

wxCONSTRUCTOR_4( wxListBox, wxWindow*, Parent, wxWindowID, Id, \
                wxPoint, Position, wxSize, Size )




bool wxListBoxBase::SetStringSelection(const wxString& s, bool select)
{
    const int sel = FindString(s);
    if ( sel == wxNOT_FOUND )
        return false;

    SetSelection(sel, select);

    return true;
}

void wxListBoxBase::SetSelection(int n)
{
    if ( !HasMultipleSelection() )
        DoChangeSingleSelection(n);

    DoSetSelection(n, true);
}

void wxListBoxBase::DeselectAll(int itemToLeaveSelected)
{
    if ( HasMultipleSelection() )
    {
        wxArrayInt selections;
        GetSelections(selections);

        size_t count = selections.GetCount();
        for ( size_t n = 0; n < count; n++ )
        {
            int item = selections[n];
            if ( item != itemToLeaveSelected )
                Deselect(item);
        }
    }
    else     {
        int sel = GetSelection();
        if ( sel != wxNOT_FOUND && sel != itemToLeaveSelected )
        {
            Deselect(sel);
        }
    }
}

void wxListBoxBase::UpdateOldSelections()
{
            if ( IsEmpty() )
    {
        m_oldSelections.clear();
        return;
    }

            #ifndef __WXMSW__
    if (HasFlag(wxLB_MULTIPLE) || HasFlag(wxLB_EXTENDED))
#endif
    {
        GetSelections( m_oldSelections );
    }
}

bool wxListBoxBase::SendEvent(wxEventType evtType, int item, bool selected)
{
    wxCommandEvent event(evtType, GetId());
    event.SetEventObject(this);

    event.SetInt(item);
    event.SetString(GetString(item));
    event.SetExtraLong(selected);

    if ( HasClientObjectData() )
        event.SetClientObject(GetClientObject(item));
    else if ( HasClientUntypedData() )
        event.SetClientData(GetClientData(item));

    return HandleWindowEvent(event);
}

bool wxListBoxBase::DoChangeSingleSelection(int item)
{
                    if ( !m_oldSelections.empty() && *m_oldSelections.begin() == item )
    {
                return false;
    }

    m_oldSelections.clear();
    m_oldSelections.push_back(item);

    return true;
}

bool wxListBoxBase::CalcAndSendEvent()
{
    wxArrayInt selections;
    GetSelections(selections);
    bool selected = true;

    if ( selections.empty() && m_oldSelections.empty() )
    {
                return false;
    }

    const size_t countSel = selections.size(),
                 countSelOld = m_oldSelections.size();
    if ( countSel == countSelOld )
    {
        bool changed = false;
        for ( size_t idx = 0; idx < countSel; idx++ )
        {
            if (selections[idx] != m_oldSelections[idx])
            {
                changed = true;
                break;
            }
        }

                if ( !changed )
           return false;
    }

    int item = wxNOT_FOUND;
    if ( selections.empty() )
    {
        selected = false;
        item = m_oldSelections[0];
    }
    else     {
                bool any_new_selected = false;
        for ( size_t idx = 0; idx < countSel; idx++ )
        {
            item = selections[idx];
            if ( m_oldSelections.Index(item) == wxNOT_FOUND )
            {
                any_new_selected = true;
                break;
            }
        }

        if ( !any_new_selected )
        {
                        bool any_new_deselected = false;
            for ( size_t idx = 0; idx < countSelOld; idx++ )
            {
                item = m_oldSelections[idx];
                if ( selections.Index(item) == wxNOT_FOUND )
                {
                    any_new_deselected = true;
                    break;
                }
            }

            if ( any_new_deselected )
            {
                                selected = false;
            }
            else
            {
                item = wxNOT_FOUND;             }
        }
    }

    wxASSERT_MSG( item != wxNOT_FOUND,
                  "Logic error in wxListBox selection event generation code" );

    m_oldSelections = selections;

    return SendEvent(wxEVT_LISTBOX, item, selected);
}


void wxListBoxBase::Command(wxCommandEvent& event)
{
    SetSelection(event.GetInt(), event.GetExtraLong() != 0);
    (void)GetEventHandler()->ProcessEvent(event);
}


void wxListBoxBase::SetFirstItem(const wxString& s)
{
    int n = FindString(s);

    wxCHECK_RET( n != wxNOT_FOUND, wxT("invalid string in wxListBox::SetFirstItem") );

    DoSetFirstItem(n);
}

void wxListBoxBase::AppendAndEnsureVisible(const wxString& s)
{
    Append(s);
    EnsureVisible(GetCount() - 1);
}

void wxListBoxBase::EnsureVisible(int WXUNUSED(n))
{
        }

wxCoord wxListBoxBase::GetLineHeight() const
{
    wxListBoxBase *self = wxConstCast(this, wxListBoxBase);

    wxClientDC dc( self );
    dc.SetFont( GetFont() );

    wxCoord y;
    dc.GetTextExtent(wxT("H"), NULL, &y);

    y += EXTRA_HEIGHT;

    return y + LINE_SPACING;
}

int wxListBoxBase::GetCountPerPage() const
{
    return GetClientSize().y / GetLineHeight();
}
#endif 