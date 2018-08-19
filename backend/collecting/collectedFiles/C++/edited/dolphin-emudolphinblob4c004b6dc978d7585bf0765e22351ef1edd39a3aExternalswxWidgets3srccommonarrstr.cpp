

#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/arrstr.h"
#include "wx/scopedarray.h"

#include "wx/beforestd.h"
#include <algorithm>
#include <functional>
#include "wx/afterstd.h"


wxArrayString::wxArrayString(size_t sz, const char** a)
{
#if !wxUSE_STD_CONTAINERS
    Init(false);
#endif
    for (size_t i=0; i < sz; i++)
        Add(a[i]);
}

wxArrayString::wxArrayString(size_t sz, const wchar_t** a)
{
#if !wxUSE_STD_CONTAINERS
    Init(false);
#endif
    for (size_t i=0; i < sz; i++)
        Add(a[i]);
}

wxArrayString::wxArrayString(size_t sz, const wxString* a)
{
#if !wxUSE_STD_CONTAINERS
    Init(false);
#endif
    for (size_t i=0; i < sz; i++)
        Add(a[i]);
}

#if !wxUSE_STD_CONTAINERS

#define   ARRAY_MAXSIZE_INCREMENT       4096

#ifndef   ARRAY_DEFAULT_INITIAL_SIZE    #define   ARRAY_DEFAULT_INITIAL_SIZE    (16)
#endif

void wxArrayString::Init(bool autoSort)
{
  m_nSize  =
  m_nCount = 0;
  m_pItems = NULL;
  m_compareFunction = NULL;
  m_autoSort = autoSort;
}

wxArrayString::wxArrayString(const wxArrayString& src)
{
  Init(src.m_autoSort);

  *this = src;
}

wxArrayString& wxArrayString::operator=(const wxArrayString& src)
{
  if ( m_nSize > 0 )
    Clear();

  Copy(src);

  m_autoSort = src.m_autoSort;

  return *this;
}

void wxArrayString::Copy(const wxArrayString& src)
{
  if ( src.m_nCount > ARRAY_DEFAULT_INITIAL_SIZE )
    Alloc(src.m_nCount);

  for ( size_t n = 0; n < src.m_nCount; n++ )
    Add(src[n]);
}

wxString *wxArrayString::Grow(size_t nIncrement)
{
    if ( (m_nSize - m_nCount) >= nIncrement )
    {
                return NULL;
    }

            #if ARRAY_DEFAULT_INITIAL_SIZE == 0
      #error "ARRAY_DEFAULT_INITIAL_SIZE must be > 0!"
    #endif

    if ( m_nSize == 0 ) {
            m_nSize = ARRAY_DEFAULT_INITIAL_SIZE;
      if (m_nSize < nIncrement)
          m_nSize = nIncrement;
      m_pItems = new wxString[m_nSize];

            return NULL;
    }
    else {
                        size_t ndefIncrement = m_nSize < ARRAY_DEFAULT_INITIAL_SIZE
                          ? ARRAY_DEFAULT_INITIAL_SIZE : m_nSize >> 1;
      if ( ndefIncrement > ARRAY_MAXSIZE_INCREMENT )
        ndefIncrement = ARRAY_MAXSIZE_INCREMENT;
      if ( nIncrement < ndefIncrement )
        nIncrement = ndefIncrement;
      m_nSize += nIncrement;
      wxString *pNew = new wxString[m_nSize];

            for ( size_t j = 0; j < m_nCount; j++ )
          pNew[j] = m_pItems[j];

      wxString* const pItemsOld = m_pItems;

      m_pItems = pNew;

      return pItemsOld;
    }
}

void wxArrayString::Empty()
{
  m_nCount = 0;
}

void wxArrayString::Clear()
{
  m_nSize  =
  m_nCount = 0;

  wxDELETEA(m_pItems);
}

wxArrayString::~wxArrayString()
{
  delete [] m_pItems;
}

void wxArrayString::reserve(size_t nSize)
{
    Alloc(nSize);
}

void wxArrayString::Alloc(size_t nSize)
{
    if ( nSize > m_nSize ) {
    wxString *pNew = new wxString[nSize];
    if ( !pNew )
        return;

    for ( size_t j = 0; j < m_nCount; j++ )
        pNew[j] = m_pItems[j];
    delete [] m_pItems;

    m_pItems = pNew;
    m_nSize  = nSize;
  }
}

