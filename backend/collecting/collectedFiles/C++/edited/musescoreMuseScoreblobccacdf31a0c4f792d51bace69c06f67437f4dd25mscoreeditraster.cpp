
#include "editraster.h"
#include "libmscore/mscore.h"
#include "musescore.h"

namespace Ms {


EditRaster::EditRaster(QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("EditRaster");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      hraster->setValue(MScore::hRaster());
      vraster->setValue(MScore::vRaster());

      MuseScore::restoreGeometry(this);
      }


void EditRaster::accept()
      {
      MScore::setHRaster(hraster->value());
      MScore::setVRaster(vraster->value());
      QDialog::accept();
      }


void EditRaster::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QDialog::hideEvent(event);
      }

}

