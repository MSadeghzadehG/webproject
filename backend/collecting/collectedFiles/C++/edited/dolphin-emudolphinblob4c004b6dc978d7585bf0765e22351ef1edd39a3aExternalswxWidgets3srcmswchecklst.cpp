


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_CHECKLISTBOX && wxUSE_OWNER_DRAWN

#include "wx/checklst.h"

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcctl.h"
    #include "wx/object.h"
    #include "wx/colour.h"
    #include "wx/font.h"
    #include "wx/bitmap.h"
    #include "wx/window.h"
    #include "wx/listbox.h"
    #include "wx/dcmemory.h"
    #include "wx/settings.h"
    #include "wx/log.h"
#endif

#include "wx/ownerdrw.h"

#include <windowsx.h>

#include "wx/renderer.h"
#include "wx/msw/private.h"
#include "wx/msw/dc.h"


#define GetItem(n)    ((wxCheckListBoxItem *)(GetItem(n)))

namespace
{
        static const int CHECKMARK_EXTRA_SPACE = 1;

        static const int CHECKMARK_LABEL_SPACE = 2;

} 


class wxCheckListBoxItem : public wxOwnerDrawn
{
public:
        wxCheckListBoxItem(wxCheckListBox *parent);

        virtual bool OnDrawItem(wxDC& dc, const wxRect& rc, wxODAction act, wxODStatus stat);

        wxCheckListBox *GetParent() const
        { return m_parent; }

    int GetIndex() const
        { return m_parent->GetItemIndex(const_cast<wxCheckListBoxItem*>(this)); }

    wxString GetName() const
        { return m_parent->GetString(GetIndex()); }


    bool IsChecked() const
        { return m_checked; }

    void Check(bool bCheck)
        { m_checked = bCheck; }

    void Toggle()
        { Check(!IsChecked()); }

private:
    wxCheckListBox *m_parent;
    bool m_checked;

    wxDECLARE_NO_COPY_CLASS(wxCheckListBoxItem);
};

wxCheckListBoxItem::wxCheckListBoxItem(wxCheckListBox *parent)
{
    m_parent = parent;
    m_checked = false;

    wxSize size = wxRendererNative::Get().GetCheckBoxSize(parent);
    size.x += 2 * CHECKMARK_EXTRA_SPACE + CHECKMARK_LABEL_SPACE;

    SetMarginWidth(size.GetWidth());
    SetBackgroundColour(parent->GetBackgroundColour());
}

bool wxCheckListBoxItem::OnDrawItem(wxDC& dc, const wxRect& rc,
                                    wxODAction act, wxODStatus stat)
{
        if ( !wxOwnerDrawn::OnDrawItem(dc, rc, act, stat) )
        return false;

        wxMSWDCImpl *impl = (wxMSWDCImpl*) dc.GetImpl();
    HDC hdc = GetHdcOf(*impl);

    wxSize size = wxRendererNative::Get().GetCheckBoxSize(GetParent());

        MemoryHDC hdcMem(hdc);
    CompatibleBitmap hBmpCheck(hdc, size.GetWidth(), size.GetHeight());

        {
        SelectInHDC selBmp(hdcMem, hBmpCheck);

        int flags = wxCONTROL_FLAT;
        if ( IsChecked() )
            flags |= wxCONTROL_CHECKED;

        wxDCTemp dcMem(hdcMem);
        wxRendererNative::Get().DrawCheckBox(GetParent(), dcMem, wxRect(size), flags);
    } 
    
        int x = rc.GetX() + CHECKMARK_EXTRA_SPACE;
    int y = rc.GetY() + (rc.GetHeight() - size.GetHeight()) / 2;

    UINT uState = stat & wxOwnerDrawn::wxODSelected ? wxDSB_SELECTED : wxDSB_NORMAL;
    wxDrawStateBitmap(hdc, hBmpCheck, x, y, uState);

    return true;
}


wxBEGIN_EVENT_TABLE(wxCheckListBox, wxListBox)
  EVT_KEY_DOWN(wxCheckListBox::OnKeyDown)
  EVT_LEFT_DOWN(wxCheckListBox::OnLeftClick)
wxEND_EVENT_TABLE()


wxCheckListBox::wxCheckListBox()
{
}

wxCheckListBox::wxCheckListBox(wxWindow *parent, wxWindowID id,
                               const wxPoint& pos, const wxSize& size,
                               int nStrings, const wxString choices[],
                               long style, const wxValidator& val,
                               const wxString& name)
{
    Create(parent, id, pos, size, nStrings, choices, style, val, name);
}

wxCheckListBox::wxCheckListBox(wxWindow *parent, wxWindowID id,
                               const wxPoint& pos, const wxSize& size,
                               const wxArrayString& choices,
                               long style, const wxValidator& val,
                               const wxString& name)
{
    Create(parent, id, pos, size, choices, style, val, name);
}

bool wxCheckListBox::Create(wxWindow *parent, wxWindowID id,
                            const wxPoint& pos, const wxSize& size,
                            int n, const wxString choices[],
                            long style,
                            const wxValidator& validator, const wxString& name)
{
    return wxListBox::Create(parent, id, pos, size, n, choices,
                             style | wxLB_OWNERDRAW, validator, name);
}

