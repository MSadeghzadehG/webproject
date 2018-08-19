
#include "editstaff.h"

#include "editdrumset.h"
#include "editpitch.h"
#include "editstafftype.h"
#include "editstringdata.h"
#include "icons.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/stringdata.h"
#include "libmscore/text.h"
#include "libmscore/undo.h"
#include "libmscore/utils.h"
#include "musescore.h"
#include "seq.h"
#include "selinstrument.h"

namespace Ms {


EditStaff::EditStaff(Staff* s, int , QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("EditStaff");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setModal(true);

      staff = nullptr;
      setStaff(s);

      MuseScore::restoreGeometry(this);

      connect(buttonBox,            SIGNAL(clicked(QAbstractButton*)), SLOT(bboxClicked(QAbstractButton*)));
      connect(changeInstrument,     SIGNAL(clicked()),            SLOT(showInstrumentDialog()));
      connect(changeStaffType,      SIGNAL(clicked()),            SLOT(showStaffTypeDialog()));
      connect(minPitchASelect,      SIGNAL(clicked()),            SLOT(minPitchAClicked()));
      connect(maxPitchASelect,      SIGNAL(clicked()),            SLOT(maxPitchAClicked()));
      connect(minPitchPSelect,      SIGNAL(clicked()),            SLOT(minPitchPClicked()));
      connect(maxPitchPSelect,      SIGNAL(clicked()),            SLOT(maxPitchPClicked()));
      connect(editStringData,       SIGNAL(clicked()),            SLOT(editStringDataClicked()));
      connect(lines,                SIGNAL(valueChanged(int)),    SLOT(numOfLinesChanged()));
      connect(lineDistance,         SIGNAL(valueChanged(double)), SLOT(lineDistanceChanged()));
      connect(showClef,             SIGNAL(clicked()),            SLOT(showClefChanged()));
      connect(showTimesig,          SIGNAL(clicked()),            SLOT(showTimeSigChanged()));
      connect(showBarlines,         SIGNAL(clicked()),            SLOT(showBarlinesChanged()));

      connect(nextButton,           SIGNAL(clicked()),            SLOT(gotoNextStaff()));
      connect(previousButton,       SIGNAL(clicked()),            SLOT(gotoPreviousStaff()));

      addAction(getAction("help"));        }


void EditStaff::setStaff(Staff* s)
      {
      if (staff != nullptr)
            delete staff;

      orgStaff = s;
      Part* part        = orgStaff->part();
      instrument        = *part->instrument();
      Score* score      = part->score();
      staff             = new Staff(score);
      staff->setSmall(0, orgStaff->small(0));
      staff->setInvisible(orgStaff->invisible());
      staff->setUserDist(orgStaff->userDist());
      staff->setColor(orgStaff->color());
      staff->setStaffType(0, orgStaff->staffType(0));
      staff->setPart(part);
      staff->setCutaway(orgStaff->cutaway());
      staff->setHideWhenEmpty(orgStaff->hideWhenEmpty());
      staff->setShowIfEmpty(orgStaff->showIfEmpty());
      staff->setUserMag(0, orgStaff->userMag(0));
      staff->setHideSystemBarLine(orgStaff->hideSystemBarLine());

            auto i = part->instruments()->upper_bound(0);         if (i == part->instruments()->end())
            _tickEnd = -1;
      else
            _tickEnd = i->first;
#if 1
      _tickStart = -1;
#else
      --i;
      if (i == part->instruments()->begin())
            _tickStart = 0;
      else
            _tickStart = i->first;
#endif

            spinExtraDistance->setValue(s->userDist() / score->spatium());
      invisible->setChecked(staff->invisible());
      small->setChecked(staff->small(0));
      color->setColor(s->color());
      partName->setText(part->partName());
      cutaway->setChecked(staff->cutaway());
      hideMode->setCurrentIndex(int(staff->hideWhenEmpty()));
      showIfEmpty->setChecked(staff->showIfEmpty());
      hideSystemBarLine->setChecked(staff->hideSystemBarLine());
      mag->setValue(staff->userMag(0) * 100.0);
      updateStaffType();
      updateInstrument();
      updateNextPreviousButtons();
      }


void EditStaff::hideEvent(QHideEvent* ev)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(ev);
      }


