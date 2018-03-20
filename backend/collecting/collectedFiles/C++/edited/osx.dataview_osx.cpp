
#include "wx/wxprec.h"

#if (wxUSE_DATAVIEWCTRL != 0) && (!defined(wxUSE_GENERICDATAVIEWCTRL) || (wxUSE_GENERICDATAVIEWCTRL == 0))

#include <limits>

#ifndef WX_PRECOMP
    #include "wx/timer.h"
    #include "wx/settings.h"
    #include "wx/dcclient.h"
    #include "wx/icon.h"
#endif

#include "wx/osx/core/dataview.h"
#include "wx/osx/private.h"
#include "wx/renderer.h"

wxString ConcatenateDataViewItemValues(wxDataViewCtrl const* dataViewCtrlPtr, wxDataViewItem const& dataViewItem)
{
  wxString dataString; 

  for (size_t i=0; i<dataViewCtrlPtr->GetColumnCount(); i++)
  {
       wxVariant dataValue;

    dataViewCtrlPtr->GetModel()->GetValue(dataValue,dataViewItem,dataViewCtrlPtr->GetColumn(i)->GetModelColumn());
    if (i > 0)
      dataString << wxT('\t');
    dataString << dataValue.MakeString();
  }
  return dataString;
}

class wxOSXDataViewModelNotifier : public wxDataViewModelNotifier
{
public:
     wxOSXDataViewModelNotifier(wxDataViewCtrl* initDataViewCtrlPtr);

     virtual bool ItemAdded   (wxDataViewItem const &parent, wxDataViewItem const &item) wxOVERRIDE;
  virtual bool ItemsAdded  (wxDataViewItem const& parent, wxDataViewItemArray const& items) wxOVERRIDE;
  virtual bool ItemChanged (wxDataViewItem const& item) wxOVERRIDE;
  virtual bool ItemsChanged(wxDataViewItemArray const& items) wxOVERRIDE;
  virtual bool ItemDeleted (wxDataViewItem const& parent, wxDataViewItem const& item) wxOVERRIDE;
  virtual bool ItemsDeleted(wxDataViewItem const& parent, wxDataViewItemArray const& items) wxOVERRIDE;
  virtual bool ValueChanged(wxDataViewItem const& item, unsigned int col) wxOVERRIDE;
  virtual bool Cleared() wxOVERRIDE;
  virtual void Resort() wxOVERRIDE;

protected:
    void AdjustRowHeight(wxDataViewItem const& item);
   void AdjustRowHeights(wxDataViewItemArray const& items);
   void AdjustAutosizedColumns();

private:
  wxDataViewCtrl* m_DataViewCtrlPtr;
};

wxOSXDataViewModelNotifier::wxOSXDataViewModelNotifier(wxDataViewCtrl* initDataViewCtrlPtr)
                           :m_DataViewCtrlPtr(initDataViewCtrlPtr)
{
  if (initDataViewCtrlPtr == NULL)
    wxFAIL_MSG("Pointer to dataview control must not be NULL");
}

bool wxOSXDataViewModelNotifier::ItemAdded(wxDataViewItem const& parent, wxDataViewItem const& item)
{
  bool noFailureFlag;


  wxCHECK_MSG(item.IsOk(),false,"Added item is invalid.");
  noFailureFlag = m_DataViewCtrlPtr->GetDataViewPeer()->Add(parent,item);
  AdjustRowHeight(item);
  return noFailureFlag;
}

bool wxOSXDataViewModelNotifier::ItemsAdded(wxDataViewItem const& parent, wxDataViewItemArray const& items)
{
  bool noFailureFlag;


   noFailureFlag = m_DataViewCtrlPtr->GetDataViewPeer()->Add(parent,items);
   AdjustRowHeights(items);
   return noFailureFlag;
}

bool wxOSXDataViewModelNotifier::ItemChanged(wxDataViewItem const& item)
{
  wxCHECK_MSG(item.IsOk(),             false,"Changed item is invalid.");
  wxCHECK_MSG(GetOwner() != NULL,false,"Owner not initialized.");
  if (m_DataViewCtrlPtr->GetDataViewPeer()->Update(GetOwner()->GetParent(item),item))
  {
       wxDataViewEvent dataViewEvent(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED,m_DataViewCtrlPtr->GetId());

    dataViewEvent.SetEventObject(m_DataViewCtrlPtr);
    dataViewEvent.SetModel(m_DataViewCtrlPtr->GetModel());
    dataViewEvent.SetItem(item);
       m_DataViewCtrlPtr->HandleWindowEvent(dataViewEvent);
       AdjustRowHeight(item);
    AdjustAutosizedColumns();
       return true;
  }
  else
    return false;
}

