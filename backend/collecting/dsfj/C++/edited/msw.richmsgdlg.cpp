
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#if wxUSE_RICHMSGDLG

#include "wx/richmsgdlg.h"
#include "wx/modalhook.h"

#ifndef WX_PRECOMP
    #include "wx/msw/private.h"
#endif

#include "wx/msw/private/msgdlg.h"


int wxRichMessageDialog::ShowModal()
{
    WX_HOOK_MODAL_DIALOG();

#ifdef wxHAS_MSW_TASKDIALOG
    using namespace wxMSWMessageDialog;

    if ( HasNativeTaskDialog() )
    {
                WinStruct<TASKDIALOGCONFIG> tdc;
        wxMSWTaskDialogConfig wxTdc(*this);

        wxTdc.MSWCommonTaskDialogInit( tdc );

                if ( !m_checkBoxText.empty() )
        {
            tdc.pszVerificationText = m_checkBoxText.t_str();
            if ( m_checkBoxValue )
                tdc.dwFlags |= TDF_VERIFICATION_FLAG_CHECKED;
        }

                if ( !m_detailedText.empty() )
            tdc.pszExpandedInformation = m_detailedText.t_str();

        TaskDialogIndirect_t taskDialogIndirect = GetTaskDialogIndirectFunc();
        if ( !taskDialogIndirect )
            return wxID_CANCEL;

                BOOL checkBoxChecked;
        int msAns;
        HRESULT hr = taskDialogIndirect( &tdc, &msAns, NULL, &checkBoxChecked );
        if ( FAILED(hr) )
        {
            wxLogApiError( "TaskDialogIndirect", hr );
            return wxID_CANCEL;
        }
        m_checkBoxValue = checkBoxChecked != FALSE;

                                        if ( (msAns == IDCANCEL)
            && !(GetMessageDialogStyle() & (wxYES_NO|wxCANCEL)) )
        {
            msAns = IDOK;
        }

        return MSWTranslateReturnCode( msAns );
    }
#endif 
            return wxGenericRichMessageDialog::ShowModal();
}

#endif 