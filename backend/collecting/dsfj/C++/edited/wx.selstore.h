
#ifndef _WX_SELSTORE_H_
#define _WX_SELSTORE_H_

#include "wx/dynarray.h"


inline int CMPFUNC_CONV wxUIntCmp(unsigned n1, unsigned n2)
{
    return (int)(n1 - n2);
}

WX_DEFINE_SORTED_EXPORTED_ARRAY_CMP_INT(unsigned, wxUIntCmp, wxSelectedIndices);


class WXDLLIMPEXP_CORE wxSelectionStore
{
public:
    wxSelectionStore() : m_itemsSel(wxUIntCmp) { Init(); }

        void SetItemCount(unsigned count);

        void Clear() { m_itemsSel.Clear(); m_count = 0; m_defaultState = false; }

        void OnItemsInserted(unsigned item, unsigned numItems);

        void OnItemDelete(unsigned item);

            bool OnItemsDeleted(unsigned item, unsigned numItems);

                bool SelectItem(unsigned item, bool select = true);

                            bool SelectRange(unsigned itemFrom, unsigned itemTo,
                     bool select = true,
                     wxArrayInt *itemsChanged = NULL);

        bool IsSelected(unsigned item) const;

        bool IsEmpty() const
    {
        return m_defaultState ? m_itemsSel.size() == m_count
                              : m_itemsSel.empty();
    }

        unsigned GetSelectedCount() const
    {
        return m_defaultState ? m_count - m_itemsSel.GetCount()
                              : m_itemsSel.GetCount();
    }

            typedef size_t IterationState;

        static const unsigned NO_SELECTION;

        unsigned GetFirstSelectedItem(IterationState& cookie) const;

        unsigned GetNextSelectedItem(IterationState& cookie) const;

private:
        void Init() { m_count = 0; m_defaultState = false; }

        unsigned m_count;

                bool m_defaultState;

        wxSelectedIndices m_itemsSel;

    wxDECLARE_NO_COPY_CLASS(wxSelectionStore);
};

#endif 
