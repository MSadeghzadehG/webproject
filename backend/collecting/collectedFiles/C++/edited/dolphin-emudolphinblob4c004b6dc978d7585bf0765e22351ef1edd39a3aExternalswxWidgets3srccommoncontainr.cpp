


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/containr.h"
#endif

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/event.h"
    #include "wx/window.h"
    #include "wx/scrolbar.h"
    #include "wx/radiobut.h"
#endif 
#define TRACE_FOCUS wxT("focus")



void wxControlContainerBase::UpdateParentCanFocus()
{
                m_winParent->SetCanFocus(m_acceptsFocusSelf && !m_acceptsFocusChildren);
}

bool wxControlContainerBase::UpdateCanFocusChildren()
{
    const bool acceptsFocusChildren = HasAnyFocusableChildren();
    if ( acceptsFocusChildren != m_acceptsFocusChildren )
    {
        m_acceptsFocusChildren = acceptsFocusChildren;

        UpdateParentCanFocus();
    }

    return m_acceptsFocusChildren;
}

bool wxControlContainerBase::HasAnyFocusableChildren() const
{
    const wxWindowList& children = m_winParent->GetChildren();
    for ( wxWindowList::const_iterator i = children.begin(),
                                     end = children.end();
          i != end;
          ++i )
    {
        const wxWindow * const child = *i;

        if ( !m_winParent->IsClientAreaChild(child) )
            continue;

                        if ( child->AcceptsFocusRecursively() )
            return true;
    }

    return false;
}

bool wxControlContainerBase::HasAnyChildrenAcceptingFocus() const
{
    const wxWindowList& children = m_winParent->GetChildren();
    for ( wxWindowList::const_iterator i = children.begin(),
                                     end = children.end();
          i != end;
          ++i )
    {
        const wxWindow * const child = *i;

        if ( !m_winParent->IsClientAreaChild(child) )
            continue;

                        if ( child->CanAcceptFocus() )
            return true;
    }

    return false;
}

bool wxControlContainerBase::DoSetFocus()
{
    wxLogTrace(TRACE_FOCUS, wxT("SetFocus on wxPanel 0x%p."),
               m_winParent->GetHandle());

    if (m_inSetFocus)
        return true;

            
    wxWindow *win = wxWindow::FindFocus();
    while ( win )
    {
        if ( win == m_winParent )
        {
                        return true;
        }

        if ( win->IsTopLevel() )
        {
                                    break;
        }

        win = win->GetParent();
    }

        m_inSetFocus = true;

    bool ret = SetFocusToChild();

    m_inSetFocus = false;

    return ret;
}

bool wxControlContainerBase::AcceptsFocus() const
{
    return m_acceptsFocusSelf && m_winParent->CanBeFocused();
}

bool wxControlContainerBase::SetFocusToChild()
{
    return wxSetFocusToChild(m_winParent, &m_winLastFocused);
}

#ifndef wxHAS_NATIVE_TAB_TRAVERSAL


wxControlContainer::wxControlContainer()
{
    m_winLastFocused = NULL;
}

void wxControlContainer::SetLastFocus(wxWindow *win)
{
                if ( win != m_winParent )
    {
                if ( win )
        {
                        wxWindow *winParent = win;
            while ( winParent != m_winParent )
            {
                win = winParent;
                winParent = win->GetParent();

                                                                
                wxASSERT_MSG( winParent,
                              wxT("Setting last focus for a window that is not our child?") );
            }
        }

        m_winLastFocused = win;

        if ( win )
        {
            wxLogTrace(TRACE_FOCUS, wxT("Set last focus to %s(%s)"),
                       win->GetClassInfo()->GetClassName(),
                       win->GetLabel().c_str());
        }
        else
        {
            wxLogTrace(TRACE_FOCUS, wxT("No more last focus"));
        }
    }
}


#if wxUSE_RADIOBTN 

wxRadioButton* wxGetPreviousButtonInGroup(wxRadioButton *btn)
{
    if ( btn->HasFlag(wxRB_GROUP) || btn->HasFlag(wxRB_SINGLE) )
        return NULL;

    const wxWindowList& siblings = btn->GetParent()->GetChildren();
    wxWindowList::compatibility_iterator nodeThis = siblings.Find(btn);
    wxCHECK_MSG( nodeThis, NULL, wxT("radio button not a child of its parent?") );

        wxWindowList::compatibility_iterator nodeBefore = nodeThis->GetPrevious();
    wxRadioButton *prevBtn = 0;
    while (nodeBefore)
    {
        prevBtn = wxDynamicCast(nodeBefore->GetData(), wxRadioButton);
        if (prevBtn)
            break;

        nodeBefore = nodeBefore->GetPrevious();
    }

    if (!prevBtn || prevBtn->HasFlag(wxRB_SINGLE))
    {
                return NULL;
    }

    return prevBtn;
}