bool wxOSXDataViewModelNotifier::ItemsChanged(wxDataViewItemArray const& items)
{
  size_t const noOfItems = items.GetCount();

  wxDataViewEvent dataViewEvent(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED,m_DataViewCtrlPtr->GetId());


  dataViewEvent.SetEventObject(m_DataViewCtrlPtr);
  dataViewEvent.SetModel(m_DataViewCtrlPtr->GetModel());
  for (size_t indexItem=0; indexItem<noOfItems; ++indexItem)
    if (m_DataViewCtrlPtr->GetDataViewPeer()->Update(GetOwner()->GetParent(items[indexItem]),items[indexItem]))
    {
           dataViewEvent.SetItem(items[indexItem]);
      m_DataViewCtrlPtr->HandleWindowEvent(dataViewEvent);
    }
    else
      return false;
   AdjustRowHeights(items);
  AdjustAutosizedColumns();
   return true;
}

bool wxOSXDataViewModelNotifier::ItemDeleted(wxDataViewItem const& parent, wxDataViewItem const& item)
{
  bool noFailureFlag;


  wxCHECK_MSG(item.IsOk(),false,"To be deleted item is invalid.");
      m_DataViewCtrlPtr->SetDeleting(true);
  noFailureFlag = m_DataViewCtrlPtr->GetDataViewPeer()->Remove(parent,item);
   m_DataViewCtrlPtr->SetDeleting(false);

  AdjustAutosizedColumns();
   return noFailureFlag;
}

bool wxOSXDataViewModelNotifier::ItemsDeleted(wxDataViewItem const& parent, wxDataViewItemArray const& items)
{
  bool noFailureFlag;


      m_DataViewCtrlPtr->SetDeleting(true);
   noFailureFlag = m_DataViewCtrlPtr->GetDataViewPeer()->Remove(parent,items);
   m_DataViewCtrlPtr->SetDeleting(false);

  AdjustAutosizedColumns();
   return noFailureFlag;
}

bool wxOSXDataViewModelNotifier::ValueChanged(wxDataViewItem const& item, unsigned int col)
{
  wxCHECK_MSG(item.IsOk(),             false,"Passed item is invalid.");
  wxCHECK_MSG(GetOwner() != NULL,false,"Owner not initialized.");
  if (m_DataViewCtrlPtr->GetDataViewPeer()->Update(GetOwner()->GetParent(item),item))
  {
    wxDataViewEvent dataViewEvent(wxEVT_DATAVIEW_ITEM_VALUE_CHANGED,m_DataViewCtrlPtr->GetId());

    dataViewEvent.SetEventObject(m_DataViewCtrlPtr);
    dataViewEvent.SetModel(m_DataViewCtrlPtr->GetModel());
    dataViewEvent.SetColumn(col);
    dataViewEvent.SetItem(item);
       m_DataViewCtrlPtr->HandleWindowEvent(dataViewEvent);

    AdjustAutosizedColumns();
       return true;
  }
  else
    return false;
}

bool wxOSXDataViewModelNotifier::Cleared()
{
  return m_DataViewCtrlPtr->GetDataViewPeer()->Reload();
}

void wxOSXDataViewModelNotifier::Resort()
{
  m_DataViewCtrlPtr->GetDataViewPeer()->Resort();
}

void wxOSXDataViewModelNotifier::AdjustRowHeight(wxDataViewItem const& item)
{
  if ((m_DataViewCtrlPtr->GetWindowStyle() & wxDV_VARIABLE_LINE_HEIGHT) != 0)
  {
      wxDataViewModel *model = GetOwner();

      int height = 20;       unsigned int num = m_DataViewCtrlPtr->GetColumnCount();
      unsigned int col;
      for (col = 0; col < num; col++)
      {
          wxDataViewColumn* column(m_DataViewCtrlPtr->GetColumnPtr(col));

          if (!(column->IsHidden()))
          {
            wxDataViewCustomRenderer *renderer = dynamic_cast<wxDataViewCustomRenderer*>(column->GetRenderer());
            if (renderer)
            {
                wxVariant value;
                model->GetValue( value, item, column->GetModelColumn() );
                renderer->SetValue( value );
                height = wxMax( height, renderer->GetSize().y );
            }
          }
      }
      if (height > 20)
        m_DataViewCtrlPtr->GetDataViewPeer()->SetRowHeight(item,height);
  }
}

