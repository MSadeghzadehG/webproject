


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifdef wxHAS_ANY_BUTTON

#include "wx/anybutton.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/brush.h"
    #include "wx/panel.h"
    #include "wx/bmpbuttn.h"
    #include "wx/settings.h"
    #include "wx/dcscreen.h"
    #include "wx/dcclient.h"
    #include "wx/toplevel.h"
    #include "wx/msw/wrapcctl.h"
    #include "wx/msw/private.h"
    #include "wx/msw/missing.h"
#endif

#include "wx/imaglist.h"
#include "wx/stockitem.h"
#include "wx/msw/private/button.h"
#include "wx/msw/private/dc.h"
#include "wx/msw/uxtheme.h"
#include "wx/private/window.h"

#if wxUSE_MARKUP
    #include "wx/generic/private/markuptext.h"
#endif 
using namespace wxMSWImpl;

#if wxUSE_UXTHEME
        #ifndef BP_PUSHBUTTON
        #define BP_PUSHBUTTON 1

        #define PBS_NORMAL    1
        #define PBS_HOT       2
        #define PBS_PRESSED   3
        #define PBS_DISABLED  4
        #define PBS_DEFAULTED 5

        #define TMT_CONTENTMARGINS 3602
    #endif

            #ifndef BCM_SETIMAGELIST
        #define BCM_SETIMAGELIST    0x1602
        #define BCM_SETTEXTMARGIN   0x1604

        enum
        {
            BUTTON_IMAGELIST_ALIGN_LEFT,
            BUTTON_IMAGELIST_ALIGN_RIGHT,
            BUTTON_IMAGELIST_ALIGN_TOP,
            BUTTON_IMAGELIST_ALIGN_BOTTOM
        };

        struct BUTTON_IMAGELIST
        {
            HIMAGELIST himl;
            RECT margin;
            UINT uAlign;
        };
    #endif
#endif 
#ifndef WM_THEMECHANGED
    #define WM_THEMECHANGED     0x031A
#endif

#ifndef ODS_NOACCEL
    #define ODS_NOACCEL         0x0100
#endif

#ifndef ODS_NOFOCUSRECT
    #define ODS_NOFOCUSRECT     0x0200
#endif

#if wxUSE_UXTHEME
extern wxWindowMSW *wxWindowBeingErased; #endif 


class wxButtonImageData
{
public:
    wxButtonImageData() { }
    virtual ~wxButtonImageData() { }

    virtual wxBitmap GetBitmap(wxAnyButton::State which) const = 0;
    virtual void SetBitmap(const wxBitmap& bitmap, wxAnyButton::State which) = 0;

    virtual wxSize GetBitmapMargins() const = 0;
    virtual void SetBitmapMargins(wxCoord x, wxCoord y) = 0;

    virtual wxDirection GetBitmapPosition() const = 0;
    virtual void SetBitmapPosition(wxDirection dir) = 0;

private:
    wxDECLARE_NO_COPY_CLASS(wxButtonImageData);
};

namespace
{

const int OD_BUTTON_MARGIN = 4;

class wxODButtonImageData : public wxButtonImageData
{
public:
    wxODButtonImageData(wxAnyButton *btn, const wxBitmap& bitmap)
    {
        SetBitmap(bitmap, wxAnyButton::State_Normal);
#if wxUSE_IMAGE
        SetBitmap(bitmap.ConvertToDisabled(), wxAnyButton::State_Disabled);
#endif
        m_dir = wxLEFT;

                        if ( btn->ShowsLabel() )
        {
            m_margin.x = btn->GetCharWidth();
            m_margin.y = btn->GetCharHeight() / 2;
        }
    }

    virtual wxBitmap GetBitmap(wxAnyButton::State which) const
    {
        return m_bitmaps[which];
    }

    virtual void SetBitmap(const wxBitmap& bitmap, wxAnyButton::State which)
    {
        m_bitmaps[which] = bitmap;
    }

    virtual wxSize GetBitmapMargins() const
    {
        return m_margin;
    }

    virtual void SetBitmapMargins(wxCoord x, wxCoord y)
    {
        m_margin = wxSize(x, y);
    }

    virtual wxDirection GetBitmapPosition() const
    {
        return m_dir;
    }

