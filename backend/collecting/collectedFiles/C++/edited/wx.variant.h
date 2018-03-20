
#ifndef _WX_VARIANT_H_
#define _WX_VARIANT_H_

#include "wx/defs.h"

#if wxUSE_VARIANT

#include "wx/object.h"
#include "wx/string.h"
#include "wx/arrstr.h"
#include "wx/list.h"
#include "wx/cpp.h"
#include "wx/longlong.h"

#if wxUSE_DATETIME
    #include "wx/datetime.h"
#endif 
#include "wx/iosfwrap.h"

class wxAny;



class WXDLLIMPEXP_BASE wxVariantData : public wxObjectRefData
{
    friend class wxVariant;
public:
    wxVariantData() { }

        virtual bool Eq(wxVariantData& data) const = 0;

#if wxUSE_STD_IOSTREAM
    virtual bool Write(wxSTD ostream& WXUNUSED(str)) const { return false; }
#endif
    virtual bool Write(wxString& WXUNUSED(str)) const { return false; }
#if wxUSE_STD_IOSTREAM
    virtual bool Read(wxSTD istream& WXUNUSED(str)) { return false; }
#endif
    virtual bool Read(wxString& WXUNUSED(str)) { return false; }
        virtual wxString GetType() const = 0;
        virtual wxClassInfo* GetValueClassInfo() { return NULL; }

            virtual wxVariantData* Clone() const { return NULL; }

#if wxUSE_ANY
        virtual bool GetAsAny(wxAny* WXUNUSED(any)) const { return false; }
#endif

protected:
                virtual ~wxVariantData() { }
};



class WXDLLIMPEXP_FWD_BASE wxVariant;

WX_DECLARE_LIST_WITH_DECL(wxVariant, wxVariantList, class WXDLLIMPEXP_BASE);

class WXDLLIMPEXP_BASE wxVariant: public wxObject
{
public:
    wxVariant();

    wxVariant(const wxVariant& variant);
    wxVariant(wxVariantData* data, const wxString& name = wxEmptyString);
#if wxUSE_ANY
    wxVariant(const wxAny& any);
#endif
    virtual ~wxVariant();

        void operator= (const wxVariant& variant);

            void operator= (wxVariantData* variantData);

    bool operator== (const wxVariant& variant) const;
    bool operator!= (const wxVariant& variant) const;

        inline void SetName(const wxString& name) { m_name = name; }
    inline const wxString& GetName() const { return m_name; }

        bool IsNull() const;

            wxVariantData* GetData() const
    {
        return (wxVariantData*) m_refData;
    }
    void SetData(wxVariantData* data) ;

        void Ref(const wxVariant& clone) { wxObject::Ref(clone); }

        bool Unshare();

        void MakeNull();

        void Clear();

            wxString GetType() const;

    bool IsType(const wxString& type) const;
    bool IsValueKindOf(const wxClassInfo* type) const;

        wxString MakeString() const;

#if wxUSE_ANY
    wxAny GetAny() const;
#endif

        wxVariant(double val, const wxString& name = wxEmptyString);
    bool operator== (double value) const;
    bool operator!= (double value) const;
    void operator= (double value) ;
    inline operator double () const {  return GetDouble(); }
    inline double GetReal() const { return GetDouble(); }
    double GetDouble() const;

        wxVariant(long val, const wxString& name = wxEmptyString);
    wxVariant(int val, const wxString& name = wxEmptyString);
    wxVariant(short val, const wxString& name = wxEmptyString);
    bool operator== (long value) const;
    bool operator!= (long value) const;
    void operator= (long value) ;
    inline operator long () const {  return GetLong(); }
    inline long GetInteger() const { return GetLong(); }
    long GetLong() const;

        wxVariant(bool val, const wxString& name = wxEmptyString);
    bool operator== (bool value) const;
    bool operator!= (bool value) const;
    void operator= (bool value) ;
    inline operator bool () const {  return GetBool(); }
    bool GetBool() const ;

    #if wxUSE_DATETIME
    wxVariant(const wxDateTime& val, const wxString& name = wxEmptyString);
    bool operator== (const wxDateTime& value) const;
    bool operator!= (const wxDateTime& value) const;
    void operator= (const wxDateTime& value) ;
    inline operator wxDateTime () const { return GetDateTime(); }
    wxDateTime GetDateTime() const;
#endif

