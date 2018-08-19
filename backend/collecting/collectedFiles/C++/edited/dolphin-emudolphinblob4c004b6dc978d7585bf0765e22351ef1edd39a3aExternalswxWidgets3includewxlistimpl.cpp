
#if wxUSE_STD_CONTAINERS

#undef  WX_DEFINE_LIST
#define WX_DEFINE_LIST(name)                                                  \
    void _WX_LIST_HELPER_##name::DeleteFunction( _WX_LIST_ITEM_TYPE_##name X )\
    {                                                                         \
        delete X;                                                             \
    }                                                                         \
    _WX_LIST_HELPER_##name::BaseListType _WX_LIST_HELPER_##name::EmptyList;

#else     #undef WX_DEFINE_LIST_2
    #define WX_DEFINE_LIST_2(T, name)     \
        void wx##name##Node::DeleteData() \
        {                                 \
            delete (T *)GetData();        \
        }

            #undef  WX_DEFINE_LIST
    #define WX_DEFINE_LIST(name) WX_DEFINE_LIST_2(_WX_LIST_ITEM_TYPE_##name, name)

#endif 
