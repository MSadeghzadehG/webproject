
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_SEARCHCTRL

#include "wx/srchctrl.h"

#ifndef WX_PRECOMP
    #include "wx/button.h"
    #include "wx/dcclient.h"
    #include "wx/menu.h"
    #include "wx/dcmemory.h"
#endif 
#if !wxUSE_NATIVE_SEARCH_CONTROL

#include "wx/image.h"

#define WXMAX(a,b) ((a)>(b)?(a):(b))


static const wxCoord MARGIN = 2;

#define LIGHT_STEP 160


class wxSearchTextCtrl : public wxTextCtrl
{
public:
    wxSearchTextCtrl(wxSearchCtrl *search, const wxString& value, int style)
        : wxTextCtrl(search, wxID_ANY, value, wxDefaultPosition, wxDefaultSize,
                     (style & ~wxBORDER_MASK) | wxNO_BORDER)
    {
        m_search = search;

        SetHint(_("Search"));

                        InvalidateBestSize();
    }

    virtual wxWindow* GetMainWindowOfCompositeControl() wxOVERRIDE
    {
        return m_search;
    }

            void DoSetValue(const wxString& value, int flags) wxOVERRIDE
    {
        wxTextCtrl::DoSetValue(value, flags);
    }

    bool DoLoadFile(const wxString& file, int fileType) wxOVERRIDE
    {
        return wxTextCtrl::DoLoadFile(file, fileType);
    }

    bool DoSaveFile(const wxString& file, int fileType) wxOVERRIDE
    {
        return wxTextCtrl::DoSaveFile(file, fileType);
    }

protected:
    void OnText(wxCommandEvent& eventText)
    {
        wxCommandEvent event(eventText);
        event.SetEventObject(m_search);
        event.SetId(m_search->GetId());

        m_search->GetEventHandler()->ProcessEvent(event);
    }

    void OnTextUrl(wxTextUrlEvent& eventText)
    {
                        wxTextUrlEvent event(
            m_search->GetId(),
            eventText.GetMouseEvent(),
            eventText.GetURLStart(),
            eventText.GetURLEnd()
            );
        event.SetEventObject(m_search);

        m_search->GetEventHandler()->ProcessEvent(event);
    }

#ifdef __WXMSW__
                                                    virtual wxSize DoGetBestSize() const
    {
        const long flags = GetWindowStyleFlag();
        wxSearchTextCtrl* const self = const_cast<wxSearchTextCtrl*>(this);

        self->SetWindowStyleFlag((flags & ~wxBORDER_MASK) | wxBORDER_DEFAULT);
        wxSize size = wxTextCtrl::DoGetBestSize();

                                                size.y -= 4;

        self->SetWindowStyleFlag(flags);

        return size;
    }
#endif 
private:
    wxSearchCtrl* m_search;

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(wxSearchTextCtrl, wxTextCtrl)
    EVT_TEXT(wxID_ANY, wxSearchTextCtrl::OnText)
    EVT_TEXT_ENTER(wxID_ANY, wxSearchTextCtrl::OnText)
    EVT_TEXT_URL(wxID_ANY, wxSearchTextCtrl::OnTextUrl)
    EVT_TEXT_MAXLEN(wxID_ANY, wxSearchTextCtrl::OnText)
wxEND_EVENT_TABLE()


class wxSearchButton : public wxControl
{
public:
    wxSearchButton(wxSearchCtrl *search, int eventType, const wxBitmap& bmp)
        : wxControl(search, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER),
          m_search(search),
          m_eventType(eventType),
          m_bmp(bmp)
    { }

    void SetBitmapLabel(const wxBitmap& label)
    {
        m_bmp = label;
        InvalidateBestSize();
    }

                            virtual bool AcceptsFocusFromKeyboard() const wxOVERRIDE { return false; }

    virtual wxWindow* GetMainWindowOfCompositeControl() wxOVERRIDE
    {
        return m_search;
    }

protected:
    wxSize DoGetBestSize() const wxOVERRIDE
    {
        return wxSize(m_bmp.GetWidth(), m_bmp.GetHeight());
    }

