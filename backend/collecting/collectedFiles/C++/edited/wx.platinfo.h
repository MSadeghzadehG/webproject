
#ifndef _WX_PLATINFO_H_
#define _WX_PLATINFO_H_

#include "wx/string.h"




enum wxOperatingSystemId
{
    wxOS_UNKNOWN = 0,                 
    wxOS_MAC_OS         = 1 << 0,         wxOS_MAC_OSX_DARWIN = 1 << 1,         wxOS_MAC = wxOS_MAC_OS|wxOS_MAC_OSX_DARWIN,

    wxOS_WINDOWS_9X     = 1 << 2,         wxOS_WINDOWS_NT     = 1 << 3,         wxOS_WINDOWS_MICRO  = 1 << 4,         wxOS_WINDOWS_CE     = 1 << 5,         wxOS_WINDOWS = wxOS_WINDOWS_9X      |
                   wxOS_WINDOWS_NT      |
                   wxOS_WINDOWS_MICRO   |
                   wxOS_WINDOWS_CE,

    wxOS_UNIX_LINUX     = 1 << 6,           wxOS_UNIX_FREEBSD   = 1 << 7,           wxOS_UNIX_OPENBSD   = 1 << 8,           wxOS_UNIX_NETBSD    = 1 << 9,           wxOS_UNIX_SOLARIS   = 1 << 10,          wxOS_UNIX_AIX       = 1 << 11,          wxOS_UNIX_HPUX      = 1 << 12,          wxOS_UNIX = wxOS_UNIX_LINUX     |
                wxOS_UNIX_FREEBSD   |
                wxOS_UNIX_OPENBSD   |
                wxOS_UNIX_NETBSD    |
                wxOS_UNIX_SOLARIS   |
                wxOS_UNIX_AIX       |
                wxOS_UNIX_HPUX,

    
    wxOS_DOS            = 1 << 15,          wxOS_OS2            = 1 << 16       };

enum wxPortId
{
    wxPORT_UNKNOWN  = 0,            
    wxPORT_BASE     = 1 << 0,       
    wxPORT_MSW      = 1 << 1,           wxPORT_MOTIF    = 1 << 2,           wxPORT_GTK      = 1 << 3,           wxPORT_DFB      = 1 << 4,           wxPORT_X11      = 1 << 5,           wxPORT_PM       = 1 << 6,           wxPORT_OS2      = wxPORT_PM,        wxPORT_MAC      = 1 << 7,           wxPORT_OSX      = wxPORT_MAC,       wxPORT_COCOA    = 1 << 8,           wxPORT_WINCE    = 1 << 9,           wxPORT_QT       = 1 << 10       };

enum wxArchitecture
{
    wxARCH_INVALID = -1,        
    wxARCH_32,                      wxARCH_64,

    wxARCH_MAX
};


enum wxEndianness
{
    wxENDIAN_INVALID = -1,      
    wxENDIAN_BIG,                   wxENDIAN_LITTLE,                wxENDIAN_PDP,               
    wxENDIAN_MAX
};

struct wxLinuxDistributionInfo
{
    wxString Id;
    wxString Release;
    wxString CodeName;
    wxString Description;

    bool operator==(const wxLinuxDistributionInfo& ldi) const
    {
        return Id == ldi.Id &&
               Release == ldi.Release &&
               CodeName == ldi.CodeName &&
               Description == ldi.Description;
    }

    bool operator!=(const wxLinuxDistributionInfo& ldi) const
    { return !(*this == ldi); }
};



class WXDLLIMPEXP_BASE wxPlatformInfo
{
public:
    wxPlatformInfo();
    wxPlatformInfo(wxPortId pid,
                   int tkMajor = -1, int tkMinor = -1,
                   wxOperatingSystemId id = wxOS_UNKNOWN,
                   int osMajor = -1, int osMinor = -1,
                   wxArchitecture arch = wxARCH_INVALID,
                   wxEndianness endian = wxENDIAN_INVALID,
                   bool usingUniversal = false);

    
    bool operator==(const wxPlatformInfo &t) const;

    bool operator!=(const wxPlatformInfo &t) const
        { return !(*this == t); }

            static const wxPlatformInfo& Get();



        
    static wxOperatingSystemId GetOperatingSystemId(const wxString &name);
    static wxPortId GetPortId(const wxString &portname);

    static wxArchitecture GetArch(const wxString &arch);
    static wxEndianness GetEndianness(const wxString &end);

        
    static wxString GetOperatingSystemFamilyName(wxOperatingSystemId os);
    static wxString GetOperatingSystemIdName(wxOperatingSystemId os);
    static wxString GetPortIdName(wxPortId port, bool usingUniversal);
    static wxString GetPortIdShortName(wxPortId port, bool usingUniversal);

