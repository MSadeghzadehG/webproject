


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_PRINTING_ARCHITECTURE && (!defined(__WXUNIVERSAL__) || !wxUSE_POSTSCRIPT_ARCHITECTURE_IN_MSW)

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcdlg.h"
    #include "wx/app.h"
    #include "wx/dcprint.h"
    #include "wx/cmndata.h"
#endif

#include "wx/printdlg.h"
#include "wx/msw/printdlg.h"
#include "wx/msw/dcprint.h"
#include "wx/paper.h"
#include "wx/modalhook.h"

#include <stdlib.h>

class WinPrinter
{
public:
        WinPrinter()
    {
        m_hPrinter = (HANDLE)NULL;
    }

    WinPrinter( const wxString& printerName )
    {
        Open( printerName );
    }

    ~WinPrinter()
    {
        Close();
    }

    BOOL Open( const wxString& printerName, LPPRINTER_DEFAULTS pDefault=(LPPRINTER_DEFAULTS)NULL )
    {
        Close();
        return OpenPrinter( wxMSW_CONV_LPTSTR(printerName), &m_hPrinter, pDefault );
    }

    BOOL Close()
    {
        BOOL result = TRUE;
        if( m_hPrinter )
        {
            result = ClosePrinter( m_hPrinter );
            m_hPrinter = (HANDLE)NULL;
        }
        return result;
    }

    operator HANDLE() { return m_hPrinter; }
    operator bool() { return m_hPrinter != (HANDLE)NULL; }

private:
    HANDLE m_hPrinter;

    wxDECLARE_NO_COPY_CLASS(WinPrinter);
};



#if wxDEBUG_LEVEL

static wxString wxGetPrintDlgError()
{
    DWORD err = CommDlgExtendedError();
    wxString msg = wxT("Unknown");
    switch (err)
    {
        case CDERR_FINDRESFAILURE: msg = wxT("CDERR_FINDRESFAILURE"); break;
        case CDERR_INITIALIZATION: msg = wxT("CDERR_INITIALIZATION"); break;
        case CDERR_LOADRESFAILURE: msg = wxT("CDERR_LOADRESFAILURE"); break;
        case CDERR_LOADSTRFAILURE: msg = wxT("CDERR_LOADSTRFAILURE"); break;
        case CDERR_LOCKRESFAILURE: msg = wxT("CDERR_LOCKRESFAILURE"); break;
        case CDERR_MEMALLOCFAILURE: msg = wxT("CDERR_MEMALLOCFAILURE"); break;
        case CDERR_MEMLOCKFAILURE: msg = wxT("CDERR_MEMLOCKFAILURE"); break;
        case CDERR_NOHINSTANCE: msg = wxT("CDERR_NOHINSTANCE"); break;
        case CDERR_NOHOOK: msg = wxT("CDERR_NOHOOK"); break;
        case CDERR_NOTEMPLATE: msg = wxT("CDERR_NOTEMPLATE"); break;
        case CDERR_STRUCTSIZE: msg = wxT("CDERR_STRUCTSIZE"); break;
        case  PDERR_RETDEFFAILURE: msg = wxT("PDERR_RETDEFFAILURE"); break;
        case  PDERR_PRINTERNOTFOUND: msg = wxT("PDERR_PRINTERNOTFOUND"); break;
        case  PDERR_PARSEFAILURE: msg = wxT("PDERR_PARSEFAILURE"); break;
        case  PDERR_NODEVICES: msg = wxT("PDERR_NODEVICES"); break;
        case  PDERR_NODEFAULTPRN: msg = wxT("PDERR_NODEFAULTPRN"); break;
        case  PDERR_LOADDRVFAILURE: msg = wxT("PDERR_LOADDRVFAILURE"); break;
        case  PDERR_INITFAILURE: msg = wxT("PDERR_INITFAILURE"); break;
        case  PDERR_GETDEVMODEFAIL: msg = wxT("PDERR_GETDEVMODEFAIL"); break;
        case  PDERR_DNDMMISMATCH: msg = wxT("PDERR_DNDMMISMATCH"); break;
        case  PDERR_DEFAULTDIFFERENT: msg = wxT("PDERR_DEFAULTDIFFERENT"); break;
        case  PDERR_CREATEICFAILURE: msg = wxT("PDERR_CREATEICFAILURE"); break;
        default: break;
    }
    return msg;
}

#endif 

