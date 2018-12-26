

#include "wx/wxprec.h"

#if defined(__BORLANDC__)
    #pragma hdrstop
#endif

#include "wx/artprov.h"

#include <gtk/gtk.h>
#include "wx/gtk/private.h"

#if !GTK_CHECK_VERSION(2,8,0)
    #define GTK_STOCK_FULLSCREEN "gtk-fullscreen"
#endif


class wxGTK2ArtProvider : public wxArtProvider
{
protected:
    virtual wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client,
                                  const wxSize& size) wxOVERRIDE;
    virtual wxIconBundle CreateIconBundle(const wxArtID& id,
                                          const wxArtClient& client) wxOVERRIDE;
};

 void wxArtProvider::InitNativeProvider()
{
    PushBack(new wxGTK2ArtProvider);
}


namespace
{

wxString wxArtIDToStock(const wxArtID& id)
{
    #define ART(wxid, gtkid) \
           if (id == wxid) return gtkid;

    ART(wxART_ERROR,                               GTK_STOCK_DIALOG_ERROR)
    ART(wxART_INFORMATION,                         GTK_STOCK_DIALOG_INFO)
    ART(wxART_WARNING,                             GTK_STOCK_DIALOG_WARNING)
    ART(wxART_QUESTION,                            GTK_STOCK_DIALOG_QUESTION)

        ART(wxART_HELP_SETTINGS,                       GTK_STOCK_SELECT_FONT)
        ART(wxART_HELP_FOLDER,                         GTK_STOCK_DIRECTORY)
    ART(wxART_HELP_PAGE,                           GTK_STOCK_FILE)
    ART(wxART_MISSING_IMAGE,                       GTK_STOCK_MISSING_IMAGE)
    ART(wxART_ADD_BOOKMARK,                        GTK_STOCK_ADD)
    ART(wxART_DEL_BOOKMARK,                        GTK_STOCK_REMOVE)
    ART(wxART_GO_BACK,                             GTK_STOCK_GO_BACK)
    ART(wxART_GO_FORWARD,                          GTK_STOCK_GO_FORWARD)
    ART(wxART_GO_UP,                               GTK_STOCK_GO_UP)
    ART(wxART_GO_DOWN,                             GTK_STOCK_GO_DOWN)
    ART(wxART_GO_TO_PARENT,                        GTK_STOCK_GO_UP)
    ART(wxART_GO_HOME,                             GTK_STOCK_HOME)
    ART(wxART_GOTO_FIRST,                          GTK_STOCK_GOTO_FIRST)
    ART(wxART_GOTO_LAST,                           GTK_STOCK_GOTO_LAST)
    ART(wxART_FILE_OPEN,                           GTK_STOCK_OPEN)
    ART(wxART_PRINT,                               GTK_STOCK_PRINT)
    ART(wxART_HELP,                                GTK_STOCK_HELP)
    ART(wxART_TIP,                                 GTK_STOCK_DIALOG_INFO)
                ART(wxART_FOLDER,                              GTK_STOCK_DIRECTORY)
    ART(wxART_FOLDER_OPEN,                         GTK_STOCK_DIRECTORY)
        ART(wxART_EXECUTABLE_FILE,                     GTK_STOCK_EXECUTE)
    ART(wxART_NORMAL_FILE,                         GTK_STOCK_FILE)
    ART(wxART_TICK_MARK,                           GTK_STOCK_APPLY)
    ART(wxART_CROSS_MARK,                          GTK_STOCK_CANCEL)

    ART(wxART_FLOPPY,                              GTK_STOCK_FLOPPY)
    ART(wxART_CDROM,                               GTK_STOCK_CDROM)
    ART(wxART_HARDDISK,                            GTK_STOCK_HARDDISK)
    ART(wxART_REMOVABLE,                           GTK_STOCK_HARDDISK)

    ART(wxART_FILE_SAVE,                           GTK_STOCK_SAVE)
    ART(wxART_FILE_SAVE_AS,                        GTK_STOCK_SAVE_AS)

    ART(wxART_COPY,                                GTK_STOCK_COPY)
    ART(wxART_CUT,                                 GTK_STOCK_CUT)
    ART(wxART_PASTE,                               GTK_STOCK_PASTE)
    ART(wxART_DELETE,                              GTK_STOCK_DELETE)
    ART(wxART_NEW,                                 GTK_STOCK_NEW)

    ART(wxART_UNDO,                                GTK_STOCK_UNDO)
    ART(wxART_REDO,                                GTK_STOCK_REDO)

    ART(wxART_PLUS,                                GTK_STOCK_ADD)
    ART(wxART_MINUS,                               GTK_STOCK_REMOVE)

    ART(wxART_CLOSE,                               GTK_STOCK_CLOSE)
    ART(wxART_QUIT,                                GTK_STOCK_QUIT)

    ART(wxART_FIND,                                GTK_STOCK_FIND)
    ART(wxART_FIND_AND_REPLACE,                    GTK_STOCK_FIND_AND_REPLACE)
    ART(wxART_FULL_SCREEN,                         GTK_STOCK_FULLSCREEN)

    #undef ART

                return id;
}

GtkIconSize ArtClientToIconSize(const wxArtClient& client)
{
    if (client == wxART_TOOLBAR)
        return GTK_ICON_SIZE_LARGE_TOOLBAR;
    else if (client == wxART_MENU || client == wxART_FRAME_ICON)
        return GTK_ICON_SIZE_MENU;
    else if (client == wxART_CMN_DIALOG || client == wxART_MESSAGE_BOX)
        return GTK_ICON_SIZE_DIALOG;
    else if (client == wxART_BUTTON)
        return GTK_ICON_SIZE_BUTTON;
    else
        return GTK_ICON_SIZE_INVALID; }

GtkIconSize FindClosestIconSize(const wxSize& size)
{
    #define NUM_SIZES 6
    static struct
    {
        GtkIconSize icon;
        gint x, y;
    } s_sizes[NUM_SIZES];
    static bool s_sizesInitialized = false;

    if (!s_sizesInitialized)
    {
        s_sizes[0].icon = GTK_ICON_SIZE_MENU;
        s_sizes[1].icon = GTK_ICON_SIZE_SMALL_TOOLBAR;
        s_sizes[2].icon = GTK_ICON_SIZE_LARGE_TOOLBAR;
        s_sizes[3].icon = GTK_ICON_SIZE_BUTTON;
        s_sizes[4].icon = GTK_ICON_SIZE_DND;
        s_sizes[5].icon = GTK_ICON_SIZE_DIALOG;
        for (size_t i = 0; i < NUM_SIZES; i++)
        {
            gtk_icon_size_lookup(s_sizes[i].icon,
                                 &s_sizes[i].x, &s_sizes[i].y);
        }
        s_sizesInitialized = true;
    }

    GtkIconSize best = GTK_ICON_SIZE_DIALOG;     unsigned distance = INT_MAX;
    for (size_t i = 0; i < NUM_SIZES; i++)
    {
                if (size.x > s_sizes[i].x || size.y > s_sizes[i].y)
            continue;

        unsigned dist = (size.x - s_sizes[i].x) * (size.x - s_sizes[i].x) +
                        (size.y - s_sizes[i].y) * (size.y - s_sizes[i].y);
        if (dist == 0)
            return s_sizes[i].icon;
        else if (dist < distance)
        {
            distance = dist;
            best = s_sizes[i].icon;
        }
    }
    return best;
}

GdkPixbuf *CreateStockIcon(const char *stockid, GtkIconSize size)
{
                            
    GtkWidget* widget = wxGTKPrivate::GetButtonWidget();
#ifdef __WXGTK3__
    GtkStyleContext* sc = gtk_widget_get_style_context(widget);
    GtkIconSet* iconset = gtk_style_context_lookup_icon_set(sc, stockid);
    GdkPixbuf* pixbuf = NULL;
    if (iconset)
        pixbuf = gtk_icon_set_render_icon_pixbuf(iconset, sc, size);
    return pixbuf;
#else
    GtkStyle* style = gtk_widget_get_style(widget);
    GtkIconSet* iconset = gtk_style_lookup_icon_set(style, stockid);

    if (!iconset)
        return NULL;

    return gtk_icon_set_render_icon(iconset, style,
                                    gtk_widget_get_default_direction(),
                                    GTK_STATE_NORMAL, size, NULL, NULL);
#endif
}

GdkPixbuf *CreateThemeIcon(const char *iconname, int size)
{
    return gtk_icon_theme_load_icon
           (
               gtk_icon_theme_get_default(),
               iconname,
               size,
               (GtkIconLookupFlags)0,
               NULL
           );
}


GdkPixbuf *CreateGtkIcon(const char *icon_name,
                         GtkIconSize stock_size, const wxSize& pixel_size)
{
        GdkPixbuf *pixbuf = CreateStockIcon(icon_name, stock_size);
    if ( pixbuf )
        return pixbuf;

        wxSize size(pixel_size);
    if ( pixel_size == wxDefaultSize )
        gtk_icon_size_lookup(stock_size, &size.x, &size.y);
    return CreateThemeIcon(icon_name, size.x);
}

template<typename SizeType, typename LoaderFunc>
wxIconBundle DoCreateIconBundle(const char *stockid,
                                const SizeType *sizes_from,
                                const SizeType *sizes_to,
                                LoaderFunc get_icon)

{
    wxIconBundle bundle;

    for ( const SizeType *i = sizes_from; i != sizes_to; ++i )
    {
        GdkPixbuf *pixbuf = get_icon(stockid, *i);
        if ( !pixbuf )
            continue;

        wxIcon icon;
        icon.CopyFromBitmap(wxBitmap(pixbuf));
        bundle.AddIcon(icon);
    }

    return bundle;
}

} 
wxBitmap wxGTK2ArtProvider::CreateBitmap(const wxArtID& id,
                                         const wxArtClient& client,
                                         const wxSize& size)
{
    const wxString stockid = wxArtIDToStock(id);

    GtkIconSize stocksize = (size == wxDefaultSize) ?
                                ArtClientToIconSize(client) :
                                FindClosestIconSize(size);
        if (stocksize == GTK_ICON_SIZE_INVALID)
        stocksize = GTK_ICON_SIZE_BUTTON;

    GdkPixbuf *pixbuf = CreateGtkIcon(stockid.utf8_str(), stocksize, size);

    if (pixbuf && size != wxDefaultSize &&
        (size.x != gdk_pixbuf_get_width(pixbuf) ||
         size.y != gdk_pixbuf_get_height(pixbuf)))
    {
        GdkPixbuf *p2 = gdk_pixbuf_scale_simple(pixbuf, size.x, size.y,
                                                GDK_INTERP_BILINEAR);
        if (p2)
        {
            g_object_unref (pixbuf);
            pixbuf = p2;
        }
    }

    return wxBitmap(pixbuf);
}

