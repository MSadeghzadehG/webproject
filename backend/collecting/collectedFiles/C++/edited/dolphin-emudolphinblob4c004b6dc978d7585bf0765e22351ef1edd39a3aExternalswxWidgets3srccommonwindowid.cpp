
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/intl.h"
#endif 
#include "wx/hashmap.h"


namespace
{

#if wxUSE_AUTOID_MANAGEMENT


static const wxUint8 ID_FREE = 0;
static const wxUint8 ID_STARTCOUNT = 1;
static const wxUint8 ID_COUNTTOOLARGE = 254;
static const wxUint8 ID_RESERVED = 255;

wxUint8 gs_autoIdsRefCount[wxID_AUTO_HIGHEST - wxID_AUTO_LOWEST + 1] = { 0 };

wxLongToLongHashMap *gs_autoIdsLargeRefCount = NULL;

wxWindowID gs_nextAutoId = wxID_AUTO_LOWEST;

void ReserveIdRefCount(wxWindowID winid)
{
    wxCHECK_RET(winid >= wxID_AUTO_LOWEST && winid <= wxID_AUTO_HIGHEST,
            wxT("invalid id range"));

    winid -= wxID_AUTO_LOWEST;

    wxCHECK_RET(gs_autoIdsRefCount[winid] == ID_FREE,
            wxT("id already in use or already reserved"));
    gs_autoIdsRefCount[winid] = ID_RESERVED;
}

void UnreserveIdRefCount(wxWindowID winid)
{
    wxCHECK_RET(winid >= wxID_AUTO_LOWEST && winid <= wxID_AUTO_HIGHEST,
            wxT("invalid id range"));

    winid -= wxID_AUTO_LOWEST;

    wxCHECK_RET(gs_autoIdsRefCount[winid] == ID_RESERVED,
            wxT("id already in use or not reserved"));
    gs_autoIdsRefCount[winid] = ID_FREE;
}

int GetIdRefCount(wxWindowID winid)
{
    winid -= wxID_AUTO_LOWEST;
    int refCount = gs_autoIdsRefCount[winid];
    if (refCount == ID_COUNTTOOLARGE)
        refCount = (*gs_autoIdsLargeRefCount)[winid];
    return refCount;
}

void IncIdRefCount(wxWindowID winid)
{
    winid -= wxID_AUTO_LOWEST;

    wxCHECK_RET(gs_autoIdsRefCount[winid] != ID_FREE, wxT("id should first be reserved"));

    if(gs_autoIdsRefCount[winid] == ID_RESERVED)
    {
        gs_autoIdsRefCount[winid] = ID_STARTCOUNT;
    }
    else if (gs_autoIdsRefCount[winid] >= ID_COUNTTOOLARGE-1)
    {
        if (gs_autoIdsRefCount[winid] == ID_COUNTTOOLARGE-1)
        {
                        if (!gs_autoIdsLargeRefCount)
                gs_autoIdsLargeRefCount = new wxLongToLongHashMap;
            (*gs_autoIdsLargeRefCount)[winid] = ID_COUNTTOOLARGE-1;

            gs_autoIdsRefCount[winid] = ID_COUNTTOOLARGE;
        }
        ++(*gs_autoIdsLargeRefCount)[winid];
    }
    else
    {
        gs_autoIdsRefCount[winid]++;
    }
}

void DecIdRefCount(wxWindowID winid)
{
    winid -= wxID_AUTO_LOWEST;

    wxCHECK_RET(gs_autoIdsRefCount[winid] != ID_FREE, wxT("id count already 0"));

            if(gs_autoIdsRefCount[winid] == ID_RESERVED)
    {
        wxFAIL_MSG(wxT("reserve id being decreased"));
        gs_autoIdsRefCount[winid] = ID_FREE;
    }
    else if(gs_autoIdsRefCount[winid] == ID_COUNTTOOLARGE)
    {
        long &largeCount = (*gs_autoIdsLargeRefCount)[winid];
        --largeCount;
        if (largeCount == 0)
        {
            gs_autoIdsLargeRefCount->erase (winid);
            gs_autoIdsRefCount[winid] = ID_FREE;

            if (gs_autoIdsLargeRefCount->empty())
                wxDELETE (gs_autoIdsLargeRefCount);
        }
    }
    else
        gs_autoIdsRefCount[winid]--;
}

#else 
static wxWindowID gs_nextAutoId = wxID_AUTO_HIGHEST;

#endif

} 

#if wxUSE_AUTOID_MANAGEMENT

void wxWindowIDRef::Assign(wxWindowID winid)
{
    if ( winid != m_id )
    {
                if ( m_id >= wxID_AUTO_LOWEST && m_id <= wxID_AUTO_HIGHEST )
            DecIdRefCount(m_id);

        m_id = winid;

                if ( m_id >= wxID_AUTO_LOWEST && m_id <= wxID_AUTO_HIGHEST )
            IncIdRefCount(m_id);
    }
}

#endif 


wxWindowID wxIdManager::ReserveId(int count)
{
    wxASSERT_MSG(count > 0, wxT("can't allocate less than 1 id"));


#if wxUSE_AUTOID_MANAGEMENT
    if ( gs_nextAutoId + count - 1 <= wxID_AUTO_HIGHEST )
    {
        wxWindowID winid = gs_nextAutoId;

        while(count--)
        {
            ReserveIdRefCount(gs_nextAutoId++);
        }

        return winid;
    }
    else
    {
        int found = 0;

        for(wxWindowID winid = wxID_AUTO_LOWEST; winid <= wxID_AUTO_HIGHEST; winid++)
        {
            if(GetIdRefCount(winid) == 0)
            {
                found++;
                if(found == count)
                {
                                                                                                                                                                                                        if(winid >= gs_nextAutoId)
                        gs_nextAutoId = winid + 1;

                    while(count--)
                        ReserveIdRefCount(winid--);

                    return winid + 1;
                }
            }
            else
            {
                found = 0;
            }
        }
    }

    wxLogError(_("Out of window IDs.  Recommend shutting down application."));
    return wxID_NONE;
#else         wxWindowID winid;

    winid = gs_nextAutoId - count + 1;

    if ( winid >= wxID_AUTO_LOWEST && winid <= wxID_AUTO_HIGHEST )
    {
                if(winid == wxID_AUTO_LOWEST)
            gs_nextAutoId = wxID_AUTO_HIGHEST;
        else
            gs_nextAutoId = winid - 1;

        return winid;
    }
    else
    {
                                gs_nextAutoId = wxID_AUTO_HIGHEST - count;
        return gs_nextAutoId + 1;
    }
#endif }

void wxIdManager::UnreserveId(wxWindowID winid, int count)
{
    wxASSERT_MSG(count > 0, wxT("can't unreserve less than 1 id"));

#if wxUSE_AUTOID_MANAGEMENT
    while (count--)
        UnreserveIdRefCount(winid++);
#else
    wxUnusedVar(winid);
    wxUnusedVar(count);
#endif
}

