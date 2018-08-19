


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_FONTENUM

#include "wx/fontenum.h"


class wxSimpleFontEnumerator : public wxFontEnumerator
{
public:
    wxSimpleFontEnumerator() { }

        virtual bool OnFacename(const wxString& facename) wxOVERRIDE
    {
        m_arrFacenames.Add(facename);
        return true;
    }

        virtual bool OnFontEncoding(const wxString& WXUNUSED(facename),
                                const wxString& encoding) wxOVERRIDE
    {
        m_arrEncodings.Add(encoding);
        return true;
    }

public:
    wxArrayString m_arrFacenames, m_arrEncodings;
};



wxArrayString wxFontEnumerator::GetFacenames(wxFontEncoding encoding, bool fixedWidthOnly)
{
    wxSimpleFontEnumerator temp;
    temp.EnumerateFacenames(encoding, fixedWidthOnly);
    return temp.m_arrFacenames;
}


wxArrayString wxFontEnumerator::GetEncodings(const wxString& facename)
{
    wxSimpleFontEnumerator temp;
    temp.EnumerateEncodings(facename);
    return temp.m_arrEncodings;
}


bool wxFontEnumerator::IsValidFacename(const wxString &facename)
{
            static wxArrayString s_arr = wxFontEnumerator::GetFacenames();

#ifdef __WXMSW__
                                if (facename.IsSameAs(wxT("Ms Shell Dlg"), false) ||
        facename.IsSameAs(wxT("Ms Shell Dlg 2"), false))
        return true;
#endif

        if (s_arr.Index(facename, false) == wxNOT_FOUND)
        return false;

    return true;
}

#ifdef wxHAS_UTF8_FONTS
bool wxFontEnumerator::EnumerateEncodingsUTF8(const wxString& facename)
{
            const wxString utf8(wxT("UTF-8"));

        if ( !facename.empty() )
    {
        OnFontEncoding(facename, utf8);
        return true;
    }

            const wxArrayString facenames(GetFacenames(wxFONTENCODING_UTF8));
    const size_t count = facenames.size();
    if ( !count )
        return false;

    for ( size_t n = 0; n < count; n++ )
    {
        OnFontEncoding(facenames[n], utf8);
    }

    return true;
}
#endif 
#endif 