        wxVariant(const wxString& val, const wxString& name = wxEmptyString);
            wxVariant(const char* val, const wxString& name = wxEmptyString);
    wxVariant(const wchar_t* val, const wxString& name = wxEmptyString);
    wxVariant(const wxCStrData& val, const wxString& name = wxEmptyString);
    wxVariant(const wxScopedCharBuffer& val, const wxString& name = wxEmptyString);
    wxVariant(const wxScopedWCharBuffer& val, const wxString& name = wxEmptyString);

    bool operator== (const wxString& value) const;
    bool operator!= (const wxString& value) const;
    wxVariant& operator=(const wxString& value);
            wxVariant& operator=(const char* value)
        { return *this = wxString(value); }
    wxVariant& operator=(const wchar_t* value)
        { return *this = wxString(value); }
    wxVariant& operator=(const wxCStrData& value)
        { return *this = value.AsString(); }
    template<typename T>
    wxVariant& operator=(const wxScopedCharTypeBuffer<T>& value)
        { return *this = value.data(); }

    inline operator wxString () const {  return MakeString(); }
    wxString GetString() const;

#if wxUSE_STD_STRING
    wxVariant(const std::string& val, const wxString& name = wxEmptyString);
    bool operator==(const std::string& value) const
        { return operator==(wxString(value)); }
    bool operator!=(const std::string& value) const
        { return operator!=(wxString(value)); }
    wxVariant& operator=(const std::string& value)
        { return operator=(wxString(value)); }
    operator std::string() const { return (operator wxString()).ToStdString(); }

    wxVariant(const wxStdWideString& val, const wxString& name = wxEmptyString);
    bool operator==(const wxStdWideString& value) const
        { return operator==(wxString(value)); }
    bool operator!=(const wxStdWideString& value) const
        { return operator!=(wxString(value)); }
    wxVariant& operator=(const wxStdWideString& value)
        { return operator=(wxString(value)); }
    operator wxStdWideString() const { return (operator wxString()).ToStdWstring(); }
#endif 
        wxVariant(const wxUniChar& val, const wxString& name = wxEmptyString);
    wxVariant(const wxUniCharRef& val, const wxString& name = wxEmptyString);
    wxVariant(char val, const wxString& name = wxEmptyString);
    wxVariant(wchar_t val, const wxString& name = wxEmptyString);
    bool operator==(const wxUniChar& value) const;
    bool operator==(const wxUniCharRef& value) const { return *this == wxUniChar(value); }
    bool operator==(char value) const { return *this == wxUniChar(value); }
    bool operator==(wchar_t value) const { return *this == wxUniChar(value); }
    bool operator!=(const wxUniChar& value) const { return !(*this == value); }
    bool operator!=(const wxUniCharRef& value) const { return !(*this == value); }
    bool operator!=(char value) const { return !(*this == value); }
    bool operator!=(wchar_t value) const { return !(*this == value); }
    wxVariant& operator=(const wxUniChar& value);
    wxVariant& operator=(const wxUniCharRef& value) { return *this = wxUniChar(value); }
    wxVariant& operator=(char value) { return *this = wxUniChar(value); }
    wxVariant& operator=(wchar_t value) { return *this = wxUniChar(value); }
    operator wxUniChar() const { return GetChar(); }
    operator char() const { return GetChar(); }
    operator wchar_t() const { return GetChar(); }
    wxUniChar GetChar() const;

        wxVariant(const wxArrayString& val, const wxString& name = wxEmptyString);
    bool operator== (const wxArrayString& value) const;
    bool operator!= (const wxArrayString& value) const;
    void operator= (const wxArrayString& value);
    operator wxArrayString () const { return GetArrayString(); }
    wxArrayString GetArrayString() const;

        wxVariant(void* ptr, const wxString& name = wxEmptyString);
    bool operator== (void* value) const;
    bool operator!= (void* value) const;
    void operator= (void* value);
    operator void* () const {  return GetVoidPtr(); }
    void* GetVoidPtr() const;

        wxVariant(wxObject* ptr, const wxString& name = wxEmptyString);
    bool operator== (wxObject* value) const;
    bool operator!= (wxObject* value) const;
    void operator= (wxObject* value);
    wxObject* GetWxObjectPtr() const;

#if wxUSE_LONGLONG
        wxVariant(wxLongLong, const wxString& name = wxEmptyString);
    bool operator==(wxLongLong value) const;
    bool operator!=(wxLongLong value) const;
    void operator=(wxLongLong value);
    operator wxLongLong() const { return GetLongLong(); }
    wxLongLong GetLongLong() const;

