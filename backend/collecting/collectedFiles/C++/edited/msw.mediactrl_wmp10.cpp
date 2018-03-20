



#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#if wxUSE_MEDIACTRL && wxUSE_ACTIVEX

#include "wx/mediactrl.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
#endif

#include "wx/msw/private.h" #include "wx/msw/ole/activex.h" 
#if 0
    #define WXTEST_ATL
#endif

#ifdef WXTEST_ATL
    #include <atlbase.h>
    CComModule _Module;
    #define min(x,y) (x < y ? x : y)
    #include <atlcom.h>
    #include <atlhost.h>
    #include <atlctl.h>
#endif


#define wxWMP10LOG(x)


const CLSID CLSID_WMP10              = {0x6BF52A50,0x394A,0x11D3,{0xB1,0x53,0x00,0xC0,0x4F,0x79,0xFA,0xA6}};
const CLSID CLSID_WMP10ALT           = {0x6BF52A52,0x394A,0x11D3,{0xB1,0x53,0x00,0xC0,0x4F,0x79,0xFA,0xA6}};

const IID IID_IWMPSettings = {0x9104D1AB,0x80C9,0x4FED,{0xAB,0xF0,0x2E,0x64,0x17,0xA6,0xDF,0x14}};
const IID IID_IWMPCore = {0xD84CCA99,0xCCE2,0x11D2,{0x9E,0xCC,0x00,0x00,0xF8,0x08,0x59,0x81}};
#ifndef WXTEST_ATL
    const IID IID_IWMPPlayer = {0x6BF52A4F,0x394A,0x11D3,{0xB1,0x53,0x00,0xC0,0x4F,0x79,0xFA,0xA6}};
#endif

const IID IID_IWMPMedia = {0x94D55E95,0x3FAC,0x11D3,{0xB1,0x55,0x00,0xC0,0x4F,0x79,0xFA,0xA6}};
const IID IID_IWMPControls = {0x74C09E02,0xF828,0x11D2,{0xA7,0x4B,0x00,0xA0,0xC9,0x05,0xF3,0x6E}};
const IID IID_IWMPPlayer2 = {0x0E6B01D1,0xD407,0x4C85,{0xBF,0x5F,0x1C,0x01,0xF6,0x15,0x02,0x80}};
const IID IID_IWMPCore2 = {0xBC17E5B7,0x7561,0x4C18,{0xBB,0x90,0x17,0xD4,0x85,0x77,0x56,0x59}};
const IID IID_IWMPCore3 = {0x7587C667,0x628F,0x499F,{0x88,0xE7,0x6A,0x6F,0x4E,0x88,0x84,0x64}};
const IID IID_IWMPNetwork = {0xEC21B779,0xEDEF,0x462D,{0xBB,0xA4,0xAD,0x9D,0xDE,0x2B,0x29,0xA7}};

enum WMPOpenState
{
    wmposUndefined  = 0,
    wmposPlaylistChanging   = 1,
    wmposPlaylistLocating   = 2,
    wmposPlaylistConnecting = 3,
    wmposPlaylistLoading    = 4,
    wmposPlaylistOpening    = 5,
    wmposPlaylistOpenNoMedia    = 6,
    wmposPlaylistChanged    = 7,
    wmposMediaChanging  = 8,
    wmposMediaLocating  = 9,
    wmposMediaConnecting    = 10,
    wmposMediaLoading   = 11,
    wmposMediaOpening   = 12,
    wmposMediaOpen  = 13,
    wmposBeginCodecAcquisition  = 14,
    wmposEndCodecAcquisition    = 15,
    wmposBeginLicenseAcquisition    = 16,
    wmposEndLicenseAcquisition  = 17,
    wmposBeginIndividualization = 18,
    wmposEndIndividualization   = 19,
    wmposMediaWaiting   = 20,
    wmposOpeningUnknownURL  = 21
};

enum WMPPlayState
{
    wmppsUndefined  = 0,
    wmppsStopped    = 1,
    wmppsPaused = 2,
    wmppsPlaying    = 3,
    wmppsScanForward    = 4,
    wmppsScanReverse    = 5,
    wmppsBuffering  = 6,
    wmppsWaiting    = 7,
    wmppsMediaEnded = 8,
    wmppsTransitioning  = 9,
    wmppsReady  = 10,
    wmppsReconnecting   = 11,
    wmppsLast   = 12
};


