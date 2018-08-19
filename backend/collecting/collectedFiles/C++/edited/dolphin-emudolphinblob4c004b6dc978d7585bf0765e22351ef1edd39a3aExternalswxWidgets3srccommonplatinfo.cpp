


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/platinfo.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/utils.h"
#endif 
#include "wx/apptrait.h"

static wxPlatformInfo gs_platInfo(wxPORT_UNKNOWN);


static const wxChar* const wxOperatingSystemIdNames[] =
{
    wxT("Apple Mac OS"),
    wxT("Apple Mac OS X"),

    wxT("Microsoft Windows 9X"),
    wxT("Microsoft Windows NT"),
    wxT("Microsoft Windows Micro"),
    wxT("Microsoft Windows CE"),

    wxT("Linux"),
    wxT("FreeBSD"),
    wxT("OpenBSD"),
    wxT("NetBSD"),

    wxT("SunOS"),
    wxT("AIX"),
    wxT("HPUX"),

    wxT("Other Unix"),
    wxT("Other Unix"),

    wxT("DOS"),
    wxT("OS/2"),

};

static const wxChar* const wxPortIdNames[] =
{
    wxT("wxBase"),
    wxT("wxMSW"),
    wxT("wxMotif"),
    wxT("wxGTK"),
    wxT("wxDFB"),
    wxT("wxX11"),
    wxT("wxOS2"),
    wxT("wxMac"),
    wxT("wxCocoa"),
    wxT("wxWinCE"),
    wxT("wxQT")
};

static const wxChar* const wxArchitectureNames[] =
{
    wxT("32 bit"),
    wxT("64 bit")
};

static const wxChar* const wxEndiannessNames[] =
{
    wxT("Big endian"),
    wxT("Little endian"),
    wxT("PDP endian")
};


static unsigned wxGetIndexFromEnumValue(int value)
{
    wxCHECK_MSG( value, (unsigned)-1, wxT("invalid enum value") );

    int n = 0;
    while ( !(value & 1) )
    {
        value >>= 1;
        n++;
    }

    wxASSERT_MSG( value == 1, wxT("more than one bit set in enum value") );

    return n;
}


wxPlatformInfo::wxPlatformInfo()
{
        *this = Get();
}

wxPlatformInfo::wxPlatformInfo(wxPortId pid, int tkMajor, int tkMinor,
                               wxOperatingSystemId id, int osMajor, int osMinor,
                               wxArchitecture arch,
                               wxEndianness endian,
                               bool usingUniversal)
{
    m_initializedForCurrentPlatform = false;

    m_tkVersionMajor = tkMajor;
    m_tkVersionMinor = tkMinor;
    m_tkVersionMicro = -1;
    m_port = pid;
    m_usingUniversal = usingUniversal;

    m_os = id;
    m_osVersionMajor = osMajor;
    m_osVersionMinor = osMinor;
    m_osVersionMicro = -1;

    m_endian = endian;
    m_arch = arch;
}

bool wxPlatformInfo::operator==(const wxPlatformInfo &t) const
{
    return m_tkVersionMajor == t.m_tkVersionMajor &&
           m_tkVersionMinor == t.m_tkVersionMinor &&
           m_tkVersionMicro == t.m_tkVersionMicro &&
           m_osVersionMajor == t.m_osVersionMajor &&
           m_osVersionMinor == t.m_osVersionMinor &&
           m_osVersionMicro == t.m_osVersionMicro &&
           m_os == t.m_os &&
           m_osDesc == t.m_osDesc &&
           m_ldi == t.m_ldi &&
           m_desktopEnv == t.m_desktopEnv &&
           m_port == t.m_port &&
           m_usingUniversal == t.m_usingUniversal &&
           m_arch == t.m_arch &&
           m_endian == t.m_endian;
}

void wxPlatformInfo::InitForCurrentPlatform()
{
    m_initializedForCurrentPlatform = true;

        const wxAppTraits * const traits = wxTheApp ? wxTheApp->GetTraits() : NULL;
    if ( !traits )
    {
        wxFAIL_MSG( wxT("failed to initialize wxPlatformInfo") );

        m_port = wxPORT_UNKNOWN;
        m_usingUniversal = false;
        m_tkVersionMajor =
        m_tkVersionMinor =
        m_tkVersionMicro = 0;
    }
    else
    {
        m_port = traits->GetToolkitVersion(&m_tkVersionMajor, &m_tkVersionMinor,
                                           &m_tkVersionMicro);
        m_usingUniversal = traits->IsUsingUniversalWidgets();
        m_desktopEnv = traits->GetDesktopEnvironment();
    }

    m_os = wxGetOsVersion(&m_osVersionMajor, &m_osVersionMinor, &m_osVersionMicro);
    m_osDesc = wxGetOsDescription();
    m_endian = wxIsPlatformLittleEndian() ? wxENDIAN_LITTLE : wxENDIAN_BIG;
    m_arch = wxIsPlatform64Bit() ? wxARCH_64 : wxARCH_32;

#ifdef __LINUX__
    m_ldi = wxGetLinuxDistributionInfo();
#endif
    }


