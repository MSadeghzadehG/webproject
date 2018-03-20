


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/dialog.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/button.h"
    #include "wx/dcclient.h"
    #include "wx/intl.h"
    #include "wx/settings.h"
    #include "wx/stattext.h"
    #include "wx/sizer.h"
    #include "wx/containr.h"
#endif

#include "wx/statline.h"
#include "wx/sysopt.h"
#include "wx/module.h"
#include "wx/bookctrl.h"
#include "wx/scrolwin.h"
#include "wx/textwrapper.h"
#include "wx/modalhook.h"

#if wxUSE_DISPLAY
#include "wx/display.h"
#endif

extern WXDLLEXPORT_DATA(const char) wxDialogNameStr[] = "dialog";


wxDEFINE_FLAGS( wxDialogStyle )
wxBEGIN_FLAGS( wxDialogStyle )
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
wxFLAGS_MEMBER(wxNO_BORDER)

wxFLAGS_MEMBER(wxTAB_TRAVERSAL)
wxFLAGS_MEMBER(wxCLIP_CHILDREN)

wxFLAGS_MEMBER(wxSTAY_ON_TOP)
wxFLAGS_MEMBER(wxCAPTION)
wxFLAGS_MEMBER(wxSYSTEM_MENU)
wxFLAGS_MEMBER(wxRESIZE_BORDER)
wxFLAGS_MEMBER(wxCLOSE_BOX)
wxFLAGS_MEMBER(wxMAXIMIZE_BOX)
wxFLAGS_MEMBER(wxMINIMIZE_BOX)
wxEND_FLAGS( wxDialogStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxDialog, wxTopLevelWindow, "wx/dialog.h");

wxBEGIN_PROPERTIES_TABLE(wxDialog)
wxPROPERTY( Title, wxString, SetTitle, GetTitle, wxString(), \
           0 , wxT("Helpstring"), wxT("group"))

wxPROPERTY_FLAGS( WindowStyle, wxDialogStyle, long, SetWindowStyleFlag, \
                 GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 , \
                 wxT("Helpstring"), wxT("group")) wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxDialog)

wxCONSTRUCTOR_6( wxDialog, wxWindow*, Parent, wxWindowID, Id, \
                wxString, Title, wxPoint, Position, wxSize, Size, long, WindowStyle)


wxBEGIN_EVENT_TABLE(wxDialogBase, wxTopLevelWindow)
    EVT_BUTTON(wxID_ANY, wxDialogBase::OnButton)

    EVT_CLOSE(wxDialogBase::OnCloseWindow)

    EVT_CHAR_HOOK(wxDialogBase::OnCharHook)
wxEND_EVENT_TABLE()

wxDialogLayoutAdapter* wxDialogBase::sm_layoutAdapter = NULL;
bool wxDialogBase::sm_layoutAdaptation = false;

wxDialogBase::wxDialogBase()
{
    m_returnCode = 0;
    m_affirmativeId = wxID_OK;
    m_escapeId = wxID_ANY;
    m_layoutAdaptationLevel = 3;
    m_layoutAdaptationDone = FALSE;
    m_layoutAdaptationMode = wxDIALOG_ADAPTATION_MODE_DEFAULT;

                SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
}

wxWindow *wxDialogBase::CheckIfCanBeUsedAsParent(wxWindow *parent) const
{
    if ( !parent )
        return NULL;

    extern WXDLLIMPEXP_DATA_BASE(wxList) wxPendingDelete;
    if ( wxPendingDelete.Member(parent) || parent->IsBeingDeleted() )
    {
                        return NULL;
    }

    if ( parent->HasExtraStyle(wxWS_EX_TRANSIENT) )
    {
                        return NULL;
    }

    if ( !parent->IsShownOnScreen() )
    {
                return NULL;
    }

    if ( parent == this )
    {
                        return NULL;
    }

    return parent;
}

wxWindow *
wxDialogBase::GetParentForModalDialog(wxWindow *parent, long style) const
{
                if ( style & wxDIALOG_NO_PARENT )
        return NULL;

        if ( parent )
        parent = CheckIfCanBeUsedAsParent(wxGetTopLevelParent(parent));

        if ( !parent )
        parent = CheckIfCanBeUsedAsParent(
                    wxGetTopLevelParent(wxGetActiveWindow()));

        if ( !parent && wxTheApp )
        parent = CheckIfCanBeUsedAsParent(wxTheApp->GetTopWindow());

    return parent;
}

