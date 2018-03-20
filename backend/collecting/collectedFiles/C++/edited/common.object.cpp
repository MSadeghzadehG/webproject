
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/object.h"
    #include "wx/hash.h"
    #include "wx/memory.h"
    #include "wx/crt.h"
#endif

#include <string.h>

#if defined __VISUALC__
    #pragma optimize("", off)
#endif

#if wxUSE_EXTENDED_RTTI
const wxClassInfo* wxObject::ms_classParents[] = { NULL } ;
wxObject* wxVariantOfPtrToObjectConverterwxObject ( const wxAny &data )
{ return data.As<wxObject*>(); }
 wxAny wxObjectToVariantConverterwxObject ( wxObject *data )
 { return wxAny( dynamic_cast<wxObject*> (data)  ) ; }

 wxClassInfo wxObject::ms_classInfo(ms_classParents , wxEmptyString , wxT("wxObject"),
            (int) sizeof(wxObject),                              \
            (wxObjectConstructorFn) 0   ,
            NULL,NULL,0 , 0 ,
            0 , wxVariantOfPtrToObjectConverterwxObject , 0 , wxObjectToVariantConverterwxObject);

 template<> void wxStringWriteValue(wxString & , wxObject* const & ){ wxFAIL_MSG("unreachable"); }
 template<> void wxStringWriteValue(wxString & , wxObject const & ){ wxFAIL_MSG("unreachable"); }

 wxClassTypeInfo s_typeInfo(wxT_OBJECT_PTR , &wxObject::ms_classInfo , NULL , NULL , typeid(wxObject*).name() ) ;
 wxClassTypeInfo s_typeInfowxObject(wxT_OBJECT , &wxObject::ms_classInfo , NULL , NULL , typeid(wxObject).name() ) ;
#else
wxClassInfo wxObject::ms_classInfo( wxT("wxObject"), 0, 0,
                                        (int) sizeof(wxObject),
                                        (wxObjectConstructorFn) 0 );
#endif

#if defined __VISUALC__
    #pragma optimize("", on)
#endif

wxClassInfo* wxClassInfo::sm_first = NULL;
wxHashTable* wxClassInfo::sm_classTable = NULL;

#if !wxUSE_EXTENDED_RTTI

wxClassInfo *wxObject::GetClassInfo() const
{
    return &wxObject::ms_classInfo;
}

#endif 
const bool wxFalse = false;

bool wxObject::IsKindOf(const wxClassInfo *info) const
{
    const wxClassInfo *thisInfo = GetClassInfo();
    return (thisInfo) ? thisInfo->IsKindOf(info) : false ;
}

#if wxUSE_MEMORY_TRACING && defined( new )
    #undef new
#endif


#ifdef _WX_WANT_NEW_SIZET_WXCHAR_INT
void *wxObject::operator new ( size_t size, const wxChar *fileName, int lineNum )
{
    return wxDebugAlloc(size, (wxChar*) fileName, lineNum, true);
}
#endif

#ifdef _WX_WANT_DELETE_VOID
void wxObject::operator delete ( void *buf )
{
    wxDebugFree(buf);
}
#endif

#ifdef _WX_WANT_DELETE_VOID_CONSTCHAR_SIZET
void wxObject::operator delete ( void *buf, const char *_fname, size_t _line )
{
    wxDebugFree(buf);
}
#endif

#ifdef _WX_WANT_DELETE_VOID_WXCHAR_INT
void wxObject::operator delete ( void *buf, const wxChar *WXUNUSED(fileName), int WXUNUSED(lineNum) )
{
     wxDebugFree(buf);
}
#endif

#ifdef _WX_WANT_ARRAY_NEW_SIZET_WXCHAR_INT
void *wxObject::operator new[] ( size_t size, const wxChar* fileName, int lineNum )
{
    return wxDebugAlloc(size, (wxChar*) fileName, lineNum, true, true);
}
#endif

#ifdef _WX_WANT_ARRAY_DELETE_VOID
void wxObject::operator delete[] ( void *buf )
{
    wxDebugFree(buf, true);
}
#endif

#ifdef _WX_WANT_ARRAY_DELETE_VOID_WXCHAR_INT
void wxObject::operator delete[] (void * buf, const wxChar*  WXUNUSED(fileName), int WXUNUSED(lineNum) )
{
    wxDebugFree(buf, true);
}
#endif



wxClassInfo::~wxClassInfo()
{
                if ( this == sm_first )
    {
        sm_first = m_next;
    }
    else
    {
        wxClassInfo *info = sm_first;
        while (info)
        {
            if ( info->m_next == this )
            {
                info->m_next = m_next;
                break;
            }

            info = info->m_next;
        }
    }
    Unregister();
}