wxRadioButton* wxGetNextButtonInGroup(wxRadioButton *btn)
{
    if (btn->HasFlag(wxRB_SINGLE))
        return NULL;

    const wxWindowList& siblings = btn->GetParent()->GetChildren();
    wxWindowList::compatibility_iterator nodeThis = siblings.Find(btn);
    wxCHECK_MSG( nodeThis, NULL, wxT("radio button not a child of its parent?") );

        wxWindowList::compatibility_iterator nodeNext = nodeThis->GetNext();
    wxRadioButton *nextBtn = 0;
    while (nodeNext)
    {
        nextBtn = wxDynamicCast(nodeNext->GetData(), wxRadioButton);
        if (nextBtn)
            break;

        nodeNext = nodeNext->GetNext();
    }

    if ( !nextBtn || nextBtn->HasFlag(wxRB_GROUP) || nextBtn->HasFlag(wxRB_SINGLE) )
    {
                return NULL;
    }

    return nextBtn;
}

wxRadioButton* wxGetFirstButtonInGroup(wxRadioButton *btn)
{
    while (true)
    {
        wxRadioButton* prevBtn = wxGetPreviousButtonInGroup(btn);
        if (!prevBtn)
            return btn;

        btn = prevBtn;
    }
}

wxRadioButton* wxGetLastButtonInGroup(wxRadioButton *btn)
{
    while (true)
    {
        wxRadioButton* nextBtn = wxGetNextButtonInGroup(btn);
        if (!nextBtn)
            return btn;

        btn = nextBtn;
    }
}

wxRadioButton* wxGetSelectedButtonInGroup(wxRadioButton *btn)
{
        if (btn->GetValue())
        return btn;

    if (btn->HasFlag(wxRB_SINGLE))
        return NULL;

    wxRadioButton *selBtn;

        for (selBtn = wxGetPreviousButtonInGroup(btn); selBtn; selBtn = wxGetPreviousButtonInGroup(selBtn))
        if (selBtn->GetValue())
            return selBtn;

        for (selBtn = wxGetNextButtonInGroup(btn); selBtn; selBtn = wxGetNextButtonInGroup(selBtn))
        if (selBtn->GetValue())
            return selBtn;

    return NULL;
}

#endif 

void wxControlContainer::HandleOnNavigationKey( wxNavigationKeyEvent& event )
{
            wxWindow *parent = m_winParent->IsTopLevel() ? NULL
                                                 : m_winParent->GetParent();

        bool goingDown = event.GetEventObject() == parent;

    const wxWindowList& children = m_winParent->GetChildren();

                                if ( event.IsWindowChange() && !goingDown )
    {
                wxWindow *bookctrl = NULL;
        for ( wxWindowList::const_iterator i = children.begin(),
                                         end = children.end();
              i != end;
              ++i )
        {
            wxWindow * const window = *i;
            if ( window->HasMultiplePages() )
            {
                if ( bookctrl )
                {
                                                                                bookctrl = NULL;
                    break;
                }

                bookctrl = window;
            }
        }

        if ( bookctrl )
        {
                                    wxNavigationKeyEvent eventCopy(event);
            eventCopy.SetEventObject(m_winParent);
            if ( bookctrl->GetEventHandler()->ProcessEvent(eventCopy) )
                return;
        }
    }

            if ( !children.GetCount() || event.IsWindowChange() )
    {
                        if ( goingDown ||
             !parent || !parent->GetEventHandler()->ProcessEvent(event) )
        {
            event.Skip();
        }

        return;
    }

        const bool forward = event.GetDirection();

            wxWindowList::compatibility_iterator node, start_node;

                if ( goingDown )
    {
                        m_winLastFocused = NULL;

                node = forward ? children.GetFirst() : children.GetLast();
    }
    else     {
        
                wxWindow *winFocus = event.GetCurrentFocus();

                if (!winFocus)
            winFocus = m_winLastFocused;

                if (!winFocus)
            winFocus = wxWindow::FindFocus();

        if ( winFocus )
        {
#if defined(__WXMSW__) && wxUSE_RADIOBTN
                                    if ( event.IsFromTab() && wxIsKindOf(winFocus, wxRadioButton ) )
                winFocus = wxGetFirstButtonInGroup((wxRadioButton*)winFocus);
#endif                         start_node = children.Find( winFocus );
        }

        if ( !start_node && m_winLastFocused )
        {
                                    start_node = children.Find( m_winLastFocused );
        }

                if ( !start_node )
        {
            start_node = children.GetFirst();
        }

                        node = forward ? start_node->GetNext() : start_node->GetPrevious();
    }

        for ( ;; )
    {
                if ( start_node && node && node == start_node )
            break;

                if ( !node )
        {
            if ( !start_node )
            {
                                break;
            }

            if ( !goingDown )
            {
                                                                                                                wxWindow *focusedParent = m_winParent;
                while ( parent )
                {
                                                                                                    if ( focusedParent->IsTopNavigationDomain(wxWindow::Navigation_Tab) )
                        break;

                    event.SetCurrentFocus( focusedParent );
                    if ( parent->GetEventHandler()->ProcessEvent( event ) )
                        return;

                    focusedParent = parent;

                    parent = parent->GetParent();
                }
            }
                        
                        node = forward ? children.GetFirst() : children.GetLast();

            continue;
        }

        wxWindow *child = node->GetData();

                if ( child->IsTopLevel() )
        {
            node = forward ? node->GetNext() : node->GetPrevious();

            continue;
        }

#if defined(__WXMSW__) && wxUSE_RADIOBTN
        if ( event.IsFromTab() )
        {
            if ( wxIsKindOf(child, wxRadioButton) )
            {
                                                if ( child->HasFlag(wxRB_GROUP) )
                {
                                        wxRadioButton *rb = wxGetSelectedButtonInGroup((wxRadioButton*)child);
                    if ( rb )
                        child = rb;
                }
                else if ( !child->HasFlag(wxRB_SINGLE) )
                {
                    node = forward ? node->GetNext() : node->GetPrevious();
                    continue;
                }
            }
        }
        else if ( m_winLastFocused &&
                  wxIsKindOf(m_winLastFocused, wxRadioButton) &&
                  !m_winLastFocused->HasFlag(wxRB_SINGLE) )
        {
            wxRadioButton * const
                lastBtn = static_cast<wxRadioButton *>(m_winLastFocused);

                                    if ( forward )
            {
                child = wxGetNextButtonInGroup(lastBtn);
                if ( !child )
                {
                                        child = wxGetFirstButtonInGroup(lastBtn);
                }
            }
            else
            {
                child = wxGetPreviousButtonInGroup(lastBtn);
                if ( !child )
                {
                                        child = wxGetLastButtonInGroup(lastBtn);
                }
            }

            if ( child == m_winLastFocused )
            {
                                                event.Skip(false);
                return;
            }
        }
#endif 
        if ( child->CanAcceptFocusFromKeyboard() )
        {
                                                            event.SetEventObject(m_winParent);

                                    wxPropagationDisabler disableProp(event);
            if ( !child->GetEventHandler()->ProcessEvent(event) )
            {
                                                m_winLastFocused = child;

                                child->SetFocusFromKbd();
            }
            
            event.Skip( false );

            return;
        }

        node = forward ? node->GetNext() : node->GetPrevious();
    }

            event.Skip();
}

