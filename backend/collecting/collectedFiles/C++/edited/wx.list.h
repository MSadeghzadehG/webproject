


#ifndef _WX_LIST_H_
#define _WX_LIST_H_


#include "wx/defs.h"
#include "wx/object.h"
#include "wx/string.h"
#include "wx/vector.h"

#if wxUSE_STD_CONTAINERS
    #include "wx/beforestd.h"
    #include <algorithm>
    #include <iterator>
    #include <list>
    #include "wx/afterstd.h"
#endif


class WXDLLIMPEXP_FWD_BASE wxObjectListNode;
typedef wxObjectListNode wxNode;

#if wxUSE_STD_CONTAINERS

#define wxLIST_COMPATIBILITY

#define WX_DECLARE_LIST_3(elT, dummy1, liT, dummy2, decl) \
    WX_DECLARE_LIST_WITH_DECL(elT, liT, decl)
#define WX_DECLARE_LIST_PTR_3(elT, dummy1, liT, dummy2, decl) \
    WX_DECLARE_LIST_3(elT, dummy1, liT, dummy2, decl)

#define WX_DECLARE_LIST_2(elT, liT, dummy, decl) \
    WX_DECLARE_LIST_WITH_DECL(elT, liT, decl)
#define WX_DECLARE_LIST_PTR_2(elT, liT, dummy, decl) \
    WX_DECLARE_LIST_2(elT, liT, dummy, decl) \

#define WX_DECLARE_LIST_WITH_DECL(elT, liT, decl) \
    WX_DECLARE_LIST_XO(elT*, liT, decl)

template<class T>
class wxList_SortFunction
{
public:
    wxList_SortFunction(wxSortCompareFunction f) : m_f(f) { }
    bool operator()(const T& i1, const T& i2)
      { return m_f((T*)&i1, (T*)&i2) < 0; }
private:
    wxSortCompareFunction m_f;
};