#if wxUSE_STATTEXT

wxSizer *wxDialogBase::CreateTextSizer(const wxString& message)
{
    wxTextSizerWrapper wrapper(this);

    return CreateTextSizer(message, wrapper);
}

wxSizer *wxDialogBase::CreateTextSizer(const wxString& message,
                                       wxTextSizerWrapper& wrapper)
{
            int widthMax = -1;
    const bool is_pda = wxSystemSettings::GetScreenType() <= wxSYS_SCREEN_PDA;
    if (is_pda)
    {
        widthMax = wxSystemSettings::GetMetric( wxSYS_SCREEN_X ) - 25;
    }

    return wrapper.CreateSizer(message, widthMax);
}

#endif 
wxSizer *wxDialogBase::CreateButtonSizer(long flags)
{
#if wxUSE_BUTTON

    return CreateStdDialogButtonSizer(flags);

#else     wxUnusedVar(flags);

    return NULL;
#endif }

wxSizer *wxDialogBase::CreateSeparatedSizer(wxSizer *sizer)
{
        #if wxUSE_STATLINE && !defined(__WXMAC__)
    wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
    topsizer->Add(new wxStaticLine(this),
                   wxSizerFlags().Expand().DoubleBorder(wxBOTTOM));
    topsizer->Add(sizer, wxSizerFlags().Expand());
    sizer = topsizer;
#endif 
    return sizer;
}

wxSizer *wxDialogBase::CreateSeparatedButtonSizer(long flags)
{
    wxSizer *sizer = CreateButtonSizer(flags);
    if ( !sizer )
        return NULL;

    return CreateSeparatedSizer(sizer);
}

#if wxUSE_BUTTON

wxStdDialogButtonSizer *wxDialogBase::CreateStdDialogButtonSizer( long flags )
{
    wxStdDialogButtonSizer *sizer = new wxStdDialogButtonSizer();

    wxButton *ok = NULL;
    wxButton *yes = NULL;
    wxButton *no = NULL;

    if (flags & wxOK)
    {
        ok = new wxButton(this, wxID_OK);
        sizer->AddButton(ok);
    }

    if (flags & wxCANCEL)
    {
        wxButton *cancel = new wxButton(this, wxID_CANCEL);
        sizer->AddButton(cancel);
    }

    if (flags & wxYES)
    {
        yes = new wxButton(this, wxID_YES);
        sizer->AddButton(yes);
    }

    if (flags & wxNO)
    {
        no = new wxButton(this, wxID_NO);
        sizer->AddButton(no);
    }

    if (flags & wxAPPLY)
    {
        wxButton *apply = new wxButton(this, wxID_APPLY);
        sizer->AddButton(apply);
    }

    if (flags & wxCLOSE)
    {
        wxButton *close = new wxButton(this, wxID_CLOSE);
        sizer->AddButton(close);
    }

    if (flags & wxHELP)
    {
        wxButton *help = new wxButton(this, wxID_HELP);
        sizer->AddButton(help);
    }

    if (flags & wxNO_DEFAULT)
    {
        if (no)
        {
            no->SetDefault();
            no->SetFocus();
        }
    }
    else
    {
        if (ok)
        {
            ok->SetDefault();
            ok->SetFocus();
        }
        else if (yes)
        {
            yes->SetDefault();
            yes->SetFocus();
        }
    }

    if (flags & wxOK)
        SetAffirmativeId(wxID_OK);
    else if (flags & wxYES)
        SetAffirmativeId(wxID_YES);
    else if (flags & wxCLOSE)
        SetAffirmativeId(wxID_CLOSE);

    sizer->Realize();

    return sizer;
}

#endif 

void wxDialogBase::EndDialog(int rc)
{
    if ( IsModal() )
        EndModal(rc);
    else
        Hide();
}

void wxDialogBase::AcceptAndClose()
{
    if ( Validate() && TransferDataFromWindow() )
    {
        EndDialog(m_affirmativeId);
    }
}

void wxDialogBase::SetAffirmativeId(int affirmativeId)
{
    m_affirmativeId = affirmativeId;
}

void wxDialogBase::SetEscapeId(int escapeId)
{
    m_escapeId = escapeId;
}

bool wxDialogBase::EmulateButtonClickIfPresent(int id)
{
#if wxUSE_BUTTON
    wxButton *btn = wxDynamicCast(FindWindow(id), wxButton);

    if ( !btn || !btn->IsEnabled() || !btn->IsShown() )
        return false;

    wxCommandEvent event(wxEVT_BUTTON, id);
    event.SetEventObject(btn);
    btn->GetEventHandler()->ProcessEvent(event);

    return true;
#else     wxUnusedVar(id);
    return false;
#endif }

bool wxDialogBase::SendCloseButtonClickEvent()
{
    int idCancel = GetEscapeId();
    switch ( idCancel )
    {
        case wxID_NONE:
                        break;

        case wxID_ANY:
                                    if ( EmulateButtonClickIfPresent(wxID_CANCEL) )
                return true;
            idCancel = GetAffirmativeId();
            wxFALLTHROUGH;

        default:
                        if ( EmulateButtonClickIfPresent(idCancel) )
                return true;
    }

    return false;
}

bool wxDialogBase::IsEscapeKey(const wxKeyEvent& event)
{
                    return event.GetKeyCode() == WXK_ESCAPE;
}

void wxDialogBase::OnCharHook(wxKeyEvent& event)
{
    if ( IsEscapeKey(event) )
    {
        if ( SendCloseButtonClickEvent() )
        {
                        return;
        }
    }

    event.Skip();
}

void wxDialogBase::OnButton(wxCommandEvent& event)
{
    const int id = event.GetId();
    if ( id == GetAffirmativeId() )
    {
        AcceptAndClose();
    }
    else if ( id == wxID_APPLY )
    {
        if ( Validate() )
            TransferDataFromWindow();

            }
    else if ( id == GetEscapeId() ||
                (id == wxID_CANCEL && GetEscapeId() == wxID_ANY) )
    {
        EndDialog(wxID_CANCEL);
    }
    else     {
        event.Skip();
    }
}


wxDEFINE_EVENT( wxEVT_WINDOW_MODAL_DIALOG_CLOSED , wxWindowModalDialogEvent  );

wxIMPLEMENT_DYNAMIC_CLASS(wxWindowModalDialogEvent, wxCommandEvent);

void wxDialogBase::ShowWindowModal ()
{
    int retval = ShowModal();
                        SetReturnCode(retval);
    SendWindowModalDialogEvent ( wxEVT_WINDOW_MODAL_DIALOG_CLOSED  );
}

void wxDialogBase::SendWindowModalDialogEvent ( wxEventType type )
{
    wxWindowModalDialogEvent event ( type, GetId());
    event.SetEventObject(this);

    if ( !GetEventHandler()->ProcessEvent(event) )
    {
                                        (void)GetParent()->GetEventHandler()->ProcessEvent(event);
    }
}


wxDialogModality wxDialogBase::GetModality() const
{
    return IsModal() ? wxDIALOG_MODALITY_APP_MODAL : wxDIALOG_MODALITY_NONE;
}


void wxDialogBase::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
    
                    static wxList closing;

    if ( closing.Member(this) )
        return;

    closing.Append(this);

                            if ( !IsShown() || !SendCloseButtonClickEvent() )
    {
                                                                EndDialog(wxID_CANCEL);
    }

    closing.DeleteObject(this);
}

void wxDialogBase::OnSysColourChanged(wxSysColourChangedEvent& event)
{
#ifndef __WXGTK__
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
    Refresh();
#endif

    event.Skip();
}

bool wxDialogBase::DoLayoutAdaptation()
{
    if (GetLayoutAdapter())
    {
        wxWindow* focusWindow = wxFindFocusDescendant(this);         if (GetLayoutAdapter()->DoLayoutAdaptation((wxDialog*) this))
        {
            if (focusWindow)
                focusWindow->SetFocus();
            return true;
        }
        else
            return false;
    }
    else
        return false;
}

bool wxDialogBase::CanDoLayoutAdaptation()
{
        bool layoutEnabled = (GetLayoutAdaptationMode() == wxDIALOG_ADAPTATION_MODE_ENABLED) || (IsLayoutAdaptationEnabled() && (GetLayoutAdaptationMode() != wxDIALOG_ADAPTATION_MODE_DISABLED));

    return (layoutEnabled && !m_layoutAdaptationDone && GetLayoutAdaptationLevel() != 0 && GetLayoutAdapter() != NULL && GetLayoutAdapter()->CanDoLayoutAdaptation((wxDialog*) this));
}

wxDialogLayoutAdapter* wxDialogBase::SetLayoutAdapter(wxDialogLayoutAdapter* adapter)
{
    wxDialogLayoutAdapter* oldLayoutAdapter = sm_layoutAdapter;
    sm_layoutAdapter = adapter;
    return oldLayoutAdapter;
}



wxIMPLEMENT_CLASS(wxDialogLayoutAdapter, wxObject);

wxIMPLEMENT_CLASS(wxStandardDialogLayoutAdapter, wxDialogLayoutAdapter);

#if defined(__WXGTK__) && !wxCHECK_VERSION(2,9,0)
#define wxEXTRA_DIALOG_HEIGHT 30
#else
#define wxEXTRA_DIALOG_HEIGHT 0
#endif

bool wxStandardDialogLayoutAdapter::CanDoLayoutAdaptation(wxDialog* dialog)
{
    if (dialog->GetSizer())
    {
        wxSize windowSize, displaySize;
        return MustScroll(dialog, windowSize, displaySize) != 0;
    }
    else
        return false;
}

bool wxStandardDialogLayoutAdapter::DoLayoutAdaptation(wxDialog* dialog)
{
    if (dialog->GetSizer())
    {
#if wxUSE_BOOKCTRL
        wxBookCtrlBase* bookContentWindow = wxDynamicCast(dialog->GetContentWindow(), wxBookCtrlBase);

        if (bookContentWindow)
        {
                        wxWindowList windows;
            for (size_t i = 0; i < bookContentWindow->GetPageCount(); i++)
            {
                wxWindow* page = bookContentWindow->GetPage(i);

                wxScrolledWindow* scrolledWindow = wxDynamicCast(page, wxScrolledWindow);
                if (scrolledWindow)
                    windows.Append(scrolledWindow);
                else if (page->GetSizer())
                {
                                        scrolledWindow = CreateScrolledWindow(page);
                    wxSizer* oldSizer = page->GetSizer();

                    wxSizer* newSizer = new wxBoxSizer(wxVERTICAL);
                    newSizer->Add(scrolledWindow,1, wxEXPAND, 0);

                    page->SetSizer(newSizer, false );

                    scrolledWindow->SetSizer(oldSizer);

                    ReparentControls(page, scrolledWindow);

                    windows.Append(scrolledWindow);
                }
            }

            FitWithScrolling(dialog, windows);
        }
        else
#endif         {
#if wxUSE_BUTTON
                                    wxScrolledWindow* scrolledWindow = CreateScrolledWindow(dialog);

            int buttonSizerBorder = 0;

                        wxSizer* buttonSizer = FindButtonSizer(true , dialog, dialog->GetSizer(), buttonSizerBorder);

                        if (!buttonSizer && dialog->GetLayoutAdaptationLevel() > wxDIALOG_ADAPTATION_STANDARD_SIZER)
                buttonSizer = FindButtonSizer(false , dialog, dialog->GetSizer(), buttonSizerBorder);

                        if (!buttonSizer && dialog->GetLayoutAdaptationLevel() > wxDIALOG_ADAPTATION_ANY_SIZER)
            {
                int count = 0;
                wxStdDialogButtonSizer* stdButtonSizer = new wxStdDialogButtonSizer;
                buttonSizer = stdButtonSizer;

                FindLooseButtons(dialog, stdButtonSizer, dialog->GetSizer(), count);
                if (count > 0)
                    stdButtonSizer->Realize();
                else
                {
                    wxDELETE(buttonSizer);
                }
            }

            if (buttonSizerBorder == 0)
                buttonSizerBorder = 5;

            ReparentControls(dialog, scrolledWindow, buttonSizer);

            wxBoxSizer* newTopSizer = new wxBoxSizer(wxVERTICAL);
            wxSizer* oldSizer = dialog->GetSizer();

            dialog->SetSizer(newTopSizer, false );

            newTopSizer->Add(scrolledWindow, 1, wxEXPAND|wxALL, 0);
            if (buttonSizer)
                newTopSizer->Add(buttonSizer, 0, wxEXPAND|wxALL, buttonSizerBorder);

            scrolledWindow->SetSizer(oldSizer);

            FitWithScrolling(dialog, scrolledWindow);
#endif         }
    }

    dialog->SetLayoutAdaptationDone(true);
    return true;
}

wxScrolledWindow* wxStandardDialogLayoutAdapter::CreateScrolledWindow(wxWindow* parent)
{
    wxScrolledWindow* scrolledWindow = new wxScrolledWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxVSCROLL|wxHSCROLL|wxBORDER_NONE);
    return scrolledWindow;
}