void wxArrayString::Shrink()
{
    if( m_nCount < m_nSize ) {
        wxString *pNew = new wxString[m_nCount];

        for ( size_t j = 0; j < m_nCount; j++ )
        pNew[j] = m_pItems[j];
    delete [] m_pItems;
    m_pItems = pNew;
    m_nSize = m_nCount;
  }
}

int wxArrayString::Index(const wxString& str, bool bCase, bool bFromEnd) const
{
  if ( m_autoSort ) {
        wxASSERT_MSG( bCase && !bFromEnd,
                  wxT("search parameters ignored for auto sorted array") );

    size_t i,
           lo = 0,
           hi = m_nCount;
    int res;
    while ( lo < hi ) {
      i = (lo + hi)/2;

      res = str.compare(m_pItems[i]);
      if ( res < 0 )
        hi = i;
      else if ( res > 0 )
        lo = i + 1;
      else
        return i;
    }

    return wxNOT_FOUND;
  }
  else {
        if ( bFromEnd ) {
      if ( m_nCount > 0 ) {
        size_t ui = m_nCount;
        do {
          if ( m_pItems[--ui].IsSameAs(str, bCase) )
            return ui;
        }
        while ( ui != 0 );
      }
    }
    else {
      for( size_t ui = 0; ui < m_nCount; ui++ ) {
        if( m_pItems[ui].IsSameAs(str, bCase) )
          return ui;
      }
    }
  }

  return wxNOT_FOUND;
}

size_t wxArrayString::Add(const wxString& str, size_t nInsert)
{
  if ( m_autoSort ) {
        size_t i,
           lo = 0,
           hi = m_nCount;
    int res;
    while ( lo < hi ) {
      i = (lo + hi)/2;

      res = m_compareFunction ? m_compareFunction(str, m_pItems[i]) : str.Cmp(m_pItems[i]);
      if ( res < 0 )
        hi = i;
      else if ( res > 0 )
        lo = i + 1;
      else {
        lo = hi = i;
        break;
      }
    }

    wxASSERT_MSG( lo == hi, wxT("binary search broken") );

    Insert(str, lo, nInsert);

    return (size_t)lo;
  }
  else {
                wxScopedArray<wxString> oldStrings(Grow(nInsert));

    for (size_t i = 0; i < nInsert; i++)
    {
                m_pItems[m_nCount + i] = str;
    }
    size_t ret = m_nCount;
    m_nCount += nInsert;
    return ret;
  }
}

void wxArrayString::Insert(const wxString& str, size_t nIndex, size_t nInsert)
{
  wxCHECK_RET( nIndex <= m_nCount, wxT("bad index in wxArrayString::Insert") );
  wxCHECK_RET( m_nCount <= m_nCount + nInsert,
               wxT("array size overflow in wxArrayString::Insert") );

  wxScopedArray<wxString> oldStrings(Grow(nInsert));

  for (int j = m_nCount - nIndex - 1; j >= 0; j--)
      m_pItems[nIndex + nInsert + j] = m_pItems[nIndex + j];

  for (size_t i = 0; i < nInsert; i++)
  {
      m_pItems[nIndex + i] = str;
  }
  m_nCount += nInsert;
}

void
wxArrayString::insert(iterator it, const_iterator first, const_iterator last)
{
    const int idx = it - begin();

        wxScopedArray<wxString> oldStrings(Grow(last - first));

        it = begin() + idx;

    while ( first != last )
    {
        it = insert(it, *first);

                        ++it;

        ++first;
    }
}

void wxArrayString::resize(size_type n, value_type v)
{
  if ( n < m_nCount )
      m_nCount = n;
  else if ( n > m_nCount )
      Add(v, n - m_nCount);
}

void wxArrayString::SetCount(size_t count)
{
    Alloc(count);

    wxString s;
    while ( m_nCount < count )
        m_pItems[m_nCount++] = s;
}

