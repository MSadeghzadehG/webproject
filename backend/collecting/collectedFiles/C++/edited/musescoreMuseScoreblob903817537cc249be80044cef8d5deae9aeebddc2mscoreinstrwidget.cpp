
#include "config.h"
#include "icons.h"
#include "instrwidget.h"
#include "stringutils.h"

#include "libmscore/clef.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/line.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/staff.h"
#include "libmscore/stafftype.h"
#include "libmscore/style.h"
#include "libmscore/system.h"
#include "libmscore/stringdata.h"
#include "libmscore/undo.h"
#include "libmscore/keysig.h"

namespace Ms {

void filterInstruments(QTreeWidget *instrumentList, const QString &searchPhrase = QString());


void filterInstruments(QTreeWidget* instrumentList, const QString &searchPhrase)
      {
      QTreeWidgetItem* item = 0;

      for (int idx = 0; (item = instrumentList->topLevelItem(idx)); ++idx) {
            int numMatchedChildren = 0;
            QTreeWidgetItem* ci = 0;

            for (int cidx = 0; (ci = item->child(cidx)); ++cidx) {
                                    QString text = ci->text(0).replace(QChar(0x266d), QChar('b'));

                                    QString removedSpecialChar = stringutils::removeLigatures(text);
                  removedSpecialChar = stringutils::removeDiacritics(removedSpecialChar);

                  bool isMatch = text.contains(searchPhrase, Qt::CaseInsensitive) || removedSpecialChar.contains(searchPhrase, Qt::CaseInsensitive);
                  ci->setHidden(!isMatch);

                  if (isMatch)
                        numMatchedChildren++;
                  }

            item->setHidden(numMatchedChildren == 0);
            item->setExpanded(numMatchedChildren > 0 && !searchPhrase.isEmpty());
            }
      }


StaffListItem::StaffListItem(PartListItem* li)
   : QTreeWidgetItem(li, STAFF_LIST_ITEM)
      {
      setPartIdx(0);
      setLinked(false);
      setClefType(ClefTypeList(ClefType::G));
      initStaffTypeCombo();
      }

StaffListItem::StaffListItem()
   : QTreeWidgetItem(STAFF_LIST_ITEM)
      {
      setPartIdx(0);
      setLinked(false);
      setClefType(ClefTypeList(ClefType::G));
      }


void StaffListItem::initStaffTypeCombo(bool forceRecreate)
      {
      if (_staffTypeCombo && !forceRecreate)                return;

                                    
      bool canUseTabs = false;       bool canUsePerc = false;
      PartListItem* part = static_cast<PartListItem*>(QTreeWidgetItem::parent());

                  if (part) {
            const StringData* stringData = part->it ? &(part->it->stringData) :
                        ( (part->part && part->part->instrument()) ? part->part->instrument()->stringData() : 0);
            canUseTabs = stringData && stringData->strings() > 0;
            canUsePerc = part->it ? part->it->useDrumset :
                        ( (part->part && part->part->instrument()) ? part->part->instrument()->useDrumset() : false);
            }
      _staffTypeCombo = new QComboBox();
      _staffTypeCombo->setAutoFillBackground(true);
      int idx = 0;
      for (const StaffType& st : StaffType::presets()) {
            if ( (st.group() == StaffGroup::STANDARD && (!canUsePerc))                            || (st.group() == StaffGroup::PERCUSSION && canUsePerc)
                        || (st.group() == StaffGroup::TAB && canUseTabs)) {
                  _staffTypeCombo->addItem(st.name(), idx);
                  }
            ++idx;
            }
      treeWidget()->setItemWidget(this, 4, _staffTypeCombo);
      connect(_staffTypeCombo, SIGNAL(currentIndexChanged(int)), SLOT(staffTypeChanged(int)) );
      }


void StaffListItem::setPartIdx(int val)
      {
      _partIdx = val;
      setText(0, InstrumentsWidget::tr("Staff %1").arg(_partIdx + 1));
      }


void StaffListItem::setClefType(const ClefTypeList& val)
      {
      _clefType = val;
      setText(2, qApp->translate("clefTable", ClefInfo::name(_clefType._transposingClef)));
      }


void StaffListItem::setLinked(bool val)
      {
      _linked = val;
      setIcon(3, _linked ? *icons[int(Icons::checkmark_ICON)] : QIcon() );
      }


void StaffListItem::setStaffType(const StaffType* st)
      {
      if (!st)                                                    _staffTypeCombo->setCurrentIndex(0);            else {
                        for (int i = 0; i < _staffTypeCombo->count(); ++i) {
                  const StaffType* _st = StaffType::preset(StaffTypes(_staffTypeCombo->itemData(i).toInt()));
                  if (*_st == *st) {
                        _staffTypeCombo->setCurrentIndex(i);
                        return;
                        }
                  }
                        for (int i = 0; i < _staffTypeCombo->count(); ++i) {
                  const StaffType* _st = StaffType::preset(StaffTypes(_staffTypeCombo->itemData(i).toInt()));
                  if (_st->isSameStructure(*st)) {
                        _staffTypeCombo->setCurrentIndex(i);
                        return;
                        }
                  }
            qDebug("StaffListItem::setStaffType: not found\n");
            _staffTypeCombo->setCurrentIndex(0);                  }
      }


void StaffListItem::setStaffType(int idx)
      {
      int i = _staffTypeCombo->findData(idx);
      if (i != -1)
            _staffTypeCombo->setCurrentIndex(i);
      }


const StaffType* StaffListItem::staffType() const
      {
      return StaffType::preset(StaffTypes((staffTypeIdx())));
      }


int StaffListItem::staffTypeIdx() const
      {
      return _staffTypeCombo->itemData(_staffTypeCombo->currentIndex()).toInt();
      }


void StaffListItem::staffTypeChanged(int idx)
      {
            int staffTypeIdx = _staffTypeCombo->itemData(idx).toInt();
      const StaffType* stfType = StaffType::preset(StaffTypes(staffTypeIdx));

      PartListItem* pli = static_cast<PartListItem*>(QTreeWidgetItem::parent());
      pli->updateClefs();

      if (_staff && _staff->staffType(0)->name() != stfType->name()) {
            if (_op != ListItemOp::I_DELETE && _op != ListItemOp::ADD)
                  _op = ListItemOp::UPDATE;
            }
      }


void PartListItem::setVisible(bool val)
      {
      setCheckState(1, val ? Qt::Checked : Qt::Unchecked);
      }


bool PartListItem::visible() const
      {
      return checkState(1) == Qt::Checked;
      }


void PartListItem::updateClefs()
      {
      for (int i = 0; i < childCount(); ++i) {
            StaffListItem* sli = static_cast<StaffListItem*>(child(i));
            const StaffType* stfType = StaffType::preset(StaffTypes(sli->staffTypeIdx()));

            ClefTypeList clefType;
            switch (stfType->group()) {
                  case StaffGroup::STANDARD:
                        clefType = sli->defaultClefType();
                        break;
                  case StaffGroup::TAB:
                        clefType = ClefTypeList(ClefType::TAB);
                        break;
                  case StaffGroup::PERCUSSION:
                        clefType = ClefTypeList(ClefType::PERC);
                        break;
                  }
            sli->setClefType(clefType);
            }
      }


PartListItem::PartListItem(Part* p, QTreeWidget* lv)
   : QTreeWidgetItem(lv, PART_LIST_ITEM)
      {
      part = p;
      it   = 0;
      op   = ListItemOp::KEEP;
      setText(0, p->partName());
      setFlags(flags() | Qt::ItemIsUserCheckable);
      }

PartListItem::PartListItem(const InstrumentTemplate* i, QTreeWidget* lv)
   : QTreeWidgetItem(lv, PART_LIST_ITEM)
      {
      part = 0;
      it   = i;
      op   = ListItemOp::ADD;
      setText(0, it->trackName);
      }


InstrumentTemplateListItem::InstrumentTemplateListItem(QString group, QTreeWidget* parent)
   : QTreeWidgetItem(parent) {
      _instrumentTemplate = 0;
      _group = group;
      setText(0, group);
      }

InstrumentTemplateListItem::InstrumentTemplateListItem(InstrumentTemplate* i, InstrumentTemplateListItem* item)
   : QTreeWidgetItem(item) {
      _instrumentTemplate = i;
      setText(0, i->trackName);
      }

InstrumentTemplateListItem::InstrumentTemplateListItem(InstrumentTemplate* i, QTreeWidget* parent)
   : QTreeWidgetItem(parent) {
      _instrumentTemplate = i;
      setText(0, i->trackName);
      }


QString InstrumentTemplateListItem::text(int col) const
      {
      switch (col) {
            case 0:
                  return _instrumentTemplate ?
                     _instrumentTemplate->trackName : _group;
            default:
                  return QString("");
            }
      }


InstrumentsWidget::InstrumentsWidget(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      splitter->setStretchFactor(0, 10);
      splitter->setStretchFactor(1, 0);
      splitter->setStretchFactor(2, 15);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      instrumentList->setSelectionMode(QAbstractItemView::ExtendedSelection);
      partiturList->setSelectionMode(QAbstractItemView::SingleSelection);
      QStringList header = (QStringList() << tr("Staves") << tr("Visible") << tr("Clef") << tr("Linked") << tr("Staff type"));
      partiturList->setHeaderLabels(header);
      partiturList->resizeColumnToContents(1);        partiturList->resizeColumnToContents(3);

      buildTemplateList();

      addButton->setEnabled(false);
      removeButton->setEnabled(false);
      upButton->setEnabled(false);
      downButton->setEnabled(false);
      belowButton->setEnabled(false);
      linkedButton->setEnabled(false);

      connect(instrumentList, SIGNAL(clicked(const QModelIndex &)), SLOT(expandOrCollapse(const QModelIndex &)));
      }


void populateGenreCombo(QComboBox* combo)
      {
      combo->clear();
      combo->addItem(qApp->translate("InstrumentsDialog", "All instruments"), "all");
      int i = 1;
      int defaultIndex = 0;
      foreach (InstrumentGenre *ig, instrumentGenres) {
            combo->addItem(ig->name, ig->id);
            if (ig->id == "common")
                  defaultIndex = i;
            ++i;
            }
      combo->setCurrentIndex(defaultIndex);
      }

 
void populateInstrumentList(QTreeWidget* instrumentList)
      {
      instrumentList->clear();
            foreach(InstrumentGroup* g, instrumentGroups) {
            InstrumentTemplateListItem* group = new InstrumentTemplateListItem(g->name, instrumentList);
            group->setFlags(Qt::ItemIsEnabled);
            foreach(InstrumentTemplate* t, g->instrumentTemplates) {
                  new InstrumentTemplateListItem(t, group);
                  }
            }
      }


void InstrumentsWidget::buildTemplateList()
      {
            search->clear();

      populateInstrumentList(instrumentList);
      populateGenreCombo(instrumentGenreFilter);
      }


void InstrumentsWidget::expandOrCollapse(const QModelIndex &model)
      {
      if(instrumentList->isExpanded(model))
            instrumentList->collapse(model);
      else
            instrumentList->expand(model);
      }


void InstrumentsWidget::genPartList(Score* cs)
      {
      partiturList->clear();

      foreach (Part* p, cs->parts()) {
            PartListItem* pli = new PartListItem(p, partiturList);
            pli->setVisible(p->show());
            foreach (Staff* s, *p->staves()) {
                  StaffListItem* sli = new StaffListItem(pli);
                  sli->setStaff(s);
                  sli->setClefType(s->clefType(0));
                  sli->setDefaultClefType(s->defaultClefType());
                  sli->setPartIdx(s->rstaff());
                  const LinkedElements* ls = s->links();
                  bool bLinked = false;
                  if (ls && !ls->empty()) {
                        for (auto le : *ls) {
                              Staff* ps = toStaff(le);
                              if (ps != s && ps->score() == s->score()) {
                                    bLinked = true;
                                    break;
                                    }
                              }
                        }
                  sli->setLinked(bLinked);
                  sli->setStaffType(s->staffType(0));                      }
            pli->updateClefs();
            partiturList->setItemExpanded(pli, true);
            }
      partiturList->resizeColumnToContents(2);        partiturList->resizeColumnToContents(4);
      }


void InstrumentsWidget::on_instrumentList_itemSelectionChanged()
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      bool flag = !wi.isEmpty();
      addButton->setEnabled(flag);
      }


