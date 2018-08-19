
#ifndef _WX_EVENT_H_
#define _WX_EVENT_H_

#include "wx/defs.h"
#include "wx/cpp.h"
#include "wx/object.h"
#include "wx/clntdata.h"

#if wxUSE_GUI
    #include "wx/gdicmn.h"
    #include "wx/cursor.h"
    #include "wx/mousestate.h"
#endif

#include "wx/dynarray.h"
#include "wx/thread.h"
#include "wx/tracker.h"
#include "wx/typeinfo.h"
#include "wx/any.h"
#include "wx/vector.h"

#include "wx/meta/convertible.h"

#if !defined(__VISUALC__) || wxCHECK_VISUALC_VERSION(8)
    #include "wx/meta/removeref.h"

    #define wxHAS_CALL_AFTER
#endif


class WXDLLIMPEXP_FWD_BASE wxList;
class WXDLLIMPEXP_FWD_BASE wxEvent;
class WXDLLIMPEXP_FWD_BASE wxEventFilter;
#if wxUSE_GUI
    class WXDLLIMPEXP_FWD_CORE wxDC;
    class WXDLLIMPEXP_FWD_CORE wxMenu;
    class WXDLLIMPEXP_FWD_CORE wxWindow;
    class WXDLLIMPEXP_FWD_CORE wxWindowBase;
#endif 
#ifdef __VISUALC__
    #define wxMSVC_FWD_MULTIPLE_BASES __multiple_inheritance
#else
    #define wxMSVC_FWD_MULTIPLE_BASES
#endif

class WXDLLIMPEXP_FWD_BASE wxMSVC_FWD_MULTIPLE_BASES wxEvtHandler;
class wxEventConnectionRef;


typedef int wxEventType;

#define wxEVT_ANY           ((wxEventType)-1)

#define wxStaticCastEvent(type, val) static_cast<type>(val)

#define wxDECLARE_EVENT_TABLE_ENTRY(type, winid, idLast, fn, obj) \
    wxEventTableEntry(type, winid, idLast, wxNewEventTableFunctor(type, fn), obj)

#define wxDECLARE_EVENT_TABLE_TERMINATOR() \
    wxEventTableEntry(wxEVT_NULL, 0, 0, 0, 0)

extern WXDLLIMPEXP_BASE wxEventType wxNewEventType();


#define wxDEFINE_EVENT( name, type ) \
    const wxEventTypeTag< type > name( wxNewEventType() )

#define wxDECLARE_EXPORTED_EVENT( expdecl, name, type ) \
    extern const expdecl wxEventTypeTag< type > name

#define wxDECLARE_EVENT( name, type ) \
    wxDECLARE_EXPORTED_EVENT( wxEMPTY_PARAMETER_VALUE, name, type )


#define wxDEFINE_EVENT_ALIAS( name, type, value ) \
    const wxEventTypeTag< type > name( value )

#define wxDECLARE_EXPORTED_EVENT_ALIAS( expdecl, name, type ) \
    extern const expdecl wxEventTypeTag< type > name


#define wxEVENT_HANDLER_CAST( functype, func ) \
    ( wxObjectEventFunction )( wxEventFunction )wxStaticCastEvent( functype, &func )


template <typename T>
class wxEventTypeTag
{
public:
        typedef T EventClass;

    wxEventTypeTag(wxEventType type) { m_type = type; }

                operator const wxEventType&() const { return m_type; }

private:
    wxEventType m_type;
};

typedef void (wxEvtHandler::*wxEventFunction)(wxEvent&);

typedef wxEventFunction wxObjectEventFunction;

class WXDLLIMPEXP_BASE wxEventFunctor
{
public:
    virtual ~wxEventFunctor();

        virtual void operator()(wxEvtHandler *, wxEvent&) = 0;

            virtual bool IsMatching(const wxEventFunctor& functor) const = 0;

            virtual wxEvtHandler *GetEvtHandler() const
        { return NULL; }

                    virtual wxEventFunction GetEvtMethod() const
        { return NULL; }

private:
    WX_DECLARE_ABSTRACT_TYPEINFO(wxEventFunctor)
};

class WXDLLIMPEXP_BASE wxObjectEventFunctor : public wxEventFunctor
{
public:
    wxObjectEventFunctor(wxObjectEventFunction method, wxEvtHandler *handler)
        : m_handler( handler ), m_method( method )
        { }

    virtual void operator()(wxEvtHandler *handler, wxEvent& event) wxOVERRIDE;

    virtual bool IsMatching(const wxEventFunctor& functor) const wxOVERRIDE
    {
        if ( wxTypeId(functor) == wxTypeId(*this) )
        {
            const wxObjectEventFunctor &other =
                static_cast< const wxObjectEventFunctor & >( functor );

            return ( m_method == other.m_method || !other.m_method ) &&
                   ( m_handler == other.m_handler || !other.m_handler );
        }
        else
            return false;
    }

    virtual wxEvtHandler *GetEvtHandler() const wxOVERRIDE
        { return m_handler; }

    virtual wxEventFunction GetEvtMethod() const wxOVERRIDE
        { return m_method; }

private:
    wxEvtHandler *m_handler;
    wxEventFunction m_method;

        wxObjectEventFunctor() { }

    WX_DECLARE_TYPEINFO_INLINE(wxObjectEventFunctor)
};

inline wxObjectEventFunctor *
wxNewEventFunctor(const wxEventType& WXUNUSED(evtType),
                  wxObjectEventFunction method,
                  wxEvtHandler *handler)
{
    return new wxObjectEventFunctor(method, handler);
}

inline wxObjectEventFunctor *
wxNewEventTableFunctor(const wxEventType& WXUNUSED(evtType),
                       wxObjectEventFunction method)
{
    return new wxObjectEventFunctor(method, NULL);
}

inline wxObjectEventFunctor
wxMakeEventFunctor(const wxEventType& WXUNUSED(evtType),
                        wxObjectEventFunction method,
                        wxEvtHandler *handler)
{
    return wxObjectEventFunctor(method, handler);
}

namespace wxPrivate
{

template <typename T> struct EventClassOf;

template <typename T>
struct EventClassOf< wxEventTypeTag<T> >
{
    typedef typename wxEventTypeTag<T>::EventClass type;
};

template <>
struct EventClassOf<wxEventType>
{
    typedef wxEvent type;
};


template <typename T, typename A, bool> struct HandlerImpl;

template <typename T, typename A>
struct HandlerImpl<T, A, true>
{
    static bool IsEvtHandler()
        { return true; }
    static T *ConvertFromEvtHandler(wxEvtHandler *p)
        { return static_cast<T *>(p); }
    static wxEvtHandler *ConvertToEvtHandler(T *p)
        { return p; }
    static wxEventFunction ConvertToEvtMethod(void (T::*f)(A&))
        { return static_cast<wxEventFunction>(
                    reinterpret_cast<void (T::*)(wxEvent&)>(f)); }
};

template <typename T, typename A>
struct HandlerImpl<T, A, false>
{
    static bool IsEvtHandler()
        { return false; }
    static T *ConvertFromEvtHandler(wxEvtHandler *)
        { return NULL; }
    static wxEvtHandler *ConvertToEvtHandler(T *)
        { return NULL; }
    static wxEventFunction ConvertToEvtMethod(void (T::*)(A&))
        { return NULL; }
};

} 
template
  <typename EventTag, typename Class, typename EventArg, typename EventHandler>
class wxEventFunctorMethod
    : public wxEventFunctor,
      private wxPrivate::HandlerImpl
              <
                Class,
                EventArg,
                wxConvertibleTo<Class, wxEvtHandler>::value != 0
              >
{
private:
    static void CheckHandlerArgument(EventArg *) { }

public:
        typedef typename wxPrivate::EventClassOf<EventTag>::type EventClass;


    wxEventFunctorMethod(void (Class::*method)(EventArg&), EventHandler *handler)
        : m_handler( handler ), m_method( method )
    {
        wxASSERT_MSG( handler || this->IsEvtHandler(),
                      "handlers defined in non-wxEvtHandler-derived classes "
                      "must be connected with a valid sink object" );

                                CheckHandlerArgument(static_cast<EventClass *>(NULL));
    }

    virtual void operator()(wxEvtHandler *handler, wxEvent& event)
    {
        Class * realHandler = m_handler;
        if ( !realHandler )
        {
            realHandler = this->ConvertFromEvtHandler(handler);

                        wxCHECK_RET( realHandler, "invalid event handler" );
        }

                                (realHandler->*m_method)(static_cast<EventArg&>(event));
    }

    virtual bool IsMatching(const wxEventFunctor& functor) const
    {
        if ( wxTypeId(functor) != wxTypeId(*this) )
            return false;

        typedef wxEventFunctorMethod<EventTag, Class, EventArg, EventHandler>
            ThisFunctor;

                const ThisFunctor& other = static_cast<const ThisFunctor &>(functor);

        return (m_method == other.m_method || other.m_method == NULL) &&
               (m_handler == other.m_handler || other.m_handler == NULL);
    }

    virtual wxEvtHandler *GetEvtHandler() const
        { return this->ConvertToEvtHandler(m_handler); }

    virtual wxEventFunction GetEvtMethod() const
        { return this->ConvertToEvtMethod(m_method); }

private:
    EventHandler *m_handler;
    void (Class::*m_method)(EventArg&);

        wxEventFunctorMethod() { }

    typedef wxEventFunctorMethod<EventTag, Class,
                                 EventArg, EventHandler> thisClass;
    WX_DECLARE_TYPEINFO_INLINE(thisClass)
};


template <typename EventTag, typename EventArg>
class wxEventFunctorFunction : public wxEventFunctor
{
private:
    static void CheckHandlerArgument(EventArg *) { }

public:
        typedef typename wxPrivate::EventClassOf<EventTag>::type EventClass;

    wxEventFunctorFunction( void ( *handler )( EventArg & ))
        : m_handler( handler )
    {
                                CheckHandlerArgument(static_cast<EventClass *>(NULL));
    }

    virtual void operator()(wxEvtHandler *WXUNUSED(handler), wxEvent& event) wxOVERRIDE
    {
                                                                m_handler(static_cast<EventArg&>(event));
    }

    virtual bool IsMatching(const wxEventFunctor &functor) const wxOVERRIDE
    {
        if ( wxTypeId(functor) != wxTypeId(*this) )
            return false;

        typedef wxEventFunctorFunction<EventTag, EventArg> ThisFunctor;

        const ThisFunctor& other = static_cast<const ThisFunctor&>( functor );

        return m_handler == other.m_handler;
    }

private:
    void (*m_handler)(EventArg&);

        wxEventFunctorFunction() { }

    typedef wxEventFunctorFunction<EventTag, EventArg> thisClass;
    WX_DECLARE_TYPEINFO_INLINE(thisClass)
};


template <typename EventTag, typename Functor>
class wxEventFunctorFunctor : public wxEventFunctor
{
public:
    typedef typename EventTag::EventClass EventArg;

    wxEventFunctorFunctor(const Functor& handler)
        : m_handler(handler), m_handlerAddr(&handler)
        { }

    virtual void operator()(wxEvtHandler *WXUNUSED(handler), wxEvent& event) wxOVERRIDE
    {
                                                                m_handler(static_cast<EventArg&>(event));
    }

    virtual bool IsMatching(const wxEventFunctor &functor) const wxOVERRIDE
    {
        if ( wxTypeId(functor) != wxTypeId(*this) )
            return false;

        typedef wxEventFunctorFunctor<EventTag, Functor> FunctorThis;

        const FunctorThis& other = static_cast<const FunctorThis&>(functor);

                        return m_handlerAddr == other.m_handlerAddr;
    }

private:
            Functor m_handler;

        const void *m_handlerAddr;

        wxEventFunctorFunctor() { }

    typedef wxEventFunctorFunctor<EventTag, Functor> thisClass;
    WX_DECLARE_TYPEINFO_INLINE(thisClass)
};



template <typename EventTag, typename EventArg>
inline wxEventFunctorFunction<EventTag, EventArg> *
wxNewEventFunctor(const EventTag&, void (*func)(EventArg &))
{
    return new wxEventFunctorFunction<EventTag, EventArg>(func);
}

template <typename EventTag, typename EventArg>
inline wxEventFunctorFunction<EventTag, EventArg>
wxMakeEventFunctor(const EventTag&, void (*func)(EventArg &))
{
    return wxEventFunctorFunction<EventTag, EventArg>(func);
}

template <typename EventTag, typename Functor>
inline wxEventFunctorFunctor<EventTag, Functor> *
wxNewEventFunctor(const EventTag&, const Functor &func)
{
    return new wxEventFunctorFunctor<EventTag, Functor>(func);
}

template <typename EventTag, typename Functor>
inline wxEventFunctorFunctor<EventTag, Functor>
wxMakeEventFunctor(const EventTag&, const Functor &func)
{
    return wxEventFunctorFunctor<EventTag, Functor>(func);
}

template
  <typename EventTag, typename Class, typename EventArg, typename EventHandler>
inline wxEventFunctorMethod<EventTag, Class, EventArg, EventHandler> *
wxNewEventFunctor(const EventTag&,
                  void (Class::*method)(EventArg&),
                  EventHandler *handler)
{
    return new wxEventFunctorMethod<EventTag, Class, EventArg, EventHandler>(
                method, handler);
}

template
    <typename EventTag, typename Class, typename EventArg, typename EventHandler>
inline wxEventFunctorMethod<EventTag, Class, EventArg, EventHandler>
wxMakeEventFunctor(const EventTag&,
                   void (Class::*method)(EventArg&),
                   EventHandler *handler)
{
    return wxEventFunctorMethod<EventTag, Class, EventArg, EventHandler>(
                method, handler);
}

template <typename EventTag, typename Class, typename EventArg>
inline wxEventFunctorMethod<EventTag, Class, EventArg, Class> *
wxNewEventTableFunctor(const EventTag&, void (Class::*method)(EventArg&))
{
    return new wxEventFunctorMethod<EventTag, Class, EventArg, Class>(
                    method, NULL);
}



    extern WXDLLIMPEXP_BASE const wxEventType wxEVT_NULL;
extern WXDLLIMPEXP_BASE const wxEventType wxEVT_FIRST;
extern WXDLLIMPEXP_BASE const wxEventType wxEVT_USER_FIRST;

    class WXDLLIMPEXP_FWD_BASE wxIdleEvent;
class WXDLLIMPEXP_FWD_BASE wxThreadEvent;
class WXDLLIMPEXP_FWD_BASE wxAsyncMethodCallEvent;
class WXDLLIMPEXP_FWD_CORE wxCommandEvent;
class WXDLLIMPEXP_FWD_CORE wxMouseEvent;
class WXDLLIMPEXP_FWD_CORE wxFocusEvent;
class WXDLLIMPEXP_FWD_CORE wxChildFocusEvent;
class WXDLLIMPEXP_FWD_CORE wxKeyEvent;
class WXDLLIMPEXP_FWD_CORE wxNavigationKeyEvent;
class WXDLLIMPEXP_FWD_CORE wxSetCursorEvent;
class WXDLLIMPEXP_FWD_CORE wxScrollEvent;
class WXDLLIMPEXP_FWD_CORE wxSpinEvent;
class WXDLLIMPEXP_FWD_CORE wxScrollWinEvent;
class WXDLLIMPEXP_FWD_CORE wxSizeEvent;
class WXDLLIMPEXP_FWD_CORE wxMoveEvent;
class WXDLLIMPEXP_FWD_CORE wxCloseEvent;
class WXDLLIMPEXP_FWD_CORE wxActivateEvent;
class WXDLLIMPEXP_FWD_CORE wxWindowCreateEvent;
class WXDLLIMPEXP_FWD_CORE wxWindowDestroyEvent;
class WXDLLIMPEXP_FWD_CORE wxShowEvent;
class WXDLLIMPEXP_FWD_CORE wxIconizeEvent;
class WXDLLIMPEXP_FWD_CORE wxMaximizeEvent;
class WXDLLIMPEXP_FWD_CORE wxMouseCaptureChangedEvent;
class WXDLLIMPEXP_FWD_CORE wxMouseCaptureLostEvent;
class WXDLLIMPEXP_FWD_CORE wxPaintEvent;
class WXDLLIMPEXP_FWD_CORE wxEraseEvent;
class WXDLLIMPEXP_FWD_CORE wxNcPaintEvent;
class WXDLLIMPEXP_FWD_CORE wxMenuEvent;
class WXDLLIMPEXP_FWD_CORE wxContextMenuEvent;
class WXDLLIMPEXP_FWD_CORE wxSysColourChangedEvent;
class WXDLLIMPEXP_FWD_CORE wxDisplayChangedEvent;
class WXDLLIMPEXP_FWD_CORE wxQueryNewPaletteEvent;
class WXDLLIMPEXP_FWD_CORE wxPaletteChangedEvent;
class WXDLLIMPEXP_FWD_CORE wxJoystickEvent;
class WXDLLIMPEXP_FWD_CORE wxDropFilesEvent;
class WXDLLIMPEXP_FWD_CORE wxInitDialogEvent;
class WXDLLIMPEXP_FWD_CORE wxUpdateUIEvent;
class WXDLLIMPEXP_FWD_CORE wxClipboardTextEvent;
class WXDLLIMPEXP_FWD_CORE wxHelpEvent;


    wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_BUTTON, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_CHECKBOX, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_CHOICE, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_LISTBOX, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_LISTBOX_DCLICK, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_CHECKLISTBOX, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MENU, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SLIDER, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_RADIOBOX, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_RADIOBUTTON, wxCommandEvent);

wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLLBAR, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_VLBOX, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_COMBOBOX, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_TOOL_RCLICKED, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_TOOL_DROPDOWN, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_TOOL_ENTER, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_COMBOBOX_DROPDOWN, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_COMBOBOX_CLOSEUP, wxCommandEvent);

    wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_BASE, wxEVT_THREAD, wxThreadEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_BASE, wxEVT_ASYNC_METHOD_CALL, wxAsyncMethodCallEvent);

    wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_LEFT_DOWN, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_LEFT_UP, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MIDDLE_DOWN, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MIDDLE_UP, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_RIGHT_DOWN, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_RIGHT_UP, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MOTION, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_ENTER_WINDOW, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_LEAVE_WINDOW, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_LEFT_DCLICK, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MIDDLE_DCLICK, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_RIGHT_DCLICK, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SET_FOCUS, wxFocusEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_KILL_FOCUS, wxFocusEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_CHILD_FOCUS, wxChildFocusEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MOUSEWHEEL, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_AUX1_DOWN, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_AUX1_UP, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_AUX1_DCLICK, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_AUX2_DOWN, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_AUX2_UP, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_AUX2_DCLICK, wxMouseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MAGNIFY, wxMouseEvent);

    wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_CHAR, wxKeyEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_CHAR_HOOK, wxKeyEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_NAVIGATION_KEY, wxNavigationKeyEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_KEY_DOWN, wxKeyEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_KEY_UP, wxKeyEvent);
#if wxUSE_HOTKEY
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_HOTKEY, wxKeyEvent);
#endif
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_AFTER_CHAR, wxKeyEvent);

    wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SET_CURSOR, wxSetCursorEvent);

    wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLL_TOP, wxScrollEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLL_BOTTOM, wxScrollEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLL_LINEUP, wxScrollEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLL_LINEDOWN, wxScrollEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLL_PAGEUP, wxScrollEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLL_PAGEDOWN, wxScrollEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLL_THUMBTRACK, wxScrollEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLL_THUMBRELEASE, wxScrollEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLL_CHANGED, wxScrollEvent);


#if wxUSE_SPINBTN

wxDECLARE_EXPORTED_EVENT_ALIAS( WXDLLIMPEXP_CORE, wxEVT_SPIN_UP,   wxSpinEvent );
wxDECLARE_EXPORTED_EVENT_ALIAS( WXDLLIMPEXP_CORE, wxEVT_SPIN_DOWN, wxSpinEvent );
wxDECLARE_EXPORTED_EVENT_ALIAS( WXDLLIMPEXP_CORE, wxEVT_SPIN,      wxSpinEvent );

#endif

    wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLLWIN_TOP, wxScrollWinEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLLWIN_BOTTOM, wxScrollWinEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLLWIN_LINEUP, wxScrollWinEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLLWIN_LINEDOWN, wxScrollWinEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLLWIN_PAGEUP, wxScrollWinEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLLWIN_PAGEDOWN, wxScrollWinEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLLWIN_THUMBTRACK, wxScrollWinEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SCROLLWIN_THUMBRELEASE, wxScrollWinEvent);

    wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SIZE, wxSizeEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MOVE, wxMoveEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_CLOSE_WINDOW, wxCloseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_END_SESSION, wxCloseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_QUERY_END_SESSION, wxCloseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_ACTIVATE_APP, wxActivateEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_ACTIVATE, wxActivateEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_CREATE, wxWindowCreateEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_DESTROY, wxWindowDestroyEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SHOW, wxShowEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_ICONIZE, wxIconizeEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MAXIMIZE, wxMaximizeEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MOUSE_CAPTURE_CHANGED, wxMouseCaptureChangedEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MOUSE_CAPTURE_LOST, wxMouseCaptureLostEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_PAINT, wxPaintEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_ERASE_BACKGROUND, wxEraseEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_NC_PAINT, wxNcPaintEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MENU_OPEN, wxMenuEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MENU_CLOSE, wxMenuEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MENU_HIGHLIGHT, wxMenuEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_CONTEXT_MENU, wxContextMenuEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SYS_COLOUR_CHANGED, wxSysColourChangedEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_DISPLAY_CHANGED, wxDisplayChangedEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_QUERY_NEW_PALETTE, wxQueryNewPaletteEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_PALETTE_CHANGED, wxPaletteChangedEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_JOY_BUTTON_DOWN, wxJoystickEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_JOY_BUTTON_UP, wxJoystickEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_JOY_MOVE, wxJoystickEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_JOY_ZMOVE, wxJoystickEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_DROP_FILES, wxDropFilesEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_INIT_DIALOG, wxInitDialogEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_BASE, wxEVT_IDLE, wxIdleEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_UPDATE_UI, wxUpdateUIEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_SIZING, wxSizeEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MOVING, wxMoveEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MOVE_START, wxMoveEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_MOVE_END, wxMoveEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_HIBERNATE, wxActivateEvent);

    wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_TEXT_COPY, wxClipboardTextEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_TEXT_CUT, wxClipboardTextEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_TEXT_PASTE, wxClipboardTextEvent);

        wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_COMMAND_LEFT_CLICK, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_COMMAND_LEFT_DCLICK, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_COMMAND_RIGHT_CLICK, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_COMMAND_RIGHT_DCLICK, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_COMMAND_SET_FOCUS, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_COMMAND_KILL_FOCUS, wxCommandEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_COMMAND_ENTER, wxCommandEvent);

    wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_HELP, wxHelpEvent);
wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_DETAILED_HELP, wxHelpEvent);

#define wxEVT_TOOL wxEVT_MENU


wxDECLARE_EXPORTED_EVENT(WXDLLIMPEXP_CORE, wxEVT_TEXT, wxCommandEvent);



enum wxEventPropagation
{
        wxEVENT_PROPAGATE_NONE = 0,

        wxEVENT_PROPAGATE_MAX = INT_MAX
};

enum wxEventCategory
{
                        wxEVT_CATEGORY_UI = 1,

                    wxEVT_CATEGORY_USER_INPUT = 2,

        wxEVT_CATEGORY_SOCKET = 4,

        wxEVT_CATEGORY_TIMER = 8,

                wxEVT_CATEGORY_THREAD = 16,


    
        wxEVT_CATEGORY_UNKNOWN = 32,

                wxEVT_CATEGORY_CLIPBOARD = 64,


    
            wxEVT_CATEGORY_NATIVE_EVENTS = wxEVT_CATEGORY_UI|wxEVT_CATEGORY_USER_INPUT,

        wxEVT_CATEGORY_ALL =
        wxEVT_CATEGORY_UI|wxEVT_CATEGORY_USER_INPUT|wxEVT_CATEGORY_SOCKET| \
        wxEVT_CATEGORY_TIMER|wxEVT_CATEGORY_THREAD|wxEVT_CATEGORY_UNKNOWN| \
        wxEVT_CATEGORY_CLIPBOARD
};



class WXDLLIMPEXP_BASE wxEvent : public wxObject
{
public:
    wxEvent(int winid = 0, wxEventType commandType = wxEVT_NULL );

    void SetEventType(wxEventType typ) { m_eventType = typ; }
    wxEventType GetEventType() const { return m_eventType; }

    wxObject *GetEventObject() const { return m_eventObject; }
    void SetEventObject(wxObject *obj) { m_eventObject = obj; }

    long GetTimestamp() const { return m_timeStamp; }
    void SetTimestamp(long ts = 0) { m_timeStamp = ts; }

    int GetId() const { return m_id; }
    void SetId(int Id) { m_id = Id; }

            wxObject *GetEventUserData() const { return m_callbackUserData; }

                    void Skip(bool skip = true) { m_skipped = skip; }
    bool GetSkipped() const { return m_skipped; }

                virtual wxEvent *Clone() const = 0;

                virtual wxEventCategory GetEventCategory() const
        { return wxEVT_CATEGORY_UI; }

            bool IsCommandEvent() const { return m_isCommandEvent; }

        bool ShouldPropagate() const
        { return m_propagationLevel != wxEVENT_PROPAGATE_NONE; }

            int StopPropagation()
    {
        int propagationLevel = m_propagationLevel;
        m_propagationLevel = wxEVENT_PROPAGATE_NONE;
        return propagationLevel;
    }

            void ResumePropagation(int propagationLevel)
    {
        m_propagationLevel = propagationLevel;
    }

            wxEvtHandler* GetPropagatedFrom() const { return m_propagatedFrom; }

                bool WasProcessed()
    {
        if ( m_wasProcessed )
            return true;

        m_wasProcessed = true;

        return false;
    }

            void SetWillBeProcessedAgain()
    {
        m_willBeProcessedAgain = true;
    }

    bool WillBeProcessedAgain()
    {
        if ( m_willBeProcessedAgain )
        {
            m_willBeProcessedAgain = false;
            return true;
        }

        return false;
    }

                bool ShouldProcessOnlyIn(wxEvtHandler *h) const
    {
        return h == m_handlerToProcessOnlyIn;
    }

                            void DidntHonourProcessOnlyIn()
    {
        m_handlerToProcessOnlyIn = NULL;
    }

protected:
    wxObject*         m_eventObject;
    wxEventType       m_eventType;
    long              m_timeStamp;
    int               m_id;

public:
        wxObject*         m_callbackUserData;

private:
        wxEvtHandler *m_handlerToProcessOnlyIn;

protected:
            int               m_propagationLevel;

            wxEvtHandler*     m_propagatedFrom;

    bool              m_skipped;
    bool              m_isCommandEvent;

                    bool m_wasProcessed;

                bool m_willBeProcessedAgain;

protected:
    wxEvent(const wxEvent&);                wxEvent& operator=(const wxEvent&); 
private:
        friend class WXDLLIMPEXP_FWD_BASE wxPropagateOnce;

        friend class WXDLLIMPEXP_FWD_BASE wxEventProcessInHandlerOnly;


    wxDECLARE_ABSTRACT_CLASS(wxEvent);
};


class WXDLLIMPEXP_BASE wxPropagationDisabler
{
public:
    wxPropagationDisabler(wxEvent& event) : m_event(event)
    {
        m_propagationLevelOld = m_event.StopPropagation();
    }

    ~wxPropagationDisabler()
    {
        m_event.ResumePropagation(m_propagationLevelOld);
    }

private:
    wxEvent& m_event;
    int m_propagationLevelOld;

    wxDECLARE_NO_COPY_CLASS(wxPropagationDisabler);
};


class WXDLLIMPEXP_BASE wxPropagateOnce
{
public:
                wxPropagateOnce(wxEvent& event, wxEvtHandler* handler = NULL)
        : m_event(event),
          m_propagatedFromOld(event.m_propagatedFrom)
    {
        wxASSERT_MSG( m_event.m_propagationLevel > 0,
                        wxT("shouldn't be used unless ShouldPropagate()!") );

        m_event.m_propagationLevel--;
        m_event.m_propagatedFrom = handler;
    }

    ~wxPropagateOnce()
    {
        m_event.m_propagatedFrom = m_propagatedFromOld;
        m_event.m_propagationLevel++;
    }

private:
    wxEvent& m_event;
    wxEvtHandler* const m_propagatedFromOld;

    wxDECLARE_NO_COPY_CLASS(wxPropagateOnce);
};

class wxEventProcessInHandlerOnly
{
public:
    wxEventProcessInHandlerOnly(wxEvent& event, wxEvtHandler *handler)
        : m_event(event),
          m_handlerToProcessOnlyInOld(event.m_handlerToProcessOnlyIn)
    {
        m_event.m_handlerToProcessOnlyIn = handler;
    }

    ~wxEventProcessInHandlerOnly()
    {
        m_event.m_handlerToProcessOnlyIn = m_handlerToProcessOnlyInOld;
    }

private:
    wxEvent& m_event;
    wxEvtHandler * const m_handlerToProcessOnlyInOld;

    wxDECLARE_NO_COPY_CLASS(wxEventProcessInHandlerOnly);
};


class WXDLLIMPEXP_BASE wxEventBasicPayloadMixin
{
public:
    wxEventBasicPayloadMixin()
        : m_commandInt(0),
          m_extraLong(0)
    {
    }

    void SetString(const wxString& s) { m_cmdString = s; }
    const wxString& GetString() const { return m_cmdString; }

    void SetInt(int i) { m_commandInt = i; }
    int GetInt() const { return m_commandInt; }

    void SetExtraLong(long extraLong) { m_extraLong = extraLong; }
    long GetExtraLong() const { return m_extraLong; }

protected:
            wxString          m_cmdString;         int               m_commandInt;
    long              m_extraLong;     
    wxDECLARE_NO_ASSIGN_CLASS(wxEventBasicPayloadMixin);
};

class WXDLLIMPEXP_BASE wxEventAnyPayloadMixin : public wxEventBasicPayloadMixin
{
public:
    wxEventAnyPayloadMixin() : wxEventBasicPayloadMixin() {}

#if wxUSE_ANY
    template<typename T>
    void SetPayload(const T& payload)
    {
        m_payload = payload;
    }

    template<typename T>
    T GetPayload() const
    {
        return m_payload.As<T>();
    }

protected:
    wxAny m_payload;
#endif 
    wxDECLARE_NO_ASSIGN_CLASS(wxEventBasicPayloadMixin);
};





enum wxIdleMode
{
            wxIDLE_PROCESS_ALL,

                    wxIDLE_PROCESS_SPECIFIED
};

class WXDLLIMPEXP_BASE wxIdleEvent : public wxEvent
{
public:
    wxIdleEvent()
        : wxEvent(0, wxEVT_IDLE),
          m_requestMore(false)
        { }
    wxIdleEvent(const wxIdleEvent& event)
        : wxEvent(event),
          m_requestMore(event.m_requestMore)
    { }

    void RequestMore(bool needMore = true) { m_requestMore = needMore; }
    bool MoreRequested() const { return m_requestMore; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxIdleEvent(*this); }

                static void SetMode(wxIdleMode mode) { sm_idleMode = mode; }

        static wxIdleMode GetMode() { return sm_idleMode; }

protected:
    bool m_requestMore;
    static wxIdleMode sm_idleMode;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxIdleEvent);
};



class WXDLLIMPEXP_BASE wxThreadEvent : public wxEvent,
                                       public wxEventAnyPayloadMixin
{
public:
    wxThreadEvent(wxEventType eventType = wxEVT_THREAD, int id = wxID_ANY)
        : wxEvent(id, eventType)
        { }

    wxThreadEvent(const wxThreadEvent& event)
        : wxEvent(event),
          wxEventAnyPayloadMixin(event)
    {
                        SetString(GetString().Clone());
    }

    virtual wxEvent *Clone() const wxOVERRIDE
    {
        return new wxThreadEvent(*this);
    }

            virtual wxEventCategory GetEventCategory() const wxOVERRIDE
        { return wxEVT_CATEGORY_THREAD; }

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxThreadEvent);
};



#ifdef wxHAS_CALL_AFTER

class wxAsyncMethodCallEvent : public wxEvent
{
public:
    wxAsyncMethodCallEvent(wxObject* object)
        : wxEvent(wxID_ANY, wxEVT_ASYNC_METHOD_CALL)
    {
        SetEventObject(object);
    }

    wxAsyncMethodCallEvent(const wxAsyncMethodCallEvent& other)
        : wxEvent(other)
    {
    }

    virtual void Execute() = 0;
};

template <typename T>
class wxAsyncMethodCallEvent0 : public wxAsyncMethodCallEvent
{
public:
    typedef T ObjectType;
    typedef void (ObjectType::*MethodType)();