void wxArrayString::RemoveAt(size_t nIndex, size_t nRemove)
{
  wxCHECK_RET( nIndex < m_nCount, wxT("bad index in wxArrayString::Remove") );
  wxCHECK_RET( nIndex + nRemove <= m_nCount,
               wxT("removing too many elements in wxArrayString::Remove") );

  for ( size_t j =  0; j < m_nCount - nIndex -nRemove; j++)
      m_pItems[nIndex + j] = m_pItems[nIndex + nRemove + j];

  m_nCount -= nRemove;
}

void wxArrayString::Remove(const wxString& sz)
{
  int iIndex = Index(sz);

  wxCHECK_RET( iIndex != wxNOT_FOUND,
               wxT("removing inexistent element in wxArrayString::Remove") );

  RemoveAt(iIndex);
}


struct wxSortPredicateAdaptor
{
    wxSortPredicateAdaptor(wxArrayString::CompareFunction compareFunction)
        : m_compareFunction(compareFunction)
    {
    }

    bool operator()(const wxString& first, const wxString& second) const
    {
        return (*m_compareFunction)(first, second) < 0;
    }

    wxArrayString::CompareFunction m_compareFunction;
};

void wxArrayString::Sort(CompareFunction compareFunction)
{
    wxCHECK_RET( !m_autoSort, wxT("can't use this method with sorted arrays") );

    std::sort(m_pItems, m_pItems + m_nCount,
                wxSortPredicateAdaptor(compareFunction));
}

struct wxSortPredicateAdaptor2
{
    wxSortPredicateAdaptor2(wxArrayString::CompareFunction2 compareFunction)
        : m_compareFunction(compareFunction)
    {
    }

    bool operator()(const wxString& first, const wxString& second) const
    {
        return (*m_compareFunction)(const_cast<wxString *>(&first),
                                    const_cast<wxString *>(&second)) < 0;
    }

    wxArrayString::CompareFunction2 m_compareFunction;
};

void wxArrayString::Sort(CompareFunction2 compareFunction)
{
    std::sort(m_pItems, m_pItems + m_nCount,
                wxSortPredicateAdaptor2(compareFunction));
}

void wxArrayString::Sort(bool reverseOrder)
{
    if ( reverseOrder )
        std::sort(m_pItems, m_pItems + m_nCount, std::greater<wxString>());
    else         std::sort(m_pItems, m_pItems + m_nCount);
}

bool wxArrayString::operator==(const wxArrayString& a) const
{
    if ( m_nCount != a.m_nCount )
        return false;

    for ( size_t n = 0; n < m_nCount; n++ )
    {
        if ( Item(n) != a[n] )
            return false;
    }

    return true;
}

#endif 

#include "wx/tokenzr.h"

wxString wxJoin(const wxArrayString& arr, const wxChar sep, const wxChar escape)
{
    size_t count = arr.size();
    if ( count == 0 )
        return wxEmptyString;

    wxString str;

                str.reserve(count*(arr[0].length() + arr[count-1].length()) / 2);

    if ( escape == wxT('\0') )
    {
                for ( size_t i = 0; i < count; i++ )
        {
            if ( i )
                str += sep;
            str += arr[i];
        }
    }
    else     {
        for ( size_t n = 0; n < count; n++ )
        {
            if ( n )
                str += sep;

            for ( wxString::const_iterator i = arr[n].begin(),
                                         end = arr[n].end();
                  i != end;
                  ++i )
            {
                const wxChar ch = *i;
                if ( ch == sep )
                    str += escape;                      str += ch;
            }
        }
    }

    str.Shrink();     return str;
}

wxArrayString wxSplit(const wxString& str, const wxChar sep, const wxChar escape)
{
    if ( escape == wxT('\0') )
    {
                return wxStringTokenize(str, sep, wxTOKEN_RET_EMPTY_ALL);
    }

    wxArrayString ret;
    wxString curr;
    wxChar prev = wxT('\0');

    for ( wxString::const_iterator i = str.begin(),
                                 end = str.end();
          i != end;
          ++i )
    {
        const wxChar ch = *i;

        if ( ch == sep )
        {
            if ( prev == escape )
            {
                                                *curr.rbegin() = sep;
            }
            else             {
                ret.push_back(curr);
                curr.clear();
            }
        }
        else         {
            curr += ch;
        }

        prev = ch;
    }

        if ( !curr.empty() || prev == sep )
        ret.Add(curr);

    return ret;
}