    virtual void SetBitmapPosition(wxDirection dir)
    {
        m_dir = dir;
    }

private:
            wxBitmap m_bitmaps[wxAnyButton::State_Max];
    wxSize m_margin;
    wxDirection m_dir;

    wxDECLARE_NO_COPY_CLASS(wxODButtonImageData);
};

#if wxUSE_UXTHEME

const int XP_BUTTON_EXTRA_MARGIN = 1;

class wxXPButtonImageData : public wxButtonImageData
{
public:
            wxXPButtonImageData(wxAnyButton *btn, const wxBitmap& bitmap)
        : m_iml(bitmap.GetWidth(), bitmap.GetHeight(),
                !bitmap.HasAlpha() ,
                wxAnyButton::State_Max + 1 ),
          m_hwndBtn(GetHwndOf(btn))
    {
                for ( int n = 0; n < wxAnyButton::State_Max; n++ )
        {
#if wxUSE_IMAGE
            m_iml.Add(n == wxAnyButton::State_Disabled ? bitmap.ConvertToDisabled()
                                                    : bitmap);
#else
            m_iml.Add(bitmap);
#endif
        }

                                                                                                        m_iml.Add(bitmap);

        m_data.himl = GetHimagelistOf(&m_iml);

                m_data.margin.left =
        m_data.margin.right =
        m_data.margin.top =
        m_data.margin.bottom = 0;

                m_data.uAlign = BUTTON_IMAGELIST_ALIGN_LEFT;

        UpdateImageInfo();
    }

    virtual wxBitmap GetBitmap(wxAnyButton::State which) const
    {
        return m_iml.GetBitmap(which);
    }

    virtual void SetBitmap(const wxBitmap& bitmap, wxAnyButton::State which)
    {
        m_iml.Replace(which, bitmap);

                        if ( which == wxAnyButton::State_Focused )
            m_iml.Replace(wxAnyButton::State_Max, bitmap);

        UpdateImageInfo();
    }

    virtual wxSize GetBitmapMargins() const
    {
        return wxSize(m_data.margin.left, m_data.margin.top);
    }

    virtual void SetBitmapMargins(wxCoord x, wxCoord y)
    {
        RECT& margin = m_data.margin;
        margin.left =
        margin.right = x;
        margin.top =
        margin.bottom = y;

        if ( !::SendMessage(m_hwndBtn, BCM_SETTEXTMARGIN, 0, (LPARAM)&margin) )
        {
            wxLogDebug("SendMessage(BCM_SETTEXTMARGIN) failed");
        }
    }

    virtual wxDirection GetBitmapPosition() const
    {
        switch ( m_data.uAlign )
        {
            default:
                wxFAIL_MSG( "invalid image alignment" );
                
            case BUTTON_IMAGELIST_ALIGN_LEFT:
                return wxLEFT;

            case BUTTON_IMAGELIST_ALIGN_RIGHT:
                return wxRIGHT;

            case BUTTON_IMAGELIST_ALIGN_TOP:
                return wxTOP;

            case BUTTON_IMAGELIST_ALIGN_BOTTOM:
                return wxBOTTOM;
        }
    }

    virtual void SetBitmapPosition(wxDirection dir)
    {
        UINT alignNew;
        switch ( dir )
        {
            default:
                wxFAIL_MSG( "invalid direction" );
                
            case wxLEFT:
                alignNew = BUTTON_IMAGELIST_ALIGN_LEFT;
                break;

            case wxRIGHT:
                alignNew = BUTTON_IMAGELIST_ALIGN_RIGHT;
                break;

            case wxTOP:
                alignNew = BUTTON_IMAGELIST_ALIGN_TOP;
                break;

            case wxBOTTOM:
                alignNew = BUTTON_IMAGELIST_ALIGN_BOTTOM;
                break;
        }

        if ( alignNew != m_data.uAlign )
        {
            m_data.uAlign = alignNew;
            UpdateImageInfo();
        }
    }

private:
    void UpdateImageInfo()
    {
        if ( !::SendMessage(m_hwndBtn, BCM_SETIMAGELIST, 0, (LPARAM)&m_data) )
        {
            wxLogDebug("SendMessage(BCM_SETIMAGELIST) failed");
        }
    }

