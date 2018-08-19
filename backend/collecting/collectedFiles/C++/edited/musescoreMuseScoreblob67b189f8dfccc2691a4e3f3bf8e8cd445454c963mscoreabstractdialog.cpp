
#include "abstractdialog.h"

namespace Ms {

AbstractDialog::AbstractDialog(QWidget * parent, Qt::WindowFlags f)
    : QDialog(parent, f)
      {
      }

AbstractDialog::~AbstractDialog()
      {

      }

void AbstractDialog::changeEvent(QEvent *event)
      {
      QDialog::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
      }
}

