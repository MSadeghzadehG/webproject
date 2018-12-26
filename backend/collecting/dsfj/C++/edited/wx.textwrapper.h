
#ifndef _WX_TEXTWRAPPER_H_
#define _WX_TEXTWRAPPER_H_

#include "wx/window.h"


class WXDLLIMPEXP_CORE wxTextWrapper
{
public:
    wxTextWrapper() { m_eol = false; }

            void Wrap(wxWindow *win, const wxString& text, int widthMax);

        virtual ~wxTextWrapper() { }

protected:
        virtual void OnOutputLine(const wxString& line) = 0;

        virtual void OnNewLine() { }

private:
        void DoOutputLine(const wxString& line)
    {
        OnOutputLine(line);

        m_eol = true;
    }

                bool IsStartOfNewLine()
    {
        if ( !m_eol )
            return false;

        m_eol = false;

        return true;
    }


    bool m_eol;

    wxDECLARE_NO_COPY_CLASS(wxTextWrapper);
};

#if wxUSE_STATTEXT

#include "wx/sizer.h"
#include "wx/stattext.h"

class wxTextSizerWrapper : public wxTextWrapper
{
public:
    wxTextSizerWrapper(wxWindow *win)
    {
        m_win = win;
        m_hLine = 0;
    }

    wxSizer *CreateSizer(const wxString& text, int widthMax)
    {
        m_sizer = new wxBoxSizer(wxVERTICAL);
        Wrap(m_win, text, widthMax);
        return m_sizer;
    }

    wxWindow *GetParent() const { return m_win; }

protected:
    virtual wxWindow *OnCreateLine(const wxString& line)
    {
        return new wxStaticText(m_win, wxID_ANY,
                                wxControl::EscapeMnemonics(line));
    }

    virtual void OnOutputLine(const wxString& line) wxOVERRIDE
    {
        if ( !line.empty() )
        {
            m_sizer->Add(OnCreateLine(line));
        }
        else         {
            if ( !m_hLine )
                m_hLine = m_win->GetCharHeight();

            m_sizer->Add(5, m_hLine);
        }
    }

private:
    wxWindow *m_win;
    wxSizer *m_sizer;
    int m_hLine;
};

#endif 
#endif 
