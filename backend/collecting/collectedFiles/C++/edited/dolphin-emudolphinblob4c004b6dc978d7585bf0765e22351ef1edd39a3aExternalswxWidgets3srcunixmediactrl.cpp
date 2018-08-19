
#include "wx/wxprec.h"

#if wxUSE_MEDIACTRL && wxUSE_GSTREAMER && !wxUSE_GSTREAMER_PLAYER

#include "wx/mediactrl.h"

#include <gst/gst.h>                
#if GST_CHECK_VERSION(1,0,0)
#include <gst/video/video.h>
#else
#include <gst/interfaces/xoverlay.h>
#endif

#ifndef  WX_PRECOMP
    #include "wx/log.h"                 #include "wx/app.h"                 #include "wx/timer.h"           #endif

#include "wx/filesys.h"             #include "wx/thread.h"              #include "wx/vector.h"              
#ifdef __WXGTK__
    #include <gtk/gtk.h>
    #include <gdk/gdkx.h>
    #include "wx/gtk/private/gtk2-compat.h"
#endif





#define wxGSTREAMER_TIMEOUT (100 * GST_MSECOND) 
#define wxTRACE_GStreamer wxT("GStreamer")

class WXDLLIMPEXP_MEDIA
    wxGStreamerMediaBackend : public wxMediaBackendCommonBase
{
public:

    wxGStreamerMediaBackend();
    virtual ~wxGStreamerMediaBackend();

    virtual bool CreateControl(wxControl* ctrl, wxWindow* parent,
                                     wxWindowID id,
                                     const wxPoint& pos,
                                     const wxSize& size,
                                     long style,
                                     const wxValidator& validator,
                                     const wxString& name) wxOVERRIDE;

    virtual bool Play() wxOVERRIDE;
    virtual bool Pause() wxOVERRIDE;
    virtual bool Stop() wxOVERRIDE;

    virtual bool Load(const wxString& fileName) wxOVERRIDE;
    virtual bool Load(const wxURI& location) wxOVERRIDE;
    virtual bool Load(const wxURI& location,
                      const wxURI& proxy) wxOVERRIDE
        { return wxMediaBackendCommonBase::Load(location, proxy); }


    virtual wxMediaState GetState() wxOVERRIDE;

    virtual bool SetPosition(wxLongLong where) wxOVERRIDE;
    virtual wxLongLong GetPosition() wxOVERRIDE;
    virtual wxLongLong GetDuration() wxOVERRIDE;

    virtual void Move(int x, int y, int w, int h) wxOVERRIDE;
    wxSize GetVideoSize() const wxOVERRIDE;

    virtual double GetPlaybackRate() wxOVERRIDE;
    virtual bool SetPlaybackRate(double dRate) wxOVERRIDE;

    virtual wxLongLong GetDownloadProgress() wxOVERRIDE;
    virtual wxLongLong GetDownloadTotal() wxOVERRIDE;

    virtual bool SetVolume(double dVolume) wxOVERRIDE;
    virtual double GetVolume() wxOVERRIDE;

        bool CheckForErrors();
    bool DoLoad(const wxString& locstring);
    wxMediaCtrl* GetControl() { return m_ctrl; }     void HandleStateChange(GstState oldstate, GstState newstate);
    bool QueryVideoSizeFromElement(GstElement* element);
    bool QueryVideoSizeFromPad(GstPad* caps);
    void SetupXOverlay();
    bool SyncStateChange(GstElement* element, GstState state,
                         gint64 llTimeout = wxGSTREAMER_TIMEOUT);
    bool TryAudioSink(GstElement* audiosink);
    bool TryVideoSink(GstElement* videosink);

    GstElement*     m_playbin;          wxSize          m_videoSize;        double          m_dRate;                                                wxLongLong      m_llPausedPos;  #if GST_CHECK_VERSION(1,0,0)
    GstVideoOverlay* m_xoverlay;     #else
    GstXOverlay*    m_xoverlay;     #endif
    wxMutex         m_asynclock;        class wxGStreamerMediaEventHandler* m_eventHandler; 
            wxMutex m_mutexErr;
    struct Error
    {
        Error(const gchar* message, const gchar* debug)
            : m_message(message, wxConvUTF8),
              m_debug(debug, wxConvUTF8)
        {
        }

        wxString m_message,
                 m_debug;
    };

    wxVector<Error> m_errors;

    friend class wxGStreamerMediaEventHandler;
    friend class wxGStreamerLoadWaitTimer;
    wxDECLARE_DYNAMIC_CLASS(wxGStreamerMediaBackend);
};