    wxAsyncMethodCallEvent0(ObjectType* object,
                            MethodType method)
        : wxAsyncMethodCallEvent(object),
          m_object(object),
          m_method(method)
    {
    }

    wxAsyncMethodCallEvent0(const wxAsyncMethodCallEvent0& other)
        : wxAsyncMethodCallEvent(other),
          m_object(other.m_object),
          m_method(other.m_method)
    {
    }

    virtual wxEvent *Clone() const wxOVERRIDE
    {
        return new wxAsyncMethodCallEvent0(*this);
    }

    virtual void Execute() wxOVERRIDE
    {
        (m_object->*m_method)();
    }

private:
    ObjectType* const m_object;
    const MethodType m_method;
};

template <typename T, typename T1>
class wxAsyncMethodCallEvent1 : public wxAsyncMethodCallEvent
{
public:
    typedef T ObjectType;
    typedef void (ObjectType::*MethodType)(T1 x1);
    typedef typename wxRemoveRef<T1>::type ParamType1;

    wxAsyncMethodCallEvent1(ObjectType* object,
                            MethodType method,
                            const ParamType1& x1)
        : wxAsyncMethodCallEvent(object),
          m_object(object),
          m_method(method),
          m_param1(x1)
    {
    }

    wxAsyncMethodCallEvent1(const wxAsyncMethodCallEvent1& other)
        : wxAsyncMethodCallEvent(other),
          m_object(other.m_object),
          m_method(other.m_method),
          m_param1(other.m_param1)
    {
    }

    virtual wxEvent *Clone() const wxOVERRIDE
    {
        return new wxAsyncMethodCallEvent1(*this);
    }

    virtual void Execute() wxOVERRIDE
    {
        (m_object->*m_method)(m_param1);
    }

private:
    ObjectType* const m_object;
    const MethodType m_method;
    const ParamType1 m_param1;
};

template <typename T, typename T1, typename T2>
class wxAsyncMethodCallEvent2 : public wxAsyncMethodCallEvent
{
public:
    typedef T ObjectType;
    typedef void (ObjectType::*MethodType)(T1 x1, T2 x2);
    typedef typename wxRemoveRef<T1>::type ParamType1;
    typedef typename wxRemoveRef<T2>::type ParamType2;

    wxAsyncMethodCallEvent2(ObjectType* object,
                            MethodType method,
                            const ParamType1& x1,
                            const ParamType2& x2)
        : wxAsyncMethodCallEvent(object),
          m_object(object),
          m_method(method),
          m_param1(x1),
          m_param2(x2)
    {
    }

    wxAsyncMethodCallEvent2(const wxAsyncMethodCallEvent2& other)
        : wxAsyncMethodCallEvent(other),
          m_object(other.m_object),
          m_method(other.m_method),
          m_param1(other.m_param1),
          m_param2(other.m_param2)
    {
    }

    virtual wxEvent *Clone() const wxOVERRIDE
    {
        return new wxAsyncMethodCallEvent2(*this);
    }

    virtual void Execute() wxOVERRIDE
    {
        (m_object->*m_method)(m_param1, m_param2);
    }

private:
    ObjectType* const m_object;
    const MethodType m_method;
    const ParamType1 m_param1;
    const ParamType2 m_param2;
};

template <typename T>
class wxAsyncMethodCallEventFunctor : public wxAsyncMethodCallEvent
{
public:
    typedef T FunctorType;

    wxAsyncMethodCallEventFunctor(wxObject *object, const FunctorType& fn)
        : wxAsyncMethodCallEvent(object),
          m_fn(fn)
    {
    }

    wxAsyncMethodCallEventFunctor(const wxAsyncMethodCallEventFunctor& other)
        : wxAsyncMethodCallEvent(other),
          m_fn(other.m_fn)
    {
    }

    virtual wxEvent *Clone() const wxOVERRIDE
    {
        return new wxAsyncMethodCallEventFunctor(*this);
    }

    virtual void Execute() wxOVERRIDE
    {
        m_fn();
    }

private:
    FunctorType m_fn;
};

#endif 

#if wxUSE_GUI




class WXDLLIMPEXP_CORE wxCommandEvent : public wxEvent,
                                        public wxEventBasicPayloadMixin
{
public:
    wxCommandEvent(wxEventType commandType = wxEVT_NULL, int winid = 0)
        : wxEvent(winid, commandType)
    {
        m_clientData = NULL;
        m_clientObject = NULL;
        m_isCommandEvent = true;

                m_propagationLevel = wxEVENT_PROPAGATE_MAX;
    }

    wxCommandEvent(const wxCommandEvent& event)
        : wxEvent(event),
          wxEventBasicPayloadMixin(event),
          m_clientData(event.m_clientData),
          m_clientObject(event.m_clientObject)
    {
                        if ( m_cmdString.empty() )
            m_cmdString = event.GetString();
    }

        void SetClientData(void* clientData) { m_clientData = clientData; }
    void *GetClientData() const { return m_clientData; }

        void SetClientObject(wxClientData* clientObject) { m_clientObject = clientObject; }
    wxClientData *GetClientObject() const { return m_clientObject; }

            wxString GetString() const;

        int GetSelection() const { return m_commandInt; }

        bool IsChecked() const { return m_commandInt != 0; }

        bool IsSelection() const { return (m_extraLong != 0); }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxCommandEvent(*this); }
    virtual wxEventCategory GetEventCategory() const wxOVERRIDE { return wxEVT_CATEGORY_USER_INPUT; }

protected:
    void*             m_clientData;        wxClientData*     m_clientObject;  
private:

    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxCommandEvent);
};

class WXDLLIMPEXP_CORE wxNotifyEvent  : public wxCommandEvent
{
public:
    wxNotifyEvent(wxEventType commandType = wxEVT_NULL, int winid = 0)
        : wxCommandEvent(commandType, winid)
        { m_bAllow = true; }

    wxNotifyEvent(const wxNotifyEvent& event)
        : wxCommandEvent(event)
        { m_bAllow = event.m_bAllow; }

        void Veto() { m_bAllow = false; }

        void Allow() { m_bAllow = true; }

        bool IsAllowed() const { return m_bAllow; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxNotifyEvent(*this); }

private:
    bool m_bAllow;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxNotifyEvent);
};




class WXDLLIMPEXP_CORE wxScrollEvent : public wxCommandEvent
{
public:
    wxScrollEvent(wxEventType commandType = wxEVT_NULL,
                  int winid = 0, int pos = 0, int orient = 0);

    int GetOrientation() const { return (int) m_extraLong; }
    int GetPosition() const { return m_commandInt; }
    void SetOrientation(int orient) { m_extraLong = (long) orient; }
    void SetPosition(int pos) { m_commandInt = pos; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxScrollEvent(*this); }

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxScrollEvent);
};



class WXDLLIMPEXP_CORE wxScrollWinEvent : public wxEvent
{
public:
    wxScrollWinEvent(wxEventType commandType = wxEVT_NULL,
                     int pos = 0, int orient = 0);
    wxScrollWinEvent(const wxScrollWinEvent& event) : wxEvent(event)
        {    m_commandInt = event.m_commandInt;
            m_extraLong = event.m_extraLong;    }

    int GetOrientation() const { return (int) m_extraLong; }
    int GetPosition() const { return m_commandInt; }
    void SetOrientation(int orient) { m_extraLong = (long) orient; }
    void SetPosition(int pos) { m_commandInt = pos; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxScrollWinEvent(*this); }

protected:
    int               m_commandInt;
    long              m_extraLong;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxScrollWinEvent);
};






enum wxMouseWheelAxis
{
    wxMOUSE_WHEEL_VERTICAL,
    wxMOUSE_WHEEL_HORIZONTAL
};

class WXDLLIMPEXP_CORE wxMouseEvent : public wxEvent,
                                      public wxMouseState
{
public:
    wxMouseEvent(wxEventType mouseType = wxEVT_NULL);
    wxMouseEvent(const wxMouseEvent& event)
        : wxEvent(event),
          wxMouseState(event)
    {
        Assign(event);
    }

        bool IsButton() const { return Button(wxMOUSE_BTN_ANY); }

        bool ButtonDown(int but = wxMOUSE_BTN_ANY) const;

        bool ButtonDClick(int but = wxMOUSE_BTN_ANY) const;

        bool ButtonUp(int but = wxMOUSE_BTN_ANY) const;

        bool Button(int but) const;

        int GetButton() const;

        bool LeftDown() const { return (m_eventType == wxEVT_LEFT_DOWN); }
    bool MiddleDown() const { return (m_eventType == wxEVT_MIDDLE_DOWN); }
    bool RightDown() const { return (m_eventType == wxEVT_RIGHT_DOWN); }
    bool Aux1Down() const { return (m_eventType == wxEVT_AUX1_DOWN); }
    bool Aux2Down() const { return (m_eventType == wxEVT_AUX2_DOWN); }

    bool LeftUp() const { return (m_eventType == wxEVT_LEFT_UP); }
    bool MiddleUp() const { return (m_eventType == wxEVT_MIDDLE_UP); }
    bool RightUp() const { return (m_eventType == wxEVT_RIGHT_UP); }
    bool Aux1Up() const { return (m_eventType == wxEVT_AUX1_UP); }
    bool Aux2Up() const { return (m_eventType == wxEVT_AUX2_UP); }

    bool LeftDClick() const { return (m_eventType == wxEVT_LEFT_DCLICK); }
    bool MiddleDClick() const { return (m_eventType == wxEVT_MIDDLE_DCLICK); }
    bool RightDClick() const { return (m_eventType == wxEVT_RIGHT_DCLICK); }
    bool Aux1DClick() const { return (m_eventType == wxEVT_AUX1_DCLICK); }
    bool Aux2DClick() const { return (m_eventType == wxEVT_AUX2_DCLICK); }

    bool Magnify() const { return (m_eventType == wxEVT_MAGNIFY); }

        bool Dragging() const
    {
        return (m_eventType == wxEVT_MOTION) && ButtonIsDown(wxMOUSE_BTN_ANY);
    }

        bool Moving() const
    {
        return (m_eventType == wxEVT_MOTION) && !ButtonIsDown(wxMOUSE_BTN_ANY);
    }

        bool Entering() const { return (m_eventType == wxEVT_ENTER_WINDOW); }

        bool Leaving() const { return (m_eventType == wxEVT_LEAVE_WINDOW); }

        int GetClickCount() const { return m_clickCount; }

        wxPoint GetLogicalPosition(const wxDC& dc) const;

                                int GetWheelRotation() const { return m_wheelRotation; }

                int GetWheelDelta() const { return m_wheelDelta; }

                wxMouseWheelAxis GetWheelAxis() const { return m_wheelAxis; }

            int GetLinesPerAction() const { return m_linesPerAction; }

            int GetColumnsPerAction() const { return m_columnsPerAction; }

        bool IsPageScroll() const { return ((unsigned int)m_linesPerAction == UINT_MAX); }

    float GetMagnification() const { return m_magnification; }
    virtual wxEvent *Clone() const wxOVERRIDE { return new wxMouseEvent(*this); }
    virtual wxEventCategory GetEventCategory() const wxOVERRIDE { return wxEVT_CATEGORY_USER_INPUT; }

    wxMouseEvent& operator=(const wxMouseEvent& event)
    {
        if (&event != this)
            Assign(event);
        return *this;
    }

public:
    int           m_clickCount;

    wxMouseWheelAxis m_wheelAxis;
    int           m_wheelRotation;
    int           m_wheelDelta;
    int           m_linesPerAction;
    int           m_columnsPerAction;
    float         m_magnification;

protected:
    void Assign(const wxMouseEvent& evt);

private:
    wxDECLARE_DYNAMIC_CLASS(wxMouseEvent);
};




class WXDLLIMPEXP_CORE wxSetCursorEvent : public wxEvent
{
public:
    wxSetCursorEvent(wxCoord x = 0, wxCoord y = 0)
        : wxEvent(0, wxEVT_SET_CURSOR),
          m_x(x), m_y(y), m_cursor()
        { }

    wxSetCursorEvent(const wxSetCursorEvent& event)
        : wxEvent(event),
          m_x(event.m_x),
          m_y(event.m_y),
          m_cursor(event.m_cursor)
        { }

    wxCoord GetX() const { return m_x; }
    wxCoord GetY() const { return m_y; }

    void SetCursor(const wxCursor& cursor) { m_cursor = cursor; }
    const wxCursor& GetCursor() const { return m_cursor; }
    bool HasCursor() const { return m_cursor.IsOk(); }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxSetCursorEvent(*this); }

private:
    wxCoord  m_x, m_y;
    wxCursor m_cursor;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxSetCursorEvent);
};




enum wxKeyCategoryFlags
{
        WXK_CATEGORY_ARROW  = 1,

        WXK_CATEGORY_PAGING = 2,

        WXK_CATEGORY_JUMP   = 4,

        WXK_CATEGORY_TAB    = 8,

        WXK_CATEGORY_CUT    = 16,

        WXK_CATEGORY_NAVIGATION = WXK_CATEGORY_ARROW |
                              WXK_CATEGORY_PAGING |
                              WXK_CATEGORY_JUMP
};

class WXDLLIMPEXP_CORE wxKeyEvent : public wxEvent,
                                    public wxKeyboardState
{
public:
    wxKeyEvent(wxEventType keyType = wxEVT_NULL);

                wxKeyEvent(const wxKeyEvent& evt);
    wxKeyEvent(wxEventType eventType, const wxKeyEvent& evt);

        int GetKeyCode() const { return (int)m_keyCode; }

        bool IsKeyInCategory(int category) const;

#if wxUSE_UNICODE
        wxChar GetUnicodeKey() const { return m_uniChar; }
#endif 
        wxUint32 GetRawKeyCode() const { return m_rawCode; }

        wxUint32 GetRawKeyFlags() const { return m_rawFlags; }

        void GetPosition(wxCoord *xpos, wxCoord *ypos) const
    {
        if (xpos)
            *xpos = GetX();
        if (ypos)
            *ypos = GetY();
    }

        void GetPosition(long *xpos, long *ypos) const
    {
        if (xpos)
            *xpos = GetX();
        if (ypos)
            *ypos = GetY();
    }

    wxPoint GetPosition() const
        { return wxPoint(GetX(), GetY()); }

        wxCoord GetX() const;

        wxCoord GetY() const;

                void DoAllowNextEvent() { m_allowNext = true; }

        bool IsNextEventAllowed() const { return m_allowNext; }


    virtual wxEvent *Clone() const wxOVERRIDE { return new wxKeyEvent(*this); }
    virtual wxEventCategory GetEventCategory() const wxOVERRIDE { return wxEVT_CATEGORY_USER_INPUT; }

            wxKeyEvent& operator=(const wxKeyEvent& evt)
    {
        if ( &evt != this )
        {
            wxEvent::operator=(evt);

                                    *static_cast<wxKeyboardState *>(this) = evt;

            DoAssignMembers(evt);
        }
        return *this;
    }

public:
            wxCoord       m_x, m_y;

    long          m_keyCode;

#if wxUSE_UNICODE
            wxChar        m_uniChar;
#endif

            wxUint32      m_rawCode;
    wxUint32      m_rawFlags;

private:
            void InitPropagation()
    {
        if ( m_eventType == wxEVT_CHAR_HOOK )
            m_propagationLevel = wxEVENT_PROPAGATE_MAX;

        m_allowNext = false;
    }

            void DoAssignMembers(const wxKeyEvent& evt)
    {
        m_x = evt.m_x;
        m_y = evt.m_y;
        m_hasPosition = evt.m_hasPosition;

        m_keyCode = evt.m_keyCode;

        m_rawCode = evt.m_rawCode;
        m_rawFlags = evt.m_rawFlags;
#if wxUSE_UNICODE
        m_uniChar = evt.m_uniChar;
#endif
    }

            void InitPositionIfNecessary() const;

                bool m_allowNext;

            bool m_hasPosition;

    wxDECLARE_DYNAMIC_CLASS(wxKeyEvent);
};



class WXDLLIMPEXP_CORE wxSizeEvent : public wxEvent
{
public:
    wxSizeEvent() : wxEvent(0, wxEVT_SIZE)
        { }
    wxSizeEvent(const wxSize& sz, int winid = 0)
        : wxEvent(winid, wxEVT_SIZE),
          m_size(sz)
        { }
    wxSizeEvent(const wxSizeEvent& event)
        : wxEvent(event),
          m_size(event.m_size), m_rect(event.m_rect)
        { }
    wxSizeEvent(const wxRect& rect, int id = 0)
        : m_size(rect.GetSize()), m_rect(rect)
        { m_eventType = wxEVT_SIZING; m_id = id; }

