


#include "libmscore/select.h"
#include "selectdialog.h"
#include "libmscore/element.h"
#include "libmscore/system.h"
#include "libmscore/score.h"
#include "libmscore/slur.h"
#include "libmscore/articulation.h"
#include "musescore.h"

namespace Ms {


SelectDialog::SelectDialog(const Element* _e, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("SelectDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      e = _e;
      type->setText(qApp->translate("elementName", e->userName().toUtf8()));

      switch (e->type()) {
            case ElementType::ACCIDENTAL:
                  subtype->setText(qApp->translate("accidental", e->subtypeName().toUtf8()));
                  break;
            case ElementType::SLUR_SEGMENT:
                  subtype->setText(qApp->translate("elementName", e->subtypeName().toUtf8()));
                  break;
            case ElementType::FINGERING:
            case ElementType::STAFF_TEXT:
                  subtype->setText(qApp->translate("TextStyle", e->subtypeName().toUtf8()));
                  break;
            case ElementType::ARTICULATION:                   subtype->setText(toArticulation(e)->userName());
                  break;
                        default: subtype->setText(e->subtypeName());
            }
      sameSubtype->setEnabled(e->subtype() != -1);
      subtype->setEnabled(e->subtype() != -1);
      inSelection->setEnabled(e->score()->selection().isRange());

      MuseScore::restoreGeometry(this);
      }


void SelectDialog::setPattern(ElementPattern* p)
      {
      p->type    = int(e->type());
      p->subtype = int(e->subtype());
      if (e->isSlurSegment())
            p->subtype = int(toSlurSegment(e)->spanner()->type());

      if (sameStaff->isChecked()) {
            p->staffStart = e->staffIdx();
            p->staffEnd = e->staffIdx() + 1;
            }
      else if (inSelection->isChecked()) {
            p->staffStart = e->score()->selection().staffStart();
            p->staffEnd = e->score()->selection().staffEnd();
            }
      else {
            p->staffStart = -1;
            p->staffEnd = -1;
            }

      p->voice   = sameVoice->isChecked() ? e->voice() : -1;
      p->subtypeValid = sameSubtype->isChecked();
      p->system  = 0;
      if (sameSystem->isChecked()) {
            do {
                  if (e->type() == ElementType::SYSTEM) {
                        p->system = static_cast<const System*>(e);
                        break;
                        }
                  e = e->parent();
                  } while (e);
            }
      }


void SelectDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

}