struct IWMPMedia : public IDispatch
{
public:
    virtual  HRESULT STDMETHODCALLTYPE get_isIdentical(
         IWMPMedia __RPC_FAR *pIWMPMedia,
         VARIANT_BOOL __RPC_FAR *pvbool) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_sourceURL(
         BSTR __RPC_FAR *pbstrSourceURL) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_name(
         BSTR __RPC_FAR *pbstrName) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_name(
         BSTR pbstrName) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_imageSourceWidth(
         long __RPC_FAR *pWidth) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_imageSourceHeight(
         long __RPC_FAR *pHeight) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_markerCount(
         long __RPC_FAR *pMarkerCount) = 0;

    virtual  HRESULT STDMETHODCALLTYPE getMarkerTime(
         long MarkerNum,
         double __RPC_FAR *pMarkerTime) = 0;

    virtual  HRESULT STDMETHODCALLTYPE getMarkerName(
         long MarkerNum,
         BSTR __RPC_FAR *pbstrMarkerName) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_duration(
         double __RPC_FAR *pDuration) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_durationString(
         BSTR __RPC_FAR *pbstrDuration) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_attributeCount(
         long __RPC_FAR *plCount) = 0;

    virtual  HRESULT STDMETHODCALLTYPE getAttributeName(
         long lIndex,
         BSTR __RPC_FAR *pbstrItemName) = 0;

    virtual  HRESULT STDMETHODCALLTYPE getItemInfo(
         BSTR bstrItemName,
         BSTR __RPC_FAR *pbstrVal) = 0;

    virtual  HRESULT STDMETHODCALLTYPE setItemInfo(
         BSTR bstrItemName,
         BSTR bstrVal) = 0;

    virtual  HRESULT STDMETHODCALLTYPE getItemInfoByAtom(
         long lAtom,
         BSTR __RPC_FAR *pbstrVal) = 0;

    virtual  HRESULT STDMETHODCALLTYPE isMemberOf(
         IUnknown __RPC_FAR *pPlaylist,
         VARIANT_BOOL __RPC_FAR *pvarfIsMemberOf) = 0;

    virtual  HRESULT STDMETHODCALLTYPE isReadOnlyItem(
         BSTR bstrItemName,
         VARIANT_BOOL __RPC_FAR *pvarfIsReadOnly) = 0;

};

struct IWMPControls : public IDispatch
{
public:
    virtual  HRESULT STDMETHODCALLTYPE get_isAvailable(
         BSTR bstrItem,
         VARIANT_BOOL __RPC_FAR *pIsAvailable) = 0;

    virtual  HRESULT STDMETHODCALLTYPE play( void) = 0;

    virtual  HRESULT STDMETHODCALLTYPE stop( void) = 0;

    virtual  HRESULT STDMETHODCALLTYPE pause( void) = 0;

    virtual  HRESULT STDMETHODCALLTYPE fastForward( void) = 0;

    virtual  HRESULT STDMETHODCALLTYPE fastReverse( void) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_currentPosition(
         double __RPC_FAR *pdCurrentPosition) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_currentPosition(
         double pdCurrentPosition) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_currentPositionString(
         BSTR __RPC_FAR *pbstrCurrentPosition) = 0;

    virtual  HRESULT STDMETHODCALLTYPE next( void) = 0;

    virtual  HRESULT STDMETHODCALLTYPE previous( void) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_currentItem(
         IWMPMedia __RPC_FAR *__RPC_FAR *ppIWMPMedia) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_currentItem(
         IWMPMedia __RPC_FAR *ppIWMPMedia) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_currentMarker(
         long __RPC_FAR *plMarker) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_currentMarker(
         long plMarker) = 0;

    virtual  HRESULT STDMETHODCALLTYPE playItem(
         IWMPMedia __RPC_FAR *pIWMPMedia) = 0;

};


struct IWMPSettings : public IDispatch
{
public:
    virtual  HRESULT STDMETHODCALLTYPE get_isAvailable(
         BSTR bstrItem,
         VARIANT_BOOL __RPC_FAR *pIsAvailable) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_autoStart(
         VARIANT_BOOL __RPC_FAR *pfAutoStart) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_autoStart(
         VARIANT_BOOL pfAutoStart) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_baseURL(
         BSTR __RPC_FAR *pbstrBaseURL) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_baseURL(
         BSTR pbstrBaseURL) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_defaultFrame(
         BSTR __RPC_FAR *pbstrDefaultFrame) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_defaultFrame(
         BSTR pbstrDefaultFrame) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_invokeURLs(
         VARIANT_BOOL __RPC_FAR *pfInvokeURLs) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_invokeURLs(
         VARIANT_BOOL pfInvokeURLs) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_mute(
         VARIANT_BOOL __RPC_FAR *pfMute) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_mute(
         VARIANT_BOOL pfMute) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_playCount(
         long __RPC_FAR *plCount) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_playCount(
         long plCount) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_rate(
         double __RPC_FAR *pdRate) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_rate(
         double pdRate) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_balance(
         long __RPC_FAR *plBalance) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_balance(
         long plBalance) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_volume(
         long __RPC_FAR *plVolume) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_volume(
         long plVolume) = 0;

