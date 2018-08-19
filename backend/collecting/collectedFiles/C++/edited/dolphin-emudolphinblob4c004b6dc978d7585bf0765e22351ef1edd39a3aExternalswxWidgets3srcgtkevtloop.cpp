


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/evtloop.h"
#include "wx/evtloopsrc.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/log.h"
#endif 
#include "wx/private/eventloopsourcesmanager.h"
#include "wx/apptrait.h"

#include <gtk/gtk.h>
#include "wx/gtk/private/gtk2-compat.h"

GdkWindow* wxGetTopLevelGDK();



wxGUIEventLoop::wxGUIEventLoop()
{
    m_exitcode = 0;
}

int wxGUIEventLoop::DoRun()
{
    guint loopLevel = gtk_main_level();

                    while ( !m_shouldExit )
    {
        gtk_main();
    }

                            if ( loopLevel )
    {
        gtk_main_quit();
    }

    OnExit();

#if wxUSE_EXCEPTIONS
            if ( wxTheApp )
        wxTheApp->RethrowStoredException();
#endif 
    return m_exitcode;
}

void wxGUIEventLoop::ScheduleExit(int rc)
{
    wxCHECK_RET( IsInsideRun(), wxT("can't call ScheduleExit() if not started") );

    m_exitcode = rc;

    m_shouldExit = true;

    gtk_main_quit();
}

void wxGUIEventLoop::WakeUp()
{
                    if ( wxTheApp )
        wxTheApp->WakeUpIdle();
}


#if wxUSE_EVENTLOOP_SOURCE

extern "C"
{
static gboolean wx_on_channel_event(GIOChannel *channel,
                                    GIOCondition condition,
                                    gpointer data)
{
    wxUnusedVar(channel); 
    wxLogTrace(wxTRACE_EVT_SOURCE,
               "wx_on_channel_event, fd=%d, condition=%08x",
               g_io_channel_unix_get_fd(channel), condition);

    wxEventLoopSourceHandler * const
        handler = static_cast<wxEventLoopSourceHandler *>(data);

    if ( (condition & G_IO_IN) || (condition & G_IO_PRI) || (condition & G_IO_HUP) )
        handler->OnReadWaiting();

    if (condition & G_IO_OUT)
        handler->OnWriteWaiting();

    if ( (condition & G_IO_ERR) || (condition & G_IO_NVAL) )
        handler->OnExceptionWaiting();

                                        return TRUE;
}
}

class wxGUIEventLoopSourcesManager : public wxEventLoopSourcesManagerBase
{
public:
    virtual wxEventLoopSource*
    AddSourceForFD(int fd, wxEventLoopSourceHandler *handler, int flags) wxOVERRIDE
    {
        wxCHECK_MSG( fd != -1, NULL, "can't monitor invalid fd" );

        int condition = 0;
        if ( flags & wxEVENT_SOURCE_INPUT )
            condition |= G_IO_IN | G_IO_PRI | G_IO_HUP;
        if ( flags & wxEVENT_SOURCE_OUTPUT )
            condition |= G_IO_OUT;
        if ( flags & wxEVENT_SOURCE_EXCEPTION )
            condition |= G_IO_ERR | G_IO_NVAL;

        GIOChannel* channel = g_io_channel_unix_new(fd);
        const unsigned sourceId  = g_io_add_watch
                                   (
                                    channel,
                                    (GIOCondition)condition,
                                    &wx_on_channel_event,
                                    handler
                                   );
                g_io_channel_unref(channel);

        if ( !sourceId )
            return NULL;

        wxLogTrace(wxTRACE_EVT_SOURCE,
                   "Adding event loop source for fd=%d with GTK id=%u",
                   fd, sourceId);


        return new wxGTKEventLoopSource(sourceId, handler, flags);
    }
};

wxEventLoopSourcesManagerBase* wxGUIAppTraits::GetEventLoopSourcesManager()
{
    static wxGUIEventLoopSourcesManager s_eventLoopSourcesManager;

    return &s_eventLoopSourcesManager;
}

wxGTKEventLoopSource::~wxGTKEventLoopSource()
{
    wxLogTrace(wxTRACE_EVT_SOURCE,
               "Removing event loop source with GTK id=%u", m_sourceId);

    g_source_remove(m_sourceId);
}

#endif 

bool wxGUIEventLoop::Pending() const
{
    if ( wxTheApp )
    {
                return wxTheApp->EventsPending();
    }

    return gtk_events_pending() != 0;
}