void InstrumentsWidget::on_partiturList_itemSelectionChanged()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty()) {
            removeButton->setEnabled(false);
            upButton->setEnabled(false);
            downButton->setEnabled(false);
            linkedButton->setEnabled(false);
            belowButton->setEnabled(false);
            return;
            }
      QTreeWidgetItem* item = wi.front();
      bool flag = item != 0;

      int count = 0;       QTreeWidgetItem* it = 0;
      QList<QTreeWidgetItem*> witems;
      if(item->type() == PART_LIST_ITEM) {
            for (int idx = 0; (it = partiturList->topLevelItem(idx)); ++idx) {
                  if (!it->isHidden()) {
                        count++;
                        witems.append(it);
                        }
                  }
            }
      else {
            for (int idx = 0; (it = item->parent()->child(idx)); ++idx) {
                  if (!it->isHidden()){
                        count++;
                        witems.append(it);
                        }
                  }
            }

      bool onlyOne = (count == 1);
      bool first = (witems.first() == item);
      bool last = (witems.last() == item);

      removeButton->setEnabled(flag && !onlyOne);
      upButton->setEnabled(flag && !onlyOne && !first);
      downButton->setEnabled(flag && !onlyOne && !last);
      linkedButton->setEnabled(item && item->type() == STAFF_LIST_ITEM);
      belowButton->setEnabled(item && item->type() == STAFF_LIST_ITEM);
      }