            wxImageList m_iml;

        BUTTON_IMAGELIST m_data;

        const HWND m_hwndBtn;


    wxDECLARE_NO_COPY_CLASS(wxXPButtonImageData);
};

#endif 
} 



void wxMSWButton::UpdateMultilineStyle(HWND hwnd, const wxString& label)
{
                        long styleOld = ::GetWindowLong(hwnd, GWL_STYLE),
         styleNew;
    if ( label.find(wxT('\n')) != wxString::npos )
        styleNew = styleOld | BS_MULTILINE;
    else
        styleNew = styleOld & ~BS_MULTILINE;

    if ( styleNew != styleOld )
        ::SetWindowLong(hwnd, GWL_STYLE, styleNew);
}

wxSize wxMSWButton::GetFittingSize(wxWindow *win,
                                   const wxSize& sizeLabel,
                                   int flags)
{
    wxSize sizeBtn = sizeLabel;

            if ( flags & Size_ExactFit )
    {
                        sizeBtn.x += (3*win->GetCharWidth());
    }
    else
    {
        sizeBtn.x += 3*win->GetCharWidth();
        sizeBtn.y += win->GetCharHeight()/2;
    }

        if ( flags & Size_AuthNeeded )
        sizeBtn.x += wxSystemSettings::GetMetric(wxSYS_SMALLICON_X);

    return sizeBtn;
}

wxSize wxMSWButton::ComputeBestFittingSize(wxControl *btn, int flags)
{
    wxClientDC dc(btn);

    wxSize sizeBtn;
    dc.GetMultiLineTextExtent(btn->GetLabelText(), &sizeBtn.x, &sizeBtn.y);

    return GetFittingSize(btn, sizeBtn, flags);
}

wxSize wxMSWButton::IncreaseToStdSizeAndCache(wxControl *btn, const wxSize& size)
{
    wxSize sizeBtn(size);

                        const wxSize sizeDef = btn->ConvertDialogToPixels(wxSize(50, 14));

                const bool incToStdSize = !btn->HasFlag(wxBU_EXACTFIT);
    if ( incToStdSize )
    {
        if ( sizeBtn.x < sizeDef.x )
            sizeBtn.x = sizeDef.x;
    }

                if ( incToStdSize || !btn->GetLabel().empty() )
    {
        if ( sizeBtn.y < sizeDef.y )
            sizeBtn.y = sizeDef.y;
    }

    btn->CacheBestSize(sizeBtn);

    return sizeBtn;
}


wxAnyButton::~wxAnyButton()
{
    delete m_imageData;
#if wxUSE_MARKUP
    delete m_markupText;
#endif }

void wxAnyButton::SetLabel(const wxString& label)
{
    wxMSWButton::UpdateMultilineStyle(GetHwnd(), label);

    wxAnyButtonBase::SetLabel(label);

#if wxUSE_MARKUP
        if ( m_markupText )
    {
        delete m_markupText;
        m_markupText = NULL;

                                                            }
#endif }


void wxAnyButton::AdjustForBitmapSize(wxSize &size) const
{
    wxCHECK_RET( m_imageData, wxT("shouldn't be called if no image") );

        const wxSize sizeBmp = m_imageData->GetBitmap(State_Normal).GetSize()
                                + 2*m_imageData->GetBitmapMargins();
    const wxDirection dirBmp = m_imageData->GetBitmapPosition();
    if ( dirBmp == wxLEFT || dirBmp == wxRIGHT )
    {
        size.x += sizeBmp.x;
        if ( sizeBmp.y > size.y )
            size.y = sizeBmp.y;
    }
    else     {
        size.y += sizeBmp.y;
        if ( sizeBmp.x > size.x )
            size.x = sizeBmp.x;
    }

                if ( !HasFlag(wxBORDER_NONE) )
    {
        int marginH = 0,
            marginV = 0;
#if wxUSE_UXTHEME
        if ( wxUxThemeEngine::GetIfActive() )
        {
            wxUxThemeHandle theme(const_cast<wxAnyButton *>(this), L"BUTTON");

            MARGINS margins;
            wxUxThemeEngine::Get()->GetThemeMargins(theme, NULL,
                                                    BP_PUSHBUTTON,
                                                    PBS_NORMAL,
                                                    TMT_CONTENTMARGINS,
                                                    NULL,
                                                    &margins);

                                                size.IncTo(wxSize(8, 8));

            marginH = margins.cxLeftWidth + margins.cxRightWidth
                        + 2*XP_BUTTON_EXTRA_MARGIN;
            marginV = margins.cyTopHeight + margins.cyBottomHeight
                        + 2*XP_BUTTON_EXTRA_MARGIN;
        }
        else
#endif         {
            marginH =
            marginV = OD_BUTTON_MARGIN;
        }

        size.IncBy(marginH, marginV);
    }
}

wxSize wxAnyButton::DoGetBestSize() const
{
    wxAnyButton * const self = const_cast<wxAnyButton *>(this);

    wxSize size;

        if ( ShowsLabel() )
    {
        int flags = 0;
        if ( HasFlag(wxBU_EXACTFIT) )
            flags |= wxMSWButton::Size_ExactFit;
        if ( DoGetAuthNeeded() )
            flags |= wxMSWButton::Size_AuthNeeded;

#if wxUSE_MARKUP
        if ( m_markupText )
        {
            wxClientDC dc(self);
            size = wxMSWButton::GetFittingSize(self,
                                               m_markupText->Measure(dc),
                                               flags);
        }
        else #endif         {
            size = wxMSWButton::ComputeBestFittingSize(self, flags);
        }
    }

    if ( m_imageData )
        AdjustForBitmapSize(size);

    return wxMSWButton::IncreaseToStdSizeAndCache(self, size);
}


