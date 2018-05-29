
#include "toolbuttonmenu.h"

namespace Ms {

ToolButtonMenu::ToolButtonMenu(QString str,
                     TYPES type,
                     QAction* defaultAction,
                     QActionGroup* alternativeActions,
                     QWidget* parent) : AccessibleToolButton(parent, defaultAction)
      {
            Q_ASSERT((type == TYPES::ACTION_SWAPPED) == alternativeActions->actions().contains(defaultAction));

      _type = type;
      _alternativeActions = alternativeActions;

      setMenu(new QMenu(str, this));
      menu()->setToolTipsVisible(true);

      setPopupMode(QToolButton::MenuButtonPopup);

      if (_type != TYPES::ACTION_SWAPPED) {
            addAction(defaultAction);
            addSeparator();
            }

      for (QAction* a : _alternativeActions->actions()) {
            a->setCheckable(true);
            a->setIconVisibleInMenu(true);
            }

      _alternativeActions->setExclusive(true);
      addActions(_alternativeActions->actions());

      connect(_alternativeActions, SIGNAL(triggered(QAction*)), this, SLOT(handleAlternativeAction(QAction*)));

      if (_type != TYPES::ACTION_SWAPPED) {
            QAction* a = _alternativeActions->actions().first();
            a->setChecked(true);
            if (_type == TYPES::ICON_CHANGED)
                  changeIcon(a);
            }
      }

void ToolButtonMenu::handleAlternativeAction(QAction* a)
      {
      Q_ASSERT(_alternativeActions->actions().contains(a));

      switch (_type) {
            case TYPES::FIXED:
                  break;
            case TYPES::ICON_CHANGED:
                  changeIcon(a);
                  break;
            case TYPES::ACTION_SWAPPED:
                  setDefaultAction(a);
                  break;
            }

      QAction* def = defaultAction();

      if (!def->isChecked())
            def->trigger();
      }

} 