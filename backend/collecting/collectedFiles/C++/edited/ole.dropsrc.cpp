


#include "wx/wxprec.h"

#if defined(__BORLANDC__)
  #pragma hdrstop
#endif

#if wxUSE_OLE && wxUSE_DRAG_AND_DROP

#ifndef WX_PRECOMP
    #include "wx/window.h"
    #include "wx/log.h"
#endif

#include "wx/dnd.h"

#include "wx/msw/private.h"

#include <oleauto.h>

#include "wx/msw/ole/oleutils.h"


class wxIDropSource : public IDropSource
{
public:
  wxIDropSource(wxDropSource *pDropSource);
  virtual ~wxIDropSource() { }

    STDMETHODIMP QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
  STDMETHODIMP GiveFeedback(DWORD dwEffect);

    DECLARE_IUNKNOWN_METHODS;

private:
  DWORD         m_grfInitKeyState;    wxDropSource *m_pDropSource;      
  wxDECLARE_NO_COPY_CLASS(wxIDropSource);
};


BEGIN_IID_TABLE(wxIDropSource)
  ADD_IID(Unknown)
  ADD_IID(DropSource)
END_IID_TABLE;

IMPLEMENT_IUNKNOWN_METHODS(wxIDropSource)

wxIDropSource::wxIDropSource(wxDropSource *pDropSource)
{
  wxASSERT( pDropSource != NULL );

  m_pDropSource = pDropSource;
  m_grfInitKeyState = 0;
}

STDMETHODIMP wxIDropSource::QueryContinueDrag(BOOL fEscapePressed,
                                              DWORD grfKeyState)
{
  if ( fEscapePressed )
    return DRAGDROP_S_CANCEL;

    if ( m_grfInitKeyState == 0 ) {
    m_grfInitKeyState = grfKeyState & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON);
  }

  if ( !(grfKeyState & m_grfInitKeyState) ) {
        return DRAGDROP_S_DROP;
  }

  return S_OK;
}

STDMETHODIMP wxIDropSource::GiveFeedback(DWORD dwEffect)
{
  wxDragResult effect;
  if ( dwEffect & DROPEFFECT_COPY )
    effect = wxDragCopy;
  else if ( dwEffect & DROPEFFECT_MOVE )
    effect = wxDragMove;
  else
    effect = wxDragNone;

  if ( m_pDropSource->GiveFeedback(effect) )
    return S_OK;

  return DRAGDROP_S_USEDEFAULTCURSORS;
}



void wxDropSource::Init()
{
    m_pIDropSource = new wxIDropSource(this);
    m_pIDropSource->AddRef();
}

wxDropSource::wxDropSource(wxWindow* WXUNUSED(win),
                           const wxCursor &cursorCopy,
                           const wxCursor &cursorMove,
                           const wxCursor &cursorStop)
            : wxDropSourceBase(cursorCopy, cursorMove, cursorStop)
{
    Init();
}

wxDropSource::wxDropSource(wxDataObject& data,
                           wxWindow* WXUNUSED(win),
                           const wxCursor &cursorCopy,
                           const wxCursor &cursorMove,
                           const wxCursor &cursorStop)
            : wxDropSourceBase(cursorCopy, cursorMove, cursorStop)
{
    Init();
    SetData(data);
}

wxDropSource::~wxDropSource()
{
    m_pIDropSource->Release();
}

wxDragResult wxDropSource::DoDragDrop(int flags)
{
  wxCHECK_MSG( m_data != NULL, wxDragNone, wxT("No data in wxDropSource!") );

  DWORD dwEffect;
  HRESULT hr = ::DoDragDrop(m_data->GetInterface(),
                            m_pIDropSource,
                            (flags & wxDrag_AllowMove)
                                ? DROPEFFECT_COPY | DROPEFFECT_MOVE
                                : DROPEFFECT_COPY,
                            &dwEffect);

  if ( hr == DRAGDROP_S_CANCEL ) {
    return wxDragCancel;
  }
  else if ( hr == DRAGDROP_S_DROP ) {
    if ( dwEffect & DROPEFFECT_COPY ) {
      return wxDragCopy;
    }
    else if ( dwEffect & DROPEFFECT_MOVE ) {
                  return (flags & wxDrag_AllowMove) ? wxDragMove : wxDragCopy;
    }
    else {
            return wxDragNone;
    }
  }
  else {
    if ( FAILED(hr) ) {
      wxLogApiError(wxT("DoDragDrop"), hr);
      wxLogError(wxT("Drag & drop operation failed."));
    }
    else {
      wxLogDebug(wxT("Unexpected success return code %08lx from DoDragDrop."),
                 hr);
    }

    return wxDragError;
  }
}

bool wxDropSource::GiveFeedback(wxDragResult effect)
{
    const wxCursor& cursor = GetCursor(effect);
    if ( cursor.IsOk() )
    {
        ::SetCursor((HCURSOR)cursor.GetHCURSOR());

        return true;
    }
    else
    {
        return false;
    }
}

#endif  