void EditStaff::updateStaffType()
      {
      StaffType* staffType = staff->staffType(0);
      lines->setValue(staffType->lines());
      lineDistance->setValue(staffType->lineDistance().val());
      showClef->setChecked(staffType->genClef());
      showTimesig->setChecked(staffType->genTimesig());
      showBarlines->setChecked(staffType->showBarlines());
      staffGroupName->setText(qApp->translate("Staff type group name", staffType->groupName()));
      }


void EditStaff::updateInstrument()
      {
      updateInterval(instrument.transpose());

      QList<StaffName>& snl = instrument.shortNames();
      QString df = snl.isEmpty() ? "" : snl[0].name();
      shortName->setPlainText(df);

      QList<StaffName>& lnl = instrument.longNames();
      df = lnl.isEmpty() ? "" : lnl[0].name();

      longName->setPlainText(df);

      if (partName->text() == instrumentName->text())                partName->setText(instrument.trackName());

      instrumentName->setText(instrument.trackName());

      _minPitchA = instrument.minPitchA();
      _maxPitchA = instrument.maxPitchA();
      _minPitchP = instrument.minPitchP();
      _maxPitchP = instrument.maxPitchP();
      minPitchA->setText(midiCodeToStr(_minPitchA));
      maxPitchA->setText(midiCodeToStr(_maxPitchA));
      minPitchP->setText(midiCodeToStr(_minPitchP));
      maxPitchP->setText(midiCodeToStr(_maxPitchP));

            int numStr = instrument.stringData() ? instrument.stringData()->strings() : 0;
      stringDataFrame->setVisible(numStr > 0);
      numOfStrings->setText(QString::number(numStr));
      }


void EditStaff::updateInterval(const Interval& iv)
      {
      int diatonic  = iv.diatonic;
      int chromatic = iv.chromatic;

      int oct = chromatic / 12;
      if (oct < 0)
            oct = -oct;

      bool upFlag = true;
      if (chromatic < 0 || diatonic < 0) {
            upFlag    = false;
            chromatic = -chromatic;
            diatonic  = -diatonic;
            }
      chromatic %= 12;
      diatonic  %= 7;

      int interval = searchInterval(diatonic, chromatic);
      if (interval == -1) {
            qDebug("EditStaff: unknown interval %d %d", diatonic, chromatic);
            interval = 0;
            }
      iList->setCurrentIndex(interval);
      up->setChecked(upFlag);
      down->setChecked(!upFlag);
      octave->setValue(oct);
      }


void EditStaff::updateNextPreviousButtons()
      {
      int staffIdx = orgStaff->idx();

      nextButton->setEnabled(staffIdx < (orgStaff->score()->nstaves() - 1));
      previousButton->setEnabled(staffIdx != 0);
      }


void EditStaff::gotoNextStaff()
      {
      Staff* nextStaff = orgStaff->score()->staff(orgStaff->idx() + 1);
      if (nextStaff)
            {
            setStaff(nextStaff);
            }
      }


void EditStaff::gotoPreviousStaff()
      {
      Staff* prevStaff = orgStaff->score()->staff(orgStaff->idx() - 1);
      if (prevStaff)
            {
            setStaff(prevStaff);
            }
      }


void EditStaff::bboxClicked(QAbstractButton* button)
      {
      QDialogButtonBox::ButtonRole br = buttonBox->buttonRole(button);
      switch(br) {
            case QDialogButtonBox::ApplyRole:
                  apply();
                  break;

            case QDialogButtonBox::AcceptRole:
                  apply();
                  
            case QDialogButtonBox::RejectRole:
                  close();
                  if (staff != nullptr)
                        delete staff;
                  break;

            default:
                  qDebug("EditStaff: unknown button %d", int(br));
                  break;
            }
      }


