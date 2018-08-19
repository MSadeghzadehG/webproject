
#ifndef __SHORTCUT_H__
#define __SHORTCUT_H__



#include "icons.h"
#include "globals.h"

namespace Ms {

class XmlWriter;
class XmlReader;


enum class ShortcutFlags : char {
      NONE        = 0,
      A_SCORE     = 1,
      A_CMD       = 1 << 1,
      A_CHECKABLE = 1 << 2,
      A_CHECKED   = 1 << 3
      };

constexpr ShortcutFlags operator| (ShortcutFlags t1, ShortcutFlags t2) {
      return static_cast<ShortcutFlags>(static_cast<int>(t1) | static_cast<int>(t2));
      }

constexpr bool operator& (ShortcutFlags t1, ShortcutFlags t2) {
      return static_cast<int>(t1) & static_cast<int>(t2);
      }

static const int KEYSEQ_SIZE = 4;


class Shortcut {
      MsWidget _assignedWidget;         int _state { 0 };                 QByteArray _key;                  QByteArray _descr;                QByteArray _text;                 QByteArray _help;                                             
      Icons _icon                            { Icons::Invalid_ICON };
      Qt::ShortcutContext _context           { Qt::WindowShortcut };
      ShortcutFlags _flags                   { ShortcutFlags::NONE };

      QList<QKeySequence> _keys;     
      QKeySequence::StandardKey _standardKey { QKeySequence::UnknownKey };
      mutable QAction* _action               { 0 };             
      static Shortcut _sc[];
      static QHash<QByteArray, Shortcut*> _shortcuts;
      void translateAction(QAction* action) const;

   public:

      Shortcut() {}
      Shortcut(
         Ms::MsWidget assignedWidget,
         int state,
         const char* key,
         const char* d    = 0,
         const char* txt  = 0,
         const char* h    = 0,
         Icons i          = Icons::Invalid_ICON,
         Qt::ShortcutContext cont = Qt::WindowShortcut,
         ShortcutFlags f = ShortcutFlags::NONE
         );

      QAction* action() const;
      const QByteArray& key() const { return _key; }
      QString descr() const;
      QString text() const;
      QString help() const;
      MsWidget assignedWidget() const { return _assignedWidget; }
      void clear();                 void reset();                 void addShortcut(const QKeySequence&);
      int state() const                        { return _state; }
      void setState(int v)                      { _state = v;     }
      bool needsScore() const                  { return _flags & ShortcutFlags::A_SCORE; }
      bool isCmd() const                       { return _flags & ShortcutFlags::A_CMD; }
      bool isCheckable() const                 { return _flags & ShortcutFlags::A_CHECKABLE; }
      bool isChecked() const                   { return _flags & ShortcutFlags::A_CHECKED; }
      Icons icon() const                       { return _icon;  }
      const QList<QKeySequence>& keys() const  { return _keys;  }
      QKeySequence::StandardKey standardKey() const { return _standardKey; }
      void setStandardKey(QKeySequence::StandardKey k);
      void setKeys(const QList<QKeySequence>& ks);

      bool compareKeys(const Shortcut&) const;
      QString keysToString() const;
      static QString getMenuShortcutString(const QMenu* menu);

      void write(Ms::XmlWriter&) const;
      void read(Ms::XmlReader&);

      static void init();
      static void retranslate();
      static void load();
      static void loadFromNewFile(QString fileLocation);
      static void save();
      static void saveToNewFile(QString fileLocation);
      static void resetToDefault();
      static bool dirty;
      static Shortcut* getShortcut(const char* key);
      static const QHash<QByteArray, Shortcut*>& shortcuts() { return _shortcuts; }
      static QActionGroup* getActionGroupForWidget(MsWidget w);
      static QActionGroup* getActionGroupForWidget(MsWidget w, Qt::ShortcutContext newShortcutContext);

      static QString keySeqToString(const QKeySequence& keySeq, QKeySequence::SequenceFormat fmt);
      static QKeySequence keySeqFromString(const QString& str, QKeySequence::SequenceFormat fmt);
      };

} #endif