class wxGStreamerMediaEventHandler : public wxEvtHandler
{
    public:
    wxGStreamerMediaEventHandler(wxGStreamerMediaBackend* be) : m_be(be)
    {
        this->Connect(wxID_ANY, wxEVT_MEDIA_FINISHED,
           wxMediaEventHandler(wxGStreamerMediaEventHandler::OnMediaFinish));
    }

    void OnMediaFinish(wxMediaEvent& event);

    wxGStreamerMediaBackend* m_be;
};


wxIMPLEMENT_DYNAMIC_CLASS(wxGStreamerMediaBackend, wxMediaBackend);


#ifdef __WXGTK__
extern "C" {
static gboolean
#ifdef __WXGTK3__
draw(GtkWidget* widget, cairo_t* cr, wxGStreamerMediaBackend* be)
#else
expose_event(GtkWidget* widget, GdkEventExpose* event, wxGStreamerMediaBackend* be)
#endif
{
                
        if(!(be->m_videoSize.x==0&&be->m_videoSize.y==0) &&
       GST_STATE(be->m_playbin) >= GST_STATE_PAUSED)
    {
                        #if GST_CHECK_VERSION(1,0,0)
        gst_video_overlay_expose(be->m_xoverlay);
#else
        gst_x_overlay_expose(be->m_xoverlay);
#endif
    }
    else
    {
        #ifdef __WXGTK3__
        GtkAllocation a;
        gtk_widget_get_allocation(widget, &a);
        cairo_rectangle(cr, 0, 0, a.width, a.height);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_fill(cr);
#else
        gdk_draw_rectangle (event->window, widget->style->black_gc, TRUE, 0, 0,
                            widget->allocation.width,
                            widget->allocation.height);
#endif
    }

    return FALSE;
}
}
#endif 
#ifdef __WXGTK__
extern "C" {
static gint gtk_window_realize_callback(GtkWidget* widget,
                                        wxGStreamerMediaBackend* be)
{
    gdk_flush();

    GdkWindow* window = gtk_widget_get_window(widget);
    wxASSERT(window);

#if GST_CHECK_VERSION(1,0,0)
    gst_video_overlay_set_window_handle(be->m_xoverlay,
                                GDK_WINDOW_XID(window)
                                );
#else
    gst_x_overlay_set_xwindow_id( GST_X_OVERLAY(be->m_xoverlay),
                                GDK_WINDOW_XID(window)
                                );
#endif
    GtkWidget* w = be->GetControl()->m_wxwindow;
#ifdef __WXGTK3__
    g_signal_connect(w, "draw", G_CALLBACK(draw), be);
#else
    g_signal_connect(w, "expose_event", G_CALLBACK(expose_event), be);
#endif
    return 0;
}
}
#endif 
extern "C" {
static void gst_finish_callback(GstElement *WXUNUSED(play),
                                wxGStreamerMediaBackend* be)
{
    wxLogTrace(wxTRACE_GStreamer, wxT("gst_finish_callback"));
    wxMediaEvent event(wxEVT_MEDIA_FINISHED);
    be->m_eventHandler->AddPendingEvent(event);
}
}

extern "C" {
static void gst_error_callback(GstElement *WXUNUSED(play),
                               GstElement *WXUNUSED(src),
                               GError     *err,
                               gchar      *debug,
                               wxGStreamerMediaBackend* be)
{
    wxMutexLocker lock(be->m_mutexErr);
    be->m_errors.push_back(wxGStreamerMediaBackend::Error(err->message, debug));
}
}

extern "C" {
static void gst_notify_caps_callback(GstPad* pad,
                                     GParamSpec* WXUNUSED(pspec),
                                     wxGStreamerMediaBackend* be)
{
    wxLogTrace(wxTRACE_GStreamer, wxT("gst_notify_caps_callback"));
    be->QueryVideoSizeFromPad(pad);
}
}

