


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/crt.h"
#endif

#include "wx/uri.h"


wxIMPLEMENT_CLASS(wxURI, wxObject);



wxURI::wxURI()
     : m_hostType(wxURI_REGNAME),
       m_fields(0)
{
}

wxURI::wxURI(const wxString& uri)
     : m_hostType(wxURI_REGNAME),
       m_fields(0)
{
    Create(uri);
}

bool wxURI::Create(const wxString& uri)
{
    if (m_fields)
        Clear();

    return Parse(uri.utf8_str());
}

void wxURI::Clear()
{
    m_scheme =
    m_userinfo =
    m_server =
    m_port =
    m_path =
    m_query =
    m_fragment = wxEmptyString;

    m_hostType = wxURI_REGNAME;

    m_fields = 0;
}




int wxURI::CharToHex(char c)
{
    if ((c >= 'A') && (c <= 'F'))
        return c - 'A' + 10;
    if ((c >= 'a') && (c <= 'f'))
        return c - 'a' + 10;
    if ((c >= '0') && (c <= '9'))
        return c - '0';

    return -1;
}


wxString wxURI::Unescape(const wxString& uri)
{
                                    const wxScopedCharBuffer& uriU8(uri.utf8_str());
    const size_t len = uriU8.length();

        wxCharBuffer buf(uriU8.length());
    char *p = buf.data();

    const char* const end = uriU8.data() + len;
    for ( const char* s = uriU8.data(); s != end; ++s, ++p )
    {
        char c = *s;
        if ( c == '%' && s < end - 2 && IsHex(s[1]) && IsHex(s[2]) )
        {
            c = (CharToHex(s[1]) << 4) | CharToHex(s[2]);
            s += 2;
        }

        *p = c;
    }

    *p = '\0';

    return wxString::FromUTF8(buf);
}

void wxURI::AppendNextEscaped(wxString& s, const char *& p)
{
                if ( p[0] == '%' && IsHex(p[1]) && IsHex(p[2]) )
    {
        s += *p++;
        s += *p++;
        s += *p++;
    }
    else     {
        static const char* hexDigits = "0123456789abcdef";

        const char c = *p++;

        s += '%';
        s += hexDigits[(c >> 4) & 15];
        s += hexDigits[c & 15];
    }
}

wxString wxURI::GetUser() const
{
            return m_userinfo(0, m_userinfo.find(':'));
}

wxString wxURI::GetPassword() const
{
      size_t posColon = m_userinfo.find(':');

      if ( posColon == wxString::npos )
          return "";

      return m_userinfo(posColon + 1, wxString::npos);
}

wxString wxURI::DoBuildURI(wxString (*funcDecode)(const wxString&)) const
{
    wxString ret;

    if (HasScheme())
        ret += m_scheme + ":";

    if (HasServer())
    {
        ret += "
        if (HasUserInfo())
            ret += funcDecode(m_userinfo) + "@";

        if (m_hostType == wxURI_REGNAME)
            ret += funcDecode(m_server);
        else
            ret += m_server;

        if (HasPort())
            ret += ":" + m_port;
    }

    ret += funcDecode(m_path);

    if (HasQuery())
        ret += "?" + funcDecode(m_query);

    if (HasFragment())
        ret += "#" + funcDecode(m_fragment);

    return ret;
}


bool wxURI::operator==(const wxURI& uri) const
{
    if (HasScheme())
    {
        if(m_scheme != uri.m_scheme)
            return false;
    }
    else if (uri.HasScheme())
        return false;


    if (HasServer())
    {
        if (HasUserInfo())
        {
            if (m_userinfo != uri.m_userinfo)
                return false;
        }
        else if (uri.HasUserInfo())
            return false;

        if (m_server != uri.m_server ||
            m_hostType != uri.m_hostType)
            return false;

        if (HasPort())
        {
            if(m_port != uri.m_port)
                return false;
        }
        else if (uri.HasPort())
            return false;
    }
    else if (uri.HasServer())
        return false;


    if (HasPath())
    {
        if(m_path != uri.m_path)
            return false;
    }
    else if (uri.HasPath())
        return false;

    if (HasQuery())
    {
        if (m_query != uri.m_query)
            return false;
    }
    else if (uri.HasQuery())
        return false;

    if (HasFragment())
    {
        if (m_fragment != uri.m_fragment)
            return false;
    }
    else if (uri.HasFragment())
        return false;

    return true;
}


