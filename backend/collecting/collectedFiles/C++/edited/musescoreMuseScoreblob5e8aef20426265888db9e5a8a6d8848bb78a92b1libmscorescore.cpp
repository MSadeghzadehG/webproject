


#include <assert.h>
#include "score.h"
#include "key.h"
#include "sig.h"
#include "clef.h"
#include "tempo.h"
#include "measure.h"
#include "page.h"
#include "undo.h"
#include "system.h"
#include "select.h"
#include "segment.h"
#include "xml.h"
#include "text.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "slur.h"
#include "staff.h"
#include "part.h"
#include "style.h"
#include "tuplet.h"
#include "lyrics.h"
#include "pitchspelling.h"
#include "line.h"
#include "volta.h"
#include "repeat.h"
#include "ottava.h"
#include "barline.h"
#include "box.h"
#include "utils.h"
#include "excerpt.h"
#include "stafftext.h"
#include "repeatlist.h"
#include "keysig.h"
#include "beam.h"
#include "stafftype.h"
#include "tempotext.h"
#include "articulation.h"
#include "revisions.h"
#include "tiemap.h"
#include "layoutbreak.h"
#include "harmony.h"
#include "mscore.h"
#ifdef OMR
#include "omr/omr.h"
#endif
#include "bracket.h"
#include "audio.h"
#include "instrtemplate.h"
#include "cursor.h"
#include "sym.h"
#include "rehearsalmark.h"
#include "breath.h"
#include "instrchange.h"

namespace Ms {

MasterScore* gscore;                 
bool scriptDebug     = false;
bool noSeq           = false;
bool noMidi          = false;
bool midiInputTrace  = false;
bool midiOutputTrace = false;
bool showRubberBand  = true;


MeasureBaseList::MeasureBaseList()
      {
      _first = 0;
      _last  = 0;
      _size  = 0;
      };


void MeasureBaseList::push_back(MeasureBase* e)
      {
      ++_size;
      if (_last) {
            _last->setNext(e);
            e->setPrev(_last);
            e->setNext(0);
            }
      else {
            _first = e;
            e->setPrev(0);
            e->setNext(0);
            }
      _last = e;
      }


void MeasureBaseList::push_front(MeasureBase* e)
      {
      ++_size;
      if (_first) {
            _first->setPrev(e);
            e->setNext(_first);
            e->setPrev(0);
            }
      else {
            _last = e;
            e->setPrev(0);
            e->setNext(0);
            }
      _first = e;
      }


void MeasureBaseList::add(MeasureBase* e)
      {
      MeasureBase* el = e->next();
      if (el == 0) {
            push_back(e);
            return;
            }
      if (el == _first) {
            push_front(e);
            return;
            }
      ++_size;
      e->setPrev(el->prev());
      el->prev()->setNext(e);
      el->setPrev(e);
      }


void MeasureBaseList::remove(MeasureBase* el)
      {
      --_size;
      if (el->prev())
            el->prev()->setNext(el->next());
      else
            _first = el->next();
      if (el->next())
            el->next()->setPrev(el->prev());
      else
            _last = el->prev();
      }


void MeasureBaseList::insert(MeasureBase* fm, MeasureBase* lm)
      {
      ++_size;
      for (MeasureBase* m = fm; m != lm; m = m->next())
            ++_size;
      MeasureBase* pm = fm->prev();
      if (pm)
            pm->setNext(fm);
      else
            _first = fm;
      MeasureBase* nm = lm->next();
      if (nm)
            nm->setPrev(lm);
      else
            _last = lm;
      }


void MeasureBaseList::remove(MeasureBase* fm, MeasureBase* lm)
      {
      --_size;
      for (MeasureBase* m = fm; m != lm; m = m->next())
            --_size;
      MeasureBase* pm = fm->prev();
      MeasureBase* nm = lm->next();
      if (pm)
            pm->setNext(nm);
      else
            _first = nm;
      if (nm)
            nm->setPrev(pm);
      else
            _last = pm;
      }


void MeasureBaseList::change(MeasureBase* ob, MeasureBase* nb)
      {
      nb->setPrev(ob->prev());
      nb->setNext(ob->next());
      if (ob->prev())
            ob->prev()->setNext(nb);
      if (ob->next())
            ob->next()->setPrev(nb);
      if (ob == _last)
            _last = nb;
      if (ob == _first)
            _first = nb;
      if (nb->type() == ElementType::HBOX || nb->type() == ElementType::VBOX
         || nb->type() == ElementType::TBOX || nb->type() == ElementType::FBOX)
            nb->setSystem(ob->system());
      foreach(Element* e, nb->el())
            e->setParent(nb);
      }


Score::Score()
   : ScoreElement(this), _is(this), _selection(this), _selectionFilter(this)
      {
      _masterScore = 0;
      Layer l;
      l.name          = "default";
      l.tags          = 1;
      _layer.append(l);
      _layerTags[0]   = "default";

      _scoreFont = ScoreFont::fontFactory("emmentaler");

      _pos[int(POS::CURRENT)] = 0;
      _pos[int(POS::LEFT)]    = 0;
      _pos[int(POS::RIGHT)]   = 0;
      _fileDivision           = MScore::division;
      _style  = MScore::defaultStyle();
      accInfo = "No selection";
      }

Score::Score(MasterScore* parent)
   : Score{}
      {
      _masterScore = parent;
      if (MScore::defaultStyleForParts())
            _style = *MScore::defaultStyleForParts();
      else {
                        _style = parent->style();

            static const Sid styles[] = {
                  Sid::pageWidth,
                  Sid::pageHeight,
                  Sid::pagePrintableWidth,
                  Sid::pageEvenLeftMargin,
                  Sid::pageOddLeftMargin,
                  Sid::pageEvenTopMargin,
                  Sid::pageEvenBottomMargin,
                  Sid::pageOddTopMargin,
                  Sid::pageOddBottomMargin,
                  Sid::pageTwosided,
                  Sid::spatium
                  };
                        for (auto i : styles)
                  _style.set(i, MScore::defaultStyle().value(i));
                        style().set(Sid::concertPitch, false);
            style().set(Sid::createMultiMeasureRests, true);
            style().set(Sid::dividerLeft, false);
            style().set(Sid::dividerRight, false);
            }
      _synthesizerState = parent->_synthesizerState;
      }

Score::Score(MasterScore* parent, const MStyle& s)
   : Score{parent}
      {
      _style  = s;
      }


Score::~Score()
      {
      foreach(MuseScoreView* v, viewer)
            v->removeScore();
            for (MeasureBase* m = _measures.first(); m;) {
            MeasureBase* nm = m->next();
            delete m;
            m = nm;
            }
      qDeleteAll(_parts);
      qDeleteAll(_staves);
      qDeleteAll(_systems);
      _masterScore = 0;
      }


void Score::addMeasure(MeasureBase* m, MeasureBase* pos)
      {
      m->setNext(pos);
      _measures.add(m);
      }




void Score::fixTicks()
      {
      int tick = 0;
      Measure* fm = firstMeasure();
      if (fm == 0)
            return;

      for (Staff* staff : _staves)
            staff->clearTimeSig();

      Fraction sig(fm->len());
      Fraction nomSig(fm->timesig());

      if (isMaster()) {
            tempomap()->clear();
            sigmap()->clear();
            sigmap()->add(0, SigEvent(fm->len(),  fm->timesig(), 0));
            }

      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() != ElementType::MEASURE) {
                  mb->setTick(tick);
                  continue;
                  }
            Measure* m       = toMeasure(mb);
            int mtick        = m->tick();
            int diff         = tick - mtick;
            int measureTicks = m->ticks();
            m->moveTicks(diff);
            if (m->mmRest())
                  m->mmRest()->moveTicks(diff);

                                                if (isMaster() && m->sectionBreak() && m->pause() != 0.0)
                  setPause(m->tick() + m->ticks(), m->pause());

                                    
            for (Segment* s = m->first(); s; s = s->next()) {
                  if (isMaster() && s->segmentType() == SegmentType::Breath) {
                        qreal length = 0.0;
                        int tick = s->tick();
                                                for (int i = 0, n = ntracks(); i < n; ++i) {
                              Element* e = s->element(i);
                              if (e && e->type() == ElementType::BREATH) {
                                    Breath* b = toBreath(e);
                                    length = qMax(length, b->pause());
                                    }
                              }
                        if (length != 0.0)
                              setPause(tick, length);
                        }
                  else if (s->segmentType() == SegmentType::TimeSig) {
                        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                              TimeSig* ts = toTimeSig(s->element(staffIdx * VOICES));
                              if (ts)
                                    staff(staffIdx)->addTimeSig(ts);
                              }
                        }
                                    else if (isMaster() && (s->segmentType() == SegmentType::ChordRest)) {
                        for (Element* e : s->annotations()) {
                              if (e->type() == ElementType::TEMPO_TEXT) {
                                    TempoText* tt = toTempoText(e);
                                    if (tt->isRelative())
                                          tt->updateRelative();
                                    setTempo(tt->segment(), tt->tempo());
                                    }
                              }
#if 0                         qreal stretch = 0.0;
                        for (unsigned i = 0; i < s->elist().size(); ++i) {
                              Element* e = s->elist().at(i);
                              if (!e)
                                    continue;
                              ChordRest* cr = toChordRest(e);
                              int nn = cr->articulations().size();
                              for (int ii = 0; ii < nn; ++ii)
                                    stretch = qMax(cr->articulations().at(ii)->timeStretch(), stretch);
                              if (stretch != 0.0 && stretch != 1.0) {
                                    qreal otempo = tempomap()->tempo(cr->tick());
                                    qreal ntempo = otempo / stretch;
                                    setTempo(cr->tick(), ntempo);
                                    int etick = cr->tick() + cr->actualTicks() - 1;
                                    auto e = tempomap()->find(etick);
                                    if (e == tempomap()->end())
                                          setTempo(etick, otempo);
                                    break;
                                    }
                              }
#endif
                        }
                  }

                                                
            if (isMaster() && (!sig.identical(m->len()) || !nomSig.identical(m->timesig()))) {
                  sig = m->len();
                  nomSig = m->timesig();
                  sigmap()->add(tick, SigEvent(sig, nomSig,  m->no()));
                  }

            tick += measureTicks;
            }
            if (tempomap()->empty())
            tempomap()->setTempo(0, 2.0);
      }


static bool validSegment(Segment* s, int startTrack, int endTrack)
      {
      for (int track = startTrack; track < endTrack; ++track) {
            if (s->element(track))
                  return true;
            }
      return false;
      }


Measure* Score::pos2measure(const QPointF& p, int* rst, int* pitch, Segment** seg, QPointF* offset) const
      {
      Measure* m = searchMeasure(p);
      if (m == 0)
            return 0;

      System* s = m->system();
      qreal y   = p.y() - s->canvasPos().y();

      int i = 0;
      for (; i < nstaves();) {
            SysStaff* stff = s->staff(i);
            if (!stff->show() || !staff(i)->show()) {
                  ++i;
                  continue;
                  }
            int ni = i;
            for (;;) {
                  ++ni;
                  if (ni == nstaves() || (s->staff(ni)->show() && staff(ni)->show()))
                        break;
                  }

            qreal sy2;
            if (ni != nstaves()) {
                  SysStaff* nstaff = s->staff(ni);
                  qreal s1y2 = stff->bbox().y() + stff->bbox().height();
                  sy2 = s1y2 + (nstaff->bbox().y() - s1y2)/2;
                  }
            else
                  sy2 = s->page()->height() - s->pos().y();               if (y > sy2) {
                  i   = ni;
                  continue;
                  }
            break;
            }

            QPointF pppp = p - m->canvasPos();
      int strack = i * VOICES;
      if (!staff(i))
            return 0;
      int etrack = VOICES + strack;

      SysStaff* sstaff = m->system()->staff(i);
      SegmentType st = SegmentType::ChordRest;
      for (Segment* segment = m->first(st); segment; segment = segment->next(st)) {
            if (!validSegment(segment, strack, etrack))
                  continue;
            Segment* ns = segment->next(st);
            for (; ns; ns = ns->next(st)) {
                  if (validSegment(ns, strack, etrack))
                        break;
                  }
            if (!ns || (pppp.x() < (segment->x() + (ns->x() - segment->x())/2.0))) {
                  *rst = i;
                  if (pitch) {
                        Staff* s = _staves[i];
                        int tick = segment->tick();
                        ClefType clef = s->clef(tick);
                        *pitch = y2pitch(pppp.y() - sstaff->bbox().y(), clef, s->spatium(tick));
                        }
                  if (offset)
                        *offset = pppp - QPointF(segment->x(), sstaff->bbox().y());
                  if (seg)
                        *seg = segment;
                  return m;
                  }
            }
      return 0;
      }


