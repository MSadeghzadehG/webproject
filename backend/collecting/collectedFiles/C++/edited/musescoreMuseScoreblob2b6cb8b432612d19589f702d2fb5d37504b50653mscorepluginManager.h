
#ifndef __PLUGIN_MANAGER_H__
#define __PLUGIN_MANAGER_H__

#include "ui_pluginManager.h"
#include "shortcut.h"

namespace Ms {



struct PluginDescription {
      QString path;
      QString version;
      QString description;
      bool load;
      Shortcut shortcut;
      QString menuPath;
      };


class PluginManager : public QDialog, public Ui::PluginManager {
      Q_OBJECT

      QMap<QString, Shortcut*> localShortcuts;
      bool shortcutsChanged;
      QList<PluginDescription> _pluginList;

      void readSettings();
      void loadList(bool forceRefresh);

      virtual void closeEvent(QCloseEvent*);
      virtual void accept();

   private slots:
      void definePluginShortcutClicked();
      void clearPluginShortcutClicked();
      void pluginListWidgetItemChanged(QListWidgetItem*, QListWidgetItem*);
      void pluginLoadToggled(QListWidgetItem*);
      void reloadPluginsClicked();

   signals:
      void closed(bool);

   public:
      PluginManager(QWidget* parent = 0);
      void writeSettings();
      void init();

      bool readPluginList();
      void writePluginList();
      void updatePluginList(bool forceRefresh=false);

      int pluginCount() {return _pluginList.size();}
      PluginDescription* getPluginDescription(int idx) {return &_pluginList[idx];}
      };


} #endif