    wxSize GetSize() const { return m_size; }
    void SetSize(wxSize size) { m_size = size; }
    wxRect GetRect() const { return m_rect; }
    void SetRect(const wxRect& rect) { m_rect = rect; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxSizeEvent(*this); }

public:
        wxSize m_size;
    wxRect m_rect; 
private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxSizeEvent);
};




class WXDLLIMPEXP_CORE wxMoveEvent : public wxEvent
{
public:
    wxMoveEvent()
        : wxEvent(0, wxEVT_MOVE)
        { }
    wxMoveEvent(const wxPoint& pos, int winid = 0)
        : wxEvent(winid, wxEVT_MOVE),
          m_pos(pos)
        { }
    wxMoveEvent(const wxMoveEvent& event)
        : wxEvent(event),
          m_pos(event.m_pos)
    { }
    wxMoveEvent(const wxRect& rect, int id = 0)
        : m_pos(rect.GetPosition()), m_rect(rect)
        { m_eventType = wxEVT_MOVING; m_id = id; }

    wxPoint GetPosition() const { return m_pos; }
    void SetPosition(const wxPoint& pos) { m_pos = pos; }
    wxRect GetRect() const { return m_rect; }
    void SetRect(const wxRect& rect) { m_rect = rect; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxMoveEvent(*this); }

protected:
    wxPoint m_pos;
    wxRect m_rect;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxMoveEvent);
};



#if wxDEBUG_LEVEL && defined(__WXMSW__)
    #define wxHAS_PAINT_DEBUG

        extern WXDLLIMPEXP_CORE int g_isPainting;
#endif 
class WXDLLIMPEXP_CORE wxPaintEvent : public wxEvent
{
public:
    wxPaintEvent(int Id = 0)
        : wxEvent(Id, wxEVT_PAINT)
    {
#ifdef wxHAS_PAINT_DEBUG
                g_isPainting++;
#endif     }

        #ifdef wxHAS_PAINT_DEBUG
    wxPaintEvent(const wxPaintEvent& event)
            : wxEvent(event)
    {
        g_isPainting++;
    }

    virtual ~wxPaintEvent()
    {
        g_isPainting--;
    }
#endif 
    virtual wxEvent *Clone() const wxOVERRIDE { return new wxPaintEvent(*this); }

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxPaintEvent);
};

class WXDLLIMPEXP_CORE wxNcPaintEvent : public wxEvent
{
public:
    wxNcPaintEvent(int winid = 0)
        : wxEvent(winid, wxEVT_NC_PAINT)
        { }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxNcPaintEvent(*this); }

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxNcPaintEvent);
};



class WXDLLIMPEXP_CORE wxEraseEvent : public wxEvent
{
public:
    wxEraseEvent(int Id = 0, wxDC *dc = NULL)
        : wxEvent(Id, wxEVT_ERASE_BACKGROUND),
          m_dc(dc)
        { }

    wxEraseEvent(const wxEraseEvent& event)
        : wxEvent(event),
          m_dc(event.m_dc)
        { }

    wxDC *GetDC() const { return m_dc; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxEraseEvent(*this); }

protected:
    wxDC *m_dc;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxEraseEvent);
};



class WXDLLIMPEXP_CORE wxFocusEvent : public wxEvent
{
public:
    wxFocusEvent(wxEventType type = wxEVT_NULL, int winid = 0)
        : wxEvent(winid, type)
        { m_win = NULL; }

    wxFocusEvent(const wxFocusEvent& event)
        : wxEvent(event)
        { m_win = event.m_win; }

                wxWindow *GetWindow() const { return m_win; }
    void SetWindow(wxWindow *win) { m_win = win; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxFocusEvent(*this); }

private:
    wxWindow *m_win;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxFocusEvent);
};

class WXDLLIMPEXP_CORE wxChildFocusEvent : public wxCommandEvent
{
public:
    wxChildFocusEvent(wxWindow *win = NULL);

    wxWindow *GetWindow() const { return (wxWindow *)GetEventObject(); }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxChildFocusEvent(*this); }

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxChildFocusEvent);
};



class WXDLLIMPEXP_CORE wxActivateEvent : public wxEvent
{
public:
            enum Reason
    {
        Reason_Mouse,
        Reason_Unknown
    };

    wxActivateEvent(wxEventType type = wxEVT_NULL, bool active = true,
                    int Id = 0, Reason activationReason = Reason_Unknown)
        : wxEvent(Id, type),
        m_activationReason(activationReason)
    {
        m_active = active;
    }
    wxActivateEvent(const wxActivateEvent& event)
        : wxEvent(event)
    {
        m_active = event.m_active;
        m_activationReason = event.m_activationReason;
    }

    bool GetActive() const { return m_active; }
    Reason GetActivationReason() const { return m_activationReason;}

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxActivateEvent(*this); }

private:
    bool m_active;
    Reason m_activationReason;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxActivateEvent);
};



class WXDLLIMPEXP_CORE wxInitDialogEvent : public wxEvent
{
public:
    wxInitDialogEvent(int Id = 0)
        : wxEvent(Id, wxEVT_INIT_DIALOG)
        { }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxInitDialogEvent(*this); }

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxInitDialogEvent);
};



class WXDLLIMPEXP_CORE wxMenuEvent : public wxEvent
{
public:
    wxMenuEvent(wxEventType type = wxEVT_NULL, int winid = 0, wxMenu* menu = NULL)
        : wxEvent(winid, type)
        { m_menuId = winid; m_menu = menu; }
    wxMenuEvent(const wxMenuEvent& event)
        : wxEvent(event)
    { m_menuId = event.m_menuId; m_menu = event.m_menu; }

        int GetMenuId() const { return m_menuId; }

        bool IsPopup() const { return m_menuId == wxID_ANY; }

        wxMenu* GetMenu() const { return m_menu; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxMenuEvent(*this); }

private:
    int     m_menuId;
    wxMenu* m_menu;

    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxMenuEvent);
};



class WXDLLIMPEXP_CORE wxCloseEvent : public wxEvent
{
public:
    wxCloseEvent(wxEventType type = wxEVT_NULL, int winid = 0)
        : wxEvent(winid, type),
          m_loggingOff(true),
          m_veto(false),                m_canVeto(true) {}

    wxCloseEvent(const wxCloseEvent& event)
        : wxEvent(event),
        m_loggingOff(event.m_loggingOff),
        m_veto(event.m_veto),
        m_canVeto(event.m_canVeto) {}

    void SetLoggingOff(bool logOff) { m_loggingOff = logOff; }
    bool GetLoggingOff() const
    {
                        wxASSERT_MSG( m_eventType != wxEVT_CLOSE_WINDOW,
                      wxT("this flag is for end session events only") );

        return m_loggingOff;
    }

    void Veto(bool veto = true)
    {
                wxCHECK_RET( m_canVeto,
                     wxT("call to Veto() ignored (can't veto this event)") );

        m_veto = veto;
    }
    void SetCanVeto(bool canVeto) { m_canVeto = canVeto; }
    bool CanVeto() const { return m_canVeto; }
    bool GetVeto() const { return m_canVeto && m_veto; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxCloseEvent(*this); }

protected:
    bool m_loggingOff,
         m_veto,
         m_canVeto;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxCloseEvent);
};



class WXDLLIMPEXP_CORE wxShowEvent : public wxEvent
{
public:
    wxShowEvent(int winid = 0, bool show = false)
        : wxEvent(winid, wxEVT_SHOW)
        { m_show = show; }
    wxShowEvent(const wxShowEvent& event)
        : wxEvent(event)
    { m_show = event.m_show; }

    void SetShow(bool show) { m_show = show; }

        bool IsShown() const { return m_show; }

#if WXWIN_COMPATIBILITY_2_8
    wxDEPRECATED( bool GetShow() const { return IsShown(); } )
#endif

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxShowEvent(*this); }

protected:
    bool m_show;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxShowEvent);
};



class WXDLLIMPEXP_CORE wxIconizeEvent : public wxEvent
{
public:
    wxIconizeEvent(int winid = 0, bool iconized = true)
        : wxEvent(winid, wxEVT_ICONIZE)
        { m_iconized = iconized; }
    wxIconizeEvent(const wxIconizeEvent& event)
        : wxEvent(event)
    { m_iconized = event.m_iconized; }

#if WXWIN_COMPATIBILITY_2_8
    wxDEPRECATED( bool Iconized() const { return IsIconized(); } )
#endif
        bool IsIconized() const { return m_iconized; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxIconizeEvent(*this); }

protected:
    bool m_iconized;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxIconizeEvent);
};


class WXDLLIMPEXP_CORE wxMaximizeEvent : public wxEvent
{
public:
    wxMaximizeEvent(int winid = 0)
        : wxEvent(winid, wxEVT_MAXIMIZE)
        { }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxMaximizeEvent(*this); }

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxMaximizeEvent);
};



enum
{
    wxJOYSTICK1,
    wxJOYSTICK2
};

enum
{
    wxJOY_BUTTON_ANY = -1,
    wxJOY_BUTTON1    = 1,
    wxJOY_BUTTON2    = 2,
    wxJOY_BUTTON3    = 4,
    wxJOY_BUTTON4    = 8
};

class WXDLLIMPEXP_CORE wxJoystickEvent : public wxEvent
{
protected:
    wxPoint   m_pos;
    int       m_zPosition;
    int       m_buttonChange;       int       m_buttonState;        int       m_joyStick;       
public:
    wxJoystickEvent(wxEventType type = wxEVT_NULL,
                    int state = 0,
                    int joystick = wxJOYSTICK1,
                    int change = 0)
        : wxEvent(0, type),
          m_pos(),
          m_zPosition(0),
          m_buttonChange(change),
          m_buttonState(state),
          m_joyStick(joystick)
    {
    }
    wxJoystickEvent(const wxJoystickEvent& event)
        : wxEvent(event),
          m_pos(event.m_pos),
          m_zPosition(event.m_zPosition),
          m_buttonChange(event.m_buttonChange),
          m_buttonState(event.m_buttonState),
          m_joyStick(event.m_joyStick)
    { }

    wxPoint GetPosition() const { return m_pos; }
    int GetZPosition() const { return m_zPosition; }
    int GetButtonState() const { return m_buttonState; }
    int GetButtonChange() const { return m_buttonChange; }
    int GetJoystick() const { return m_joyStick; }

    void SetJoystick(int stick) { m_joyStick = stick; }
    void SetButtonState(int state) { m_buttonState = state; }
    void SetButtonChange(int change) { m_buttonChange = change; }
    void SetPosition(const wxPoint& pos) { m_pos = pos; }
    void SetZPosition(int zPos) { m_zPosition = zPos; }

        bool IsButton() const { return ((GetEventType() == wxEVT_JOY_BUTTON_DOWN) ||
            (GetEventType() == wxEVT_JOY_BUTTON_UP)); }

        bool IsMove() const { return (GetEventType() == wxEVT_JOY_MOVE); }

        bool IsZMove() const { return (GetEventType() == wxEVT_JOY_ZMOVE); }

        bool ButtonDown(int but = wxJOY_BUTTON_ANY) const
    { return ((GetEventType() == wxEVT_JOY_BUTTON_DOWN) &&
            ((but == wxJOY_BUTTON_ANY) || (but == m_buttonChange))); }

        bool ButtonUp(int but = wxJOY_BUTTON_ANY) const
    { return ((GetEventType() == wxEVT_JOY_BUTTON_UP) &&
            ((but == wxJOY_BUTTON_ANY) || (but == m_buttonChange))); }

        bool ButtonIsDown(int but =  wxJOY_BUTTON_ANY) const
    { return (((but == wxJOY_BUTTON_ANY) && (m_buttonState != 0)) ||
            ((m_buttonState & but) == but)); }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxJoystickEvent(*this); }

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxJoystickEvent);
};



class WXDLLIMPEXP_CORE wxDropFilesEvent : public wxEvent
{
public:
    int       m_noFiles;
    wxPoint   m_pos;
    wxString* m_files;

    wxDropFilesEvent(wxEventType type = wxEVT_NULL,
                     int noFiles = 0,
                     wxString *files = NULL)
        : wxEvent(0, type),
          m_noFiles(noFiles),
          m_pos(),
          m_files(files)
        { }

        wxDropFilesEvent(const wxDropFilesEvent& other)
        : wxEvent(other),
          m_noFiles(other.m_noFiles),
          m_pos(other.m_pos),
          m_files(NULL)
    {
        m_files = new wxString[m_noFiles];
        for ( int n = 0; n < m_noFiles; n++ )
        {
            m_files[n] = other.m_files[n];
        }
    }

    virtual ~wxDropFilesEvent()
    {
        delete [] m_files;
    }

    wxPoint GetPosition() const { return m_pos; }
    int GetNumberOfFiles() const { return m_noFiles; }
    wxString *GetFiles() const { return m_files; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxDropFilesEvent(*this); }

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxDropFilesEvent);
};




enum wxUpdateUIMode
{
            wxUPDATE_UI_PROCESS_ALL,

                    wxUPDATE_UI_PROCESS_SPECIFIED
};

class WXDLLIMPEXP_CORE wxUpdateUIEvent : public wxCommandEvent
{
public:
    wxUpdateUIEvent(wxWindowID commandId = 0)
        : wxCommandEvent(wxEVT_UPDATE_UI, commandId)
    {
        m_checked =
        m_enabled =
        m_shown =
        m_setEnabled =
        m_setShown =
        m_setText =
        m_setChecked = false;
    }
    wxUpdateUIEvent(const wxUpdateUIEvent& event)
        : wxCommandEvent(event),
          m_checked(event.m_checked),
          m_enabled(event.m_enabled),
          m_shown(event.m_shown),
          m_setEnabled(event.m_setEnabled),
          m_setShown(event.m_setShown),
          m_setText(event.m_setText),
          m_setChecked(event.m_setChecked),
          m_text(event.m_text)
    { }

    bool GetChecked() const { return m_checked; }
    bool GetEnabled() const { return m_enabled; }
    bool GetShown() const { return m_shown; }
    wxString GetText() const { return m_text; }
    bool GetSetText() const { return m_setText; }
    bool GetSetChecked() const { return m_setChecked; }
    bool GetSetEnabled() const { return m_setEnabled; }
    bool GetSetShown() const { return m_setShown; }

    void Check(bool check) { m_checked = check; m_setChecked = true; }
    void Enable(bool enable) { m_enabled = enable; m_setEnabled = true; }
    void Show(bool show) { m_shown = show; m_setShown = true; }
    void SetText(const wxString& text) { m_text = text; m_setText = true; }

            static void SetUpdateInterval(long updateInterval) { sm_updateInterval = updateInterval; }

        static long GetUpdateInterval() { return sm_updateInterval; }

        static bool CanUpdate(wxWindowBase *win);

            static void ResetUpdateTime();

                static void SetMode(wxUpdateUIMode mode) { sm_updateMode = mode; }

        static wxUpdateUIMode GetMode() { return sm_updateMode; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxUpdateUIEvent(*this); }

protected:
    bool          m_checked;
    bool          m_enabled;
    bool          m_shown;
    bool          m_setEnabled;
    bool          m_setShown;
    bool          m_setText;
    bool          m_setChecked;
    wxString      m_text;
#if wxUSE_LONGLONG
    static wxLongLong       sm_lastUpdate;
#endif
    static long             sm_updateInterval;
    static wxUpdateUIMode   sm_updateMode;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxUpdateUIEvent);
};



class WXDLLIMPEXP_CORE wxSysColourChangedEvent : public wxEvent
{
public:
    wxSysColourChangedEvent()
        : wxEvent(0, wxEVT_SYS_COLOUR_CHANGED)
        { }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxSysColourChangedEvent(*this); }

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxSysColourChangedEvent);
};



class WXDLLIMPEXP_CORE wxMouseCaptureChangedEvent : public wxEvent
{
public:
    wxMouseCaptureChangedEvent(wxWindowID winid = 0, wxWindow* gainedCapture = NULL)
        : wxEvent(winid, wxEVT_MOUSE_CAPTURE_CHANGED),
          m_gainedCapture(gainedCapture)
        { }

    wxMouseCaptureChangedEvent(const wxMouseCaptureChangedEvent& event)
        : wxEvent(event),
          m_gainedCapture(event.m_gainedCapture)
        { }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxMouseCaptureChangedEvent(*this); }

    wxWindow* GetCapturedWindow() const { return m_gainedCapture; }