WXLRESULT wxAnyButton::MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam)
{
    if ( nMsg == WM_LBUTTONDBLCLK )
    {
                        (void)wxControl::MSWWindowProc(WM_LBUTTONDOWN, wParam, lParam);

            }
#if wxUSE_UXTHEME
    else if ( nMsg == WM_THEMECHANGED )
    {
                        InvalidateBestSize();
    }
#endif                 else if ( (nMsg == WM_MOUSEMOVE && !m_mouseInWindow) ||
                nMsg == WM_MOUSELEAVE )
    {
        if (
                IsEnabled() &&
                (
#if wxUSE_UXTHEME
                wxUxThemeEngine::GetIfActive() ||
#endif                  (m_imageData && m_imageData->GetBitmap(State_Current).IsOk())
                )
           )
        {
            Refresh();
        }
    }

        return wxControl::MSWWindowProc(nMsg, wParam, lParam);
}


wxBitmap wxAnyButton::DoGetBitmap(State which) const
{
    return m_imageData ? m_imageData->GetBitmap(which) : wxBitmap();
}

void wxAnyButton::DoSetBitmap(const wxBitmap& bitmap, State which)
{
    if ( !bitmap.IsOk() )
    {
        if ( m_imageData  )
        {
                                    if ( which == State_Normal )
            {
                delete m_imageData;
                m_imageData = NULL;
            }
            else
            {
                                wxBitmap bmpNormal = m_imageData->GetBitmap(State_Normal);
                m_imageData->SetBitmap(which == State_Disabled
                                            ? bmpNormal.ConvertToDisabled()
                                            : bmpNormal,
                                       which);
            }
        }

        return;
    }

#if wxUSE_UXTHEME
    wxXPButtonImageData *oldData = NULL;
#endif 
        if ( m_imageData &&
          bitmap.GetSize() != m_imageData->GetBitmap(State_Normal).GetSize() )
    {
        wxASSERT_MSG( (which == State_Normal) || bitmap.IsNull(),
                      "Must set normal bitmap with the new size first" );

#if wxUSE_UXTHEME
        if ( ShowsLabel() && wxUxThemeEngine::GetIfActive() )
        {
                                                oldData = static_cast<wxXPButtonImageData *>(m_imageData);
            m_imageData = NULL;
        }
#endif             }

        if ( !m_imageData )
    {
#if wxUSE_UXTHEME
                                        if ( ShowsLabel() && wxUxThemeEngine::GetIfActive() )
        {
            m_imageData = new wxXPButtonImageData(this, bitmap);

            if ( oldData )
            {
                                m_imageData->SetBitmapPosition(oldData->GetBitmapPosition());

                const wxSize oldMargins = oldData->GetBitmapMargins();
                m_imageData->SetBitmapMargins(oldMargins.x, oldMargins.y);

                                
                delete oldData;
            }
        }
        else
#endif         {
            m_imageData = new wxODButtonImageData(this, bitmap);
            MakeOwnerDrawn();
        }
    }
    else
    {
        m_imageData->SetBitmap(bitmap, which);
    }

                if ( which == State_Normal )
        InvalidateBestSize();

    Refresh();
}

