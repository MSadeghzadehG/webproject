
#include "timesigproperties.h"
#include "libmscore/timesig.h"
#include "libmscore/mcursor.h"
#include "libmscore/durationtype.h"
#include "libmscore/score.h"
#include "libmscore/chord.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "exampleview.h"
#include "musescore.h"
#include "icons.h"

namespace Ms {

extern void populateIconPalette(Palette* p, const IconAction* a);


TimeSigProperties::TimeSigProperties(TimeSig* t, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("TimeSigProperties");
      setupUi(this);
      fourfourButton->setIcon(*icons[int(Icons::timesig_common_ICON)]);
      allaBreveButton->setIcon(*icons[int(Icons::timesig_allabreve_ICON)]);

      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      timesig = t;

      zText->setText(timesig->numeratorString());
      nText->setText(timesig->denominatorString());
                  QRegExp rx("[0-9+CO()\\x00A2\\x00D8]*");
      QValidator *validator = new QRegExpValidator(rx, this);
      zText->setValidator(validator);
      nText->setValidator(validator);

      Fraction nominal = timesig->sig() / timesig->stretch();
      nominal.reduce();
      zNominal->setValue(nominal.numerator());
      nNominal->setValue(nominal.denominator());
      Fraction sig(timesig->sig());
      zActual->setValue(sig.numerator());
      nActual->setValue(sig.denominator());
      zNominal->setEnabled(false);
      nNominal->setEnabled(false);

                               zActual->setEnabled(false);
      nActual->setEnabled(false);
      switch (timesig->timeSigType()) {
            case TimeSigType::NORMAL:
                  textButton->setChecked(true);
                  break;
            case TimeSigType::FOUR_FOUR:
                  fourfourButton->setChecked(true);
                  break;
            case TimeSigType::ALLA_BREVE:
                  allaBreveButton->setChecked(true);
                  break;
            }

            struct ProlatioTable {
            SymId id;
            Icons icon;
            };
      static const std::vector<ProlatioTable> prolatioList = {
            { SymId::mensuralProlation1,  Icons::timesig_prolatio01_ICON },              { SymId::mensuralProlation2,  Icons::timesig_prolatio02_ICON },              { SymId::mensuralProlation3,  Icons::timesig_prolatio03_ICON },              { SymId::mensuralProlation4,  Icons::timesig_prolatio04_ICON },              { SymId::mensuralProlation5,  Icons::timesig_prolatio05_ICON },              { SymId::mensuralProlation7,  Icons::timesig_prolatio07_ICON },              { SymId::mensuralProlation8,  Icons::timesig_prolatio08_ICON },              { SymId::mensuralProlation10, Icons::timesig_prolatio10_ICON },              { SymId::mensuralProlation11, Icons::timesig_prolatio11_ICON },              };

      ScoreFont* scoreFont = gscore->scoreFont();
      int idx = 0;
      otherCombo->clear();
      for (ProlatioTable t : prolatioList) {
            const QString& str = scoreFont->toString(t.id);
            if (str.size() > 0) {
                  otherCombo->addItem(*icons[int(t.icon)],"", int(t.id));
                                    if (timesig->timeSigType() == TimeSigType::NORMAL && timesig->denominatorString().isEmpty()
                     && timesig->numeratorString() == str) {
                        textButton->setChecked(false);
                        otherButton->setChecked(true);
                        otherCombo->setCurrentIndex(idx);
                        }
                  }
            idx++;
            }

      Groups g = t->groups();
      if (g.empty())
            g = Groups::endings(timesig->sig());           groups->setSig(timesig->sig(), g);

      MuseScore::restoreGeometry(this);
      }


void TimeSigProperties::accept()
      {
      TimeSigType ts = TimeSigType::NORMAL;
      if (textButton->isChecked())
            ts = TimeSigType::NORMAL;
      else if (fourfourButton->isChecked())
            ts = TimeSigType::FOUR_FOUR;
      else if (allaBreveButton->isChecked())
            ts = TimeSigType::ALLA_BREVE;
      else if (otherButton->isChecked()) {
                        ts = TimeSigType::NORMAL;
            ScoreFont* scoreFont = timesig->score()->scoreFont();
            SymId symId = (SymId)( otherCombo->itemData(otherCombo->currentIndex()).toInt() );
                        timesig->setNumeratorString(scoreFont->toString(symId));
            timesig->setDenominatorString(QString());
            }

      Fraction actual(zActual->value(), nActual->value());
      Fraction nominal(zNominal->value(), nNominal->value());
      timesig->setSig(actual, ts);
      timesig->setStretch(nominal / actual);

      if (zText->text() != timesig->numeratorString())
            timesig->setNumeratorString(zText->text());
      if (nText->text() != timesig->denominatorString())
            timesig->setDenominatorString(nText->text());

      Groups g = groups->groups();
      timesig->setGroups(g);
      QDialog::accept();
      }


void TimeSigProperties::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }
}

