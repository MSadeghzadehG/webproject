


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_STACKWALKER

#ifndef WX_PRECOMP
    #include "wx/string.h"
#endif

#include "wx/stackwalk.h"

#include "wx/msw/debughlp.h"

#if wxUSE_DBGHELP



void wxStackFrame::OnGetName()
{
    if ( m_hasName )
        return;

    m_hasName = true;

        if ( !wxDbgHelpDLL::CallSymFromAddr
                        (
                            ::GetCurrentProcess(),
                            GetSymAddr(),
                            &m_offset,
                            &m_name
                        ) )
    {
        wxDbgHelpDLL::LogError(wxT("SymFromAddr"));
    }
}

void wxStackFrame::OnGetLocation()
{
    if ( m_hasLocation )
        return;

    m_hasLocation = true;

                wxDbgHelpDLL::CallSymGetLineFromAddr(::GetCurrentProcess(), GetSymAddr(),
                                         &m_filename, &m_line);
}

bool
wxStackFrame::GetParam(size_t n,
                       wxString *type,
                       wxString *name,
                       wxString *value) const
{
    if ( !DoGetParamCount() )
        ConstCast()->OnGetParam();

    if ( n >= DoGetParamCount() )
        return false;

    if ( type )
        *type = m_paramTypes[n];
    if ( name )
        *name = m_paramNames[n];
    if ( value )
        *value = m_paramValues[n];

    return true;
}

void wxStackFrame::OnParam(wxSYMBOL_INFO *pSymInfo)
{
    m_paramTypes.Add(wxEmptyString);
    m_paramNames.Add(pSymInfo->Name);

                    #ifdef _CPPUNWIND
    try
#else
    __try
#endif
    {
                        DWORD_PTR pValue = m_addrFrame + pSymInfo->Address;
        m_paramValues.Add(wxDbgHelpDLL::DumpSymbol(pSymInfo, (void *)pValue));
    }
#ifdef _CPPUNWIND
    catch ( ... )
#else
    __except ( EXCEPTION_EXECUTE_HANDLER )
#endif
    {
        m_paramValues.Add(wxEmptyString);
    }
}

BOOL CALLBACK
EnumSymbolsProc(wxPSYMBOL_INFO pSymInfo, ULONG WXUNUSED(SymSize), PVOID data)
{
    wxStackFrame *frame = static_cast<wxStackFrame *>(data);

        if ( pSymInfo->Flags & IMAGEHLP_SYMBOL_INFO_PARAMETER )
    {
        frame->OnParam(pSymInfo);
    }

        return TRUE;
}

void wxStackFrame::OnGetParam()
{
        IMAGEHLP_STACK_FRAME imagehlpStackFrame;
    wxZeroMemory(imagehlpStackFrame);
    imagehlpStackFrame.InstructionOffset = GetSymAddr();
    if ( !wxDbgHelpDLL::SymSetContext
                        (
                            ::GetCurrentProcess(),
                            &imagehlpStackFrame,
                            0                                   ) )
    {
                        if ( ::GetLastError() != ERROR_INVALID_ADDRESS )
        {
            wxDbgHelpDLL::LogError(wxT("SymSetContext"));
        }

        return;
    }

    if ( !wxDbgHelpDLL::CallSymEnumSymbols
                        (
                            ::GetCurrentProcess(),
                            NULL,                                           EnumSymbolsProc,                                this                                        ) )
    {
        wxDbgHelpDLL::LogError(wxT("SymEnumSymbols"));
    }
}



