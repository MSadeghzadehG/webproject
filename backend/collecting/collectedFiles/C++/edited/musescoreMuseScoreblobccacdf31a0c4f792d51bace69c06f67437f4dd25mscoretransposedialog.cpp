
#include "transposedialog.h"
#include "libmscore/score.h"
#include "musescore.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/key.h"
#include "libmscore/staff.h"
#include "libmscore/harmony.h"
#include "libmscore/part.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/measure.h"
#include "libmscore/undo.h"
#include "libmscore/keysig.h"
#include "libmscore/utils.h"
#include "libmscore/segment.h"
#include "libmscore/stafftype.h"
#include "libmscore/clef.h"

namespace Ms {


TransposeDialog::TransposeDialog(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("TransposeDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      connect(transposeByKey, SIGNAL(clicked(bool)), SLOT(transposeByKeyToggled(bool)));
      connect(transposeByInterval, SIGNAL(clicked(bool)), SLOT(transposeByIntervalToggled(bool)));

      MuseScore::restoreGeometry(this);
      }


void TransposeDialog::transposeByKeyToggled(bool val)
      {
      transposeByInterval->setChecked(!val);
      }


void TransposeDialog::transposeByIntervalToggled(bool val)
      {
      transposeByKey->setChecked(!val);
      }


TransposeMode TransposeDialog::mode() const
      {
      return chromaticBox->isChecked()
                  ? (transposeByKey->isChecked() ? TransposeMode::BY_KEY : TransposeMode::BY_INTERVAL)
                  : TransposeMode::DIATONICALLY;
      }


void TransposeDialog::enableTransposeByKey(bool val)
      {
      transposeByKey->setEnabled(val);
      transposeByInterval->setChecked(!val);
      transposeByKey->setChecked(val);
      }


void TransposeDialog::enableTransposeChordNames(bool val)
      {
      transposeChordNames->setEnabled(val);
      transposeChordNames->setChecked(!val);
      transposeChordNames->setChecked(val);
      }


TransposeDirection TransposeDialog::direction() const
      {
      switch(mode())
      {
      case TransposeMode::BY_KEY:
            if (closestKey->isChecked())
                  return TransposeDirection::CLOSEST;
            return upKey->isChecked() ? TransposeDirection::UP : TransposeDirection::DOWN;
      case TransposeMode::BY_INTERVAL:
            return upInterval->isChecked() ? TransposeDirection::UP : TransposeDirection::DOWN;
      case TransposeMode::DIATONICALLY:
            return upDiatonic->isChecked() ? TransposeDirection::UP : TransposeDirection::DOWN;
      }
      return TransposeDirection::UP;
      }


void TransposeDialog::on_chromaticBox_toggled(bool val)
{
      diatonicBox->setChecked(!val);
}

void TransposeDialog::on_diatonicBox_toggled(bool val)
{
    chromaticBox->setChecked(!val);
}


void TransposeDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }
}