#if !GST_CHECK_VERSION(1,0,0)
extern "C" {
static void gst_notify_stream_info_callback(GstElement* WXUNUSED(element),
                                            GParamSpec* WXUNUSED(pspec),
                                            wxGStreamerMediaBackend* be)
{
    wxLogTrace(wxTRACE_GStreamer, wxT("gst_notify_stream_info_callback"));
    be->QueryVideoSizeFromElement(be->m_playbin);
}
}
#endif

extern "C" {
static gboolean gst_bus_async_callback(GstBus* WXUNUSED(bus),
                                       GstMessage* message,
                                       wxGStreamerMediaBackend* be)
{
    if ( GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR )
    {
        GError* error;
        gchar* debug;
        gst_message_parse_error(message, &error, &debug);
        gst_error_callback(NULL, NULL, error, debug, be);
        return FALSE;
    }

    if(((GstElement*)GST_MESSAGE_SRC(message)) != be->m_playbin)
        return TRUE;
    if(be->m_asynclock.TryLock() != wxMUTEX_NO_ERROR)
        return TRUE;

    switch(GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_STATE_CHANGED:
        {
            GstState oldstate, newstate, pendingstate;
            gst_message_parse_state_changed(message, &oldstate,
                                            &newstate, &pendingstate);
            be->HandleStateChange(oldstate, newstate);
            break;
        }
        case GST_MESSAGE_EOS:
        {
            gst_finish_callback(NULL, be);
            break;
        }

        default:
            break;
    }

    be->m_asynclock.Unlock();
    return FALSE; }

static GstBusSyncReply gst_bus_sync_callback(GstBus* bus,
                                             GstMessage* message,
                                             wxGStreamerMediaBackend* be)
{
        #if GST_CHECK_VERSION(1,0,0)
    if (!gst_is_video_overlay_prepare_window_handle_message (message))
#else
    if (GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT ||
        !gst_structure_has_name (message->structure, "prepare-xwindow-id"))
#endif
    {
                                                        if(gst_bus_async_callback(bus, message, be))
            return GST_BUS_PASS;
        else
            return GST_BUS_DROP;
    }

    wxLogTrace(wxTRACE_GStreamer, wxT("Got prepare-xwindow-id"));
    be->SetupXOverlay();
    return GST_BUS_DROP; }
}


void wxGStreamerMediaBackend::HandleStateChange(GstState oldstate,
                                                GstState newstate)
{
    switch(newstate)
    {
        case GST_STATE_PLAYING:
            wxLogTrace(wxTRACE_GStreamer, wxT("Play event"));
            QueuePlayEvent();
            break;
        case GST_STATE_PAUSED:
                                                if(oldstate < GST_STATE_PAUSED || oldstate == newstate)
                break;
            if(wxGStreamerMediaBackend::GetPosition() != 0)
            {
                wxLogTrace(wxTRACE_GStreamer, wxT("Pause event"));
                QueuePauseEvent();
            }
            else
            {
                wxLogTrace(wxTRACE_GStreamer, wxT("Stop event"));
                QueueStopEvent();
            }
            break;
       default:             break;
    }
}

#if !GST_CHECK_VERSION(1,0,0)
bool wxGStreamerMediaBackend::QueryVideoSizeFromElement(GstElement* element)
{
    const GList *list = NULL;
    g_object_get (G_OBJECT (element), "stream-info", &list, NULL);

    for ( ; list != NULL; list = list->next)
    {
        GObject *info = (GObject *) list->data;
        gint type;
        GParamSpec *pspec;
        GEnumValue *val;
        GstPad *pad = NULL;

        g_object_get (info, "type", &type, NULL);
        pspec = g_object_class_find_property (
                        G_OBJECT_GET_CLASS (info), "type");
        val = g_enum_get_value (G_PARAM_SPEC_ENUM (pspec)->enum_class, type);

        if (!strncasecmp(val->value_name, "video", 5) ||
            !strncmp(val->value_name, "GST_STREAM_TYPE_VIDEO", 21))
        {
                                    pspec = g_object_class_find_property (
                        G_OBJECT_GET_CLASS (info), "object");

            if (!pspec)
                g_object_get (info, "pad", &pad, NULL);
            else
                g_object_get (info, "object", &pad, NULL);

            if(!QueryVideoSizeFromPad(pad))
            {
                                g_signal_connect(
                pad,
                "notify::caps",
                G_CALLBACK(gst_notify_caps_callback),
                this);
            }
            break;
        }    }
        if(list == NULL)
    {
        m_videoSize = wxSize(0,0);
        return false;
    }

    return true;
}
#endif