void Score::dragPosition(const QPointF& p, int* rst, Segment** seg) const
      {
      Measure* m = searchMeasure(p);
      if (m == 0)
            return;

      System* s = m->system();
      qreal y   = p.y() - s->canvasPos().y();

      int i;
      for (i = 0; i < nstaves();) {
            SysStaff* stff = s->staff(i);
            if (!stff->show() || !staff(i)->show()) {
                  ++i;
                  continue;
                  }
            int ni = i;
            for (;;) {
                  ++ni;
                  if (ni == nstaves() || (s->staff(ni)->show() && staff(ni)->show()))
                        break;
                  }

            qreal sy2;
            if (ni != nstaves()) {
                  SysStaff* nstaff = s->staff(ni);
                  qreal s1y2       = stff->bbox().y() + stff->bbox().height();
                  if (i == *rst)
                        sy2 = s1y2 + (nstaff->bbox().y() - s1y2);
                  else if (ni == *rst)
                        sy2 = s1y2;
                  else
                        sy2 = s1y2 + (nstaff->bbox().y() - s1y2) * .5;
                  }
            else
                  sy2 = s->page()->height() - s->pos().y();
            if (y > sy2) {
                  i   = ni;
                  continue;
                  }
            break;
            }

            QPointF pppp = p - m->canvasPos();
      int strack   = i * VOICES;
      if (!staff(i))
            return;
      int etrack = staff(i)->part()->nstaves() * VOICES + strack;

      SegmentType st = SegmentType::ChordRest;
      for (Segment* segment = m->first(st); segment; segment = segment->next(st)) {
            if (!validSegment(segment, strack, etrack))
                  continue;
            Segment* ns = segment->next(st);
            for (; ns; ns = ns->next(st)) {
                  if (validSegment(ns, strack, etrack))
                        break;
                  }
            if (!ns) {
                  *rst = i;
                  *seg = segment;
                  return;
                  }
            if (*seg == segment) {
                  if (pppp.x() < (segment->x() + (ns->x() - segment->x()))) {
                        *rst = i;
                        *seg = segment;
                        return;
                        }
                  }
            else if (*seg == ns) {
                  if (pppp.x() <= segment->x()) {
                        *rst = i;
                        *seg = segment;
                        return;
                        }
                  }
            else {
                  if (pppp.x() < (segment->x() + (ns->x() - segment->x())/2.0)) {
                        *rst = i;
                        *seg = segment;
                        return;
                        }
                  }
            }
      return;
      }


void Score::setShowInvisible(bool v)
      {
      _showInvisible = v;
      setUpdateAll();
      }


void Score::setShowUnprintable(bool v)
      {
      _showUnprintable = v;
      setUpdateAll();
      }


void Score::setShowFrames(bool v)
      {
      _showFrames = v;
      setUpdateAll();
      }


void Score::setShowPageborders(bool v)
      {
      _showPageborders = v;
      setUpdateAll();
      }


void Score::setMarkIrregularMeasures(bool v)
      {
      _markIrregularMeasures = v;
      setUpdateAll();
      }


bool Score::dirty() const
      {
      return !undoStack()->isClean();
      }


void Score::spell()
      {
      for (int i = 0; i < nstaves(); ++i) {
            std::vector<Note*> notes;
            for (Segment* s = firstSegment(SegmentType::All); s; s = s->next1()) {
                  int strack = i * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        Element* e = s->element(track);
                        if (e && e->type() == ElementType::CHORD)
                              notes.insert(notes.end(),
                                 toChord(e)->notes().begin(),
                                 toChord(e)->notes().end());
                        }
                  }
            spellNotelist(notes);
            }
      }

void Score::spell(int startStaff, int endStaff, Segment* startSegment, Segment* endSegment)
      {
      for (int i = startStaff; i < endStaff; ++i) {
            std::vector<Note*> notes;
            for (Segment* s = startSegment; s && s != endSegment; s = s->next()) {
                  int strack = i * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        Element* e = s->element(track);
                        if (e && e->type() == ElementType::CHORD)
                              notes.insert(notes.end(),
                                 toChord(e)->notes().begin(),
                                 toChord(e)->notes().end());
                        }
                  }
            spellNotelist(notes);
            }
      }


Note* prevNote(Note* n)
      {
      Chord* chord = n->chord();
      Segment* seg = chord->segment();
      const std::vector<Note*> nl = chord->notes();
      auto i = std::find(nl.begin(), nl.end(), n);
      if (i != nl.begin())
            return *(i-1);
      int staff      = n->staffIdx();
      int startTrack = staff * VOICES + n->voice() - 1;
      int endTrack   = 0;
      while (seg) {
            if (seg->segmentType() == SegmentType::ChordRest) {
                  for (int track = startTrack; track >= endTrack; --track) {
                        Element* e = seg->element(track);
                        if (e && e->type() == ElementType::CHORD)
                              return toChord(e)->upNote();
                        }
                  }
            seg = seg->prev1();
            startTrack = staff * VOICES + VOICES - 1;
            }
      return n;
      }


static Note* nextNote(Note* n)
      {
      Chord* chord = n->chord();
      const std::vector<Note*> nl = chord->notes();
      auto i = std::find(nl.begin(), nl.end(), n);
      if (i != nl.end()) {
            ++i;
            if (i != nl.end())
                  return *i;
            }
      Segment* seg   = chord->segment();
      int staff      = n->staffIdx();
      int startTrack = staff * VOICES + n->voice() + 1;
      int endTrack   = staff * VOICES + VOICES;
      while (seg) {
            if (seg->segmentType() == SegmentType::ChordRest) {
                  for (int track = startTrack; track < endTrack; ++track) {
                        Element* e = seg->element(track);
                        if (e && e->type() == ElementType::CHORD) {
                              return ((Chord*)e)->downNote();
                              }
                        }
                  }
            seg = seg->next1();
            startTrack = staff * VOICES;
            }
      return n;
      }


void Score::spell(Note* note)
      {
      std::vector<Note*> notes;

      notes.push_back(note);
      Note* nn = nextNote(note);
      notes.push_back(nn);
      nn = nextNote(nn);
      notes.push_back(nn);
      nn = nextNote(nn);
      notes.push_back(nn);

      nn = prevNote(note);
      notes.insert(notes.begin(), nn);
      nn = prevNote(nn);
      notes.insert(notes.begin(), nn);
      nn = prevNote(nn);
      notes.insert(notes.begin(), nn);

      int opt = Ms::computeWindow(notes, 0, 7);
      note->setTpc(Ms::tpc(3, note->pitch(), opt));
      }


void Score::appendPart(Part* p)
      {
      _parts.append(p);
      }


Page* Score::searchPage(const QPointF& p) const
      {
      for (Page* page : pages()) {
            if (page->bbox().translated(page->pos()).contains(p))
                  return page;
            }
      return 0;
      }


QList<System*> Score::searchSystem(const QPointF& pos) const
      {
      QList<System*> systems;
      Page* page = searchPage(pos);
      if (page == 0)
            return systems;
      qreal y = pos.y() - page->pos().y();        const QList<System*>* sl = &page->systems();
      qreal y2;
      int n = sl->size();
      for (int i = 0; i < n; ++i) {
            System* s = sl->at(i);
            System* ns = 0;                           int ii = i + 1;
            for (; ii < n; ++ii) {
                  ns = sl->at(ii);
                  if (ns->y() != s->y())
                        break;
                  }
            if ((ii == n) || (ns == 0))
                  y2 = page->height();
            else {
                  qreal sy2 = s->y() + s->bbox().height();
                  y2         = sy2 + (ns->y() - sy2) * .5;
                  }
            if (y < y2) {
                  systems.append(s);
                  for (int ii = i+1; ii < n; ++ii) {
                        if (sl->at(ii)->y() != s->y())
                              break;
                        systems.append(sl->at(ii));
                        }
                  return systems;
                  }
            }
      return systems;
      }


Measure* Score::searchMeasure(const QPointF& p) const
      {
      QList<System*> systems = searchSystem(p);
      for (System* system : systems) {
            qreal x = p.x() - system->canvasPos().x();
            for (MeasureBase* mb : system->measures()) {
                  if (mb->isMeasure() && (x < (mb->x() + mb->bbox().width())))
                        return toMeasure(mb);
                  }
            }
      return 0;
      }


static Segment* getNextValidInputSegment(Segment* s, int track, int voice)
      {
      if (s == 0)
            return 0;
      Q_ASSERT(s->segmentType() == SegmentType::ChordRest);
            ChordRest* cr1;
      for (Segment* s1 = s; s1; s1 = s1->prev(SegmentType::ChordRest)) {
            cr1 = toChordRest(s1->element(track + voice));
            if (cr1)
                  break;
            }
      int nextTick = (cr1 == 0) ? s->measure()->tick() : cr1->tick() + cr1->actualTicks();

      static const SegmentType st { SegmentType::ChordRest };
      while (s) {
            if (s->element(track + voice))
                  break;
            if (voice && s->tick() == nextTick)
                  return s;
#if 0
            int v;
            for (v = 0; v < VOICES; ++v) {
                  if (s->element(track + v))
                        break;
                  }
            if ((v != VOICES) && voice) {
                  int ntick;
                  bool skipChord = false;
                  bool ns        = false;
                  for (Segment* s1 = s->measure()->first(st); s1; s1 = s1->next(st)) {
                        ChordRest* cr = toChordRest(s1->element(track + voice));
                        if (cr) {
                              if (ns)
                                    return s1;
                              ntick = s1->tick() + cr->actualTicks();
                              skipChord = true;
                              }
                        if (s1 == s)
                              ns = true;
                        if (skipChord) {
                              if (s->tick() >= ntick)
                                    skipChord = false;
                              }
                        if (!skipChord && ns)
                              return s1;
                        }
                  if (!skipChord)
                        return s;
                  }
#endif
            s = s->next(st);
            }
      return s;
      }


bool Score::getPosition(Position* pos, const QPointF& p, int voice) const
      {
      Measure* measure = searchMeasure(p);
      if (measure == 0)
            return false;

      pos->fret = FRET_NONE;
                        pos->staffIdx      = 0;
      SysStaff* sstaff   = 0;
      System* system     = measure->system();
      qreal y           = p.y() - system->pagePos().y();
      for (; pos->staffIdx < nstaves(); ++pos->staffIdx) {
            Staff* st = staff(pos->staffIdx);
            if (!st->part()->show())
                  continue;
            qreal sy2;
            SysStaff* ss = system->staff(pos->staffIdx);
            if (!ss->show())
                  continue;
            SysStaff* nstaff = 0;

                        for (int i = pos->staffIdx + 1; i < nstaves(); ++i) {
                  Staff* st = staff(i);
                  if (!st->part()->show())
                        continue;
                  nstaff = system->staff(i);
                  if (!nstaff->show()) {
                        nstaff = 0;
                        continue;
                        }
                  break;
                  }

            if (nstaff) {
                  qreal s1y2 = ss->bbox().bottom();
                  sy2        = system->page()->canvasPos().y() + s1y2 + (nstaff->bbox().y() - s1y2) * .5;
                  }
            else
                  sy2 = system->page()->canvasPos().y() + system->page()->height() - system->pagePos().y();               if (y < sy2) {
                  sstaff = ss;
                  break;
                  }
            }
      if (sstaff == 0)
            return false;

                        QPointF pppp(p - measure->canvasPos());
      qreal x         = pppp.x();
      Segment* segment = 0;
      pos->segment     = 0;

            int track = pos->staffIdx * VOICES;

      for (segment = measure->first(SegmentType::ChordRest); segment;) {
            segment = getNextValidInputSegment(segment, track, voice);
            if (segment == 0)
                  break;
            Segment* ns = getNextValidInputSegment(segment->next(SegmentType::ChordRest), track, voice);

            qreal x1 = segment->x();
            qreal x2;
            qreal d;
            if (ns) {
                  x2    = ns->x();
                  d     = x2 - x1;
                  }
            else {
                  x2    = measure->bbox().width();
                  d     = (x2 - x1) * 2.0;
                  x     = x1;
                  pos->segment = segment;
                  break;
                  }

            if (x < (x1 + d * .5)) {
                  x = x1;
                  pos->segment = segment;
                  break;
                  }
            segment = ns;
            }
      if (segment == 0)
            return false;
                        Staff* s    = staff(pos->staffIdx);
      qreal mag   = s->mag(segment->tick());
      int tick    = segment->tick();
            qreal lineDist = s->staffType(tick)->lineDistance().val() * (s->isTabStaff(measure->tick()) ? 1 : .5) * mag * spatium();

      pos->line  = lrint((pppp.y() - sstaff->bbox().y()) / lineDist);
      if (s->isTabStaff(measure->tick())) {
            if (pos->line < -1 || pos->line > s->lines(tick)+1)
                  return false;
            if (pos->line < 0)
                  pos->line = 0;
            else if (pos->line >= s->lines(tick))
                  pos->line = s->lines(tick) - 1;
            }
      else {
            int minLine   = absStep(0);
            ClefType clef = s->clef(pos->segment->tick());
            minLine       = relStep(minLine, clef);
            int maxLine   = absStep(127);
            maxLine       = relStep(maxLine, clef);

            if (pos->line > minLine || pos->line < maxLine)
                  return false;
            }

      y         = sstaff->y() + pos->line * lineDist;
      pos->pos  = QPointF(x, y) + measure->canvasPos();
      return true;
      }


bool Score::checkHasMeasures() const
      {
      Page* page = pages().front();
      const QList<System*>* sl = &page->systems();
      if (sl == 0 || sl->empty() || sl->front()->measures().empty()) {
            qDebug("first create measure, then repeat operation");
            return false;
            }
      return true;
      }