void InstrumentsWidget::on_instrumentList_itemDoubleClicked(QTreeWidgetItem*, int)
      {
      on_addButton_clicked();
      }


void InstrumentsWidget::on_addButton_clicked()
      {
      foreach(QTreeWidgetItem* i, instrumentList->selectedItems()) {
            InstrumentTemplateListItem* item = static_cast<InstrumentTemplateListItem*>(i);
            const InstrumentTemplate* it     = item->instrumentTemplate();
            if (it == 0)
                  continue;
            PartListItem* pli = new PartListItem(it, partiturList);
            pli->setFirstColumnSpanned(true);
            pli->op = ListItemOp::ADD;

            int n = it->nstaves();
            for (int i = 0; i < n; ++i) {
                  StaffListItem* sli = new StaffListItem(pli);
                  sli->setOp(ListItemOp::ADD);
                  sli->setStaff(0);
                  sli->setPartIdx(i);
                  sli->setDefaultClefType(it->clefType(i));
                  sli->setStaffType(it->staffTypePreset);
                  }
            pli->updateClefs();
            partiturList->setItemExpanded(pli, true);
            partiturList->clearSelection();                 partiturList->setItemSelected(pli, true);
            }
      emit completeChanged(true);
      }


void InstrumentsWidget::on_removeButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item   = wi.front();
      QTreeWidgetItem* parent = item->parent();

      if (parent) {
            if (((StaffListItem*)item)->op() == ListItemOp::ADD) {
                  if (parent->childCount() == 1) {
                        partiturList->takeTopLevelItem(partiturList->indexOfTopLevelItem(parent));
                        delete parent;
                        }
                  else {
                        parent->takeChild(parent->indexOfChild(item));
                        delete item;
                        }
                  }
            else {
                  ((StaffListItem*)item)->setOp(ListItemOp::I_DELETE);
                  item->setHidden(true);

                  
                  int idx = parent->indexOfChild(item);
                  StaffListItem* sli = static_cast<StaffListItem*>(parent->child(idx+1));
                  if (sli) {
                        StaffListItem* sli2 = static_cast<StaffListItem*>(parent->child(idx+2));
                        if (sli->linked() && !(sli2 && sli2->linked())) {
                              sli->setLinked(false);
                              partiturList->update();
                              }
                        }
                  }
            static_cast<PartListItem*>(parent)->updateClefs();
            partiturList->setItemSelected(parent, true);
            }
      else {
            int idx = partiturList->indexOfTopLevelItem(item);
            if (((PartListItem*)item)->op == ListItemOp::ADD)
                  delete item;
            else {
                  ((PartListItem*)item)->op = ListItemOp::I_DELETE;
                  item->setHidden(true);
                  }
                        int plusIdx = 0;
            QTreeWidgetItem* nextParent = partiturList->topLevelItem(idx + plusIdx);
            while (nextParent && nextParent->isHidden()) {
                  plusIdx++;
                  nextParent = partiturList->topLevelItem(idx + plusIdx);
                  }
            if(!nextParent) {                   plusIdx = 1;
                  nextParent = partiturList->topLevelItem(idx - plusIdx);
                  while (nextParent && nextParent->isHidden()) {
                       plusIdx++;
                       nextParent = partiturList->topLevelItem(idx - plusIdx);
                  }
                  }
            partiturList->setItemSelected(nextParent, true);
            }
      }