const wxPlatformInfo& wxPlatformInfo::Get()
{
    static bool initialized = false;
    if ( !initialized )
    {
        gs_platInfo.InitForCurrentPlatform();
        initialized = true;
    }

    return gs_platInfo;
}


wxString wxPlatformInfo::GetOperatingSystemDirectory()
{
    return wxGetOSDirectory();
}




wxString wxPlatformInfo::GetOperatingSystemFamilyName(wxOperatingSystemId os)
{
    const wxChar* string = wxT("Unknown");
    if ( os & wxOS_MAC )
        string = wxT("Macintosh");
    else if ( os & wxOS_WINDOWS )
        string = wxT("Windows");
    else if ( os & wxOS_UNIX )
        string = wxT("Unix");
    else if ( os == wxOS_DOS )
        string = wxT("DOS");
    else if ( os == wxOS_OS2 )
        string = wxT("OS/2");

    return string;
}

wxString wxPlatformInfo::GetOperatingSystemIdName(wxOperatingSystemId os)
{
    const unsigned idx = wxGetIndexFromEnumValue(os);

    wxCHECK_MSG( idx < WXSIZEOF(wxOperatingSystemIdNames), wxEmptyString,
                 wxT("invalid OS id") );

    return wxOperatingSystemIdNames[idx];
}

wxString wxPlatformInfo::GetPortIdName(wxPortId port, bool usingUniversal)
{
    const unsigned idx = wxGetIndexFromEnumValue(port);

    wxCHECK_MSG( idx < WXSIZEOF(wxPortIdNames), wxEmptyString,
                 wxT("invalid port id") );

    wxString ret = wxPortIdNames[idx];

    if ( usingUniversal )
        ret += wxT("/wxUniversal");

    return ret;
}

wxString wxPlatformInfo::GetPortIdShortName(wxPortId port, bool usingUniversal)
{
    const unsigned idx = wxGetIndexFromEnumValue(port);

    wxCHECK_MSG( idx < WXSIZEOF(wxPortIdNames), wxEmptyString,
                 wxT("invalid port id") );

    wxString ret = wxPortIdNames[idx];
    ret = ret.Mid(2).Lower();       
    if ( usingUniversal )
        ret += wxT("univ");

    return ret;
}

wxString wxPlatformInfo::GetArchName(wxArchitecture arch)
{
    wxCOMPILE_TIME_ASSERT( WXSIZEOF(wxArchitectureNames) == wxARCH_MAX,
                           wxArchitectureNamesMismatch );

    return wxArchitectureNames[arch];
}

wxString wxPlatformInfo::GetEndiannessName(wxEndianness end)
{
    wxCOMPILE_TIME_ASSERT( WXSIZEOF(wxEndiannessNames) == wxENDIAN_MAX,
                           wxEndiannessNamesMismatch );

    return wxEndiannessNames[end];
}

bool wxPlatformInfo::CheckOSVersion(int major, int minor, int micro) const
{
                if (m_initializedForCurrentPlatform)
        return wxCheckOsVersion(major, minor, micro);
    else
        return DoCheckVersion(GetOSMajorVersion(),
                            GetOSMinorVersion(),
                            GetOSMicroVersion(),
                            major,
                            minor,
                            micro);
}


wxOperatingSystemId wxPlatformInfo::GetOperatingSystemId(const wxString &str)
{
    for ( size_t i = 0; i < WXSIZEOF(wxOperatingSystemIdNames); i++ )
    {
        if ( wxString(wxOperatingSystemIdNames[i]).CmpNoCase(str) == 0 )
            return (wxOperatingSystemId)(1 << i);
    }

    return wxOS_UNKNOWN;
}

wxPortId wxPlatformInfo::GetPortId(const wxString &str)
{
        for ( size_t i = 0; i < WXSIZEOF(wxPortIdNames); i++ )
    {
        wxPortId current = (wxPortId)(1 << i);

        if ( wxString(wxPortIdNames[i]).CmpNoCase(str) == 0 ||
             GetPortIdShortName(current, true).CmpNoCase(str) == 0 ||
             GetPortIdShortName(current, false).CmpNoCase(str) == 0 )
            return current;
    }

    return wxPORT_UNKNOWN;
}

wxArchitecture wxPlatformInfo::GetArch(const wxString &arch)
{
    if ( arch.Contains(wxT("32")) )
        return wxARCH_32;

    if ( arch.Contains(wxT("64")) )
        return wxARCH_64;

    return wxARCH_INVALID;
}

wxEndianness wxPlatformInfo::GetEndianness(const wxString& end)
{
    const wxString endl(end.Lower());
    if ( endl.StartsWith(wxT("little")) )
        return wxENDIAN_LITTLE;

    if ( endl.StartsWith(wxT("big")) )
        return wxENDIAN_BIG;

    return wxENDIAN_INVALID;
}