#if wxUSE_BUTTON

wxSizer* wxStandardDialogLayoutAdapter::FindButtonSizer(bool stdButtonSizer, wxDialog* dialog, wxSizer* sizer, int& retBorder, int accumlatedBorder)
{
    for ( wxSizerItemList::compatibility_iterator node = sizer->GetChildren().GetFirst();
          node; node = node->GetNext() )
    {
        wxSizerItem *item = node->GetData();
        wxSizer *childSizer = item->GetSizer();

        if ( childSizer )
        {
            int newBorder = accumlatedBorder;
            if (item->GetFlag() & wxALL)
                newBorder += item->GetBorder();

            if (stdButtonSizer)             {
                wxStdDialogButtonSizer* buttonSizer = wxDynamicCast(childSizer, wxStdDialogButtonSizer);
                if (buttonSizer)
                {
                    sizer->Detach(childSizer);
                    retBorder = newBorder;
                    return buttonSizer;
                }
            }
            else             {
                wxBoxSizer* buttonSizer = wxDynamicCast(childSizer, wxBoxSizer);
                if (buttonSizer && IsOrdinaryButtonSizer(dialog, buttonSizer))
                {
                    sizer->Detach(childSizer);
                    retBorder = newBorder;
                    return buttonSizer;
                }
            }

            wxSizer* s = FindButtonSizer(stdButtonSizer, dialog, childSizer, retBorder, newBorder);
            if (s)
                return s;
        }
    }
    return NULL;
}