bool wxGStreamerMediaBackend::QueryVideoSizeFromPad(GstPad* pad)
{
#if GST_CHECK_VERSION(1,0,0)
    GstCaps* caps = gst_pad_get_current_caps(pad);
#else
    const GstCaps* caps = GST_PAD_CAPS(pad);
#endif
    if ( caps )
    {
        const GstStructure *s = gst_caps_get_structure (caps, 0);
        wxASSERT(s);

        gst_structure_get_int (s, "width", &m_videoSize.x);
        gst_structure_get_int (s, "height", &m_videoSize.y);

        const GValue *par;
        par = gst_structure_get_value (s, "pixel-aspect-ratio");

        if (par)
        {
            wxLogTrace(wxTRACE_GStreamer,
                       wxT("pixel-aspect-ratio found in pad"));
            int num = par->data[0].v_int,
                den = par->data[1].v_int;

                        if (num > den)
                m_videoSize.x = (int) ((float) num * m_videoSize.x / den);
            else
                m_videoSize.y = (int) ((float) den * m_videoSize.y / num);
        }

        wxLogTrace(wxTRACE_GStreamer, wxT("Adjusted video size: [%i,%i]"),
                    m_videoSize.x, m_videoSize.y);
#if GST_CHECK_VERSION(1,0,0)
        gst_caps_unref (caps);
#endif

        NotifyMovieSizeChanged ();

        return true;
    } 
    m_videoSize = wxSize(0,0);

    NotifyMovieSizeChanged ();

    return false; }

void wxGStreamerMediaBackend::SetupXOverlay()
{
    #ifdef __WXGTK__
    if (!gtk_widget_get_realized(m_ctrl->m_wxwindow))
    {
                g_signal_connect (m_ctrl->m_wxwindow,
                          "realize",
                          G_CALLBACK (gtk_window_realize_callback),
                          this);
    }
    else
    {
        gdk_flush();

        GdkWindow* window = gtk_widget_get_window(m_ctrl->m_wxwindow);
        wxASSERT(window);
#endif
#if GST_CHECK_VERSION(1,0,0)
        gst_video_overlay_set_window_handle(m_xoverlay,
#else
        gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(m_xoverlay),
#endif
#ifdef __WXGTK__
                        GDK_WINDOW_XID(window)
#else
                        ctrl->GetHandle()
#endif
                                  );
#ifdef __WXGTK__
        GtkWidget* w = m_ctrl->m_wxwindow;
#ifdef __WXGTK3__
        g_signal_connect(w, "draw", G_CALLBACK(draw), this);
#else
        g_signal_connect(w, "expose_event", G_CALLBACK(expose_event), this);
#endif
    } #endif
}

bool wxGStreamerMediaBackend::SyncStateChange(GstElement* element,
                                              GstState desiredstate,
                                              gint64 llTimeout)
{
    GstBus* bus = gst_element_get_bus(element);
    GstMessage* message;
    bool bBreak = false,
         bSuccess = false;
    gint64 llTimeWaited = 0;

    do
    {
#if 1
                                                if(gst_bus_have_pending(bus) == FALSE)
        {
            if(llTimeWaited >= llTimeout)
                return true;             llTimeWaited += 10*GST_MSECOND;
            wxMilliSleep(10);
            continue;
        }

        message = gst_bus_pop(bus);
#else
        message = gst_bus_poll(bus, (GstMessageType)
                           (GST_MESSAGE_STATE_CHANGED |
                            GST_MESSAGE_ERROR |
                            GST_MESSAGE_EOS), llTimeout);
        if(!message)
            return true;
#endif
        if(((GstElement*)GST_MESSAGE_SRC(message)) == element)
        {
            switch(GST_MESSAGE_TYPE(message))
            {
                case GST_MESSAGE_STATE_CHANGED:
                {
                    GstState oldstate, newstate, pendingstate;
                    gst_message_parse_state_changed(message, &oldstate,
                                                    &newstate, &pendingstate);
                    if(newstate == desiredstate)
                    {
                        bSuccess = bBreak = true;
                    }
                    break;
                }
                case GST_MESSAGE_ERROR:
                {
                    GError* error;
                    gchar* debug;
                    gst_message_parse_error(message, &error, &debug);
                    gst_error_callback(NULL, NULL, error, debug, this);
                    bBreak = true;
                    break;
                }
                case GST_MESSAGE_EOS:
                    wxLogSysError(wxT("Reached end of stream prematurely"));
                    bBreak = true;
                    break;
                default:
                    break;             }
        }

        gst_message_unref(message);
    }while(!bBreak);

    return bSuccess;
}

