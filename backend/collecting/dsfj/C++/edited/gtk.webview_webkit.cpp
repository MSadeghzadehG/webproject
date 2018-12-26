
#include "wx/wxprec.h"

#if wxUSE_WEBVIEW && wxUSE_WEBVIEW_WEBKIT

#include "wx/stockitem.h"
#include "wx/gtk/webview_webkit.h"
#include "wx/gtk/control.h"
#include "wx/gtk/private.h"
#include "wx/filesys.h"
#include "wx/base64.h"
#include "wx/log.h"
#include <webkit/webkit.h>


extern "C"
{

static void
wxgtk_webview_webkit_load_status(GtkWidget* widget,
                                 GParamSpec*,
                                 wxWebViewWebKit *webKitCtrl)
{
    wxString url = webKitCtrl->GetCurrentURL();

    WebKitLoadStatus status;
    g_object_get(G_OBJECT(widget), "load-status", &status, NULL);

    wxString target; 
    if (status == WEBKIT_LOAD_FINISHED)
    {
        WebKitWebBackForwardList* hist = webkit_web_view_get_back_forward_list(WEBKIT_WEB_VIEW(widget));
        WebKitWebHistoryItem* item = webkit_web_back_forward_list_get_current_item(hist);
                                if(!item || (WEBKIT_IS_WEB_HISTORY_ITEM(item) && 
                     webkit_web_history_item_get_uri(item) != url))
        {
            WebKitWebHistoryItem*
                newitem = webkit_web_history_item_new_with_data
                          (
                            url.utf8_str(),
                            webKitCtrl->GetCurrentTitle().utf8_str()
                          );
            webkit_web_back_forward_list_add_item(hist, newitem);
        }

        webKitCtrl->m_busy = false;
        wxWebViewEvent event(wxEVT_WEBVIEW_LOADED,
                             webKitCtrl->GetId(),
                             url, target);

        if (webKitCtrl && webKitCtrl->GetEventHandler())
            webKitCtrl->GetEventHandler()->ProcessEvent(event);
    }
    else if (status ==  WEBKIT_LOAD_COMMITTED)
    {
        webKitCtrl->m_busy = true;
        wxWebViewEvent event(wxEVT_WEBVIEW_NAVIGATED,
                             webKitCtrl->GetId(),
                             url, target);

        if (webKitCtrl && webKitCtrl->GetEventHandler())
            webKitCtrl->GetEventHandler()->ProcessEvent(event);
    }
}

static gboolean
wxgtk_webview_webkit_navigation(WebKitWebView *,
                                WebKitWebFrame *frame,
                                WebKitNetworkRequest *request,
                                WebKitWebNavigationAction *,
                                WebKitWebPolicyDecision *policy_decision,
                                wxWebViewWebKit *webKitCtrl)
{
    const gchar* uri = webkit_network_request_get_uri(request);
    wxString target = webkit_web_frame_get_name (frame);
    
            if(webKitCtrl->m_creating)
    {
        webKitCtrl->m_creating = false;
        wxWebViewEvent event(wxEVT_WEBVIEW_NEWWINDOW,
                             webKitCtrl->GetId(),
                             wxString(uri, wxConvUTF8),
                             target);

        if(webKitCtrl && webKitCtrl->GetEventHandler())
            webKitCtrl->GetEventHandler()->ProcessEvent(event);
        
        webkit_web_policy_decision_ignore(policy_decision);
        return TRUE;
    }

    if(webKitCtrl->m_guard)
    {
        webKitCtrl->m_guard = false;
                        webKitCtrl->m_vfsurl = webkit_network_request_get_uri(request);
        webkit_web_policy_decision_use(policy_decision);
        return FALSE;
    }

    webKitCtrl->m_busy = true;

    wxWebViewEvent event(wxEVT_WEBVIEW_NAVIGATING,
                         webKitCtrl->GetId(),
                         wxString( uri, wxConvUTF8 ),
                         target);

    if (webKitCtrl && webKitCtrl->GetEventHandler())
        webKitCtrl->GetEventHandler()->ProcessEvent(event);

    if (!event.IsAllowed())
    {
        webKitCtrl->m_busy = false;
        webkit_web_policy_decision_ignore(policy_decision);
        return TRUE;
    }
    else
    {
        wxString wxuri = uri;
        wxSharedPtr<wxWebViewHandler> handler;
        wxVector<wxSharedPtr<wxWebViewHandler> > handlers = webKitCtrl->GetHandlers();
                for(wxVector<wxSharedPtr<wxWebViewHandler> >::iterator it = handlers.begin();
            it != handlers.end(); ++it)
        {
            if(wxuri.substr(0, (*it)->GetName().length()) == (*it)->GetName())
            {
                handler = (*it);
            }
        }
                        if(handler)
        {
            webKitCtrl->m_guard = true;
            wxFSFile* file = handler->GetFile(wxuri);
            if(file)
            {
                webKitCtrl->SetPage(*file->GetStream(), wxuri);
            }
                        webkit_web_policy_decision_ignore(policy_decision);
            return TRUE;
        }
        return FALSE;
    }
}

static gboolean
wxgtk_webview_webkit_error(WebKitWebView*,
                           WebKitWebFrame*,
                           gchar *uri,
                           gpointer web_error,
                           wxWebViewWebKit* webKitWindow)
{
    webKitWindow->m_busy = false;
    wxWebViewNavigationError type = wxWEBVIEW_NAV_ERR_OTHER;

    GError* error = (GError*)web_error;
    wxString description(error->message, wxConvUTF8);

    if (strcmp(g_quark_to_string(error->domain), "soup_http_error_quark") == 0)
    {
        switch (error->code)
        {
            case SOUP_STATUS_CANCELLED:
                type = wxWEBVIEW_NAV_ERR_USER_CANCELLED;
                break;

            case SOUP_STATUS_CANT_RESOLVE:
            case SOUP_STATUS_NOT_FOUND:
                type = wxWEBVIEW_NAV_ERR_NOT_FOUND;
                break;

            case SOUP_STATUS_CANT_RESOLVE_PROXY:
            case SOUP_STATUS_CANT_CONNECT:
            case SOUP_STATUS_CANT_CONNECT_PROXY:
            case SOUP_STATUS_SSL_FAILED:
            case SOUP_STATUS_IO_ERROR:
                type = wxWEBVIEW_NAV_ERR_CONNECTION;
                break;

            case SOUP_STATUS_MALFORMED:
                            type = wxWEBVIEW_NAV_ERR_REQUEST;
                break;

                        
            case SOUP_STATUS_BAD_REQUEST:
                type = wxWEBVIEW_NAV_ERR_REQUEST;
                break;

            case SOUP_STATUS_UNAUTHORIZED:
            case SOUP_STATUS_FORBIDDEN:
                type = wxWEBVIEW_NAV_ERR_AUTH;
                break;

            case SOUP_STATUS_METHOD_NOT_ALLOWED:
            case SOUP_STATUS_NOT_ACCEPTABLE:
                type = wxWEBVIEW_NAV_ERR_SECURITY;
                break;

            case SOUP_STATUS_PROXY_AUTHENTICATION_REQUIRED:
                type = wxWEBVIEW_NAV_ERR_AUTH;
                break;

            case SOUP_STATUS_REQUEST_TIMEOUT:
                type = wxWEBVIEW_NAV_ERR_CONNECTION;
                break;

                        case SOUP_STATUS_REQUEST_ENTITY_TOO_LARGE:
            case SOUP_STATUS_REQUEST_URI_TOO_LONG:
            case SOUP_STATUS_UNSUPPORTED_MEDIA_TYPE:
                type = wxWEBVIEW_NAV_ERR_REQUEST;
                break;

            case SOUP_STATUS_BAD_GATEWAY:
            case SOUP_STATUS_SERVICE_UNAVAILABLE:
            case SOUP_STATUS_GATEWAY_TIMEOUT:
                type = wxWEBVIEW_NAV_ERR_CONNECTION;
                break;

            case SOUP_STATUS_HTTP_VERSION_NOT_SUPPORTED:
                type = wxWEBVIEW_NAV_ERR_REQUEST;
                break;
                                }
    }
    else if (strcmp(g_quark_to_string(error->domain),
                    "webkit-network-error-quark") == 0)
    {
        switch (error->code)
        {
                        
            case WEBKIT_NETWORK_ERROR_UNKNOWN_PROTOCOL:
                type = wxWEBVIEW_NAV_ERR_REQUEST;
                break;

            case WEBKIT_NETWORK_ERROR_CANCELLED:
                type = wxWEBVIEW_NAV_ERR_USER_CANCELLED;
                break;

            case WEBKIT_NETWORK_ERROR_FILE_DOES_NOT_EXIST:
                type = wxWEBVIEW_NAV_ERR_NOT_FOUND;
                break;
        }
    }
    else if (strcmp(g_quark_to_string(error->domain),
                    "webkit-policy-error-quark") == 0)
    {
        switch (error->code)
        {
                                                            case WEBKIT_POLICY_ERROR_CANNOT_USE_RESTRICTED_PORT:
                type = wxWEBVIEW_NAV_ERR_SECURITY;
                break;
        }
    }
    

    wxWebViewEvent event(wxEVT_WEBVIEW_ERROR,
                         webKitWindow->GetId(),
                         uri, "");
    event.SetString(description);
    event.SetInt(type);

    if (webKitWindow && webKitWindow->GetEventHandler())
    {
        webKitWindow->GetEventHandler()->ProcessEvent(event);
    }

    return FALSE;
}

static gboolean
wxgtk_webview_webkit_new_window(WebKitWebView*,
                                WebKitWebFrame *frame,
                                WebKitNetworkRequest *request,
                                WebKitWebNavigationAction*,
                                WebKitWebPolicyDecision *policy_decision,
                                wxWebViewWebKit *webKitCtrl)
{
    const gchar* uri = webkit_network_request_get_uri(request);

    wxString target = webkit_web_frame_get_name (frame);
    wxWebViewEvent event(wxEVT_WEBVIEW_NEWWINDOW,
                                       webKitCtrl->GetId(),
                                       wxString( uri, wxConvUTF8 ),
                                       target);

    if (webKitCtrl && webKitCtrl->GetEventHandler())
        webKitCtrl->GetEventHandler()->ProcessEvent(event);

        webkit_web_policy_decision_ignore(policy_decision);
    return TRUE;
}

static void
wxgtk_webview_webkit_title_changed(WebKitWebView*,
                                   WebKitWebFrame*,
                                   gchar *title,
                                   wxWebViewWebKit *webKitCtrl)
{
    wxWebViewEvent event(wxEVT_WEBVIEW_TITLE_CHANGED,
                         webKitCtrl->GetId(),
                         webKitCtrl->GetCurrentURL(),
                         "");
    event.SetString(wxString(title, wxConvUTF8));

    if (webKitCtrl && webKitCtrl->GetEventHandler())
        webKitCtrl->GetEventHandler()->ProcessEvent(event);

}

static void
wxgtk_webview_webkit_resource_req(WebKitWebView *,
                                  WebKitWebFrame *,
                                  WebKitWebResource *,
                                  WebKitNetworkRequest *request,
                                  WebKitNetworkResponse *,
                                  wxWebViewWebKit *webKitCtrl)
{
    wxString uri = webkit_network_request_get_uri(request);

    wxSharedPtr<wxWebViewHandler> handler;
    wxVector<wxSharedPtr<wxWebViewHandler> > handlers = webKitCtrl->GetHandlers();

        for(wxVector<wxSharedPtr<wxWebViewHandler> >::iterator it = handlers.begin();
        it != handlers.end(); ++it)
    {
        if(uri.substr(0, (*it)->GetName().length()) == (*it)->GetName())
        {
            handler = (*it);
        }
    }
            if(handler)
    {
                        if(webKitCtrl->m_vfsurl == uri)
            return;

        wxFSFile* file = handler->GetFile(uri);
        if(file)
        {
                        size_t size = file->GetStream()->GetLength();
            char *buffer = new char[size];
            file->GetStream()->Read(buffer, size);
            wxString data = wxBase64Encode(buffer, size);
            delete[] buffer;
            wxString mime = file->GetMimeType();
            wxString path = "data:" + mime + ";base64," + data;
                        webkit_network_request_set_uri(request, path.utf8_str());
        }

    }
}

#if WEBKIT_CHECK_VERSION(1, 10, 0)

static gboolean
wxgtk_webview_webkit_context_menu(WebKitWebView *,
                                  GtkWidget *,
                                  WebKitHitTestResult *,
                                  gboolean,
                                  wxWebViewWebKit *webKitCtrl)
{
    if(webKitCtrl->IsContextMenuEnabled())
        return FALSE;
    else
        return TRUE;
}

#endif

static WebKitWebView*
wxgtk_webview_webkit_create_webview(WebKitWebView *web_view,
                                    WebKitWebFrame*,
                                    wxWebViewWebKit *webKitCtrl)
{
            webKitCtrl->m_creating = true;
    return web_view;
}

} 