private:
    wxWindow* m_gainedCapture;

    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxMouseCaptureChangedEvent);
};



class WXDLLIMPEXP_CORE wxMouseCaptureLostEvent : public wxEvent
{
public:
    wxMouseCaptureLostEvent(wxWindowID winid = 0)
        : wxEvent(winid, wxEVT_MOUSE_CAPTURE_LOST)
    {}

    wxMouseCaptureLostEvent(const wxMouseCaptureLostEvent& event)
        : wxEvent(event)
    {}

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxMouseCaptureLostEvent(*this); }

    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxMouseCaptureLostEvent);
};


class WXDLLIMPEXP_CORE wxDisplayChangedEvent : public wxEvent
{
private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxDisplayChangedEvent);

public:
    wxDisplayChangedEvent()
        : wxEvent(0, wxEVT_DISPLAY_CHANGED)
        { }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxDisplayChangedEvent(*this); }
};



class WXDLLIMPEXP_CORE wxPaletteChangedEvent : public wxEvent
{
public:
    wxPaletteChangedEvent(wxWindowID winid = 0)
        : wxEvent(winid, wxEVT_PALETTE_CHANGED),
          m_changedWindow(NULL)
        { }

    wxPaletteChangedEvent(const wxPaletteChangedEvent& event)
        : wxEvent(event),
          m_changedWindow(event.m_changedWindow)
        { }

    void SetChangedWindow(wxWindow* win) { m_changedWindow = win; }
    wxWindow* GetChangedWindow() const { return m_changedWindow; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxPaletteChangedEvent(*this); }

protected:
    wxWindow*     m_changedWindow;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxPaletteChangedEvent);
};



class WXDLLIMPEXP_CORE wxQueryNewPaletteEvent : public wxEvent
{
public:
    wxQueryNewPaletteEvent(wxWindowID winid = 0)
        : wxEvent(winid, wxEVT_QUERY_NEW_PALETTE),
          m_paletteRealized(false)
        { }
    wxQueryNewPaletteEvent(const wxQueryNewPaletteEvent& event)
        : wxEvent(event),
        m_paletteRealized(event.m_paletteRealized)
    { }

        void SetPaletteRealized(bool realized) { m_paletteRealized = realized; }
    bool GetPaletteRealized() const { return m_paletteRealized; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxQueryNewPaletteEvent(*this); }

protected:
    bool m_paletteRealized;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxQueryNewPaletteEvent);
};


class WXDLLIMPEXP_CORE wxNavigationKeyEvent : public wxEvent
{
public:
    wxNavigationKeyEvent()
        : wxEvent(0, wxEVT_NAVIGATION_KEY),
          m_flags(IsForward | FromTab),              m_focus(NULL)
        {
            m_propagationLevel = wxEVENT_PROPAGATE_NONE;
        }

    wxNavigationKeyEvent(const wxNavigationKeyEvent& event)
        : wxEvent(event),
          m_flags(event.m_flags),
          m_focus(event.m_focus)
        { }

        bool GetDirection() const
        { return (m_flags & IsForward) != 0; }
    void SetDirection(bool bForward)
        { if ( bForward ) m_flags |= IsForward; else m_flags &= ~IsForward; }

            bool IsWindowChange() const
        { return (m_flags & WinChange) != 0; }
    void SetWindowChange(bool bIs)
        { if ( bIs ) m_flags |= WinChange; else m_flags &= ~WinChange; }

            bool IsFromTab() const
        { return (m_flags & FromTab) != 0; }
    void SetFromTab(bool bIs)
        { if ( bIs ) m_flags |= FromTab; else m_flags &= ~FromTab; }

            wxWindow* GetCurrentFocus() const { return m_focus; }
    void SetCurrentFocus(wxWindow *win) { m_focus = win; }

        void SetFlags(long flags) { m_flags = flags; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxNavigationKeyEvent(*this); }

    enum wxNavigationKeyEventFlags
    {
        IsBackward = 0x0000,
        IsForward = 0x0001,
        WinChange = 0x0002,
        FromTab = 0x0004
    };

    long m_flags;
    wxWindow *m_focus;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxNavigationKeyEvent);
};



class WXDLLIMPEXP_CORE wxWindowCreateEvent : public wxCommandEvent
{
public:
    wxWindowCreateEvent(wxWindow *win = NULL);

    wxWindow *GetWindow() const { return (wxWindow *)GetEventObject(); }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxWindowCreateEvent(*this); }

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxWindowCreateEvent);
};

class WXDLLIMPEXP_CORE wxWindowDestroyEvent : public wxCommandEvent
{
public:
    wxWindowDestroyEvent(wxWindow *win = NULL);

    wxWindow *GetWindow() const { return (wxWindow *)GetEventObject(); }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxWindowDestroyEvent(*this); }

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxWindowDestroyEvent);
};



class WXDLLIMPEXP_CORE wxHelpEvent : public wxCommandEvent
{
public:
        enum Origin
    {
        Origin_Unknown,            Origin_Keyboard,           Origin_HelpButton      };

    wxHelpEvent(wxEventType type = wxEVT_NULL,
                wxWindowID winid = 0,
                const wxPoint& pt = wxDefaultPosition,
                Origin origin = Origin_Unknown)
        : wxCommandEvent(type, winid),
          m_pos(pt),
          m_origin(GuessOrigin(origin))
    { }
    wxHelpEvent(const wxHelpEvent& event)
        : wxCommandEvent(event),
          m_pos(event.m_pos),
          m_target(event.m_target),
          m_link(event.m_link),
          m_origin(event.m_origin)
    { }

        const wxPoint& GetPosition() const { return m_pos; }
    void SetPosition(const wxPoint& pos) { m_pos = pos; }

        const wxString& GetLink() const { return m_link; }
    void SetLink(const wxString& link) { m_link = link; }

        const wxString& GetTarget() const { return m_target; }
    void SetTarget(const wxString& target) { m_target = target; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxHelpEvent(*this); }

        Origin GetOrigin() const { return m_origin; }
    void SetOrigin(Origin origin) { m_origin = origin; }

protected:
    wxPoint   m_pos;
    wxString  m_target;
    wxString  m_link;
    Origin    m_origin;

            static Origin GuessOrigin(Origin origin);

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxHelpEvent);
};



class WXDLLIMPEXP_CORE wxClipboardTextEvent : public wxCommandEvent
{
public:
    wxClipboardTextEvent(wxEventType type = wxEVT_NULL,
                     wxWindowID winid = 0)
        : wxCommandEvent(type, winid)
    { }
    wxClipboardTextEvent(const wxClipboardTextEvent& event)
        : wxCommandEvent(event)
    { }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxClipboardTextEvent(*this); }

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxClipboardTextEvent);
};



class WXDLLIMPEXP_CORE wxContextMenuEvent : public wxCommandEvent
{
public:
    wxContextMenuEvent(wxEventType type = wxEVT_NULL,
                       wxWindowID winid = 0,
                       const wxPoint& pt = wxDefaultPosition)
        : wxCommandEvent(type, winid),
          m_pos(pt)
    { }
    wxContextMenuEvent(const wxContextMenuEvent& event)
        : wxCommandEvent(event),
        m_pos(event.m_pos)
    { }

        const wxPoint& GetPosition() const { return m_pos; }
    void SetPosition(const wxPoint& pos) { m_pos = pos; }

    virtual wxEvent *Clone() const wxOVERRIDE { return new wxContextMenuEvent(*this); }

protected:
    wxPoint   m_pos;

private:
    wxDECLARE_DYNAMIC_CLASS_NO_ASSIGN(wxContextMenuEvent);
};




#endif 



struct WXDLLIMPEXP_BASE wxEventTableEntryBase
{
    wxEventTableEntryBase(int winid, int idLast,
                          wxEventFunctor* fn, wxObject *data)
        : m_id(winid),
          m_lastId(idLast),
          m_fn(fn),
          m_callbackUserData(data)
    {
        wxASSERT_MSG( idLast == wxID_ANY || winid <= idLast,
                      "invalid IDs range: lower bound > upper bound" );
    }

    wxEventTableEntryBase( const wxEventTableEntryBase &entry )
        : m_id( entry.m_id ),
          m_lastId( entry.m_lastId ),
          m_fn( entry.m_fn ),
          m_callbackUserData( entry.m_callbackUserData )
    {
                                        
        const_cast<wxEventTableEntryBase&>( entry ).m_fn = NULL;
    }

    ~wxEventTableEntryBase()
    {
        delete m_fn;
    }

            int m_id,
        m_lastId;

        wxEventFunctor* m_fn;

        wxObject* m_callbackUserData;

private:
    wxDECLARE_NO_ASSIGN_CLASS(wxEventTableEntryBase);
};

struct WXDLLIMPEXP_BASE wxEventTableEntry : public wxEventTableEntryBase
{
    wxEventTableEntry(const int& evType, int winid, int idLast,
                      wxEventFunctor* fn, wxObject *data)
        : wxEventTableEntryBase(winid, idLast, fn, data),
        m_eventType(evType)
    { }

                            const int& m_eventType;

private:
    wxDECLARE_NO_ASSIGN_CLASS(wxEventTableEntry);
};

struct WXDLLIMPEXP_BASE wxDynamicEventTableEntry : public wxEventTableEntryBase
{
    wxDynamicEventTableEntry(int evType, int winid, int idLast,
                             wxEventFunctor* fn, wxObject *data)
        : wxEventTableEntryBase(winid, idLast, fn, data),
          m_eventType(evType)
    { }

                int m_eventType;

private:
    wxDECLARE_NO_ASSIGN_CLASS(wxDynamicEventTableEntry);
};


struct WXDLLIMPEXP_BASE wxEventTable
{
    const wxEventTable *baseTable;        const wxEventTableEntry *entries; };


WX_DEFINE_ARRAY_PTR(const wxEventTableEntry*, wxEventTableEntryPointerArray);

class WXDLLIMPEXP_BASE wxEventHashTable
{
private:
        struct EventTypeTable
    {
        wxEventType                   eventType;
        wxEventTableEntryPointerArray eventEntryTable;
    };
    typedef EventTypeTable* EventTypeTablePointer;

public:
                    wxEventHashTable(const wxEventTable &table);
        ~wxEventHashTable();

            bool HandleEvent(wxEvent& event, wxEvtHandler *self);

        void Clear();

#if wxUSE_MEMORY_TRACING
            static void ClearAll();
#endif 
protected:
        void InitHashTable();
        void AddEntry(const wxEventTableEntry &entry);
        void AllocEventTypeTable(size_t size);
            void GrowEventTypeTable();

protected:
    const wxEventTable    &m_table;
    bool                   m_rebuildHash;

    size_t                 m_size;
    EventTypeTablePointer *m_eventTypeTable;

    static wxEventHashTable* sm_first;
    wxEventHashTable* m_previous;
    wxEventHashTable* m_next;

    wxDECLARE_NO_COPY_CLASS(wxEventHashTable);
};


class WXDLLIMPEXP_BASE wxEvtHandler : public wxObject
                                    , public wxTrackable
{
public:
    wxEvtHandler();
    virtual ~wxEvtHandler();


        
    wxEvtHandler *GetNextHandler() const { return m_nextHandler; }
    wxEvtHandler *GetPreviousHandler() const { return m_previousHandler; }
    virtual void SetNextHandler(wxEvtHandler *handler) { m_nextHandler = handler; }
    virtual void SetPreviousHandler(wxEvtHandler *handler) { m_previousHandler = handler; }

    void SetEvtHandlerEnabled(bool enabled) { m_enabled = enabled; }
    bool GetEvtHandlerEnabled() const { return m_enabled; }

    void Unlink();
    bool IsUnlinked() const;


        
                        static void AddFilter(wxEventFilter* filter);

        static void RemoveFilter(wxEventFilter* filter);


        
                virtual bool ProcessEvent(wxEvent& event);

                    bool SafelyProcessEvent(wxEvent& event);
        
                                                    bool ProcessEventLocally(wxEvent& event);

                        virtual void QueueEvent(wxEvent *event);

            virtual void AddPendingEvent(const wxEvent& event)
    {
                                                        QueueEvent(event.Clone());
    }

    void ProcessPendingEvents();
        
    void DeletePendingEvents();

#if wxUSE_THREADS
    bool ProcessThreadEvent(const wxEvent& event);
        #endif

#ifdef wxHAS_CALL_AFTER
                            
    template <typename T>
    void CallAfter(void (T::*method)())
    {
        QueueEvent(
            new wxAsyncMethodCallEvent0<T>(static_cast<T*>(this), method)
        );
    }

                    template <typename T, typename T1, typename P1>
    void CallAfter(void (T::*method)(T1 x1), P1 x1)
    {
        QueueEvent(
            new wxAsyncMethodCallEvent1<T, T1>(
                static_cast<T*>(this), method, x1)
        );
    }

    template <typename T, typename T1, typename T2, typename P1, typename P2>
    void CallAfter(void (T::*method)(T1 x1, T2 x2), P1 x1, P2 x2)
    {
        QueueEvent(
            new wxAsyncMethodCallEvent2<T, T1, T2>(
                static_cast<T*>(this), method, x1, x2)
        );
    }

    template <typename T>
    void CallAfter(const T& fn)
    {
        QueueEvent(new wxAsyncMethodCallEventFunctor<T>(this, fn));
    }
#endif 

        
                                
            void Connect(int winid,
                 int lastId,
                 wxEventType eventType,
                 wxObjectEventFunction func,
                 wxObject *userData = NULL,
                 wxEvtHandler *eventSink = NULL)
    {
        DoBind(winid, lastId, eventType,
                  wxNewEventFunctor(eventType, func, eventSink),
                  userData);
    }

        void Connect(int winid,
                 wxEventType eventType,
                 wxObjectEventFunction func,
                 wxObject *userData = NULL,
                 wxEvtHandler *eventSink = NULL)
        { Connect(winid, wxID_ANY, eventType, func, userData, eventSink); }

        void Connect(wxEventType eventType,
                 wxObjectEventFunction func,
                 wxObject *userData = NULL,
                 wxEvtHandler *eventSink = NULL)
        { Connect(wxID_ANY, wxID_ANY, eventType, func, userData, eventSink); }

    bool Disconnect(int winid,
                    int lastId,
                    wxEventType eventType,
                    wxObjectEventFunction func = NULL,
                    wxObject *userData = NULL,
                    wxEvtHandler *eventSink = NULL)
    {
        return DoUnbind(winid, lastId, eventType,
                            wxMakeEventFunctor(eventType, func, eventSink),
                            userData );
    }

    bool Disconnect(int winid = wxID_ANY,
                    wxEventType eventType = wxEVT_NULL,
                    wxObjectEventFunction func = NULL,
                    wxObject *userData = NULL,
                    wxEvtHandler *eventSink = NULL)
        { return Disconnect(winid, wxID_ANY, eventType, func, userData, eventSink); }

    bool Disconnect(wxEventType eventType,
                    wxObjectEventFunction func,
                    wxObject *userData = NULL,
                    wxEvtHandler *eventSink = NULL)
        { return Disconnect(wxID_ANY, eventType, func, userData, eventSink); }

        template <typename EventTag, typename EventArg>
    void Bind(const EventTag& eventType,
              void (*function)(EventArg &),
              int winid = wxID_ANY,
              int lastId = wxID_ANY,
              wxObject *userData = NULL)
    {
        DoBind(winid, lastId, eventType,
                  wxNewEventFunctor(eventType, function),
                  userData);
    }


    template <typename EventTag, typename EventArg>
    bool Unbind(const EventTag& eventType,
                void (*function)(EventArg &),
                int winid = wxID_ANY,
                int lastId = wxID_ANY,
                wxObject *userData = NULL)
    {
        return DoUnbind(winid, lastId, eventType,
                            wxMakeEventFunctor(eventType, function),
                            userData);
    }

        template <typename EventTag, typename Functor>
    void Bind(const EventTag& eventType,
              const Functor &functor,
              int winid = wxID_ANY,
              int lastId = wxID_ANY,
              wxObject *userData = NULL)
    {
        DoBind(winid, lastId, eventType,
                  wxNewEventFunctor(eventType, functor),
                  userData);
    }


    template <typename EventTag, typename Functor>
    bool Unbind(const EventTag& eventType,
                const Functor &functor,
                int winid = wxID_ANY,
                int lastId = wxID_ANY,
                wxObject *userData = NULL)
    {
        return DoUnbind(winid, lastId, eventType,
                            wxMakeEventFunctor(eventType, functor),
                            userData);
    }


        
    template <typename EventTag, typename Class, typename EventArg, typename EventHandler>
    void Bind(const EventTag &eventType,
              void (Class::*method)(EventArg &),
              EventHandler *handler,
              int winid = wxID_ANY,
              int lastId = wxID_ANY,
              wxObject *userData = NULL)
    {
        DoBind(winid, lastId, eventType,
                  wxNewEventFunctor(eventType, method, handler),
                  userData);
    }

    template <typename EventTag, typename Class, typename EventArg, typename EventHandler>
    bool Unbind(const EventTag &eventType,
                void (Class::*method)(EventArg&),
                EventHandler *handler,
                int winid = wxID_ANY,
                int lastId = wxID_ANY,
                wxObject *userData = NULL )
    {
        return DoUnbind(winid, lastId, eventType,
                            wxMakeEventFunctor(eventType, method, handler),
                            userData);
    }

        void SetClientObject( wxClientData *data ) { DoSetClientObject(data); }
    wxClientData *GetClientObject() const { return DoGetClientObject(); }

    void SetClientData( void *data ) { DoSetClientData(data); }
    void *GetClientData() const { return DoGetClientData(); }


        
                            static bool ProcessEventIfMatchesId(const wxEventTableEntryBase& tableEntry,
                                        wxEvtHandler *handler,
                                        wxEvent& event);

                        wxDynamicEventTableEntry* GetFirstDynamicEntry(size_t& cookie) const;
    wxDynamicEventTableEntry* GetNextDynamicEntry(size_t& cookie) const;

    virtual bool SearchEventTable(wxEventTable& table, wxEvent& event);
    bool SearchDynamicEventTable( wxEvent& event );

        void ClearEventHashTable() { GetEventHashTable().Clear(); }
    void OnSinkDestroyed( wxEvtHandler *sink );


private:
    void DoBind(int winid,
                   int lastId,
                   wxEventType eventType,
                   wxEventFunctor *func,
                   wxObject* userData = NULL);

    bool DoUnbind(int winid,
                      int lastId,
                      wxEventType eventType,
                      const wxEventFunctor& func,
                      wxObject *userData = NULL);

    static const wxEventTableEntry sm_eventTableEntries[];

protected:
        
                virtual bool TryBefore(wxEvent& event);

                        bool TryHereOnly(wxEvent& event);

            bool TryBeforeAndHere(wxEvent& event)
    {
        return TryBefore(event) || TryHereOnly(event);
    }

                    virtual bool TryAfter(wxEvent& event);

#if WXWIN_COMPATIBILITY_2_8
        wxDEPRECATED_BUT_USED_INTERNALLY_INLINE(
        virtual bool TryValidator(wxEvent& WXUNUSED(event)), return false; )

    wxDEPRECATED_BUT_USED_INTERNALLY_INLINE(
        virtual bool TryParent(wxEvent& event), return DoTryApp(event); )
#endif 
                    virtual bool OnDynamicBind(wxDynamicEventTableEntry& WXUNUSED(entry))
    {
        return true;
    }


    static const wxEventTable sm_eventTable;
    virtual const wxEventTable *GetEventTable() const;

    static wxEventHashTable   sm_eventHashTable;
    virtual wxEventHashTable& GetEventHashTable() const;

    wxEvtHandler*       m_nextHandler;
    wxEvtHandler*       m_previousHandler;

    typedef wxVector<wxDynamicEventTableEntry*> DynamicEvents;
    DynamicEvents* m_dynamicEvents;

    wxList*             m_pendingEvents;

#if wxUSE_THREADS
        wxCriticalSection m_pendingEventsLock;
#endif 
        bool                m_enabled;


                        union
    {
        wxClientData *m_clientObject;
        void         *m_clientData;
    };

        wxClientDataType m_clientDataType;

        virtual void DoSetClientObject( wxClientData *data );
    virtual wxClientData *DoGetClientObject() const;

    virtual void DoSetClientData( void *data );
    virtual void *DoGetClientData() const;

        wxEventConnectionRef *FindRefInTrackerList(wxEvtHandler *handler);

private:
        bool DoTryApp(wxEvent& event);

        bool DoTryChain(wxEvent& event);

        static wxEventFilter* ms_filterList;

    wxDECLARE_DYNAMIC_CLASS_NO_COPY(wxEvtHandler);
};