        wxVariant(wxULongLong, const wxString& name = wxEmptyString);
    bool operator==(wxULongLong value) const;
    bool operator!=(wxULongLong value) const;
    void operator=(wxULongLong value);
    operator wxULongLong() const { return GetULongLong(); }
    wxULongLong GetULongLong() const;
#endif

            
    wxVariant(const wxVariantList& val, const wxString& name = wxEmptyString);     bool operator== (const wxVariantList& value) const;
    bool operator!= (const wxVariantList& value) const;
    void operator= (const wxVariantList& value) ;
        wxVariant operator[] (size_t idx) const;
    wxVariant& operator[] (size_t idx) ;
    wxVariantList& GetList() const ;

        size_t GetCount() const;

        void NullList();

        void Append(const wxVariant& value);

        void Insert(const wxVariant& value);

        bool Member(const wxVariant& value) const;

        bool Delete(size_t item);

        void ClearList();

public:
        bool Convert(long* value) const;
    bool Convert(bool* value) const;
    bool Convert(double* value) const;
    bool Convert(wxString* value) const;
    bool Convert(wxUniChar* value) const;
    bool Convert(char* value) const;
    bool Convert(wchar_t* value) const;
#if wxUSE_DATETIME
    bool Convert(wxDateTime* value) const;
#endif #if wxUSE_LONGLONG
    bool Convert(wxLongLong* value) const;
    bool Convert(wxULongLong* value) const;
  #ifdef wxLongLong_t
    bool Convert(wxLongLong_t* value) const
    {
        wxLongLong temp;
        if ( !Convert(&temp) )
            return false;
        *value = temp.GetValue();
        return true;
    }
    bool Convert(wxULongLong_t* value) const
    {
        wxULongLong temp;
        if ( !Convert(&temp) )
            return false;
        *value = temp.GetValue();
        return true;
    }
  #endif #endif 
protected:
    virtual wxObjectRefData *CreateRefData() const wxOVERRIDE;
    virtual wxObjectRefData *CloneRefData(const wxObjectRefData *data) const wxOVERRIDE;

    wxString        m_name;

private:
    wxDECLARE_DYNAMIC_CLASS(wxVariant);
};


#if wxUSE_ANY

#include "wx/any.h"

typedef wxVariantData* (*wxVariantDataFactory)(const wxAny& any);

class WXDLLIMPEXP_BASE wxAnyToVariantRegistration
{
public:
    wxAnyToVariantRegistration(wxVariantDataFactory factory);
    virtual ~wxAnyToVariantRegistration();

    virtual wxAnyValueType* GetAssociatedType() = 0;
    wxVariantDataFactory GetFactory() const { return m_factory; }
private:
    wxVariantDataFactory    m_factory;
};

template<typename T>
class wxAnyToVariantRegistrationImpl : public wxAnyToVariantRegistration
{
public:
    wxAnyToVariantRegistrationImpl(wxVariantDataFactory factory)
        : wxAnyToVariantRegistration(factory)
    {
    }

    virtual wxAnyValueType* GetAssociatedType() wxOVERRIDE
    {
        return wxAnyValueTypeImpl<T>::GetInstance();
    }
private:
};

#define DECLARE_WXANY_CONVERSION() \
virtual bool GetAsAny(wxAny* any) const wxOVERRIDE; \
static wxVariantData* VariantDataFactory(const wxAny& any);

#define _REGISTER_WXANY_CONVERSION(T, CLASSNAME, FUNC) \
static wxAnyToVariantRegistrationImpl<T> \
    gs_##CLASSNAME##AnyToVariantRegistration = \
    wxAnyToVariantRegistrationImpl<T>(&FUNC);

#define REGISTER_WXANY_CONVERSION(T, CLASSNAME) \
_REGISTER_WXANY_CONVERSION(T, CLASSNAME, CLASSNAME::VariantDataFactory)

#define IMPLEMENT_TRIVIAL_WXANY_CONVERSION(T, CLASSNAME) \
bool CLASSNAME::GetAsAny(wxAny* any) const \
{ \
    *any = m_value; \
    return true; \
} \
wxVariantData* CLASSNAME::VariantDataFactory(const wxAny& any) \
{ \
    return new CLASSNAME(any.As<T>()); \
} \
REGISTER_WXANY_CONVERSION(T, CLASSNAME)