bool wxURI::IsReference() const
{
    return !HasScheme() || !HasServer();
}


bool wxURI::IsRelative() const
{
    return !HasScheme() && !HasServer();
}


bool wxURI::Parse(const char *uri)
{
    uri = ParseScheme(uri);
    if ( uri )
        uri = ParseAuthority(uri);
    if ( uri )
        uri = ParsePath(uri);
    if ( uri )
        uri = ParseQuery(uri);
    if ( uri )
        uri = ParseFragment(uri);

        return uri && *uri == '\0';
}

const char* wxURI::ParseScheme(const char *uri)
{
    const char * const start = uri;

        if ( IsAlpha(*uri) )
    {
        m_scheme += *uri++;

                while (IsAlpha(*uri) || IsDigit(*uri) ||
               *uri == '+'   ||
               *uri == '-'   ||
               *uri == '.')
        {
            m_scheme += *uri++;
        }

                if (*uri == ':')
        {
                        m_fields |= wxURI_SCHEME;

                        ++uri;
        }
        else         {
            uri = start;             m_scheme.clear();
        }
    }
    
    return uri;
}

const char* wxURI::ParseAuthority(const char* uri)
{
        if ( uri[0] == '/' && uri[1] == '/' )
    {
                uri += 2;

                        if(m_scheme != "file")
        {
                        uri = ParseUserInfo(uri);
            uri = ParseServer(uri);
            return ParsePort(uri);
        }
    }

    return uri;
}

const char* wxURI::ParseUserInfo(const char* uri)
{
    const char * const start = uri;

        while ( *uri && *uri != '@' && *uri != '/' && *uri != '#' && *uri != '?' )
    {
        if ( IsUnreserved(*uri) || IsSubDelim(*uri) || *uri == ':' )
            m_userinfo += *uri++;
        else
            AppendNextEscaped(m_userinfo, uri);
    }

    if ( *uri++ == '@' )
    {
                m_fields |= wxURI_USERINFO;
    }
    else
    {
        uri = start;         m_userinfo.clear();
    }

    return uri;
}

const char* wxURI::ParseServer(const char* uri)
{
    const char * const start = uri;

            if (*uri == '[')
    {
        ++uri;
        if (ParseIPv6address(uri) && *uri == ']')
        {
            m_hostType = wxURI_IPV6ADDRESS;

            m_server.assign(start + 1, uri - start - 1);
            ++uri;
        }
        else
        {
            uri = start + 1; 
            if (ParseIPvFuture(uri) && *uri == ']')
            {
                m_hostType = wxURI_IPVFUTURE;

                m_server.assign(start + 1, uri - start - 1);
                ++uri;
            }
            else             {
                uri = start;
            }
        }
    }
    else     {
        if (ParseIPv4address(uri))
        {
            m_hostType = wxURI_IPV4ADDRESS;

            m_server.assign(start, uri - start);
        }
        else
        {
            uri = start;
        }
    }

    if ( m_hostType == wxURI_REGNAME )
    {
        uri = start;
                while ( *uri && *uri != '/' && *uri != ':' && *uri != '#' && *uri != '?' )
        {
            if ( IsUnreserved(*uri) || IsSubDelim(*uri) )
                m_server += *uri++;
            else
                AppendNextEscaped(m_server, uri);
        }
    }

    m_fields |= wxURI_SERVER;

    return uri;
}


const char* wxURI::ParsePort(const char* uri)
{
        if( *uri == ':' )
    {
        ++uri;
        while ( IsDigit(*uri) )
        {
            m_port += *uri++;
        }

        m_fields |= wxURI_PORT;
    }

    return uri;
}