void InstrumentsWidget::on_upButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();

      if (item->type() == PART_LIST_ITEM) {
            bool isExpanded = partiturList->isItemExpanded(item);
            int idx = partiturList->indexOfTopLevelItem(item);
                        if (idx) {
                  partiturList->selectionModel()->clear();
                  QTreeWidgetItem* item = partiturList->takeTopLevelItem(idx);
                                                      int numOfStaffListItems = item->childCount();
                  int staffIdx[numOfStaffListItems];
                  for (int itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx)
                        staffIdx[itemIdx] = (static_cast<StaffListItem*>(item->child(itemIdx)))->staffTypeIdx();
                                    int minusIdx = 1;
                  QTreeWidgetItem* prevParent = partiturList->topLevelItem(idx - minusIdx);
                  while (prevParent && prevParent->isHidden()) {
                       minusIdx++;
                       prevParent = partiturList->topLevelItem(idx - minusIdx);
                  }
                  partiturList->insertTopLevelItem(idx - minusIdx, item);
                                    for (int itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx) {
                        StaffListItem* staffItem = static_cast<StaffListItem*>(item->child(itemIdx));
                        staffItem->initStaffTypeCombo(true);
                        staffItem->setStaffType(staffIdx[itemIdx]);
                        }
                  partiturList->setItemExpanded(item, isExpanded);
                  partiturList->setItemSelected(item, true);
                  }
            }
      else {
            QTreeWidgetItem* parent = item->parent();
            int idx = parent->indexOfChild(item);
                        if (idx) {
                  partiturList->selectionModel()->clear();
                  StaffListItem* item = static_cast<StaffListItem*>(parent->takeChild(idx));
                                                      int staffTypeIdx = item->staffTypeIdx();
                  parent->insertChild(idx - 1, item);
                                    item->initStaffTypeCombo(true);
                  item->setStaffType(staffTypeIdx);
                  partiturList->setItemSelected(item, true);
                  }
            else {
                                    int parentIdx = partiturList->indexOfTopLevelItem(parent);
                                    if (parentIdx) {
                        partiturList->selectionModel()->clear();
                        QTreeWidgetItem* prevParent = partiturList->topLevelItem(parentIdx - 1);
                        StaffListItem* sli = static_cast<StaffListItem*>(parent->takeChild(idx));
                        int staffTypeIdx = sli->staffTypeIdx();
                        prevParent->addChild(sli);
                        sli->initStaffTypeCombo(true);
                        sli->setStaffType(staffTypeIdx);
                        partiturList->setItemSelected(sli, true);
                                                                                                }
                  }
            }
      }


