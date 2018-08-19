
#include "editstringdata.h"
#include "editpitch.h"
#include "musescore.h"

namespace Ms {


EditStringData::EditStringData(QWidget *parent, QList<instrString> * strings, int * frets)
   : QDialog(parent)
      {
      setObjectName("EditStringData");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      _strings = strings;
      QStringList hdrLabels;
      int         numOfStrings = _strings->size();
      hdrLabels << tr("Open", "string data") << tr("Pitch", "string data");
      stringList->setHorizontalHeaderLabels(hdrLabels);
      stringList->setRowCount(numOfStrings);
      
      if(numOfStrings > 0) {
            int   i;
            instrString strg;
                                    for(i=0; i < numOfStrings; i++) {
                  strg = (*_strings)[numOfStrings - i - 1];
                  _stringsLoc.append(strg);
                  QTableWidgetItem *newCheck = new QTableWidgetItem();
                  newCheck->setFlags(Qt::ItemFlag(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled));
                  newCheck->setCheckState(strg.open ? Qt::Checked : Qt::Unchecked);
                  stringList->setItem(i, 0, newCheck);
                  QTableWidgetItem *newPitch = new QTableWidgetItem(midiCodeToStr(strg.pitch));
                  stringList->setItem(i, 1, newPitch);
                  }
            stringList->setCurrentCell(0, 1);
            }
            else {
            editString->setEnabled(false);
            deleteString->setEnabled(false);
            }

      _frets = frets;
      numOfFrets->setValue(*_frets);

      connect(deleteString, SIGNAL(clicked()), SLOT(deleteStringClicked()));
      connect(editString,   SIGNAL(clicked()), SLOT(editStringClicked()));
      connect(newString,    SIGNAL(clicked()), SLOT(newStringClicked()));
      connect(stringList,   SIGNAL(doubleClicked(QModelIndex)),     SLOT(editStringClicked()));
      connect(stringList,   SIGNAL(itemClicked(QTableWidgetItem*)), SLOT(listItemClicked(QTableWidgetItem *)));
      _modified = false;

      MuseScore::restoreGeometry(this);
      }

EditStringData::~EditStringData()
{
}


void EditStringData::hideEvent(QHideEvent* ev)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(ev);
      }


void EditStringData::deleteStringClicked()
      {
      int         i = stringList->currentRow();

            _stringsLoc.removeAt(i);
      stringList->model()->removeRow(i);
            if (stringList->rowCount() == 0) {
            editString->setEnabled(false);
            deleteString->setEnabled(false);
            }
      _modified = true;
      }


void EditStringData::editStringClicked()
      {
      int         i = stringList->currentRow();
      int         newCode;

      EditPitch* ep = new EditPitch(this, _stringsLoc[i].pitch);
      if ( (newCode=ep->exec()) != -1) {
                        _stringsLoc[i].pitch = newCode;
            QTableWidgetItem * item = stringList->item(i, 1);
            item->setText(midiCodeToStr(newCode));
            _modified = true;
            }
      }


void EditStringData::listItemClicked(QTableWidgetItem * item)
      {
      int col = item->column();
      if (col != 0)                             return;
      int row = item->row();

            bool open = !_stringsLoc[row].open;
      _stringsLoc[row].open = open;
      stringList->item(row, col)->setCheckState(open ? Qt::Checked : Qt::Unchecked);
      _modified = true;
      }


void EditStringData::newStringClicked()
      {
      int         i, newCode;

      EditPitch* ep = new EditPitch(this);
      if ( (newCode=ep->exec()) != -1) {
                        i = stringList->currentRow() + 1;
            if(i <= 0)
                  i = stringList->rowCount();
                        instrString strg = {newCode, 0};
            _stringsLoc.insert(i, strg);
            stringList->insertRow(i);
            QTableWidgetItem *newCheck = new QTableWidgetItem();
            newCheck->setFlags(Qt::ItemFlag(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled));
            newCheck->setCheckState(strg.open ? Qt::Checked : Qt::Unchecked);
            stringList->setItem(i, 0, newCheck);
            QTableWidgetItem *newPitch = new QTableWidgetItem(midiCodeToStr(strg.pitch));
            stringList->setItem(i, 1, newPitch);
                        stringList->setCurrentCell(i, 1);
            editString->setEnabled(true);
            deleteString->setEnabled(true);
            _modified = true;
            }
      }


void EditStringData::accept()
      {
                  if(_modified) {
            _strings->clear();
            for(int i=_stringsLoc.size()-1; i >= 0; i--)
                  _strings->append(_stringsLoc[i]);
            }
      if(*_frets != numOfFrets->value()) {
            *_frets = numOfFrets->value();
            _modified = true;
            }

      if(_modified)
            QDialog::accept();
      else
            QDialog::reject();                  }


static const char* g_cNoteName[] = {
      QT_TRANSLATE_NOOP("editstringdata", "C"),
      QT_TRANSLATE_NOOP("editstringdata", "C#"),
      QT_TRANSLATE_NOOP("editstringdata", "D"),
      QT_TRANSLATE_NOOP("editstringdata", "Eb"),
      QT_TRANSLATE_NOOP("editstringdata", "E"),
      QT_TRANSLATE_NOOP("editstringdata", "F"),
      QT_TRANSLATE_NOOP("editstringdata", "F#"),
      QT_TRANSLATE_NOOP("editstringdata", "G"),
      QT_TRANSLATE_NOOP("editstringdata", "Ab"),
      QT_TRANSLATE_NOOP("editstringdata", "A"),
      QT_TRANSLATE_NOOP("editstringdata", "Bb"),
      QT_TRANSLATE_NOOP("editstringdata", "B")
      };

QString EditStringData::midiCodeToStr(int midiCode)
      {
      return QString("%1 %2").arg(qApp->translate("editstringdata", g_cNoteName[midiCode % 12])).arg(midiCode / 12 - 1);
      }
}