bool wxGStreamerMediaBackend::TryAudioSink(GstElement* audiosink)
{
    if( !GST_IS_ELEMENT(audiosink) )
    {
        if(G_IS_OBJECT(audiosink))
            g_object_unref(audiosink);
        return false;
    }

    return true;
}

bool wxGStreamerMediaBackend::TryVideoSink(GstElement* videosink)
{
#if GST_CHECK_VERSION(1,0,0)
        if( !GST_IS_BIN(videosink) && !GST_IS_VIDEO_OVERLAY(videosink) )
    {
        if(G_IS_OBJECT(videosink))
            g_object_unref(videosink);
        return false;
    }

                if( GST_IS_BIN(videosink) )
        m_xoverlay = (GstVideoOverlay*)
                        gst_bin_get_by_interface (GST_BIN (videosink),
                                                  GST_TYPE_VIDEO_OVERLAY);
    else
        m_xoverlay = (GstVideoOverlay*) videosink;

    if ( !GST_IS_VIDEO_OVERLAY(m_xoverlay) )
    {
        g_object_unref(videosink);
        return false;
    }
#else
        if( !GST_IS_BIN(videosink) && !GST_IS_X_OVERLAY(videosink) )
    {
        if(G_IS_OBJECT(videosink))
            g_object_unref(videosink);
        return false;
    }

                if( GST_IS_BIN(videosink) )
        m_xoverlay = (GstXOverlay*)
                        gst_bin_get_by_interface (GST_BIN (videosink),
                                                  GST_TYPE_X_OVERLAY);
    else
        m_xoverlay = (GstXOverlay*) videosink;

    if ( !GST_IS_X_OVERLAY(m_xoverlay) )
    {
        g_object_unref(videosink);
        return false;
    }
#endif
    return true;
}

void wxGStreamerMediaEventHandler::OnMediaFinish(wxMediaEvent& WXUNUSED(event))
{
                if(m_be->SendStopEvent())
    {
                                                wxMutexLocker lock(m_be->m_asynclock);

                gst_element_set_state (m_be->m_playbin, GST_STATE_READY);
        m_be->SyncStateChange(m_be->m_playbin, GST_STATE_READY);

                                gst_element_set_state (m_be->m_playbin, GST_STATE_PAUSED);
        m_be->SyncStateChange(m_be->m_playbin, GST_STATE_PAUSED);
        m_be->m_llPausedPos = 0;

                m_be->QueueFinishEvent();
    }
}


wxGStreamerMediaBackend::wxGStreamerMediaBackend()
    : m_playbin(NULL),
      m_eventHandler(NULL)
{
}

wxGStreamerMediaBackend::~wxGStreamerMediaBackend()
{
        if(m_playbin)
    {
        wxASSERT( GST_IS_OBJECT(m_playbin) );
        gst_element_set_state (m_playbin, GST_STATE_NULL);
        gst_object_unref (GST_OBJECT (m_playbin));
        delete m_eventHandler;
    }
}

bool wxGStreamerMediaBackend::CheckForErrors()
{
    wxMutexLocker lock(m_mutexErr);
    if ( m_errors.empty() )
        return false;

    for ( unsigned n = 0; n < m_errors.size(); n++ )
    {
        const Error& err = m_errors[n];

        wxLogTrace(wxTRACE_GStreamer,
                   "gst_error_callback: %s", err.m_debug);
        wxLogError(_("Media playback error: %s"), err.m_message);
    }

    m_errors.clear();

    return true;
}