bool wxStandardDialogLayoutAdapter::IsOrdinaryButtonSizer(wxDialog* dialog, wxBoxSizer* sizer)
{
    if (sizer->GetOrientation() != wxHORIZONTAL)
        return false;

    for ( wxSizerItemList::compatibility_iterator node = sizer->GetChildren().GetFirst();
          node; node = node->GetNext() )
    {
        wxSizerItem *item = node->GetData();
        wxButton *childButton = wxDynamicCast(item->GetWindow(), wxButton);

        if (childButton && IsStandardButton(dialog, childButton))
            return true;
    }
    return false;
}

bool wxStandardDialogLayoutAdapter::IsStandardButton(wxDialog* dialog, wxButton* button)
{
    wxWindowID id = button->GetId();

    return (id == wxID_OK || id == wxID_CANCEL || id == wxID_YES || id == wxID_NO || id == wxID_SAVE ||
            id == wxID_APPLY || id == wxID_HELP || id == wxID_CONTEXT_HELP || dialog->IsMainButtonId(id));
}

bool wxStandardDialogLayoutAdapter::FindLooseButtons(wxDialog* dialog, wxStdDialogButtonSizer* buttonSizer, wxSizer* sizer, int& count)
{
    wxSizerItemList::compatibility_iterator node = sizer->GetChildren().GetFirst();
    while (node)
    {
        wxSizerItemList::compatibility_iterator next = node->GetNext();
        wxSizerItem *item = node->GetData();
        wxSizer *childSizer = item->GetSizer();
        wxButton *childButton = wxDynamicCast(item->GetWindow(), wxButton);

        if (childButton && IsStandardButton(dialog, childButton))
        {
            sizer->Detach(childButton);
            buttonSizer->AddButton(childButton);
            count ++;
        }

        if (childSizer)
            FindLooseButtons(dialog, buttonSizer, childSizer, count);

        node = next;
    }
    return true;
}