static HGLOBAL
wxCreateDevNames(const wxString& driverName,
                 const wxString& printerName,
                 const wxString& portName)
{
    HGLOBAL hDev = NULL;
        if (driverName.empty() && printerName.empty() && portName.empty())
    {
    }
    else
    {
        hDev = GlobalAlloc(GPTR, 4*sizeof(WORD)+
                           ( driverName.length() + 1 +
            printerName.length() + 1 +
                             portName.length()+1 ) * sizeof(wxChar) );
        LPDEVNAMES lpDev = (LPDEVNAMES)GlobalLock(hDev);
        lpDev->wDriverOffset = sizeof(WORD) * 4 / sizeof(wxChar);
        wxStrcpy((wxChar*)lpDev + lpDev->wDriverOffset, driverName);

        lpDev->wDeviceOffset = (WORD)( lpDev->wDriverOffset +
                                       driverName.length() + 1 );
        wxStrcpy((wxChar*)lpDev + lpDev->wDeviceOffset, printerName);

        lpDev->wOutputOffset = (WORD)( lpDev->wDeviceOffset +
                                       printerName.length() + 1 );
        wxStrcpy((wxChar*)lpDev + lpDev->wOutputOffset, portName);

        lpDev->wDefault = 0;

        GlobalUnlock(hDev);
    }

    return hDev;
}

wxIMPLEMENT_CLASS(wxWindowsPrintNativeData, wxPrintNativeDataBase);

wxWindowsPrintNativeData::wxWindowsPrintNativeData()
{
    m_devMode = NULL;
    m_devNames = NULL;
    m_customWindowsPaperId = 0;
}

wxWindowsPrintNativeData::~wxWindowsPrintNativeData()
{
    if ( m_devMode )
        ::GlobalFree(static_cast<HGLOBAL>(m_devMode));

    if ( m_devNames )
        ::GlobalFree(static_cast<HGLOBAL>(m_devNames));
}

bool wxWindowsPrintNativeData::IsOk() const
{
    return (m_devMode != NULL) ;
}