    virtual  HRESULT STDMETHODCALLTYPE getMode(
         BSTR bstrMode,
         VARIANT_BOOL __RPC_FAR *pvarfMode) = 0;

    virtual  HRESULT STDMETHODCALLTYPE setMode(
         BSTR bstrMode,
         VARIANT_BOOL varfMode) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_enableErrorDialogs(
         VARIANT_BOOL __RPC_FAR *pfEnableErrorDialogs) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_enableErrorDialogs(
         VARIANT_BOOL pfEnableErrorDialogs) = 0;

};

struct IWMPNetwork : public IDispatch
{
public:
    virtual  HRESULT STDMETHODCALLTYPE get_bandWidth(
         long __RPC_FAR *plBandwidth) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_recoveredPackets(
         long __RPC_FAR *plRecoveredPackets) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_sourceProtocol(
         BSTR __RPC_FAR *pbstrSourceProtocol) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_receivedPackets(
         long __RPC_FAR *plReceivedPackets) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_lostPackets(
         long __RPC_FAR *plLostPackets) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_receptionQuality(
         long __RPC_FAR *plReceptionQuality) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_bufferingCount(
         long __RPC_FAR *plBufferingCount) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_bufferingProgress(
         long __RPC_FAR *plBufferingProgress) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_bufferingTime(
         long __RPC_FAR *plBufferingTime) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_bufferingTime(
         long plBufferingTime) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_frameRate(
         long __RPC_FAR *plFrameRate) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_maxBitRate(
         long __RPC_FAR *plBitRate) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_bitRate(
         long __RPC_FAR *plBitRate) = 0;

    virtual  HRESULT STDMETHODCALLTYPE getProxySettings(
         BSTR bstrProtocol,
         long __RPC_FAR *plProxySetting) = 0;

    virtual  HRESULT STDMETHODCALLTYPE setProxySettings(
         BSTR bstrProtocol,
         long lProxySetting) = 0;

    virtual  HRESULT STDMETHODCALLTYPE getProxyName(
         BSTR bstrProtocol,
         BSTR __RPC_FAR *pbstrProxyName) = 0;

    virtual  HRESULT STDMETHODCALLTYPE setProxyName(
         BSTR bstrProtocol,
         BSTR bstrProxyName) = 0;

    virtual  HRESULT STDMETHODCALLTYPE getProxyPort(
         BSTR bstrProtocol,
         long __RPC_FAR *lProxyPort) = 0;

    virtual  HRESULT STDMETHODCALLTYPE setProxyPort(
         BSTR bstrProtocol,
         long lProxyPort) = 0;

    virtual  HRESULT STDMETHODCALLTYPE getProxyExceptionList(
         BSTR bstrProtocol,
         BSTR __RPC_FAR *pbstrExceptionList) = 0;

    virtual  HRESULT STDMETHODCALLTYPE setProxyExceptionList(
         BSTR bstrProtocol,
         BSTR pbstrExceptionList) = 0;

    virtual  HRESULT STDMETHODCALLTYPE getProxyBypassForLocal(
         BSTR bstrProtocol,
         VARIANT_BOOL __RPC_FAR *pfBypassForLocal) = 0;

    virtual  HRESULT STDMETHODCALLTYPE setProxyBypassForLocal(
         BSTR bstrProtocol,
         VARIANT_BOOL fBypassForLocal) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_maxBandwidth(
         long __RPC_FAR *lMaxBandwidth) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_maxBandwidth(
         long lMaxBandwidth) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_downloadProgress(
         long __RPC_FAR *plDownloadProgress) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_encodedFrameRate(
         long __RPC_FAR *plFrameRate) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_framesSkipped(
         long __RPC_FAR *plFrames) = 0;

};