wxIMPLEMENT_DYNAMIC_CLASS(wxWebViewWebKit, wxWebView);

wxWebViewWebKit::wxWebViewWebKit()
{
    m_web_view = NULL;
}

bool wxWebViewWebKit::Create(wxWindow *parent,
                      wxWindowID id,
                      const wxString &url,
                      const wxPoint& pos,
                      const wxSize& size,
                      long style,
                      const wxString& name)
{
    m_busy = false;
    m_guard = false;
    m_creating = false;
    FindClear();

            style |= wxHSCROLL | wxVSCROLL;

    if (!PreCreation( parent, pos, size ) ||
        !CreateBase( parent, id, pos, size, style, wxDefaultValidator, name ))
    {
        wxFAIL_MSG( wxT("wxWebViewWebKit creation failed") );
        return false;
    }

    m_web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    GTKCreateScrolledWindowWith(GTK_WIDGET(m_web_view));
    g_object_ref(m_widget);

    g_signal_connect_after(m_web_view, "navigation-policy-decision-requested",
                           G_CALLBACK(wxgtk_webview_webkit_navigation),
                           this);
    g_signal_connect_after(m_web_view, "load-error",
                           G_CALLBACK(wxgtk_webview_webkit_error),
                           this);

    g_signal_connect_after(m_web_view, "new-window-policy-decision-requested",
                           G_CALLBACK(wxgtk_webview_webkit_new_window), this);

    g_signal_connect_after(m_web_view, "title-changed",
                           G_CALLBACK(wxgtk_webview_webkit_title_changed), this);

    g_signal_connect_after(m_web_view, "resource-request-starting",
                           G_CALLBACK(wxgtk_webview_webkit_resource_req), this);
      
#if WEBKIT_CHECK_VERSION(1, 10, 0)    
     g_signal_connect_after(m_web_view, "context-menu",
                           G_CALLBACK(wxgtk_webview_webkit_context_menu), this);
#endif
     
     g_signal_connect_after(m_web_view, "create-web-view",
                           G_CALLBACK(wxgtk_webview_webkit_create_webview), this);

    m_parent->DoAddChild( this );

    PostCreation(size);

    
    webkit_web_view_load_uri(m_web_view, url.utf8_str());

        WebKitWebBackForwardList* history;
    history = webkit_web_view_get_back_forward_list(m_web_view);
    m_historyLimit = webkit_web_back_forward_list_get_limit(history);

        g_signal_connect_after(m_web_view, "notify::load-status",
                           G_CALLBACK(wxgtk_webview_webkit_load_status),
                           this);

    return true;
}