void InstrumentsWidget::on_downButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return;
      QTreeWidgetItem* item = wi.front();
      if (item->type() == PART_LIST_ITEM) {
            bool isExpanded = partiturList->isItemExpanded(item);
            int idx = partiturList->indexOfTopLevelItem(item);
            int n = partiturList->topLevelItemCount();
                        if (idx < (n-1)) {
                  partiturList->selectionModel()->clear();
                  QTreeWidgetItem* item = partiturList->takeTopLevelItem(idx);
                                                      int numOfStaffListItems = item->childCount();
                  int staffIdx[numOfStaffListItems];
                  int itemIdx;
                  for (itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx)
                        staffIdx[itemIdx] = (static_cast<StaffListItem*>(item->child(itemIdx)))->staffTypeIdx();
                                    int plusIdx = 1;
                  QTreeWidgetItem* nextParent = partiturList->topLevelItem(idx + plusIdx);
                  while (nextParent && nextParent->isHidden()) {
                       plusIdx++;
                       nextParent = partiturList->topLevelItem(idx + plusIdx);
                  }
                  partiturList->insertTopLevelItem(idx + plusIdx, item);
                                    for (itemIdx=0; itemIdx < numOfStaffListItems; ++itemIdx) {
                        StaffListItem* staffItem = static_cast<StaffListItem*>(item->child(itemIdx));
                        staffItem->initStaffTypeCombo(true);
                        staffItem->setStaffType(staffIdx[itemIdx]);
                        }
                  partiturList->setItemExpanded(item, isExpanded);
                  partiturList->setItemSelected(item, true);
                  }
            }
      else {
            QTreeWidgetItem* parent = item->parent();
            int idx = parent->indexOfChild(item);
            int n = parent->childCount();
                        if (idx < (n-1)) {
                  partiturList->selectionModel()->clear();
                  StaffListItem* item = static_cast<StaffListItem*>(parent->takeChild(idx));
                                                      int staffTypeIdx = item->staffTypeIdx();
                  parent->insertChild(idx+1, item);
                                    item->initStaffTypeCombo(true);
                  item->setStaffType(staffTypeIdx);
                  partiturList->setItemSelected(item, true);
                  }
            else {
                                    int parentIdx = partiturList->indexOfTopLevelItem(parent);
                  int n = partiturList->topLevelItemCount();
                                    if (parentIdx < (n-1)) {
                        partiturList->selectionModel()->clear();
                        StaffListItem* sli = static_cast<StaffListItem*>(parent->takeChild(idx));
                        QTreeWidgetItem* nextParent = partiturList->topLevelItem(parentIdx - 1);
                        int staffTypeIdx = sli->staffTypeIdx();
                        nextParent->addChild(sli);
                        sli->initStaffTypeCombo(true);
                        sli->setStaffType(staffTypeIdx);
                        partiturList->setItemSelected(sli, true);
                                                                                                }
                  }
            }
      }


