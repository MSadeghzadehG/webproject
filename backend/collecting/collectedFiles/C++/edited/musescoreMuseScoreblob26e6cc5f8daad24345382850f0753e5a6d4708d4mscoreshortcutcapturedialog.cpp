
#include "shortcutcapturedialog.h"
#include "musescore.h"
#include "shortcut.h"

namespace Ms {


ShortcutCaptureDialog::ShortcutCaptureDialog(Shortcut* _s, QMap<QString, Shortcut*> ls, QWidget* parent)
   : QDialog(parent)
      {
      setObjectName("ShortcutCaptureDialog");
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      localShortcuts = ls;
      s = _s;

      addButton->setEnabled(false);
      replaceButton->setEnabled(false);
      oshrtLabel->setText(s->keysToString());
      oshrtTextLabel->setAccessibleDescription(s->keysToString());
      oshrtLabel->setEnabled(false);
      connect(clearButton, SIGNAL(clicked()), SLOT(clearClicked()));
      connect(addButton, SIGNAL(clicked()), SLOT(addClicked()));
      connect(replaceButton, SIGNAL(clicked()), SLOT(replaceClicked()));
      clearClicked();

      nshrtLabel->installEventFilter(this);
      MuseScore::restoreGeometry(this);
      }


void ShortcutCaptureDialog::addClicked()
      {
      done(1);
      }


void ShortcutCaptureDialog::replaceClicked()
      {
      done(2);
      }


ShortcutCaptureDialog::~ShortcutCaptureDialog()
      {
      nshrtLabel->removeEventFilter(this);
      releaseKeyboard();
      }


bool ShortcutCaptureDialog::eventFilter(QObject* , QEvent* e)
      {
      if (e->type() == QEvent::KeyPress) {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(e);
            if(keyEvent->key() == Qt::Key_Tab || keyEvent->key() == Qt::Key_Backtab){
                  QWidget::keyPressEvent(keyEvent);
                  return true;
                  }
            keyPress(keyEvent);
            return true;
            }
      return false;
      }



void ShortcutCaptureDialog::keyPress(QKeyEvent* e)
      {
      if (key.count() >= 4)
            return;
      int k = e->key();
      if (k == 0 || k == Qt::Key_Shift || k == Qt::Key_Control ||
         k == Qt::Key_Meta || k == Qt::Key_Alt || k == Qt::Key_AltGr
         || k == Qt::Key_CapsLock || k == Qt::Key_NumLock
         || k == Qt::Key_ScrollLock || k == Qt::Key_unknown)
            return;

      k += e->modifiers();
            if ((k & Qt::ShiftModifier) && ((e->key() < 0x41) || (e->key() > 0x5a) || (e->key() >= 0x01000000))) {
            qDebug() << k;
      	k -= Qt::ShiftModifier;
            qDebug() << k;
            }

      switch(key.count()) {
            case 0: key = QKeySequence(k); break;
            case 1: key = QKeySequence(key[0], k); break;
            case 2: key = QKeySequence(key[0], key[1], k); break;
            case 3: key = QKeySequence(key[0], key[1], key[2], k); break;
            default:
                  qDebug("Internal error: bad key count");
                  break;
            }

            bool conflict = false;
      QString msgString;

      for (Shortcut* ss : localShortcuts) {
            if (s == ss)
                  continue;
            if (!(s->state() & ss->state()))                      continue;

            QList<QKeySequence> skeys = QKeySequence::keyBindings(ss->standardKey());

            for (const QKeySequence& ks : skeys) {
                  if (ks == key) {
                        msgString = tr("Shortcut conflicts with %1").arg(ss->descr());
                        conflict = true;
                        break;
                        }
                  }

            for (const QKeySequence& ks : ss->keys()) {
                  if (ks == key) {
                        msgString = tr("Shortcut conflicts with %1").arg(ss->descr());
                        conflict = true;
                        break;
                        }
                  }
            if (conflict)
                  break;
            }

      messageLabel->setText(msgString);

      if (conflict) {
            if (!nshrtLabel->accessibleName().contains(tr("Shortcut conflicts with")))
                  nshrtLabel->setAccessibleName(msgString);
            }
      else {
            if (!nshrtLabel->accessibleName().contains("New shortcut"))
                  nshrtLabel->setAccessibleName(tr("New shortcut"));
            }
      addButton->setEnabled(conflict == false);
      replaceButton->setEnabled(conflict == false);
      QString keyStr = Shortcut::keySeqToString(key, QKeySequence::NativeText);
      nshrtLabel->setText(keyStr);

      QString A = keyStr;
      QString B = Shortcut::keySeqToString(key, QKeySequence::PortableText);
qDebug("capture key 0x%x  modifiers 0x%x virt 0x%x scan 0x%x <%s><%s>",
      k,
      int(e->modifiers()),
      int(e->nativeVirtualKey()),
      int(e->nativeScanCode()),
      qPrintable(A),
      qPrintable(B)
      );
      }


void ShortcutCaptureDialog::clearClicked()
      {
      if (!nshrtLabel->accessibleName().contains("New shortcut"))
            nshrtLabel->setAccessibleName(tr("New shortcut"));

      nshrtLabel->setAccessibleName(tr("New shortcut"));
      messageLabel->setText("");
      addButton->setEnabled(false);
      replaceButton->setEnabled(false);
      nshrtLabel->setText("");
      key = 0;
      nshrtLabel->setFocus();
      }


void ShortcutCaptureDialog::hideEvent(QHideEvent* event)
      {
      MuseScore::saveGeometry(this);
      QWidget::hideEvent(event);
      }

}