struct IWMPCore : public IDispatch
{
public:
    virtual  HRESULT STDMETHODCALLTYPE close( void) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_URL(
         BSTR __RPC_FAR *pbstrURL) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_URL(
         BSTR pbstrURL) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_openState(
         WMPOpenState __RPC_FAR *pwmpos) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_playState(
         WMPPlayState __RPC_FAR *pwmpps) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_controls(
         IWMPControls __RPC_FAR *__RPC_FAR *ppControl) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_settings(
         IWMPSettings __RPC_FAR *__RPC_FAR *ppSettings) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_currentMedia(
         IWMPMedia __RPC_FAR *__RPC_FAR *ppMedia) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_currentMedia(
         IUnknown __RPC_FAR *ppMedia) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_mediaCollection(
         IUnknown __RPC_FAR *__RPC_FAR *ppMediaCollection) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_playlistCollection(
         IUnknown __RPC_FAR *__RPC_FAR *ppPlaylistCollection) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_versionInfo(
         BSTR __RPC_FAR *pbstrVersionInfo) = 0;

    virtual  HRESULT STDMETHODCALLTYPE launchURL(
         BSTR bstrURL) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_network(
         IWMPNetwork __RPC_FAR *__RPC_FAR *ppQNI) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_currentPlaylist(
         IUnknown __RPC_FAR *__RPC_FAR *ppPL) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_currentPlaylist(
         IUnknown __RPC_FAR *ppPL) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_cdromCollection(
         IUnknown __RPC_FAR *__RPC_FAR *ppCdromCollection) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_closedCaption(
         IUnknown __RPC_FAR *__RPC_FAR *ppClosedCaption) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_isOnline(
         VARIANT_BOOL __RPC_FAR *pfOnline) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_Error(
         IUnknown __RPC_FAR *__RPC_FAR *ppError) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_status(
         BSTR __RPC_FAR *pbstrStatus) = 0;

};

struct IWMPCore2 : public IWMPCore
{
public:
    virtual  HRESULT STDMETHODCALLTYPE get_dvd(
         IUnknown __RPC_FAR *__RPC_FAR *ppDVD) = 0;

};

struct IWMPCore3 : public IWMPCore2
{
public:
    virtual  HRESULT STDMETHODCALLTYPE newPlaylist(
         BSTR bstrName,
         BSTR bstrURL,
         IUnknown __RPC_FAR *__RPC_FAR *ppPlaylist) = 0;

    virtual  HRESULT STDMETHODCALLTYPE newMedia(
         BSTR bstrURL,
         IWMPMedia __RPC_FAR *__RPC_FAR *ppMedia) = 0;

};

#ifdef WXTEST_ATL
    MIDL_INTERFACE("6BF52A4F-394A-11D3-B153-00C04F79FAA6")
#else
    struct
#endif
IWMPPlayer : public IWMPCore
{
public:
    virtual  HRESULT STDMETHODCALLTYPE get_enabled(
         VARIANT_BOOL __RPC_FAR *pbEnabled) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_enabled(
         VARIANT_BOOL pbEnabled) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_fullScreen(
         VARIANT_BOOL __RPC_FAR *pbFullScreen) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_fullScreen(
        VARIANT_BOOL pbFullScreen) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_enableContextMenu(
         VARIANT_BOOL __RPC_FAR *pbEnableContextMenu) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_enableContextMenu(
        VARIANT_BOOL pbEnableContextMenu) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_uiMode(
         BSTR pbstrMode) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_uiMode(
         BSTR __RPC_FAR *pbstrMode) = 0;
};

struct IWMPPlayer2 : public IWMPCore
{
public:
    virtual  HRESULT STDMETHODCALLTYPE get_enabled(
         VARIANT_BOOL __RPC_FAR *pbEnabled) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_enabled(
         VARIANT_BOOL pbEnabled) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_fullScreen(
         VARIANT_BOOL __RPC_FAR *pbFullScreen) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_fullScreen(
        VARIANT_BOOL pbFullScreen) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_enableContextMenu(
         VARIANT_BOOL __RPC_FAR *pbEnableContextMenu) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_enableContextMenu(
        VARIANT_BOOL pbEnableContextMenu) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_uiMode(
         BSTR pbstrMode) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_uiMode(
         BSTR __RPC_FAR *pbstrMode) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_stretchToFit(
         VARIANT_BOOL __RPC_FAR *pbEnabled) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_stretchToFit(
         VARIANT_BOOL pbEnabled) = 0;

    virtual  HRESULT STDMETHODCALLTYPE get_windowlessVideo(
         VARIANT_BOOL __RPC_FAR *pbEnabled) = 0;

    virtual  HRESULT STDMETHODCALLTYPE put_windowlessVideo(
         VARIANT_BOOL pbEnabled) = 0;

};


class WXDLLIMPEXP_MEDIA wxWMP10MediaBackend : public wxMediaBackendCommonBase
{
public:
    wxWMP10MediaBackend();
    virtual ~wxWMP10MediaBackend();