const char* wxURI::ParsePath(const char* uri)
{
                                                                                        
    if ( IsEndPath(*uri) )
        return uri;

    const bool isAbs = *uri == '/';
    if ( isAbs )
        m_path += *uri++;

    wxArrayString segments;
    wxString segment;
    for ( ;; )
    {
        const bool endPath = IsEndPath(*uri);
        if ( endPath || *uri == '/' )
        {
                        if ( segment == ".." )
            {
                if ( !segments.empty() && *segments.rbegin() != ".." )
                    segments.pop_back();
                else if ( !isAbs )
                    segments.push_back("..");
            }
            else if ( segment == "." )
            {
                                                if ( endPath )
                    segments.push_back("");
            }
            else             {
                segments.push_back(segment);
            }

            if ( endPath )
                break;

            segment.clear();
            ++uri;
            continue;
        }

        if ( IsUnreserved(*uri) || IsSubDelim(*uri) || *uri == ':' || *uri == '@' )
            segment += *uri++;
        else
            AppendNextEscaped(segment, uri);
    }

    m_path += wxJoin(segments, '/', '\0');
    m_fields |= wxURI_PATH;

    return uri;
}


const char* wxURI::ParseQuery(const char* uri)
{
        if ( *uri == '?' )
    {
        ++uri;
        while ( *uri && *uri != '#' )
        {
            if ( IsUnreserved(*uri) || IsSubDelim(*uri) ||
                    *uri == ':' || *uri == '@' || *uri == '/' || *uri == '?' )
                m_query += *uri++;
            else
                AppendNextEscaped(m_query, uri);
        }

        m_fields |= wxURI_QUERY;
    }

    return uri;
}


const char* wxURI::ParseFragment(const char* uri)
{
        if ( *uri == '#' )
    {
        ++uri;
        while ( *uri )
        {
            if ( IsUnreserved(*uri) || IsSubDelim(*uri) ||
                    *uri == ':' || *uri == '@' || *uri == '/' || *uri == '?')
                m_fragment += *uri++;
            else
                AppendNextEscaped(m_fragment, uri);
        }

        m_fields |= wxURI_FRAGMENT;
    }

    return uri;
}



wxArrayString wxURI::SplitInSegments(const wxString& path)
{
    return wxSplit(path, '/', '\0' );
}

void wxURI::Resolve(const wxURI& base, int flags)
{
    wxASSERT_MSG(!base.IsReference(),
                "wxURI to inherit from must not be a reference!");

                if ( !(flags & wxURI_STRICT) &&
            HasScheme() && base.HasScheme() &&
                m_scheme == base.m_scheme )
    {
        m_fields -= wxURI_SCHEME;
    }


                            if (HasScheme())
        return;

        m_scheme = base.m_scheme;
    m_fields |= wxURI_SCHEME;

                            if (HasServer())
        return;

        if (base.HasUserInfo())
    {
        m_userinfo = base.m_userinfo;
        m_fields |= wxURI_USERINFO;
    }

    m_server = base.m_server;
    m_hostType = base.m_hostType;
    m_fields |= wxURI_SERVER;

    if (base.HasPort())
    {
        m_port = base.m_port;
        m_fields |= wxURI_PORT;
    }


        if (!HasPath())
    {
                m_path = base.m_path;
        m_fields |= wxURI_PATH;


                                                if (!HasQuery())
        {
            m_query = base.m_query;
            m_fields |= wxURI_QUERY;
        }
    }
    else if ( m_path.empty() || m_path[0u] != '/' )
    {
                                                                                
        wxArrayString our(SplitInSegments(m_path)),
                      result(SplitInSegments(base.m_path));

        if ( !result.empty() )
            result.pop_back();

        if ( our.empty() )
        {
                                                our.push_back("");
        }

        const wxArrayString::const_iterator end = our.end();
        for ( wxArrayString::const_iterator i = our.begin(); i != end; ++i )
        {
            if ( i->empty() || *i == "." )
            {
                                                if ( i == end - 1 )
                    result.push_back("");
                continue;
            }

            if ( *i == ".." )
            {
                if ( !result.empty() )
                {
                    result.pop_back();

                    if ( i == end - 1 )
                        result.push_back("");
                }
                            }
            else
            {
                if ( result.empty() )
                {
                                        result.push_back("");
                }

                result.push_back(*i);
            }
        }

        m_path = wxJoin(result, '/', '\0');
    }

    }


bool wxURI::ParseH16(const char*& uri)
{
        if(!IsHex(*++uri))
        return false;

    if(IsHex(*++uri) && IsHex(*++uri) && IsHex(*++uri))
        ++uri;

    return true;
}