bool wxGStreamerMediaBackend::CreateControl(wxControl* ctrl, wxWindow* parent,
                                wxWindowID id,
                                const wxPoint& pos,
                                const wxSize& size,
                                long style,
                                const wxValidator& validator,
                                const wxString& name)
{
            
    #if wxUSE_UNICODE
    int i;
    char **argvGST = new char*[wxTheApp->argc + 1];
    for ( i = 0; i < wxTheApp->argc; i++ )
    {
        argvGST[i] = wxStrdupA(wxTheApp->argv[i].utf8_str());
    }

    argvGST[wxTheApp->argc] = NULL;

    int argcGST = wxTheApp->argc;
#else
#define argcGST wxTheApp->argc
#define argvGST wxTheApp->argv
#endif

        gboolean bInited;
    GError* error = NULL;
    bInited = gst_init_check(&argcGST, &argvGST, &error);

    #if wxUSE_UNICODE
    for ( i = 0; i < argcGST; i++ )
    {
        free(argvGST[i]);
    }

    delete [] argvGST;
#endif

    if(!bInited)        {
        if(error)
        {
            wxLogSysError(wxT("Could not initialize GStreamer\n")
                          wxT("Error Message:%s"),
                          (const wxChar*) wxConvUTF8.cMB2WX(error->message)
                         );
            g_error_free(error);
        }
        else
            wxLogSysError(wxT("Could not initialize GStreamer"));

        return false;
    }

                m_ctrl = wxStaticCast(ctrl, wxMediaCtrl);

#ifdef __WXGTK__
        m_ctrl->m_noExpose = true;
#endif

    if( !m_ctrl->wxControl::Create(parent, id, pos, size,
                            style,                              validator, name) )
    {
        wxFAIL_MSG(wxT("Could not create wxControl!!!"));
        return false;
    }

#ifdef __WXGTK__
                gtk_widget_set_double_buffered(m_ctrl->m_wxwindow, FALSE);
#endif

            m_ctrl->SetBackgroundStyle(wxBG_STYLE_CUSTOM);

        m_playbin = gst_element_factory_make ("playbin", "play");
    if (!GST_IS_ELEMENT(m_playbin))
    {
        if(G_IS_OBJECT(m_playbin))
            g_object_unref(m_playbin);
        wxLogSysError(wxT("Got an invalid playbin"));
        return false;
    }

            gst_bus_add_watch (gst_element_get_bus(m_playbin),
                       (GstBusFunc) gst_bus_async_callback, this);
#if GST_CHECK_VERSION(1,0,0)
    gst_bus_set_sync_handler(gst_element_get_bus(m_playbin),
                             (GstBusSyncHandler) gst_bus_sync_callback, this, NULL);
#else
    gst_bus_set_sync_handler(gst_element_get_bus(m_playbin),
                             (GstBusSyncHandler) gst_bus_sync_callback, this);
#endif

        GstElement* audiosink;
#if !GST_CHECK_VERSION(1,0,0)
    audiosink = gst_element_factory_make ("gconfaudiosink", "audio-sink");
    if( !TryAudioSink(audiosink) )
    {
#endif
                audiosink = gst_element_factory_make ("autoaudiosink", "audio-sink");
        if( !TryAudioSink(audiosink) )
        {
            audiosink = gst_element_factory_make ("alsasink", "alsa-output");
            if( !TryAudioSink(audiosink) )
            {
                audiosink = gst_element_factory_make ("osssink", "play_audio");
                if( !TryAudioSink(audiosink) )
                {
                    wxLogSysError(wxT("Could not find a valid audiosink"));
                    return false;
                }
            }
        }
#if !GST_CHECK_VERSION(1,0,0)
    }
#endif

            GstElement* videosink;
#if !GST_CHECK_VERSION(1,0,0)
    videosink = gst_element_factory_make ("gconfvideosink", "video-sink");
    if( !TryVideoSink(videosink) )
    {
#endif
        videosink = gst_element_factory_make ("autovideosink", "video-sink");
        if( !TryVideoSink(videosink) )
        {
            videosink = gst_element_factory_make ("xvimagesink", "video-sink");
            if( !TryVideoSink(videosink) )
            {
                                videosink =
                    gst_element_factory_make ("ximagesink", "video-sink");
                if( !TryVideoSink(videosink) )
                {
                    g_object_unref(audiosink);
                    wxLogSysError(wxT("Could not find a suitable video sink"));
                    return false;
                }
            }
        }
#if !GST_CHECK_VERSION(1,0,0)
    }
#endif

#if GST_CHECK_VERSION(1,0,0)
    GstPad *video_sinkpad = gst_element_get_static_pad (videosink, "sink");
    g_signal_connect (video_sinkpad, "notify::caps", G_CALLBACK (gst_notify_caps_callback), this);
    gst_object_unref (video_sinkpad);
#else
    g_signal_connect(m_playbin, "notify::stream-info",
                     G_CALLBACK(gst_notify_stream_info_callback), this);
#endif

            SetupXOverlay();

            g_object_set (G_OBJECT (m_playbin),
                  "video-sink", videosink,
                  "audio-sink", audiosink,
                   NULL);

    m_eventHandler = new wxGStreamerMediaEventHandler(this);
    return true;
}