#if 0

void Score::moveBracket(int staffIdx, int srcCol, int dstCol)
      {
      for (System* system : systems())
            system->moveBracket(staffIdx, srcCol, dstCol);
      }
#endif


static void spatiumHasChanged(void* data, Element* e)
      {
      qreal* val = (qreal*)data;
      e->spatiumChanged(val[0], val[1]);
      }


void Score::spatiumChanged(qreal oldValue, qreal newValue)
      {
      qreal data[2];
      data[0] = oldValue;
      data[1] = newValue;
      scanElements(data, spatiumHasChanged, true);
      for (Staff* staff : _staves)
            staff->spatiumChanged(oldValue, newValue);
      _noteHeadWidth = _scoreFont->width(SymId::noteheadBlack, newValue / SPATIUM20);
      }


static void updateStyle(void*, Element* e)
      {
      e->styleChanged();
      }


void Score::styleChanged()
      {
      scanElements(0, updateStyle);
      if (headerText())
            headerText()->styleChanged();
      if (footerText())
            footerText()->styleChanged();
      setLayoutAll();
      }


Measure* Score::getCreateMeasure(int tick)
      {
      Measure* last = lastMeasure();
      if (last == 0 || ((last->tick() + last->ticks()) <= tick)) {
            int lastTick  = last ? (last->tick()+last->ticks()) : 0;
            while (tick >= lastTick) {
                  Measure* m = new Measure(this);
                  Fraction ts = sigmap()->timesig(lastTick).timesig();
                  m->setTick(lastTick);
                  m->setTimesig(ts);
                  m->setLen(ts);
                  measures()->add(toMeasureBase(m));
                  lastTick += ts.ticks();
                  }
            }
      return tick2measure(tick);
      }




void Score::addElement(Element* element)
      {
      Element* parent = element->parent();
      element->triggerLayout();


      ElementType et = element->type();
      if (et == ElementType::MEASURE
         || (et == ElementType::HBOX && element->parent()->type() != ElementType::VBOX)
         || et == ElementType::VBOX
         || et == ElementType::TBOX
         || et == ElementType::FBOX
         ) {
            measures()->add(toMeasureBase(element));
            return;
            }

      if (parent)
            parent->add(element);

      switch (et) {
            case ElementType::BEAM:
                  {
                  Beam* b = toBeam(element);
                  int n = b->elements().size();
                  for (int i = 0; i < n; ++i)
                        b->elements().at(i)->setBeam(b);
                  }
                  break;

            case ElementType::SLUR:
                  addLayoutFlags(LayoutFlag::PLAY_EVENTS);
                  
            case ElementType::VOLTA:
            case ElementType::TRILL:
            case ElementType::VIBRATO:
            case ElementType::PEDAL:
            case ElementType::TEXTLINE:
            case ElementType::HAIRPIN:
            case ElementType::LET_RING:
            case ElementType::PALM_MUTE:
                  {
                  Spanner* spanner = toSpanner(element);
                  if (et == ElementType::TEXTLINE && spanner->anchor() == Spanner::Anchor::NOTE)
                        break;
                  addSpanner(spanner);
                  for (SpannerSegment* ss : spanner->spannerSegments()) {
                        if (ss->system())
                              ss->system()->add(ss);
                        }
                  }
                  break;

            case ElementType::OTTAVA:
                  {
                  Ottava* o = toOttava(element);
                  addSpanner(o);
                  foreach(SpannerSegment* ss, o->spannerSegments()) {
                        if (ss->system())
                              ss->system()->add(ss);
                        }
                  cmdState().layoutFlags |= LayoutFlag::FIX_PITCH_VELO;
                  o->staff()->updateOttava();
                  _playlistDirty = true;
                  }
                  break;

            case ElementType::DYNAMIC:
                  cmdState().layoutFlags |= LayoutFlag::FIX_PITCH_VELO;
                  _playlistDirty = true;
                  break;

            case ElementType::TEMPO_TEXT:
                  {
                  TempoText* tt = toTempoText(element);
                  if (tt->isRelative())
                        tt->updateRelative();
                  setTempo(tt->segment(), tt->tempo());
                  }
                  break;

            case ElementType::INSTRUMENT_CHANGE: {
                  InstrumentChange* ic = toInstrumentChange(element);
                  int tickStart = ic->segment()->tick();
                  auto i = ic->part()->instruments()->upper_bound(tickStart);
                  int tickEnd;
                  if (i == ic->part()->instruments()->end())
                        tickEnd = -1;
                  else
                        tickEnd = i->first;
                  Interval oldV = ic->part()->instrument(tickStart)->transpose();
                  ic->part()->setInstrument(ic->instrument(), tickStart);
                  transpositionChanged(ic->part(), oldV, tickStart, tickEnd);
                  masterScore()->rebuildMidiMapping();
                  cmdState()._instrumentsChanged = true;
                  }
                  break;

            case ElementType::CHORD:
                  setPlaylistDirty();
                                                      break;

            case ElementType::NOTE:
            case ElementType::TREMOLO:
            case ElementType::ARTICULATION:
            case ElementType::ARPEGGIO:
                  {
                  Element* cr = parent;
                  if (cr->isChord())
                        createPlayEvents(toChord(cr));
                  }
                  break;

            default:
                  break;
            }
      setLayout(element->tick());
      }


void Score::removeElement(Element* element)
      {
      Element* parent = element->parent();
      setLayout(element->tick());


            
      ElementType et = element->type();

      if (et == ElementType::MEASURE
         || (et == ElementType::HBOX && !parent->isVBox())
         || et == ElementType::VBOX
         || et == ElementType::TBOX
         || et == ElementType::FBOX
            ) {
            MeasureBase* mb = toMeasureBase(element);
            measures()->remove(mb);
            System* system = mb->system();
            Page* page = system->page();
            if (element->isVBox() && system->measures().size() == 1) {
                  auto i = std::find(page->systems().begin(), page->systems().end(), system);
                  page->systems().erase(i);
                  }
            return;
            }

      if (et == ElementType::BEAM) {                      element->setParent(0);
            parent = 0;
            }

      if (parent)
            parent->remove(element);

      switch (et) {
            case ElementType::BEAM:
                  for (ChordRest* cr : toBeam(element)->elements())
                        cr->setBeam(0);
                  break;

            case ElementType::SLUR:
                  addLayoutFlags(LayoutFlag::PLAY_EVENTS);
                  
            case ElementType::VOLTA:
            case ElementType::TRILL:
            case ElementType::VIBRATO:
            case ElementType::PEDAL:
            case ElementType::LET_RING:
            case ElementType::PALM_MUTE:
            case ElementType::TEXTLINE:
            case ElementType::HAIRPIN:
                  {
                  Spanner* spanner = toSpanner(element);
                  if (et == ElementType::TEXTLINE && spanner->anchor() == Spanner::Anchor::NOTE)
                        break;
                  removeSpanner(spanner);
                  }
                  break;

            case ElementType::OTTAVA:
                  {
                  Ottava* o = toOttava(element);
                  removeSpanner(o);
                  foreach(SpannerSegment* ss, o->spannerSegments()) {
                        if (ss->system())
                              ss->system()->remove(ss);
                        }
                  o->staff()->updateOttava();
                  cmdState().layoutFlags |= LayoutFlag::FIX_PITCH_VELO;
                  _playlistDirty = true;
                  }
                  break;

            case ElementType::DYNAMIC:
                  cmdState().layoutFlags |= LayoutFlag::FIX_PITCH_VELO;
                  _playlistDirty = true;
                  break;

            case ElementType::CHORD:
            case ElementType::REST:
                  {
                  ChordRest* cr = toChordRest(element);
                  if (cr->beam())
                        cr->beam()->remove(cr);
                  for (Lyrics* lyr : cr->lyrics())
                        lyr->removeFromScore();
                                    }
                  break;
            case ElementType::TEMPO_TEXT:
                  {
                  TempoText* tt = toTempoText(element);
                  int tick = tt->segment()->tick();
                  tempomap()->delTempo(tick);
                  }
                  break;
            case ElementType::INSTRUMENT_CHANGE: {
                  InstrumentChange* ic = toInstrumentChange(element);
                  int tickStart = ic->segment()->tick();
                  auto i = ic->part()->instruments()->upper_bound(tickStart);
                  int tickEnd;
                  if (i == ic->part()->instruments()->end())
                        tickEnd = -1;
                  else
                        tickEnd = i->first;
                  Interval oldV = ic->part()->instrument(tickStart)->transpose();
                  ic->part()->removeInstrument(tickStart);
                  transpositionChanged(ic->part(), oldV, tickStart, tickEnd);
                  masterScore()->rebuildMidiMapping();
                  cmdState()._instrumentsChanged = true;
                  }
                  break;

            case ElementType::TREMOLO:
            case ElementType::ARTICULATION:
            case ElementType::ARPEGGIO:
                  {
                  Element* cr = element->parent();
                  if (cr->isChord())
                        createPlayEvents(toChord(cr));
                  }
                  break;

            default:
                  break;
            }
      setLayout(element->tick());
      }


Measure* Score::firstMeasure() const
      {
      MeasureBase* mb = _measures.first();
      while (mb && mb->type() != ElementType::MEASURE)
            mb = mb->next();

      return toMeasure(mb);
      }


Measure* Score::firstMeasureMM() const
      {
      Measure* m = firstMeasure();
      if (m && styleB(Sid::createMultiMeasureRests) && m->hasMMRest())
            return m->mmRest();
      return m;
      }


MeasureBase* Score::firstMM() const
      {
      MeasureBase* m = _measures.first();
      if (m
         && m->type() == ElementType::MEASURE
         && styleB(Sid::createMultiMeasureRests)
         && toMeasure(m)->hasMMRest()) {
            return toMeasure(m)->mmRest();
            }
      return m;
      }


MeasureBase* Score::measure(int idx) const
      {
      MeasureBase* mb = _measures.first();
      for (int i = 0; i < idx; ++i) {
            mb = mb->next();
            if (mb == 0)
                  return 0;
            }
      return mb;
      }


Measure* Score::lastMeasure() const
      {
      MeasureBase* mb = _measures.last();
      while (mb && mb->type() != ElementType::MEASURE)
            mb = mb->prev();
      return toMeasure(mb);
      }


Measure* Score::lastMeasureMM() const
      {
      Measure* m = lastMeasure();
      if (m && styleB(Sid::createMultiMeasureRests)) {
            Measure* m1 = const_cast<Measure*>(toMeasure(m->mmRest1()));
            if (m1)
                  return m1;
            }
      return m;
      }


int Score::endTick() const
      {
      Measure* m = lastMeasure();
      return m ? m->endTick() : 0;
      }


Segment* Score::firstSegment(SegmentType segType) const
      {
      Segment* seg;
      Measure* m = firstMeasure();
      if (!m)
            seg = 0;
      else {
            seg = m->first();
            if (seg && !(seg->segmentType() & segType))
                  seg = seg->next1(segType);
            }

#ifdef SCRIPT_INTERFACE
      #endif
      return seg;
      }


Segment* Score::firstSegmentMM(SegmentType segType) const
      {
      Measure* m = firstMeasureMM();
      return m ? m->first(segType) : 0;
      }


Segment* Score::lastSegment() const
      {
      Measure* m = lastMeasure();
      return m ? m->last() : 0;
      }


qreal Score::utick2utime(int tick) const
      {
      return repeatList()->utick2utime(tick);
      }


int Score::utime2utick(qreal utime) const
      {
      return repeatList()->utime2utick(utime);
      }


int Score::inputPos() const
      {
      return _is.tick();
      }


void Score::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            mb->scanElements(data, func, all);
            if (mb->type() == ElementType::MEASURE) {
                  Measure* m = toMeasure(mb);
                  Measure* mmr = m->mmRest();
                  if (mmr)
                        mmr->scanElements(data, func, all);
                  }
            }
      for (Page* page : pages()) {
            for (System* s :page->systems())
                  s->scanElements(data, func, all);
            func(data, page);
            }
      }


void Score::scanElementsInRange(void* data, void (*func)(void*, Element*), bool all)
      {
      Segment* startSeg = _selection.startSegment();
      for (Segment* s = startSeg; s && s !=_selection.endSegment(); s = s->next1()) {
            s->scanElements(data, func, all);
            Measure* m = s->measure();
            if (m && s == m->first()) {
                  Measure* mmr = m->mmRest();
                  if (mmr)
                        mmr->scanElements(data, func, all);
                  }
            }
      for (Element* e : _selection.elements()) {
            if (e->isSpanner()) {
                  Spanner* spanner = toSpanner(e);
                  for (SpannerSegment* ss : spanner->spannerSegments()) {
                        ss->scanElements(data, func, all);
                        }
                  }
            }
      }


void Score::setSelection(const Selection& s)
      {
      deselectAll();
      _selection = s;

      foreach(Element* e, _selection.elements())
            e->setSelected(true);
      }


Text* Score::getText(SubStyleId subStyle)
      {
      MeasureBase* m = first();
      if (m && m->type() == ElementType::VBOX) {
            for (Element* e : m->el()) {
                  if (e->type() == ElementType::TEXT && toText(e)->subStyleId() == subStyle)
                        return toText(e);
                  }
            }
      return 0;
      }


