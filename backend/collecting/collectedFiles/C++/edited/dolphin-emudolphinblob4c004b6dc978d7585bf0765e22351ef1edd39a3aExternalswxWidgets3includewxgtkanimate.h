
#ifndef _WX_GTKANIMATEH__
#define _WX_GTKANIMATEH__

typedef struct _GdkPixbufAnimation GdkPixbufAnimation;
typedef struct _GdkPixbufAnimationIter GdkPixbufAnimationIter;


class WXDLLIMPEXP_ADV wxAnimation : public wxAnimationBase
{
public:
    wxAnimation(const wxString &name, wxAnimationType type = wxANIMATION_TYPE_ANY)
        : m_pixbuf(NULL) { LoadFile(name, type); }
    wxAnimation(GdkPixbufAnimation *p = NULL);
    wxAnimation(const wxAnimation&);
    ~wxAnimation() { UnRef(); }

    wxAnimation& operator= (const wxAnimation&);

    virtual bool IsOk() const wxOVERRIDE
        { return m_pixbuf != NULL; }


    
    virtual unsigned int GetFrameCount() const wxOVERRIDE { return 0; }
    virtual wxImage GetFrame(unsigned int frame) const wxOVERRIDE;

            virtual int GetDelay(unsigned int WXUNUSED(frame)) const wxOVERRIDE { return 0; }

    virtual wxSize GetSize() const wxOVERRIDE;

    virtual bool LoadFile(const wxString &name, wxAnimationType type = wxANIMATION_TYPE_ANY) wxOVERRIDE;
    virtual bool Load(wxInputStream &stream, wxAnimationType type = wxANIMATION_TYPE_ANY) wxOVERRIDE;

    public:     
    GdkPixbufAnimation *GetPixbuf() const
        { return m_pixbuf; }
    void SetPixbuf(GdkPixbufAnimation* p);

protected:
    GdkPixbufAnimation *m_pixbuf;

private:
    void UnRef();

    typedef wxAnimationBase base_type;
    wxDECLARE_DYNAMIC_CLASS(wxAnimation);
};



#define wxAN_FIT_ANIMATION       0x0010

class WXDLLIMPEXP_ADV wxAnimationCtrl: public wxAnimationCtrlBase
{
public:
    wxAnimationCtrl() { Init(); }
    wxAnimationCtrl(wxWindow *parent,
                        wxWindowID id,
                        const wxAnimation& anim = wxNullAnimation,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize,
                        long style = wxAC_DEFAULT_STYLE,
                        const wxString& name = wxAnimationCtrlNameStr)
    {
        Init();

        Create(parent, id, anim, pos, size, style, name);
    }

    bool Create(wxWindow *parent, wxWindowID id,
                const wxAnimation& anim = wxNullAnimation,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = wxAC_DEFAULT_STYLE,
                const wxString& name = wxAnimationCtrlNameStr);

    ~wxAnimationCtrl();

public:     
    void OnTimer(wxTimerEvent &);

public:     
    virtual bool LoadFile(const wxString& filename, wxAnimationType type = wxANIMATION_TYPE_ANY) wxOVERRIDE;
    virtual bool Load(wxInputStream& stream, wxAnimationType type = wxANIMATION_TYPE_ANY) wxOVERRIDE;

    virtual void SetAnimation(const wxAnimation &anim) wxOVERRIDE;
    virtual wxAnimation GetAnimation() const wxOVERRIDE
        { return wxAnimation(m_anim); }

    virtual bool Play() wxOVERRIDE;
    virtual void Stop() wxOVERRIDE;

    virtual bool IsPlaying() const wxOVERRIDE;

    bool SetBackgroundColour( const wxColour &colour ) wxOVERRIDE;

protected:

    virtual void DisplayStaticImage() wxOVERRIDE;
    virtual wxSize DoGetBestSize() const wxOVERRIDE;
    void FitToAnimation();
    void ClearToBackgroundColour();

    void ResetAnim();
    void ResetIter();

protected:      
    GdkPixbufAnimation *m_anim;
    GdkPixbufAnimationIter *m_iter;

    wxTimer m_timer;
    bool m_bPlaying;

private:
    typedef wxAnimationCtrlBase base_type;

    void Init();

    wxDECLARE_DYNAMIC_CLASS(wxAnimationCtrl);
    wxDECLARE_EVENT_TABLE();
};

#endif 