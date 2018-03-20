


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_REARRANGECTRL

#ifndef WX_PRECOMP
    #include "wx/button.h"
    #include "wx/stattext.h"
    #include "wx/sizer.h"
#endif 
#include "wx/rearrangectrl.h"


extern
WXDLLIMPEXP_DATA_CORE(const char) wxRearrangeListNameStr[] = "wxRearrangeList";

wxBEGIN_EVENT_TABLE(wxRearrangeList, wxCheckListBox)
    EVT_CHECKLISTBOX(wxID_ANY, wxRearrangeList::OnCheck)
wxEND_EVENT_TABLE()

bool wxRearrangeList::Create(wxWindow *parent,
                             wxWindowID id,
                             const wxPoint& pos,
                             const wxSize& size,
                             const wxArrayInt& order,
                             const wxArrayString& items,
                             long style,
                             const wxValidator& validator,
                             const wxString& name)
{
            const size_t count = items.size();
    wxCHECK_MSG( order.size() == count, false, "arrays not in sync" );

    wxArrayString itemsInOrder;
    itemsInOrder.reserve(count);
    size_t n;
    for ( n = 0; n < count; n++ )
    {
        int idx = order[n];
        if ( idx < 0 )
            idx = -idx - 1;
        itemsInOrder.push_back(items[idx]);
    }

        if ( !wxCheckListBox::Create(parent, id, pos, size, itemsInOrder,
                                 style, validator, name) )
        return false;

        for ( n = 0; n < count; n++ )
    {
        if ( order[n] >= 0 )
        {
                                    wxCheckListBox::Check(n);
        }
    }

    m_order = order;

    return true;
}

bool wxRearrangeList::CanMoveCurrentUp() const
{
    const int sel = GetSelection();
    return sel != wxNOT_FOUND && sel != 0;
}

bool wxRearrangeList::CanMoveCurrentDown() const
{
    const int sel = GetSelection();
    return sel != wxNOT_FOUND && static_cast<unsigned>(sel) != GetCount() - 1;
}

bool wxRearrangeList::MoveCurrentUp()
{
    const int sel = GetSelection();
    if ( sel == wxNOT_FOUND || sel == 0 )
        return false;

    Swap(sel, sel - 1);
    SetSelection(sel - 1);

    return true;
}

bool wxRearrangeList::MoveCurrentDown()
{
    const int sel = GetSelection();
    if ( sel == wxNOT_FOUND || static_cast<unsigned>(sel) == GetCount() - 1 )
        return false;

    Swap(sel, sel + 1);
    SetSelection(sel + 1);

    return true;
}

void wxRearrangeList::Swap(int pos1, int pos2)
{
        wxSwap(m_order[pos1], m_order[pos2]);


    
        const wxString stringTmp = GetString(pos1);
    SetString(pos1, GetString(pos2));
    SetString(pos2, stringTmp);

        const bool checkedTmp = IsChecked(pos1);
    wxCheckListBox::Check(pos1, IsChecked(pos2));
    wxCheckListBox::Check(pos2, checkedTmp);

        switch ( GetClientDataType() )
    {
        case wxClientData_None:
                        break;

        case wxClientData_Object:
            {
                wxClientData * const dataTmp = DetachClientObject(pos1);
                SetClientObject(pos1, DetachClientObject(pos2));
                SetClientObject(pos2, dataTmp);
            }
            break;

        case wxClientData_Void:
            {
                void * const dataTmp = GetClientData(pos1);
                SetClientData(pos1, GetClientData(pos2));
                SetClientData(pos2, dataTmp);
            }
            break;
    }
}

void wxRearrangeList::Check(unsigned int item, bool check)
{
    if ( check == IsChecked(item) )
        return;

    wxCheckListBox::Check(item, check);

    m_order[item] = ~m_order[item];
}

void wxRearrangeList::OnCheck(wxCommandEvent& event)
{
        const int n = event.GetInt();

    if ( (m_order[n] >= 0) != IsChecked(n) )
        m_order[n] = ~m_order[n];
}


wxBEGIN_EVENT_TABLE(wxRearrangeCtrl, wxPanel)
    EVT_UPDATE_UI(wxID_UP, wxRearrangeCtrl::OnUpdateButtonUI)
    EVT_UPDATE_UI(wxID_DOWN, wxRearrangeCtrl::OnUpdateButtonUI)

    EVT_BUTTON(wxID_UP, wxRearrangeCtrl::OnButton)
    EVT_BUTTON(wxID_DOWN, wxRearrangeCtrl::OnButton)