QString Score::metaTag(const QString& s) const
      {
      if (_metaTags.contains(s))
            return _metaTags.value(s);
      return _masterScore->_metaTags.value(s);
      }


void Score::setMetaTag(const QString& tag, const QString& val)
      {
      _metaTags.insert(tag, val);
      }


void MasterScore::addExcerpt(Excerpt* ex)
      {
      Score* score = ex->partScore();

      for (Staff* s : score->staves()) {
            const LinkedElements* ls = s->links();
            if (ls == 0)
                  continue;
            for (auto le : *ls) {
                  Staff* ps = toStaff(le);
                  if (ps->score() == this) {
                        ex->parts().append(ps->part());
                        break;
                        }
                  }
            }
      if (ex->tracks().isEmpty()) {                                     QMultiMap<int, int> tracks;
            for (Staff* s : score->staves()) {
                  const LinkedElements* ls = s->links();
                  if (ls == 0)
                        continue;
                  for (auto le : *ls) {
                        Staff* ps = toStaff(le);
                        if (ps->primaryStaff()) {
                              for (int i = 0; i < VOICES; i++)
                                    tracks.insert(ps->idx() * VOICES + i % VOICES, s->idx() * VOICES + i % VOICES);
                              break;
                              }
                        }
                  }
            ex->setTracks(tracks);
            }
      excerpts().append(ex);
      setExcerptsChanged(true);
      }


void MasterScore::removeExcerpt(Excerpt* ex)
      {
      if (excerpts().removeOne(ex)) {
            setExcerptsChanged(true);
                        }
      else
            qDebug("removeExcerpt:: ex not found");
      }


MasterScore* MasterScore::clone()
      {
      QBuffer buffer;
      buffer.open(QIODevice::WriteOnly);
      XmlWriter xml(this, &buffer);
      xml.header();

      xml.stag("museScore version=\"" MSC_VERSION "\"");
      write(xml, false);
      xml.etag();

      buffer.close();

      XmlReader r(buffer.buffer());
      MasterScore* score = new MasterScore(style());
      score->read1(r, true);

      score->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
      score->doLayout();
      return score;
      }


void Score::setSynthesizerState(const SynthesizerState& s)
      {
            _synthesizerState = s;
      }


void Score::removeAudio()
      {
      delete _audio;
      _audio = 0;
      }


bool Score::appendScore(Score* score, bool addPageBreak, bool addSectionBreak)
      {
      if (parts().size() < score->parts().size() || staves().size() < score->staves().size()) {
            qDebug("Score to append has %d parts and %d staves, but this score only has %d parts and %d staves.", score->parts().size(), score->staves().size(), parts().size(), staves().size());
            return false;
            }

      if (!last()) {
            qDebug("This score doesn't have any MeasureBase objects.");
            return false;
            }

      TieMap tieMap;
      int tickOfAppend = last()->endTick();

            if (addPageBreak) {
            if (!last()->pageBreak()) {
                  last()->undoSetBreak(false, LayoutBreak::Type::LINE);                   last()->undoSetBreak(true, LayoutBreak::Type::PAGE);                    }
            }
      else if (!last()->lineBreak() && !last()->pageBreak())
            last()->undoSetBreak(true, LayoutBreak::Type::LINE);
      if (addSectionBreak && !last()->sectionBreak())
            last()->undoSetBreak(true, LayoutBreak::Type::SECTION);

            if (styleB(Sid::concertPitch) != score->styleB(Sid::concertPitch))
            score->cmdConcertPitchChanged(styleB(Sid::concertPitch), true);

            Measure* fm = score->firstMeasure();
      if (fm) {
            Segment* seg = fm->findSegmentR(SegmentType::HeaderClef, 0);
            if (seg) {
                  for (int staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
                        int track    = staffIdx * VOICES;
                        Staff* staff = score->staff(staffIdx);
                        Clef* clef   = toClef(seg->element(track));

                                                                        if (clef && clef->generated() && clef->clefType() != this->staff(staffIdx)->clef(tickOfAppend)) {
                                                                                          score->undoChangeClef(staff, seg, clef->clefType());
                              }
                        }
                  }
            }

            MeasureBaseList* ml = &score->_measures;
      for (MeasureBase* mb = ml->first(); mb; mb = mb->next()) {
            MeasureBase* nmb;
            if (mb->type() == ElementType::MEASURE)
                  nmb = toMeasure(mb)->cloneMeasure(this, &tieMap);
            else
                  nmb = mb->clone();
            nmb->setNext(0);
            nmb->setPrev(0);
            nmb->setScore(this);
            _measures.add(nmb);
            }
      fixTicks();
      Measure* firstAppendedMeasure = tick2measure(tickOfAppend);

                  for (Measure* m = firstAppendedMeasure; m; m = m->nextMeasure()) {
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  Fraction f;
                  for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                        for (int v = 0; v < VOICES; ++v) {
                              ChordRest* cr = toChordRest(s->element(staffIdx * VOICES + v));
                              if (cr == 0)
                                    continue;
                              f += cr->actualFraction();
                              }
                        }
                  if (f.isZero())
                        addRest(m->tick(), staffIdx*VOICES, TDuration(TDuration::DurationType::V_MEASURE), 0);
                  }
            }

            if (firstAppendedMeasure) {
            Segment* seg = firstAppendedMeasure->getSegment(SegmentType::KeySig, tickOfAppend);
            for (Staff* st : score->staves()) {
                  int staffIdx = st->idx();
                  Staff* joinedStaff = staff(staffIdx);
                                    if (!seg->element(staffIdx * VOICES)) {
                                                                        if (joinedStaff->key(tickOfAppend - 1) == Key::C)
                              continue;
                        Key key = Key::C;
                        KeySig* ks = new KeySig(this);
                        ks->setTrack(staffIdx * VOICES);
                        ks->setKey(key);
                        ks->setParent(seg);
                        addElement(ks);
                        }
                                    for (auto k : *(st->keyList())) {
                        int tick = k.first;
                        KeySigEvent key = k.second;
                        joinedStaff->setKey(tick + tickOfAppend, key);
                        }
                  }
            }

            for (auto sp : score->spanner()) {
            Spanner* spanner = sp.second;
            Spanner* ns = toSpanner(spanner->clone());
            ns->setScore(this);
            ns->setParent(0);
            ns->setTick(spanner->tick() + tickOfAppend);
            ns->setTick2(spanner->tick2() + tickOfAppend);
            if (ns->type() == ElementType::SLUR) {
                                    ns->setStartElement(0);
                  ns->setEndElement(0);
                  Measure* sm = tick2measure(ns->tick());
                  if (sm)
                        ns->setStartElement(sm->findChordRest(ns->tick(), ns->track()));
                  Measure * em = tick2measure(ns->tick2());
                  if (em)
                        ns->setEndElement(em->findChordRest(ns->tick2(), ns->track2()));
                  if (!ns->startElement())
                        qDebug("clone Slur: no start element");
                  if (!ns->endElement())
                        qDebug("clone Slur: no end element");
                  }
            addElement(ns);
            }
      setLayoutAll();
      return true;
      }


void Score::splitStaff(int staffIdx, int splitPoint)
      {

                        Staff* s  = staff(staffIdx);
      Part*  p  = s->part();
      Staff* ns = new Staff(this);
      ns->setPart(p);
            int staffIdxPart = staffIdx - p->staff(0)->idx();
      undoInsertStaff(ns, staffIdxPart + 1, false);

      Clef* clef = new Clef(this);
      clef->setClefType(ClefType::F);
      clef->setTrack((staffIdx+1) * VOICES);
      Segment* seg = firstMeasure()->getSegment(SegmentType::HeaderClef, 0);
      clef->setParent(seg);
      undoAddElement(clef);
      clef->layout();

      undoChangeKeySig(ns, 0, s->keySigEvent(0));

      masterScore()->rebuildMidiMapping();
      cmdState()._instrumentsChanged = true;
      doLayout();

                        select(0, SelectType::SINGLE, 0);
      int strack = staffIdx * VOICES;
      int dtrack = (staffIdx + 1) * VOICES;

      for (Segment* s = firstSegment(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            for (int voice = 0; voice < VOICES; ++voice) {
                  Element* e = s->element(strack + voice);
                  if (!(e && e->isChord()))
                        continue;
                  Chord* c = toChord(e);
                  QList<Note*> removeNotes;
                  foreach(Note* note, c->notes()) {
                        if (note->pitch() >= splitPoint)
                              continue;
                        Chord* chord = toChord(s->element(dtrack + voice));
                        Q_ASSERT(!chord || (chord->isChord()));
                        if (chord == 0) {
                              chord = new Chord(*c);
                              qDeleteAll(chord->notes());
                              chord->notes().clear();
                              chord->setTrack(dtrack + voice);
                              undoAddElement(chord);
                              }
                        Note* nnote = new Note(*note);
                        nnote->setTrack(dtrack + voice);
                        chord->add(nnote);
                        nnote->updateLine();
                        removeNotes.append(note);
                        }
                  c->sortNotes();
                  for (Note* note : removeNotes) {
                        undoRemoveElement(note);
                        Chord* chord = note->chord();
                        if (chord->notes().empty()) {
                              for (auto sp : spanner()) {
                                    Slur* slur = toSlur(sp.second);
                                    if (slur->type() != ElementType::SLUR)
                                          continue;
                                    if (slur->startCR() == chord) {
                                          slur->undoChangeProperty(Pid::TRACK, slur->track()+VOICES);
                                          for (ScoreElement* ee : slur->linkList()) {
                                                Slur* lslur = toSlur(ee);
                                                lslur->setStartElement(0);
                                                }
                                          }
                                    if (slur->endCR() == chord) {
                                          slur->undoChangeProperty(Pid::SPANNER_TRACK2, slur->track2()+VOICES);
                                          for (ScoreElement* ee : slur->linkList()) {
                                                Slur* lslur = toSlur(ee);
                                                lslur->setEndElement(0);
                                                }
                                          }
                                    }
                              undoRemoveElement(chord);
                              }
                        }
                  }
            }
                              int ctick  = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
                  ChordRest* cr = toChordRest(s->element(dtrack));
                  if (cr == 0)
                        continue;
                  int rest = s->tick() - ctick;
                  if (rest) {
                                                Segment* s = tick2segment(ctick);
                        if (s == 0) {
                              qDebug("no segment at %d", ctick);
                              continue;
                              }
                        setRest(ctick, dtrack, Fraction::fromTicks(rest), false, 0);
                        }
                  ctick = s->tick() + cr->actualTicks();
                  }
            int rest = m->tick() + m->ticks() - ctick;
            if (rest) {
                  setRest(ctick, dtrack, Fraction::fromTicks(rest), false, 0);
                  ctick += rest;
                  }
            }
                        ctick  = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
                  ChordRest* cr = toChordRest(s->element(strack));
                  if (cr == 0)
                        continue;
                  int rest = s->tick() - ctick;
                  if (rest) {
                                                Segment* s = tick2segment(ctick);
                        if (s == 0) {
                              qDebug("no segment at %d", ctick);
                              continue;
                              }
                        setRest(ctick, strack, Fraction::fromTicks(rest), false, 0);
                        }
                  ctick = s->tick() + cr->actualTicks();
                  }
            int rest = m->tick() + m->ticks() - ctick;
            if (rest) {
                  setRest(ctick, strack, Fraction::fromTicks(rest), false, 0);
                  ctick += rest;
                  }
            }
      }


void Score::cmdRemovePart(Part* part)
      {
      int sidx   = staffIdx(part);
      int n      = part->nstaves();

      for (int i = 0; i < n; ++i)
            cmdRemoveStaff(sidx);

      undoRemovePart(part, sidx);
      }


void Score::insertPart(Part* part, int idx)
      {
      int staff = 0;
      for (QList<Part*>::iterator i = _parts.begin(); i != _parts.end(); ++i) {
            if (staff >= idx) {
                  _parts.insert(i, part);
                  return;
                  }
            staff += (*i)->nstaves();
            }
      _parts.push_back(part);
      }


void Score::removePart(Part* part)
      {
      _parts.removeAt(_parts.indexOf(part));
      }


void Score::insertStaff(Staff* staff, int ridx)
      {
      staff->part()->insertStaff(staff, ridx);

      int idx = staffIdx(staff->part()) + ridx;
      _staves.insert(idx, staff);

      for (auto i = staff->score()->spanner().cbegin(); i != staff->score()->spanner().cend(); ++i) {
            Spanner* s = i->second;
            if (s->systemFlag())
                  continue;
            if (s->staffIdx() >= idx) {
                  int t = s->track() + VOICES;
                  if (t >= ntracks())
                        t = ntracks() - 1;
                  s->setTrack(t);
                  for (SpannerSegment* ss : s->spannerSegments())
                        ss->setTrack(t);
                  if (s->track2() != -1) {
                        t = s->track2() + VOICES;
                        s->setTrack2(t < ntracks() ? t : s->track());
                        }
                  }
            }
#if 0
      for (Spanner* s : staff->score()->unmanagedSpanners()) {
            if (s->systemFlag())
                  continue;
            if (s->staffIdx() >= idx) {
                  int t = s->track() + VOICES;
                  s->setTrack(t < ntracks() ? t : ntracks() - 1);
                  if (s->track2() != -1) {
                        t = s->track2() + VOICES;
                        s->setTrack2(t < ntracks() ? t : s->track());
                        }
                  }
            }
#endif
      }