wxSize wxAnyButton::DoGetBitmapMargins() const
{
    return m_imageData ? m_imageData->GetBitmapMargins() : wxSize(0, 0);
}

void wxAnyButton::DoSetBitmapMargins(wxCoord x, wxCoord y)
{
    wxCHECK_RET( m_imageData, "SetBitmap() must be called first" );

    m_imageData->SetBitmapMargins(x, y);
    InvalidateBestSize();
}

void wxAnyButton::DoSetBitmapPosition(wxDirection dir)
{
    if ( m_imageData )
        m_imageData->SetBitmapPosition(dir);
    InvalidateBestSize();
}


#if wxUSE_MARKUP

bool wxAnyButton::DoSetLabelMarkup(const wxString& markup)
{
    if ( !wxAnyButtonBase::DoSetLabelMarkup(markup) )
        return false;

    if ( !m_markupText )
    {
        m_markupText = new wxMarkupText(markup);
        MakeOwnerDrawn();
    }
    else
    {
                m_markupText->SetMarkup(markup);
    }

    Refresh();

    return true;
}

#endif 

namespace
{

wxAnyButton::State GetButtonState(wxAnyButton *btn, UINT state)
{
    if ( state & ODS_DISABLED )
        return wxAnyButton::State_Disabled;

                const wxAnyButton::State btnState = btn->GetNormalState();

    if ( btnState == wxAnyButton::State_Pressed || state & ODS_SELECTED )
        return wxAnyButton::State_Pressed;

    if ( btn->HasCapture() || btn->IsMouseInWindow() )
        return wxAnyButton::State_Current;

    if ( state & ODS_FOCUS )
        return wxAnyButton::State_Focused;

    return btnState;
}

void DrawButtonText(HDC hdc,
                    RECT *pRect,
                    wxAnyButton *btn,
                    int flags)
{
    const wxString text = btn->GetLabel();

            if ( !wxUxThemeEngine::GetIfActive() && !btn->IsEnabled() )
    {
                                        
                RECT rc;
        ::SetRectEmpty(&rc);
        ::DrawText(hdc, text.t_str(), text.length(), &rc, DT_CALCRECT);

        const LONG h = rc.bottom - rc.top;

                        int y0;
        if ( btn->HasFlag(wxBU_BOTTOM) )
        {
            y0 = pRect->bottom - h;
        }
        else if ( !btn->HasFlag(wxBU_TOP) )
        {
                        y0 = pRect->top + (pRect->bottom - pRect->top)/2 - h/2;
        }
        else         {
            y0 = pRect->top;
        }

        UINT dsFlags = DSS_DISABLED;
        if( flags & DT_HIDEPREFIX )
            dsFlags |= (DSS_HIDEPREFIX | DST_PREFIXTEXT);
        else
            dsFlags |= DST_TEXT;

        const wxArrayString lines = wxSplit(text, '\n', '\0');
        const int hLine = h / lines.size();
        for ( size_t lineNum = 0; lineNum < lines.size(); lineNum++ )
        {
                        ::SetRectEmpty(&rc);
            ::DrawText(hdc, lines[lineNum].t_str(), lines[lineNum].length(),
                       &rc, DT_CALCRECT);
            const LONG w = rc.right - rc.left;

                                                if ( (!btn->HasFlag(wxBU_LEFT) && !btn->HasFlag(wxBU_RIGHT)) ||
                    lines.size() > 1 )
            {
                                rc.left = pRect->left + (pRect->right - pRect->left)/2 - w/2;
                rc.right = rc.left + w;
            }
            else if ( btn->HasFlag(wxBU_RIGHT) )
            {
                rc.right = pRect->right;
                rc.left = rc.right - w;
            }
            else             {
                rc.left = pRect->left;
                rc.right = rc.left  + w;
            }

            ::OffsetRect(&rc, 0, y0 + lineNum * hLine);

            ::DrawState(hdc, NULL, NULL, wxMSW_CONV_LPARAM(lines[lineNum]),
                        lines[lineNum].length(),
                        rc.left, rc.top, rc.right, rc.bottom, dsFlags);
        }
    }
    else     {
        if ( text.find(wxT('\n')) != wxString::npos )
        {
            
                        flags |= DT_CENTER;

                        RECT rc;
            ::CopyRect(&rc, pRect);
            ::DrawText(hdc, text.t_str(), text.length(), &rc,
                       DT_CENTER | DT_CALCRECT);

                        const LONG w = rc.right - rc.left;
            const LONG h = rc.bottom - rc.top;
            rc.left = pRect->left + (pRect->right - pRect->left)/2 - w/2;
            rc.right = rc.left+w;
            rc.top = pRect->top + (pRect->bottom - pRect->top)/2 - h/2;
            rc.bottom = rc.top+h;

            ::DrawText(hdc, text.t_str(), text.length(), &rc, flags);
        }
        else         {
                        if ( btn->HasFlag(wxBU_RIGHT) )
            {
                flags |= DT_RIGHT;
            }
            else if ( !btn->HasFlag(wxBU_LEFT) )
            {
                flags |= DT_CENTER;
            }
            
            if ( btn->HasFlag(wxBU_BOTTOM) )
            {
                flags |= DT_BOTTOM;
            }
            else if ( !btn->HasFlag(wxBU_TOP) )
            {
                flags |= DT_VCENTER;
            }
            
                                    ::DrawText(hdc, text.t_str(), text.length(), pRect,
                       flags | DT_SINGLELINE );
        }
    }
}

void DrawRect(HDC hdc, const RECT& r)
{
    wxDrawLine(hdc, r.left, r.top, r.right, r.top);
    wxDrawLine(hdc, r.right, r.top, r.right, r.bottom);
    wxDrawLine(hdc, r.right, r.bottom, r.left, r.bottom);
    wxDrawLine(hdc, r.left, r.bottom, r.left, r.top);
}


void DrawButtonFrame(HDC hdc, RECT& rectBtn,
                     bool selected, bool pushed)
{
    RECT r;
    CopyRect(&r, &rectBtn);

    AutoHPEN hpenBlack(GetSysColor(COLOR_3DDKSHADOW)),
             hpenGrey(GetSysColor(COLOR_3DSHADOW)),
             hpenLightGr(GetSysColor(COLOR_3DLIGHT)),
             hpenWhite(GetSysColor(COLOR_3DHILIGHT));

    SelectInHDC selectPen(hdc, hpenBlack);

    r.right--;
    r.bottom--;

    if ( pushed )
    {
        DrawRect(hdc, r);

        (void)SelectObject(hdc, hpenGrey);
        ::InflateRect(&r, -1, -1);

        DrawRect(hdc, r);
    }
    else     {
        if ( selected )
        {
            DrawRect(hdc, r);

            ::InflateRect(&r, -1, -1);
        }

        wxDrawLine(hdc, r.left, r.bottom, r.right, r.bottom);
        wxDrawLine(hdc, r.right, r.bottom, r.right, r.top - 1);

        (void)SelectObject(hdc, hpenWhite);
        wxDrawLine(hdc, r.left, r.bottom - 1, r.left, r.top);
        wxDrawLine(hdc, r.left, r.top, r.right, r.top);

        (void)SelectObject(hdc, hpenLightGr);
        wxDrawLine(hdc, r.left + 1, r.bottom - 2, r.left + 1, r.top + 1);
        wxDrawLine(hdc, r.left + 1, r.top + 1, r.right - 1, r.top + 1);

        (void)SelectObject(hdc, hpenGrey);
        wxDrawLine(hdc, r.left + 1, r.bottom - 1, r.right - 1, r.bottom - 1);
        wxDrawLine(hdc, r.right - 1, r.bottom - 1, r.right - 1, r.top);
    }

    InflateRect(&rectBtn, -OD_BUTTON_MARGIN, -OD_BUTTON_MARGIN);
}

#if wxUSE_UXTHEME
void DrawXPBackground(wxAnyButton *button, HDC hdc, RECT& rectBtn, UINT state)
{
    wxUxThemeHandle theme(button, L"BUTTON");

            static const int uxStates[] =
    {
        PBS_NORMAL, PBS_HOT, PBS_PRESSED, PBS_DISABLED, PBS_DEFAULTED
    };

    int iState = uxStates[GetButtonState(button, state)];

    wxUxThemeEngine * const engine = wxUxThemeEngine::Get();

        if ( engine->IsThemeBackgroundPartiallyTransparent
                 (
                    theme,
                    BP_PUSHBUTTON,
                    iState
                 ) )
    {
                                                                                wxWindowBeingErased = button;

        engine->DrawThemeParentBackground(GetHwndOf(button), hdc, &rectBtn);

        wxWindowBeingErased = NULL;
    }

        engine->DrawThemeBackground(theme, hdc, BP_PUSHBUTTON, iState,
                                &rectBtn, NULL);

        MARGINS margins;
    engine->GetThemeMargins(theme, hdc, BP_PUSHBUTTON, iState,
                            TMT_CONTENTMARGINS, &rectBtn, &margins);
    ::InflateRect(&rectBtn, -margins.cxLeftWidth, -margins.cyTopHeight);
    ::InflateRect(&rectBtn, -XP_BUTTON_EXTRA_MARGIN, -XP_BUTTON_EXTRA_MARGIN);

    if ( button->UseBgCol() )
    {
        COLORREF colBg = wxColourToRGB(button->GetBackgroundColour());
        AutoHBRUSH hbrushBackground(colBg);

                RECT rectClient;
        ::CopyRect(&rectClient, &rectBtn);
        ::InflateRect(&rectClient, -1, -1);
        FillRect(hdc, &rectClient, hbrushBackground);
    }
}
#endif 
} 

