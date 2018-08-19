
#ifndef __EDITRASTER_H__
#define __EDITRASTER_H__

#include "ui_editraster.h"

namespace Ms {


class EditRaster : public QDialog, public Ui::EditRaster {
      Q_OBJECT

      virtual void hideEvent(QHideEvent*);

   private slots:
      virtual void accept();

   public:
      EditRaster(QWidget* parent = 0);
      };


} #endif