void EditStaff::apply()
      {
      Score* score  = orgStaff->score();
      Part* part    = orgStaff->part();

      QString sn = shortName->toPlainText();
      QString ln = longName->toPlainText();
      if (!Text::validateText(sn) || !Text::validateText(ln)) {
            QMessageBox msgBox;
            msgBox.setText(tr("The instrument name is invalid."));
            msgBox.exec();
            return;
            }
      shortName->setPlainText(sn);        longName->setPlainText(ln);

      int intervalIdx = iList->currentIndex();
      bool upFlag     = up->isChecked();

      Interval interval  = intervalList[intervalIdx];
      interval.diatonic  += octave->value() * 7;
      interval.chromatic += octave->value() * 12;

      if (!upFlag)
            interval.flip();
      instrument.setTranspose(interval);

      instrument.setMinPitchA(_minPitchA);
      instrument.setMaxPitchA(_maxPitchA);
      instrument.setMinPitchP(_minPitchP);
      instrument.setMaxPitchP(_maxPitchP);

      instrument.setShortName(sn);
      instrument.setLongName(ln);

      bool inv       = invisible->isChecked();
      ClefTypeList clefType = instrument.clefType(orgStaff->rstaff());
      qreal userDist = spinExtraDistance->value();
      bool ifEmpty   = showIfEmpty->isChecked();
      bool hideSystemBL = hideSystemBarLine->isChecked();
      bool cutAway      = cutaway->isChecked();
      Staff::HideMode hideEmpty = Staff::HideMode(hideMode->currentIndex());

      QString newPartName = partName->text().simplified();
      if (!(instrument == *part->instrument()) || part->partName() != newPartName) {
                        Interval v1 = instrument.transpose();
            Interval v2 = part->instrument()->transpose();

            score->undo(new ChangePart(part, new Instrument(instrument), newPartName));
            emit instrumentChanged();

            if (v1 != v2)
                  score->transpositionChanged(part, v2, _tickStart, _tickEnd);
            }
      orgStaff->undoChangeProperty(Pid::MAG, mag->value() / 100.0);
      orgStaff->undoChangeProperty(Pid::COLOR, color->color());
      orgStaff->undoChangeProperty(Pid::SMALL, small->isChecked());

      if (inv != orgStaff->invisible()
         || clefType != orgStaff->defaultClefType()
         || userDist != orgStaff->userDist()
         || cutAway != orgStaff->cutaway()
         || hideEmpty != orgStaff->hideWhenEmpty()
         || ifEmpty != orgStaff->showIfEmpty()
         || hideSystemBL != orgStaff->hideSystemBarLine()
         ) {
            score->undo(new ChangeStaff(orgStaff, inv, clefType, userDist * score->spatium(), hideEmpty, ifEmpty, cutAway, hideSystemBL));
            }

      if ( !(*orgStaff->staffType(0) == *staff->staffType(0)) ) {
                        score->undo(new ChangeStaffType(orgStaff, *staff->staffType(0)));
            }

      score->update();
      score->masterScore()->updateChannel();
      }


void EditStaff::minPitchAClicked()
      {
      int         newCode;
      EditPitch ep(this, instrument.minPitchA());
      ep.setWindowModality(Qt::WindowModal);
      if ( (newCode = ep.exec()) != -1) {
            minPitchA->setText(midiCodeToStr(newCode));
            _minPitchA = newCode;
            }
      }

void EditStaff::maxPitchAClicked()
      {
      int         newCode;
      EditPitch ep(this, instrument.maxPitchA());
      ep.setWindowModality(Qt::WindowModal);
      if ( (newCode = ep.exec()) != -1) {
            maxPitchA->setText(midiCodeToStr(newCode));
            _maxPitchA = newCode;
            }
      }

void EditStaff::minPitchPClicked()
      {
      int         newCode;
      EditPitch ep(this, instrument.minPitchP());
      ep.setWindowModality(Qt::WindowModal);
      if ( (newCode = ep.exec()) != -1) {
            minPitchP->setText(midiCodeToStr(newCode));
            _minPitchP = newCode;
            }
      }

void EditStaff::maxPitchPClicked()
      {
      int         newCode;
      EditPitch ep(this, instrument.maxPitchP());
      ep.setWindowModality(Qt::WindowModal);
      if ( (newCode = ep.exec()) != -1) {
            maxPitchP->setText(midiCodeToStr(newCode));
            _maxPitchP = newCode;
            }
      }


void EditStaff::lineDistanceChanged()
      {
      staff->staffType(0)->setLineDistance(Spatium(lineDistance->value()));
      }