void Score::removeStaff(Staff* staff)
      {
      int idx = staff->idx();
      for (auto i = staff->score()->spanner().cbegin(); i != staff->score()->spanner().cend(); ++i) {
            Spanner* s = i->second;
            if (s->staffIdx() > idx) {
                  int t = s->track() - VOICES;
                  if (t < 0)
                        t = 0;
                  s->setTrack(t);
                  for (SpannerSegment* ss : s->spannerSegments())
                        ss->setTrack(t);
                  if (s->track2() != -1) {
                        t = s->track2() - VOICES;
                        s->setTrack2(t >= 0 ? t : s->track());
                        }
                  }
            }
#if 0
      for (Spanner* s : staff->score()->unmanagedSpanners()) {
            if (s->staffIdx() > idx) {
                  int t = s->track() - VOICES;
                  s->setTrack(t >= 0 ? t : 0);
                  if (s->track2() != -1) {
                        t = s->track2() - VOICES;
                        s->setTrack2(t >= 0 ? t : s->track());
                        }
                  }
            }
#endif
      _staves.removeAll(staff);
      staff->part()->removeStaff(staff);
      }


void Score::adjustBracketsDel(int sidx, int eidx)
      {
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            Staff* staff = _staves[staffIdx];
            for (BracketItem* bi : staff->brackets()) {
                  int span = bi->bracketSpan();
                  if ((span == 0) || ((staffIdx + span) < sidx) || (staffIdx > eidx))
                        continue;
                  if ((sidx >= staffIdx) && (eidx <= (staffIdx + span)))
                        bi->undoChangeProperty(Pid::BRACKET_SPAN, span - (eidx-sidx));
                  }
#if 0             int span = staff->barLineSpan();
            if ((sidx >= staffIdx) && (eidx <= (staffIdx + span))) {
                  int newSpan = span - (eidx-sidx) + 1;
                  int lastSpannedStaffIdx = staffIdx + newSpan - 1;
                  int tick = 0;
                  undoChangeBarLineSpan(staff, newSpan, 0, (_staves[lastSpannedStaffIdx]->lines(0)-1)*2);
                  }
#endif
            }
      }


void Score::adjustBracketsIns(int sidx, int eidx)
      {
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            Staff* staff = _staves[staffIdx];
            for (BracketItem* bi : staff->brackets()) {
                  int span = bi->bracketSpan();
                  if ((span == 0) || ((staffIdx + span) < sidx) || (staffIdx > eidx))
                        continue;
                  if ((sidx >= staffIdx) && (eidx < (staffIdx + span)))
                        bi->undoChangeProperty(Pid::BRACKET_SPAN, span + (eidx-sidx));
                  }
#if 0             int span = staff->barLineSpan();
            if ((sidx >= staffIdx) && (eidx < (staffIdx + span))) {
                  int idx = staffIdx + span - 1;
                  if (idx >= _staves.size())
                        idx = _staves.size() - 1;
                  undoChangeBarLineSpan(staff, span, 0, (_staves[idx]->lines()-1)*2);
                  }
#endif
            }
      }


void Score::adjustKeySigs(int sidx, int eidx, KeyList km)
      {
      for (int staffIdx = sidx; staffIdx < eidx; ++staffIdx) {
            Staff* staff = _staves[staffIdx];
            for (auto i = km.begin(); i != km.end(); ++i) {
                  int tick = i->first;
                  Measure* measure = tick2measure(tick);
                  if (!measure)
                        continue;
                  if (staff->isDrumStaff(tick))
                        continue;
                  KeySigEvent oKey = i->second;
                  KeySigEvent nKey = oKey;
                  int diff = -staff->part()->instrument(tick)->transpose().chromatic;
                  if (diff != 0 && !styleB(Sid::concertPitch) && !oKey.custom() && !oKey.isAtonal())
                        nKey.setKey(transposeKey(nKey.key(), diff));
                  staff->setKey(tick, nKey);
                  KeySig* keysig = new KeySig(this);
                  keysig->setTrack(staffIdx * VOICES);
                  keysig->setKeySigEvent(nKey);
                  Segment* s = measure->getSegment(SegmentType::KeySig, tick);
                  s->add(keysig);
                  }
            }
      }


void Score::cmdRemoveStaff(int staffIdx)
      {
      Staff* s = staff(staffIdx);
      adjustBracketsDel(staffIdx, staffIdx+1);

      QList<Spanner*> sl;
      for (auto i = _spanner.cbegin(); i != _spanner.cend(); ++i) {
            Spanner* s = i->second;
            if (s->staffIdx() == staffIdx && (staffIdx != 0 || !s->systemFlag()))
                  sl.append(s);
            }
      for (Spanner* s : _unmanagedSpanner) {
            if (s->staffIdx() == staffIdx && (staffIdx != 0 || !s->systemFlag()))
                  sl.append(s);
            }
      for (auto i : sl) {
            i->undoUnlink();
            undo(new RemoveElement(i));
            }

      undoRemoveStaff(s);

            
      if (s->links()) {
            Staff* sameScoreLinkedStaff = 0;
            auto staves = s->links();
            for (auto le : *staves) {
                  Staff* staff = toStaff(le);
                  if (staff == s)
                        continue;
                  Score* lscore = staff->score();
                  if (lscore != this) {
                        lscore->undoRemoveStaff(staff);
                        s->score()->undo(new Unlink(staff));
                        if (staff->part()->nstaves() == 0) {
                              int pIndex    = lscore->staffIdx(staff->part());
                              lscore->undoRemovePart(staff->part(), pIndex);
                              }
                        }
                  else                        sameScoreLinkedStaff = staff;
                  }
            if (sameScoreLinkedStaff)
                  s->score()->undo(new Unlink(s));             }
      }


void Score::sortStaves(QList<int>& dst)
      {
      systems().clear();        _parts.clear();
      Part* curPart = 0;
      QList<Staff*> dl;
      foreach (int idx, dst) {
            Staff* staff = _staves[idx];
            if (staff->part() != curPart) {
                  curPart = staff->part();
                  curPart->staves()->clear();
                  _parts.push_back(curPart);
                  }
            curPart->staves()->push_back(staff);
            dl.push_back(staff);
            }
      _staves = dl;

      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            m->sortStaves(dst);
            if (m->hasMMRest())
                  m->mmRest()->sortStaves(dst);
            }
      for (auto i : _spanner.map()) {
            Spanner* sp = i.second;
            if (sp->systemFlag())
                  continue;
            int voice    = sp->voice();
            int staffIdx = sp->staffIdx();
            int idx = dst.indexOf(staffIdx);
            if (idx >=0) {
                  sp->setTrack(idx * VOICES + voice);
                  if (sp->track2() != -1)
                        sp->setTrack2(idx * VOICES +(sp->track2() % VOICES));                   }
            }
      }


void Score::cmdConcertPitchChanged(bool flag, bool )
      {
      undoChangeStyleVal(Sid::concertPitch, flag);       
      for (Staff* staff : _staves) {
            if (staff->staffType(0)->group() == StaffGroup::PERCUSSION)                         continue;
                        Interval interval = staff->part()->instrument()->transpose();
            if (interval.isZero() && staff->part()->instruments()->size() == 1)
                  continue;
            if (!flag)
                  interval.flip();

            int staffIdx   = staff->idx();
            int startTrack = staffIdx * VOICES;
            int endTrack   = startTrack + VOICES;

            transposeKeys(staffIdx, staffIdx + 1, 0, lastSegment()->tick(), interval, true, !flag);

            for (Segment* segment = firstSegment(SegmentType::ChordRest); segment; segment = segment->next1(SegmentType::ChordRest)) {
                  interval = staff->part()->instrument(segment->tick())->transpose();
                  if (!flag)
                        interval.flip();
                  for (Element* e : segment->annotations()) {
                        if (!e->isHarmony() || (e->track() < startTrack) || (e->track() >= endTrack))
                              continue;
                        Harmony* h  = toHarmony(e);
                        int rootTpc = transposeTpc(h->rootTpc(), interval, true);
                        int baseTpc = transposeTpc(h->baseTpc(), interval, true);
                        for (ScoreElement* e : h->linkList()) {
                                                                                          Harmony* he = toHarmony(e);                                  if (he->staff() == h->staff())
                                    undoTransposeHarmony(he, rootTpc, baseTpc);
                              }
                        }
                  }
            }
      }


void Score::addAudioTrack()
      {
            }


void Score::padToggle(Pad n)
      {
      int oldDots = _is.duration().dots();
      switch (n) {
            case Pad::NOTE00:
                  _is.setDuration(TDuration::DurationType::V_LONG);
                  break;
            case Pad::NOTE0:
                  _is.setDuration(TDuration::DurationType::V_BREVE);
                  break;
            case Pad::NOTE1:
                  _is.setDuration(TDuration::DurationType::V_WHOLE);
                  break;
            case Pad::NOTE2:
                  _is.setDuration(TDuration::DurationType::V_HALF);
                  break;
            case Pad::NOTE4:
                  _is.setDuration(TDuration::DurationType::V_QUARTER);
                  break;
            case Pad::NOTE8:
                  _is.setDuration(TDuration::DurationType::V_EIGHTH);
                  break;
            case Pad::NOTE16:
                  _is.setDuration(TDuration::DurationType::V_16TH);
                  break;
            case Pad::NOTE32:
                  _is.setDuration(TDuration::DurationType::V_32ND);
                  break;
            case Pad::NOTE64:
                  _is.setDuration(TDuration::DurationType::V_64TH);
                  break;
            case Pad::NOTE128:
                  _is.setDuration(TDuration::DurationType::V_128TH);
                  break;
            case Pad::REST:
                  _is.setRest(!_is.rest());
                  break;
            case Pad::DOT:
                  if ((_is.duration().dots() == 1) || (_is.duration() == TDuration::DurationType::V_1024TH))
                        _is.setDots(0);
                  else
                        _is.setDots(1);
                  break;
            case Pad::DOTDOT:
                  if ((_is.duration().dots() == 2)
                     || (_is.duration() == TDuration::DurationType::V_512TH)
                     || (_is.duration() == TDuration::DurationType::V_1024TH))
                        _is.setDots(0);
                  else
                        _is.setDots(2);
                  break;
            case Pad::DOT3:
                  if ((_is.duration().dots() == 3)
                     || (_is.duration() == TDuration::DurationType::V_256TH)
                     || (_is.duration() == TDuration::DurationType::V_512TH)
                     || (_is.duration() == TDuration::DurationType::V_1024TH))
                        _is.setDots(0);
                  else
                        _is.setDots(3);
                  break;
            case Pad::DOT4:
                  if ((_is.duration().dots() == 4)
                     || (_is.duration() == TDuration::DurationType::V_128TH)
                     || (_is.duration() == TDuration::DurationType::V_256TH)
                     || (_is.duration() == TDuration::DurationType::V_512TH)
                     || (_is.duration() == TDuration::DurationType::V_1024TH))
                        _is.setDots(0);
                  else
                        _is.setDots(4);
                  break;
            }
      if (n >= Pad::NOTE00 && n <= Pad::NOTE128) {
            _is.setDots(0);
                                                            if (noteEntryMode()) {
                  if (usingNoteEntryMethod(NoteEntryMethod::RHYTHM)) {
                        switch (oldDots) {
                              case 1:
                                    padToggle(Pad::DOT);
                                    break;
                              case 2:
                                    padToggle(Pad::DOTDOT);
                                    break;
                              }
                        NoteVal nval;
                        if (_is.rest()) {
                                                            nval = NoteVal();
                              }
                        else {
                                                            Staff* s = staff(_is.track() / VOICES);
                              int tick = _is.tick();
                              ClefType clef = s->clef(tick);
                              Key key = s->key(tick);
                              nval = NoteVal(line2pitch(4, clef, key));
                              }
                        setNoteRest(_is.segment(), _is.track(), nval, _is.duration().fraction());
                        _is.moveToNextInputPos();
                        }
                  else
                        _is.setRest(false);
                  }
            }

      if (noteEntryMode() || !selection().isSingle())
            return;

            Element* e = selection().element();
      if (e && e->isRest()) {
            Rest* r = toRest(e);
            if (r->isFullMeasureRest())
                  _is.setDots(0);
            }

            ChordRest* cr = selection().cr();
      if (cr && cr->isRest() && cr->measure()->isMMRest()) {
            Measure* m = cr->measure()->mmRestFirst();
            if (m)
                  cr = m->findChordRest(0, 0);
            }

      if (!cr)
            return;

      if (cr->isChord() && (toChord(cr)->isGrace())) {
                                                undoChangeChordRestLen(cr, _is.duration());
            }
      else
            changeCRlen(cr, _is.duration());
      }


void Score::deselect(Element* el)
      {
      addRefresh(el->abbox());
      _selection.remove(el);
      setSelectionChanged(true);
      update();
      }


