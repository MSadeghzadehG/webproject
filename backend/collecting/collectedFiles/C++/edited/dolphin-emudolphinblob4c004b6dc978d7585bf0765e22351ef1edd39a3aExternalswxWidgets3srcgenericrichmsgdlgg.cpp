
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#if wxUSE_RICHMSGDLG

#ifndef WX_PRECOMP
    #include "wx/checkbox.h"
    #include "wx/stattext.h"
    #include "wx/sizer.h"
#endif

#include "wx/collpane.h"
#include "wx/richmsgdlg.h"

wxIMPLEMENT_CLASS(wxRichMessageDialog, wxDialog)


wxBEGIN_EVENT_TABLE(wxGenericRichMessageDialog, wxRichMessageDialogBase)
    EVT_COLLAPSIBLEPANE_CHANGED(wxID_ANY,
                                wxGenericRichMessageDialog::OnPaneChanged)
wxEND_EVENT_TABLE()

void wxGenericRichMessageDialog::OnPaneChanged(wxCollapsiblePaneEvent& event)
{
    if ( event.GetCollapsed() )
        m_detailsPane->SetLabel( m_detailsExpanderCollapsedLabel );
    else
        m_detailsPane->SetLabel( m_detailsExpanderExpandedLabel );
}


void wxGenericRichMessageDialog::AddMessageDialogCheckBox(wxSizer *sizer)
{
    if ( !m_checkBoxText.empty() )
    {
        m_checkBox = new wxCheckBox(this, wxID_ANY, m_checkBoxText);
        m_checkBox->SetValue(m_checkBoxValue);

        sizer->Add(m_checkBox, wxSizerFlags().Left().Border(wxLEFT|wxTOP, 10));
    }
}

void wxGenericRichMessageDialog::AddMessageDialogDetails(wxSizer *sizer)
{
    if ( !m_detailedText.empty() )
    {
        wxSizer *sizerDetails = new wxBoxSizer( wxHORIZONTAL );

        m_detailsPane =
            new wxCollapsiblePane( this, -1, m_detailsExpanderCollapsedLabel );

                wxWindow *windowPane = m_detailsPane->GetPane();
        wxSizer *sizerPane = new wxBoxSizer( wxHORIZONTAL );
        sizerPane->Add( new wxStaticText( windowPane, -1, m_detailedText ) );
        windowPane->SetSizer( sizerPane );

        sizerDetails->Add( m_detailsPane, wxSizerFlags().Right().Expand() );
        sizer->Add( sizerDetails, 0, wxTOP|wxLEFT|wxRIGHT | wxALIGN_LEFT, 10 );
    }
}

bool wxGenericRichMessageDialog::IsCheckBoxChecked() const
{
            return m_checkBox ? m_checkBox->IsChecked() : m_checkBoxValue;
}

#endif 