wxIconBundle
wxGTK2ArtProvider::CreateIconBundle(const wxArtID& id,
                                    const wxArtClient& WXUNUSED(client))
{
    wxIconBundle bundle;
    const wxString stockid = wxArtIDToStock(id);

        GtkWidget* widget = wxGTKPrivate::GetButtonWidget();
#ifdef __WXGTK3__
    GtkStyleContext* sc = gtk_widget_get_style_context(widget);
    GtkIconSet* iconset = gtk_style_context_lookup_icon_set(sc, stockid.utf8_str());
#else
    GtkStyle* style = gtk_widget_get_style(widget);
    GtkIconSet* iconset = gtk_style_lookup_icon_set(style, stockid.utf8_str());
#endif
    if ( iconset )
    {
        GtkIconSize *sizes;
        gint n_sizes;
        gtk_icon_set_get_sizes(iconset, &sizes, &n_sizes);
        bundle = DoCreateIconBundle
                              (
                                  stockid.utf8_str(),
                                  sizes, sizes + n_sizes,
                                  &CreateStockIcon
                              );
        g_free(sizes);
        return bundle;
    }

        gint *sizes = gtk_icon_theme_get_icon_sizes
                  (
                      gtk_icon_theme_get_default(),
                      stockid.utf8_str()
                  );
    if ( !sizes )
        return bundle;

    gint *last = sizes;
    while ( *last )
        last++;

    bundle = DoCreateIconBundle
                          (
                              stockid.utf8_str(),
                              sizes, last,
                              &CreateThemeIcon
                          );
    g_free(sizes);

    return bundle;
}



wxSize wxArtProvider::GetNativeSizeHint(const wxArtClient& client)
{
        GtkIconSize gtk_size = ArtClientToIconSize(client);
        if (gtk_size == GTK_ICON_SIZE_INVALID)
        return wxDefaultSize;
    gint width, height;
    gtk_icon_size_lookup( gtk_size, &width, &height);
    return wxSize(width, height);
}