#define WX_DECLARE_LIST_XO(elT, liT, decl)                                    \
    decl _WX_LIST_HELPER_##liT                                                \
    {                                                                         \
        typedef elT _WX_LIST_ITEM_TYPE_##liT;                                 \
        typedef std::list<elT> BaseListType;                                  \
    public:                                                                   \
        static BaseListType EmptyList;                                        \
        static void DeleteFunction( _WX_LIST_ITEM_TYPE_##liT X );             \
    };                                                                        \
                                                                              \
    class liT : public std::list<elT>                                          \
    {                                                                         \
    private:                                                                  \
        typedef std::list<elT> BaseListType;                                  \
                                                                              \
        bool m_destroy;                                                       \
                                                                              \
    public:                                                                   \
        class compatibility_iterator                                           \
        {                                                                     \
        private:                                                              \
            friend class liT;                                                 \
                                                                              \
            iterator m_iter;                                                  \
            liT * m_list;                                                     \
                                                                              \
        public:                                                               \
            compatibility_iterator()                                          \
                : m_iter(_WX_LIST_HELPER_##liT::EmptyList.end()), m_list( NULL ) {}                  \
            compatibility_iterator( liT* li, iterator i )                     \
                : m_iter( i ), m_list( li ) {}                                \
            compatibility_iterator( const liT* li, iterator i )               \
                : m_iter( i ), m_list( const_cast< liT* >( li ) ) {}          \
                                                                              \
            compatibility_iterator* operator->() { return this; }             \
            const compatibility_iterator* operator->() const { return this; } \
                                                                              \
            bool operator==(const compatibility_iterator& i) const            \
            {                                                                 \
                wxASSERT_MSG( m_list && i.m_list,                             \
                              wxT("comparing invalid iterators is illegal") ); \
                return (m_list == i.m_list) && (m_iter == i.m_iter);          \
            }                                                                 \
            bool operator!=(const compatibility_iterator& i) const            \
                { return !( operator==( i ) ); }                              \
            operator bool() const                                             \
                { return m_list ? m_iter != m_list->end() : false; }          \
            bool operator !() const                                           \
                { return !( operator bool() ); }                              \
                                                                              \
            elT GetData() const                                               \
                { return *m_iter; }                                           \
            void SetData( elT e )                                             \
                { *m_iter = e; }                                              \
                                                                              \
            compatibility_iterator GetNext() const                            \
            {                                                                 \
                iterator i = m_iter;                                          \
                return compatibility_iterator( m_list, ++i );                 \
            }                                                                 \
            compatibility_iterator GetPrevious() const                        \
            {                                                                 \
                if ( m_iter == m_list->begin() )                              \
                    return compatibility_iterator();                          \
                                                                              \
                iterator i = m_iter;                                          \
                return compatibility_iterator( m_list, --i );                 \
            }                                                                 \
            int IndexOf() const                                               \
            {                                                                 \
                return *this ? (int)std::distance( m_list->begin(), m_iter )  \
                             : wxNOT_FOUND;                                   \
            }                                                                 \
        };                                                                    \
    public:                                                                   \
        liT() : m_destroy( false ) {}                                         \
                                                                              \
        compatibility_iterator Find( const elT e ) const                      \
        {                                                                     \
          liT* _this = const_cast< liT* >( this );                            \
          return compatibility_iterator( _this,                               \
                     std::find( _this->begin(), _this->end(), e ) );          \
        }                                                                     \
                                                                              \
        bool IsEmpty() const                                                  \
            { return empty(); }                                               \
        size_t GetCount() const                                               \
            { return size(); }                                                \
        int Number() const                                                    \
            { return static_cast< int >( GetCount() ); }                      \
                                                                              \
        compatibility_iterator Item( size_t idx ) const                       \
        {                                                                     \
            iterator i = const_cast< liT* >(this)->begin();                   \
            std::advance( i, idx );                                           \
            return compatibility_iterator( this, i );                         \
        }                                                                     \
        elT operator[](size_t idx) const                                      \
        {                                                                     \
            return Item(idx).GetData();                                       \
        }                                                                     \
                                                                              \
        compatibility_iterator GetFirst() const                               \
        {                                                                     \
            return compatibility_iterator( this,                              \
                const_cast< liT* >(this)->begin() );                          \
        }                                                                     \
        compatibility_iterator GetLast() const                                \
        {                                                                     \
            iterator i = const_cast< liT* >(this)->end();                     \
            return compatibility_iterator( this, !empty() ? --i : i );        \
        }                                                                     \
        bool Member( elT e ) const                                            \
            { return Find( e ); }                                             \
        compatibility_iterator Nth( int n ) const                             \
            { return Item( n ); }                                             \
        int IndexOf( elT e ) const                                            \
            { return Find( e ).IndexOf(); }                                   \
                                                                              \
        compatibility_iterator Append( elT e )                                \
        {                                                                     \
            push_back( e );                                                   \
            return GetLast();                                                 \
        }                                                                     \
        compatibility_iterator Insert( elT e )                                \
        {                                                                     \
            push_front( e );                                                  \
            return compatibility_iterator( this, begin() );                   \
        }                                                                     \
        compatibility_iterator Insert(const compatibility_iterator & i, elT e)\
        {                                                                     \
            return compatibility_iterator( this, insert( i.m_iter, e ) );     \
        }                                                                     \
        compatibility_iterator Insert( size_t idx, elT e )                    \
        {                                                                     \
            return compatibility_iterator( this,                              \
                                           insert( Item( idx ).m_iter, e ) ); \
        }                                                                     \
                                                                              \
        void DeleteContents( bool destroy )                                   \
            { m_destroy = destroy; }                                          \
        bool GetDeleteContents() const                                        \
            { return m_destroy; }                                             \
        void Erase( const compatibility_iterator& i )                         \
        {                                                                     \
            if ( m_destroy )                                                  \
                _WX_LIST_HELPER_##liT::DeleteFunction( i->GetData() );        \
            erase( i.m_iter );                                                \
        }                                                                     \
        bool DeleteNode( const compatibility_iterator& i )                    \
        {                                                                     \
            if( i )                                                           \
            {                                                                 \
                Erase( i );                                                   \
                return true;                                                  \
            }                                                                 \
            return false;                                                     \
        }                                                                     \
        bool DeleteObject( elT e )                                            \
        {                                                                     \
            return DeleteNode( Find( e ) );                                   \
        }                                                                     \
        void Clear()                                                          \
        {                                                                     \
            if ( m_destroy )                                                  \
                std::for_each( begin(), end(),                                \
                               _WX_LIST_HELPER_##liT::DeleteFunction );       \
            clear();                                                          \
        }                                                                     \
                   \
        void Sort( wxSortCompareFunction compfunc )                           \
            { sort( wxList_SortFunction<elT>(compfunc ) ); }                  \
        ~liT() { Clear(); }                                                   \
                                                                              \
                                        \
        friend class compatibility_iterator;                                  \
    }

#define WX_DECLARE_LIST(elementtype, listname)                              \
    WX_DECLARE_LIST_WITH_DECL(elementtype, listname, class)
#define WX_DECLARE_LIST_PTR(elementtype, listname)                          \
    WX_DECLARE_LIST(elementtype, listname)

#define WX_DECLARE_EXPORTED_LIST(elementtype, listname)                     \
    WX_DECLARE_LIST_WITH_DECL(elementtype, listname, class WXDLLIMPEXP_CORE)
#define WX_DECLARE_EXPORTED_LIST_PTR(elementtype, listname)                 \
    WX_DECLARE_EXPORTED_LIST(elementtype, listname)

#define WX_DECLARE_USER_EXPORTED_LIST(elementtype, listname, usergoo)       \
    WX_DECLARE_LIST_WITH_DECL(elementtype, listname, class usergoo)
#define WX_DECLARE_USER_EXPORTED_LIST_PTR(elementtype, listname, usergoo)   \
    WX_DECLARE_USER_EXPORTED_LIST(elementtype, listname, usergoo)

#define WX_DEFINE_LIST(name)    "don't forget to include listimpl.cpp!"

#define WX_DEFINE_EXPORTED_LIST(name)      WX_DEFINE_LIST(name)
#define WX_DEFINE_USER_EXPORTED_LIST(name) WX_DEFINE_LIST(name)

#else 

#define wxLIST_COMPATIBILITY


union wxListKeyValue
{
    long integer;
    wxString *string;
};

class WXDLLIMPEXP_BASE wxListKey
{
public:
        wxListKey() : m_keyType(wxKEY_NONE)
        { }
    wxListKey(long i) : m_keyType(wxKEY_INTEGER)
        { m_key.integer = i; }
    wxListKey(const wxString& s) : m_keyType(wxKEY_STRING)
        { m_key.string = new wxString(s); }
    wxListKey(const char *s) : m_keyType(wxKEY_STRING)
        { m_key.string = new wxString(s); }
    wxListKey(const wchar_t *s) : m_keyType(wxKEY_STRING)
        { m_key.string = new wxString(s); }

        wxKeyType GetKeyType() const { return m_keyType; }
    const wxString GetString() const
        { wxASSERT( m_keyType == wxKEY_STRING ); return *m_key.string; }
    long GetNumber() const
        { wxASSERT( m_keyType == wxKEY_INTEGER ); return m_key.integer; }

                bool operator==(wxListKeyValue value) const ;

        ~wxListKey()
    {
        if ( m_keyType == wxKEY_STRING )
            delete m_key.string;
    }

private:
    wxKeyType m_keyType;
    wxListKeyValue m_key;
};


extern WXDLLIMPEXP_DATA_BASE(wxListKey) wxDefaultListKey;

class WXDLLIMPEXP_FWD_BASE wxListBase;

class WXDLLIMPEXP_BASE wxNodeBase
{
friend class wxListBase;
public:
        wxNodeBase(wxListBase *list = NULL,
               wxNodeBase *previous = NULL,
               wxNodeBase *next = NULL,
               void *data = NULL,
               const wxListKey& key = wxDefaultListKey);

    virtual ~wxNodeBase();

        wxString GetKeyString() const { return *m_key.string; }
    long GetKeyInteger() const { return m_key.integer; }

        void SetKeyString(const wxString& s) { m_key.string = new wxString(s); }
    void SetKeyInteger(long i) { m_key.integer = i; }

#ifdef wxLIST_COMPATIBILITY
        wxDEPRECATED( wxNode *Next() const );
    wxDEPRECATED( wxNode *Previous() const );
    wxDEPRECATED( wxObject *Data() const );
#endif 
protected:
        wxNodeBase *GetNext() const { return m_next; }
    wxNodeBase *GetPrevious() const { return m_previous; }

    void *GetData() const { return m_data; }
    void SetData(void *data) { m_data = data; }

        int IndexOf() const;

    virtual void DeleteData() { }
public:
        void** GetDataPtr() const { return &(const_cast<wxNodeBase*>(this)->m_data); }
private:
        wxListKeyValue m_key;

    void        *m_data;            wxNodeBase  *m_next,                        *m_previous;

    wxListBase  *m_list;        
    wxDECLARE_NO_COPY_CLASS(wxNodeBase);
};


class WXDLLIMPEXP_FWD_BASE wxList;

class WXDLLIMPEXP_BASE wxListBase
{
friend class wxNodeBase; friend class wxHashTableBase;   
public:
        wxListBase(wxKeyType keyType = wxKEY_NONE)
        { Init(keyType); }
    virtual ~wxListBase();

                size_t GetCount() const { return m_count; }

            bool IsEmpty() const { return m_count == 0; }

    
            void Clear();

            void DeleteContents(bool destroy) { m_destroy = destroy; }

           bool GetDeleteContents() const
        { return m_destroy; }

          wxKeyType GetKeyType() const
        { return m_keyType; }

          void SetKeyType(wxKeyType keyType)
        { wxASSERT( m_count==0 ); m_keyType = keyType; }

#ifdef wxLIST_COMPATIBILITY
        wxDEPRECATED( int Number() const );                 wxDEPRECATED( wxNode *First() const );              wxDEPRECATED( wxNode *Last() const );               wxDEPRECATED( wxNode *Nth(size_t n) const );    
        wxDEPRECATED( operator wxList&() const );
#endif 
protected:

        
        virtual wxNodeBase *CreateNode(wxNodeBase *prev, wxNodeBase *next,
                                   void *data,
                                   const wxListKey& key = wxDefaultListKey) = 0;


                wxListBase(size_t count, void *elements[]);
            wxListBase(void *object, ... );

protected:
    void Assign(const wxListBase& list)
        { Clear(); DoCopy(list); }

            wxNodeBase *GetFirst() const { return m_nodeFirst; }
    wxNodeBase *GetLast() const { return m_nodeLast; }

            wxNodeBase *Item(size_t index) const;

            void *operator[](size_t n) const
    {
        wxNodeBase *node = Item(n);

        return node ? node->GetData() : NULL;
    }

                wxNodeBase *Prepend(void *object)
        { return (wxNodeBase *)wxListBase::Insert(object); }
            wxNodeBase *Append(void *object);
            wxNodeBase *Insert(void *object)
        { return Insert(static_cast<wxNodeBase *>(NULL), object); }
            wxNodeBase *Insert(size_t pos, void *object)
        { return pos == GetCount() ? Append(object)
                                   : Insert(Item(pos), object); }
            wxNodeBase *Insert(wxNodeBase *prev, void *object);

            wxNodeBase *Append(long key, void *object);
    wxNodeBase *Append(const wxString& key, void *object);

                    wxNodeBase *DetachNode(wxNodeBase *node);
            bool DeleteNode(wxNodeBase *node);
                    bool DeleteObject(void *object);

                wxNodeBase *Find(const void *object) const;

            wxNodeBase *Find(const wxListKey& key) const;

        int IndexOf( void *object ) const;

            void Sort(const wxSortCompareFunction compfunc);

        void *FirstThat(wxListIterateFunction func);
    void ForEach(wxListIterateFunction func);
    void *LastThat(wxListIterateFunction func);

            void Reverse();
    void DeleteNodes(wxNodeBase* first, wxNodeBase* last);
private:

            void Init(wxKeyType keyType = wxKEY_NONE);

                void DoCopy(const wxListBase& list);
            wxNodeBase *AppendCommon(wxNodeBase *node);
            void DoDeleteNode(wxNodeBase *node);

    size_t m_count;                 bool m_destroy;                 wxNodeBase *m_nodeFirst,                   *m_nodeLast;

    wxKeyType m_keyType;        };




#define WX_DECLARE_LIST_4(T, Tbase, name, nodetype, classexp, ptrop)        \
    typedef int (*wxSortFuncFor_##name)(const T **, const T **);            \
                                                                            \
    classexp nodetype : public wxNodeBase                                   \
    {                                                                       \
    public:                                                                 \
        nodetype(wxListBase *list = NULL,                                   \
                 nodetype *previous = NULL,                                 \
                 nodetype *next = NULL,                                     \
                 T *data = NULL,                                            \
                 const wxListKey& key = wxDefaultListKey)                   \
            : wxNodeBase(list, previous, next, data, key) { }               \
                                                                            \
        nodetype *GetNext() const                                           \
            { return (nodetype *)wxNodeBase::GetNext(); }                   \
        nodetype *GetPrevious() const                                       \
            { return (nodetype *)wxNodeBase::GetPrevious(); }               \
                                                                            \
        T *GetData() const                                                  \
            { return (T *)wxNodeBase::GetData(); }                          \
        void SetData(T *data)                                               \
            { wxNodeBase::SetData(data); }                                  \
                                                                            \
    protected:                                                              \
        virtual void DeleteData();                                          \
                                                                            \
        wxDECLARE_NO_COPY_CLASS(nodetype);                                  \
    };                                                                      \
                                                                            \
    classexp name : public wxListBase                                       \
    {                                                                       \
    public:                                                                 \
        typedef nodetype Node;                                              \
        classexp compatibility_iterator                                     \
        {                                                                   \
        public:                                                             \
            compatibility_iterator(Node *ptr = NULL) : m_ptr(ptr) { }       \
                                                                            \
            Node *operator->() const { return m_ptr; }                      \
            operator Node *() const { return m_ptr; }                       \
                                                                            \
        private:                                                            \
            Node *m_ptr;                                                    \
        };                                                                  \
                                                                            \
        name(wxKeyType keyType = wxKEY_NONE) : wxListBase(keyType)          \
            { }                                                             \
        name(const name& list) : wxListBase(list.GetKeyType())              \
            { Assign(list); }                                               \
        name(size_t count, T *elements[])                                   \
            : wxListBase(count, (void **)elements) { }                      \
                                                                            \
        name& operator=(const name& list)                                   \
            { if (&list != this) Assign(list); return *this; }              \
                                                                            \
        nodetype *GetFirst() const                                          \
            { return (nodetype *)wxListBase::GetFirst(); }                  \
        nodetype *GetLast() const                                           \
            { return (nodetype *)wxListBase::GetLast(); }                   \
                                                                            \
        nodetype *Item(size_t index) const                                  \
            { return (nodetype *)wxListBase::Item(index); }                 \
                                                                            \
        T *operator[](size_t index) const                                   \
        {                                                                   \
            nodetype *node = Item(index);                                   \
            return node ? (T*)(node->GetData()) : NULL;                     \
        }                                                                   \
                                                                            \
        nodetype *Append(Tbase *object)                                     \
            { return (nodetype *)wxListBase::Append(object); }              \
        nodetype *Insert(Tbase *object)                                     \
            { return (nodetype *)Insert(static_cast<nodetype *>(NULL),      \
                                        object); }                          \
        nodetype *Insert(size_t pos, Tbase *object)                         \
            { return (nodetype *)wxListBase::Insert(pos, object); }         \
        nodetype *Insert(nodetype *prev, Tbase *object)                     \
            { return (nodetype *)wxListBase::Insert(prev, object); }        \
                                                                            \
        nodetype *Append(long key, void *object)                            \
            { return (nodetype *)wxListBase::Append(key, object); }         \
        nodetype *Append(const wxChar *key, void *object)                   \
            { return (nodetype *)wxListBase::Append(key, object); }         \
                                                                            \
        nodetype *DetachNode(nodetype *node)                                \
            { return (nodetype *)wxListBase::DetachNode(node); }            \
        bool DeleteNode(nodetype *node)                                     \
            { return wxListBase::DeleteNode(node); }                        \
        bool DeleteObject(Tbase *object)                                    \
            { return wxListBase::DeleteObject(object); }                    \
        void Erase(nodetype *it)                                            \
            { DeleteNode(it); }                                             \
                                                                            \
        nodetype *Find(const Tbase *object) const                           \
            { return (nodetype *)wxListBase::Find(object); }                \
                                                                            \
        virtual nodetype *Find(const wxListKey& key) const                  \
            { return (nodetype *)wxListBase::Find(key); }                   \
                                                                            \
        bool Member(const Tbase *object) const                              \
            { return Find(object) != NULL; }                                \
                                                                            \
        int IndexOf(Tbase *object) const                                    \
            { return wxListBase::IndexOf(object); }                         \
                                                                            \
        void Sort(wxSortCompareFunction func)                               \
            { wxListBase::Sort(func); }                                     \
        void Sort(wxSortFuncFor_##name func)                                \
            { Sort((wxSortCompareFunction)func); }                          \
                                                                            \
    protected:                                                              \
        virtual wxNodeBase *CreateNode(wxNodeBase *prev, wxNodeBase *next,  \
                               void *data,                                  \
                               const wxListKey& key = wxDefaultListKey)     \
            {                                                               \
                return new nodetype(this,                                   \
                                    (nodetype *)prev, (nodetype *)next,     \
                                    (T *)data, key);                        \
            }                                                               \
                                                         \
    public:                                                                 \
        typedef size_t size_type;                                           \
        typedef int difference_type;                                        \
        typedef T* value_type;                                              \
        typedef Tbase* base_value_type;                                     \
        typedef value_type& reference;                                      \
        typedef const value_type& const_reference;                          \
        typedef base_value_type& base_reference;                            \
        typedef const base_value_type& const_base_reference;                \
                                                                            \
        classexp iterator                                                   \
        {                                                                   \
            typedef name list;                                              \
        public:                                                             \
            typedef nodetype Node;                                          \
            typedef iterator itor;                                          \
            typedef T* value_type;                                          \
            typedef value_type* ptr_type;                                   \
            typedef value_type& reference;                                  \
                                                                            \
            Node* m_node;                                                   \
            Node* m_init;                                                   \
        public:                                                             \
            typedef reference reference_type;                               \
            typedef ptr_type pointer_type;                                  \
                                                                            \
            iterator(Node* node, Node* init) : m_node(node), m_init(init) {}\
            iterator() : m_node(NULL), m_init(NULL) { }                     \
            reference_type operator*() const                                \
                { return *(pointer_type)m_node->GetDataPtr(); }             \
            ptrop                                                           \
            itor& operator++()                                              \
            {                                                               \
                wxASSERT_MSG( m_node, wxT("uninitialized iterator") );      \
                m_node = m_node->GetNext();                                 \
                return *this;                                               \
            }                                                               \
            const itor operator++(int)                                      \
            {                                                               \
                itor tmp = *this;                                           \
                wxASSERT_MSG( m_node, wxT("uninitialized iterator") );      \
                m_node = m_node->GetNext();                                 \
                return tmp;                                                 \
            }                                                               \
            itor& operator--()                                              \
            {                                                               \
                m_node = m_node ? m_node->GetPrevious() : m_init;           \
                return *this;                                               \
            }                                                               \
            const itor operator--(int)                                      \
            {                                                               \
                itor tmp = *this;                                           \
                m_node = m_node ? m_node->GetPrevious() : m_init;           \
                return tmp;                                                 \
            }                                                               \
            bool operator!=(const itor& it) const                           \
                { return it.m_node != m_node; }                             \
            bool operator==(const itor& it) const                           \
                { return it.m_node == m_node; }                             \
        };                                                                  \
        classexp const_iterator                                             \
        {                                                                   \
            typedef name list;                                              \
        public:                                                             \
            typedef nodetype Node;                                          \
            typedef T* value_type;                                          \
            typedef const value_type& const_reference;                      \
            typedef const_iterator itor;                                    \
            typedef value_type* ptr_type;                                   \
                                                                            \
            Node* m_node;                                                   \
            Node* m_init;                                                   \
        public:                                                             \
            typedef const_reference reference_type;                         \
            typedef const ptr_type pointer_type;                            \
                                                                            \
            const_iterator(Node* node, Node* init)                          \
                : m_node(node), m_init(init) { }                            \
            const_iterator() : m_node(NULL), m_init(NULL) { }               \
            const_iterator(const iterator& it)                              \
                : m_node(it.m_node), m_init(it.m_init) { }                  \
            reference_type operator*() const                                \
                { return *(pointer_type)m_node->GetDataPtr(); }             \
            ptrop                                                           \
            itor& operator++()                                              \
            {                                                               \
                wxASSERT_MSG( m_node, wxT("uninitialized iterator") );      \
                m_node = m_node->GetNext();                                 \
                return *this;                                               \
            }                                                               \
            const itor operator++(int)                                      \
            {                                                               \
                itor tmp = *this;                                           \
                wxASSERT_MSG( m_node, wxT("uninitialized iterator") );      \
                m_node = m_node->GetNext();                                 \
                return tmp;                                                 \
            }                                                               \
            itor& operator--()                                              \
            {                                                               \
                m_node = m_node ? m_node->GetPrevious() : m_init;           \
                return *this;                                               \
            }                                                               \
            const itor operator--(int)                                      \
            {                                                               \
                itor tmp = *this;                                           \
                m_node = m_node ? m_node->GetPrevious() : m_init;           \
                return tmp;                                                 \
            }                                                               \
            bool operator!=(const itor& it) const                           \
                { return it.m_node != m_node; }                             \
            bool operator==(const itor& it) const                           \
                { return it.m_node == m_node; }                             \
        };                                                                  \
        classexp reverse_iterator                                           \
        {                                                                   \
            typedef name list;                                              \
        public:                                                             \
            typedef nodetype Node;                                          \
            typedef T* value_type;                                          \
            typedef reverse_iterator itor;                                  \
            typedef value_type* ptr_type;                                   \
            typedef value_type& reference;                                  \
                                                                            \
            Node* m_node;                                                   \
            Node* m_init;                                                   \
        public:                                                             \
            typedef reference reference_type;                               \
            typedef ptr_type pointer_type;                                  \
                                                                            \
            reverse_iterator(Node* node, Node* init)                        \
                : m_node(node), m_init(init) { }                            \
            reverse_iterator() : m_node(NULL), m_init(NULL) { }             \
            reference_type operator*() const                                \
                { return *(pointer_type)m_node->GetDataPtr(); }             \
            ptrop                                                           \
            itor& operator++()                                              \
                { m_node = m_node->GetPrevious(); return *this; }           \
            const itor operator++(int)                                      \
            { itor tmp = *this; m_node = m_node->GetPrevious(); return tmp; }\
            itor& operator--()                                              \
            { m_node = m_node ? m_node->GetNext() : m_init; return *this; } \
            const itor operator--(int)                                      \
            {                                                               \
                itor tmp = *this;                                           \
                m_node = m_node ? m_node->GetNext() : m_init;               \
                return tmp;                                                 \
            }                                                               \
            bool operator!=(const itor& it) const                           \
                { return it.m_node != m_node; }                             \
            bool operator==(const itor& it) const                           \
                { return it.m_node == m_node; }                             \
        };                                                                  \
        classexp const_reverse_iterator                                     \
        {                                                                   \
            typedef name list;                                              \
        public:                                                             \
            typedef nodetype Node;                                          \
            typedef T* value_type;                                          \
            typedef const_reverse_iterator itor;                            \
            typedef value_type* ptr_type;                                   \
            typedef const value_type& const_reference;                      \
                                                                            \
            Node* m_node;                                                   \
            Node* m_init;                                                   \
        public:                                                             \
            typedef const_reference reference_type;                         \
            typedef const ptr_type pointer_type;                            \
                                                                            \
            const_reverse_iterator(Node* node, Node* init)                  \
                : m_node(node), m_init(init) { }                            \
            const_reverse_iterator() : m_node(NULL), m_init(NULL) { }       \
            const_reverse_iterator(const reverse_iterator& it)              \
                : m_node(it.m_node), m_init(it.m_init) { }                  \
            reference_type operator*() const                                \
                { return *(pointer_type)m_node->GetDataPtr(); }             \
            ptrop                                                           \
            itor& operator++()                                              \
                { m_node = m_node->GetPrevious(); return *this; }           \
            const itor operator++(int)                                      \
            { itor tmp = *this; m_node = m_node->GetPrevious(); return tmp; }\
            itor& operator--()                                              \
                { m_node = m_node ? m_node->GetNext() : m_init; return *this;}\
            const itor operator--(int)                                      \
            {                                                               \
                itor tmp = *this;                                           \
                m_node = m_node ? m_node->GetNext() : m_init;               \
                return tmp;                                                 \
            }                                                               \
            bool operator!=(const itor& it) const                           \
                { return it.m_node != m_node; }                             \
            bool operator==(const itor& it) const                           \
                { return it.m_node == m_node; }                             \
        };                                                                  \
                                                                            \
        wxEXPLICIT name(size_type n, const_reference v = value_type())      \
            { assign(n, v); }                                               \
        name(const const_iterator& first, const const_iterator& last)       \
            { assign(first, last); }                                        \
        iterator begin() { return iterator(GetFirst(), GetLast()); }        \
        const_iterator begin() const                                        \
            { return const_iterator(GetFirst(), GetLast()); }               \
        iterator end() { return iterator(NULL, GetLast()); }                \
        const_iterator end() const { return const_iterator(NULL, GetLast()); }\
        reverse_iterator rbegin()                                           \
            { return reverse_iterator(GetLast(), GetFirst()); }             \
        const_reverse_iterator rbegin() const                               \
            { return const_reverse_iterator(GetLast(), GetFirst()); }       \
        reverse_iterator rend() { return reverse_iterator(NULL, GetFirst()); }\
        const_reverse_iterator rend() const                                 \
            { return const_reverse_iterator(NULL, GetFirst()); }            \
        void resize(size_type n, value_type v = value_type())               \
        {                                                                   \
            while (n < size())                                              \
                pop_back();                                                 \
            while (n > size())                                              \
                push_back(v);                                                \
        }                                                                   \
        size_type size() const { return GetCount(); }                       \
        size_type max_size() const { return INT_MAX; }                      \
        bool empty() const { return IsEmpty(); }                            \
        reference front() { return *begin(); }                              \
        const_reference front() const { return *begin(); }                  \
        reference back() { iterator tmp = end(); return *--tmp; }           \
        const_reference back() const { const_iterator tmp = end(); return *--tmp; }\
        void push_front(const_reference v = value_type())                   \
            { Insert(GetFirst(), (const_base_reference)v); }                \
        void pop_front() { DeleteNode(GetFirst()); }                        \
        void push_back(const_reference v = value_type())                    \
            { Append((const_base_reference)v); }                            \
        void pop_back() { DeleteNode(GetLast()); }                          \
        void assign(const_iterator first, const const_iterator& last)       \
        {                                                                   \
            clear();                                                        \
            for(; first != last; ++first)                                   \
                Append((const_base_reference)*first);                       \
        }                                                                   \
        void assign(size_type n, const_reference v = value_type())          \
        {                                                                   \
            clear();                                                        \
            for(size_type i = 0; i < n; ++i)                                \
                Append((const_base_reference)v);                            \
        }                                                                   \
        iterator insert(const iterator& it, const_reference v)              \
        {                                                                   \
            if ( it == end() )                                              \
            {                                                               \
                Append((const_base_reference)v);                            \
                                                                         \
                iterator itins(end());                                      \
                return --itins;                                             \
            }                                                               \
            else                                                            \
            {                                                               \
                Insert(it.m_node, (const_base_reference)v);                 \
                iterator itins(it);                                         \
                return --itins;                                             \
            }                                                               \
        }                                                                   \
        void insert(const iterator& it, size_type n, const_reference v)     \
        {                                                                   \
            for(size_type i = 0; i < n; ++i)                                \
                insert(it, v);                                              \
        }                                                                   \
        void insert(const iterator& it,                                     \
                    const_iterator first, const const_iterator& last)       \
        {                                                                   \
            for(; first != last; ++first)                                   \
                insert(it, *first);                                         \
        }                                                                   \
        iterator erase(const iterator& it)                                  \
        {                                                                   \
            iterator next = iterator(it.m_node->GetNext(), GetLast());      \
            DeleteNode(it.m_node); return next;                             \
        }                                                                   \
        iterator erase(const iterator& first, const iterator& last)         \
        {                                                                   \
            iterator next = last;                                           \
            if ( next != end() )                                            \
                ++next;                                                     \
            DeleteNodes(first.m_node, last.m_node);                         \
            return next;                                                    \
        }                                                                   \
        void clear() { Clear(); }                                           \
        void splice(const iterator& it, name& l, const iterator& first, const iterator& last)\
            { insert(it, first, last); l.erase(first, last); }              \
        void splice(const iterator& it, name& l)                            \
            { splice(it, l, l.begin(), l.end() ); }                         \
        void splice(const iterator& it, name& l, const iterator& first)     \
        {                                                                   \
            if ( it != first )                                              \
            {                                                               \
                insert(it, *first);                                         \
                l.erase(first);                                             \
            }                                                               \
        }                                                                   \
        void remove(const_reference v)                                      \
            { DeleteObject((const_base_reference)v); }                      \
        void reverse()                                                      \
            { Reverse(); }                                                  \
                                                                     \
    }

#define WX_LIST_PTROP                                                       \
            pointer_type operator->() const                                 \
                { return (pointer_type)m_node->GetDataPtr(); }
#define WX_LIST_PTROP_NONE

#define WX_DECLARE_LIST_3(T, Tbase, name, nodetype, classexp)               \
    WX_DECLARE_LIST_4(T, Tbase, name, nodetype, classexp, WX_LIST_PTROP_NONE)
#define WX_DECLARE_LIST_PTR_3(T, Tbase, name, nodetype, classexp)        \
    WX_DECLARE_LIST_4(T, Tbase, name, nodetype, classexp, WX_LIST_PTROP)

#define WX_DECLARE_LIST_2(elementtype, listname, nodename, classexp)        \
    WX_DECLARE_LIST_3(elementtype, elementtype, listname, nodename, classexp)
#define WX_DECLARE_LIST_PTR_2(elementtype, listname, nodename, classexp)        \
    WX_DECLARE_LIST_PTR_3(elementtype, elementtype, listname, nodename, classexp)

#define WX_DECLARE_LIST(elementtype, listname)                              \
    typedef elementtype _WX_LIST_ITEM_TYPE_##listname;                      \
    WX_DECLARE_LIST_2(elementtype, listname, wx##listname##Node, class)
#define WX_DECLARE_LIST_PTR(elementtype, listname)                              \
    typedef elementtype _WX_LIST_ITEM_TYPE_##listname;                      \
    WX_DECLARE_LIST_PTR_2(elementtype, listname, wx##listname##Node, class)

#define WX_DECLARE_LIST_WITH_DECL(elementtype, listname, decl) \
    typedef elementtype _WX_LIST_ITEM_TYPE_##listname;                      \
    WX_DECLARE_LIST_2(elementtype, listname, wx##listname##Node, decl)

#define WX_DECLARE_EXPORTED_LIST(elementtype, listname)                     \
    WX_DECLARE_LIST_WITH_DECL(elementtype, listname, class WXDLLIMPEXP_CORE)

#define WX_DECLARE_EXPORTED_LIST_PTR(elementtype, listname)                     \
    typedef elementtype _WX_LIST_ITEM_TYPE_##listname;                      \
    WX_DECLARE_LIST_PTR_2(elementtype, listname, wx##listname##Node, class WXDLLIMPEXP_CORE)

#define WX_DECLARE_USER_EXPORTED_LIST(elementtype, listname, usergoo)       \
    typedef elementtype _WX_LIST_ITEM_TYPE_##listname;                      \
    WX_DECLARE_LIST_2(elementtype, listname, wx##listname##Node, class usergoo)
#define WX_DECLARE_USER_EXPORTED_LIST_PTR(elementtype, listname, usergoo)       \
    typedef elementtype _WX_LIST_ITEM_TYPE_##listname;                      \
    WX_DECLARE_LIST_PTR_2(elementtype, listname, wx##listname##Node, class usergoo)

#define WX_DEFINE_LIST(name)    "don't forget to include listimpl.cpp!"

#define WX_DEFINE_EXPORTED_LIST(name)      WX_DEFINE_LIST(name)
#define WX_DEFINE_USER_EXPORTED_LIST(name) WX_DEFINE_LIST(name)

#endif 


#if defined(wxLIST_COMPATIBILITY)


#if !wxUSE_STD_CONTAINERS


inline wxNode *wxNodeBase::Next() const { return (wxNode *)GetNext(); }
inline wxNode *wxNodeBase::Previous() const { return (wxNode *)GetPrevious(); }
inline wxObject *wxNodeBase::Data() const { return (wxObject *)GetData(); }


inline int wxListBase::Number() const { return (int)GetCount(); }
inline wxNode *wxListBase::First() const { return (wxNode *)GetFirst(); }
inline wxNode *wxListBase::Last() const { return (wxNode *)GetLast(); }
inline wxNode *wxListBase::Nth(size_t n) const { return (wxNode *)Item(n); }
inline wxListBase::operator wxList&() const { return *(wxList*)this; }

#endif



WX_DECLARE_LIST_2(wxObject, wxObjectList, wxObjectListNode,
                        class WXDLLIMPEXP_BASE);

class WXDLLIMPEXP_BASE wxList : public wxObjectList
{
public:
#if defined(wxWARN_COMPAT_LIST_USE) && !wxUSE_STD_CONTAINERS
    wxList() { }
    wxDEPRECATED( wxList(int key_type) );
#elif !wxUSE_STD_CONTAINERS
    wxList(int key_type = wxKEY_NONE);
#endif

       ~wxList() { }

#if !wxUSE_STD_CONTAINERS
    wxList& operator=(const wxList& list)
        { if (&list != this) Assign(list); return *this; }

        void Sort(wxSortCompareFunction compfunc) { wxListBase::Sort(compfunc); }
#endif 
    template<typename T>
    wxVector<T> AsVector() const
    {
        wxVector<T> vector(size());
        size_t i = 0;

        for ( const_iterator it = begin(); it != end(); ++it )
        {
            vector[i++] = static_cast<T>(*it);
        }

        return vector;
    }

};

#if !wxUSE_STD_CONTAINERS

WX_DECLARE_LIST_2(wxChar, wxStringListBase, wxStringListNode, class WXDLLIMPEXP_BASE);

class WXDLLIMPEXP_BASE wxStringList : public wxStringListBase
{
public:
            #ifdef wxWARN_COMPAT_LIST_USE
    wxStringList();
    wxDEPRECATED( wxStringList(const wxChar *first ...) ); #else
    wxStringList();
    wxStringList(const wxChar *first ...); #endif

                    wxStringList(const wxStringList& other) : wxStringListBase() { DeleteContents(true); DoCopy(other); }
    wxStringList& operator=(const wxStringList& other)
    {
        if (&other != this)
        {
            Clear();
            DoCopy(other);
        }
        return *this;
    }

                wxNode *Add(const wxChar *s);

            wxNode *Prepend(const wxChar *s);

    bool Delete(const wxChar *s);

    wxChar **ListToArray(bool new_copies = false) const;
    bool Member(const wxChar *s) const;

        void Sort();

private:
    void DoCopy(const wxStringList&); };

#else 
WX_DECLARE_LIST_XO(wxString, wxStringListBase, class WXDLLIMPEXP_BASE);

class WXDLLIMPEXP_BASE wxStringList : public wxStringListBase
{
public:
    compatibility_iterator Append(wxChar* s)
        { wxString tmp = s; delete[] s; return wxStringListBase::Append(tmp); }
    compatibility_iterator Insert(wxChar* s)
        { wxString tmp = s; delete[] s; return wxStringListBase::Insert(tmp); }
    compatibility_iterator Insert(size_t pos, wxChar* s)
    {
        wxString tmp = s;
        delete[] s;
        return wxStringListBase::Insert(pos, tmp);
    }
    compatibility_iterator Add(const wxChar* s)
        { push_back(s); return GetLast(); }
    compatibility_iterator Prepend(const wxChar* s)
        { push_front(s); return GetFirst(); }
};

#endif 
#endif 
#define WX_CLEAR_LIST(type, list)                                            \
    {                                                                        \
        type::iterator it, en;                                               \
        for( it = (list).begin(), en = (list).end(); it != en; ++it )        \
            delete *it;                                                      \
        (list).clear();                                                      \
    }

#define WX_APPEND_LIST(list, other)                                           \
    {                                                                         \
        wxList::compatibility_iterator node = other->GetFirst();              \
        while ( node )                                                        \
        {                                                                     \
            (list)->push_back(node->GetData());                               \
            node = node->GetNext();                                           \
        }                                                                     \
    }

#endif 