void wxAnyButton::MakeOwnerDrawn()
{
    if ( !IsOwnerDrawn() )
    {
                        long style = GetWindowLong(GetHwnd(), GWL_STYLE);
        style &= ~(BS_3STATE | BS_AUTO3STATE | BS_AUTOCHECKBOX | BS_AUTORADIOBUTTON | BS_CHECKBOX | BS_DEFPUSHBUTTON | BS_GROUPBOX | BS_PUSHBUTTON | BS_RADIOBUTTON | BS_PUSHLIKE);
        style |= BS_OWNERDRAW;
        SetWindowLong(GetHwnd(), GWL_STYLE, style);
    }
}

bool wxAnyButton::IsOwnerDrawn() const
{
    long style = GetWindowLong(GetHwnd(), GWL_STYLE);
    return ( (style & BS_OWNERDRAW) == BS_OWNERDRAW );
}

bool wxAnyButton::SetBackgroundColour(const wxColour &colour)
{
    if ( !wxControl::SetBackgroundColour(colour) )
    {
                return false;
    }

    MakeOwnerDrawn();

    Refresh();

    return true;
}

bool wxAnyButton::SetForegroundColour(const wxColour &colour)
{
    if ( !wxControl::SetForegroundColour(colour) )
    {
                return false;
    }

    MakeOwnerDrawn();

    Refresh();

    return true;
}

