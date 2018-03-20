

#include "wx/wxprec.h"

#if defined(__BORLANDC__)
#pragma hdrstop
#endif

#if wxUSE_OLE && (wxUSE_DRAG_AND_DROP || wxUSE_DATAOBJ)

#ifndef WX_PRECOMP
    #include "wx/msw/wrapwin.h"
#endif

#include  <rpc.h>                       
#include  "wx/msw/ole/uuid.h"




#define   UUID_CSTRLEN  100     
Uuid::Uuid(const Uuid& uuid)
{
    m_uuid = uuid.m_uuid;

    #ifdef _UNICODE
  UuidToString(&m_uuid, (unsigned short **)&m_pszUuid);
#else
  UuidToString(&m_uuid, &m_pszUuid);
#endif

    m_pszCForm = new wxChar[UUID_CSTRLEN];
    memcpy(m_pszCForm, uuid.m_pszCForm, UUID_CSTRLEN*sizeof(wxChar));
}

Uuid& Uuid::operator=(const Uuid& uuid)
{
  m_uuid = uuid.m_uuid;

    #ifdef _UNICODE
  UuidToString(&m_uuid, (unsigned short **)&m_pszUuid);
#else
  UuidToString(&m_uuid, &m_pszUuid);
#endif

    if ( !m_pszCForm )
    m_pszCForm = new wxChar[UUID_CSTRLEN];

    memcpy(m_pszCForm, uuid.m_pszCForm, UUID_CSTRLEN*sizeof(wxChar));

  return *this;
}

bool Uuid::operator==(const Uuid& uuid) const
{
            return IsEqualGUID(m_uuid, uuid.m_uuid) != 0;
}

Uuid::~Uuid()
{
      if ( m_pszUuid )
#ifdef _UNICODE
    RpcStringFree((unsigned short **)&m_pszUuid);
#else
    RpcStringFree(&m_pszUuid);
#endif

      if ( m_pszCForm )
    delete [] m_pszCForm;
}

void Uuid::Set(const UUID &uuid)
{
  m_uuid = uuid;

  #ifdef _UNICODE
  UuidToString(&m_uuid, (unsigned short **)&m_pszUuid);
#else
  UuidToString(&m_uuid, &m_pszUuid);
#endif

    UuidToCForm();
}

void Uuid::Create()
{
  UUID uuid;

    UuidCreate(&uuid);

  Set(uuid);
}

bool Uuid::Set(const wxChar *pc)
{
  #ifdef _UNICODE
  if ( UuidFromString((unsigned short *)pc, &m_uuid) != RPC_S_OK)
#else
  if ( UuidFromString((wxUChar *)pc, &m_uuid) != RPC_S_OK)
#endif
        return false;

  #ifdef _UNICODE
  UuidToString(&m_uuid, (unsigned short **)&m_pszUuid);
#else
  UuidToString(&m_uuid, &m_pszUuid);
#endif

    UuidToCForm();

  return true;
}

void Uuid::UuidToCForm()
{
  if ( m_pszCForm == NULL )
    m_pszCForm = new wxChar[UUID_CSTRLEN];

  wsprintf(m_pszCForm, wxT("0x%8.8X,0x%4.4X,0x%4.4X,0x%2.2X,0x2.2%X,0x2.2%X,0x2.2%X,0x2.2%X,0x2.2%X,0x2.2%X,0x2.2%X"),
           m_uuid.Data1, m_uuid.Data2, m_uuid.Data3,
           m_uuid.Data4[0], m_uuid.Data4[1], m_uuid.Data4[2], m_uuid.Data4[3],
           m_uuid.Data4[4], m_uuid.Data4[5], m_uuid.Data4[6], m_uuid.Data4[7]);
}

#endif
  