bool wxGUIEventLoop::Dispatch()
{
    wxCHECK_MSG( IsRunning(), false, wxT("can't call Dispatch() if not running") );

        return !gtk_main_iteration();
}

extern "C" {
static gboolean wx_event_loop_timeout(void* data)
{
    bool* expired = static_cast<bool*>(data);
    *expired = true;

        return FALSE;
}
}

int wxGUIEventLoop::DispatchTimeout(unsigned long timeout)
{
    bool expired = false;
    const unsigned id = g_timeout_add(timeout, wx_event_loop_timeout, &expired);
    bool quit = gtk_main_iteration() != 0;

    if ( expired )
        return -1;

    g_source_remove(id);

    return !quit;
}


extern "C" {
static void wxgtk_main_do_event(GdkEvent* event, void* data)
{
            
                        
                wxEventCategory cat = wxEVT_CATEGORY_UNKNOWN,
                    cat2 = wxEVT_CATEGORY_UNKNOWN;
    switch (event->type)
    {
    case GDK_SELECTION_REQUEST:
    case GDK_SELECTION_NOTIFY:
    case GDK_SELECTION_CLEAR:
    case GDK_OWNER_CHANGE:
        cat = wxEVT_CATEGORY_CLIPBOARD;
        break;

    case GDK_KEY_PRESS:
    case GDK_KEY_RELEASE:
    case GDK_BUTTON_PRESS:
    case GDK_2BUTTON_PRESS:
    case GDK_3BUTTON_PRESS:
    case GDK_BUTTON_RELEASE:
    case GDK_SCROLL:            case GDK_CLIENT_EVENT:
        cat = wxEVT_CATEGORY_USER_INPUT;
        break;

    case GDK_PROPERTY_NOTIFY:
                                        cat2 = wxEVT_CATEGORY_CLIPBOARD;
        
    case GDK_PROXIMITY_IN:
    case GDK_PROXIMITY_OUT:

    case GDK_MOTION_NOTIFY:
    case GDK_ENTER_NOTIFY:
    case GDK_LEAVE_NOTIFY:
    case GDK_VISIBILITY_NOTIFY:

    case GDK_FOCUS_CHANGE:
    case GDK_CONFIGURE:
    case GDK_WINDOW_STATE:
    case GDK_SETTING:
    case GDK_DELETE:
    case GDK_DESTROY:

    case GDK_EXPOSE:
#ifndef __WXGTK3__
    case GDK_NO_EXPOSE:
#endif
    case GDK_MAP:
    case GDK_UNMAP:

    case GDK_DRAG_ENTER:
    case GDK_DRAG_LEAVE:
    case GDK_DRAG_MOTION:
    case GDK_DRAG_STATUS:
    case GDK_DROP_START:
    case GDK_DROP_FINISHED:
#if GTK_CHECK_VERSION(2,8,0)
    case GDK_GRAB_BROKEN:
#endif
#if GTK_CHECK_VERSION(2,14,0)
    case GDK_DAMAGE:
#endif
        cat = wxEVT_CATEGORY_UI;
        break;

    default:
        cat = wxEVT_CATEGORY_UNKNOWN;
        break;
    }

    wxGUIEventLoop* evtloop = static_cast<wxGUIEventLoop*>(data);

        if (evtloop->IsEventAllowedInsideYield(cat) ||
            (cat2 != wxEVT_CATEGORY_UNKNOWN &&
                evtloop->IsEventAllowedInsideYield(cat2)))
    {
                gtk_main_do_event(event);
    }
    else if (event->type != GDK_NOTHING)
    {
                        evtloop->StoreGdkEventForLaterProcessing(gdk_event_copy(event));
    }
}
}

void wxGUIEventLoop::DoYieldFor(long eventsToProcess)
{
                                    gdk_event_handler_set(wxgtk_main_do_event, this, NULL);
    while (Pending())           gtk_main_iteration();
    gdk_event_handler_set ((GdkEventFunc)gtk_main_do_event, NULL, NULL);

    wxEventLoopBase::DoYieldFor(eventsToProcess);

        GdkDisplay* disp = gdk_window_get_display(wxGetTopLevelGDK());
    for (size_t i=0; i<m_arrGdkEvents.GetCount(); i++)
    {
        GdkEvent* ev = (GdkEvent*)m_arrGdkEvents[i];

                gdk_display_put_event(disp, ev);
        gdk_event_free(ev);
    }

    m_arrGdkEvents.Clear();
}
