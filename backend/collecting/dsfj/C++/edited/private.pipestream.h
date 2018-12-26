
#ifndef _WX_MSW_PRIVATE_PIPESTREAM_H_
#define _WX_MSW_PRIVATE_PIPESTREAM_H_

class wxPipeInputStream : public wxInputStream
{
public:
    wxEXPLICIT wxPipeInputStream(HANDLE hInput);
    virtual ~wxPipeInputStream();

        bool IsOpened() const { return m_hInput != INVALID_HANDLE_VALUE; }

        virtual bool CanRead() const;

protected:
    virtual size_t OnSysRead(void *buffer, size_t len);

protected:
    HANDLE m_hInput;

    wxDECLARE_NO_COPY_CLASS(wxPipeInputStream);
};

class wxPipeOutputStream: public wxOutputStream
{
public:
    wxEXPLICIT wxPipeOutputStream(HANDLE hOutput);
    virtual ~wxPipeOutputStream() { Close(); }
    bool Close();

protected:
    size_t OnSysWrite(const void *buffer, size_t len);

protected:
    HANDLE m_hOutput;

    wxDECLARE_NO_COPY_CLASS(wxPipeOutputStream);
};

#endif 