wxEND_EVENT_TABLE()

void wxRearrangeCtrl::Init()
{
    m_list = NULL;
}

bool
wxRearrangeCtrl::Create(wxWindow *parent,
                        wxWindowID id,
                        const wxPoint& pos,
                        const wxSize& size,
                        const wxArrayInt& order,
                        const wxArrayString& items,
                        long style,
                        const wxValidator& validator,
                        const wxString& name)
{
        if ( !wxPanel::Create(parent, id, pos, size, wxTAB_TRAVERSAL, name) )
        return false;

    m_list = new wxRearrangeList(this, wxID_ANY,
                                 wxDefaultPosition, wxDefaultSize,
                                 order, items,
                                 style, validator);
    wxButton * const btnUp = new wxButton(this, wxID_UP);
    wxButton * const btnDown = new wxButton(this, wxID_DOWN);

        wxSizer * const sizerBtns = new wxBoxSizer(wxVERTICAL);
    sizerBtns->Add(btnUp, wxSizerFlags().Centre().Border(wxBOTTOM));
    sizerBtns->Add(btnDown, wxSizerFlags().Centre().Border(wxTOP));

    wxSizer * const sizerTop = new wxBoxSizer(wxHORIZONTAL);
    sizerTop->Add(m_list, wxSizerFlags(1).Expand().Border(wxRIGHT));
    sizerTop->Add(sizerBtns, wxSizerFlags(0).Centre().Border(wxLEFT));
    SetSizer(sizerTop);

    m_list->SetFocus();

    return true;
}

void wxRearrangeCtrl::OnUpdateButtonUI(wxUpdateUIEvent& event)
{
    event.Enable( event.GetId() == wxID_UP ? m_list->CanMoveCurrentUp()
                                           : m_list->CanMoveCurrentDown() );
}

void wxRearrangeCtrl::OnButton(wxCommandEvent& event)
{
    if ( event.GetId() == wxID_UP )
        m_list->MoveCurrentUp();
    else
        m_list->MoveCurrentDown();
}


extern
WXDLLIMPEXP_DATA_CORE(const char) wxRearrangeDialogNameStr[] = "wxRearrangeDlg";

namespace
{

enum wxRearrangeDialogSizerPositions
{
    Pos_Label,
    Pos_Ctrl,
    Pos_Buttons,
    Pos_Max
};

} 
bool wxRearrangeDialog::Create(wxWindow *parent,
                               const wxString& message,
                               const wxString& title,
                               const wxArrayInt& order,
                               const wxArrayString& items,
                               const wxPoint& pos,
                               const wxString& name)
{
    if ( !wxDialog::Create(parent, wxID_ANY, title,
                           pos, wxDefaultSize,
                           wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
                           name) )
        return false;

    m_ctrl = new wxRearrangeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                 order, items);

            wxSizer * const sizerTop = new wxBoxSizer(wxVERTICAL);

    if ( !message.empty() )
    {
        sizerTop->Add(new wxStaticText(this, wxID_ANY, message),
                      wxSizerFlags().Border());
    }
    else
    {
                        sizerTop->AddSpacer(0);
    }

    sizerTop->Add(m_ctrl,
                  wxSizerFlags(1).Expand().Border());
    sizerTop->Add(CreateSeparatedButtonSizer(wxOK | wxCANCEL),
                  wxSizerFlags().Expand().Border());
    SetSizerAndFit(sizerTop);

    return true;
}

void wxRearrangeDialog::AddExtraControls(wxWindow *win)
{
    wxSizer * const sizer = GetSizer();
    wxCHECK_RET( sizer, "the dialog must be created first" );

    wxASSERT_MSG( sizer->GetChildren().GetCount() == Pos_Max,
                  "calling AddExtraControls() twice?" );

    sizer->Insert(Pos_Buttons, win, wxSizerFlags().Expand().Border());

    win->MoveAfterInTabOrder(m_ctrl);

        sizer->SetSizeHints(this);
}

wxRearrangeList *wxRearrangeDialog::GetList() const
{
    wxCHECK_MSG( m_ctrl, NULL, "the dialog must be created first" );

    return m_ctrl->GetList();
}

wxArrayInt wxRearrangeDialog::GetOrder() const
{
    wxCHECK_MSG( m_ctrl, wxArrayInt(), "the dialog must be created first" );

    return m_ctrl->GetList()->GetCurrentOrder();
}

#endif 