void Score::select(Element* e, SelectType type, int staffIdx)
      {
      if (e && (e->isNote() || e->isRest())) {
            Element* ee = e;
            if (ee->isNote())
                  ee = ee->parent();
            int tick = toChordRest(ee)->segment()->tick();
            if (playPos() != tick)
                  setPlayPos(tick);
            }
      if (MScore::debugMode)
            qDebug("select element <%s> type %d(state %d) staff %d",
               e ? e->name() : "", int(type), int(selection().state()), e ? e->staffIdx() : -1);

      switch (type) {
            case SelectType::SINGLE:
                  selectSingle(e, staffIdx);
                  break;
            case SelectType::ADD:
                  selectAdd(e);
                  break;
            case SelectType::RANGE:
                  selectRange(e, staffIdx);
                  break;
            }
      setSelectionChanged(true);
      }


void Score::selectSingle(Element* e, int staffIdx)
      {
      SelState selState = _selection.state();
      deselectAll();
      if (e == 0) {
            selState = SelState::NONE;
            setUpdateAll();
            }
      else {
            if (e->isMeasure()) {
                  select(e, SelectType::RANGE, staffIdx);
                  return;
                  }
            addRefresh(e->abbox());
            _selection.add(e);
            _is.setTrack(e->track());
            selState = SelState::LIST;
            if (e->type() == ElementType::NOTE) {
                  e = e->parent();
                  }
            if (e->isChordRest()) {
                  _is.setLastSegment(_is.segment());
                  _is.setSegment(toChordRest(e)->segment());
                  }
            }
      _selection.setActiveSegment(0);
      _selection.setActiveTrack(0);

      _selection.setState(selState);
      }


void Score::selectAdd(Element* e)
      {
      SelState selState = _selection.state();

      if (_selection.isRange()) {
            select(0, SelectType::SINGLE, 0);
            return;
            }

      if (e->isMeasure()) {
            Measure* m = toMeasure(e);
            int tick  = m->tick();
            if (_selection.isNone()) {
                  _selection.setRange(m->tick2segment(tick),
                                      m == lastMeasure() ? 0 : m->last(),
                                      0,
                                      nstaves());
                  setUpdateAll();
                  selState = SelState::RANGE;
                  _selection.updateSelectedElements();
                  }
            }
      else {             addRefresh(e->abbox());
            if (_selection.elements().contains(e))
                  _selection.remove(e);
            else {
                  selState = SelState::LIST;
                  _selection.add(e);
                  }
            }
      _selection.setState(selState);
      }


void Score::selectRange(Element* e, int staffIdx)
      {
      int activeTrack = e->track();
            bool endRangeSelected = selection().isRange() && selection().endSegment() == nullptr;
      if (e->type() == ElementType::MEASURE) {
            Measure* m = toMeasure(e);
            int tick  = m->tick();
            int etick = tick + m->ticks();
            activeTrack = staffIdx * VOICES;
            if (_selection.isNone()
               || (_selection.isList() && !_selection.isSingle())) {
                        if (_selection.isList())
                              deselectAll();
                  _selection.setRange(m->tick2segment(tick),
                                      m == lastMeasure() ? 0 : m->last(),
                                      staffIdx,
                                      staffIdx + 1);
                  }
            else if (_selection.isRange()) {
                  _selection.extendRangeSelection(m->tick2segment(tick),
                                                  m == lastMeasure() ? 0 : m->last(),
                                                  staffIdx,
                                                  tick,
                                                  etick);
                  }
            else if (_selection.isSingle()) {
                  Element* oe = selection().element();
                  if (oe->isNote() || oe->isChordRest()) {
                        if (oe->isNote())
                              oe = oe->parent();

                        ChordRest* cr = toChordRest(oe);
                        int oetick = cr->segment()->tick();
                        Segment* startSegment = cr->segment();
                        Segment* endSegment = m->last();
                        if (tick < oetick) {
                              startSegment = m->tick2segment(tick);
                              if (etick <= oetick)
                                    endSegment = cr->nextSegmentAfterCR(SegmentType::ChordRest
                                                                                    | SegmentType::EndBarLine
                                                                                    | SegmentType::Clef);

                              }
                        int staffStart = staffIdx;
                        int endStaff = staffIdx + 1;
                        if (staffStart > cr->staffIdx())
                              staffStart = cr->staffIdx();
                        else if (cr->staffIdx() >= endStaff)
                              endStaff = cr->staffIdx() + 1;
                        _selection.setRange(startSegment, endSegment, staffStart, endStaff);
                        }
                  else {
                        deselectAll();
                        _selection.setRange(m->tick2segment(tick),
                                            m == lastMeasure() ? 0 : m->last(),
                                            staffIdx,
                                            staffIdx + 1);
                        }
                  }
            else {
                  qDebug("SELECT_RANGE: measure: sel state %d", int(_selection.state()));
                  return;
                  }
            }
      else if (e->type() == ElementType::NOTE || e->isChordRest()) {
            if (e->type() == ElementType::NOTE)
                  e = e->parent();
            ChordRest* cr = toChordRest(e);

            if (_selection.isNone()
                || (_selection.isList() && !_selection.isSingle())) {
                  if (_selection.isList())
                        deselectAll();
                  _selection.setRange(cr->segment(),
                                      cr->nextSegmentAfterCR(SegmentType::ChordRest
                                                             | SegmentType::EndBarLine
                                                             | SegmentType::Clef),
                                      e->staffIdx(),
                                      e->staffIdx() + 1);
                  activeTrack = cr->track();
                  }
            else if (_selection.isSingle()) {
                  Element* oe = _selection.element();
                  if (oe && (oe->type() == ElementType::NOTE || oe->type() == ElementType::REST)) {
                        if (oe->type() == ElementType::NOTE)
                              oe = oe->parent();
                        ChordRest* ocr = toChordRest(oe);

                        Segment* endSeg = tick2segmentMM(ocr->segment()->tick() + ocr->actualTicks());
                        if (!endSeg)
                              endSeg = ocr->segment()->next();

                        _selection.setRange(ocr->segment(),
                                            endSeg,
                                            oe->staffIdx(),
                                            oe->staffIdx() + 1);
                        _selection.extendRangeSelection(cr);

                        }
                  else {
                        select(e, SelectType::SINGLE, 0);
                        return;
                        }
                  }
            else if (_selection.isRange()) {
                  _selection.extendRangeSelection(cr);
                  }
            else {
                  qDebug("sel state %d", int(_selection.state()));
                  return;
                  }
            if (!endRangeSelected && !_selection.endSegment())
                  _selection.setEndSegment(cr->segment()->nextCR());
            if (!_selection.startSegment())
                  _selection.setStartSegment(cr->segment());
            }
      else {
            select(e, SelectType::SINGLE, staffIdx);
            return;
            }

      _selection.setActiveTrack(activeTrack);

            if (_selection.startSegment() && !noteEntryMode())
            setPlayPos(_selection.startSegment()->tick());

      _selection.updateSelectedElements();
      }


void Score::collectMatch(void* data, Element* e)
      {
      ElementPattern* p = static_cast<ElementPattern*>(data);
      if (p->type != int(e->type()))
            return;

      if (p->type == int(ElementType::NOTE)) {
            if (p->subtype < 0) {
                  if (!(toNote(e)->chord()->isGrace()))
                        return;
                  }
            else if ((toNote(e)->chord()->isGrace()) || (p->subtype != e->subtype()))
                  return;
            }
      else if (p->subtypeValid && p->subtype != e->subtype())
            return;

      if ((p->staffStart != -1)
         && ((p->staffStart > e->staffIdx()) || (p->staffEnd <= e->staffIdx())))
            return;

      if (p->voice != -1 && p->voice != e->voice())
            return;

      if (p->system) {
            Element* ee = e;
            do {
                  if (ee->type() == ElementType::SYSTEM) {
                        if (p->system != ee)
                              return;
                        break;
                        }
                  ee = ee->parent();
                  } while (ee);
            }
      p->el.append(e);
      }


void Score::collectNoteMatch(void* data, Element* e)
      {
      NotePattern* p = static_cast<NotePattern*>(data);
      if (!e->isNote())
            return;
      Note* n = toNote(e);
      if (p->type != NoteType::INVALID && p->type != n->noteType())
            return;
      if (p->pitch != -1 && p->pitch != n->pitch())
            return;
      if (p->string != STRING_NONE && p->string != n->string())
            return;
      if (p->tpc != Tpc::TPC_INVALID && p->tpc != n->tpc())
            return;
      if (p->notehead != NoteHead::Group::HEAD_INVALID && p->notehead != n->headGroup())
            return;
      if (p->duration.type() != TDuration::DurationType::V_INVALID && p->duration != n->chord()->actualDurationType())
            return;
      if ((p->staffStart != -1)
         && ((p->staffStart > e->staffIdx()) || (p->staffEnd <= e->staffIdx())))
            return;
      if (p->voice != -1 && p->voice != e->voice())
            return;
      if (p->system && (p->system != n->chord()->segment()->system()))
            return;
      p->el.append(n);
      }



void Score::selectSimilar(Element* e, bool sameStaff)
      {
      ElementType type = e->type();
      Score* score = e->score();

      ElementPattern pattern;
      pattern.type = int(type);
      pattern.subtype = 0;
      pattern.subtypeValid = false;
      if (type == ElementType::NOTE) {
            if (toNote(e)->chord()->isGrace())
                  pattern.subtype = -1;             else
                  pattern.subtype = e->subtype();
            }
      pattern.staffStart = sameStaff ? e->staffIdx() : -1;
      pattern.staffEnd = sameStaff ? e->staffIdx() + 1 : -1;
      pattern.voice   = -1;
      pattern.system  = 0;

      score->scanElements(&pattern, collectMatch);

      score->select(0, SelectType::SINGLE, 0);
      for (Element* e : pattern.el)
            score->select(e, SelectType::ADD, 0);
      }


void Score::selectSimilarInRange(Element* e)
      {
      ElementType type = e->type();
      Score* score = e->score();

      ElementPattern pattern;
      pattern.type    = int(type);
      pattern.subtype = 0;
      pattern.subtypeValid = false;
      if (type == ElementType::NOTE) {
            if (toNote(e)->chord()->isGrace())
                  pattern.subtype = -1;             else
                  pattern.subtype = e->subtype();
            pattern.subtypeValid = true;
            }
      pattern.staffStart = selection().staffStart();
      pattern.staffEnd = selection().staffEnd();
      pattern.voice   = -1;
      pattern.system  = 0;

      score->scanElementsInRange(&pattern, collectMatch);

      score->select(0, SelectType::SINGLE, 0);
      for (Element* e : pattern.el)
            score->select(e, SelectType::ADD, 0);
      }


void Score::lassoSelect(const QRectF& bbox)
      {
      select(0, SelectType::SINGLE, 0);
      QRectF fr(bbox.normalized());
      foreach(Page* page, pages()) {
            QRectF pr(page->bbox());
            QRectF frr(fr.translated(-page->pos()));
            if (pr.right() < frr.left())
                  continue;
            if (pr.left() > frr.right())
                  break;

            QList<Element*> el = page->items(frr);
            for (int i = 0; i < el.size(); ++i) {
                  Element* e = el.at(i);
                  if (frr.contains(e->abbox())) {
                        if (e->type() != ElementType::MEASURE && e->selectable())
                              select(e, SelectType::ADD, 0);
                        }
                  }
            }
      }


void Score::lassoSelectEnd()
      {
      int noteRestCount     = 0;
      Segment* startSegment = 0;
      Segment* endSegment   = 0;
      int startStaff        = 0x7fffffff;
      int endStaff          = 0;
      const ChordRest* endCR = 0;

      if (_selection.elements().empty()) {
            _selection.setState(SelState::NONE);
            setUpdateAll();
            return;
            }
      _selection.setState(SelState::LIST);

      foreach(const Element* e, _selection.elements()) {
            if (e->type() != ElementType::NOTE && e->type() != ElementType::REST)
                  continue;
            ++noteRestCount;
            if (e->type() == ElementType::NOTE)
                  e = e->parent();
            Segment* seg = static_cast<const ChordRest*>(e)->segment();
            if ((startSegment == 0) || (*seg < *startSegment))
                  startSegment = seg;
            if ((endSegment == 0) || (*seg > *endSegment)) {
                  endSegment = seg;
                  endCR = static_cast<const ChordRest*>(e);
                  }
            int idx = e->staffIdx();
            if (idx < startStaff)
                  startStaff = idx;
            if (idx > endStaff)
                  endStaff = idx;
            }
      if (noteRestCount > 0) {
            endSegment = endCR->nextSegmentAfterCR(SegmentType::ChordRest
               | SegmentType::EndBarLine
               | SegmentType::Clef);
            _selection.setRange(startSegment, endSegment, startStaff, endStaff+1);
            if (!_selection.isRange())
                  _selection.setState(SelState::RANGE);
            _selection.updateSelectedElements();
            }
      setUpdateAll();
      }