void wxStackWalker::WalkFrom(const CONTEXT *pCtx, size_t skip, size_t maxDepth)
{
    if ( !wxDbgHelpDLL::Init() )
    {
                                wxLogDebug(wxT("Failed to get stack backtrace: %s"),
                   wxDbgHelpDLL::GetErrorMessage().c_str());
        return;
    }

                        const HANDLE hProcess = ::GetCurrentProcess();

    if ( !wxDbgHelpDLL::CallSymInitialize
                        (
                            hProcess,
                            TRUE                            ) )
    {
        wxDbgHelpDLL::LogError(wxT("SymInitialize"));

        return;
    }

    CONTEXT ctx = *pCtx; 
    DWORD dwMachineType;

        STACKFRAME sf;
    wxZeroMemory(sf);

#if defined(_M_AMD64)
    sf.AddrPC.Offset       = ctx.Rip;
    sf.AddrPC.Mode         = AddrModeFlat;
    sf.AddrStack.Offset    = ctx.Rsp;
    sf.AddrStack.Mode      = AddrModeFlat;
    sf.AddrFrame.Offset    = ctx.Rbp;
    sf.AddrFrame.Mode      = AddrModeFlat;

    dwMachineType = IMAGE_FILE_MACHINE_AMD64;
#elif  defined(_M_IX86)
    sf.AddrPC.Offset       = ctx.Eip;
    sf.AddrPC.Mode         = AddrModeFlat;
    sf.AddrStack.Offset    = ctx.Esp;
    sf.AddrStack.Mode      = AddrModeFlat;
    sf.AddrFrame.Offset    = ctx.Ebp;
    sf.AddrFrame.Mode      = AddrModeFlat;

    dwMachineType = IMAGE_FILE_MACHINE_I386;
#else
    #error "Need to initialize STACKFRAME on non x86"
#endif 
        for ( size_t nLevel = 0; nLevel < maxDepth; nLevel++ )
    {
                if ( !wxDbgHelpDLL::StackWalk
                            (
                                dwMachineType,
                                hProcess,
                                ::GetCurrentThread(),
                                &sf,
                                &ctx,
                                NULL,                                       wxDbgHelpDLL::SymFunctionTableAccess,
                                wxDbgHelpDLL::SymGetModuleBase,
                                NULL                                    ) )
        {
            if ( ::GetLastError() )
                wxDbgHelpDLL::LogError(wxT("StackWalk"));

            break;
        }

                if ( nLevel >= skip )
        {
            wxStackFrame frame(nLevel - skip,
                               wxUIntToPtr(sf.AddrPC.Offset),
                               sf.AddrFrame.Offset);

            OnStackFrame(frame);
        }
    }

    if ( !wxDbgHelpDLL::SymCleanup(hProcess) )
    {
        wxDbgHelpDLL::LogError(wxT("SymCleanup"));
    }
}

void wxStackWalker::WalkFrom(const _EXCEPTION_POINTERS *ep, size_t skip, size_t maxDepth)
{
    WalkFrom(ep->ContextRecord, skip, maxDepth);
}

#if wxUSE_ON_FATAL_EXCEPTION

void wxStackWalker::WalkFromException(size_t maxDepth)
{
    extern EXCEPTION_POINTERS *wxGlobalSEInformation;

    wxCHECK_RET( wxGlobalSEInformation,
                 wxT("wxStackWalker::WalkFromException() can only be called from wxApp::OnFatalException()") );

        WalkFrom(wxGlobalSEInformation, 0, maxDepth);
}

#endif 
void wxStackWalker::Walk(size_t skip, size_t WXUNUSED(maxDepth))
{
                                __try
    {
        RaiseException(0x1976, 0, 0, NULL);
    }
    __except( WalkFrom((EXCEPTION_POINTERS *)GetExceptionInformation(),
                       skip + 2), EXCEPTION_CONTINUE_EXECUTION )
    {
                    }
}

#else 


void wxStackFrame::OnGetName()
{
}

void wxStackFrame::OnGetLocation()
{
}

bool
wxStackFrame::GetParam(size_t WXUNUSED(n),
                       wxString * WXUNUSED(type),
                       wxString * WXUNUSED(name),
                       wxString * WXUNUSED(value)) const
{
    return false;
}

void wxStackFrame::OnParam(wxSYMBOL_INFO * WXUNUSED(pSymInfo))
{
}

void wxStackFrame::OnGetParam()
{
}


void
wxStackWalker::WalkFrom(const CONTEXT * WXUNUSED(pCtx),
                        size_t WXUNUSED(skip),
                        size_t WXUNUSED(maxDepth))
{
}

void
wxStackWalker::WalkFrom(const _EXCEPTION_POINTERS * WXUNUSED(ep),
                        size_t WXUNUSED(skip),
                        size_t WXUNUSED(maxDepth))
{
}

#if wxUSE_ON_FATAL_EXCEPTION
void wxStackWalker::WalkFromException(size_t WXUNUSED(maxDepth))
{
}
#endif 
void wxStackWalker::Walk(size_t WXUNUSED(skip), size_t WXUNUSED(maxDepth))
{
}

#endif 
#endif 