WX_DEFINE_ARRAY_WITH_DECL_PTR(wxEvtHandler *, wxEvtHandlerArray, class WXDLLIMPEXP_BASE);


inline void wxObjectEventFunctor::operator()(wxEvtHandler *handler, wxEvent& event)
{
    wxEvtHandler * const realHandler = m_handler ? m_handler : handler;

    (realHandler->*m_method)(event);
}


class wxEventConnectionRef : public wxTrackerNode
{
public:
    wxEventConnectionRef() : m_src(NULL), m_sink(NULL), m_refCount(0) { }
    wxEventConnectionRef(wxEvtHandler *src, wxEvtHandler *sink)
        : m_src(src), m_sink(sink), m_refCount(1)
    {
        m_sink->AddNode(this);
    }

        virtual void OnObjectDestroy( ) wxOVERRIDE
    {
        if ( m_src )
            m_src->OnSinkDestroyed( m_sink );
        delete this;
    }

    virtual wxEventConnectionRef *ToEventConnection() wxOVERRIDE { return this; }

    void IncRef() { m_refCount++; }
    void DecRef()
    {
        if ( !--m_refCount )
        {
                        if ( m_sink )
                m_sink->RemoveNode(this);
            delete this;
        }
    }

private:
    wxEvtHandler *m_src,
                 *m_sink;
    int m_refCount;

    friend class wxEvtHandler;

    wxDECLARE_NO_ASSIGN_CLASS(wxEventConnectionRef);
};

inline void wxPostEvent(wxEvtHandler *dest, const wxEvent& event)
{
    wxCHECK_RET( dest, "need an object to post event to" );

    dest->AddPendingEvent(event);
}

inline void wxQueueEvent(wxEvtHandler *dest, wxEvent *event)
{
    wxCHECK_RET( dest, "need an object to queue event for" );

    dest->QueueEvent(event);
}

typedef void (wxEvtHandler::*wxEventFunction)(wxEvent&);
typedef void (wxEvtHandler::*wxIdleEventFunction)(wxIdleEvent&);
typedef void (wxEvtHandler::*wxThreadEventFunction)(wxThreadEvent&);

#define wxEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxEventFunction, func)
#define wxIdleEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxIdleEventFunction, func)
#define wxThreadEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxThreadEventFunction, func)

#if wxUSE_GUI


class WXDLLIMPEXP_CORE wxEventBlocker : public wxEvtHandler
{
public:
    wxEventBlocker(wxWindow *win, wxEventType type = wxEVT_ANY);
    virtual ~wxEventBlocker();

    void Block(wxEventType type)
    {
        m_eventsToBlock.push_back(type);
    }

    virtual bool ProcessEvent(wxEvent& event) wxOVERRIDE;

protected:
    wxArrayInt m_eventsToBlock;
    wxWindow *m_window;

    wxDECLARE_NO_COPY_CLASS(wxEventBlocker);
};

typedef void (wxEvtHandler::*wxCommandEventFunction)(wxCommandEvent&);
typedef void (wxEvtHandler::*wxScrollEventFunction)(wxScrollEvent&);
typedef void (wxEvtHandler::*wxScrollWinEventFunction)(wxScrollWinEvent&);
typedef void (wxEvtHandler::*wxSizeEventFunction)(wxSizeEvent&);
typedef void (wxEvtHandler::*wxMoveEventFunction)(wxMoveEvent&);
typedef void (wxEvtHandler::*wxPaintEventFunction)(wxPaintEvent&);
typedef void (wxEvtHandler::*wxNcPaintEventFunction)(wxNcPaintEvent&);
typedef void (wxEvtHandler::*wxEraseEventFunction)(wxEraseEvent&);
typedef void (wxEvtHandler::*wxMouseEventFunction)(wxMouseEvent&);
typedef void (wxEvtHandler::*wxCharEventFunction)(wxKeyEvent&);
typedef void (wxEvtHandler::*wxFocusEventFunction)(wxFocusEvent&);
typedef void (wxEvtHandler::*wxChildFocusEventFunction)(wxChildFocusEvent&);
typedef void (wxEvtHandler::*wxActivateEventFunction)(wxActivateEvent&);
typedef void (wxEvtHandler::*wxMenuEventFunction)(wxMenuEvent&);
typedef void (wxEvtHandler::*wxJoystickEventFunction)(wxJoystickEvent&);
typedef void (wxEvtHandler::*wxDropFilesEventFunction)(wxDropFilesEvent&);
typedef void (wxEvtHandler::*wxInitDialogEventFunction)(wxInitDialogEvent&);
typedef void (wxEvtHandler::*wxSysColourChangedEventFunction)(wxSysColourChangedEvent&);
typedef void (wxEvtHandler::*wxDisplayChangedEventFunction)(wxDisplayChangedEvent&);
typedef void (wxEvtHandler::*wxUpdateUIEventFunction)(wxUpdateUIEvent&);
typedef void (wxEvtHandler::*wxCloseEventFunction)(wxCloseEvent&);
typedef void (wxEvtHandler::*wxShowEventFunction)(wxShowEvent&);
typedef void (wxEvtHandler::*wxIconizeEventFunction)(wxIconizeEvent&);
typedef void (wxEvtHandler::*wxMaximizeEventFunction)(wxMaximizeEvent&);
typedef void (wxEvtHandler::*wxNavigationKeyEventFunction)(wxNavigationKeyEvent&);
typedef void (wxEvtHandler::*wxPaletteChangedEventFunction)(wxPaletteChangedEvent&);
typedef void (wxEvtHandler::*wxQueryNewPaletteEventFunction)(wxQueryNewPaletteEvent&);
typedef void (wxEvtHandler::*wxWindowCreateEventFunction)(wxWindowCreateEvent&);
typedef void (wxEvtHandler::*wxWindowDestroyEventFunction)(wxWindowDestroyEvent&);
typedef void (wxEvtHandler::*wxSetCursorEventFunction)(wxSetCursorEvent&);
typedef void (wxEvtHandler::*wxNotifyEventFunction)(wxNotifyEvent&);
typedef void (wxEvtHandler::*wxHelpEventFunction)(wxHelpEvent&);
typedef void (wxEvtHandler::*wxContextMenuEventFunction)(wxContextMenuEvent&);
typedef void (wxEvtHandler::*wxMouseCaptureChangedEventFunction)(wxMouseCaptureChangedEvent&);
typedef void (wxEvtHandler::*wxMouseCaptureLostEventFunction)(wxMouseCaptureLostEvent&);
typedef void (wxEvtHandler::*wxClipboardTextEventFunction)(wxClipboardTextEvent&);


#define wxCommandEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxCommandEventFunction, func)
#define wxScrollEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxScrollEventFunction, func)
#define wxScrollWinEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxScrollWinEventFunction, func)
#define wxSizeEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxSizeEventFunction, func)
#define wxMoveEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMoveEventFunction, func)
#define wxPaintEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxPaintEventFunction, func)
#define wxNcPaintEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxNcPaintEventFunction, func)
#define wxEraseEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxEraseEventFunction, func)
#define wxMouseEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMouseEventFunction, func)
#define wxCharEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxCharEventFunction, func)
#define wxKeyEventHandler(func) wxCharEventHandler(func)
#define wxFocusEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxFocusEventFunction, func)
#define wxChildFocusEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxChildFocusEventFunction, func)
#define wxActivateEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxActivateEventFunction, func)
#define wxMenuEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMenuEventFunction, func)
#define wxJoystickEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxJoystickEventFunction, func)
#define wxDropFilesEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxDropFilesEventFunction, func)
#define wxInitDialogEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxInitDialogEventFunction, func)
#define wxSysColourChangedEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxSysColourChangedEventFunction, func)
#define wxDisplayChangedEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxDisplayChangedEventFunction, func)
#define wxUpdateUIEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxUpdateUIEventFunction, func)
#define wxCloseEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxCloseEventFunction, func)
#define wxShowEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxShowEventFunction, func)
#define wxIconizeEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxIconizeEventFunction, func)
#define wxMaximizeEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMaximizeEventFunction, func)
#define wxNavigationKeyEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxNavigationKeyEventFunction, func)
#define wxPaletteChangedEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxPaletteChangedEventFunction, func)
#define wxQueryNewPaletteEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxQueryNewPaletteEventFunction, func)
#define wxWindowCreateEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxWindowCreateEventFunction, func)
#define wxWindowDestroyEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxWindowDestroyEventFunction, func)
#define wxSetCursorEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxSetCursorEventFunction, func)
#define wxNotifyEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxNotifyEventFunction, func)
#define wxHelpEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxHelpEventFunction, func)
#define wxContextMenuEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxContextMenuEventFunction, func)
#define wxMouseCaptureChangedEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMouseCaptureChangedEventFunction, func)
#define wxMouseCaptureLostEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxMouseCaptureLostEventFunction, func)
#define wxClipboardTextEventHandler(func) \
    wxEVENT_HANDLER_CAST(wxClipboardTextEventFunction, func)

#endif 

#define wxDECLARE_EVENT_TABLE()                                         \
    private:                                                            \
        static const wxEventTableEntry sm_eventTableEntries[];          \
    protected:                                                          \
        wxCLANG_WARNING_SUPPRESS(inconsistent-missing-override)         \
        const wxEventTable* GetEventTable() const;                      \
        wxEventHashTable& GetEventHashTable() const;                    \
        wxCLANG_WARNING_RESTORE(inconsistent-missing-override)          \
        static const wxEventTable        sm_eventTable;                 \
        static wxEventHashTable          sm_eventHashTable