wxClassInfo *wxClassInfo::FindClass(const wxString& className)
{
    if ( sm_classTable )
    {
        return (wxClassInfo *)wxClassInfo::sm_classTable->Get(className);
    }
    else
    {
        for ( wxClassInfo *info = sm_first; info ; info = info->m_next )
        {
            if ( className == info->GetClassName() )
                return info;
        }

        return NULL;
    }
}


void wxClassInfo::Register()
{
#if wxDEBUG_LEVEL
        static int entry = 0;
#endif 
    wxHashTable *classTable;

    if ( !sm_classTable )
    {
                classTable = new wxHashTable(wxKEY_STRING);
    }
    else
    {
                wxASSERT_MSG(++entry == 1, wxT("wxClassInfo::Register() reentrance"));
        classTable = sm_classTable;
    }

                        wxASSERT_MSG( classTable->Get(m_className) == NULL,
        wxString::Format
        (
            wxT("Class \"%s\" already in RTTI table - have you used wxIMPLEMENT_DYNAMIC_CLASS() multiple times or linked some object file twice)?"),
            m_className
        )
    );

    classTable->Put(m_className, (wxObject *)this);

        if ( sm_classTable != classTable )
    {
        if ( !sm_classTable )
        {
                        sm_classTable = classTable;
        }
        else
        {
                                    delete classTable;
            Register();
        }
    }

#if wxDEBUG_LEVEL
    entry = 0;
#endif }

void wxClassInfo::Unregister()
{
    if ( sm_classTable )
    {
        sm_classTable->Delete(m_className);
        if ( sm_classTable->GetCount() == 0 )
        {
            wxDELETE(sm_classTable);
        }
    }
}

wxObject *wxCreateDynamicObject(const wxString& name)
{
    if ( wxClassInfo::sm_classTable )
    {
        wxClassInfo *info = (wxClassInfo *)wxClassInfo::sm_classTable->Get(name);
        return info ? info->CreateObject() : NULL;
    }
    else     {
        for ( wxClassInfo *info = wxClassInfo::sm_first;
              info;
              info = info->m_next )
        {
            if (info->m_className && wxStrcmp(info->m_className, name) == 0)
                return info->CreateObject();
        }

        return NULL;
    }
}

wxClassInfo::const_iterator::value_type
wxClassInfo::const_iterator::operator*() const
{
    return (wxClassInfo*)m_node->GetData();
}

wxClassInfo::const_iterator& wxClassInfo::const_iterator::operator++()
{
    m_node = m_table->Next();
    return *this;
}

const wxClassInfo::const_iterator wxClassInfo::const_iterator::operator++(int)
{
    wxClassInfo::const_iterator tmp = *this;
    m_node = m_table->Next();
    return tmp;
}

wxClassInfo::const_iterator wxClassInfo::begin_classinfo()
{
    sm_classTable->BeginFind();

    return const_iterator(sm_classTable->Next(), sm_classTable);
}

wxClassInfo::const_iterator wxClassInfo::end_classinfo()
{
    return const_iterator(NULL, NULL);
}


void wxRefCounter::DecRef()
{
    wxASSERT_MSG( m_count > 0, "invalid ref data count" );

    if ( --m_count == 0 )
        delete this;
}



void wxObject::Ref(const wxObject& clone)
{
        if (m_refData == clone.m_refData)
        return;

        UnRef();

        if ( clone.m_refData )
    {
        m_refData = clone.m_refData;
        m_refData->IncRef();
    }
}

void wxObject::UnRef()
{
    if ( m_refData )
    {
        m_refData->DecRef();
        m_refData = NULL;
    }
}

void wxObject::AllocExclusive()
{
    if ( !m_refData )
    {
        m_refData = CreateRefData();
    }
    else if ( m_refData->GetRefCount() > 1 )
    {
                const wxObjectRefData* ref = m_refData;
        UnRef();

                m_refData = CloneRefData(ref);
    }
    
    wxASSERT_MSG( m_refData && m_refData->GetRefCount() == 1,
                  wxT("wxObject::AllocExclusive() failed.") );
}

wxObjectRefData *wxObject::CreateRefData() const
{
        wxFAIL_MSG( wxT("CreateRefData() must be overridden if called!") );

    return NULL;
}

wxObjectRefData *
wxObject::CloneRefData(const wxObjectRefData * WXUNUSED(data)) const
{
        wxFAIL_MSG( wxT("CloneRefData() must be overridden if called!") );

    return NULL;
}
