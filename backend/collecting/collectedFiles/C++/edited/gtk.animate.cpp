
#include "wx/wxprec.h"

#if wxUSE_ANIMATIONCTRL && !defined(__WXUNIVERSAL__)

#include "wx/animate.h"

#ifndef WX_PRECOMP
    #include "wx/image.h"
    #include "wx/log.h"
    #include "wx/stream.h"
#endif

#include "wx/wfstream.h"
#include "wx/gtk/private.h"

#include <gtk/gtk.h>



void gdk_pixbuf_area_updated(GdkPixbufLoader *loader,
                             gint             WXUNUSED(x),
                             gint             WXUNUSED(y),
                             gint             WXUNUSED(width),
                             gint             WXUNUSED(height),
                             wxAnimation      *anim)
{
    if (anim && anim->GetPixbuf() == NULL)
    {
                        anim->SetPixbuf(gdk_pixbuf_loader_get_animation(loader));
    }
}



wxIMPLEMENT_DYNAMIC_CLASS(wxAnimation, wxAnimationBase);

wxAnimation::wxAnimation(const wxAnimation& that)
    : base_type(that)
{
    m_pixbuf = that.m_pixbuf;
    if (m_pixbuf)
        g_object_ref(m_pixbuf);
}

wxAnimation::wxAnimation(GdkPixbufAnimation *p)
{
    m_pixbuf = p;
    if ( m_pixbuf )
        g_object_ref(m_pixbuf);
}

wxAnimation& wxAnimation::operator=(const wxAnimation& that)
{
    if (this != &that)
    {
        base_type::operator=(that);
        UnRef();
        m_pixbuf = that.m_pixbuf;
        if (m_pixbuf)
            g_object_ref(m_pixbuf);
    }
    return *this;
}

bool wxAnimation::LoadFile(const wxString &name, wxAnimationType WXUNUSED(type))
{
    UnRef();
    m_pixbuf = gdk_pixbuf_animation_new_from_file(wxGTK_CONV_FN(name), NULL);
    return IsOk();
}

bool wxAnimation::Load(wxInputStream &stream, wxAnimationType type)
{
    UnRef();

    char anim_type[12];
    switch (type)
    {
    case wxANIMATION_TYPE_GIF:
        strcpy(anim_type, "gif");
        break;

    case wxANIMATION_TYPE_ANI:
        strcpy(anim_type, "ani");
        break;

    default:
        anim_type[0] = '\0';
        break;
    }

        GError *error = NULL;
    GdkPixbufLoader *loader;
    if (type != wxANIMATION_TYPE_INVALID && type != wxANIMATION_TYPE_ANY)
        loader = gdk_pixbuf_loader_new_with_type(anim_type, &error);
    else
        loader = gdk_pixbuf_loader_new();

    if (!loader ||
        error != NULL)      {
        wxLogDebug(wxT("Could not create the loader for '%s' animation type: %s"),
                   anim_type, error->message);
        return false;
    }

        g_signal_connect(loader, "area-updated", G_CALLBACK(gdk_pixbuf_area_updated), this);

    guchar buf[2048];
    bool data_written = false;
    while (stream.IsOk())
    {
                if (!stream.Read(buf, sizeof(buf)) &&
            stream.GetLastError() != wxSTREAM_EOF)           {
                        gdk_pixbuf_loader_close(loader, NULL);
            return false;
        }

                if (!gdk_pixbuf_loader_write(loader, buf, stream.LastRead(), &error))
        {
            wxLogDebug(wxT("Could not write to the loader: %s"), error->message);

                        gdk_pixbuf_loader_close(loader, NULL);
            return false;
        }

        data_written = true;
    }

    if (!data_written)
    {
        wxLogDebug("Could not read data from the stream...");
        return false;
    }

                if (!gdk_pixbuf_loader_close(loader, &error))
    {
        wxLogDebug(wxT("Could not close the loader: %s"), error->message);
        return false;
    }

        return data_written;
}

wxImage wxAnimation::GetFrame(unsigned int WXUNUSED(frame)) const
{
    return wxNullImage;
}

wxSize wxAnimation::GetSize() const
{
    return wxSize(gdk_pixbuf_animation_get_width(m_pixbuf),
                  gdk_pixbuf_animation_get_height(m_pixbuf));
}

void wxAnimation::UnRef()
{
    if (m_pixbuf)
        g_object_unref(m_pixbuf);
    m_pixbuf = NULL;
}

void wxAnimation::SetPixbuf(GdkPixbufAnimation* p)
{
    UnRef();
    m_pixbuf = p;
    if (m_pixbuf)
        g_object_ref(m_pixbuf);
}


wxIMPLEMENT_DYNAMIC_CLASS(wxAnimationCtrl, wxAnimationCtrlBase);
wxBEGIN_EVENT_TABLE(wxAnimationCtrl, wxAnimationCtrlBase)
    EVT_TIMER(wxID_ANY, wxAnimationCtrl::OnTimer)
wxEND_EVENT_TABLE()

void wxAnimationCtrl::Init()
{
    m_anim = NULL;
    m_iter = NULL;
    m_bPlaying = false;
}