wxWebViewWebKit::~wxWebViewWebKit()
{
    if (m_web_view)
        GTKDisconnect(m_web_view);
}

bool wxWebViewWebKit::Enable( bool enable )
{
    if (!wxControl::Enable(enable))
        return false;

    gtk_widget_set_sensitive(gtk_bin_get_child(GTK_BIN(m_widget)), enable);

        
    return true;
}

GdkWindow*
wxWebViewWebKit::GTKGetWindow(wxArrayGdkWindows& WXUNUSED(windows)) const
{
    GdkWindow* window = gtk_widget_get_parent_window(m_widget);
    return window;
}

void wxWebViewWebKit::ZoomIn()
{
    webkit_web_view_zoom_in(m_web_view);
}

void wxWebViewWebKit::ZoomOut()
{
    webkit_web_view_zoom_out(m_web_view);
}

void wxWebViewWebKit::SetWebkitZoom(float level)
{
    webkit_web_view_set_zoom_level(m_web_view, level);
}

float wxWebViewWebKit::GetWebkitZoom() const
{
    return webkit_web_view_get_zoom_level(m_web_view);
}

void wxWebViewWebKit::Stop()
{
     webkit_web_view_stop_loading(m_web_view);
}

void wxWebViewWebKit::Reload(wxWebViewReloadFlags flags)
{
    if (flags & wxWEBVIEW_RELOAD_NO_CACHE)
    {
        webkit_web_view_reload_bypass_cache(m_web_view);
    }
    else
    {
        webkit_web_view_reload(m_web_view);
    }
}

