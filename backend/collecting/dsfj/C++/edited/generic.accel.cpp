


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_ACCEL

#ifndef WX_PRECOMP
    #include "wx/list.h"
    #include "wx/event.h"
#endif 
#include "wx/accel.h"

#include <ctype.h>


WX_DECLARE_LIST(wxAcceleratorEntry, wxAccelList);
#include "wx/listimpl.cpp"
WX_DEFINE_LIST(wxAccelList)


class wxAccelRefData : public wxObjectRefData
{
public:
    wxAccelRefData()
    {
    }

    wxAccelRefData(const wxAccelRefData& data)
        : wxObjectRefData()
    {
        m_accels = data.m_accels;
    }

    virtual ~wxAccelRefData()
    {
        WX_CLEAR_LIST(wxAccelList, m_accels);
    }

    wxAccelList m_accels;
};

#define M_ACCELDATA ((wxAccelRefData *)m_refData)




wxIMPLEMENT_DYNAMIC_CLASS(wxAcceleratorTable, wxObject);

wxAcceleratorTable::wxAcceleratorTable()
{
}

wxAcceleratorTable::wxAcceleratorTable(int n, const wxAcceleratorEntry entries[])
{
    m_refData = new wxAccelRefData;

    for ( int i = 0; i < n; i++ )
    {
        const wxAcceleratorEntry& entry = entries[i];

        int keycode = entry.GetKeyCode();
        if ( wxIsascii(keycode) )
            keycode = wxToupper(keycode);

        M_ACCELDATA->m_accels.Append(new wxAcceleratorEntry(entry.GetFlags(),
                                                            keycode,
                                                            entry.GetCommand()));
    }
}

wxAcceleratorTable::~wxAcceleratorTable()
{
}

bool wxAcceleratorTable::IsOk() const
{
    return m_refData != NULL;
}


void wxAcceleratorTable::Add(const wxAcceleratorEntry& entry)
{
    AllocExclusive();

    if ( !m_refData )
    {
        m_refData = new wxAccelRefData;
    }

    M_ACCELDATA->m_accels.Append(new wxAcceleratorEntry(entry));
}

void wxAcceleratorTable::Remove(const wxAcceleratorEntry& entry)
{
    AllocExclusive();

    wxAccelList::compatibility_iterator node = M_ACCELDATA->m_accels.GetFirst();
    while ( node )
    {
        const wxAcceleratorEntry *entryCur = node->GetData();

                                if ((entryCur->GetKeyCode() == entry.GetKeyCode()) &&
            (entryCur->GetFlags() == entry.GetFlags()))
        {
            delete node->GetData();
            M_ACCELDATA->m_accels.Erase(node);

            return;
        }

        node = node->GetNext();
    }

    wxFAIL_MSG(wxT("deleting inexistent accel from wxAcceleratorTable"));
}


const wxAcceleratorEntry *
wxAcceleratorTable::GetEntry(const wxKeyEvent& event) const
{
    if ( !IsOk() )
    {
                return NULL;
    }

    wxAccelList::compatibility_iterator node = M_ACCELDATA->m_accels.GetFirst();
    while ( node )
    {
        const wxAcceleratorEntry *entry = node->GetData();

                if ( event.m_keyCode == entry->GetKeyCode() )
        {
            int flags = entry->GetFlags();

                        if ( (((flags & wxACCEL_CTRL) != 0) == event.ControlDown()) &&
                 (((flags & wxACCEL_SHIFT) != 0) == event.ShiftDown()) &&
                 (((flags & wxACCEL_ALT) != 0) == event.AltDown()) )
            {
                return entry;
            }
        }

        node = node->GetNext();
    }

    return NULL;
}

wxMenuItem *wxAcceleratorTable::GetMenuItem(const wxKeyEvent& event) const
{
    const wxAcceleratorEntry *entry = GetEntry(event);

    return entry ? entry->GetMenuItem() : NULL;
}

int wxAcceleratorTable::GetCommand(const wxKeyEvent& event) const
{
    const wxAcceleratorEntry *entry = GetEntry(event);

    return entry ? entry->GetCommand() : -1;
}

wxObjectRefData *wxAcceleratorTable::CreateRefData() const
{
    return new wxAccelRefData;
}

wxObjectRefData *wxAcceleratorTable::CloneRefData(const wxObjectRefData *data) const
{
    return new wxAccelRefData(*(wxAccelRefData *)data);
}

#endif 