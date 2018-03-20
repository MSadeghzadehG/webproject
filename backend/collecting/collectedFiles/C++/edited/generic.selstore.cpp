


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/selstore.h"


const unsigned wxSelectionStore::NO_SELECTION = static_cast<unsigned>(-1);


bool wxSelectionStore::IsSelected(unsigned item) const
{
    bool isSel = m_itemsSel.Index(item) != wxNOT_FOUND;

            return m_defaultState ? !isSel : isSel;
}


bool wxSelectionStore::SelectItem(unsigned item, bool select)
{
                size_t index = m_itemsSel.IndexForInsert(item);
    bool isSel = index < m_itemsSel.GetCount() && m_itemsSel[index] == item;

    if ( select != m_defaultState )
    {
        if ( !isSel )
        {
            m_itemsSel.AddAt(item, index);

            return true;
        }
    }
    else     {
        if ( isSel )
        {
            m_itemsSel.RemoveAt(index);
            return true;
        }
    }

    return false;
}

bool wxSelectionStore::SelectRange(unsigned itemFrom, unsigned itemTo,
                                   bool select,
                                   wxArrayInt *itemsChanged)
{
                static const unsigned MANY_ITEMS = 100;

    wxASSERT_MSG( itemFrom <= itemTo, wxT("should be in order") );

        if ( itemTo - itemFrom > m_count/2 )
    {
        if ( select != m_defaultState )
        {
                        m_defaultState = select;

                                    wxSelectedIndices selOld = m_itemsSel;
            m_itemsSel.Empty();

                        
            unsigned item;
            for ( item = 0; item < itemFrom; item++ )
            {
                if ( selOld.Index(item) == wxNOT_FOUND )
                    m_itemsSel.Add(item);
            }

            for ( item = itemTo + 1; item < m_count; item++ )
            {
                if ( selOld.Index(item) == wxNOT_FOUND )
                    m_itemsSel.Add(item);
            }

                        itemsChanged = NULL;
        }
        else         {
                        size_t count = m_itemsSel.GetCount(),
                   start = m_itemsSel.IndexForInsert(itemFrom),
                   end = m_itemsSel.IndexForInsert(itemTo);

            if ( start == count || m_itemsSel[start] < itemFrom )
            {
                start++;
            }

            if ( end == count || m_itemsSel[end] > itemTo )
            {
                end--;
            }

            if ( start <= end )
            {
                                for ( int i = end; i >= (int)start; i-- )
                {
                    if ( itemsChanged )
                    {
                        if ( itemsChanged->GetCount() > MANY_ITEMS )
                        {
                                                        itemsChanged = NULL;
                        }
                        else
                        {
                            itemsChanged->Add(m_itemsSel[i]);
                        }
                    }

                    m_itemsSel.RemoveAt(i);
                }
            }
        }
    }
    else     {
        if ( itemsChanged )
        {
            itemsChanged->Empty();
        }

                for ( unsigned item = itemFrom; item <= itemTo; item++ )
        {
            if ( SelectItem(item, select) && itemsChanged )
            {
                itemsChanged->Add(item);

                if ( itemsChanged->GetCount() > MANY_ITEMS )
                {
                                                                                itemsChanged = NULL;
                }
            }
        }
    }

        return itemsChanged != NULL;
}


void wxSelectionStore::OnItemsInserted(unsigned item, unsigned numItems)
{
    const size_t count = m_itemsSel.GetCount();

    size_t idx = m_itemsSel.IndexForInsert(item);

    for ( size_t i = idx; i < count; i++ )
    {
        m_itemsSel[i] += numItems;
    }

    if ( m_defaultState )
    {
                                for ( unsigned n = item; n < item + numItems; n++ )
        {
            m_itemsSel.AddAt(item, idx++);
        }
    }

    m_count += numItems;
}

void wxSelectionStore::OnItemDelete(unsigned item)
{
    size_t count = m_itemsSel.GetCount(),
           i = m_itemsSel.IndexForInsert(item);

    if ( i < count && m_itemsSel[i] == item )
    {
                m_itemsSel.RemoveAt(i);

        count--;
    }

        while ( i < count )
    {
                wxASSERT_MSG( m_itemsSel[i] > item, wxT("logic error") );

        m_itemsSel[i++]--;
    }

    m_count--;
}

bool wxSelectionStore::OnItemsDeleted(unsigned item, unsigned numItems)
{
    bool anyDeletedInSelItems = false,
         allDeletedInSelItems = true;

    size_t i = m_itemsSel.IndexForInsert(item);

    const unsigned firstAfterDeleted = item + numItems;
    while ( i < m_itemsSel.size() )
    {
        if ( m_itemsSel[i] < firstAfterDeleted )
        {
                                                m_itemsSel.RemoveAt(i);

            anyDeletedInSelItems = true;
        }
        else
        {
                        m_itemsSel[i++] -= numItems;

            allDeletedInSelItems = false;
        }
    }

    m_count -= numItems;

    return m_defaultState ? allDeletedInSelItems : anyDeletedInSelItems;
}


void wxSelectionStore::SetItemCount(unsigned count)
{
            if ( count < m_count )
    {
        for ( size_t i = m_itemsSel.GetCount(); i > 0; i-- )
        {
            if ( m_itemsSel[i - 1] >= count )
                m_itemsSel.RemoveAt(i - 1);
        }
    }

        m_count = count;
}


unsigned wxSelectionStore::GetFirstSelectedItem(IterationState& cookie) const
{
    cookie = 0;

    return GetNextSelectedItem(cookie);
}

unsigned wxSelectionStore::GetNextSelectedItem(IterationState& cookie) const
{
    if ( m_defaultState )
    {
                                for ( unsigned item = cookie; ; item++ )
        {
            if ( item >= m_count )
                return NO_SELECTION;

            if ( IsSelected(item) )
            {
                cookie = item + 1;
                return item;
            }
        }
    }
    else     {
        if ( cookie >= m_itemsSel.size() )
            return NO_SELECTION;

        return m_itemsSel[cookie++];
    }
}