void wxWebViewWebKit::LoadURL(const wxString& url)
{
    webkit_web_view_load_uri(m_web_view, wxGTK_CONV(url));
}


void wxWebViewWebKit::GoBack()
{
    webkit_web_view_go_back(m_web_view);
}

void wxWebViewWebKit::GoForward()
{
    webkit_web_view_go_forward(m_web_view);
}


bool wxWebViewWebKit::CanGoBack() const
{
    return webkit_web_view_can_go_back(m_web_view);
}


bool wxWebViewWebKit::CanGoForward() const
{
    return webkit_web_view_can_go_forward(m_web_view);
}

void wxWebViewWebKit::ClearHistory()
{
    WebKitWebBackForwardList* history;
    history = webkit_web_view_get_back_forward_list(m_web_view);
    webkit_web_back_forward_list_clear(history);
}

void wxWebViewWebKit::EnableHistory(bool enable)
{
    WebKitWebBackForwardList* history;
    history = webkit_web_view_get_back_forward_list(m_web_view);
    if(enable)
    {
        webkit_web_back_forward_list_set_limit(history, m_historyLimit);
    }
    else
    {
        webkit_web_back_forward_list_set_limit(history, 0);
    }
}

wxVector<wxSharedPtr<wxWebViewHistoryItem> > wxWebViewWebKit::GetBackwardHistory()
{
    wxVector<wxSharedPtr<wxWebViewHistoryItem> > backhist;
    WebKitWebBackForwardList* history;
    history = webkit_web_view_get_back_forward_list(m_web_view);
    GList* list = webkit_web_back_forward_list_get_back_list_with_limit(history,
                                                                        m_historyLimit);
        for(int i = g_list_length(list) - 1; i >= 0 ; i--)
    {
        WebKitWebHistoryItem* gtkitem = (WebKitWebHistoryItem*)g_list_nth_data(list, i);
        wxWebViewHistoryItem* wxitem = new wxWebViewHistoryItem(
                                   webkit_web_history_item_get_uri(gtkitem),
                                   webkit_web_history_item_get_title(gtkitem));
        wxitem->m_histItem = gtkitem;
        wxSharedPtr<wxWebViewHistoryItem> item(wxitem);
        backhist.push_back(item);
    }
    return backhist;
}