bool wxWindowsPrintNativeData::TransferTo( wxPrintData &data )
{
    if ( !m_devMode )
        InitializeDevMode();

    if ( !m_devMode )
        return false;

    GlobalPtrLock lockDevMode(m_devMode);

    LPDEVMODE devMode = static_cast<LPDEVMODE>(lockDevMode.Get());

        if (devMode->dmFields & DM_ORIENTATION)
        data.SetOrientation( (wxPrintOrientation)devMode->dmOrientation );

        if (devMode->dmFields & DM_COLLATE)
    {
        if (devMode->dmCollate == DMCOLLATE_TRUE)
            data.SetCollate( true );
        else
            data.SetCollate( false );
    }

        if (devMode->dmFields & DM_COPIES)
        data.SetNoCopies( devMode->dmCopies );

        if (devMode->dmFields & DM_DEFAULTSOURCE) {
        switch (devMode->dmDefaultSource) {
            case DMBIN_ONLYONE        : data.SetBin(wxPRINTBIN_ONLYONE       ); break;
            case DMBIN_LOWER          : data.SetBin(wxPRINTBIN_LOWER         ); break;
            case DMBIN_MIDDLE         : data.SetBin(wxPRINTBIN_MIDDLE        ); break;
            case DMBIN_MANUAL         : data.SetBin(wxPRINTBIN_MANUAL        ); break;
            case DMBIN_ENVELOPE       : data.SetBin(wxPRINTBIN_ENVELOPE      ); break;
            case DMBIN_ENVMANUAL      : data.SetBin(wxPRINTBIN_ENVMANUAL     ); break;
            case DMBIN_AUTO           : data.SetBin(wxPRINTBIN_AUTO          ); break;
            case DMBIN_TRACTOR        : data.SetBin(wxPRINTBIN_TRACTOR       ); break;
            case DMBIN_SMALLFMT       : data.SetBin(wxPRINTBIN_SMALLFMT      ); break;
            case DMBIN_LARGEFMT       : data.SetBin(wxPRINTBIN_LARGEFMT      ); break;
            case DMBIN_LARGECAPACITY  : data.SetBin(wxPRINTBIN_LARGECAPACITY ); break;
            case DMBIN_CASSETTE       : data.SetBin(wxPRINTBIN_CASSETTE      ); break;
            case DMBIN_FORMSOURCE     : data.SetBin(wxPRINTBIN_FORMSOURCE    ); break;
            default:
                if (devMode->dmDefaultSource >= DMBIN_USER)
                    data.SetBin((wxPrintBin)((devMode->dmDefaultSource)-DMBIN_USER+(int)wxPRINTBIN_USER));
                else
                    data.SetBin(wxPRINTBIN_DEFAULT);
        }
    } else {
        data.SetBin(wxPRINTBIN_DEFAULT);
    }
    if (devMode->dmFields & DM_MEDIATYPE)
    {
        wxASSERT( (int)devMode->dmMediaType != wxPRINTMEDIA_DEFAULT );
        data.SetMedia(devMode->dmMediaType);
    }
        if (devMode->dmDeviceName[0] != 0)
                data.SetPrinterName( wxString(devMode->dmDeviceName, CCHDEVICENAME) );

        if (devMode->dmFields & DM_COLOR)
    {
        if (devMode->dmColor == DMCOLOR_COLOR)
            data.SetColour( true );
        else
            data.SetColour( false );
    }
    else
        data.SetColour( true );

    
                
    bool foundPaperSize = false;
    if ((devMode->dmFields & DM_PAPERSIZE) && (devMode->dmPaperSize < DMPAPER_USER))
    {
        if (wxThePrintPaperDatabase)
        {
            wxPrintPaperType* paper = wxThePrintPaperDatabase->FindPaperTypeByPlatformId(devMode->dmPaperSize);
            if (paper)
            {
                data.SetPaperId( paper->GetId() );
                data.SetPaperSize( wxSize(paper->GetWidth() / 10,paper->GetHeight() / 10) );
                m_customWindowsPaperId = 0;
                foundPaperSize = true;
            }
        }
        else
        {
                        wxFAIL_MSG(wxT("Paper database wasn't initialized in wxPrintData::ConvertFromNative."));
            data.SetPaperId( wxPAPER_NONE );
            data.SetPaperSize( wxSize(0,0) );
            m_customWindowsPaperId = 0;

            return false;
        }
    }

    if (!foundPaperSize) {
        if ((devMode->dmFields & DM_PAPERWIDTH) && (devMode->dmFields & DM_PAPERLENGTH))
        {
                        data.SetPaperSize( wxSize(devMode->dmPaperWidth / 10, devMode->dmPaperLength / 10) );
            data.SetPaperId( wxPAPER_NONE );
            m_customWindowsPaperId = devMode->dmPaperSize;
        }
        else
        {
                                                            data.SetPaperSize( wxSize(0,0) );
            data.SetPaperId( wxPAPER_NONE );
            m_customWindowsPaperId = devMode->dmPaperSize;
        }
    }

    
    if (devMode->dmFields & DM_DUPLEX)
    {
        switch (devMode->dmDuplex)
        {
            case DMDUP_HORIZONTAL:   data.SetDuplex( wxDUPLEX_HORIZONTAL ); break;
            case DMDUP_VERTICAL:     data.SetDuplex( wxDUPLEX_VERTICAL ); break;
            default:
            case DMDUP_SIMPLEX:      data.SetDuplex( wxDUPLEX_SIMPLEX ); break;
        }
    }
    else
        data.SetDuplex( wxDUPLEX_SIMPLEX );

    
    if (devMode->dmFields & DM_PRINTQUALITY)
    {
        switch (devMode->dmPrintQuality)
        {
            case DMRES_MEDIUM:  data.SetQuality( wxPRINT_QUALITY_MEDIUM ); break;
            case DMRES_LOW:     data.SetQuality( wxPRINT_QUALITY_LOW ); break;
            case DMRES_DRAFT:   data.SetQuality( wxPRINT_QUALITY_DRAFT ); break;
            case DMRES_HIGH:    data.SetQuality( wxPRINT_QUALITY_HIGH ); break;
            default:
            {
                                                                data.SetQuality( devMode->dmPrintQuality );
                break;

            }
        }
    }
    else
        data.SetQuality( wxPRINT_QUALITY_HIGH );

    if (devMode->dmDriverExtra > 0)
        data.SetPrivData( (char *)devMode+devMode->dmSize, devMode->dmDriverExtra );
    else
        data.SetPrivData( NULL, 0 );

    if ( m_devNames )
    {
        GlobalPtrLock lockDevNames(m_devNames);
        LPDEVNAMES lpDevNames = static_cast<LPDEVNAMES>(lockDevNames.Get());

        
                        
                wxString printerName = (LPTSTR)lpDevNames + lpDevNames->wDeviceOffset;

        
        if (!printerName.empty())
            data.SetPrinterName( printerName );
    }

    return true;
}

