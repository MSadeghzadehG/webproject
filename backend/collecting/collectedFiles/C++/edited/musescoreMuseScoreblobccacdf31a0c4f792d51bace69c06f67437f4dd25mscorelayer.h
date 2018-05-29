
#ifndef __LAYER_H__
#define __LAYER_H__

#include "ui_layer.h"

namespace Ms {

class Score;


class LayerManager : public QDialog, public Ui::LayerManager {
      Q_OBJECT

      Score* score;

      virtual void hideEvent(QHideEvent*);

   private slots:
      void createClicked();
      void deleteClicked();
      void addTagClicked();
      void deleteTagClicked();
      virtual void accept();

   public:
      LayerManager(Score*, QWidget* parent = 0);
      };


} #endif