    static wxString GetArchName(wxArchitecture arch);
    static wxString GetEndiannessName(wxEndianness end);


        
    int GetOSMajorVersion() const
        { return m_osVersionMajor; }
    int GetOSMinorVersion() const
        { return m_osVersionMinor; }
    int GetOSMicroVersion() const
        { return m_osVersionMicro; }

        bool CheckOSVersion(int major, int minor, int micro = 0) const;

    int GetToolkitMajorVersion() const
        { return m_tkVersionMajor; }
    int GetToolkitMinorVersion() const
        { return m_tkVersionMinor; }
    int GetToolkitMicroVersion() const
        { return m_tkVersionMicro; }

    bool CheckToolkitVersion(int major, int minor, int micro = 0) const
    {
        return DoCheckVersion(GetToolkitMajorVersion(),
                              GetToolkitMinorVersion(),
                              GetToolkitMicroVersion(),
                              major,
                              minor,
                              micro);
    }

    bool IsUsingUniversalWidgets() const
        { return m_usingUniversal; }

    wxOperatingSystemId GetOperatingSystemId() const
        { return m_os; }
    wxLinuxDistributionInfo GetLinuxDistributionInfo() const
        { return m_ldi; }
    wxPortId GetPortId() const
        { return m_port; }
    wxArchitecture GetArchitecture() const
        { return m_arch; }
    wxEndianness GetEndianness() const
        { return m_endian; }


        
    wxString GetOperatingSystemFamilyName() const
        { return GetOperatingSystemFamilyName(m_os); }
    wxString GetOperatingSystemIdName() const
        { return GetOperatingSystemIdName(m_os); }
    wxString GetPortIdName() const
        { return GetPortIdName(m_port, m_usingUniversal); }
    wxString GetPortIdShortName() const
        { return GetPortIdShortName(m_port, m_usingUniversal); }
    wxString GetArchName() const
        { return GetArchName(m_arch); }
    wxString GetEndiannessName() const
        { return GetEndiannessName(m_endian); }
    wxString GetOperatingSystemDescription() const
        { return m_osDesc; }
    wxString GetDesktopEnvironment() const
        { return m_desktopEnv; }

    static wxString GetOperatingSystemDirectory();
                                
        
    void SetOSVersion(int major, int minor, int micro = 0)
    {
        m_osVersionMajor = major;
        m_osVersionMinor = minor;
        m_osVersionMicro = micro;
    }

    void SetToolkitVersion(int major, int minor, int micro = 0)
    {
        m_tkVersionMajor = major;
        m_tkVersionMinor = minor;
        m_tkVersionMicro = micro;
    }

    void SetOperatingSystemId(wxOperatingSystemId n)
        { m_os = n; }
    void SetOperatingSystemDescription(const wxString& desc)
        { m_osDesc = desc; }
    void SetPortId(wxPortId n)
        { m_port = n; }
    void SetArchitecture(wxArchitecture n)
        { m_arch = n; }
    void SetEndianness(wxEndianness n)
        { m_endian = n; }

    void SetDesktopEnvironment(const wxString& de)
        { m_desktopEnv = de; }
    void SetLinuxDistributionInfo(const wxLinuxDistributionInfo& di)
        { m_ldi = di; }


        
    bool IsOk() const
    {
        return m_osVersionMajor != -1 && m_osVersionMinor != -1 &&
               m_osVersionMicro != -1 &&
               m_os != wxOS_UNKNOWN &&
               !m_osDesc.IsEmpty() &&
               m_tkVersionMajor != -1 && m_tkVersionMinor != -1 &&
               m_tkVersionMicro != -1 &&
               m_port != wxPORT_UNKNOWN &&
               m_arch != wxARCH_INVALID &&
               m_endian != wxENDIAN_INVALID;

                   }


protected:
    static bool DoCheckVersion(int majorCur, int minorCur, int microCur,
                               int major, int minor, int micro)
    {
        return majorCur > major
            || (majorCur == major && minorCur > minor)
            || (majorCur == major && minorCur == minor && microCur >= micro);
    }

    bool m_initializedForCurrentPlatform;

    void InitForCurrentPlatform();


        
            int m_osVersionMajor,
        m_osVersionMinor,
        m_osVersionMicro;

        wxOperatingSystemId m_os;

        wxString m_osDesc;


        
    wxString m_desktopEnv;
    wxLinuxDistributionInfo m_ldi;


        
            int m_tkVersionMajor, m_tkVersionMinor, m_tkVersionMicro;

        wxPortId m_port;

        bool m_usingUniversal;


        
        wxArchitecture m_arch;

        wxEndianness m_endian;
};



#endif 