bool wxGStreamerMediaBackend::Load(const wxString& fileName)
{
    return DoLoad(wxFileSystem::FileNameToURL(fileName));
}

bool wxGStreamerMediaBackend::Load(const wxURI& location)
{
#if !GST_CHECK_VERSION(1,0,0)
    if(location.GetScheme().CmpNoCase(wxT("file")) == 0)
    {
        wxString uristring = location.BuildUnescapedURI();

                        return DoLoad(wxString(wxT("file:                      uristring.Right(uristring.length() - 5)
                     );
    }
    else
#endif
        return DoLoad(location.BuildURI());
}

bool wxGStreamerMediaBackend::DoLoad(const wxString& locstring)
{
    wxMutexLocker lock(m_asynclock); 
        m_llPausedPos = 0;
    m_dRate = 1.0;
    m_videoSize = wxSize(0,0);

        if( gst_element_set_state (m_playbin,
                               GST_STATE_READY) == GST_STATE_CHANGE_FAILURE ||
        !SyncStateChange(m_playbin, GST_STATE_READY))
    {
        CheckForErrors();

        wxLogError(_("Failed to prepare playing \"%s\"."), locstring);
        return false;
    }

        gst_element_set_state (m_playbin, GST_STATE_NULL);

            wxASSERT(gst_uri_protocol_is_valid("file"));
    wxASSERT(gst_uri_is_valid(locstring.mb_str()));

    g_object_set (G_OBJECT (m_playbin), "uri",
                  (const char*)locstring.mb_str(), NULL);

            if( gst_element_set_state (m_playbin,
                               GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE ||
        !SyncStateChange(m_playbin, GST_STATE_PAUSED))
    {
        CheckForErrors();
        return false;                                                 }

                if ( CheckForErrors() )
        return false;


    NotifyMovieLoaded();     return true;
}


bool wxGStreamerMediaBackend::Play()
{
    if (gst_element_set_state (m_playbin,
                               GST_STATE_PLAYING) == GST_STATE_CHANGE_FAILURE)
    {
        CheckForErrors();
        return false;
    }

    return true;
}

bool wxGStreamerMediaBackend::Pause()
{
    m_llPausedPos = wxGStreamerMediaBackend::GetPosition();
    if (gst_element_set_state (m_playbin,
                               GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE)
    {
        CheckForErrors();
        return false;
    }
    return true;
}

bool wxGStreamerMediaBackend::Stop()
{
    {           wxMutexLocker lock(m_asynclock);
        if(gst_element_set_state (m_playbin,
                                  GST_STATE_PAUSED) == GST_STATE_CHANGE_FAILURE ||
          !SyncStateChange(m_playbin, GST_STATE_PAUSED))
        {
            CheckForErrors();
            wxLogSysError(wxT("Could not set state to paused for Stop()"));
            return false;
        }
    }   
    bool bSeekedOK = wxGStreamerMediaBackend::SetPosition(0);

    if(!bSeekedOK)
    {
        wxLogSysError(wxT("Could not seek to initial position in Stop()"));
        return false;
    }

    QueueStopEvent();     return true;
}

wxMediaState wxGStreamerMediaBackend::GetState()
{
    switch(GST_STATE(m_playbin))
    {
        case GST_STATE_PLAYING:
            return wxMEDIASTATE_PLAYING;
        case GST_STATE_PAUSED:
            if (m_llPausedPos == 0)
                return wxMEDIASTATE_STOPPED;
            else
                return wxMEDIASTATE_PAUSED;
        default:            return wxMEDIASTATE_STOPPED;
    }
}

wxLongLong wxGStreamerMediaBackend::GetPosition()
{
    if(GetState() != wxMEDIASTATE_PLAYING)
        return m_llPausedPos;
    else
    {
        gint64 pos;
#if GST_CHECK_VERSION(1,0,0)
        if (!gst_element_query_position(m_playbin, GST_FORMAT_TIME, &pos) ||
            pos == -1)
            return 0;
#else
        GstFormat fmtTime = GST_FORMAT_TIME;

        if (!gst_element_query_position(m_playbin, &fmtTime, &pos) ||
            fmtTime != GST_FORMAT_TIME || pos == -1)
            return 0;
#endif
        return pos / GST_MSECOND ;
    }
}

bool wxGStreamerMediaBackend::SetPosition(wxLongLong where)
{
    gst_element_seek (m_playbin, m_dRate, GST_FORMAT_TIME,
       (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                      GST_SEEK_TYPE_SET, where.GetValue() * GST_MSECOND,
                      GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE );

    m_llPausedPos = where;
    return true;
}

wxLongLong wxGStreamerMediaBackend::GetDuration()
{
    gint64 length;
#if GST_CHECK_VERSION(1,0,0)
    if(!gst_element_query_duration(m_playbin, GST_FORMAT_TIME, &length) ||
       length == -1)
        return 0;
#else
    GstFormat fmtTime = GST_FORMAT_TIME;

    if(!gst_element_query_duration(m_playbin, &fmtTime, &length) ||
       fmtTime != GST_FORMAT_TIME || length == -1)
        return 0;
#endif
    return length / GST_MSECOND ;
}

void wxGStreamerMediaBackend::Move(int WXUNUSED(x),
                                   int WXUNUSED(y),
                                   int WXUNUSED(w),
                                   int WXUNUSED(h))
{
}

wxSize wxGStreamerMediaBackend::GetVideoSize() const
{
    return m_videoSize;
}

double wxGStreamerMediaBackend::GetPlaybackRate()
{
    return m_dRate;                                         }

bool wxGStreamerMediaBackend::SetPlaybackRate(double )
{
#if 0     if( gst_element_seek (m_playbin, dRate, GST_FORMAT_TIME,
                 (GstSeekFlags)(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT),
                          GST_SEEK_TYPE_CUR, 0,
                          GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE ) )
    {
        m_dRate = dRate;
        return true;
    }
#endif

        return false;
}

wxLongLong wxGStreamerMediaBackend::GetDownloadProgress()
{
    return 0;
}

wxLongLong wxGStreamerMediaBackend::GetDownloadTotal()
{
    gint64 length;
#if GST_CHECK_VERSION(1,0,0)
    if (!gst_element_query_duration(m_playbin, GST_FORMAT_BYTES, &length) ||
         length == -1)
        return 0;
#else
    GstFormat fmtBytes = GST_FORMAT_BYTES;

    if (!gst_element_query_duration(m_playbin, &fmtBytes, &length) ||
          fmtBytes != GST_FORMAT_BYTES || length == -1)
        return 0;
#endif
    return length;
}

bool wxGStreamerMediaBackend::SetVolume(double dVolume)
{
    if(g_object_class_find_property(
            G_OBJECT_GET_CLASS(G_OBJECT(m_playbin)),
            "volume") != NULL)
    {
        g_object_set(G_OBJECT(m_playbin), "volume", dVolume, NULL);
        return true;
    }
    else
    {
        wxLogTrace(wxTRACE_GStreamer,
            wxT("SetVolume: volume prop not found - 0.8.5 of ")
            wxT("gst-plugins probably needed"));
    return false;
    }
}

double wxGStreamerMediaBackend::GetVolume()
{
    double dVolume = 1.0;

    if(g_object_class_find_property(
            G_OBJECT_GET_CLASS(G_OBJECT(m_playbin)),
            "volume") != NULL)
    {
        g_object_get(G_OBJECT(m_playbin), "volume", &dVolume, NULL);
    }
    else
    {
        wxLogTrace(wxTRACE_GStreamer,
            wxT("GetVolume: volume prop not found - 0.8.5 of ")
            wxT("gst-plugins probably needed"));
    }

    return dVolume;
}

#include "wx/html/forcelnk.h"
FORCE_LINK_ME(basewxmediabackends)

#endif 