void wxControlContainer::HandleOnWindowDestroy(wxWindowBase *child)
{
    if ( child == m_winLastFocused )
        m_winLastFocused = NULL;
}


void wxControlContainer::HandleOnFocus(wxFocusEvent& event)
{
    wxLogTrace(TRACE_FOCUS, wxT("OnFocus on wxPanel 0x%p, name: %s"),
               m_winParent->GetHandle(),
               m_winParent->GetName().c_str() );

    DoSetFocus();

    event.Skip();
}


#else
  
bool wxControlContainer::SetFocusToChild()
{
    return wxSetFocusToChild(m_winParent, NULL);
}


#endif 

bool wxSetFocusToChild(wxWindow *win, wxWindow **childLastFocused)
{
    wxCHECK_MSG( win, false, wxT("wxSetFocusToChild(): invalid window") );
        
    if ( childLastFocused && *childLastFocused )
    {
                if ( (*childLastFocused)->GetParent() == win )
        {
                                    wxWindow *deepestVisibleWindow = NULL;

            while ( *childLastFocused )
            {
                if ( (*childLastFocused)->IsShown() )
                {
                    if ( !deepestVisibleWindow )
                        deepestVisibleWindow = *childLastFocused;
                }
                else
                    deepestVisibleWindow = NULL;

                *childLastFocused = (*childLastFocused)->GetParent();
            }

            if ( deepestVisibleWindow )
            {
                *childLastFocused = deepestVisibleWindow;

                wxLogTrace(TRACE_FOCUS,
                           wxT("SetFocusToChild() => last child (0x%p)."),
                           (*childLastFocused)->GetHandle());

                                                (*childLastFocused)->SetFocus();
                return true;
            }
        }
        else
        {
                        *childLastFocused = NULL;
        }
    }

        wxWindowList::compatibility_iterator node = win->GetChildren().GetFirst();
    while ( node )
    {
        wxWindow *child = node->GetData();
        node = node->GetNext();

                if ( !win->IsClientAreaChild(child) )
            continue;

        if ( child->CanAcceptFocusFromKeyboard() && !child->IsTopLevel() )
        {
#if defined(__WXMSW__) && wxUSE_RADIOBTN
                                    wxRadioButton* btn = wxDynamicCast(child, wxRadioButton);
            if (btn)
            {
                wxRadioButton* selected = wxGetSelectedButtonInGroup(btn);
                if (selected)
                    child = selected;
            }
#endif 
            wxLogTrace(TRACE_FOCUS,
                       wxT("SetFocusToChild() => first child (0x%p)."),
                       child->GetHandle());

            if (childLastFocused)
                *childLastFocused = child;
            child->SetFocusFromKbd();
            return true;
        }
    }

    return false;
}