    void OnLeftUp(wxMouseEvent&)
    {
        wxCommandEvent event(m_eventType, m_search->GetId());
        event.SetEventObject(m_search);

        if ( m_eventType == wxEVT_SEARCHCTRL_SEARCH_BTN )
        {
                                                event.SetString(m_search->GetValue());
        }

        GetEventHandler()->ProcessEvent(event);

        m_search->SetFocus();

#if wxUSE_MENUS
        if ( m_eventType == wxEVT_SEARCHCTRL_SEARCH_BTN )
        {
                        m_search->PopupSearchMenu();
        }
#endif     }

    void OnPaint(wxPaintEvent&)
    {
        wxPaintDC dc(this);
        dc.DrawBitmap(m_bmp, 0,0, true);
    }


private:
    wxSearchCtrl *m_search;
    wxEventType   m_eventType;
    wxBitmap      m_bmp;

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(wxSearchButton, wxControl)
    EVT_LEFT_UP(wxSearchButton::OnLeftUp)
    EVT_PAINT(wxSearchButton::OnPaint)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(wxSearchCtrl, wxSearchCtrlBase)
    EVT_SEARCHCTRL_CANCEL_BTN(wxID_ANY, wxSearchCtrl::OnCancelButton)
    EVT_SET_FOCUS(wxSearchCtrl::OnSetFocus)
    EVT_SIZE(wxSearchCtrl::OnSize)
wxEND_EVENT_TABLE()

wxIMPLEMENT_DYNAMIC_CLASS(wxSearchCtrl, wxSearchCtrlBase);




wxSearchCtrl::wxSearchCtrl()
{
    Init();
}

wxSearchCtrl::wxSearchCtrl(wxWindow *parent, wxWindowID id,
           const wxString& value,
           const wxPoint& pos,
           const wxSize& size,
           long style,
           const wxValidator& validator,
           const wxString& name)
{
    Init();

    Create(parent, id, value, pos, size, style, validator, name);
}

void wxSearchCtrl::Init()
{
    m_text = NULL;
    m_searchButton = NULL;
    m_cancelButton = NULL;
#if wxUSE_MENUS
    m_menu = NULL;
#endif 
    m_searchButtonVisible = true;
    m_cancelButtonVisible = false;

    m_searchBitmapUser = false;
    m_cancelBitmapUser = false;
#if wxUSE_MENUS
    m_searchMenuBitmapUser = false;
#endif }

bool wxSearchCtrl::Create(wxWindow *parent, wxWindowID id,
            const wxString& value,
            const wxPoint& pos,
            const wxSize& size,
            long style,
            const wxValidator& validator,
            const wxString& name)
{
        style &= ~wxBORDER_MASK;
#ifdef __WXGTK__
    style |= wxBORDER_SUNKEN;
#elif defined(__WXMSW__)
                if (GetDefaultBorder() == wxBORDER_SUNKEN)
        style |= wxBORDER_SIMPLE;
#else
    style |= wxBORDER_SIMPLE;
#endif
    if ( !wxSearchCtrlBaseBaseClass::Create(parent, id, pos, size,
                                            style, validator, name) )
    {
        return false;
    }

    m_text = new wxSearchTextCtrl(this, value, style);

    m_searchButton = new wxSearchButton(this,
                                        wxEVT_SEARCHCTRL_SEARCH_BTN,
                                        m_searchBitmap);
    m_cancelButton = new wxSearchButton(this,
                                        wxEVT_SEARCHCTRL_CANCEL_BTN,
                                        m_cancelBitmap);

    SetBackgroundColour( m_text->GetBackgroundColour() );

    RecalcBitmaps();

    SetInitialSize(size);
    Move(pos);
    return true;
}

wxSearchCtrl::~wxSearchCtrl()
{
    delete m_text;
    delete m_searchButton;
    delete m_cancelButton;
#if wxUSE_MENUS
    delete m_menu;
#endif }


#if wxUSE_MENUS

void wxSearchCtrl::SetMenu( wxMenu* menu )
{
    if ( menu == m_menu )
    {
                return;
    }
    bool hadMenu = (m_menu != NULL);
    delete m_menu;
    m_menu = menu;

    if ( m_menu && !hadMenu )
    {
        m_searchButton->SetBitmapLabel(m_searchMenuBitmap);
        m_searchButton->Refresh();
    }
    else if ( !m_menu && hadMenu )
    {
        m_searchButton->SetBitmapLabel(m_searchBitmap);
        if ( m_searchButtonVisible )
        {
            m_searchButton->Refresh();
        }
    }
    LayoutControls();
}

wxMenu* wxSearchCtrl::GetMenu()
{
    return m_menu;
}

#endif 
void wxSearchCtrl::ShowSearchButton( bool show )
{
    if ( m_searchButtonVisible == show )
    {
                return;
    }
    m_searchButtonVisible = show;
    if ( m_searchButtonVisible )
    {
        RecalcBitmaps();
    }

    LayoutControls();
}

bool wxSearchCtrl::IsSearchButtonVisible() const
{
    return m_searchButtonVisible;
}


void wxSearchCtrl::ShowCancelButton( bool show )
{
    if ( m_cancelButtonVisible == show )
    {
                return;
    }
    m_cancelButtonVisible = show;

    LayoutControls();
}

bool wxSearchCtrl::IsCancelButtonVisible() const
{
    return m_cancelButtonVisible;
}

void wxSearchCtrl::SetDescriptiveText(const wxString& text)
{
    m_text->SetHint(text);
}

wxString wxSearchCtrl::GetDescriptiveText() const
{
    return m_text->GetHint();
}


wxSize wxSearchCtrl::DoGetBestClientSize() const
{
    wxSize sizeText = m_text->GetBestSize();
    wxSize sizeSearch(0,0);
    wxSize sizeCancel(0,0);
    int searchMargin = 0;
    int cancelMargin = 0;
    if ( m_searchButtonVisible || HasMenu() )
    {
        sizeSearch = m_searchButton->GetBestSize();
        searchMargin = MARGIN;
    }
    if ( m_cancelButtonVisible )
    {
        sizeCancel = m_cancelButton->GetBestSize();
        cancelMargin = MARGIN;
    }

    int horizontalBorder = 1 + ( sizeText.y - sizeText.y * 14 / 21 ) / 2;

        int height = sizeText.y;
    return wxSize(sizeSearch.x + searchMargin + sizeText.x + cancelMargin + sizeCancel.x + 2*horizontalBorder,
                  height);
}

void wxSearchCtrl::DoMoveWindow(int x, int y, int width, int height)
{
    wxSearchCtrlBase::DoMoveWindow(x, y, width, height);

    LayoutControls();
}

void wxSearchCtrl::LayoutControls()
{
    if ( !m_text )
        return;

    const wxSize sizeTotal = GetClientSize();
    int width = sizeTotal.x,
        height = sizeTotal.y;

    wxSize sizeText = m_text->GetBestSize();
        int horizontalBorder = 1 + ( sizeText.y - sizeText.y * 14 / 21 ) / 2;
    int x = horizontalBorder;
    width -= horizontalBorder*2;

    wxSize sizeSearch(0,0);
    wxSize sizeCancel(0,0);
    int searchMargin = 0;
    int cancelMargin = 0;
    if ( m_searchButtonVisible || HasMenu() )
    {
        sizeSearch = m_searchButton->GetBestSize();
        searchMargin = MARGIN;
    }
    if ( m_cancelButtonVisible )
    {
        sizeCancel = m_cancelButton->GetBestSize();
        cancelMargin = MARGIN;
    }
    m_searchButton->Show( m_searchButtonVisible || HasMenu() );
    m_cancelButton->Show( m_cancelButtonVisible );

    if ( sizeSearch.x + sizeCancel.x > width )
    {
        sizeSearch.x = width/2;
        sizeCancel.x = width/2;
        searchMargin = 0;
        cancelMargin = 0;
    }
    wxCoord textWidth = width - sizeSearch.x - sizeCancel.x - searchMargin - cancelMargin - 1;
    if (textWidth < 0) textWidth = 0;

    
    m_searchButton->SetSize(x, (height - sizeSearch.y) / 2,
                            sizeSearch.x, height);
    x += sizeSearch.x;
    x += searchMargin;

#ifdef __WXMSW__
                    int textY = 1;
#else
    int textY = 0;
#endif

    m_text->SetSize(x, textY, textWidth, height-textY);
    x += textWidth;
    x += cancelMargin;

    m_cancelButton->SetSize(x, (height - sizeCancel.y) / 2,
                            sizeCancel.x, height);
}

wxWindowList wxSearchCtrl::GetCompositeWindowParts() const
{
    wxWindowList parts;
    parts.push_back(m_text);
    parts.push_back(m_searchButton);
    parts.push_back(m_cancelButton);
    return parts;
}


wxString wxSearchCtrl::DoGetValue() const
{
    return m_text->GetValue();
}
wxString wxSearchCtrl::GetRange(long from, long to) const
{
    return m_text->GetRange(from, to);
}

int wxSearchCtrl::GetLineLength(long lineNo) const
{
    return m_text->GetLineLength(lineNo);
}
wxString wxSearchCtrl::GetLineText(long lineNo) const
{
    return m_text->GetLineText(lineNo);
}
int wxSearchCtrl::GetNumberOfLines() const
{
    return m_text->GetNumberOfLines();
}

bool wxSearchCtrl::IsModified() const
{
    return m_text->IsModified();
}
bool wxSearchCtrl::IsEditable() const
{
    return m_text->IsEditable();
}

bool wxSearchCtrl::IsSingleLine() const
{
    return m_text->IsSingleLine();
}
bool wxSearchCtrl::IsMultiLine() const
{
    return m_text->IsMultiLine();
}

void wxSearchCtrl::GetSelection(long* from, long* to) const
{
    m_text->GetSelection(from, to);
}

wxString wxSearchCtrl::GetStringSelection() const
{
    return m_text->GetStringSelection();
}


void wxSearchCtrl::Clear()
{
    m_text->Clear();
}
void wxSearchCtrl::Replace(long from, long to, const wxString& value)
{
    m_text->Replace(from, to, value);
}
void wxSearchCtrl::Remove(long from, long to)
{
    m_text->Remove(from, to);
}

bool wxSearchCtrl::LoadFile(const wxString& file)
{
    return m_text->LoadFile(file);
}
bool wxSearchCtrl::SaveFile(const wxString& file)
{
    return m_text->SaveFile(file);
}

void wxSearchCtrl::MarkDirty()
{
    m_text->MarkDirty();
}
void wxSearchCtrl::DiscardEdits()
{
    m_text->DiscardEdits();
}

void wxSearchCtrl::SetMaxLength(unsigned long len)
{
    m_text->SetMaxLength(len);
}

void wxSearchCtrl::WriteText(const wxString& text)
{
    m_text->WriteText(text);
}
void wxSearchCtrl::AppendText(const wxString& text)
{
    m_text->AppendText(text);
}

bool wxSearchCtrl::EmulateKeyPress(const wxKeyEvent& event)
{
    return m_text->EmulateKeyPress(event);
}

bool wxSearchCtrl::SetStyle(long start, long end, const wxTextAttr& style)
{
    return m_text->SetStyle(start, end, style);
}
bool wxSearchCtrl::GetStyle(long position, wxTextAttr& style)
{
    return m_text->GetStyle(position, style);
}
bool wxSearchCtrl::SetDefaultStyle(const wxTextAttr& style)
{
    return m_text->SetDefaultStyle(style);
}
const wxTextAttr& wxSearchCtrl::GetDefaultStyle() const
{
    return m_text->GetDefaultStyle();
}

long wxSearchCtrl::XYToPosition(long x, long y) const
{
    return m_text->XYToPosition(x, y);
}
bool wxSearchCtrl::PositionToXY(long pos, long *x, long *y) const
{
    return m_text->PositionToXY(pos, x, y);
}

void wxSearchCtrl::ShowPosition(long pos)
{
    m_text->ShowPosition(pos);
}

wxTextCtrlHitTestResult wxSearchCtrl::HitTest(const wxPoint& pt, long *pos) const
{
    return m_text->HitTest(pt, pos);
}
wxTextCtrlHitTestResult wxSearchCtrl::HitTest(const wxPoint& pt,
                                        wxTextCoord *col,
                                        wxTextCoord *row) const
{
    return m_text->HitTest(pt, col, row);
}

void wxSearchCtrl::Copy()
{
    m_text->Copy();
}
void wxSearchCtrl::Cut()
{
    m_text->Cut();
}
void wxSearchCtrl::Paste()
{
    m_text->Paste();
}

bool wxSearchCtrl::CanCopy() const
{
    return m_text->CanCopy();
}
bool wxSearchCtrl::CanCut() const
{
    return m_text->CanCut();
}
bool wxSearchCtrl::CanPaste() const
{
    return m_text->CanPaste();
}

void wxSearchCtrl::Undo()
{
    m_text->Undo();
}
void wxSearchCtrl::Redo()
{
    m_text->Redo();
}

bool wxSearchCtrl::CanUndo() const
{
    return m_text->CanUndo();
}
bool wxSearchCtrl::CanRedo() const
{
    return m_text->CanRedo();
}

void wxSearchCtrl::SetInsertionPoint(long pos)
{
    m_text->SetInsertionPoint(pos);
}
void wxSearchCtrl::SetInsertionPointEnd()
{
    m_text->SetInsertionPointEnd();
}
long wxSearchCtrl::GetInsertionPoint() const
{
    return m_text->GetInsertionPoint();
}
long wxSearchCtrl::GetLastPosition() const
{
    return m_text->GetLastPosition();
}

void wxSearchCtrl::SetSelection(long from, long to)
{
    m_text->SetSelection(from, to);
}
void wxSearchCtrl::SelectAll()
{
    m_text->SelectAll();
}

void wxSearchCtrl::SetEditable(bool editable)
{
    m_text->SetEditable(editable);
}

bool wxSearchCtrl::SetFont(const wxFont& font)
{
    if ( !wxSearchCtrlBase::SetFont(font) )
        return false;

        RecalcBitmaps();

    return true;
}

bool wxSearchCtrl::SetBackgroundColour(const wxColour& colour)
{
    if ( !wxSearchCtrlBase::SetBackgroundColour(colour) )
        return false;

            RecalcBitmaps();

    return true;
}

void wxSearchCtrl::SetSearchBitmap( const wxBitmap& bitmap )
{
    m_searchBitmap = bitmap;
    m_searchBitmapUser = bitmap.IsOk();
    if ( m_searchBitmapUser )
    {
        if ( m_searchButton && !HasMenu() )
        {
            m_searchButton->SetBitmapLabel( m_searchBitmap );
        }
    }
    else
    {
                RecalcBitmaps();
    }
}

#if wxUSE_MENUS

void wxSearchCtrl::SetSearchMenuBitmap( const wxBitmap& bitmap )
{
    m_searchMenuBitmap = bitmap;
    m_searchMenuBitmapUser = bitmap.IsOk();
    if ( m_searchMenuBitmapUser )
    {
        if ( m_searchButton && m_menu )
        {
            m_searchButton->SetBitmapLabel( m_searchMenuBitmap );
        }
    }
    else
    {
                RecalcBitmaps();
    }
}

#endif 
void wxSearchCtrl::SetCancelBitmap( const wxBitmap& bitmap )
{
    m_cancelBitmap = bitmap;
    m_cancelBitmapUser = bitmap.IsOk();
    if ( m_cancelBitmapUser )
    {
        if ( m_cancelButton )
        {
            m_cancelButton->SetBitmapLabel( m_cancelBitmap );
        }
    }
    else
    {
                RecalcBitmaps();
    }
}

#if 0

#if wxHAS_TEXT_WINDOW_STREAM
int overflow(int i);
#endif 
wxTextCtrl& operator<<(const wxString& s);
wxTextCtrl& operator<<(int i);
wxTextCtrl& operator<<(long i);
wxTextCtrl& operator<<(float f);
wxTextCtrl& operator<<(double d);
wxTextCtrl& operator<<(const wxChar c);
#endif

void wxSearchCtrl::DoSetValue(const wxString& value, int flags)
{
    m_text->DoSetValue(value, flags);
}

bool wxSearchCtrl::DoLoadFile(const wxString& file, int fileType)
{
    return m_text->DoLoadFile(file, fileType);
}

bool wxSearchCtrl::DoSaveFile(const wxString& file, int fileType)
{
    return m_text->DoSaveFile(file, fileType);
}

void wxSearchCtrl::DoUpdateWindowUI(wxUpdateUIEvent& event)
{
    wxSearchCtrlBase::DoUpdateWindowUI(event);
}

bool wxSearchCtrl::ShouldInheritColours() const
{
    return true;
}

static int GetMultiplier()
{
    int depth = ::wxDisplayDepth();

    if  ( depth >= 24 )
    {
        return 8;
    }
    return 6;
}

static void RescaleBitmap(wxBitmap& bmp, const wxSize& sizeNeeded)
{
    wxCHECK_RET( sizeNeeded.IsFullySpecified(), wxS("New size must be given") );

#if wxUSE_IMAGE
    wxImage img = bmp.ConvertToImage();
    img.Rescale(sizeNeeded.x, sizeNeeded.y);
    bmp = wxBitmap(img);
#else         wxBitmap newBmp(sizeNeeded, bmp.GetDepth());
#if defined(__WXMSW__) || defined(__WXOSX__)
        newBmp.UseAlpha(bmp.HasAlpha());
#endif     {
        wxMemoryDC dc(newBmp);
        double scX = (double)sizeNeeded.GetWidth() / bmp.GetWidth();
        double scY = (double)sizeNeeded.GetHeight() / bmp.GetHeight();
        dc.SetUserScale(scX, scY);
        dc.DrawBitmap(bmp, 0, 0);
    }
    bmp = newBmp;
#endif }

wxBitmap wxSearchCtrl::RenderSearchBitmap( int x, int y, bool renderDrop )
{
    wxColour bg = GetBackgroundColour();
    wxColour fg = GetForegroundColour().ChangeLightness(LIGHT_STEP-20);

                
        if ( 14*x > y*20 )
    {
                x = y*20/14;
    }
    else
    {
                y = x*14/20;
    }

            
    int multiplier = GetMultiplier();
    int penWidth = multiplier * 2;

    penWidth = penWidth * x / 20;

    wxBitmap bitmap( multiplier*x, multiplier*y );
    wxMemoryDC mem;
    mem.SelectObject(bitmap);

        mem.SetBrush( wxBrush(bg) );
    mem.SetPen( wxPen(bg) );
    mem.DrawRectangle(0,0,bitmap.GetWidth(),bitmap.GetHeight());

        mem.SetBrush( wxBrush(fg) );
    mem.SetPen( wxPen(fg) );
    int glassBase = 5 * x / 20;
    int glassFactor = 2*glassBase + 1;
    int radius = multiplier*glassFactor/2;
    mem.DrawCircle(radius,radius,radius);
    mem.SetBrush( wxBrush(bg) );
    mem.SetPen( wxPen(bg) );
    mem.DrawCircle(radius,radius,radius-penWidth);

        int lineStart = radius + (radius-penWidth/2) * 707 / 1000; 
    mem.SetPen( wxPen(fg) );
    mem.SetBrush( wxBrush(fg) );
    int handleCornerShift = penWidth * 707 / 1000 / 2;     handleCornerShift = WXMAX( handleCornerShift, 1 );
    int handleBase = 4 * x / 20;
    int handleLength = 2*handleBase+1;
    wxPoint handlePolygon[] =
    {
        wxPoint(-handleCornerShift,+handleCornerShift),
        wxPoint(+handleCornerShift,-handleCornerShift),
        wxPoint(multiplier*handleLength/2+handleCornerShift,multiplier*handleLength/2-handleCornerShift),
        wxPoint(multiplier*handleLength/2-handleCornerShift,multiplier*handleLength/2+handleCornerShift),
    };
    mem.DrawPolygon(WXSIZEOF(handlePolygon),handlePolygon,lineStart,lineStart);

        int triangleX = 13 * x / 20;
    int triangleY = 5 * x / 20;
    int triangleBase = 3 * x / 20;
    int triangleFactor = triangleBase*2+1;
    if ( renderDrop )
    {
        wxPoint dropPolygon[] =
        {
            wxPoint(multiplier*0,multiplier*0),             wxPoint(multiplier*triangleFactor-1,multiplier*0),             wxPoint(multiplier*triangleFactor/2,multiplier*triangleFactor/2),         };
        mem.DrawPolygon(WXSIZEOF(dropPolygon),dropPolygon,multiplier*triangleX,multiplier*triangleY);
    }
    mem.SelectObject(wxNullBitmap);

            
    if ( multiplier != 1 )
    {
        RescaleBitmap(bitmap, wxSize(x, y));
    }
    if ( !renderDrop )
    {
                bitmap = bitmap.GetSubBitmap(wxRect(0,0, y,y));
    }

    return bitmap;
}

wxBitmap wxSearchCtrl::RenderCancelBitmap( int x, int y )
{
    wxColour bg = GetBackgroundColour();
    wxColour fg = GetForegroundColour().ChangeLightness(LIGHT_STEP);

                
            if ( x > y )
    {
                x = y;
    }
    else
    {
                y = x;
    }

            
    int multiplier = GetMultiplier();

    int penWidth = multiplier * x / 14;

    wxBitmap bitmap( multiplier*x, multiplier*y );
    wxMemoryDC mem;
    mem.SelectObject(bitmap);

        mem.SetBrush( wxBrush(bg) );
    mem.SetPen( wxPen(bg) );
    mem.DrawRectangle(0,0,bitmap.GetWidth(),bitmap.GetHeight());

        mem.SetBrush( wxBrush(fg) );
    mem.SetPen( wxPen(fg) );
    int radius = multiplier*x/2;
    mem.DrawCircle(radius,radius,radius);

        int lineStartBase = 4 * x / 14;
    int lineLength = x - 2*lineStartBase;

    mem.SetPen( wxPen(bg) );
    mem.SetBrush( wxBrush(bg) );
    int handleCornerShift = penWidth/2;
    handleCornerShift = WXMAX( handleCornerShift, 1 );
    wxPoint handlePolygon[] =
    {
        wxPoint(-handleCornerShift,+handleCornerShift),
        wxPoint(+handleCornerShift,-handleCornerShift),
        wxPoint(multiplier*lineLength+handleCornerShift,multiplier*lineLength-handleCornerShift),
        wxPoint(multiplier*lineLength-handleCornerShift,multiplier*lineLength+handleCornerShift),
    };
    mem.DrawPolygon(WXSIZEOF(handlePolygon),handlePolygon,multiplier*lineStartBase,multiplier*lineStartBase);
    wxPoint handlePolygon2[] =
    {
        wxPoint(+handleCornerShift,+handleCornerShift),
        wxPoint(-handleCornerShift,-handleCornerShift),
        wxPoint(multiplier*lineLength-handleCornerShift,-multiplier*lineLength-handleCornerShift),
        wxPoint(multiplier*lineLength+handleCornerShift,-multiplier*lineLength+handleCornerShift),
    };
    mem.DrawPolygon(WXSIZEOF(handlePolygon2),handlePolygon2,multiplier*lineStartBase,multiplier*(x-lineStartBase));

            
    if ( multiplier != 1 )
    {
        RescaleBitmap(bitmap, wxSize(x, y));
    }

    return bitmap;
}

void wxSearchCtrl::RecalcBitmaps()
{
    if ( !m_text )
    {
        return;
    }
    wxSize sizeText = m_text->GetBestSize();

    int bitmapHeight = sizeText.y - 4;
    int bitmapWidth  = sizeText.y * 20 / 14;

    if ( !m_searchBitmapUser )
    {
        if (
            !m_searchBitmap.IsOk() ||
            m_searchBitmap.GetHeight() != bitmapHeight ||
            m_searchBitmap.GetWidth() != bitmapWidth
            )
        {
            m_searchBitmap = RenderSearchBitmap(bitmapWidth,bitmapHeight,false);
            if ( !HasMenu() )
            {
                m_searchButton->SetBitmapLabel(m_searchBitmap);
            }
        }
            }

#if wxUSE_MENUS
    if ( !m_searchMenuBitmapUser )
    {
        if (
            !m_searchMenuBitmap.IsOk() ||
            m_searchMenuBitmap.GetHeight() != bitmapHeight ||
            m_searchMenuBitmap.GetWidth() != bitmapWidth
            )
        {
            m_searchMenuBitmap = RenderSearchBitmap(bitmapWidth,bitmapHeight,true);
            if ( m_menu )
            {
                m_searchButton->SetBitmapLabel(m_searchMenuBitmap);
            }
        }
            }
#endif 
    if ( !m_cancelBitmapUser )
    {
        if (
            !m_cancelBitmap.IsOk() ||
            m_cancelBitmap.GetHeight() != bitmapHeight ||
            m_cancelBitmap.GetWidth() != bitmapHeight
            )
        {
            m_cancelBitmap = RenderCancelBitmap(bitmapHeight,bitmapHeight);             m_cancelButton->SetBitmapLabel(m_cancelBitmap);
        }
            }
}

void wxSearchCtrl::OnCancelButton( wxCommandEvent& event )
{
    m_text->Clear();
    event.Skip();
}

void wxSearchCtrl::OnSetFocus( wxFocusEvent&  )
{
    if ( m_text )
    {
        m_text->SetFocus();
    }
}

void wxSearchCtrl::OnSize( wxSizeEvent& WXUNUSED(event) )
{
    LayoutControls();
}

#if wxUSE_MENUS

void wxSearchCtrl::PopupSearchMenu()
{
    if ( m_menu )
    {
        wxSize size = GetSize();
        PopupMenu( m_menu, 0, size.y );
    }
}

#endif 
#endif 
#endif 