void EditStaff::numOfLinesChanged()
      {
      staff->staffType(0)->setLines(lines->value());
      }

void EditStaff::showClefChanged()
      {
      staff->staffType(0)->setGenClef(showClef->checkState() == Qt::Checked);
      }

void EditStaff::showTimeSigChanged()
      {
      staff->staffType(0)->setGenTimesig(showTimesig->checkState() == Qt::Checked);
      }

void EditStaff::showBarlinesChanged()
      {
      staff->staffType(0)->setShowBarlines(showBarlines->checkState() == Qt::Checked);
      }


void EditStaff::showInstrumentDialog()
      {
      SelectInstrument si(&instrument, this);
      si.setWindowModality(Qt::WindowModal);
      if (si.exec()) {
            instrument = Instrument::fromTemplate(si.instrTemplate());
            updateInstrument();
            }
      }


void EditStaff::editStringDataClicked()
      {
      int                     frets = instrument.stringData()->frets();
      QList<instrString>      stringList = instrument.stringData()->stringList();

      EditStringData* esd = new EditStringData(this, &stringList, &frets);
      esd->setWindowModality(Qt::WindowModal);
      if (esd->exec()) {
            StringData stringData(frets, stringList);

                        if (stringList.size() > 0) {
                                                      int oldHighestStringPitch     = INT16_MIN;
                  int highestStringPitch        = INT16_MIN;
                  int lowestStringPitch         = INT16_MAX;
                  for (const instrString& str : stringList) {
                        if (str.pitch > highestStringPitch) highestStringPitch = str.pitch;
                        if (str.pitch < lowestStringPitch)  lowestStringPitch  = str.pitch;
                        }
                                    for (const instrString& str : instrument.stringData()->stringList())
                        if (str.pitch > oldHighestStringPitch) oldHighestStringPitch = str.pitch;
                                    if (oldHighestStringPitch == INT16_MIN)
                        oldHighestStringPitch = instrument.maxPitchA();

                                    instrument.setMinPitchA(lowestStringPitch);
                  instrument.setMinPitchP(lowestStringPitch);
                                    instrument.setMaxPitchA(instrument.maxPitchA() + highestStringPitch - oldHighestStringPitch);
                  instrument.setMaxPitchP(instrument.maxPitchP() + highestStringPitch - oldHighestStringPitch);
                                    minPitchA->setText(midiCodeToStr(instrument.minPitchA()));
                  maxPitchA->setText(midiCodeToStr(instrument.maxPitchA()));
                  minPitchP->setText(midiCodeToStr(instrument.minPitchP()));
                  maxPitchP->setText(midiCodeToStr(instrument.maxPitchP()));
                                    }

                        instrument.setStringData(stringData);
            numOfStrings->setText(QString::number(stringData.strings()));
            }
      }


static const char* g_cNoteName[] = {
      QT_TRANSLATE_NOOP("editstaff", "C"),
      QT_TRANSLATE_NOOP("editstaff", "C#"),
      QT_TRANSLATE_NOOP("editstaff", "D"),
      QT_TRANSLATE_NOOP("editstaff", "Eb"),
      QT_TRANSLATE_NOOP("editstaff", "E"),
      QT_TRANSLATE_NOOP("editstaff", "F"),
      QT_TRANSLATE_NOOP("editstaff", "F#"),
      QT_TRANSLATE_NOOP("editstaff", "G"),
      QT_TRANSLATE_NOOP("editstaff", "Ab"),
      QT_TRANSLATE_NOOP("editstaff", "A"),
      QT_TRANSLATE_NOOP("editstaff", "Bb"),
      QT_TRANSLATE_NOOP("editstaff", "B")
      };

QString EditStaff::midiCodeToStr(int midiCode)
      {
      return QString("%1 %2").arg(qApp->translate("editstaff", g_cNoteName[midiCode % 12])).arg(midiCode / 12 - 1);
      }


void EditStaff::showStaffTypeDialog()
      {
      EditStaffType editor(this, staff);
      editor.setWindowModality(Qt::WindowModal);
      if (editor.exec()) {
            staff->setStaffType(0, editor.getStaffType());
            updateStaffType();
            }
      }
}