void Score::addLyrics(int tick, int staffIdx, const QString& txt)
      {
      if (txt.trimmed().isEmpty())
            return;
      Measure* measure = tick2measure(tick);
      Segment* seg     = measure->findSegment(SegmentType::ChordRest, tick);
      if (seg == 0) {
            qDebug("no segment found for lyrics<%s> at tick %d",
               qPrintable(txt), tick);
            return;
            }

      bool lyricsAdded = false;
      for (int voice = 0; voice < VOICES; ++voice) {
            int track = staffIdx * VOICES + voice;
            ChordRest* cr = toChordRest(seg->element(track));
            if (cr) {
                  Lyrics* l = new Lyrics(this);
                  l->setXmlText(txt);
                  l->setTrack(track);
                  cr->add(l);
                  lyricsAdded = true;
                  break;
                  }
            }
      if (!lyricsAdded) {
            qDebug("no chord/rest for lyrics<%s> at tick %d, staff %d",
               qPrintable(txt), tick, staffIdx);
            }
      }


void Score::setTempo(Segment* segment, qreal tempo)
      {
      setTempo(segment->tick(), tempo);
      }

void Score::setTempo(int tick, qreal tempo)
      {
      tempomap()->setTempo(tick, tempo);
      _playlistDirty = true;
      }


void Score::removeTempo(int tick)
      {
      tempomap()->delTempo(tick);
      _playlistDirty = true;
      }


void Score::setPause(int tick, qreal seconds)
      {
      tempomap()->setPause(tick, seconds);
      _playlistDirty = true;
      }


qreal Score::tempo(int tick) const
      {
      return tempomap()->tempo(tick);
      }


qreal Score::loWidth() const
      {
      return styleD(Sid::pageWidth) * DPI;
      }


qreal Score::loHeight() const
      {
      return styleD(Sid::pageHeight) * DPI;
      }


void Score::cmdSelectAll()
      {
      if (_measures.size() == 0)
            return;
      deselectAll();
      Measure* first = firstMeasureMM();
      if (!first)
            return;
      Measure* last = lastMeasureMM();
      selectRange(first, 0);
      selectRange(last, nstaves() - 1);
      setUpdateAll();
      update();
      }


void Score::cmdSelectSection()
      {
      Segment* s = _selection.startSegment();
      if (s == 0)
            return;
      MeasureBase* sm = s->measure();
      MeasureBase* em = sm;
      while (sm->prev()) {
            if (sm->prev()->sectionBreak())
                  break;
            sm = sm->prev();
            }
      while (em->next()) {
            if (em->sectionBreak())
                  break;
            em = em->next();
            }
      while (sm && sm->type() != ElementType::MEASURE)
            sm = sm->next();
      while (em && em->type() != ElementType::MEASURE)
            em = em->next();
      if (sm == 0 || em == 0)
            return;

      _selection.setRange(toMeasure(sm)->first(), toMeasure(em)->last(), 0, nstaves());
      }


void Score::undo(UndoCommand* cmd, EditData* ed) const
      {
      undoStack()->push(cmd, ed);
      }


int Score::linkId()
      {
      return (masterScore()->_linkId)++;
      }

void Score::linkId(int val)
      {
      Score* s = masterScore();
      if (val >= s->_linkId)
            s->_linkId = val + 1;         }


QList<Score*> Score::scoreList()
      {
      QList<Score*> scores;
      Score* root = masterScore();
      scores.append(root);
      for (const Excerpt* ex : root->excerpts()) {
            if (ex->partScore())
                  scores.append(ex->partScore());
            }
      return scores;
      }


bool Score::switchLayer(const QString& s)
      {
      int layerIdx = 0;
      for (const Layer& l : layer()) {
            if (s == l.name) {
                  setCurrentLayer(layerIdx);
                  return true;
                  }
            ++layerIdx;
            }
      return false;
      }


void Score::appendPart(const QString& name)
      {
      static InstrumentTemplate defaultInstrument;
      InstrumentTemplate* t;

      t = searchTemplate(name);
      if (t == 0) {
            qDebug("appendPart: <%s> not found", qPrintable(name));
            t = &defaultInstrument;
            }

      if (t->channel.empty()) {
            Channel a;
            a.chorus = 0;
            a.reverb = 0;
            a.name   = "normal";
            a.bank   = 0;
            a.volume = 100;
            a.pan    = 64;             t->channel.append(a);
            }
      Part* part = new Part(this);
      part->initFromInstrTemplate(t);
      int n = nstaves();
      for (int i = 0; i < t->nstaves(); ++i) {
            Staff* staff = new Staff(this);
            staff->setPart(part);
            staff->setLines(0, t->staffLines[i]);
            staff->setSmall(0, t->smallStaff[i]);
            if (i == 0) {
                  staff->setBracketType(0, t->bracket[0]);
                  staff->setBracketSpan(0, t->nstaves());
                  }
            undoInsertStaff(staff, i);
            }
      part->staves()->front()->setBarLineSpan(part->nstaves());
      undoInsertPart(part, n);
      fixTicks();
      masterScore()->rebuildMidiMapping();
      }


void Score::appendMeasures(int n)
      {
      for (int i = 0; i < n; ++i)
            insertMeasure(ElementType::MEASURE, 0, false);
      }

#ifdef SCRIPT_INTERFACE

void Score::addText(const QString& type, const QString& txt)
      {
      MeasureBase* measure = first();
      if (measure == 0 || measure->type() != ElementType::VBOX) {
            insertMeasure(ElementType::VBOX, measure);
            measure = first();
            }
      SubStyleId stid = SubStyleId::DEFAULT;
      if (type == "title")
            stid = SubStyleId::TITLE;
      else if (type == "subtitle")
            stid = SubStyleId::SUBTITLE;
      Text* text = new Text(stid, this);
      text->setParent(measure);
      text->setXmlText(txt);
      undoAddElement(text);
      }


Cursor* Score::newCursor()
      {
      return new Cursor(this);
      }
#endif


void Score::addSpanner(Spanner* s)
      {
      _spanner.addSpanner(s);
      }


void Score::removeSpanner(Spanner* s)
      {
      _spanner.removeSpanner(s);
      }


bool Score::isSpannerStartEnd(int tick, int track) const
      {
      for (auto i : _spanner.map()) {
            if (i.second->track() != track)
                  continue;
            if (i.second->tick() == tick || i.second->tick2() == tick)
                  return true;
            }
      return false;
      }

void Score::insertTime(int tick, int len)
      {
      for (Staff* staff : staves())
            staff->insertTime(tick, len);
      for (Part* part : parts())
            part->insertTime(tick, len);
      }


void Score::addUnmanagedSpanner(Spanner* s)
      {
      _unmanagedSpanner.insert(s);
      }


void Score::removeUnmanagedSpanner(Spanner* s)
      {
      _unmanagedSpanner.erase(s);
      }


void Score::setPos(POS pos, int tick)
      {
      if (tick < 0)
            tick = 0;
      if (tick != _pos[int(pos)])
            _pos[int(pos)] = tick;
                        emit posChanged(pos, unsigned(tick));
      }


QList<int> Score::uniqueStaves() const
      {
      QList<int> sl;

      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            Staff* s = staff(staffIdx);
            if (s->links()) {
                  bool alreadyInList = false;
                  for (int idx : sl) {
                        if (s->links()->contains(staff(idx))) {
                              alreadyInList = true;
                              break;
                              }
                        }
                  if (alreadyInList)
                        continue;
                  }
            sl.append(staffIdx);
            }
      return sl;
      }


ChordRest* Score::findCR(int tick, int track) const
      {
      Measure* m = tick2measureMM(tick);
      if (!m) {
            qDebug("findCR: no measure for tick %d", tick);
            return nullptr;
            }
            if (m->isMMRest())
            tick = m->tick();
      Segment* s = m->first(SegmentType::ChordRest);
      for (Segment* ns = s; ; ns = ns->next(SegmentType::ChordRest)) {
            if (ns == 0 || ns->tick() > tick)
                  break;
            Element* el = ns->element(track);
            if (el && el->isRest() && toRest(el)->isGap())
                  continue;
            else if (el)
                  s = ns;
            }
      Element* el = s->element(track);
      if (el && el->isRest() && toRest(el)->isGap())
            s = 0;
      if (s)
            return toChordRest(s->element(track));
      return nullptr;
      }


ChordRest* Score::findCRinStaff(int tick, int staffIdx) const
      {
      int ptick = tick - 1;
      Measure* m = tick2measureMM(ptick);
      if (!m) {
            qDebug("findCRinStaff: no measure for tick %d", ptick);
            return nullptr;
            }
            if (m->isMMRest())
            ptick = m->tick();
      Segment* s = m->first(SegmentType::ChordRest);
      int strack = staffIdx * VOICES;
      int etrack = strack + VOICES;
      int actualTrack = strack;

      int lastTick = -1;
      for (Segment* ns = s; ; ns = ns->next(SegmentType::ChordRest)) {
            if (ns == 0 || ns->tick() > ptick)
                  break;
                        for (int t = strack; t < etrack; ++t) {
                  ChordRest* cr = toChordRest(ns->element(t));
                  if (cr) {
                        int endTick = cr->tick() + cr->actualTicks();
                                                                        int fudge = cr->tuplet() ? 5 : 0;
                        if (endTick + fudge >= lastTick && endTick - fudge <= tick) {
                              s = ns;
                              actualTrack = t;
                              lastTick = endTick;
                              }
                        }
                  }
            }
      if (s)
            return toChordRest(s->element(actualTrack));
      return nullptr;
      }


void MasterScore::setSoloMute()
      {
      for (int i = 0; i < _midiMapping.size(); i++) {
            Channel* b = _midiMapping[i].articulation;
            if (b->solo) {
                  b->soloMute = false;
                  for (int j = 0; j < _midiMapping.size(); j++) {
                        Channel* a = _midiMapping[j].articulation;
                        bool sameMidiMapping = _midiMapping[i].port == _midiMapping[j].port && _midiMapping[i].channel == _midiMapping[j].channel;
                        a->soloMute = (i != j && !a->solo && !sameMidiMapping);
                        a->solo     = (i == j || a->solo || sameMidiMapping);
                        }
                  }
            }
      }


void Score::setImportedFilePath(const QString& filePath)
      {
      _importedFilePath = filePath;
      }


int Score::nmeasures()
      {
      int n = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure())
            n++;
      return n;
      }


bool Score::hasLyrics()
      {
      SegmentType st = SegmentType::ChordRest;
      for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            for (int i = 0; i < ntracks(); ++i) {
                  ChordRest* cr = toChordRest(seg->element(i));
                  if (cr && !cr->lyrics().empty())
                        return true;
                  }
            }
      return false;
      }


bool Score::hasHarmonies()
      {
      SegmentType st = SegmentType::ChordRest;
      for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            for (Element* e : seg->annotations()) {
                  if (e->type() == ElementType::HARMONY)
                        return true;
                  }
            }
      return false;
      }


int Score::lyricCount()
      {
      int count = 0;
      SegmentType st = SegmentType::ChordRest;
      for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            for (int i = 0; i < ntracks(); ++i) {
                  ChordRest* cr = toChordRest(seg->element(i));
                  if (cr)
                        count += cr->lyrics().size();
                  }
            }
      return count;
      }


int Score::harmonyCount()
      {
      int count = 0;
      SegmentType st = SegmentType::ChordRest;
      for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            for (Element* e : seg->annotations()) {
                  if (e->type() == ElementType::HARMONY)
                        count++;
                  }
            }
      return count;
      }


QString Score::extractLyrics()
      {
      QString result;
      updateRepeatList(true);
      setPlaylistDirty();
      SegmentType st = SegmentType::ChordRest;
      for (int track = 0; track < ntracks(); track += VOICES) {
            bool found = false;
            unsigned maxLyrics = 1;
            for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
                  m->setPlaybackCount(0);
                  }
                        for (const RepeatSegment* rs : *repeatList()) {
                  int startTick  = rs->tick;
                  int endTick    = startTick + rs->len();
                  for (Measure* m = tick2measure(startTick); m; m = m->nextMeasure()) {
                        int playCount = m->playbackCount();
                        for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
                                                            ChordRest* cr = toChordRest(seg->element(track));
                              if (!cr || cr->lyrics().empty())
                                    continue;
                              if (cr->lyrics().size() > maxLyrics)
                                    maxLyrics = cr->lyrics().size();
                              if (playCount >= int(cr->lyrics().size()))
                                    continue;
                              Lyrics* l = cr->lyrics(playCount, Placement::BELOW);                                if (!l)
                                    continue;
                              found = true;
                              QString lyric = l->plainText().trimmed();
                              if (l->syllabic() == Lyrics::Syllabic::SINGLE || l->syllabic() == Lyrics::Syllabic::END)
                                    result += lyric + " ";
                              else if (l->syllabic() == Lyrics::Syllabic::BEGIN || l->syllabic() == Lyrics::Syllabic::MIDDLE)
                                    result += lyric;
                              }
                        m->setPlaybackCount(m->playbackCount() + 1);
                        if (m->tick() + m->ticks() >= endTick)
                              break;
                        }
                  }
                        for (unsigned lyricsNumber = 0; lyricsNumber < maxLyrics; lyricsNumber++) {
                  for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
                        unsigned playCount = m->playbackCount();
                        if (lyricsNumber >= playCount) {
                              for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
                                                                        ChordRest* cr = toChordRest(seg->element(track));
                                    if (!cr || cr->lyrics().empty())
                                          continue;
                                    if (cr->lyrics().size() > maxLyrics)
                                          maxLyrics = cr->lyrics().size();
                                    if (lyricsNumber >= cr->lyrics().size())
                                          continue;
                                    Lyrics* l = cr->lyrics(lyricsNumber, Placement::BELOW);                                      if (!l)
                                          continue;
                                    found = true;
                                    QString lyric = l->plainText().trimmed();
                                    if (l->syllabic() == Lyrics::Syllabic::SINGLE || l->syllabic() == Lyrics::Syllabic::END)
                                          result += lyric + " ";
                                    else if (l->syllabic() == Lyrics::Syllabic::BEGIN || l->syllabic() == Lyrics:: Syllabic::MIDDLE)
                                          result += lyric;
                                    }
                              }
                        }
                  }
            if (found)
                  result += "\n\n";
            }
      return result.trimmed();
      }


