


#include "wx/wxprec.h"


#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_STATLINE

#include "wx/statline.h"

#ifndef WX_PRECOMP
    #include "wx/statbox.h"
#endif



bool wxStaticLine::Create( wxWindow *parent,
                           wxWindowID id,
                           const wxPoint &pos,
                           const wxSize &size,
                           long style,
                           const wxString &name)
{
    m_statbox = NULL;

    if ( !CreateBase(parent, id, pos, size, style, wxDefaultValidator, name) )
        return false;

        
    wxSize sizeReal = AdjustSize(size);

    m_statbox = new wxStaticBox(parent, id, wxEmptyString, pos, sizeReal, style, name);

    return true;
}

wxStaticLine::~wxStaticLine()
{
    delete m_statbox;
}

WXWidget wxStaticLine::GetMainWidget() const
{
    return m_statbox->GetMainWidget();
}

void wxStaticLine::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
    m_statbox->SetSize(x, y, width, height, sizeFlags);
}

void wxStaticLine::DoMoveWindow(int x, int y, int width, int height)
{
    m_statbox->SetSize(x, y, width, height);
}

#endif
  