    virtual bool CreateControl(wxControl* ctrl, wxWindow* parent,
                                     wxWindowID id,
                                     const wxPoint& pos,
                                     const wxSize& size,
                                     long style,
                                     const wxValidator& validator,
                                     const wxString& name);

    virtual bool Play();
    virtual bool Pause();
    virtual bool Stop();

    virtual bool Load(const wxString& fileName);
    virtual bool Load(const wxURI& location);
    virtual bool Load(const wxURI& location, const wxURI& proxy);

    bool DoLoad(const wxString& location);
    void FinishLoad();

    virtual wxMediaState GetState();

    virtual bool SetPosition(wxLongLong where);
    virtual wxLongLong GetPosition();
    virtual wxLongLong GetDuration();

    virtual void Move(int x, int y, int w, int h);
    wxSize GetVideoSize() const;

    virtual double GetPlaybackRate();
    virtual bool SetPlaybackRate(double);

    virtual double GetVolume();
    virtual bool SetVolume(double);

    virtual bool ShowPlayerControls(wxMediaCtrlPlayerControls flags);

    virtual wxLongLong GetDownloadProgress();
    virtual wxLongLong GetDownloadTotal();


#ifdef WXTEST_ATL
        CAxWindow  m_wndView;
#else
        wxActiveXContainer* m_pAX;
#endif
    IWMPPlayer* m_pWMPPlayer;           IWMPSettings* m_pWMPSettings;       IWMPControls* m_pWMPControls;       wxSize m_bestSize;              
    bool m_bWasStateChanged;            wxEvtHandler* m_evthandler;

    friend class wxWMP10MediaEvtHandler;
    wxDECLARE_DYNAMIC_CLASS(wxWMP10MediaBackend);
};

#ifndef WXTEST_ATL
class WXDLLIMPEXP_MEDIA wxWMP10MediaEvtHandler : public wxEvtHandler
{
public:
    wxWMP10MediaEvtHandler(wxWMP10MediaBackend *amb) :
       m_amb(amb)
    {
        m_amb->m_pAX->Connect(m_amb->m_pAX->GetId(),
            wxEVT_ACTIVEX,
            wxActiveXEventHandler(wxWMP10MediaEvtHandler::OnActiveX),
            NULL, this
                              );
    }

    void OnActiveX(wxActiveXEvent& event);

private:
    wxWMP10MediaBackend *m_amb;

    wxDECLARE_NO_COPY_CLASS(wxWMP10MediaEvtHandler);
};
#endif



wxIMPLEMENT_DYNAMIC_CLASS(wxWMP10MediaBackend, wxMediaBackend);

wxWMP10MediaBackend::wxWMP10MediaBackend()
                 :
#ifndef WXTEST_ATL
                m_pAX(NULL),
#endif
                m_pWMPPlayer(NULL),
                m_pWMPSettings(NULL),
                m_pWMPControls(NULL)

{
    m_evthandler = NULL;
}

wxWMP10MediaBackend::~wxWMP10MediaBackend()
{
    if(m_pWMPPlayer)
    {
#ifndef WXTEST_ATL
        m_pAX->DissociateHandle();
        delete m_pAX;

        if (m_evthandler)
        {
            m_ctrl->RemoveEventHandler(m_evthandler);
            delete m_evthandler;
        }
#else
        AtlAxWinTerm();
        _Module.Term();
#endif

        m_pWMPPlayer->Release();
        if (m_pWMPSettings)
            m_pWMPSettings->Release();
        if (m_pWMPControls)
            m_pWMPControls->Release();
    }
}