#endif 
void wxStandardDialogLayoutAdapter::ReparentControls(wxWindow* parent, wxWindow* reparentTo, wxSizer* buttonSizer)
{
    DoReparentControls(parent, reparentTo, buttonSizer);
}

void wxStandardDialogLayoutAdapter::DoReparentControls(wxWindow* parent, wxWindow* reparentTo, wxSizer* buttonSizer)
{
    wxWindowList::compatibility_iterator node = parent->GetChildren().GetFirst();
    while (node)
    {
        wxWindowList::compatibility_iterator next = node->GetNext();

        wxWindow *win = node->GetData();

                if (win != reparentTo && (!buttonSizer || !buttonSizer->GetItem(win)))
        {
            win->Reparent(reparentTo);
#ifdef __WXMSW__
                        ::SetWindowPos((HWND) win->GetHWND(), HWND_BOTTOM, -1, -1, -1, -1, SWP_NOMOVE|SWP_NOSIZE);
#endif
        }

        node = next;
    }
}

int wxStandardDialogLayoutAdapter::MustScroll(wxDialog* dialog, wxSize& windowSize, wxSize& displaySize)
{
    return DoMustScroll(dialog, windowSize, displaySize);
}

int wxStandardDialogLayoutAdapter::DoMustScroll(wxDialog* dialog, wxSize& windowSize, wxSize& displaySize)
{
    wxSize minWindowSize = dialog->GetSizer()->GetMinSize();
    windowSize = dialog->GetSize();
    windowSize = wxSize(wxMax(windowSize.x, minWindowSize.x), wxMax(windowSize.y, minWindowSize.y));
#if wxUSE_DISPLAY
    displaySize = wxDisplay(wxDisplay::GetFromWindow(dialog)).GetClientArea().GetSize();
#else
    displaySize = wxGetClientDisplayRect().GetSize();
#endif

    int flags = 0;

    if (windowSize.y >= (displaySize.y - wxEXTRA_DIALOG_HEIGHT))
        flags |= wxVERTICAL;
    if (windowSize.x >= displaySize.x)
        flags |= wxHORIZONTAL;

    return flags;
}