bool wxAnimationCtrl::Create( wxWindow *parent, wxWindowID id,
                              const wxAnimation& anim,
                              const wxPoint& pos,
                              const wxSize& size,
                              long style,
                              const wxString& name)
{
    if (!PreCreation( parent, pos, size ) ||
        !base_type::CreateBase(parent, id, pos, size, style & wxWINDOW_STYLE_MASK,
                               wxDefaultValidator, name))
    {
        wxFAIL_MSG( wxT("wxAnimationCtrl creation failed") );
        return false;
    }

    SetWindowStyle(style);

    m_widget = gtk_image_new();
    g_object_ref(m_widget);

    m_parent->DoAddChild( this );

    PostCreation(size);
    SetInitialSize(size);

    if (anim.IsOk())
        SetAnimation(anim);

        m_timer.SetOwner(this);

    return true;
}

wxAnimationCtrl::~wxAnimationCtrl()
{
    ResetAnim();
    ResetIter();
}

bool wxAnimationCtrl::LoadFile(const wxString &filename, wxAnimationType type)
{
    wxFileInputStream fis(filename);
    if (!fis.IsOk())
        return false;
    return Load(fis, type);
}

bool wxAnimationCtrl::Load(wxInputStream& stream, wxAnimationType type)
{
    wxAnimation anim;
    if ( !anim.Load(stream, type) || !anim.IsOk() )
        return false;

    SetAnimation(anim);
    return true;
}

void wxAnimationCtrl::SetAnimation(const wxAnimation &anim)
{
    if (IsPlaying())
        Stop();

    ResetAnim();
    ResetIter();

        m_anim = anim.GetPixbuf();

        if (m_anim)
    {
                g_object_ref(m_anim);

        if (!this->HasFlag(wxAC_NO_AUTORESIZE))
            FitToAnimation();
    }

    DisplayStaticImage();
}

void wxAnimationCtrl::FitToAnimation()
{
    if (!m_anim)
        return;

    int w = gdk_pixbuf_animation_get_width(m_anim),
        h = gdk_pixbuf_animation_get_height(m_anim);

        SetSize(w, h);
}

void wxAnimationCtrl::ResetAnim()
{
    if (m_anim)
        g_object_unref(m_anim);
    m_anim = NULL;
}

void wxAnimationCtrl::ResetIter()
{
    if (m_iter)
        g_object_unref(m_iter);
    m_iter = NULL;
}

bool wxAnimationCtrl::Play()
{
    if (m_anim == NULL)
        return false;

        ResetIter();
    m_iter = gdk_pixbuf_animation_get_iter (m_anim, NULL);
    m_bPlaying = true;

            int n = gdk_pixbuf_animation_iter_get_delay_time(m_iter);
    if (n >= 0)
        m_timer.Start(n, true);

    return true;
}

void wxAnimationCtrl::Stop()
{
        if (IsPlaying())
        m_timer.Stop();
    m_bPlaying = false;

    ResetIter();
    DisplayStaticImage();
}

void wxAnimationCtrl::DisplayStaticImage()
{
    wxASSERT(!IsPlaying());

        UpdateStaticImage();

    if (m_bmpStaticReal.IsOk())
    {
                gtk_image_set_from_pixbuf(GTK_IMAGE(m_widget),
                                      m_bmpStaticReal.GetPixbuf());
    }
    else
    {
        if (m_anim)
        {
                                    gtk_image_set_from_pixbuf(GTK_IMAGE(m_widget),
                                        gdk_pixbuf_animation_get_static_image(m_anim));
        }
        else
        {
            ClearToBackgroundColour();
        }
    }
}

bool wxAnimationCtrl::IsPlaying() const
{
                    return m_bPlaying;
}

wxSize wxAnimationCtrl::DoGetBestSize() const
{
    if (m_anim && !this->HasFlag(wxAC_NO_AUTORESIZE))
    {
        return wxSize(gdk_pixbuf_animation_get_width(m_anim),
                      gdk_pixbuf_animation_get_height(m_anim));
    }

    return wxSize(100,100);
}

void wxAnimationCtrl::ClearToBackgroundColour()
{
    wxSize sz = GetClientSize();
    GdkPixbuf *newpix = gdk_pixbuf_new(GDK_COLORSPACE_RGB, false, 8,
                                       sz.GetWidth(), sz.GetHeight());
    if (!newpix)
        return;

    wxColour clr = GetBackgroundColour();
    guint32 col = (clr.Red() << 24) | (clr.Green() << 16) | (clr.Blue() << 8);
    gdk_pixbuf_fill(newpix, col);

    gtk_image_set_from_pixbuf(GTK_IMAGE(m_widget), newpix);
    g_object_unref(newpix);
}

bool wxAnimationCtrl::SetBackgroundColour( const wxColour &colour )
{
                if (!wxControl::SetBackgroundColour(colour))
        return false;

                if ( !IsPlaying() )
        DisplayStaticImage();

    return true;
}



void wxAnimationCtrl::OnTimer(wxTimerEvent& WXUNUSED(ev))
{
    wxASSERT(m_iter != NULL);

            if (gdk_pixbuf_animation_iter_advance(m_iter, NULL))
    {
                int n = gdk_pixbuf_animation_iter_get_delay_time(m_iter);
        if (n >= 0)
            m_timer.Start(n, true);

        gtk_image_set_from_pixbuf(GTK_IMAGE(m_widget),
                                  gdk_pixbuf_animation_iter_get_pixbuf(m_iter));
    }
    else
    {
                m_timer.Start(10, true);
    }
}

#endif      