void wxOSXDataViewModelNotifier::AdjustRowHeights(wxDataViewItemArray const& items)
{
  if ((m_DataViewCtrlPtr->GetWindowStyle() & wxDV_VARIABLE_LINE_HEIGHT) != 0)
  {
      size_t const noOfItems = items.GetCount();

      wxDataViewModel *model = GetOwner();

      for (size_t itemIndex=0; itemIndex<noOfItems; ++itemIndex)
      {
        int height = 20;         unsigned int num = m_DataViewCtrlPtr->GetColumnCount();
        unsigned int col;

        for (col = 0; col < num; col++)
        {
            wxDataViewColumn* column(m_DataViewCtrlPtr->GetColumnPtr(col));

            if (!(column->IsHidden()))
            {
              wxDataViewCustomRenderer *renderer = dynamic_cast<wxDataViewCustomRenderer*>(column->GetRenderer());
              if (renderer)
              {
                  wxVariant value;
                  model->GetValue( value, items[itemIndex], column->GetModelColumn() );
                  renderer->SetValue( value );
                  height = wxMax( height, renderer->GetSize().y );
              }
            }
        }
        if (height > 20)
          m_DataViewCtrlPtr->GetDataViewPeer()->SetRowHeight(items[itemIndex],height);
      }
  }
}

void wxOSXDataViewModelNotifier::AdjustAutosizedColumns()
{
  unsigned count = m_DataViewCtrlPtr->GetColumnCount();
  for ( unsigned col = 0; col < count; col++ )
  {
      wxDataViewColumn *column = m_DataViewCtrlPtr->GetColumnPtr(col);

      if ( column->GetWidthVariable() == wxCOL_WIDTH_AUTOSIZE )
        m_DataViewCtrlPtr->GetDataViewPeer()->FitColumnWidthToContent(col);
  }
}

wxDataViewCustomRenderer::~wxDataViewCustomRenderer()
{
  delete m_DCPtr;
}

wxDC* wxDataViewCustomRenderer::GetDC()
{
  if ((m_DCPtr == NULL) && (GetOwner() != NULL) && (GetOwner()->GetOwner() != NULL))
    m_DCPtr = new wxClientDC(GetOwner()->GetOwner());
  return m_DCPtr;
}

void wxDataViewCustomRenderer::SetDC(wxDC* newDCPtr)
{
  delete m_DCPtr;
  m_DCPtr = newDCPtr;
}


wxDataViewCtrl::~wxDataViewCtrl()
{
            WX_CLEAR_ARRAY(m_ColumnPtrs);

      if (m_ModelNotifier != NULL)
    m_ModelNotifier->GetOwner()->RemoveNotifier(m_ModelNotifier);
}

void wxDataViewCtrl::Init()
{
  m_CustomRendererPtr = NULL;
  m_Deleting          = false;
  m_cgContext         = NULL;
  m_ModelNotifier     = NULL;
}

bool wxDataViewCtrl::Create(wxWindow *parent,
                            wxWindowID id,
                            const wxPoint& pos,
                            const wxSize& size,
                            long style,
                            const wxValidator& validator,
                            const wxString& name)
{
  DontCreatePeer();
  if (!(wxControl::Create(parent,id,pos,size,style,validator,name)))
    return false;
  SetPeer(::CreateDataView(this,parent,id,pos,size,style,GetExtraStyle()));

  MacPostControlCreate(pos,size);

  return true;
}

bool wxDataViewCtrl::AssociateModel(wxDataViewModel* model)
{
  wxDataViewWidgetImpl* dataViewWidgetPtr(GetDataViewPeer());


  wxCHECK_MSG(dataViewWidgetPtr != NULL,false,"Pointer to native control must not be NULL.");

      if ( m_ModelNotifier )
  {
      m_ModelNotifier->GetOwner()->RemoveNotifier(m_ModelNotifier);
      m_ModelNotifier = NULL;
  }

  if (wxDataViewCtrlBase::AssociateModel(model) && dataViewWidgetPtr->AssociateModel(model))
  {
    if (model != NULL)
    {
      m_ModelNotifier = new wxOSXDataViewModelNotifier(this);
      model->AddNotifier(m_ModelNotifier);
    }
    return true;
  }
  else
    return false;
}

bool wxDataViewCtrl::AppendColumn(wxDataViewColumn* columnPtr)
{
   return wxDataViewCtrl::InsertColumn( GetColumnCount(), columnPtr );
}

bool wxDataViewCtrl::PrependColumn(wxDataViewColumn* columnPtr)
{
   return wxDataViewCtrl::InsertColumn( 0, columnPtr );
}