bool wxAnyButton::MSWIsPushed() const
{
    return (SendMessage(GetHwnd(), BM_GETSTATE, 0, 0) & BST_PUSHED) != 0;
}

bool wxAnyButton::MSWOnDraw(WXDRAWITEMSTRUCT *wxdis)
{
    LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)wxdis;
    HDC hdc = lpDIS->hDC;

    UINT state = lpDIS->itemState;
    switch ( GetButtonState(this, state) )
    {
        case State_Disabled:
            state |= ODS_DISABLED;
            break;
        case State_Pressed:
            state |= ODS_SELECTED;
            break;
        case State_Focused:
            state |= ODS_FOCUS;
            break;
        default:
            break;
    }

    bool pushed = MSWIsPushed();

    RECT rectBtn;
    CopyRect(&rectBtn, &lpDIS->rcItem);

        if ( !HasFlag(wxBORDER_NONE) )
    {
#if wxUSE_UXTHEME
        if ( wxUxThemeEngine::GetIfActive() )
        {
            DrawXPBackground(this, hdc, rectBtn, state);
        }
        else
#endif         {
            COLORREF colBg = wxColourToRGB(GetBackgroundColour());

                        AutoHBRUSH hbrushBackground(colBg);
            FillRect(hdc, &rectBtn, hbrushBackground);

                        bool selected = (state & ODS_SELECTED) != 0;
            if ( !selected )
            {
                wxTopLevelWindow *
                    tlw = wxDynamicCast(wxGetTopLevelParent(this), wxTopLevelWindow);
                if ( tlw )
                {
                    selected = tlw->GetDefaultItem() == this;
                }
            }

            DrawButtonFrame(hdc, rectBtn, selected, pushed);
        }

                if ( (state & ODS_FOCUS) && !(state & ODS_NOFOCUSRECT) )
        {
            DrawFocusRect(hdc, &rectBtn);

#if wxUSE_UXTHEME
            if ( !wxUxThemeEngine::GetIfActive() )
#endif             {
                if ( pushed )
                {
                                        OffsetRect(&rectBtn, 1, 1);
                }
            }
        }
    }
    else
    {
                COLORREF colBg = wxColourToRGB(GetBackgroundColour());
        AutoHBRUSH hbrushBackground(colBg);
        FillRect(hdc, &rectBtn, hbrushBackground);
    }

        if ( m_imageData )
    {
        wxBitmap bmp = m_imageData->GetBitmap(GetButtonState(this, state));
        if ( !bmp.IsOk() )
            bmp = m_imageData->GetBitmap(State_Normal);

        const wxSize sizeBmp = bmp.GetSize();
        const wxSize margin = m_imageData->GetBitmapMargins();
        const wxSize sizeBmpWithMargins(sizeBmp + 2*margin);
        wxRect rectButton(wxRectFromRECT(rectBtn));

                        wxRect rectBitmap = wxRect(sizeBmp).CentreIn(rectButton);

                if ( ShowsLabel() )
        {
            switch ( m_imageData->GetBitmapPosition() )
            {
                default:
                    wxFAIL_MSG( "invalid direction" );
                    
                case wxLEFT:
                    rectBitmap.x = rectButton.x + margin.x;
                    rectButton.x += sizeBmpWithMargins.x;
                    rectButton.width -= sizeBmpWithMargins.x;
                    break;

                case wxRIGHT:
                    rectBitmap.x = rectButton.GetRight() - sizeBmp.x - margin.x;
                    rectButton.width -= sizeBmpWithMargins.x;
                    break;

                case wxTOP:
                    rectBitmap.y = rectButton.y + margin.y;
                    rectButton.y += sizeBmpWithMargins.y;
                    rectButton.height -= sizeBmpWithMargins.y;
                    break;

                case wxBOTTOM:
                    rectBitmap.y = rectButton.GetBottom() - sizeBmp.y - margin.y;
                    rectButton.height -= sizeBmpWithMargins.y;
                    break;
            }
        }

        wxDCTemp dst((WXHDC)hdc);
        dst.DrawBitmap(bmp, rectBitmap.GetPosition(), true);

        wxCopyRectToRECT(rectButton, rectBtn);
    }


        if ( ShowsLabel() )
    {
        COLORREF colFg = state & ODS_DISABLED
                            ? ::GetSysColor(COLOR_GRAYTEXT)
                            : wxColourToRGB(GetForegroundColour());

        wxTextColoursChanger changeFg(hdc, colFg, CLR_INVALID);
        wxBkModeChanger changeBkMode(hdc, wxBRUSHSTYLE_TRANSPARENT);

#if wxUSE_MARKUP
        if ( m_markupText )
        {
            wxDCTemp dc((WXHDC)hdc);
            dc.SetTextForeground(wxColour(colFg));
            dc.SetFont(GetFont());

            m_markupText->Render(dc, wxRectFromRECT(rectBtn),
                                 state & ODS_NOACCEL
                                    ? wxMarkupText::Render_Default
                                    : wxMarkupText::Render_ShowAccels);
        }
        else #endif         {
                                                DrawButtonText(hdc, &rectBtn, this,
                           state & ODS_NOACCEL ? DT_HIDEPREFIX : 0);
        }
    }

    return true;
}

#endif 