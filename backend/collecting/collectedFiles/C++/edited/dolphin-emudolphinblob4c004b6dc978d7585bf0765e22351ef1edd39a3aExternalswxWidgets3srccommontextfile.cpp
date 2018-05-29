

#include  "wx/wxprec.h"

#ifdef    __BORLANDC__
    #pragma hdrstop
#endif  
#if !wxUSE_FILE || !wxUSE_TEXTBUFFER
    #undef wxUSE_TEXTFILE
    #define wxUSE_TEXTFILE 0
#endif 
#if wxUSE_TEXTFILE

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/intl.h"
    #include "wx/file.h"
    #include "wx/log.h"
#endif

#include "wx/textfile.h"
#include "wx/filename.h"
#include "wx/buffer.h"


wxTextFile::wxTextFile(const wxString& strFileName)
          : wxTextBuffer(strFileName)
{
}



bool wxTextFile::OnExists() const
{
    return wxFile::Exists(m_strBufferName);
}


bool wxTextFile::OnOpen(const wxString &strBufferName, wxTextBufferOpenMode openMode)
{
    wxFile::OpenMode fileOpenMode = wxFile::read_write;

    switch ( openMode )
    {
        case ReadAccess:
            fileOpenMode = wxFile::read;
            break;

        case WriteAccess:
            fileOpenMode = wxFile::write;
            break;
    }

    if ( fileOpenMode == wxFile::read_write )
    {
                wxFAIL_MSG( wxT("unknown open mode in wxTextFile::Open") );
        return false;
    }

    return m_file.Open(strBufferName, fileOpenMode);
}


bool wxTextFile::OnClose()
{
    return m_file.Close();
}


bool wxTextFile::OnRead(const wxMBConv& conv)
{
        wxASSERT_MSG( m_file.IsOpened(), wxT("can't read closed file") );

                                size_t bufSize = 0;

        static const size_t BLOCK_SIZE = 4096;

    wxCharBuffer buf;

            wxFileOffset fileLength;
    {
        wxLogNull logNull;
        fileLength = m_file.Length();
    }

            const bool seekable = fileLength != wxInvalidOffset && fileLength != 0;
    if ( seekable )
    {
                bufSize = fileLength;
        if ( !buf.extend(bufSize) )
            return false;

                wxASSERT_MSG( m_file.Tell() == 0, wxT("should be at start of file") );

        char *dst = buf.data();
        for ( size_t nRemaining = bufSize; nRemaining > 0; )
        {
            size_t nToRead = BLOCK_SIZE;

                                    if ( nToRead > nRemaining )
                nToRead = nRemaining;

            ssize_t nRead = m_file.Read(dst, nToRead);

            if ( nRead == wxInvalidOffset )
            {
                                return false;
            }

            if ( nRead == 0 )
            {
                                                break;
            }

            dst += nRead;
            nRemaining -= nRead;
        }

        wxASSERT_MSG( dst - buf.data() == (wxFileOffset)bufSize,
                      wxT("logic error") );
    }
    else     {
        char block[BLOCK_SIZE];
        for ( ;; )
        {
            ssize_t nRead = m_file.Read(block, WXSIZEOF(block));

            if ( nRead == wxInvalidOffset )
            {
                                return false;
            }

            if ( nRead == 0 )
            {
                                                if ( bufSize == 0 )
                    return true;

                                break;
            }

                        if ( !buf.extend(bufSize + nRead) )
                return false;

                        memcpy(buf.data() + bufSize, block, nRead);
            bufSize += nRead;
        }
    }

    const wxString str(buf, conv, bufSize);

    #if wxUSE_UNICODE
    if ( bufSize > 4 && str.empty() )
    {
        wxLogError(_("Failed to convert file \"%s\" to Unicode."), GetName());
        return false;
    }
#endif 
        buf.reset();


    
        wxString::const_iterator lineStart = str.begin();
    const wxString::const_iterator end = str.end();
    for ( wxString::const_iterator p = lineStart; p != end; p++ )
    {
        const wxChar ch = *p;
        if ( ch == '\r' || ch == '\n' )
        {
                        wxTextFileType lineType = wxTextFileType_None;
            if ( ch == '\r' )
            {
                wxString::const_iterator next = p + 1;
                if ( next != end && *next == '\n' )
                    lineType = wxTextFileType_Dos;
                else
                    lineType = wxTextFileType_Mac;
            }
            else             {
                lineType = wxTextFileType_Unix;
            }

            AddLine(wxString(lineStart, p), lineType);

                        if ( lineType == wxTextFileType_Dos )
                p++;

            lineStart = p + 1;
        }
    }

        if ( lineStart != end )
    {
                        wxString lastLine(lineStart, end);
        AddLine(lastLine, wxTextFileType_None);
    }

    return true;
}


bool wxTextFile::OnWrite(wxTextFileType typeNew, const wxMBConv& conv)
{
    wxFileName fn = m_strBufferName;

            if ( !fn.IsAbsolute() )
        fn.Normalize(wxPATH_NORM_ENV_VARS | wxPATH_NORM_DOTS | wxPATH_NORM_TILDE |
                     wxPATH_NORM_ABSOLUTE | wxPATH_NORM_LONG);

    wxTempFile fileTmp(fn.GetFullPath());

    if ( !fileTmp.IsOpened() ) {
        wxLogError(_("can't write buffer '%s' to disk."), m_strBufferName);
        return false;
    }

    size_t nCount = GetLineCount();
    for ( size_t n = 0; n < nCount; n++ ) {
        fileTmp.Write(GetLine(n) +
                      GetEOL(typeNew == wxTextFileType_None ? GetLineType(n)
                                                            : typeNew),
                      conv);
    }

        return fileTmp.Commit();
}

#endif 