StaffListItem* InstrumentsWidget::on_belowButton_clicked()
      {
      QList<QTreeWidgetItem*> wi = partiturList->selectedItems();
      if (wi.isEmpty())
            return 0;
      QTreeWidgetItem* item = wi.front();
      if (item->type() != STAFF_LIST_ITEM)
            return 0;

      StaffListItem* sli  = static_cast<StaffListItem*>(item);
      PartListItem* pli   = static_cast<PartListItem*>(sli->QTreeWidgetItem::parent());
      StaffListItem* nsli = new StaffListItem();
      nsli->setStaff(0);
            nsli->setOp(ListItemOp::ADD);
      int ridx = pli->indexOfChild(sli) + 1;
      pli->insertChild(ridx, nsli);
      nsli->initStaffTypeCombo();                     nsli->setStaffType(sli->staffType());     
      ClefTypeList clefType;
      if (pli->it)
            clefType = pli->it->clefType(ridx);
      else
            clefType = pli->part->instrument()->clefType(ridx);
      nsli->setDefaultClefType(clefType);
      pli->updateClefs();

      partiturList->clearSelection();                 partiturList->setItemSelected(nsli, true);
      pli->updateClefs();
      return nsli;
      }


void InstrumentsWidget::on_linkedButton_clicked()
      {
      StaffListItem* nsli = on_belowButton_clicked();
      if (nsli)
            nsli->setLinked(true);
      }


void InstrumentsWidget::on_search_textChanged(const QString &searchPhrase)
      {
      instrumentGenreFilter->blockSignals(true);
      instrumentGenreFilter->setCurrentIndex(0);
      instrumentGenreFilter->blockSignals(false);
      filterInstruments(instrumentList, searchPhrase);
      }



