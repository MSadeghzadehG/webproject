
#ifndef _WX_DND_H_BASE_
#define _WX_DND_H_BASE_

#include "wx/defs.h"

#if wxUSE_DRAG_AND_DROP

#include "wx/dataobj.h"
#include "wx/cursor.h"


enum
{
    wxDrag_CopyOnly    = 0,     wxDrag_AllowMove   = 1,     wxDrag_DefaultMove = 3  };

enum wxDragResult
{
    wxDragError,        wxDragNone,         wxDragCopy,         wxDragMove,         wxDragLink,         wxDragCancel    };

WXDLLIMPEXP_CORE bool wxIsDragResultOk(wxDragResult res);


class WXDLLIMPEXP_CORE wxDropSourceBase
{
public:
    wxDropSourceBase(const wxCursor &cursorCopy = wxNullCursor,
                     const wxCursor &cursorMove = wxNullCursor,
                     const wxCursor &cursorStop = wxNullCursor)
        : m_cursorCopy(cursorCopy),
          m_cursorMove(cursorMove),
          m_cursorStop(cursorStop)
        { m_data = NULL; }
    virtual ~wxDropSourceBase() { }

        void SetData(wxDataObject& data)
      { m_data = &data; }

    wxDataObject *GetDataObject()
      { return m_data; }

        void SetCursor(wxDragResult res, const wxCursor& cursor)
    {
        if ( res == wxDragCopy )
            m_cursorCopy = cursor;
        else if ( res == wxDragMove )
            m_cursorMove = cursor;
        else
            m_cursorStop = cursor;
    }

                        virtual wxDragResult DoDragDrop(int flags = wxDrag_CopyOnly) = 0;

                virtual bool GiveFeedback(wxDragResult WXUNUSED(effect)) { return false; }

protected:
    const wxCursor& GetCursor(wxDragResult res) const
    {
        if ( res == wxDragCopy )
            return m_cursorCopy;
        else if ( res == wxDragMove )
            return m_cursorMove;
        else
            return m_cursorStop;
    }

        wxDataObject *m_data;

        wxCursor m_cursorCopy,
             m_cursorMove,
             m_cursorStop;

    wxDECLARE_NO_COPY_CLASS(wxDropSourceBase);
};


class WXDLLIMPEXP_CORE wxDropTargetBase
{
public:
                wxDropTargetBase(wxDataObject *dataObject = NULL)
        { m_dataObject = dataObject; m_defaultAction = wxDragNone; }
        virtual ~wxDropTargetBase()
        { delete m_dataObject; }

        wxDataObject *GetDataObject() const
        { return m_dataObject; }
    void SetDataObject(wxDataObject *dataObject)
        { if (m_dataObject) delete m_dataObject;
    m_dataObject = dataObject; }

                                
        virtual wxDragResult OnEnter(wxCoord x, wxCoord y, wxDragResult def)
        { return OnDragOver(x, y, def); }

            virtual wxDragResult OnDragOver(wxCoord WXUNUSED(x), wxCoord WXUNUSED(y),
                                    wxDragResult def)
        { return def; }

            virtual void OnLeave() { }

                virtual bool OnDrop(wxCoord x, wxCoord y) = 0;

                        virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def) = 0;

            virtual bool GetData() = 0;

                    void SetDefaultAction(wxDragResult action)
        { m_defaultAction = action; }

            wxDragResult GetDefaultAction()
        { return m_defaultAction; }

protected:
    wxDataObject *m_dataObject;
    wxDragResult m_defaultAction;

    wxDECLARE_NO_COPY_CLASS(wxDropTargetBase);
};


#if defined(__WXMSW__)
    #include "wx/msw/ole/dropsrc.h"
    #include "wx/msw/ole/droptgt.h"
#elif defined(__WXMOTIF__)
    #include "wx/motif/dnd.h"
#elif defined(__WXX11__)
    #include "wx/x11/dnd.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/dnd.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/dnd.h"
#elif defined(__WXMAC__)
    #include "wx/osx/dnd.h"
#elif defined(__WXQT__)
    #include "wx/qt/dnd.h"
#endif


class WXDLLIMPEXP_CORE wxTextDropTarget : public wxDropTarget
{
public:
    wxTextDropTarget();

    virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& text) = 0;

    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def) wxOVERRIDE;

private:
    wxDECLARE_NO_COPY_CLASS(wxTextDropTarget);
};

class WXDLLIMPEXP_CORE wxFileDropTarget : public wxDropTarget
{
public:
    wxFileDropTarget();

        virtual bool OnDropFiles(wxCoord x, wxCoord y,
                             const wxArrayString& filenames) = 0;

    virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def) wxOVERRIDE;

private:
    wxDECLARE_NO_COPY_CLASS(wxFileDropTarget);
};

#endif 
#endif 