#define wxBEGIN_EVENT_TABLE(theClass, baseClass) \
    const wxEventTable theClass::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass::sm_eventTableEntries[0] }; \
    const wxEventTable *theClass::GetEventTable() const \
        { return &theClass::sm_eventTable; } \
    wxEventHashTable theClass::sm_eventHashTable(theClass::sm_eventTable); \
    wxEventHashTable &theClass::GetEventHashTable() const \
        { return theClass::sm_eventHashTable; } \
    const wxEventTableEntry theClass::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE1(theClass, baseClass, T1) \
    template<typename T1> \
    const wxEventTable theClass<T1>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1>::sm_eventTableEntries[0] }; \
    template<typename T1> \
    const wxEventTable *theClass<T1>::GetEventTable() const \
        { return &theClass<T1>::sm_eventTable; } \
    template<typename T1> \
    wxEventHashTable theClass<T1>::sm_eventHashTable(theClass<T1>::sm_eventTable); \
    template<typename T1> \
    wxEventHashTable &theClass<T1>::GetEventHashTable() const \
        { return theClass<T1>::sm_eventHashTable; } \
    template<typename T1> \
    const wxEventTableEntry theClass<T1>::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE2(theClass, baseClass, T1, T2) \
    template<typename T1, typename T2> \
    const wxEventTable theClass<T1, T2>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1, T2>::sm_eventTableEntries[0] }; \
    template<typename T1, typename T2> \
    const wxEventTable *theClass<T1, T2>::GetEventTable() const \
        { return &theClass<T1, T2>::sm_eventTable; } \
    template<typename T1, typename T2> \
    wxEventHashTable theClass<T1, T2>::sm_eventHashTable(theClass<T1, T2>::sm_eventTable); \
    template<typename T1, typename T2> \
    wxEventHashTable &theClass<T1, T2>::GetEventHashTable() const \
        { return theClass<T1, T2>::sm_eventHashTable; } \
    template<typename T1, typename T2> \
    const wxEventTableEntry theClass<T1, T2>::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE3(theClass, baseClass, T1, T2, T3) \
    template<typename T1, typename T2, typename T3> \
    const wxEventTable theClass<T1, T2, T3>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1, T2, T3>::sm_eventTableEntries[0] }; \
    template<typename T1, typename T2, typename T3> \
    const wxEventTable *theClass<T1, T2, T3>::GetEventTable() const \
        { return &theClass<T1, T2, T3>::sm_eventTable; } \
    template<typename T1, typename T2, typename T3> \
    wxEventHashTable theClass<T1, T2, T3>::sm_eventHashTable(theClass<T1, T2, T3>::sm_eventTable); \
    template<typename T1, typename T2, typename T3> \
    wxEventHashTable &theClass<T1, T2, T3>::GetEventHashTable() const \
        { return theClass<T1, T2, T3>::sm_eventHashTable; } \
    template<typename T1, typename T2, typename T3> \
    const wxEventTableEntry theClass<T1, T2, T3>::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE4(theClass, baseClass, T1, T2, T3, T4) \
    template<typename T1, typename T2, typename T3, typename T4> \
    const wxEventTable theClass<T1, T2, T3, T4>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1, T2, T3, T4>::sm_eventTableEntries[0] }; \
    template<typename T1, typename T2, typename T3, typename T4> \
    const wxEventTable *theClass<T1, T2, T3, T4>::GetEventTable() const \
        { return &theClass<T1, T2, T3, T4>::sm_eventTable; } \
    template<typename T1, typename T2, typename T3, typename T4> \
    wxEventHashTable theClass<T1, T2, T3, T4>::sm_eventHashTable(theClass<T1, T2, T3, T4>::sm_eventTable); \
    template<typename T1, typename T2, typename T3, typename T4> \
    wxEventHashTable &theClass<T1, T2, T3, T4>::GetEventHashTable() const \
        { return theClass<T1, T2, T3, T4>::sm_eventHashTable; } \
    template<typename T1, typename T2, typename T3, typename T4> \
    const wxEventTableEntry theClass<T1, T2, T3, T4>::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE5(theClass, baseClass, T1, T2, T3, T4, T5) \
    template<typename T1, typename T2, typename T3, typename T4, typename T5> \
    const wxEventTable theClass<T1, T2, T3, T4, T5>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1, T2, T3, T4, T5>::sm_eventTableEntries[0] }; \
    template<typename T1, typename T2, typename T3, typename T4, typename T5> \
    const wxEventTable *theClass<T1, T2, T3, T4, T5>::GetEventTable() const \
        { return &theClass<T1, T2, T3, T4, T5>::sm_eventTable; } \
    template<typename T1, typename T2, typename T3, typename T4, typename T5> \
    wxEventHashTable theClass<T1, T2, T3, T4, T5>::sm_eventHashTable(theClass<T1, T2, T3, T4, T5>::sm_eventTable); \
    template<typename T1, typename T2, typename T3, typename T4, typename T5> \
    wxEventHashTable &theClass<T1, T2, T3, T4, T5>::GetEventHashTable() const \
        { return theClass<T1, T2, T3, T4, T5>::sm_eventHashTable; } \
    template<typename T1, typename T2, typename T3, typename T4, typename T5> \
    const wxEventTableEntry theClass<T1, T2, T3, T4, T5>::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE7(theClass, baseClass, T1, T2, T3, T4, T5, T6, T7) \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> \
    const wxEventTable theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventTableEntries[0] }; \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> \
    const wxEventTable *theClass<T1, T2, T3, T4, T5, T6, T7>::GetEventTable() const \
        { return &theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventTable; } \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> \
    wxEventHashTable theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventHashTable(theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventTable); \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> \
    wxEventHashTable &theClass<T1, T2, T3, T4, T5, T6, T7>::GetEventHashTable() const \
        { return theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventHashTable; } \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> \
    const wxEventTableEntry theClass<T1, T2, T3, T4, T5, T6, T7>::sm_eventTableEntries[] = { \

#define wxBEGIN_EVENT_TABLE_TEMPLATE8(theClass, baseClass, T1, T2, T3, T4, T5, T6, T7, T8) \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> \
    const wxEventTable theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventTable = \
        { &baseClass::sm_eventTable, &theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventTableEntries[0] }; \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> \
    const wxEventTable *theClass<T1, T2, T3, T4, T5, T6, T7, T8>::GetEventTable() const \
        { return &theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventTable; } \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> \
    wxEventHashTable theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventHashTable(theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventTable); \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> \
    wxEventHashTable &theClass<T1, T2, T3, T4, T5, T6, T7, T8>::GetEventHashTable() const \
        { return theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventHashTable; } \
    template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> \
    const wxEventTableEntry theClass<T1, T2, T3, T4, T5, T6, T7, T8>::sm_eventTableEntries[] = { \

#define wxEND_EVENT_TABLE() \
    wxDECLARE_EVENT_TABLE_TERMINATOR() };



#define wx__DECLARE_EVT2(evt, id1, id2, fn) \
    wxDECLARE_EVENT_TABLE_ENTRY(evt, id1, id2, fn, NULL),
#define wx__DECLARE_EVT1(evt, id, fn) \
    wx__DECLARE_EVT2(evt, id, wxID_ANY, fn)
#define wx__DECLARE_EVT0(evt, fn) \
    wx__DECLARE_EVT1(evt, wxID_ANY, fn)


#define EVT_CUSTOM(event, winid, func) \
    wx__DECLARE_EVT1(event, winid, wxEventHandler(func))
#define EVT_CUSTOM_RANGE(event, id1, id2, func) \
    wx__DECLARE_EVT2(event, id1, id2, wxEventHandler(func))

#define EVT_COMMAND(winid, event, func) \
    wx__DECLARE_EVT1(event, winid, wxCommandEventHandler(func))

#define EVT_COMMAND_RANGE(id1, id2, event, func) \
    wx__DECLARE_EVT2(event, id1, id2, wxCommandEventHandler(func))

#define EVT_NOTIFY(event, winid, func) \
    wx__DECLARE_EVT1(event, winid, wxNotifyEventHandler(func))

#define EVT_NOTIFY_RANGE(event, id1, id2, func) \
    wx__DECLARE_EVT2(event, id1, id2, wxNotifyEventHandler(func))

#define EVT_SIZE(func)  wx__DECLARE_EVT0(wxEVT_SIZE, wxSizeEventHandler(func))
#define EVT_SIZING(func)  wx__DECLARE_EVT0(wxEVT_SIZING, wxSizeEventHandler(func))
#define EVT_MOVE(func)  wx__DECLARE_EVT0(wxEVT_MOVE, wxMoveEventHandler(func))
#define EVT_MOVING(func)  wx__DECLARE_EVT0(wxEVT_MOVING, wxMoveEventHandler(func))
#define EVT_MOVE_START(func)  wx__DECLARE_EVT0(wxEVT_MOVE_START, wxMoveEventHandler(func))
#define EVT_MOVE_END(func)  wx__DECLARE_EVT0(wxEVT_MOVE_END, wxMoveEventHandler(func))
#define EVT_CLOSE(func)  wx__DECLARE_EVT0(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(func))
#define EVT_END_SESSION(func)  wx__DECLARE_EVT0(wxEVT_END_SESSION, wxCloseEventHandler(func))
#define EVT_QUERY_END_SESSION(func)  wx__DECLARE_EVT0(wxEVT_QUERY_END_SESSION, wxCloseEventHandler(func))
#define EVT_PAINT(func)  wx__DECLARE_EVT0(wxEVT_PAINT, wxPaintEventHandler(func))
#define EVT_NC_PAINT(func)  wx__DECLARE_EVT0(wxEVT_NC_PAINT, wxNcPaintEventHandler(func))
#define EVT_ERASE_BACKGROUND(func)  wx__DECLARE_EVT0(wxEVT_ERASE_BACKGROUND, wxEraseEventHandler(func))
#define EVT_CHAR(func)  wx__DECLARE_EVT0(wxEVT_CHAR, wxCharEventHandler(func))
#define EVT_KEY_DOWN(func)  wx__DECLARE_EVT0(wxEVT_KEY_DOWN, wxKeyEventHandler(func))
#define EVT_KEY_UP(func)  wx__DECLARE_EVT0(wxEVT_KEY_UP, wxKeyEventHandler(func))
#if wxUSE_HOTKEY
#define EVT_HOTKEY(winid, func)  wx__DECLARE_EVT1(wxEVT_HOTKEY, winid, wxCharEventHandler(func))
#endif
#define EVT_CHAR_HOOK(func)  wx__DECLARE_EVT0(wxEVT_CHAR_HOOK, wxCharEventHandler(func))
#define EVT_MENU_OPEN(func)  wx__DECLARE_EVT0(wxEVT_MENU_OPEN, wxMenuEventHandler(func))
#define EVT_MENU_CLOSE(func)  wx__DECLARE_EVT0(wxEVT_MENU_CLOSE, wxMenuEventHandler(func))
#define EVT_MENU_HIGHLIGHT(winid, func)  wx__DECLARE_EVT1(wxEVT_MENU_HIGHLIGHT, winid, wxMenuEventHandler(func))
#define EVT_MENU_HIGHLIGHT_ALL(func)  wx__DECLARE_EVT0(wxEVT_MENU_HIGHLIGHT, wxMenuEventHandler(func))
#define EVT_SET_FOCUS(func)  wx__DECLARE_EVT0(wxEVT_SET_FOCUS, wxFocusEventHandler(func))
#define EVT_KILL_FOCUS(func)  wx__DECLARE_EVT0(wxEVT_KILL_FOCUS, wxFocusEventHandler(func))
#define EVT_CHILD_FOCUS(func)  wx__DECLARE_EVT0(wxEVT_CHILD_FOCUS, wxChildFocusEventHandler(func))
#define EVT_ACTIVATE(func)  wx__DECLARE_EVT0(wxEVT_ACTIVATE, wxActivateEventHandler(func))
#define EVT_ACTIVATE_APP(func)  wx__DECLARE_EVT0(wxEVT_ACTIVATE_APP, wxActivateEventHandler(func))
#define EVT_HIBERNATE(func)  wx__DECLARE_EVT0(wxEVT_HIBERNATE, wxActivateEventHandler(func))
#define EVT_END_SESSION(func)  wx__DECLARE_EVT0(wxEVT_END_SESSION, wxCloseEventHandler(func))
#define EVT_QUERY_END_SESSION(func)  wx__DECLARE_EVT0(wxEVT_QUERY_END_SESSION, wxCloseEventHandler(func))
#define EVT_DROP_FILES(func)  wx__DECLARE_EVT0(wxEVT_DROP_FILES, wxDropFilesEventHandler(func))
#define EVT_INIT_DIALOG(func)  wx__DECLARE_EVT0(wxEVT_INIT_DIALOG, wxInitDialogEventHandler(func))
#define EVT_SYS_COLOUR_CHANGED(func) wx__DECLARE_EVT0(wxEVT_SYS_COLOUR_CHANGED, wxSysColourChangedEventHandler(func))
#define EVT_DISPLAY_CHANGED(func)  wx__DECLARE_EVT0(wxEVT_DISPLAY_CHANGED, wxDisplayChangedEventHandler(func))
#define EVT_SHOW(func) wx__DECLARE_EVT0(wxEVT_SHOW, wxShowEventHandler(func))
#define EVT_MAXIMIZE(func) wx__DECLARE_EVT0(wxEVT_MAXIMIZE, wxMaximizeEventHandler(func))
#define EVT_ICONIZE(func) wx__DECLARE_EVT0(wxEVT_ICONIZE, wxIconizeEventHandler(func))
#define EVT_NAVIGATION_KEY(func) wx__DECLARE_EVT0(wxEVT_NAVIGATION_KEY, wxNavigationKeyEventHandler(func))
#define EVT_PALETTE_CHANGED(func) wx__DECLARE_EVT0(wxEVT_PALETTE_CHANGED, wxPaletteChangedEventHandler(func))
#define EVT_QUERY_NEW_PALETTE(func) wx__DECLARE_EVT0(wxEVT_QUERY_NEW_PALETTE, wxQueryNewPaletteEventHandler(func))
#define EVT_WINDOW_CREATE(func) wx__DECLARE_EVT0(wxEVT_CREATE, wxWindowCreateEventHandler(func))
#define EVT_WINDOW_DESTROY(func) wx__DECLARE_EVT0(wxEVT_DESTROY, wxWindowDestroyEventHandler(func))
#define EVT_SET_CURSOR(func) wx__DECLARE_EVT0(wxEVT_SET_CURSOR, wxSetCursorEventHandler(func))
#define EVT_MOUSE_CAPTURE_CHANGED(func) wx__DECLARE_EVT0(wxEVT_MOUSE_CAPTURE_CHANGED, wxMouseCaptureChangedEventHandler(func))
#define EVT_MOUSE_CAPTURE_LOST(func) wx__DECLARE_EVT0(wxEVT_MOUSE_CAPTURE_LOST, wxMouseCaptureLostEventHandler(func))

#define EVT_LEFT_DOWN(func) wx__DECLARE_EVT0(wxEVT_LEFT_DOWN, wxMouseEventHandler(func))
#define EVT_LEFT_UP(func) wx__DECLARE_EVT0(wxEVT_LEFT_UP, wxMouseEventHandler(func))
#define EVT_MIDDLE_DOWN(func) wx__DECLARE_EVT0(wxEVT_MIDDLE_DOWN, wxMouseEventHandler(func))
#define EVT_MIDDLE_UP(func) wx__DECLARE_EVT0(wxEVT_MIDDLE_UP, wxMouseEventHandler(func))
#define EVT_RIGHT_DOWN(func) wx__DECLARE_EVT0(wxEVT_RIGHT_DOWN, wxMouseEventHandler(func))
#define EVT_RIGHT_UP(func) wx__DECLARE_EVT0(wxEVT_RIGHT_UP, wxMouseEventHandler(func))
#define EVT_MOTION(func) wx__DECLARE_EVT0(wxEVT_MOTION, wxMouseEventHandler(func))
#define EVT_LEFT_DCLICK(func) wx__DECLARE_EVT0(wxEVT_LEFT_DCLICK, wxMouseEventHandler(func))
#define EVT_MIDDLE_DCLICK(func) wx__DECLARE_EVT0(wxEVT_MIDDLE_DCLICK, wxMouseEventHandler(func))
#define EVT_RIGHT_DCLICK(func) wx__DECLARE_EVT0(wxEVT_RIGHT_DCLICK, wxMouseEventHandler(func))
#define EVT_LEAVE_WINDOW(func) wx__DECLARE_EVT0(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(func))
#define EVT_ENTER_WINDOW(func) wx__DECLARE_EVT0(wxEVT_ENTER_WINDOW, wxMouseEventHandler(func))
#define EVT_MOUSEWHEEL(func) wx__DECLARE_EVT0(wxEVT_MOUSEWHEEL, wxMouseEventHandler(func))
#define EVT_MOUSE_AUX1_DOWN(func) wx__DECLARE_EVT0(wxEVT_AUX1_DOWN, wxMouseEventHandler(func))
#define EVT_MOUSE_AUX1_UP(func) wx__DECLARE_EVT0(wxEVT_AUX1_UP, wxMouseEventHandler(func))
#define EVT_MOUSE_AUX1_DCLICK(func) wx__DECLARE_EVT0(wxEVT_AUX1_DCLICK, wxMouseEventHandler(func))
#define EVT_MOUSE_AUX2_DOWN(func) wx__DECLARE_EVT0(wxEVT_AUX2_DOWN, wxMouseEventHandler(func))
#define EVT_MOUSE_AUX2_UP(func) wx__DECLARE_EVT0(wxEVT_AUX2_UP, wxMouseEventHandler(func))
#define EVT_MOUSE_AUX2_DCLICK(func) wx__DECLARE_EVT0(wxEVT_AUX2_DCLICK, wxMouseEventHandler(func))
#define EVT_MAGNIFY(func) wx__DECLARE_EVT0(wxEVT_MAGNIFY, wxMouseEventHandler(func))

#define EVT_MOUSE_EVENTS(func) \
    EVT_LEFT_DOWN(func) \
    EVT_LEFT_UP(func) \
    EVT_LEFT_DCLICK(func) \
    EVT_MIDDLE_DOWN(func) \
    EVT_MIDDLE_UP(func) \
    EVT_MIDDLE_DCLICK(func) \
    EVT_RIGHT_DOWN(func) \
    EVT_RIGHT_UP(func) \
    EVT_RIGHT_DCLICK(func) \
    EVT_MOUSE_AUX1_DOWN(func) \
    EVT_MOUSE_AUX1_UP(func) \
    EVT_MOUSE_AUX1_DCLICK(func) \
    EVT_MOUSE_AUX2_DOWN(func) \
    EVT_MOUSE_AUX2_UP(func) \
    EVT_MOUSE_AUX2_DCLICK(func) \
    EVT_MOTION(func) \
    EVT_LEAVE_WINDOW(func) \
    EVT_ENTER_WINDOW(func) \
    EVT_MOUSEWHEEL(func) \
    EVT_MAGNIFY(func)

#define EVT_SCROLLWIN_TOP(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_TOP, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_BOTTOM(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_BOTTOM, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_LINEUP(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_LINEUP, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_LINEDOWN(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_LINEDOWN, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_PAGEUP(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_PAGEUP, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_PAGEDOWN(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_PAGEDOWN, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_THUMBTRACK(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_THUMBTRACK, wxScrollWinEventHandler(func))
#define EVT_SCROLLWIN_THUMBRELEASE(func) wx__DECLARE_EVT0(wxEVT_SCROLLWIN_THUMBRELEASE, wxScrollWinEventHandler(func))

#define EVT_SCROLLWIN(func) \
    EVT_SCROLLWIN_TOP(func) \
    EVT_SCROLLWIN_BOTTOM(func) \
    EVT_SCROLLWIN_LINEUP(func) \
    EVT_SCROLLWIN_LINEDOWN(func) \
    EVT_SCROLLWIN_PAGEUP(func) \
    EVT_SCROLLWIN_PAGEDOWN(func) \
    EVT_SCROLLWIN_THUMBTRACK(func) \
    EVT_SCROLLWIN_THUMBRELEASE(func)

#define EVT_SCROLL_TOP(func) wx__DECLARE_EVT0(wxEVT_SCROLL_TOP, wxScrollEventHandler(func))
#define EVT_SCROLL_BOTTOM(func) wx__DECLARE_EVT0(wxEVT_SCROLL_BOTTOM, wxScrollEventHandler(func))
#define EVT_SCROLL_LINEUP(func) wx__DECLARE_EVT0(wxEVT_SCROLL_LINEUP, wxScrollEventHandler(func))
#define EVT_SCROLL_LINEDOWN(func) wx__DECLARE_EVT0(wxEVT_SCROLL_LINEDOWN, wxScrollEventHandler(func))
#define EVT_SCROLL_PAGEUP(func) wx__DECLARE_EVT0(wxEVT_SCROLL_PAGEUP, wxScrollEventHandler(func))
#define EVT_SCROLL_PAGEDOWN(func) wx__DECLARE_EVT0(wxEVT_SCROLL_PAGEDOWN, wxScrollEventHandler(func))
#define EVT_SCROLL_THUMBTRACK(func) wx__DECLARE_EVT0(wxEVT_SCROLL_THUMBTRACK, wxScrollEventHandler(func))
#define EVT_SCROLL_THUMBRELEASE(func) wx__DECLARE_EVT0(wxEVT_SCROLL_THUMBRELEASE, wxScrollEventHandler(func))
#define EVT_SCROLL_CHANGED(func) wx__DECLARE_EVT0(wxEVT_SCROLL_CHANGED, wxScrollEventHandler(func))

#define EVT_SCROLL(func) \
    EVT_SCROLL_TOP(func) \
    EVT_SCROLL_BOTTOM(func) \
    EVT_SCROLL_LINEUP(func) \
    EVT_SCROLL_LINEDOWN(func) \
    EVT_SCROLL_PAGEUP(func) \
    EVT_SCROLL_PAGEDOWN(func) \
    EVT_SCROLL_THUMBTRACK(func) \
    EVT_SCROLL_THUMBRELEASE(func) \
    EVT_SCROLL_CHANGED(func)

#define EVT_COMMAND_SCROLL_TOP(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_TOP, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_BOTTOM(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_BOTTOM, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_LINEUP(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_LINEUP, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_LINEDOWN(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_LINEDOWN, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_PAGEUP(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_PAGEUP, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_PAGEDOWN(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_PAGEDOWN, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_THUMBTRACK(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_THUMBTRACK, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_THUMBRELEASE(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_THUMBRELEASE, winid, wxScrollEventHandler(func))
#define EVT_COMMAND_SCROLL_CHANGED(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLL_CHANGED, winid, wxScrollEventHandler(func))

#define EVT_COMMAND_SCROLL(winid, func) \
    EVT_COMMAND_SCROLL_TOP(winid, func) \
    EVT_COMMAND_SCROLL_BOTTOM(winid, func) \
    EVT_COMMAND_SCROLL_LINEUP(winid, func) \
    EVT_COMMAND_SCROLL_LINEDOWN(winid, func) \
    EVT_COMMAND_SCROLL_PAGEUP(winid, func) \
    EVT_COMMAND_SCROLL_PAGEDOWN(winid, func) \
    EVT_COMMAND_SCROLL_THUMBTRACK(winid, func) \
    EVT_COMMAND_SCROLL_THUMBRELEASE(winid, func) \
    EVT_COMMAND_SCROLL_CHANGED(winid, func)

#define EVT_CHECKBOX(winid, func) wx__DECLARE_EVT1(wxEVT_CHECKBOX, winid, wxCommandEventHandler(func))
#define EVT_CHOICE(winid, func) wx__DECLARE_EVT1(wxEVT_CHOICE, winid, wxCommandEventHandler(func))
#define EVT_LISTBOX(winid, func) wx__DECLARE_EVT1(wxEVT_LISTBOX, winid, wxCommandEventHandler(func))
#define EVT_LISTBOX_DCLICK(winid, func) wx__DECLARE_EVT1(wxEVT_LISTBOX_DCLICK, winid, wxCommandEventHandler(func))
#define EVT_MENU(winid, func) wx__DECLARE_EVT1(wxEVT_MENU, winid, wxCommandEventHandler(func))
#define EVT_MENU_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_MENU, id1, id2, wxCommandEventHandler(func))
#define EVT_BUTTON(winid, func) wx__DECLARE_EVT1(wxEVT_BUTTON, winid, wxCommandEventHandler(func))
#define EVT_SLIDER(winid, func) wx__DECLARE_EVT1(wxEVT_SLIDER, winid, wxCommandEventHandler(func))
#define EVT_RADIOBOX(winid, func) wx__DECLARE_EVT1(wxEVT_RADIOBOX, winid, wxCommandEventHandler(func))
#define EVT_RADIOBUTTON(winid, func) wx__DECLARE_EVT1(wxEVT_RADIOBUTTON, winid, wxCommandEventHandler(func))
#define EVT_SCROLLBAR(winid, func) wx__DECLARE_EVT1(wxEVT_SCROLLBAR, winid, wxCommandEventHandler(func))
#define EVT_VLBOX(winid, func) wx__DECLARE_EVT1(wxEVT_VLBOX, winid, wxCommandEventHandler(func))
#define EVT_COMBOBOX(winid, func) wx__DECLARE_EVT1(wxEVT_COMBOBOX, winid, wxCommandEventHandler(func))
#define EVT_TOOL(winid, func) wx__DECLARE_EVT1(wxEVT_TOOL, winid, wxCommandEventHandler(func))
#define EVT_TOOL_DROPDOWN(winid, func) wx__DECLARE_EVT1(wxEVT_TOOL_DROPDOWN, winid, wxCommandEventHandler(func))
#define EVT_TOOL_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_TOOL, id1, id2, wxCommandEventHandler(func))
#define EVT_TOOL_RCLICKED(winid, func) wx__DECLARE_EVT1(wxEVT_TOOL_RCLICKED, winid, wxCommandEventHandler(func))
#define EVT_TOOL_RCLICKED_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_TOOL_RCLICKED, id1, id2, wxCommandEventHandler(func))
#define EVT_TOOL_ENTER(winid, func) wx__DECLARE_EVT1(wxEVT_TOOL_ENTER, winid, wxCommandEventHandler(func))
#define EVT_CHECKLISTBOX(winid, func) wx__DECLARE_EVT1(wxEVT_CHECKLISTBOX, winid, wxCommandEventHandler(func))
#define EVT_COMBOBOX_DROPDOWN(winid, func) wx__DECLARE_EVT1(wxEVT_COMBOBOX_DROPDOWN, winid, wxCommandEventHandler(func))
#define EVT_COMBOBOX_CLOSEUP(winid, func) wx__DECLARE_EVT1(wxEVT_COMBOBOX_CLOSEUP, winid, wxCommandEventHandler(func))

#define EVT_COMMAND_LEFT_CLICK(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_LEFT_CLICK, winid, wxCommandEventHandler(func))
#define EVT_COMMAND_LEFT_DCLICK(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_LEFT_DCLICK, winid, wxCommandEventHandler(func))
#define EVT_COMMAND_RIGHT_CLICK(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_RIGHT_CLICK, winid, wxCommandEventHandler(func))
#define EVT_COMMAND_RIGHT_DCLICK(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_RIGHT_DCLICK, winid, wxCommandEventHandler(func))
#define EVT_COMMAND_SET_FOCUS(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_SET_FOCUS, winid, wxCommandEventHandler(func))
#define EVT_COMMAND_KILL_FOCUS(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_KILL_FOCUS, winid, wxCommandEventHandler(func))
#define EVT_COMMAND_ENTER(winid, func) wx__DECLARE_EVT1(wxEVT_COMMAND_ENTER, winid, wxCommandEventHandler(func))


#define EVT_JOY_BUTTON_DOWN(func) wx__DECLARE_EVT0(wxEVT_JOY_BUTTON_DOWN, wxJoystickEventHandler(func))
#define EVT_JOY_BUTTON_UP(func) wx__DECLARE_EVT0(wxEVT_JOY_BUTTON_UP, wxJoystickEventHandler(func))
#define EVT_JOY_MOVE(func) wx__DECLARE_EVT0(wxEVT_JOY_MOVE, wxJoystickEventHandler(func))
#define EVT_JOY_ZMOVE(func) wx__DECLARE_EVT0(wxEVT_JOY_ZMOVE, wxJoystickEventHandler(func))

#define EVT_JOYSTICK_EVENTS(func) \
    EVT_JOY_BUTTON_DOWN(func) \
    EVT_JOY_BUTTON_UP(func) \
    EVT_JOY_MOVE(func) \
    EVT_JOY_ZMOVE(func)

#define EVT_IDLE(func) wx__DECLARE_EVT0(wxEVT_IDLE, wxIdleEventHandler(func))

#define EVT_UPDATE_UI(winid, func) wx__DECLARE_EVT1(wxEVT_UPDATE_UI, winid, wxUpdateUIEventHandler(func))
#define EVT_UPDATE_UI_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_UPDATE_UI, id1, id2, wxUpdateUIEventHandler(func))

#define EVT_HELP(winid, func) wx__DECLARE_EVT1(wxEVT_HELP, winid, wxHelpEventHandler(func))
#define EVT_HELP_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_HELP, id1, id2, wxHelpEventHandler(func))
#define EVT_DETAILED_HELP(winid, func) wx__DECLARE_EVT1(wxEVT_DETAILED_HELP, winid, wxHelpEventHandler(func))
#define EVT_DETAILED_HELP_RANGE(id1, id2, func) wx__DECLARE_EVT2(wxEVT_DETAILED_HELP, id1, id2, wxHelpEventHandler(func))

#define EVT_CONTEXT_MENU(func) wx__DECLARE_EVT0(wxEVT_CONTEXT_MENU, wxContextMenuEventHandler(func))
#define EVT_COMMAND_CONTEXT_MENU(winid, func) wx__DECLARE_EVT1(wxEVT_CONTEXT_MENU, winid, wxContextMenuEventHandler(func))

#define EVT_TEXT_CUT(winid, func) wx__DECLARE_EVT1(wxEVT_TEXT_CUT, winid, wxClipboardTextEventHandler(func))
#define EVT_TEXT_COPY(winid, func) wx__DECLARE_EVT1(wxEVT_TEXT_COPY, winid, wxClipboardTextEventHandler(func))
#define EVT_TEXT_PASTE(winid, func) wx__DECLARE_EVT1(wxEVT_TEXT_PASTE, winid, wxClipboardTextEventHandler(func))

#define EVT_THREAD(id, func)  wx__DECLARE_EVT1(wxEVT_THREAD, id, wxThreadEventHandler(func))


#if wxUSE_GUI

WXDLLIMPEXP_CORE wxWindow* wxFindFocusDescendant(wxWindow* ancestor);

#endif 



#define DECLARE_EVENT_TABLE_ENTRY(type, winid, idLast, fn, obj) \
    wxDECLARE_EVENT_TABLE_ENTRY(type, winid, idLast, fn, obj)
#define DECLARE_EVENT_TABLE_TERMINATOR()               wxDECLARE_EVENT_TABLE_TERMINATOR()
#define DECLARE_EVENT_TABLE()                          wxDECLARE_EVENT_TABLE();
#define BEGIN_EVENT_TABLE(a,b)                         wxBEGIN_EVENT_TABLE(a,b)
#define BEGIN_EVENT_TABLE_TEMPLATE1(a,b,c)             wxBEGIN_EVENT_TABLE_TEMPLATE1(a,b,c)
#define BEGIN_EVENT_TABLE_TEMPLATE2(a,b,c,d)           wxBEGIN_EVENT_TABLE_TEMPLATE2(a,b,c,d)
#define BEGIN_EVENT_TABLE_TEMPLATE3(a,b,c,d,e)         wxBEGIN_EVENT_TABLE_TEMPLATE3(a,b,c,d,e)
#define BEGIN_EVENT_TABLE_TEMPLATE4(a,b,c,d,e,f)       wxBEGIN_EVENT_TABLE_TEMPLATE4(a,b,c,d,e,f)
#define BEGIN_EVENT_TABLE_TEMPLATE5(a,b,c,d,e,f,g)     wxBEGIN_EVENT_TABLE_TEMPLATE5(a,b,c,d,e,f,g)
#define BEGIN_EVENT_TABLE_TEMPLATE6(a,b,c,d,e,f,g,h)   wxBEGIN_EVENT_TABLE_TEMPLATE6(a,b,c,d,e,f,g,h)
#define END_EVENT_TABLE()                              wxEND_EVENT_TABLE()

#define BEGIN_DECLARE_EVENT_TYPES()
#define END_DECLARE_EVENT_TYPES()
#define DECLARE_EXPORTED_EVENT_TYPE(expdecl, name, value) \
    extern expdecl const wxEventType name;
#define DECLARE_EVENT_TYPE(name, value) \
    DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CORE, name, value)
