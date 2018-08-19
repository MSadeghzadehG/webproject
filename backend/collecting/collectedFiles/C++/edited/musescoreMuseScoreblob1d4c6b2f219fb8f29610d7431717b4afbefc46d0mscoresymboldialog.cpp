
#include "symboldialog.h"
#include "palette.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/sym.h"
#include "libmscore/style.h"
#include "libmscore/element.h"
#include "libmscore/symbol.h"
#include "preferences.h"

namespace Ms {

extern MasterScore* gscore;
extern QMap<QString, QStringList>* smuflRanges();


void SymbolDialog::createSymbolPalette()
      {
      sp = new Palette();
      createSymbols();
      }


void SymbolDialog::createSymbols()
      {
      int currentIndex = fontList->currentIndex();
      const ScoreFont* f = &ScoreFont::scoreFonts()[currentIndex];
            ScoreFont::fontFactory(f->name());
      sp->clear();
      for (auto name : (*smuflRanges())[range]) {
            SymId id     = Sym::name2id(name);
            if (search->text().isEmpty()
               || Sym::id2userName(id).contains(search->text(), Qt::CaseInsensitive)) {
                  Symbol* s = new Symbol(gscore);
                  s->setSym(SymId(id), f);
                  sp->append(s, Sym::id2userName(SymId(id)));
                  }
            }
      }


SymbolDialog::SymbolDialog(const QString& s, QWidget* parent)
   : QWidget(parent, Qt::WindowFlags(Qt::Dialog | Qt::Window))
      {
      setupUi(this);
      range = s;              int idx = 0;
      int currentIndex = 0;
      for (const ScoreFont& f : ScoreFont::scoreFonts()) {
            fontList->addItem(f.name());
            if (f.name() == "Bravura")
                  currentIndex = idx;
            ++idx;
            }
      fontList->setCurrentIndex(currentIndex);

      QLayout* l = new QVBoxLayout();
      frame->setLayout(l);
      createSymbolPalette();

      QScrollArea* sa = new PaletteScrollArea(sp);
      l->addWidget(sa);

      sp->setAcceptDrops(false);
      sp->setDrawGrid(true);
      sp->setSelectable(true);

      connect(systemFlag, SIGNAL(stateChanged(int)), SLOT(systemFlagChanged(int)));
      connect(fontList, SIGNAL(currentIndexChanged(int)), SLOT(systemFontChanged(int)));

      sa->setWidget(sp);
      }


void SymbolDialog::systemFlagChanged(int state)
      {
      bool sysFlag = state == Qt::Checked;
      for (int i = 0; i < sp->size(); ++i) {
            Element* e = sp->element(i);
            if (e && e->type() == ElementType::SYMBOL)
                  static_cast<Symbol*>(e)->setSystemFlag(sysFlag);
            }
      }


void SymbolDialog::systemFontChanged(int)
      {
      createSymbols();
      }

void SymbolDialog::on_search_textChanged(const QString &searchPhrase)
      {
      Q_UNUSED(searchPhrase);
      createSymbols();
      }

void SymbolDialog::on_clearSearch_clicked()
      {
      search->clear();
      createSymbols();
      }


void SymbolDialog::changeEvent(QEvent *event)
      {
      QWidget::changeEvent(event);
      if (event->type() == QEvent::LanguageChange)
            retranslate();
      }

}