void wxWindowsPrintNativeData::InitializeDevMode(const wxString& printerName, WinPrinter* printer)
{
    if (m_devMode)
        return;

    LPTSTR szPrinterName = wxMSW_CONV_LPTSTR(printerName);

                    if ( !m_devMode && !printerName.IsEmpty() )
    {
                if ( printer && printer->Open( printerName ) == TRUE )
        {
            DWORD dwNeeded, dwRet;

                                    dwNeeded = DocumentProperties( NULL,
                *printer,                        szPrinterName,                   NULL,                            NULL,                            0 );             
                                                                        dwNeeded += 1024;

            LPDEVMODE tempDevMode = static_cast<LPDEVMODE>( GlobalAlloc( GMEM_FIXED | GMEM_ZEROINIT, dwNeeded ) );

                                    dwRet = DocumentProperties( NULL,
                *printer,
                szPrinterName,
                tempDevMode,                     NULL,                            DM_OUT_BUFFER ); 
            if ( dwRet != IDOK )
            {
                                GlobalFree( tempDevMode );
                printer->Close();
            }
            else
            {
                m_devMode = tempDevMode;
                tempDevMode = NULL;
            }
        }
    }

    if ( !m_devMode )
    {
                PRINTDLG pd;

        memset(&pd, 0, sizeof(PRINTDLG));
        pd.lStructSize    = sizeof(PRINTDLG);

        pd.hwndOwner      = NULL;
        pd.hDevMode       = NULL;         pd.hDevNames      = NULL; 
        pd.Flags          = PD_RETURNDEFAULT;
        pd.nCopies        = 1;

                        if (!PrintDlg(&pd))
        {
            if ( pd.hDevMode )
                GlobalFree(pd.hDevMode);
            if ( pd.hDevNames )
                GlobalFree(pd.hDevNames);
            pd.hDevMode = NULL;
            pd.hDevNames = NULL;

#if wxDEBUG_LEVEL
            wxLogDebug(wxT("Printing error: ") + wxGetPrintDlgError());
#endif         }
        else
        {
            m_devMode = pd.hDevMode;
            pd.hDevMode = NULL;

                        if ( pd.hDevNames )
                GlobalFree(pd.hDevNames);
            pd.hDevNames = NULL;

                                    
        }
    }

}

bool wxWindowsPrintNativeData::TransferFrom( const wxPrintData &data )
{
    WinPrinter printer;
    LPTSTR szPrinterName = wxMSW_CONV_LPTSTR(data.GetPrinterName());

    if (!m_devMode)
        InitializeDevMode(data.GetPrinterName(), &printer);

    HGLOBAL hDevMode = static_cast<HGLOBAL>(m_devMode);

    if ( hDevMode )
    {
        GlobalPtrLock lockDevMode(hDevMode);
        DEVMODE * const devMode = static_cast<DEVMODE *>(lockDevMode.Get());

                devMode->dmOrientation = (short)data.GetOrientation();

                devMode->dmCollate = (data.GetCollate() ? DMCOLLATE_TRUE : DMCOLLATE_FALSE);
        devMode->dmFields |= DM_COLLATE;

                devMode->dmCopies = (short)data.GetNoCopies();
        devMode->dmFields |= DM_COPIES;

                wxString name = data.GetPrinterName();
        if (!name.empty())
        {
                                    wxStrlcpy(reinterpret_cast<wxChar *>(devMode->dmDeviceName),
                      name.t_str(),
                      WXSIZEOF(devMode->dmDeviceName));
        }

                if (data.GetColour())
            devMode->dmColor = DMCOLOR_COLOR;
        else
            devMode->dmColor = DMCOLOR_MONOCHROME;
        devMode->dmFields |= DM_COLOR;

        
                
        wxPrintPaperType *paperType = NULL;

        const wxPaperSize paperId = data.GetPaperId();
        if ( paperId != wxPAPER_NONE && wxThePrintPaperDatabase )
        {
            paperType = wxThePrintPaperDatabase->FindPaperType(paperId);
        }

        if ( paperType )
        {
            devMode->dmPaperSize = (short)paperType->GetPlatformId();
            devMode->dmFields |= DM_PAPERSIZE;
        }
        else         {
            const wxSize paperSize = data.GetPaperSize();
            if ( paperSize != wxDefaultSize )
            {
                                if(m_customWindowsPaperId != 0)
                    devMode->dmPaperSize = m_customWindowsPaperId;
                else
                    devMode->dmPaperSize = DMPAPER_USER;
                devMode->dmPaperWidth = (short)(paperSize.x * 10);
                devMode->dmPaperLength = (short)(paperSize.y * 10);
                devMode->dmFields |= DM_PAPERWIDTH;
                devMode->dmFields |= DM_PAPERLENGTH;

                                                                                                                                            }
                                }

                short duplex;
        switch (data.GetDuplex())
        {
            case wxDUPLEX_HORIZONTAL:
                duplex = DMDUP_HORIZONTAL;
                break;
            case wxDUPLEX_VERTICAL:
                duplex = DMDUP_VERTICAL;
                break;
            default:
                            duplex = DMDUP_SIMPLEX;
                break;
        }
        devMode->dmDuplex = duplex;
        devMode->dmFields |= DM_DUPLEX;

        
        short quality;
        switch (data.GetQuality())
        {
            case wxPRINT_QUALITY_MEDIUM:
                quality = DMRES_MEDIUM;
                break;
            case wxPRINT_QUALITY_LOW:
                quality = DMRES_LOW;
                break;
            case wxPRINT_QUALITY_DRAFT:
                quality = DMRES_DRAFT;
                break;
            case wxPRINT_QUALITY_HIGH:
                quality = DMRES_HIGH;
                break;
            default:
                quality = (short)data.GetQuality();
                devMode->dmYResolution = quality;
                devMode->dmFields |= DM_YRESOLUTION;
                break;
        }
        devMode->dmPrintQuality = quality;
        devMode->dmFields |= DM_PRINTQUALITY;

        if (data.GetPrivDataLen() > 0)
        {
            memcpy( (char *)devMode+devMode->dmSize, data.GetPrivData(), data.GetPrivDataLen() );
            devMode->dmDriverExtra = (WXWORD)data.GetPrivDataLen();
        }

        if (data.GetBin() != wxPRINTBIN_DEFAULT)
        {
            switch (data.GetBin())
            {
                case wxPRINTBIN_ONLYONE:        devMode->dmDefaultSource = DMBIN_ONLYONE;       break;
                case wxPRINTBIN_LOWER:          devMode->dmDefaultSource = DMBIN_LOWER;         break;
                case wxPRINTBIN_MIDDLE:         devMode->dmDefaultSource = DMBIN_MIDDLE;        break;
                case wxPRINTBIN_MANUAL:         devMode->dmDefaultSource = DMBIN_MANUAL;        break;
                case wxPRINTBIN_ENVELOPE:       devMode->dmDefaultSource = DMBIN_ENVELOPE;      break;
                case wxPRINTBIN_ENVMANUAL:      devMode->dmDefaultSource = DMBIN_ENVMANUAL;     break;
                case wxPRINTBIN_AUTO:           devMode->dmDefaultSource = DMBIN_AUTO;          break;
                case wxPRINTBIN_TRACTOR:        devMode->dmDefaultSource = DMBIN_TRACTOR;       break;
                case wxPRINTBIN_SMALLFMT:       devMode->dmDefaultSource = DMBIN_SMALLFMT;      break;
                case wxPRINTBIN_LARGEFMT:       devMode->dmDefaultSource = DMBIN_LARGEFMT;      break;
                case wxPRINTBIN_LARGECAPACITY:  devMode->dmDefaultSource = DMBIN_LARGECAPACITY; break;
                case wxPRINTBIN_CASSETTE:       devMode->dmDefaultSource = DMBIN_CASSETTE;      break;
                case wxPRINTBIN_FORMSOURCE:     devMode->dmDefaultSource = DMBIN_FORMSOURCE;    break;

                default:
                    devMode->dmDefaultSource = (short)(DMBIN_USER + data.GetBin() - wxPRINTBIN_USER);                     break;
            }

            devMode->dmFields |= DM_DEFAULTSOURCE;
        }
        if (data.GetMedia() != wxPRINTMEDIA_DEFAULT)
        {
            devMode->dmMediaType = data.GetMedia();
            devMode->dmFields |= DM_MEDIATYPE;
        }

        if( printer )
        {
                                                            DocumentProperties( NULL,
                printer,
                szPrinterName,
                (LPDEVMODE)hDevMode,                 (LPDEVMODE)hDevMode,                 DM_IN_BUFFER |                       DM_OUT_BUFFER );             }
    }

    if ( m_devNames )
    {
        ::GlobalFree(static_cast<HGLOBAL>(m_devNames));
    }

        m_devNames = wxCreateDevNames(wxEmptyString, data.GetPrinterName(), wxEmptyString);

    return true;
}


wxIMPLEMENT_CLASS(wxWindowsPrintDialog, wxPrintDialogBase);

wxWindowsPrintDialog::wxWindowsPrintDialog(wxWindow *p, wxPrintDialogData* data)
{
    Create(p, data);
}

wxWindowsPrintDialog::wxWindowsPrintDialog(wxWindow *p, wxPrintData* data)
{
    wxPrintDialogData data2;
    if ( data )
        data2 = *data;

    Create(p, &data2);
}

bool wxWindowsPrintDialog::Create(wxWindow *p, wxPrintDialogData* data)
{
    m_dialogParent = p;
    m_printerDC = NULL;
    m_destroyDC = true;

        m_printDlg = NULL;

    if ( data )
        m_printDialogData = *data;

    return true;
}

wxWindowsPrintDialog::~wxWindowsPrintDialog()
{
    PRINTDLG *pd = (PRINTDLG *) m_printDlg;
    if (pd && pd->hDevMode)
        GlobalFree(pd->hDevMode);
    if ( pd )
        delete pd;

    if (m_destroyDC && m_printerDC)
        delete m_printerDC;
}

int wxWindowsPrintDialog::ShowModal()
{
    WX_HOOK_MODAL_DIALOG();

    wxWindow* const parent = GetParentForModalDialog(m_parent, GetWindowStyle());
    WXHWND hWndParent = parent ? GetHwndOf(parent) : NULL;

    ConvertToNative( m_printDialogData );

    PRINTDLG *pd = (PRINTDLG*) m_printDlg;

    pd->hwndOwner = hWndParent;

    bool ret = (PrintDlg( pd ) != 0);

    pd->hwndOwner = 0;

    if ( ret && (pd->hDC) )
    {
        wxPrinterDC *pdc = new wxPrinterDCFromHDC( (WXHDC) pd->hDC );
        m_printerDC = pdc;
        ConvertFromNative( m_printDialogData );
        return wxID_OK;
    }
    else
    {
        return wxID_CANCEL;
    }
}

wxDC *wxWindowsPrintDialog::GetPrintDC()
{
    if (m_printerDC)
    {
        m_destroyDC = false;
        return m_printerDC;
    }
    else
        return NULL;
}

bool wxWindowsPrintDialog::ConvertToNative( wxPrintDialogData &data )
{
    wxWindowsPrintNativeData *native_data =
        (wxWindowsPrintNativeData *) data.GetPrintData().GetNativeData();
    data.GetPrintData().ConvertToNative();

    PRINTDLG *pd = (PRINTDLG*) m_printDlg;

        if (pd)
        return false;

    pd = new PRINTDLG;
    memset( pd, 0, sizeof(PRINTDLG) );
    m_printDlg = (void*) pd;

    pd->lStructSize    = sizeof(PRINTDLG);
    pd->hwndOwner      = NULL;
    pd->hDevMode       = NULL;     pd->hDevNames      = NULL; 
    pd->Flags          = PD_RETURNDEFAULT;
    pd->nCopies        = 1;

            if (pd->hDevMode)
        GlobalFree(pd->hDevMode);

            if (pd->hDevNames)
        GlobalFree(pd->hDevNames);

    pd->hDevMode = static_cast<HGLOBAL>(native_data->GetDevMode());
    native_data->SetDevMode(NULL);

        
    pd->hDevNames = static_cast<HGLOBAL>(native_data->GetDevNames());
    native_data->SetDevNames(NULL);


    pd->hDC = NULL;
    pd->nFromPage = (WORD)data.GetFromPage();
    pd->nToPage = (WORD)data.GetToPage();
    pd->nMinPage = (WORD)data.GetMinPage();
    pd->nMaxPage = (WORD)data.GetMaxPage();
    pd->nCopies = (WORD)data.GetNoCopies();

    pd->Flags = PD_RETURNDC;
    pd->lStructSize = sizeof( PRINTDLG );

    pd->hwndOwner = NULL;
    pd->hInstance = NULL;
    pd->lCustData = 0;
    pd->lpfnPrintHook = NULL;
    pd->lpfnSetupHook = NULL;
    pd->lpPrintTemplateName = NULL;
    pd->lpSetupTemplateName = NULL;
    pd->hPrintTemplate = NULL;
    pd->hSetupTemplate = NULL;

    if ( data.GetAllPages() )
        pd->Flags |= PD_ALLPAGES;
    if ( data.GetSelection() )
        pd->Flags |= PD_SELECTION;
    if ( data.GetCollate() )
        pd->Flags |= PD_COLLATE;
    if ( data.GetPrintToFile() )
        pd->Flags |= PD_PRINTTOFILE;
    if ( !data.GetEnablePrintToFile() )
        pd->Flags |= PD_DISABLEPRINTTOFILE;
    if ( !data.GetEnableSelection() )
        pd->Flags |= PD_NOSELECTION;
    if ( !data.GetEnablePageNumbers() )
        pd->Flags |= PD_NOPAGENUMS;
    else if ( (!data.GetAllPages()) && (!data.GetSelection()) && (data.GetFromPage() != 0) && (data.GetToPage() != 0))
        pd->Flags |= PD_PAGENUMS;
    if ( data.GetEnableHelp() )
        pd->Flags |= PD_SHOWHELP;

    return true;
}

bool wxWindowsPrintDialog::ConvertFromNative( wxPrintDialogData &data )
{
    PRINTDLG *pd = (PRINTDLG*) m_printDlg;
    if ( pd == NULL )
        return false;

    wxWindowsPrintNativeData *native_data =
        (wxWindowsPrintNativeData *) data.GetPrintData().GetNativeData();

        if (pd->hDevMode)
    {
        if (native_data->GetDevMode())
        {
            ::GlobalFree(static_cast<HGLOBAL>(native_data->GetDevMode()));
        }
        native_data->SetDevMode(pd->hDevMode);
        pd->hDevMode = NULL;
    }

        if (pd->hDevNames)
    {
        if (native_data->GetDevNames())
        {
            ::GlobalFree(static_cast<HGLOBAL>(native_data->GetDevNames()));
        }
        native_data->SetDevNames(pd->hDevNames);
        pd->hDevNames = NULL;
    }

            native_data->TransferTo( data.GetPrintData() );

    data.SetFromPage( pd->nFromPage );
    data.SetToPage( pd->nToPage );
    data.SetMinPage( pd->nMinPage );
    data.SetMaxPage( pd->nMaxPage );
    data.SetNoCopies( pd->nCopies );

    data.SetAllPages( (((pd->Flags & PD_PAGENUMS) != PD_PAGENUMS) && ((pd->Flags & PD_SELECTION) != PD_SELECTION)) );
    data.SetSelection( ((pd->Flags & PD_SELECTION) == PD_SELECTION) );
    data.SetCollate( ((pd->Flags & PD_COLLATE) == PD_COLLATE) );
    data.SetPrintToFile( ((pd->Flags & PD_PRINTTOFILE) == PD_PRINTTOFILE) );
    data.EnablePrintToFile( ((pd->Flags & PD_DISABLEPRINTTOFILE) != PD_DISABLEPRINTTOFILE) );
    data.EnableSelection( ((pd->Flags & PD_NOSELECTION) != PD_NOSELECTION) );
    data.EnablePageNumbers( ((pd->Flags & PD_NOPAGENUMS) != PD_NOPAGENUMS) );
    data.EnableHelp( ((pd->Flags & PD_SHOWHELP) == PD_SHOWHELP) );

    return true;
}


wxIMPLEMENT_CLASS(wxWindowsPageSetupDialog, wxPageSetupDialogBase);

wxWindowsPageSetupDialog::wxWindowsPageSetupDialog()
{
    m_dialogParent = NULL;
    m_pageDlg = NULL;
}

wxWindowsPageSetupDialog::wxWindowsPageSetupDialog(wxWindow *p, wxPageSetupDialogData *data)
{
    Create(p, data);
}

bool wxWindowsPageSetupDialog::Create(wxWindow *p, wxPageSetupDialogData *data)
{
    m_dialogParent = p;
    m_pageDlg = NULL;

    if (data)
        m_pageSetupData = (*data);

    return true;
}

wxWindowsPageSetupDialog::~wxWindowsPageSetupDialog()
{
    PAGESETUPDLG *pd = (PAGESETUPDLG *)m_pageDlg;
    if ( pd && pd->hDevMode )
        GlobalFree(pd->hDevMode);
    if ( pd && pd->hDevNames )
        GlobalFree(pd->hDevNames);
    if ( pd )
        delete pd;
}

int wxWindowsPageSetupDialog::ShowModal()
{
    WX_HOOK_MODAL_DIALOG();

    ConvertToNative( m_pageSetupData );

    PAGESETUPDLG *pd = (PAGESETUPDLG *) m_pageDlg;
    if (m_dialogParent)
        pd->hwndOwner = (HWND) m_dialogParent->GetHWND();
    else if (wxTheApp->GetTopWindow())
        pd->hwndOwner = (HWND) wxTheApp->GetTopWindow()->GetHWND();
    else
        pd->hwndOwner = 0;
    BOOL retVal = PageSetupDlg( pd ) ;
    pd->hwndOwner = 0;
    if (retVal)
    {
        ConvertFromNative( m_pageSetupData );
        return wxID_OK;
    }
    else
        return wxID_CANCEL;
}

bool wxWindowsPageSetupDialog::ConvertToNative( wxPageSetupDialogData &data )
{
    wxWindowsPrintNativeData *native_data =
        (wxWindowsPrintNativeData *) data.GetPrintData().GetNativeData();
    data.GetPrintData().ConvertToNative();

    PAGESETUPDLG *pd = (PAGESETUPDLG*) m_pageDlg;

        if (pd)
        return false;

    pd = new PAGESETUPDLG;
    m_pageDlg = (void *)pd;

            if ( data.GetDefaultInfo() )
    {
        pd->hDevMode = NULL;
        pd->hDevNames = NULL;
    }
    else
    {
                        
        pd->hDevMode = (HGLOBAL) native_data->GetDevMode();
        native_data->SetDevMode(NULL);

                
                        
        pd->hDevNames = (HGLOBAL) native_data->GetDevNames();
        native_data->SetDevNames(NULL);
    }

    pd->Flags = PSD_MARGINS|PSD_MINMARGINS;

    if ( data.GetDefaultMinMargins() )
        pd->Flags |= PSD_DEFAULTMINMARGINS;
    if ( !data.GetEnableMargins() )
        pd->Flags |= PSD_DISABLEMARGINS;
    if ( !data.GetEnableOrientation() )
        pd->Flags |= PSD_DISABLEORIENTATION;
    if ( !data.GetEnablePaper() )
        pd->Flags |= PSD_DISABLEPAPER;
    if ( !data.GetEnablePrinter() )
        pd->Flags |= PSD_DISABLEPRINTER;
    if ( data.GetDefaultInfo() )
        pd->Flags |= PSD_RETURNDEFAULT;
    if ( data.GetEnableHelp() )
        pd->Flags |= PSD_SHOWHELP;

        pd->Flags |= PSD_INHUNDREDTHSOFMILLIMETERS;

    pd->lStructSize = sizeof( PAGESETUPDLG );
    pd->hwndOwner = NULL;
    pd->hInstance = NULL;
        pd->ptPaperSize.x = data.GetPaperSize().x * 100;
    pd->ptPaperSize.y = data.GetPaperSize().y * 100;

    pd->rtMinMargin.left = data.GetMinMarginTopLeft().x * 100;
    pd->rtMinMargin.top = data.GetMinMarginTopLeft().y * 100;
    pd->rtMinMargin.right = data.GetMinMarginBottomRight().x * 100;
    pd->rtMinMargin.bottom = data.GetMinMarginBottomRight().y * 100;

    pd->rtMargin.left = data.GetMarginTopLeft().x * 100;
    pd->rtMargin.top = data.GetMarginTopLeft().y * 100;
    pd->rtMargin.right = data.GetMarginBottomRight().x * 100;
    pd->rtMargin.bottom = data.GetMarginBottomRight().y * 100;

    pd->lCustData = 0;
    pd->lpfnPageSetupHook = NULL;
    pd->lpfnPagePaintHook = NULL;
    pd->hPageSetupTemplate = NULL;
    pd->lpPageSetupTemplateName = NULL;

    return true;
}

bool wxWindowsPageSetupDialog::ConvertFromNative( wxPageSetupDialogData &data )
{
    PAGESETUPDLG *pd = (PAGESETUPDLG *) m_pageDlg;
    if ( !pd )
        return false;

    wxWindowsPrintNativeData *native_data =
        (wxWindowsPrintNativeData *) data.GetPrintData().GetNativeData();

        if (pd->hDevMode)
    {
        if (native_data->GetDevMode())
        {
                        GlobalFree((HGLOBAL) native_data->GetDevMode());
        }
        native_data->SetDevMode( (void*) pd->hDevMode );
        pd->hDevMode = NULL;
    }

        
        if (pd->hDevNames)
    {
        if (native_data->GetDevNames())
        {
                        GlobalFree((HGLOBAL) native_data->GetDevNames());
        }
        native_data->SetDevNames((void*) pd->hDevNames);
        pd->hDevNames = NULL;
    }

    data.GetPrintData().ConvertFromNative();

    pd->Flags = PSD_MARGINS|PSD_MINMARGINS;

    data.SetDefaultMinMargins( ((pd->Flags & PSD_DEFAULTMINMARGINS) == PSD_DEFAULTMINMARGINS) );
    data.EnableMargins( ((pd->Flags & PSD_DISABLEMARGINS) != PSD_DISABLEMARGINS) );
    data.EnableOrientation( ((pd->Flags & PSD_DISABLEORIENTATION) != PSD_DISABLEORIENTATION) );
    data.EnablePaper( ((pd->Flags & PSD_DISABLEPAPER) != PSD_DISABLEPAPER) );
    data.EnablePrinter( ((pd->Flags & PSD_DISABLEPRINTER) != PSD_DISABLEPRINTER) );
    data.SetDefaultInfo( ((pd->Flags & PSD_RETURNDEFAULT) == PSD_RETURNDEFAULT) );
    data.EnableHelp( ((pd->Flags & PSD_SHOWHELP) == PSD_SHOWHELP) );

        if (data.GetPrintData().GetOrientation() == wxLANDSCAPE)
        data.SetPaperSize( wxSize(pd->ptPaperSize.y / 100, pd->ptPaperSize.x / 100) );
    else
        data.SetPaperSize( wxSize(pd->ptPaperSize.x / 100, pd->ptPaperSize.y / 100) );

    data.SetMinMarginTopLeft( wxPoint(pd->rtMinMargin.left / 100, pd->rtMinMargin.top / 100) );
    data.SetMinMarginBottomRight( wxPoint(pd->rtMinMargin.right / 100, pd->rtMinMargin.bottom / 100) );

    data.SetMarginTopLeft( wxPoint(pd->rtMargin.left / 100, pd->rtMargin.top / 100) );
    data.SetMarginBottomRight( wxPoint(pd->rtMargin.right / 100, pd->rtMargin.bottom / 100) );

    return true;
}

#endif
    