#define DECLARE_LOCAL_EVENT_TYPE(name, value) \
    DECLARE_EXPORTED_EVENT_TYPE(wxEMPTY_PARAMETER_VALUE, name, value)
#define DEFINE_EVENT_TYPE(name) const wxEventType name = wxNewEventType();
#define DEFINE_LOCAL_EVENT_TYPE(name) DEFINE_EVENT_TYPE(name)

#define wxEVT_COMMAND_THREAD                  wxEVT_THREAD
#define wxEVT_COMMAND_BUTTON_CLICKED          wxEVT_BUTTON
#define wxEVT_COMMAND_CHECKBOX_CLICKED        wxEVT_CHECKBOX
#define wxEVT_COMMAND_CHOICE_SELECTED         wxEVT_CHOICE
#define wxEVT_COMMAND_LISTBOX_SELECTED        wxEVT_LISTBOX
#define wxEVT_COMMAND_LISTBOX_DOUBLECLICKED   wxEVT_LISTBOX_DCLICK
#define wxEVT_COMMAND_CHECKLISTBOX_TOGGLED    wxEVT_CHECKLISTBOX
#define wxEVT_COMMAND_MENU_SELECTED           wxEVT_MENU
#define wxEVT_COMMAND_TOOL_CLICKED            wxEVT_TOOL
#define wxEVT_COMMAND_SLIDER_UPDATED          wxEVT_SLIDER
#define wxEVT_COMMAND_RADIOBOX_SELECTED       wxEVT_RADIOBOX
#define wxEVT_COMMAND_RADIOBUTTON_SELECTED    wxEVT_RADIOBUTTON
#define wxEVT_COMMAND_SCROLLBAR_UPDATED       wxEVT_SCROLLBAR
#define wxEVT_COMMAND_VLBOX_SELECTED          wxEVT_VLBOX
#define wxEVT_COMMAND_COMBOBOX_SELECTED       wxEVT_COMBOBOX
#define wxEVT_COMMAND_TOOL_RCLICKED           wxEVT_TOOL_RCLICKED
#define wxEVT_COMMAND_TOOL_DROPDOWN_CLICKED   wxEVT_TOOL_DROPDOWN
#define wxEVT_COMMAND_TOOL_ENTER              wxEVT_TOOL_ENTER
#define wxEVT_COMMAND_COMBOBOX_DROPDOWN       wxEVT_COMBOBOX_DROPDOWN
#define wxEVT_COMMAND_COMBOBOX_CLOSEUP        wxEVT_COMBOBOX_CLOSEUP
#define wxEVT_COMMAND_TEXT_COPY               wxEVT_TEXT_COPY
#define wxEVT_COMMAND_TEXT_CUT                wxEVT_TEXT_CUT
#define wxEVT_COMMAND_TEXT_PASTE              wxEVT_TEXT_PASTE
#define wxEVT_COMMAND_TEXT_UPDATED            wxEVT_TEXT

#endif 