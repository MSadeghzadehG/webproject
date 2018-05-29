


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_STACKWALKER

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/app.h"
    #include "wx/log.h"
    #include "wx/utils.h"
#endif

#include "wx/stackwalk.h"
#include "wx/stdpaths.h"

#include <execinfo.h>

#ifdef HAVE_CXA_DEMANGLE
    #include <cxxabi.h>
#endif 

class wxStdioPipe
{
public:
        wxStdioPipe(const char *command, const char *type)
    {
        m_fp = popen(command, type);
    }

        operator FILE *() const { return m_fp; }

        ~wxStdioPipe()
    {
        if ( m_fp )
            pclose(m_fp);
    }

private:
    FILE *m_fp;

    wxDECLARE_NO_COPY_CLASS(wxStdioPipe);
};



void wxStackFrame::OnGetName()
{
    if ( !m_name.empty() )
        return;

                    
            wxString syminfo = wxString::FromAscii(m_syminfo);
    const size_t posOpen = syminfo.find(wxT('('));
    if ( posOpen != wxString::npos )
    {
        const size_t posPlus = syminfo.find(wxT('+'), posOpen + 1);
        if ( posPlus != wxString::npos )
        {
            const size_t posClose = syminfo.find(wxT(')'), posPlus + 1);
            if ( posClose != wxString::npos )
            {
                if ( m_name.empty() )
                {
                    m_name.assign(syminfo, posOpen + 1, posPlus - posOpen - 1);

#ifdef HAVE_CXA_DEMANGLE
                    int rc = -1;
                    char *cppfunc = __cxxabiv1::__cxa_demangle
                                    (
                                        m_name.mb_str(),
                                        NULL,                                         NULL,                                         &rc
                                    );
                    if ( rc == 0 )
                        m_name = wxString::FromAscii(cppfunc);

                    free(cppfunc);
#endif                 }

                unsigned long ofs;
                if ( wxString(syminfo, posPlus + 1, posClose - posPlus - 1).
                        ToULong(&ofs, 0) )
                    m_offset = ofs;
            }
        }

        m_module.assign(syminfo, posOpen);
    }
#ifndef __WXOSX__
    else     {
        m_module = syminfo;
    }
#endif }



#define MAX_FRAMES          200

#define CHARS_PER_FRAME    (sizeof(void*) * 2 + 3)

#define BUFSIZE            (50 + MAX_FRAMES*CHARS_PER_FRAME)

void *wxStackWalker::ms_addresses[MAX_FRAMES];
char **wxStackWalker::ms_symbols = NULL;
int wxStackWalker::m_depth = 0;
wxString wxStackWalker::ms_exepath;
static char g_buf[BUFSIZE];


void wxStackWalker::SaveStack(size_t maxDepth)
{
        maxDepth = wxMin(WXSIZEOF(ms_addresses)/sizeof(void*), maxDepth);
    m_depth = backtrace(ms_addresses, maxDepth*sizeof(void*));
    if ( !m_depth )
        return;

    ms_symbols = backtrace_symbols(ms_addresses, m_depth);
}

void wxStackWalker::ProcessFrames(size_t skip)
{
    wxStackFrame frames[MAX_FRAMES];

    if (!ms_symbols || !m_depth)
        return;

            skip += 1;

                int numFrames = InitFrames(frames, m_depth - skip,
                               &ms_addresses[skip], &ms_symbols[skip]);

        for ( int n = 0; n < numFrames; n++ )
        OnStackFrame(frames[n]);
}

void wxStackWalker::FreeStack()
{
            if (ms_symbols)
        free( ms_symbols );
    ms_symbols = NULL;
    m_depth = 0;
}

namespace
{

bool ReadLine(FILE* fp, unsigned long num, wxString* line)
{
    if ( !fgets(g_buf, WXSIZEOF(g_buf), fp) )
    {
        wxUnusedVar(num); 
        wxLogDebug(wxS("cannot read address information for stack frame #%lu"),
                   num);
        return false;
    }

    *line = wxString::FromAscii(g_buf);
    line->RemoveLast();

    return true;
}

} 
int wxStackWalker::InitFrames(wxStackFrame *arr, size_t n, void **addresses, char **syminfo)
{
            wxString exepath = wxStackWalker::GetExePath();
    if ( exepath.empty() )
    {
        exepath = wxStandardPaths::Get().GetExecutablePath();
        if ( exepath.empty() )
        {
            wxLogDebug(wxT("Cannot parse stack frame because the executable ")
                       wxT("path could not be detected"));
            return 0;
        }
    }

        #ifdef __WXOSX__
    int len = snprintf(g_buf, BUFSIZE, "atos -p %d", (int)getpid());
#else
    int len = snprintf(g_buf, BUFSIZE, "addr2line -C -f -e \"%s\"", (const char*) exepath.mb_str());
#endif
    len = (len <= 0) ? strlen(g_buf) : len;         for (size_t i=0; i<n; i++)
    {
        snprintf(&g_buf[len], BUFSIZE - len, " %p", addresses[i]);
        len = strlen(g_buf);
    }

    
    wxStdioPipe fp(g_buf, "r");
    if ( !fp )
        return 0;

            wxString name, filename;
    unsigned long line = 0,
                  curr = 0;
    for  ( size_t i = 0; i < n; i++ )
    {
#ifdef __WXOSX__
        wxString buffer;
        if ( !ReadLine(fp, i, &buffer) )
            return false;

        line = 0;
        filename.clear();

                                                const size_t posIn = buffer.find("(in ");
        if ( posIn != wxString::npos )
        {
            name.assign(buffer, 0, posIn);

            size_t posAt = buffer.find(") (", posIn + 3);
            if ( posAt != wxString::npos )
            {
                posAt += 3; 
                                wxString location(buffer, posAt, buffer.length() - posAt - 2);

                wxString linenum;
                filename = location.BeforeFirst(':', &linenum);
                if ( !linenum.empty() )
                    linenum.ToULong(&line);
            }
        }
#else                 if ( !ReadLine(fp, i, &name) )
            return false;

        name = wxString::FromAscii(g_buf);
        name.RemoveLast(); 
        if ( name == wxT("??") )
            name.clear();

                if ( !ReadLine(fp, i, &filename) )
            return false;

        const size_t posColon = filename.find(wxT(':'));
        if ( posColon != wxString::npos )
        {
                                    wxString(filename, posColon + 1, wxString::npos).ToULong(&line);

                        filename.erase(posColon);
            if ( filename == wxT("??") )
                filename.clear();
        }
        else
        {
            wxLogDebug(wxT("Unexpected addr2line format: \"%s\" - ")
                       wxT("the semicolon is missing"),
                       filename.c_str());
        }
#endif 
                                arr[curr++].Set(name, filename, syminfo[i], i, line, addresses[i]);
    }

    return curr;
}

void wxStackWalker::Walk(size_t skip, size_t maxDepth)
{
        SaveStack(maxDepth);

        ProcessFrames(skip);

        FreeStack();
}

#endif 