bool wxWMP10MediaBackend::CreateControl(wxControl* ctrl, wxWindow* parent,
                                     wxWindowID id,
                                     const wxPoint& pos,
                                     const wxSize& size,
                                     long style,
                                     const wxValidator& validator,
                                     const wxString& name)
{
#ifndef WXTEST_ATL
    if( ::CoCreateInstance(CLSID_WMP10, NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IWMPPlayer, (void**)&m_pWMPPlayer) != 0 )
    {
        if( ::CoCreateInstance(CLSID_WMP10ALT, NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IWMPPlayer, (void**)&m_pWMPPlayer) != 0 )
            return false;

        if( m_pWMPPlayer->get_settings(&m_pWMPSettings) != 0)
        {
            m_pWMPPlayer->Release();
            wxLogSysError(wxT("Could not obtain settings from WMP10!"));
            return false;
        }

        if( m_pWMPPlayer->get_controls(&m_pWMPControls) != 0)
        {
            m_pWMPSettings->Release();
            m_pWMPPlayer->Release();
            wxLogSysError(wxT("Could not obtain controls from WMP10!"));
            return false;
        }
    }
#endif

                                    if ( !ctrl->wxControl::Create(parent, id, pos, size,
                            (style & ~wxBORDER_MASK) | wxBORDER_NONE,
                            validator, name) )
        return false;

                    m_ctrl = wxStaticCast(ctrl, wxMediaCtrl);

#ifndef WXTEST_ATL
    m_pAX = new wxActiveXContainer(ctrl, IID_IWMPPlayer, m_pWMPPlayer);

        m_evthandler = new wxWMP10MediaEvtHandler(this);
    m_ctrl->PushEventHandler(m_evthandler);
#else
    _Module.Init(NULL, ::GetModuleHandle(NULL));
    AtlAxWinInit();
    CComPtr<IAxWinHostWindow>  spHost;

    HRESULT hr;
    RECT rcClient;
    ::GetClientRect((HWND)ctrl->GetHandle(), &rcClient);
    m_wndView.Create((HWND)ctrl->GetHandle(), rcClient, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
    hr = m_wndView.QueryHost(&spHost);
    hr = spHost->CreateControl(CComBSTR(wxT("{6BF52A52-394A-11d3-B153-00C04F79FAA6}")), m_wndView, 0);
    hr = m_wndView.QueryControl(&m_pWMPPlayer);

    if( m_pWMPPlayer->get_settings(&m_pWMPSettings) != 0)
    {
        m_pWMPPlayer->Release();
        wxLogSysError(wxT("Could not obtain settings from WMP10!"));
        return false;
    }

    if( m_pWMPPlayer->get_controls(&m_pWMPControls) != 0)
    {
        m_pWMPSettings->Release();
        m_pWMPPlayer->Release();
        wxLogSysError(wxT("Could not obtain controls from WMP10!"));
        return false;
    }
#endif

                
    IWMPPlayer2* pWMPPlayer2;     if(m_pWMPPlayer->QueryInterface(IID_IWMPPlayer2, (void**)&pWMPPlayer2) == 0)
    {
                        pWMPPlayer2->put_windowlessVideo(VARIANT_TRUE);
        pWMPPlayer2->put_stretchToFit(VARIANT_TRUE);
        pWMPPlayer2->Release();
    }

        m_pWMPSettings->put_autoStart(VARIANT_TRUE);
        wxWMP10MediaBackend::ShowPlayerControls(wxMEDIACTRLPLAYERCONTROLS_NONE);
        wxWMP10MediaBackend::SetVolume(1.0);

            m_ctrl->SetBackgroundStyle(wxBG_STYLE_CUSTOM);

        return true;
}

bool wxWMP10MediaBackend::Load(const wxString& fileName)
{
    return DoLoad(fileName);
}

bool wxWMP10MediaBackend::Load(const wxURI& location)
{
    return DoLoad(location.BuildURI());
}

bool wxWMP10MediaBackend::Load(const wxURI& location,
                               const wxURI& proxy)
{
    bool bOK = false;

    IWMPNetwork* pWMPNetwork;
    if( m_pWMPPlayer->get_network(&pWMPNetwork) == 0 )
    {
        long lOldSetting;
        if( pWMPNetwork->getProxySettings(
                    wxBasicString(location.GetScheme()).Get(), &lOldSetting
                                        ) == 0 &&

            pWMPNetwork->setProxySettings(
                    wxBasicString(location.GetScheme()).Get(),                                 2) == 0)         {
            BSTR bsOldName = NULL;
            long lOldPort = 0;

            pWMPNetwork->getProxyName(
                        wxBasicString(location.GetScheme()).Get(),
                        &bsOldName);
            pWMPNetwork->getProxyPort(
                        wxBasicString(location.GetScheme()).Get(),
                        &lOldPort);

            long lPort;
            wxString server;
            if(proxy.IsReference())
            {
                server = proxy.GetScheme();
                lPort = wxAtoi(proxy.GetPath());
            }
            else
            {
                server = proxy.GetServer();
                lPort = wxAtoi(proxy.GetPort());
            }

            if( pWMPNetwork->setProxyName(
                        wxBasicString(location.GetScheme()).Get(),                         wxBasicString(server).Get() ) == 0  &&

                pWMPNetwork->setProxyPort(
                        wxBasicString(location.GetScheme()).Get(),                         lPort
                                         ) == 0
              )
            {
                bOK = DoLoad(location.BuildURI());

                pWMPNetwork->setProxySettings(
                    wxBasicString(location.GetScheme()).Get(),                                 lOldSetting);
                if(bsOldName)
                    pWMPNetwork->setProxyName(
                        wxBasicString(location.GetScheme()).Get(),                                     bsOldName);

                if(lOldPort)
                    pWMPNetwork->setProxyPort(
                        wxBasicString(location.GetScheme()).Get(),                                 lOldPort);

                pWMPNetwork->Release();
            }
            else
                pWMPNetwork->Release();

        }
        else
            pWMPNetwork->Release();

    }

    return bOK;
}

bool wxWMP10MediaBackend::DoLoad(const wxString& location)
{
    HRESULT hr;

#if 0                     
    IWMPCore3* pWMPCore3;
    double outDuration;
    if(m_pWMPPlayer->QueryInterface(IID_IWMPCore3, (void**) &pWMPCore3) == 0)
    {
        IWMPMedia* pWMPMedia;

        if( (hr = pWMPCore3->newMedia(wxBasicString(location).Get(),
                               &pWMPMedia)) == 0)
        {
                        pWMPMedia->get_duration(&outDuration);
            pWMPCore3->put_currentMedia(pWMPMedia);
            pWMPMedia->Release();
        }

        pWMPCore3->Release();
    }
    else
#endif
    {
                hr = m_pWMPPlayer->put_URL( wxBasicString(location).Get() );
    }

    if(FAILED(hr))
    {
        wxWMP10LOG(hr);
        return false;
    }

    return true;
}

void wxWMP10MediaBackend::FinishLoad()
{
                                IWMPMedia* pWMPMedia;
    if(m_pWMPPlayer->get_currentMedia(&pWMPMedia) == 0)
    {
        pWMPMedia->get_imageSourceWidth((long*)&m_bestSize.x);
        pWMPMedia->get_imageSourceHeight((long*)&m_bestSize.y);
        pWMPMedia->Release();
    }
    else
    {
        wxLogDebug(wxT("Could not get media"));
    }

    NotifyMovieLoaded();
}

bool wxWMP10MediaBackend::ShowPlayerControls(wxMediaCtrlPlayerControls flags)
{
    if(!flags)
    {
        m_pWMPPlayer->put_enabled(VARIANT_FALSE);
        m_pWMPPlayer->put_uiMode(wxBasicString(wxT("none")).Get());
    }
    else
    {
                m_pWMPPlayer->put_uiMode(wxBasicString(wxT("full")).Get());
        m_pWMPPlayer->put_enabled(VARIANT_TRUE);
    }

    return true;
}

bool wxWMP10MediaBackend::Play()
{
        HRESULT hr = m_pWMPControls->play();
    if(SUCCEEDED(hr))
    {
       m_bWasStateChanged = true;
       return true;
    }
    wxWMP10LOG(hr);
    return false;
}

bool wxWMP10MediaBackend::Pause()
{
    HRESULT hr = m_pWMPControls->pause();
    if(SUCCEEDED(hr))
    {
        m_bWasStateChanged = true;
        return true;
    }
    wxWMP10LOG(hr);
    return false;
}

bool wxWMP10MediaBackend::Stop()
{
    HRESULT hr = m_pWMPControls->stop();
    if(SUCCEEDED(hr))
    {
                wxWMP10MediaBackend::SetPosition(0);
        m_bWasStateChanged = true;
        return true;
    }
    wxWMP10LOG(hr);
    return false;
}

bool wxWMP10MediaBackend::SetPosition(wxLongLong where)
{
    HRESULT hr = m_pWMPControls->put_currentPosition(
                        ((LONGLONG)where.GetValue()) / 1000.0
                                     );
    if(FAILED(hr))
    {
        wxWMP10LOG(hr);
        return false;
    }

    return true;
}

wxLongLong wxWMP10MediaBackend::GetPosition()
{
    double outCur;
    HRESULT hr = m_pWMPControls->get_currentPosition(&outCur);
    if(FAILED(hr))
    {
        wxWMP10LOG(hr);
        return 0;
    }

        outCur *= 1000;
    wxLongLong ll;
    ll.Assign(outCur);

    return ll;
}

double wxWMP10MediaBackend::GetVolume()
{
    long lVolume;
    HRESULT hr = m_pWMPSettings->get_volume(&lVolume);
    if(FAILED(hr))
    {
        wxWMP10LOG(hr);
        return 0.0;
    }

    return (double)lVolume / 100.0;
}

bool wxWMP10MediaBackend::SetVolume(double dVolume)
{
    HRESULT hr = m_pWMPSettings->put_volume( (long) (dVolume * 100.0) );
    if(FAILED(hr))
    {
        wxWMP10LOG(hr);
        return false;
    }
    return true;
}

wxLongLong wxWMP10MediaBackend::GetDuration()
{
    double outDuration = 0.0;

    IWMPMedia* pWMPMedia;
    if(m_pWMPPlayer->get_currentMedia(&pWMPMedia) == 0)
    {
        if(pWMPMedia->get_duration(&outDuration) != 0)
        {
            wxLogDebug(wxT("get_duration failed"));
        }
        pWMPMedia->Release();
    }


        outDuration *= 1000;
    wxLongLong ll;
    ll.Assign(outDuration);

    return ll;
}

wxMediaState wxWMP10MediaBackend::GetState()
{
    WMPPlayState nState;
    HRESULT hr = m_pWMPPlayer->get_playState(&nState);
    if(FAILED(hr))
    {
        wxWMP10LOG(hr);
        return wxMEDIASTATE_STOPPED;
    }

    switch(nState)
    {
    case wmppsPaused:
        return wxMEDIASTATE_PAUSED;
    case wmppsPlaying:
        return wxMEDIASTATE_PLAYING;
    default:
        return wxMEDIASTATE_STOPPED;
    }
}

double wxWMP10MediaBackend::GetPlaybackRate()
{
    double dRate;
    HRESULT hr = m_pWMPSettings->get_rate(&dRate);
    if(FAILED(hr))
    {
        wxWMP10LOG(hr);
        return 0.0;
    }
    return dRate;
}

bool wxWMP10MediaBackend::SetPlaybackRate(double dRate)
{
    HRESULT hr = m_pWMPSettings->put_rate(dRate);
    if(FAILED(hr))
    {
        wxWMP10LOG(hr);
        return false;
    }

    return true;
}

wxSize wxWMP10MediaBackend::GetVideoSize() const
{
    return m_bestSize;
}

void wxWMP10MediaBackend::Move(int WXUNUSED(x), int WXUNUSED(y),
#ifdef WXTEST_ATL
                            int w, int h
#else
                            int WXUNUSED(w), int WXUNUSED(h)
#endif
                            )
{
#ifdef WXTEST_ATL
    m_wndView.MoveWindow(0,0,w,h);
#endif
}

wxLongLong wxWMP10MediaBackend::GetDownloadProgress()
{
    IWMPNetwork* pWMPNetwork;
    if( m_pWMPPlayer->get_network(&pWMPNetwork) == 0 )
    {
        long lPercentProg;
        if(pWMPNetwork->get_downloadProgress(&lPercentProg) == 0)
        {
            pWMPNetwork->Release();
            return (GetDownloadTotal() * lPercentProg) / 100;
        }
        pWMPNetwork->Release();
    }
    return 0;
}

wxLongLong wxWMP10MediaBackend::GetDownloadTotal()
{
    IWMPMedia* pWMPMedia;
    if(m_pWMPPlayer->get_currentMedia(&pWMPMedia) == 0)
    {
        BSTR bsOut;
        pWMPMedia->getItemInfo(wxBasicString(wxT("FileSize")).Get(),
                               &bsOut);

        wxString sFileSize = wxConvertStringFromOle(bsOut);
        long lFS;
        sFileSize.ToLong(&lFS);
        pWMPMedia->Release();
        return lFS;
    }

    return 0;
}


#ifndef WXTEST_ATL
void wxWMP10MediaEvtHandler::OnActiveX(wxActiveXEvent& event)
{
    switch(event.GetDispatchId())
    {
    case 0x000013ed:         if(event.ParamCount() >= 1)
        {
            switch (event[0].GetInteger())
            {
            case wmppsMediaEnded:                 if ( m_amb->SendStopEvent() )
                {
                                                                                
                                        m_amb->QueueFinishEvent();
                }
                break;

            case wmppsStopped:                 m_amb->QueueStopEvent();
                break;
            case wmppsPaused:                 m_amb->QueuePauseEvent();
                break;
            case wmppsPlaying:                 m_amb->QueuePlayEvent();
                break;
            default:
                break;
            }
        }
        else
            event.Skip();
        break;

    case 0x00001389:         if(event.ParamCount() >= 1)
        {
            int nState = event[0].GetInteger();
            if(nState == wmposMediaOpen)
            {
                                m_amb->m_bWasStateChanged = false;
                m_amb->FinishLoad();
                if(!m_amb->m_bWasStateChanged)
                    m_amb->Stop();
            }
        }
        else
            event.Skip();
        break;

    case 0x0000196e:         m_amb->m_ctrl->SetFocus();
        break;

    default:
        event.Skip();
        return;
    }
}

#endif

#include "wx/link.h"
wxFORCE_LINK_THIS_MODULE(wxmediabackend_wmp10)

#endif 