wxVector<wxSharedPtr<wxWebViewHistoryItem> > wxWebViewWebKit::GetForwardHistory()
{
    wxVector<wxSharedPtr<wxWebViewHistoryItem> > forwardhist;
    WebKitWebBackForwardList* history;
    history = webkit_web_view_get_back_forward_list(m_web_view);
    GList* list = webkit_web_back_forward_list_get_forward_list_with_limit(history,
                                                                           m_historyLimit);
    for(guint i = 0; i < g_list_length(list); i++)
    {
        WebKitWebHistoryItem* gtkitem = (WebKitWebHistoryItem*)g_list_nth_data(list, i);
        wxWebViewHistoryItem* wxitem = new wxWebViewHistoryItem(
                                   webkit_web_history_item_get_uri(gtkitem),
                                   webkit_web_history_item_get_title(gtkitem));
        wxitem->m_histItem = gtkitem;
        wxSharedPtr<wxWebViewHistoryItem> item(wxitem);
        forwardhist.push_back(item);
    }
    return forwardhist;
}

void wxWebViewWebKit::LoadHistoryItem(wxSharedPtr<wxWebViewHistoryItem> item)
{
    WebKitWebHistoryItem* gtkitem = (WebKitWebHistoryItem*)item->m_histItem;
    if(gtkitem)
    {
        webkit_web_view_go_to_back_forward_item(m_web_view,
                                                WEBKIT_WEB_HISTORY_ITEM(gtkitem));
    }
}

