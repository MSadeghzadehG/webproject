
#include "sectionbreakprop.h"
#include "libmscore/layoutbreak.h"
#include "musescore.h"

namespace Ms {


SectionBreakProperties::SectionBreakProperties(LayoutBreak* lb, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("SectionBreakProperties");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      _pause->setValue(lb->pause());
      _startWithLongNames->setChecked(lb->startWithLongNames());
      _startWithMeasureOne->setChecked(lb->startWithMeasureOne());
      MuseScore::restoreGeometry(this);
      }


double SectionBreakProperties::pause() const
      {
      return _pause->value();
      }


bool SectionBreakProperties::startWithLongNames() const
      {
      return _startWithLongNames->isChecked();
      }


bool SectionBreakProperties::startWithMeasureOne() const
      {
      return _startWithMeasureOne->isChecked();
      }


void SectionBreakProperties::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

}

