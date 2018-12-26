
#ifndef _WX_UNIX_PIPE_H_
#define _WX_UNIX_PIPE_H_

#include <unistd.h>
#include <fcntl.h>

#include "wx/log.h"
#include "wx/intl.h"


class wxPipe
{
public:
        enum Direction
    {
        Read,
        Write
    };

    enum
    {
        INVALID_FD = -1
    };

        wxPipe() { m_fds[Read] = m_fds[Write] = INVALID_FD; }

        bool Create()
    {
        if ( pipe(m_fds) == -1 )
        {
            wxLogSysError(wxGetTranslation("Pipe creation failed"));

            return false;
        }

        return true;
    }

        bool MakeNonBlocking(Direction which)
    {
        const int flags = fcntl(m_fds[which], F_GETFL, 0);
        if ( flags == -1 )
            return false;

        return fcntl(m_fds[which], F_SETFL, flags | O_NONBLOCK) == 0;
    }

        bool IsOk() const { return m_fds[Read] != INVALID_FD; }

        int operator[](Direction which) const { return m_fds[which]; }

            int Detach(Direction which)
    {
        int fd = m_fds[which];
        m_fds[which] = INVALID_FD;

        return fd;
    }

        void Close()
    {
        for ( size_t n = 0; n < WXSIZEOF(m_fds); n++ )
        {
            if ( m_fds[n] != INVALID_FD )
            {
                close(m_fds[n]);
                m_fds[n] = INVALID_FD;
            }
        }
    }

        ~wxPipe() { Close(); }

private:
    int m_fds[2];
};

#endif 