bool wxWebViewWebKit::CanCut() const
{
    return webkit_web_view_can_cut_clipboard(m_web_view);
}

bool wxWebViewWebKit::CanCopy() const
{
    return webkit_web_view_can_copy_clipboard(m_web_view);
}

bool wxWebViewWebKit::CanPaste() const
{
    return webkit_web_view_can_paste_clipboard(m_web_view);
}

void wxWebViewWebKit::Cut()
{
    webkit_web_view_cut_clipboard(m_web_view);
}

void wxWebViewWebKit::Copy()
{
    webkit_web_view_copy_clipboard(m_web_view);
}

void wxWebViewWebKit::Paste()
{
    webkit_web_view_paste_clipboard(m_web_view);
}

bool wxWebViewWebKit::CanUndo() const
{
    return webkit_web_view_can_undo(m_web_view);
}

bool wxWebViewWebKit::CanRedo() const
{
    return webkit_web_view_can_redo(m_web_view);
}

void wxWebViewWebKit::Undo()
{
    webkit_web_view_undo(m_web_view);
}

void wxWebViewWebKit::Redo()
{
    webkit_web_view_redo(m_web_view);
}

wxString wxWebViewWebKit::GetCurrentURL() const
{
            return wxString::FromUTF8(webkit_web_view_get_uri(m_web_view));
}


wxString wxWebViewWebKit::GetCurrentTitle() const
{
    return wxString::FromUTF8(webkit_web_view_get_title(m_web_view));
}


wxString wxWebViewWebKit::GetPageSource() const
{
    WebKitWebFrame* frame = webkit_web_view_get_main_frame(m_web_view);
    WebKitWebDataSource* src = webkit_web_frame_get_data_source (frame);

                return wxString(webkit_web_data_source_get_data (src)->str, wxConvUTF8);
}


wxWebViewZoom wxWebViewWebKit::GetZoom() const
{
    float zoom = GetWebkitZoom();

        if (zoom <= 0.65)
    {
        return wxWEBVIEW_ZOOM_TINY;
    }
    else if (zoom > 0.65 && zoom <= 0.90)
    {
        return wxWEBVIEW_ZOOM_SMALL;
    }
    else if (zoom > 0.90 && zoom <= 1.15)
    {
        return wxWEBVIEW_ZOOM_MEDIUM;
    }
    else if (zoom > 1.15 && zoom <= 1.45)
    {
        return wxWEBVIEW_ZOOM_LARGE;
    }
    else if (zoom > 1.45)
    {
        return wxWEBVIEW_ZOOM_LARGEST;
    }

        wxFAIL;
    return wxWEBVIEW_ZOOM_MEDIUM;
}


void wxWebViewWebKit::SetZoom(wxWebViewZoom zoom)
{
        switch (zoom)
    {
        case wxWEBVIEW_ZOOM_TINY:
            SetWebkitZoom(0.6f);
            break;

        case wxWEBVIEW_ZOOM_SMALL:
            SetWebkitZoom(0.8f);
            break;

        case wxWEBVIEW_ZOOM_MEDIUM:
            SetWebkitZoom(1.0f);
            break;

        case wxWEBVIEW_ZOOM_LARGE:
            SetWebkitZoom(1.3);
            break;

        case wxWEBVIEW_ZOOM_LARGEST:
            SetWebkitZoom(1.6);
            break;

        default:
            wxFAIL;
    }
}