bool wxDataViewCtrl::InsertColumn(unsigned int pos, wxDataViewColumn* columnPtr)
{
  wxDataViewWidgetImpl* dataViewWidgetPtr(GetDataViewPeer());

   wxCHECK_MSG(dataViewWidgetPtr != NULL,                                         false,"Pointer to native control must not be NULL.");
  wxCHECK_MSG(columnPtr != NULL,                                                 false,"Column pointer must not be NULL.");
  wxCHECK_MSG(columnPtr->GetRenderer() != NULL,                                  false,"Column does not have a renderer.");

   if (wxDataViewCtrlBase::InsertColumn(pos,columnPtr))
  {
    m_ColumnPtrs.Add(columnPtr);
       if (dataViewWidgetPtr->InsertColumn(pos,columnPtr))
    {
                     if (GetColumnCount() == 1)
        SetExpanderColumn(columnPtr);
           return true;
    }
    else
    {
           m_ColumnPtrs.Remove(columnPtr);
      delete columnPtr;
           wxFAIL_MSG("Column could not be added to native control.");
           return false;
    }
  }
  else
  {
       delete columnPtr;
    wxFAIL_MSG("Could not add column to internal structures.");
       return false;
  }
}

bool wxDataViewCtrl::ClearColumns()
{
  if (GetDataViewPeer()->ClearColumns())
  {
    WX_CLEAR_ARRAY(m_ColumnPtrs);
    return true;
  }
  else
    return false;
}

bool wxDataViewCtrl::DeleteColumn(wxDataViewColumn* columnPtr)
{
  if (GetDataViewPeer()->DeleteColumn(columnPtr))
  {
    m_ColumnPtrs.Remove(columnPtr);
    delete columnPtr;
    return true;
  }
  else
    return false;
}

wxDataViewColumn* wxDataViewCtrl::GetColumn(unsigned int pos) const
{
  return GetDataViewPeer()->GetColumn(pos);
}

unsigned int wxDataViewCtrl::GetColumnCount() const
{
  return m_ColumnPtrs.GetCount();
}

int wxDataViewCtrl::GetColumnPosition(wxDataViewColumn const* columnPtr) const
{
  return GetDataViewPeer()->GetColumnPosition(columnPtr);
}

void wxDataViewCtrl::Collapse(wxDataViewItem const& item)
{
  GetDataViewPeer()->Collapse(item);
}

void wxDataViewCtrl::EnsureVisible(wxDataViewItem const& item, wxDataViewColumn const* columnPtr)
{
  if (item.IsOk())
  {
    ExpandAncestors(item);     GetDataViewPeer()->EnsureVisible(item,columnPtr);
  }
}

void wxDataViewCtrl::Expand(wxDataViewItem const& item)
{
  return GetDataViewPeer()->Expand(item);
}

bool wxDataViewCtrl::IsExpanded( const wxDataViewItem & item ) const
{
  return (item.IsOk() && GetDataViewPeer()->IsExpanded(item));
}

wxDataViewColumn* wxDataViewCtrl::GetSortingColumn() const
{
  return GetDataViewPeer()->GetSortingColumn();
}

unsigned int wxDataViewCtrl::GetCount() const
{
  return GetDataViewPeer()->GetCount();
}

wxDataViewItem wxDataViewCtrl::DoGetCurrentItem() const
{
    return GetDataViewPeer()->GetCurrentItem();
}

void wxDataViewCtrl::DoSetCurrentItem(const wxDataViewItem& item)
{
    GetDataViewPeer()->SetCurrentItem(item);
}

wxDataViewColumn *wxDataViewCtrl::GetCurrentColumn() const
{
    return GetDataViewPeer()->GetCurrentColumn();
}

wxRect wxDataViewCtrl::GetItemRect(wxDataViewItem const& item, wxDataViewColumn const* columnPtr) const
{
  if (item.IsOk() && (columnPtr != NULL))
    return GetDataViewPeer()->GetRectangle(item,columnPtr);
  else
    return wxRect();
}

int wxDataViewCtrl::GetSelectedItemsCount() const
{
  return GetDataViewPeer()->GetSelectedItemsCount();
}

int wxDataViewCtrl::GetSelections(wxDataViewItemArray& sel) const
{
  return GetDataViewPeer()->GetSelections(sel);
}

void wxDataViewCtrl::HitTest(wxPoint const& point, wxDataViewItem& item, wxDataViewColumn*& columnPtr) const
{
  return GetDataViewPeer()->HitTest(point,item,columnPtr);
}

bool wxDataViewCtrl::IsSelected(wxDataViewItem const& item) const
{
  return GetDataViewPeer()->IsSelected(item);
}

