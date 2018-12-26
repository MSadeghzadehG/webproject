
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_VALIDATORS

#include "wx/validate.h"

#ifndef WX_PRECOMP
    #include "wx/window.h"
#endif

const wxValidator wxDefaultValidator;

wxIMPLEMENT_DYNAMIC_CLASS(wxValidator, wxEvtHandler);

bool wxValidator::ms_isSilent = false;

wxValidator::wxValidator()
{
  m_validatorWindow = NULL;
}

wxValidator::~wxValidator()
{
}

#endif
  