void wxWebViewWebKit::SetZoomType(wxWebViewZoomType type)
{
    webkit_web_view_set_full_content_zoom(m_web_view,
                                          (type == wxWEBVIEW_ZOOM_TYPE_LAYOUT ?
                                          TRUE : FALSE));
}

wxWebViewZoomType wxWebViewWebKit::GetZoomType() const
{
    gboolean fczoom = webkit_web_view_get_full_content_zoom(m_web_view);

    if (fczoom) return wxWEBVIEW_ZOOM_TYPE_LAYOUT;
    else        return wxWEBVIEW_ZOOM_TYPE_TEXT;
}

bool wxWebViewWebKit::CanSetZoomType(wxWebViewZoomType) const
{
        return true;
}

void wxWebViewWebKit::DoSetPage(const wxString& html, const wxString& baseUri)
{
    webkit_web_view_load_string (m_web_view,
                                 html.mb_str(wxConvUTF8),
                                 "text/html",
                                 "UTF-8",
                                 baseUri.mb_str(wxConvUTF8));
}

void wxWebViewWebKit::Print()
{
    WebKitWebFrame* frame = webkit_web_view_get_main_frame(m_web_view);
    webkit_web_frame_print (frame);

                    
}


bool wxWebViewWebKit::IsBusy() const
{
    return m_busy;

        
}

void wxWebViewWebKit::SetEditable(bool enable)
{
    webkit_web_view_set_editable(m_web_view, enable);
}

bool wxWebViewWebKit::IsEditable() const
{
    return webkit_web_view_get_editable(m_web_view);
}

void wxWebViewWebKit::DeleteSelection()
{
    webkit_web_view_delete_selection(m_web_view);
}

bool wxWebViewWebKit::HasSelection() const
{
    return webkit_web_view_has_selection(m_web_view);
}

void wxWebViewWebKit::SelectAll()
{
    webkit_web_view_select_all(m_web_view);
}

wxString wxWebViewWebKit::GetSelectedText() const
{
    WebKitDOMDocument* doc;
    WebKitDOMDOMWindow* win;
    WebKitDOMDOMSelection* sel;
    WebKitDOMRange* range;

    doc = webkit_web_view_get_dom_document(m_web_view);
    win = webkit_dom_document_get_default_view(WEBKIT_DOM_DOCUMENT(doc));
    sel = webkit_dom_dom_window_get_selection(WEBKIT_DOM_DOM_WINDOW(win));
    range = webkit_dom_dom_selection_get_range_at(WEBKIT_DOM_DOM_SELECTION(sel),
                                                  0, NULL);
    return wxString(webkit_dom_range_get_text(WEBKIT_DOM_RANGE(range)),
                    wxConvUTF8);
}

wxString wxWebViewWebKit::GetSelectedSource() const
{
    WebKitDOMDocument* doc;
    WebKitDOMDOMWindow* win;
    WebKitDOMDOMSelection* sel;
    WebKitDOMRange* range;
    WebKitDOMElement* div;
    WebKitDOMDocumentFragment* clone;
    WebKitDOMHTMLElement* html;

    doc = webkit_web_view_get_dom_document(m_web_view);
    win = webkit_dom_document_get_default_view(WEBKIT_DOM_DOCUMENT(doc));
    sel = webkit_dom_dom_window_get_selection(WEBKIT_DOM_DOM_WINDOW(win));
    range = webkit_dom_dom_selection_get_range_at(WEBKIT_DOM_DOM_SELECTION(sel),
                                                  0, NULL);
    div = webkit_dom_document_create_element(WEBKIT_DOM_DOCUMENT(doc), "div", NULL);

    clone = webkit_dom_range_clone_contents(WEBKIT_DOM_RANGE(range), NULL);
    webkit_dom_node_append_child(&div->parent_instance, &clone->parent_instance, NULL);
    html = (WebKitDOMHTMLElement*)div;

    return wxString(webkit_dom_html_element_get_inner_html(WEBKIT_DOM_HTML_ELEMENT(html)),
                    wxConvUTF8);
}