bool wxURI::ParseIPv4address(const char*& uri)
{
                                size_t iIPv4 = 0;
    if (IsDigit(*uri))
    {
        ++iIPv4;


                if( IsDigit(*++uri) && IsDigit(*++uri) &&
                      !( (*(uri-2) < '2') ||
                        (*(uri-2) == '2' &&
               (*(uri-1) < '5' || (*(uri-1) == '5' && *uri <= '5'))
             )
            )
          )
        {
            return false;
        }

        if(IsDigit(*uri))++uri;

                for(; iIPv4 < 4; ++iIPv4)
        {
            if (*uri != '.' || !IsDigit(*++uri))
                break;

                        if( IsDigit(*++uri) && IsDigit(*++uri) &&
                              !( (*(uri-2) < '2') ||
                                (*(uri-2) == '2' &&
                   (*(uri-1) < '5' || (*(uri-1) == '5' && *uri <= '5'))
                 )
                )
              )
            {
                return false;
            }
            if(IsDigit(*uri))++uri;
        }
    }
    return iIPv4 == 4;
}

bool wxURI::ParseIPv6address(const char*& uri)
{
                                    
    size_t numPrefix = 0,
              maxPostfix;

    bool bEndHex = false;

    for( ; numPrefix < 6; ++numPrefix)
    {
        if(!ParseH16(uri))
        {
            --uri;
            bEndHex = true;
            break;
        }

        if(*uri != ':')
        {
            break;
        }
    }

    if(!bEndHex && !ParseH16(uri))
    {
        --uri;

        if (numPrefix)
            return false;

        if (*uri == ':')
        {
            if (*++uri != ':')
                return false;

            maxPostfix = 5;
        }
        else
            maxPostfix = 6;
    }
    else
    {
        if (*uri != ':' || *(uri+1) != ':')
        {
            if (numPrefix != 6)
                return false;

            while (*--uri != ':') {}
            ++uri;

            const char * const start = uri;
                                    if (ParseH16(uri) && *uri == ':' && ParseH16(uri))
                return true;

            uri = start;

            if (ParseIPv4address(uri))
                return true;
            else
                return false;
        }
        else
        {
            uri += 2;

            if (numPrefix > 3)
                maxPostfix = 0;
            else
                maxPostfix = 4 - numPrefix;
        }
    }

    bool bAllowAltEnding = maxPostfix == 0;

    for(; maxPostfix != 0; --maxPostfix)
    {
        if(!ParseH16(uri) || *uri != ':')
            return false;
    }

    if(numPrefix <= 4)
    {
        const char * const start = uri;
                        if (ParseH16(uri) && *uri == ':' && ParseH16(uri))
            return true;

        uri = start;

        if (ParseIPv4address(uri))
            return true;

        uri = start;

        if (!bAllowAltEnding)
            return false;
    }

    if(numPrefix <= 5 && ParseH16(uri))
        return true;

    return true;
}

bool wxURI::ParseIPvFuture(const char*& uri)
{
        if (*++uri != 'v' || !IsHex(*++uri))
        return false;

    while (IsHex(*++uri))
        ;

    if (*uri != '.' || !(IsUnreserved(*++uri) || IsSubDelim(*uri) || *uri == ':'))
        return false;

    while(IsUnreserved(*++uri) || IsSubDelim(*uri) || *uri == ':') {}

    return true;
}



bool wxURI::IsUnreserved(char c)
{
    return IsAlpha(c) ||
           IsDigit(c) ||
           c == '-' ||
           c == '.' ||
           c == '_' ||
           c == '~'
           ;
}

bool wxURI::IsReserved(char c)
{
    return IsGenDelim(c) || IsSubDelim(c);
}

bool wxURI::IsGenDelim(char c)
{
    return c == ':' ||
           c == '/' ||
           c == '?' ||
           c == '#' ||
           c == '[' ||
           c == ']' ||
           c == '@';
}

bool wxURI::IsSubDelim(char c)
{
    return c == '!' ||
           c == '$' ||
           c == '&' ||
           c == '\'' ||
           c == '(' ||
           c == ')' ||
           c == '*' ||
           c == '+' ||
           c == ',' ||
           c == ';' ||
           c == '='
           ;
}

bool wxURI::IsHex(char c)
{
    return IsDigit(c) ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

bool wxURI::IsAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool wxURI::IsDigit(char c)
{
    return c >= '0' && c <= '9';
}

bool wxURI::IsEndPath(char c)
{
    return c == '\0' || c == '#' || c == '?';
}