bool wxStandardDialogLayoutAdapter::FitWithScrolling(wxDialog* dialog, wxWindowList& windows)
{
    return DoFitWithScrolling(dialog, windows);
}

bool wxStandardDialogLayoutAdapter::FitWithScrolling(wxDialog* dialog, wxScrolledWindow* scrolledWindow)
{
    return DoFitWithScrolling(dialog, scrolledWindow);
}

bool wxStandardDialogLayoutAdapter::DoFitWithScrolling(wxDialog* dialog, wxScrolledWindow* scrolledWindow)
{
    wxWindowList windows;
    windows.Append(scrolledWindow);
    return DoFitWithScrolling(dialog, windows);
}

bool wxStandardDialogLayoutAdapter::DoFitWithScrolling(wxDialog* dialog, wxWindowList& windows)
{
    wxSizer* sizer = dialog->GetSizer();
    if (!sizer)
        return false;

    sizer->SetSizeHints(dialog);

    wxSize windowSize, displaySize;
    int scrollFlags = DoMustScroll(dialog, windowSize, displaySize);
    int scrollBarSize = 20;

    if (scrollFlags)
    {
        int scrollBarExtraX = 0, scrollBarExtraY = 0;
        bool resizeHorizontally = (scrollFlags & wxHORIZONTAL) != 0;
        bool resizeVertically = (scrollFlags & wxVERTICAL) != 0;

        if (windows.GetCount() != 0)
        {
                        if ((resizeVertically && !resizeHorizontally) && (windowSize.x < (displaySize.x - scrollBarSize)))
                scrollBarExtraX = scrollBarSize;
            if ((resizeHorizontally && !resizeVertically) && (windowSize.y < (displaySize.y - scrollBarSize)))
                scrollBarExtraY = scrollBarSize;
        }

        wxWindowList::compatibility_iterator node = windows.GetFirst();
        while (node)
        {
            wxWindow *win = node->GetData();
            wxScrolledWindow* scrolledWindow = wxDynamicCast(win, wxScrolledWindow);
            if (scrolledWindow)
            {
                scrolledWindow->SetScrollRate(resizeHorizontally ? 10 : 0, resizeVertically ? 10 : 0);

                if (scrolledWindow->GetSizer())
                    scrolledWindow->GetSizer()->Fit(scrolledWindow);
            }

            node = node->GetNext();
        }

        wxSize limitTo = windowSize + wxSize(scrollBarExtraX, scrollBarExtraY);
        if (resizeVertically)
            limitTo.y = displaySize.y - wxEXTRA_DIALOG_HEIGHT;
        if (resizeHorizontally)
            limitTo.x = displaySize.x;

        dialog->SetMinSize(limitTo);
        dialog->SetSize(limitTo);

        dialog->SetSizeHints( limitTo.x, limitTo.y, dialog->GetMaxWidth(), dialog->GetMaxHeight() );
    }

    return true;
}



class wxDialogLayoutAdapterModule: public wxModule
{
    wxDECLARE_DYNAMIC_CLASS(wxDialogLayoutAdapterModule);
public:
    wxDialogLayoutAdapterModule() {}
    virtual void OnExit() wxOVERRIDE { delete wxDialogBase::SetLayoutAdapter(NULL); }
    virtual bool OnInit() wxOVERRIDE { wxDialogBase::SetLayoutAdapter(new wxStandardDialogLayoutAdapter); return true; }
};

wxIMPLEMENT_DYNAMIC_CLASS(wxDialogLayoutAdapterModule, wxModule);