void wxWebViewWebKit::ClearSelection()
{
    WebKitDOMDocument* doc;
    WebKitDOMDOMWindow* win;
    WebKitDOMDOMSelection* sel;

    doc = webkit_web_view_get_dom_document(m_web_view);
    win = webkit_dom_document_get_default_view(WEBKIT_DOM_DOCUMENT(doc));
    sel = webkit_dom_dom_window_get_selection(WEBKIT_DOM_DOM_WINDOW(win));
    webkit_dom_dom_selection_remove_all_ranges(WEBKIT_DOM_DOM_SELECTION(sel));

}

wxString wxWebViewWebKit::GetPageText() const
{
    WebKitDOMDocument* doc;
    WebKitDOMHTMLElement* body;

    doc = webkit_web_view_get_dom_document(m_web_view);
    body = webkit_dom_document_get_body(WEBKIT_DOM_DOCUMENT(doc));
    return wxString(webkit_dom_html_element_get_inner_text(WEBKIT_DOM_HTML_ELEMENT(body)),
                    wxConvUTF8);
}

void wxWebViewWebKit::RunScript(const wxString& javascript)
{
    webkit_web_view_execute_script(m_web_view,
                                   javascript.mb_str(wxConvUTF8));
}

void wxWebViewWebKit::RegisterHandler(wxSharedPtr<wxWebViewHandler> handler)
{
    m_handlerList.push_back(handler);
}

void wxWebViewWebKit::EnableContextMenu(bool enable)
{
#if !WEBKIT_CHECK_VERSION(1, 10, 0)     g_object_set(webkit_web_view_get_settings(m_web_view), 
                 "enable-default-context-menu", enable, NULL);
#endif
    wxWebView::EnableContextMenu(enable);
}

long wxWebViewWebKit::Find(const wxString& text, int flags)
{
    bool newSearch = false;
    if(text != m_findText || 
       (flags & wxWEBVIEW_FIND_MATCH_CASE) != (m_findFlags & wxWEBVIEW_FIND_MATCH_CASE))
    {
        newSearch = true;
                webkit_web_view_unmark_text_matches(m_web_view);
        webkit_web_view_set_highlight_text_matches(m_web_view, false);
    }

    m_findFlags = flags;
    m_findText = text;

        if(text == "")
    {
        webkit_web_view_unmark_text_matches(m_web_view);
        webkit_web_view_set_highlight_text_matches(m_web_view, false);
        ClearSelection();
        return wxNOT_FOUND;
    }

    bool wrap = false, matchCase = false, forward = true;
    if(flags & wxWEBVIEW_FIND_WRAP)
        wrap = true;
    if(flags & wxWEBVIEW_FIND_MATCH_CASE)
        matchCase = true;
    if(flags & wxWEBVIEW_FIND_BACKWARDS)
        forward = false;

    if(newSearch)
    {
                m_findCount = webkit_web_view_mark_text_matches(m_web_view, wxGTK_CONV(text), matchCase, 0);
                m_findPosition = -1;
        return m_findCount;
    }
    else
    {
        if(forward)
            m_findPosition++;
        else
            m_findPosition--;
        if(m_findPosition < 0)
            m_findPosition += m_findCount;
        if(m_findPosition > m_findCount)
            m_findPosition -= m_findCount;
    }

        bool highlight = flags & wxWEBVIEW_FIND_HIGHLIGHT_RESULT ? true : false;
    webkit_web_view_set_highlight_text_matches(m_web_view, highlight);     

    if(!webkit_web_view_search_text(m_web_view, wxGTK_CONV(text), matchCase, forward, wrap))
    {
        m_findPosition = -1;
        ClearSelection();
        return wxNOT_FOUND;
    }
    return newSearch ? m_findCount : m_findPosition;
}

void wxWebViewWebKit::FindClear()
{
    m_findCount = 0;
    m_findFlags = 0;
    m_findText = "";
    m_findPosition = -1;
}

wxVisualAttributes
wxWebViewWebKit::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
     return GetDefaultAttributesFromGTKWidget(webkit_web_view_new());
}


#endif 