void InstrumentsWidget::on_instrumentGenreFilter_currentIndexChanged(int index)
      {
      QString id = instrumentGenreFilter->itemData(index).toString();
            filterInstrumentsByGenre(instrumentList, id);
      }


void InstrumentsWidget::filterInstrumentsByGenre(QTreeWidget *instrumentList, QString genre)
      {
      QTreeWidgetItemIterator iList(instrumentList);
      while (*iList) {
            (*iList)->setHidden(true);
            InstrumentTemplateListItem* itli = static_cast<InstrumentTemplateListItem*>(*iList);
            InstrumentTemplate *it=itli->instrumentTemplate();

            if(it) {
                  if (genre == "all" || it->genreMember(genre)) {
                        (*iList)->setHidden(false);

                        QTreeWidgetItem *iParent = (*iList)->parent();
                        while(iParent) {
                              if(!iParent->isHidden())
                                    break;

                              iParent->setHidden(false);
                              iParent = iParent->parent();
                              }
                        }
                  }
            ++iList;
            }
      }


void InstrumentsWidget::createInstruments(Score* cs)
      {
                        QTreeWidget* pl = partiturList;
      Part* part   = 0;
      int staffIdx = 0;

      QTreeWidgetItem* item = 0;
      for (int idx = 0; (item = pl->topLevelItem(idx)); ++idx) {
            PartListItem* pli = (PartListItem*)item;
            if (pli->op != ListItemOp::ADD) {
                  qDebug("bad op");
                  continue;
                  }
            const InstrumentTemplate* t = ((PartListItem*)item)->it;
            part = new Part(cs);
            part->initFromInstrTemplate(t);

            pli->part = part;
            QTreeWidgetItem* ci = 0;
            int rstaff = 0;
            for (int cidx = 0; (ci = pli->child(cidx)); ++cidx) {
                  if (ci->isHidden())
                        continue;
                  StaffListItem* sli = (StaffListItem*)ci;
                  Staff* staff       = new Staff(cs);
                  staff->setPart(part);
                  sli->setStaff(staff);
                  ++rstaff;

                  staff->init(t, sli->staffType(), cidx);
                  staff->setDefaultClefType(sli->defaultClefType());

                  if (sli->linked() && !part->staves()->isEmpty()) {
                        Staff* linkedStaff = part->staves()->back();
                        staff->linkTo(linkedStaff);
                        }
                  part->staves()->push_back(staff);
                  cs->staves().insert(staffIdx + rstaff, staff);
                  }
                        if (part->staff(0)->barLineSpan() > rstaff) {
                  part->staff(0)->setBracketType(0, BracketType::NO_BRACKET);
                  }

                        cs->insertPart(part, staffIdx);
            int sidx = cs->staffIdx(part);
            int eidx = sidx + part->nstaves();
            for (Measure* m = cs->firstMeasure(); m; m = m->nextMeasure())
                  m->cmdAddStaves(sidx, eidx, true);
            staffIdx += rstaff;
            }
#if 0                         for (int staffIdx = 0; staffIdx < cs->nstaves();) {
            Staff* staff = cs->staff(staffIdx);
            int barLineSpan = staff->barLineSpan();
            if (barLineSpan == 0)
                  staff->setBarLineSpan(1);
            int nstaffIdx = staffIdx + barLineSpan;

            for (int idx = staffIdx+1; idx < nstaffIdx; ++idx) {
                  Staff* tStaff = cs->staff(idx);
                  if (tStaff)
                        tStaff->setBarLineSpan(0);
                  }

            staffIdx = nstaffIdx;
            }
#endif
      cs->setLayoutAll();
      }


void InstrumentsWidget::init()
      {
      partiturList->clear();
      instrumentList->clearSelection();
      addButton->setEnabled(false);
      removeButton->setEnabled(false);
      upButton->setEnabled(false);
      downButton->setEnabled(false);
      linkedButton->setEnabled(false);
      belowButton->setEnabled(false);
      emit completeChanged(false);
      }


QTreeWidget* InstrumentsWidget::getPartiturList()
      {
      return partiturList;
      }
}