void wxDataViewCtrl::Select(wxDataViewItem const& item)
{
  if (item.IsOk())
  {
    ExpandAncestors(item);     GetDataViewPeer()->Select(item);
  }
}

void wxDataViewCtrl::SelectAll()
{
  GetDataViewPeer()->SelectAll();
}

void wxDataViewCtrl::SetSelections(wxDataViewItemArray const& sel)
{
    size_t const noOfSelections = sel.GetCount();

    size_t i;

    wxDataViewItem last_parent;


       for (i = 0; i < noOfSelections; i++)
    {
        wxDataViewItem item   = sel[i];
        wxDataViewItem parent = GetModel()->GetParent( item );

        if (parent.IsOk() && (parent != last_parent))
          ExpandAncestors(item);
        last_parent = parent;
    }

       wxDataViewWidgetImpl* dataViewWidgetPtr(GetDataViewPeer()); 
    for (i=0; i<noOfSelections; ++i)
      dataViewWidgetPtr->Select(sel[i]);
}

void wxDataViewCtrl::Unselect(wxDataViewItem const& item)
{
  if (item.IsOk())
    GetDataViewPeer()->Unselect(item);
}

void wxDataViewCtrl::UnselectAll()
{
  GetDataViewPeer()->UnselectAll();
}

wxDataViewWidgetImpl* wxDataViewCtrl::GetDataViewPeer() const
{
  return dynamic_cast<wxDataViewWidgetImpl*>(GetPeer());
}

void wxDataViewCtrl::AddChildren(wxDataViewItem const& parentItem)
{
  int noOfChildren;

  wxDataViewItemArray items;


  wxCHECK_RET(GetModel() != NULL,"Model pointer not initialized.");
  noOfChildren = GetModel()->GetChildren(parentItem,items);
  (void) GetModel()->ItemsAdded(parentItem,items);
}

void wxDataViewCtrl::EditItem(const wxDataViewItem& item, const wxDataViewColumn *column)
{
    GetDataViewPeer()->StartEditor(item, GetColumnPosition(column));
}

void wxDataViewCtrl::FinishCustomItemEditing()
{
  if (GetCustomRendererItem().IsOk())
  {
    GetCustomRendererPtr()->FinishEditing();
    SetCustomRendererItem(wxDataViewItem());
    SetCustomRendererPtr (NULL);
  }
}


wxVisualAttributes
wxDataViewCtrl::GetClassDefaultAttributes(wxWindowVariant WXUNUSED(variant))
{
    wxVisualAttributes attr;

    attr.colFg = wxSystemSettings::GetColour( wxSYS_COLOUR_WINDOWTEXT );
    attr.colBg = wxSystemSettings::GetColour( wxSYS_COLOUR_LISTBOX );
    static wxFont font = wxFont(wxOSX_SYSTEM_FONT_VIEWS);
    attr.font = font;

    return attr;
}

void wxDataViewCtrl::DoSetExpanderColumn()
{
  if (GetExpanderColumn() != NULL)
    GetDataViewPeer()->DoSetExpanderColumn(GetExpanderColumn());
}

void wxDataViewCtrl::DoSetIndent()
{
  GetDataViewPeer()->DoSetIndent(GetIndent());
}

void wxDataViewCtrl::OnSize(wxSizeEvent& event)
{
  unsigned int const noOfColumns = GetColumnCount();


   for (unsigned int i=0; i<noOfColumns; ++i)
  {
    wxDataViewColumn* dataViewColumnPtr(GetColumn(i));

    if (dataViewColumnPtr != NULL)
    {
      wxDataViewCustomRenderer* dataViewCustomRendererPtr(dynamic_cast<wxDataViewCustomRenderer*>(dataViewColumnPtr->GetRenderer()));

      if (dataViewCustomRendererPtr != NULL)
        dataViewCustomRendererPtr->SetDC(NULL);
    }
  }

   GetDataViewPeer()->OnSize();

  event.Skip();
}

wxSize wxDataViewCtrl::DoGetBestSize() const
{
    wxSize best = wxControl::DoGetBestSize();
    best.y = 80;

    return best;
}

void wxDataViewCtrl::OnMouse(wxMouseEvent& event)
{
    event.Skip();
}

wxIMPLEMENT_DYNAMIC_CLASS(wxDataViewCtrl,wxDataViewCtrlBase);

wxBEGIN_EVENT_TABLE(wxDataViewCtrl,wxDataViewCtrlBase)
  EVT_SIZE(wxDataViewCtrl::OnSize)
  EVT_MOTION(wxDataViewCtrl::OnMouse)
wxEND_EVENT_TABLE()

#endif 