bool wxCheckListBox::Create(wxWindow *parent, wxWindowID id,
                            const wxPoint& pos, const wxSize& size,
                            const wxArrayString& choices,
                            long style,
                            const wxValidator& validator, const wxString& name)
{
    return wxListBox::Create(parent, id, pos, size, choices,
                             style | wxLB_OWNERDRAW, validator, name);
}


wxOwnerDrawn *wxCheckListBox::CreateLboxItem(size_t WXUNUSED(n))
{
    wxCheckListBoxItem *pItem = new wxCheckListBoxItem(this);
    return pItem;
}

bool wxCheckListBox::MSWOnMeasure(WXMEASUREITEMSTRUCT *item)
{
    if ( wxListBox::MSWOnMeasure(item) )
    {
        MEASUREITEMSTRUCT *pStruct = (MEASUREITEMSTRUCT *)item;

        wxSize size = wxRendererNative::Get().GetCheckBoxSize(this);
        size.x += 2 * CHECKMARK_EXTRA_SPACE;
        size.y += 2 * CHECKMARK_EXTRA_SPACE;

                pStruct->itemWidth += size.GetWidth();

        if ( pStruct->itemHeight < static_cast<unsigned int>(size.GetHeight()) )
            pStruct->itemHeight = size.GetHeight();

        return true;
    }

    return false;
  }


bool wxCheckListBox::IsChecked(unsigned int uiIndex) const
{
    wxCHECK_MSG( IsValid(uiIndex), false, wxT("bad wxCheckListBox index") );

    return GetItem(uiIndex)->IsChecked();
}

void wxCheckListBox::Check(unsigned int uiIndex, bool bCheck)
{
    wxCHECK_RET( IsValid(uiIndex), wxT("bad wxCheckListBox index") );

    GetItem(uiIndex)->Check(bCheck);
    RefreshItem(uiIndex);
}

void wxCheckListBox::Toggle(unsigned int uiIndex)
{
    wxCHECK_RET( IsValid(uiIndex), wxT("bad wxCheckListBox index") );

    GetItem(uiIndex)->Toggle();
    RefreshItem(uiIndex);
}


void wxCheckListBox::OnKeyDown(wxKeyEvent& event)
{
        enum
    {
        NONE,
        TOGGLE,
        SET,
        CLEAR
    } oper;

    switch ( event.GetKeyCode() )
    {
        case WXK_SPACE:
            oper = TOGGLE;
            break;

        case WXK_NUMPAD_ADD:
        case '+':
            oper = SET;
            break;

        case WXK_NUMPAD_SUBTRACT:
        case '-':
            oper = CLEAR;
            break;

        default:
            oper = NONE;
    }

    if ( oper != NONE )
    {
        wxArrayInt selections;
        int count = 0;
        if ( HasMultipleSelection() )
        {
            count = GetSelections(selections);
        }
        else
        {
            int sel = GetSelection();
            if (sel != -1)
            {
                count = 1;
                selections.Add(sel);
            }
        }

        for ( int i = 0; i < count; i++ )
        {
            int nItem = selections[i];

            switch ( oper )
            {
                case TOGGLE:
                    Toggle(nItem);
                    break;

                case SET:
                case CLEAR:
                    Check(nItem, oper == SET);
                    break;

                default:
                    wxFAIL_MSG( wxT("what should this key do?") );
            }

                                    SendEvent(nItem);
        }
    }
    else     {
        event.Skip();
    }
}

void wxCheckListBox::OnLeftClick(wxMouseEvent& event)
{
    
    int nItem = HitTest(event.GetX(), event.GetY());

    if ( nItem != wxNOT_FOUND )
    {
        wxRect rect;
        GetItemRect(nItem, rect);

                wxSize size = wxRendererNative::Get().GetCheckBoxSize(this);
        rect.x += CHECKMARK_EXTRA_SPACE;
        rect.y += (rect.GetHeight() - size.GetHeight()) / 2;
        rect.SetSize(size);

        if ( rect.Contains(event.GetX(), event.GetY()) )
        {
                                                                                    SetFocus();
            if ( FindFocus() == this )
            {
                Toggle(nItem);
                SendEvent(nItem);

                                                int h;
                GetClientSize(NULL, &h);
                if ( rect.GetBottom() > h )
                    ScrollLines(1);
            }
        }
        else
        {
                        event.Skip();
        }
    }
    else
    {
                event.Skip();
    }
}

wxSize wxCheckListBox::DoGetBestClientSize() const
{
    wxSize best = wxListBox::DoGetBestClientSize();

        wxSize size = wxRendererNative::Get().GetCheckBoxSize(const_cast<wxCheckListBox*>(this));
    size.x += 2 * CHECKMARK_EXTRA_SPACE;
    size.y += 2 * CHECKMARK_EXTRA_SPACE;

    best.x += size.GetWidth();
    if ( best.y < size.GetHeight() )
        best.y = size.GetHeight();

    CacheBestSize(best);
    return best;
}

#endif 