int Score::keysig()
      {
      Key result = Key::C;
      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            Staff* st = staff(staffIdx);
            Key key = st->key(0);
            if (st->staffType(0)->group() == StaffGroup::PERCUSSION || st->keySigEvent(0).custom() || st->keySigEvent(0).isAtonal())                         continue;
            result = key;
            int diff = st->part()->instrument()->transpose().chromatic;
            if (!styleB(Sid::concertPitch) && diff)
                  result = transposeKey(key, diff);
            break;
            }
      return int(result);
      }


int Score::duration()
      {
      updateRepeatList(true);
      RepeatSegment* rs = repeatList()->last();
      return lrint(utick2utime(rs->utick + rs->len()));
      }


QString Score::createRehearsalMarkText(RehearsalMark* current) const
      {
      int tick = current->segment()->tick();
      RehearsalMark* before = 0;
      RehearsalMark* after = 0;
      for (Segment* s = firstSegment(SegmentType::All); s; s = s->next1()) {
            for (Element* e : s->annotations()) {
                  if (e && e->type() == ElementType::REHEARSAL_MARK) {
                        if (s->tick() < tick)
                              before = toRehearsalMark(e);
                        else if (s->tick() > tick) {
                              after = toRehearsalMark(e);
                              break;
                              }
                        }
                  }
            if (after)
                  break;
            }
      QString s = "A";
      QString s1 = before ? before->xmlText() : "";
      QString s2 = after ? after->xmlText()  : "";
      if (s1.isEmpty())
            return s;
      s = nextRehearsalMarkText(before, current);           if (s == current->xmlText()) {
                        return s;
            }
      else if (s == s2) {
                        if (s1[0].isLetter()) {
                  if (s1.size() == 2)
                        s = s1[0] + QChar::fromLatin1(s1[1].toLatin1() + 1);                    else
                        s = s1 + QChar::fromLatin1('1');                                        }
            else {
                  s = s1 + QChar::fromLatin1('A');                                              }
            }
      return s;
      }


QString Score::nextRehearsalMarkText(RehearsalMark* previous, RehearsalMark* current) const
      {
      QString previousText = previous->xmlText();
      QString fallback = current ? current->xmlText() : previousText + "'";

      if (previousText.length() == 1 && previousText[0].isLetter()) {
                        if (previousText == "Z")
                  return "AA";
            else if (previousText == "z")
                  return "aa";
            else
                  return QChar::fromLatin1(previousText[0].toLatin1() + 1);
            }
      else if (previousText.length() == 2 && previousText[0].isLetter() && previousText[1].isLetter()) {
                        if (previousText[0] == previousText[1]) {
                                    if (previousText.toUpper() != "ZZ") {
                        QString c = QChar::fromLatin1(previousText[0].toLatin1() + 1);
                        return c + c;
                        }
                  else {
                        return fallback;
                        }
                  }
            else {
                  return fallback;
                  }
            }
      else {
                        bool ok;
            int n = previousText.toInt(&ok);
            if (!ok) {
                  return fallback;
                  }
            else if (current && n == previous->segment()->measure()->no() + 1) {
                                    n = current->segment()->measure()->no() + 1;
                  return QString("%1").arg(n);
                  }
            else {
                                    n = previousText.toInt() + 1;
                  return QString("%1").arg(n);
                  }
            }
      }


void Score::changeVoice(int voice)
      {
      startCmd();
      QList<Element*> el;
      QList<Element*> oel = selection().elements();           for (Element* e : oel) {
            if (e->type() == ElementType::NOTE) {
                  Note* note   = toNote(e);
                  Chord* chord = note->chord();

                                    if (note->tieFor() || note->tieBack())
                        continue;

                                    if (chord->isGrace())
                        continue;

                  if (chord->voice() != voice) {
                        Segment* s       = chord->segment();
                        Measure* m       = s->measure();
                        int notes        = chord->notes().size();
                        int dstTrack     = chord->staffIdx() * VOICES + voice;
                        ChordRest* dstCR = toChordRest(s->element(dstTrack));
                        Chord* dstChord  = nullptr;

                        if (excerpt() && excerpt()->tracks().key(dstTrack, -1) == -1)
                              break;

                        
                        if (dstCR && dstCR->type() == ElementType::CHORD && dstCR->globalDuration() == chord->globalDuration()) {
                                                                                          dstChord = toChord(dstCR);
                              }

                        else if (dstCR && dstCR->type() == ElementType::REST && dstCR->globalDuration() == chord->globalDuration()) {
                                                                                                                        dstChord = new Chord(this);
                              dstChord->setTrack(dstTrack);
                              dstChord->setDurationType(chord->durationType());
                              dstChord->setDuration(chord->duration());
                              dstChord->setTuplet(dstCR->tuplet());
                              dstChord->setParent(s);
                              undoRemoveElement(dstCR);
                              }

                        else if (!chord->tuplet()) {
                                                                                                                        ChordRest* pcr = nullptr;
                              ChordRest* ncr = nullptr;
                              for (Segment* s2 = m->first(SegmentType::ChordRest); s2; s2 = s2->next()) {
                                    if (s2->segmentType() != SegmentType::ChordRest)
                                          continue;
                                    ChordRest* cr2 = toChordRest(s2->element(dstTrack));
                                    if (!cr2 || cr2->type() == ElementType::REST)
                                          continue;
                                    if (s2->tick() < s->tick()) {
                                          pcr = cr2;
                                          continue;
                                          }
                                    else if (s2->tick() >= s->tick()) {
                                          ncr = cr2;
                                          break;
                                          }
                                    }
                              int gapStart = pcr ? pcr->tick() + pcr->actualTicks() : m->tick();
                              int gapEnd = ncr ? ncr->tick() : m->tick() + m->ticks();
                              if (gapStart <= s->tick() && gapEnd >= s->tick() + chord->actualTicks()) {
                                                                        dstChord = new Chord(this);
                                    dstChord->setTrack(dstTrack);
                                    dstChord->setDurationType(chord->durationType());
                                    dstChord->setDuration(chord->duration());
                                    dstChord->setParent(s);
                                                                        if (voice && !dstCR)
                                          expandVoice(s,  dstTrack);
                                    makeGapVoice(s, dstTrack, chord->actualFraction(), s->tick());
                                    }
                              }

                                                if (dstChord) {
                                                            Note* newNote = new Note(*note);
                              newNote->setSelected(false);
                              newNote->setParent(dstChord);
                              undoAddElement(newNote);
                              el.append(newNote);
                                                            if (dstChord != dstCR)
                                    undoAddCR(dstChord, m, s->tick());
                                                            if (notes > 1) {
                                    undoRemoveElement(note);
                                    }
                              else if (notes == 1) {
                                                                        Rest* r = new Rest(this);
                                    r->setTrack(chord->track());
                                    r->setDurationType(chord->durationType());
                                    r->setDuration(chord->duration());
                                    r->setTuplet(chord->tuplet());
                                    r->setParent(s);
                                                                        for (Chord* gc : chord->graceNotes()) {
                                          Chord* ngc = new Chord(*gc);
                                          undoRemoveElement(gc);
                                          ngc->setParent(dstChord);
                                          ngc->setTrack(dstChord->track());
                                          undoAddElement(ngc);
                                          }
                                                                        undoRemoveElement(chord);
                                    undoAddCR(r, m, s->tick());
                                    }
                              }
                        }
                  }
            }

      if (!el.empty())
            selection().clear();
      for (Element* e : el)
            select(e, SelectType::ADD, -1);
      setLayoutAll();
      endCmd();
      }

#if 0

void Score::cropPage(qreal margins)
      {
      if (npages() == 1) {
            Page* page = pages()[0];
            if (page) {
                  QRectF ttbox = page->tbbox();

                  qreal margin = margins / INCH;
                  f.setSize(QSizeF((ttbox.width() / DPI) + 2 * margin, (ttbox.height()/ DPI) + 2 * margin));

                  qreal offset = curFormat->oddLeftMargin() - ttbox.x() / DPI;
                  if (offset < 0)
                        offset = 0.0;
                  f.setOddLeftMargin(margin + offset);
                  f.setEvenLeftMargin(margin + offset);
                  f.setOddBottomMargin(margin);
                  f.setOddTopMargin(margin);
                  f.setEvenBottomMargin(margin);
                  f.setEvenTopMargin(margin);

                  undoChangePageFormat(&f, spatium(), pageNumberOffset());
                  }
            }
      }
#endif


QVariant Score::getProperty(Pid id) const
      {
      switch (id) {
            default:
                  qDebug("Score::getProperty: unhandled id");
                  return QVariant();
            }
      }


bool Score::setProperty(Pid id, const QVariant& )
      {
      switch (id) {
            default:
                  qDebug("Score::setProperty: unhandled id");
                  break;
            }
      setLayoutAll();
      return true;
      }


QVariant Score::propertyDefault(Pid id) const
      {
      switch (id) {
            default:
                  return QVariant();
            }
      }


void Score::setStyle(const MStyle& s)
      {
      style() = s;
      }


MasterScore::MasterScore()
   : Score()
      {
      _tempomap    = new TempoMap;
      _sigmap      = new TimeSigMap();
      _repeatList  = new RepeatList(this);
      _revisions   = new Revisions;
      setMasterScore(this);

#if defined(Q_OS_WIN)
      metaTags().insert("platform", "Microsoft Windows");
#elif defined(Q_OS_MAC)
      metaTags().insert("platform", "Apple Macintosh");
#elif defined(Q_OS_LINUX)
      metaTags().insert("platform", "Linux");
#else
      metaTags().insert("platform", "Unknown");
#endif
      metaTags().insert("movementNumber", "");
      metaTags().insert("movementTitle", "");
      metaTags().insert("workNumber", "");
      metaTags().insert("workTitle", "");
      metaTags().insert("arranger", "");
      metaTags().insert("composer", "");
      metaTags().insert("lyricist", "");
      metaTags().insert("poet", "");
      metaTags().insert("translator", "");
      metaTags().insert("source", "");
      metaTags().insert("copyright", "");
      metaTags().insert("creationDate", QDate::currentDate().toString(Qt::ISODate));
      }

MasterScore::MasterScore(const MStyle& s)
   : MasterScore{}
      {
      _movements = new Movements;
      _movements->push_back(this);
      setStyle(s);
      }

MasterScore::~MasterScore()
      {
      delete _revisions;
      delete _repeatList;
      delete _sigmap;
      delete _tempomap;
      qDeleteAll(_excerpts);
      }


void MasterScore::setMovements(Movements* m)
      {
      _movements = m;
      if (_movements)
            _movements->push_back(this);
      }


bool MasterScore::isSavable() const
      {
            return fileInfo()->isWritable() || !fileInfo()->exists();
      }


void MasterScore::setTempomap(TempoMap* tm)
      {
      delete _tempomap;
      _tempomap = tm;
      }


void MasterScore::removeOmr()
      {
      _showOmr = false;
#ifdef OMR
      delete _omr;
#endif
      _omr = 0;
      }


void MasterScore::setName(const QString& ss)
      {
      QString s(ss);
      s.replace('/', '_');          if (!(s.endsWith(".mscz") || s.endsWith(".mscx")))
            s += ".mscz";
      info.setFile(s);
      }


QString MasterScore::title() const
      {
      return fileInfo()->completeBaseName();
      }

QString Score::title() const
      {
      return _excerpt->title();
      }


void Score::addRefresh(const QRectF& r)
      {
      _updateState.refresh |= r;
      cmdState().setUpdateMode(UpdateMode::Update);
      }


int Score::staffIdx(const Part* part) const
      {
      int idx = 0;
      for (Part* p : _parts) {
            if (p == part)
                  break;
            idx += p->nstaves();
            }
      return idx;
      }


void MasterScore::setUpdateAll()
      {
      _cmdState.setUpdateMode(UpdateMode::UpdateAll);
      }


void MasterScore::setLayoutAll()
      {
      _cmdState.setTick(0);
      _cmdState.setTick(measures()->last() ? measures()->last()->endTick() : 0);
      }


void MasterScore::setLayout(int t)
      {
      if (t >= 0)
            _cmdState.setTick(t);
      }


bool Score::isTopScore() const
      {
      return !(isMaster() && static_cast<const MasterScore*>(this)->prev());
      }


Movements::Movements()
   : std::vector<MasterScore*>()
      {
      _undo = new UndoStack();
      }

Movements::~Movements()
      {
      qDeleteAll(_pages);
      delete _undo;
      }

}