#else 
#define DECLARE_WXANY_CONVERSION()
#define REGISTER_WXANY_CONVERSION(T, CLASSNAME)
#define IMPLEMENT_TRIVIAL_WXANY_CONVERSION(T, CLASSNAME)

#endif 

#define DECLARE_VARIANT_OBJECT(classname) \
    DECLARE_VARIANT_OBJECT_EXPORTED(classname, wxEMPTY_PARAMETER_VALUE)

#define DECLARE_VARIANT_OBJECT_EXPORTED(classname,expdecl) \
expdecl classname& operator << ( classname &object, const wxVariant &variant ); \
expdecl wxVariant& operator << ( wxVariant &variant, const classname &object );

#define IMPLEMENT_VARIANT_OBJECT(classname) \
    IMPLEMENT_VARIANT_OBJECT_EXPORTED(classname, wxEMPTY_PARAMETER_VALUE)

#define IMPLEMENT_VARIANT_OBJECT_EXPORTED_NO_EQ(classname,expdecl) \
class classname##VariantData: public wxVariantData \
{ \
public:\
    classname##VariantData() {} \
    classname##VariantData( const classname &value ) { m_value = value; } \
\
    classname &GetValue() { return m_value; } \
\
    virtual bool Eq(wxVariantData& data) const wxOVERRIDE; \
\
    virtual wxString GetType() const wxOVERRIDE; \
    virtual wxClassInfo* GetValueClassInfo() wxOVERRIDE; \
\
    virtual wxVariantData* Clone() const wxOVERRIDE { return new classname##VariantData(m_value); } \
\
    DECLARE_WXANY_CONVERSION() \
protected:\
    classname m_value; \
};\
\
wxString classname##VariantData::GetType() const\
{\
    return m_value.GetClassInfo()->GetClassName();\
}\
\
wxClassInfo* classname##VariantData::GetValueClassInfo()\
{\
    return m_value.GetClassInfo();\
}\
\
expdecl classname& operator << ( classname &value, const wxVariant &variant )\
{\
    wxASSERT( variant.GetType() == #classname );\
    \
    classname##VariantData *data = (classname##VariantData*) variant.GetData();\
    value = data->GetValue();\
    return value;\
}\
\
expdecl wxVariant& operator << ( wxVariant &variant, const classname &value )\
{\
    classname##VariantData *data = new classname##VariantData( value );\
    variant.SetData( data );\
    return variant;\
} \
IMPLEMENT_TRIVIAL_WXANY_CONVERSION(classname, classname##VariantData)

#define IMPLEMENT_VARIANT_OBJECT_EXPORTED(classname,expdecl) \
IMPLEMENT_VARIANT_OBJECT_EXPORTED_NO_EQ(classname,wxEMPTY_PARAMETER_VALUE expdecl) \
\
bool classname##VariantData::Eq(wxVariantData& data) const \
{\
    wxASSERT( GetType() == data.GetType() );\
\
    classname##VariantData & otherData = (classname##VariantData &) data;\
\
    return otherData.m_value == m_value;\
}\


#define IMPLEMENT_VARIANT_OBJECT_SHALLOWCMP(classname) \
    IMPLEMENT_VARIANT_OBJECT_EXPORTED_SHALLOWCMP(classname, wxEMPTY_PARAMETER_VALUE)
#define IMPLEMENT_VARIANT_OBJECT_EXPORTED_SHALLOWCMP(classname,expdecl) \
IMPLEMENT_VARIANT_OBJECT_EXPORTED_NO_EQ(classname,wxEMPTY_PARAMETER_VALUE expdecl) \
\
bool classname##VariantData::Eq(wxVariantData& data) const \
{\
    wxASSERT( GetType() == data.GetType() );\
\
    classname##VariantData & otherData = (classname##VariantData &) data;\
\
    return (otherData.m_value.IsSameAs(m_value));\
}\



#define wxGetVariantCast(var,classname) \
    ((classname*)(var.IsValueKindOf(&classname::ms_classInfo) ?\
                  var.GetWxObjectPtr() : NULL));

#ifndef wxNO_RTTI
    #define wxDynamicCastVariantData(data, classname) dynamic_cast<classname*>(data)
#endif

#define wxStaticCastVariantData(data, classname) static_cast<classname*>(data)

extern wxVariant WXDLLIMPEXP_BASE wxNullVariant;

#endif 
#endif 