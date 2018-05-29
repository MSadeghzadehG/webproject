





#include <math.h>
#include "config.h"
#include "file.h"
#include "libmscore/score.h"
#include "libmscore/rest.h"
#include "libmscore/chord.h"
#include "libmscore/sig.h"
#include "libmscore/key.h"
#include "libmscore/clef.h"
#include "libmscore/note.h"
#include "libmscore/segment.h"
#include "libmscore/xml.h"
#include "libmscore/beam.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/measure.h"
#include "libmscore/style.h"
#include "musicxml.h"
#include "libmscore/slur.h"
#include "libmscore/hairpin.h"
#include "libmscore/dynamic.h"
#include "libmscore/barline.h"
#include "libmscore/timesig.h"
#include "libmscore/ottava.h"
#include "libmscore/pedal.h"
#include "libmscore/text.h"
#include "libmscore/tuplet.h"
#include "libmscore/lyrics.h"
#include "libmscore/volta.h"
#include "libmscore/keysig.h"
#include "libmscore/bracket.h"
#include "libmscore/arpeggio.h"
#include "libmscore/jump.h"
#include "libmscore/marker.h"
#include "libmscore/tremolo.h"
#include "libmscore/trill.h"
#include "libmscore/harmony.h"
#include "libmscore/tempotext.h"
#include "libmscore/sym.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/utils.h"
#include "libmscore/articulation.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/element.h"
#include "libmscore/glissando.h"
#include "libmscore/navigate.h"
#include "libmscore/spanner.h"
#include "libmscore/drumset.h"
#include "preferences.h"
#include "libmscore/mscore.h"
#include "libmscore/accidental.h"
#include "libmscore/breath.h"
#include "libmscore/chordline.h"
#include "libmscore/figuredbass.h"
#include "libmscore/stringdata.h"
#include "libmscore/rehearsalmark.h"
#include "thirdparty/qzip/qzipwriter_p.h"
#include "libmscore/fret.h"
#include "libmscore/tie.h"
#include "libmscore/undo.h"
#include "libmscore/textline.h"
#include "libmscore/fermata.h"
#include "musicxmlfonthandler.h"

namespace Ms {



#ifdef DEBUG_CLEF
#define clefDebug(...) qDebug(__VA_ARGS__)
#else
#define clefDebug(...) {}
#endif


typedef QMap<int, const FiguredBass*> FigBassMap;


class Attributes {
      bool inAttributes;

public:
      Attributes() { start(); }
      void doAttr(XmlWriter& xml, bool attr);
      void start();
      void stop(XmlWriter& xml);
      };


void Attributes::doAttr(XmlWriter& xml, bool attr)
      {
      if (!inAttributes && attr) {
            xml.stag("attributes");
            inAttributes = true;
            }
      else if (inAttributes && !attr) {
            xml.etag();
            inAttributes = false;
            }
      }


void Attributes::start()
      {
      inAttributes = false;
      }


void Attributes::stop(XmlWriter& xml)
      {
      if (inAttributes) {
            xml.etag();
            inAttributes = false;
            }
      }


class Notations {
      bool notationsPrinted;

public:
      Notations() { notationsPrinted = false; }
      void tag(XmlWriter& xml);
      void etag(XmlWriter& xml);
      };


class Articulations {
      bool articulationsPrinted;

public:
      Articulations() { articulationsPrinted = false; }
      void tag(XmlWriter& xml);
      void etag(XmlWriter& xml);
      };


class Ornaments {
      bool ornamentsPrinted;

public:
      Ornaments() { ornamentsPrinted = false; }
      void tag(XmlWriter& xml);
      void etag(XmlWriter& xml);
      };


class Technical {
      bool technicalPrinted;

public:
      Technical() { technicalPrinted = false; }
      void tag(XmlWriter& xml);
      void etag(XmlWriter& xml);
      };


class SlurHandler {
      const Slur* slur[MAX_NUMBER_LEVEL];
      bool started[MAX_NUMBER_LEVEL];
      int findSlur(const Slur* s) const;

public:
      SlurHandler();
      void doSlurs(Chord* chord, Notations& notations, XmlWriter& xml);

private:
      void doSlurStart(const Slur* s, Notations& notations, XmlWriter& xml);
      void doSlurStop(const Slur* s, Notations& notations, XmlWriter& xml);
      };


class GlissandoHandler {
      const Note* glissNote[MAX_NUMBER_LEVEL];
      const Note* slideNote[MAX_NUMBER_LEVEL];
      int findNote(const Note* note, int type) const;

public:
      GlissandoHandler();
      void doGlissandoStart(Glissando* gliss, Notations& notations, XmlWriter& xml);
      void doGlissandoStop(Glissando* gliss, Notations& notations, XmlWriter& xml);
      };


typedef QHash<const Chord*, const Trill*> TrillHash;
typedef QMap<const Instrument*, int> MxmlInstrumentMap;

class ExportMusicXml {
      Score* _score;
      XmlWriter xml;
      SlurHandler sh;
      GlissandoHandler gh;
      int tick;
      Attributes attr;
      TextLine const* brackets[MAX_NUMBER_LEVEL];
      Hairpin const* hairpins[MAX_NUMBER_LEVEL];
      Ottava const* ottavas[MAX_NUMBER_LEVEL];
      Trill const* trills[MAX_NUMBER_LEVEL];
      int div;
      double millimeters;
      int tenths;
      TrillHash trillStart;
      TrillHash trillStop;
      MxmlInstrumentMap instrMap;

      int findHairpin(const Hairpin* tl) const;
      int findBracket(const TextLine* tl) const;
      int findOttava(const Ottava* tl) const;
      int findTrill(const Trill* tl) const;
      void chord(Chord* chord, int staff, const std::vector<Lyrics*>* ll, bool useDrumset);
      void rest(Rest* chord, int staff);
      void clef(int staff, const Clef* clef);
      void timesig(TimeSig* tsig);
      void keysig(const KeySig* ks, ClefType ct, int staff = 0, bool visible = true);
      void barlineLeft(Measure* m);
      void barlineRight(Measure* m);
      void lyrics(const std::vector<Lyrics*>* ll, const int trk);
      void work(const MeasureBase* measure);
      void calcDivMoveToTick(int t);
      void calcDivisions();
      void keysigTimesig(const Measure* m, const Part* p);
      void chordAttributes(Chord* chord, Notations& notations, Technical& technical,
                           TrillHash& trillStart, TrillHash& trillStop);
      void wavyLineStartStop(Chord* chord, Notations& notations, Ornaments& ornaments,
                             TrillHash& trillStart, TrillHash& trillStop);
      void print(Measure* m, int idx, int staffCount, int staves);
      void findAndExportClef(Measure* m, const int staves, const int strack, const int etrack);
      void writeElement(Element* el, const Measure* m, int sstaff, bool useDrumset);

public:
      ExportMusicXml(Score* s)
            : xml(s)
            {
            _score = s; tick = 0; div = 1; tenths = 40;
            millimeters = _score->spatium() * tenths / (10 * DPMM);
            }
      void write(QIODevice* dev);
      void credits(XmlWriter& xml);
      void moveToTick(int t);
      void words(Text const* const text, int staff);
      void rehearsal(RehearsalMark const* const rmk, int staff);
      void hairpin(Hairpin const* const hp, int staff, int tick);
      void ottava(Ottava const* const ot, int staff, int tick);
      void pedal(Pedal const* const pd, int staff, int tick);
      void textLine(TextLine const* const tl, int staff, int tick);
      void dynamic(Dynamic const* const dyn, int staff);
      void symbol(Symbol const* const sym, int staff);
      void tempoText(TempoText const* const text, int staff);
      void harmony(Harmony const* const, FretDiagram const* const fd, int offset = 0);
      Score* score() const { return _score; };
      double getTenthsFromInches(double) const;
      double getTenthsFromDots(double) const;
      };


static QString addPositioningAttributes(Element const* const el, bool isSpanStart = true)
      {
      if (!preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT))
            return "";

            
      const float positionElipson = 0.1f;
      float defaultX = 0;
      float defaultY = 0;
      float relativeX = 0;
      float relativeY = 0;
      float spatium = el->spatium();

      const SLine* span = nullptr;
      if (el->isSLine())
            span = static_cast<const SLine*>(el);

      if (span && !span->spannerSegments().isEmpty()) {
            if (isSpanStart) {
                  const auto seg = span->spannerSegments().first();
                  const auto userOff = seg->userOff();
                  const auto p = seg->pos();
                  relativeX = userOff.x();
                  defaultY = p.y();

                                    
                  }
            else {
                  const auto seg = span->spannerSegments().last();
                  const auto userOff = seg->userOff();                   const auto userOff2 = seg->userOff2();                                     
                                    
                                                      relativeX = userOff.x() + userOff2.x();

                                                      }
            }
      else {
            defaultX = el->ipos().x();               defaultY = el->ipos().y();
            relativeX = el->userOff().x();
            relativeY = el->userOff().y();
            }

            defaultX *=  10 / spatium;
      defaultY *=  -10 / spatium;
      relativeX *=  10 / spatium;
      relativeY *=  -10 / spatium;

      QString res;
      if (fabsf(defaultX) > positionElipson)
            res += QString(" default-x=\"%1\"").arg(QString::number(defaultX, 'f', 2));
      if (fabsf(defaultY) > positionElipson)
            res += QString(" default-y=\"%1\"").arg(QString::number(defaultY, 'f', 2));
      if (fabsf(relativeX) > positionElipson)
            res += QString(" relative-x=\"%1\"").arg(QString::number(relativeX, 'f', 2));
      if (fabsf(relativeY) > positionElipson)
            res += QString(" relative-y=\"%1\"").arg(QString::number(relativeY, 'f', 2));

      return res;
      }


void Notations::tag(XmlWriter& xml)
      {
      if (!notationsPrinted)
            xml.stag("notations");
      notationsPrinted = true;
      }


void Notations::etag(XmlWriter& xml)
      {
      if (notationsPrinted)
            xml.etag();
      notationsPrinted = false;
      }


void Articulations::tag(XmlWriter& xml)
      {
      if (!articulationsPrinted)
            xml.stag("articulations");
      articulationsPrinted = true;
      }


void Articulations::etag(XmlWriter& xml)
      {
      if (articulationsPrinted)
            xml.etag();
      articulationsPrinted = false;
      }


void Ornaments::tag(XmlWriter& xml)
      {
      if (!ornamentsPrinted)
            xml.stag("ornaments");
      ornamentsPrinted = true;
      }


void Ornaments::etag(XmlWriter& xml)
      {
      if (ornamentsPrinted)
            xml.etag();
      ornamentsPrinted = false;
      }


void Technical::tag(XmlWriter& xml)
      {
      if (!technicalPrinted)
            xml.stag("technical");
      technicalPrinted = true;
      }


void Technical::etag(XmlWriter& xml)
      {
      if (technicalPrinted)
            xml.etag();
      technicalPrinted = false;
      }




static QString color2xml(const Element* el)
      {
      if (el->color() != MScore::defaultColor)
            return QString(" color=\"%1\"").arg(el->color().name().toUpper());
      else
            return "";
      }


SlurHandler::SlurHandler()
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            slur[i] = 0;
            started[i] = false;
            }
      }

static QString slurTieLineStyle(const SlurTie* s)
      {
      QString lineType;
      QString rest;
      switch (s->lineType()) {
            case 1:
                  lineType = "dotted";
                  break;
            case 2:
                  lineType = "dashed";
                  break;
            default:
                  lineType = "";
            }
      if (!lineType.isEmpty())
            rest = QString(" line-type=\"%1\"").arg(lineType);
      return rest;
      }


int SlurHandler::findSlur(const Slur* s) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (slur[i] == s) return i;
      return -1;
      }


static const Chord* findFirstChord(const Slur* s)
      {
      const Element* e1 = s->startElement();
      if (e1 == 0 || e1->type() != ElementType::CHORD) {
            qDebug("no valid start chord for slur %p", s);
            return 0;
            }

      const Element* e2 = s->endElement();
      if (e2 == 0 || e2->type() != ElementType::CHORD) {
            qDebug("no valid stop chord for slur %p", s);
            return 0;
            }

      const Chord* c1 = static_cast<const Chord*>(e1);
      const Chord* c2 = static_cast<const Chord*>(e2);

      if (c1->tick() < c2->tick())
            return c1;
      else if (c1->tick() > c2->tick())
            return c2;
      else {
                        if (!c1->isGrace() && !c2->isGrace()) {
                                                      qDebug("invalid slur between chords %p and %p at tick %d", c1, c2, c1->tick());
                  return 0;
                  }
            else if (c1->isGraceBefore() && !c2->isGraceBefore())
                  return c1;              else if (c1->isGraceAfter() && !c2->isGraceAfter())
                  return c2;              else if (c2->isGraceBefore() && !c1->isGraceBefore())
                  return c2;              else if (c2->isGraceAfter() && !c1->isGraceAfter())
                  return c1;              else {
                                                      if ((c1->isGraceBefore() && c1->graceIndex() < c2->graceIndex())
                      || (c1->isGraceAfter() && c1->graceIndex() > c2->graceIndex()))
                        return c1;
                  else
                        return c2;
                  }
            }

            return 0;
      }


void SlurHandler::doSlurs(Chord* chord, Notations& notations, XmlWriter& xml)
      {
            for (int i = 0; i < 2; ++i) {
                        for (auto it : chord->score()->spanner()) {
                  Spanner* sp = it.second;
                  if (sp->generated() || sp->type() != ElementType::SLUR)
                        continue;
                  if (chord == sp->startElement() || chord == sp->endElement()) {
                        const Slur* s = static_cast<const Slur*>(sp);
                        const Chord* firstChord = findFirstChord(s);
                        if (firstChord) {
                              if (i == 0) {
                                                                        if (firstChord != chord)
                                          doSlurStop(s, notations, xml);
                                    }
                              else {
                                                                        if (firstChord == chord)
                                          doSlurStart(s, notations, xml);
                                    }
                              }
                        }
                  }
            }
      }


void SlurHandler::doSlurStart(const Slur* s, Notations& notations, XmlWriter& xml)
      {
            int i = findSlur(s);
            QString tagName = "slur";
      tagName += slurTieLineStyle(s);       tagName += color2xml(s);
      tagName += QString(" type=\"start\"%1")
            .arg(s->slurDirection() == Direction::UP ? " placement=\"above\"" : "");
      tagName += addPositioningAttributes(s, true);

      if (i >= 0) {
                        slur[i] = 0;
            started[i] = false;
            notations.tag(xml);
            tagName += QString(" number=\"%1\"").arg(i + 1);
            xml.tagE(tagName);
            }
      else {
                        i = findSlur(0);
            if (i >= 0) {
                  slur[i] = s;
                  started[i] = true;
                  notations.tag(xml);
                  tagName += QString(" number=\"%1\"").arg(i + 1);
                  xml.tagE(tagName);
                  }
            else
                  qDebug("no free slur slot");
            }
      }



void SlurHandler::doSlurStop(const Slur* s, Notations& notations, XmlWriter& xml)
      {
            int i = findSlur(s);
      if (i < 0) {
                        i = findSlur(0);
            if (i >= 0) {
                  slur[i] = s;
                  started[i] = false;
                  notations.tag(xml);
                  QString tagName = QString("slur type=\"stop\" number=\"%1\"").arg(i + 1);
                  tagName += addPositioningAttributes(s, false);
                  xml.tagE(tagName);
                  }
            else
                  qDebug("no free slur slot");
            }
      else {
                        slur[i] = 0;
            started[i] = false;
            notations.tag(xml);
            QString tagName = QString("slur type=\"stop\" number=\"%1\"").arg(i + 1);
            tagName += addPositioningAttributes(s, false);
            xml.tagE(tagName);
            }
      }




static void glissando(const Glissando* gli, int number, bool start, Notations& notations, XmlWriter& xml)
      {
      GlissandoType st = gli->glissandoType();
      QString tagName;
      switch (st) {
            case GlissandoType::STRAIGHT:
                  tagName = "slide line-type=\"solid\"";
                  break;
            case GlissandoType::WAVY:
                  tagName = "glissando line-type=\"wavy\"";
                  break;
            default:
                  qDebug("unknown glissando subtype %d", int(st));
                  return;
                  break;
            }
      tagName += QString(" number=\"%1\" type=\"%2\"").arg(number).arg(start ? "start" : "stop");
      tagName += color2xml(gli);
      tagName += addPositioningAttributes(gli, start);
      notations.tag(xml);
      if (start && gli->showText() && gli->text() != "")
            xml.tag(tagName, gli->text());
      else
            xml.tagE(tagName);
      }


GlissandoHandler::GlissandoHandler()
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            glissNote[i] = 0;
            slideNote[i] = 0;
            }
      }


int GlissandoHandler::findNote(const Note* note, int type) const
      {
      if (type != 0 && type != 1) {
            qDebug("GlissandoHandler::findNote: unknown glissando subtype %d", type);
            return -1;
            }
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (type == 0 && slideNote[i] == note) return i;
            if (type == 1 && glissNote[i] == note) return i;
            }
      return -1;
      }


void GlissandoHandler::doGlissandoStart(Glissando* gliss, Notations& notations, XmlWriter& xml)
      {
      GlissandoType type = gliss->glissandoType();
      if (type != GlissandoType::STRAIGHT && type != GlissandoType::WAVY) {
            qDebug("doGlissandoStart: unknown glissando subtype %d", int(type));
            return;
            }
      Note* note = static_cast<Note*>(gliss->startElement());
            int i = findNote(note, int(type));
      if (i >= 0) {
                        qDebug("doGlissandoStart: note for glissando/slide %p already on list", gliss);
            if (type == GlissandoType::STRAIGHT) slideNote[i] = 0;
            if (type == GlissandoType::WAVY) glissNote[i] = 0;
            }
            i = findNote(0, int(type));
      if (i >= 0) {
            if (type == GlissandoType::STRAIGHT) slideNote[i] = note;
            if (type == GlissandoType::WAVY) glissNote[i] = note;
            glissando(gliss, i + 1, true, notations, xml);
            }
      else
            qDebug("doGlissandoStart: no free slot");
      }


void GlissandoHandler::doGlissandoStop(Glissando* gliss, Notations& notations, XmlWriter& xml)
      {
      GlissandoType type = gliss->glissandoType();
      if (type != GlissandoType::STRAIGHT && type != GlissandoType::WAVY) {
            qDebug("doGlissandoStart: unknown glissando subtype %d", int(type));
            return;
            }
      Note* note = static_cast<Note*>(gliss->startElement());
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (type == GlissandoType::STRAIGHT && slideNote[i] == note) {
                  slideNote[i] = 0;
                  glissando(gliss, i + 1, false, notations, xml);
                  return;
                  }
            if (type == GlissandoType::WAVY && glissNote[i] == note) {
                  glissNote[i] = 0;
                  glissando(gliss, i + 1, false, notations, xml);
                  return;
                  }
            }
      qDebug("doGlissandoStop: glissando note %p not found", note);
      }


class DirectionsAnchor {
      Element* direct;              Element* anchor;              bool start;                   int tick;               
public:
      DirectionsAnchor(Element* a, bool s, int t) { direct = 0; anchor = a; start = s; tick = t; }
      DirectionsAnchor(int t) { direct = 0; anchor = 0; start = true; tick = t; }
      Element* getDirect() { return direct; }
      Element* getAnchor() { return anchor; }
      bool getStart() { return start; }
      int getTick() { return tick; }
      void setDirect(Element* d) { direct = d; }
      };



static void findTrillAnchors(const Trill* trill, Chord*& startChord, Chord*& stopChord)
      {
      const Segment* seg = trill->startSegment();
      const int endTick  = trill->tick2();
      const int strack   = trill->track();
                  for (int i = 0; i < VOICES; ++i) {
            Element* el = seg->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != ElementType::CHORD)
                  continue;
            startChord = static_cast<Chord*>(el);
            Segment* s = trill->score()->tick2segmentEnd(strack + i, endTick);
            if (!s)
                  continue;
            el = s->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != ElementType::CHORD)
                  continue;
            stopChord = static_cast<Chord*>(el);
            return;
            }
            for (int i = 0; i < VOICES; ++i) {
            Element* el = seg->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != ElementType::CHORD)
                  continue;
            startChord = static_cast<Chord*>(el);
            break;                  }
      for (int i = 0; i < VOICES; ++i) {
            Segment* s = trill->score()->tick2segmentEnd(strack + i, endTick);
            if (!s)
                  continue;
            Element* el = s->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != ElementType::CHORD)
                  continue;
            stopChord = static_cast<Chord*>(el);
            break;                  }
      }


static void findTrills(Measure* measure, int strack, int etrack, TrillHash& trillStart, TrillHash& trillStop)
      {
            int stick = measure->tick();
      int etick = measure->tick() + measure->ticks();
      for (auto it = measure->score()->spanner().lower_bound(stick); it != measure->score()->spanner().upper_bound(etick); ++it) {
            Spanner* e = it->second;
                        if (e->type() == ElementType::TRILL && strack <= e->track() && e->track() < etrack
                && e->tick() >= measure->tick() && e->tick() < (measure->tick() + measure->ticks()))
                  {
                                                                        const Trill* tr = static_cast<const Trill*>(e);
                  Chord* startChord = 0;                    Chord* stopChord = 0;   
                  findTrillAnchors(tr, startChord, stopChord);
                  
                  if (startChord && stopChord) {
                        trillStart.insert(startChord, tr);
                        trillStop.insert(stopChord, tr);
                        }
                  }
            }
      }


typedef QList<int> IntVector;
static IntVector integers;
static IntVector primes;


static bool canDivideBy(int d)
      {
      bool res = true;
      for (int i = 0; i < integers.count(); i++) {
            if ((integers[i] <= 1) || ((integers[i] % d) != 0)) {
                  res = false;
                  }
            }
      return res;
      }


static void divideBy(int d)
      {
      for (int i = 0; i < integers.count(); i++) {
            integers[i] /= d;
            }
      }

static void addInteger(int len)
      {
      if (!integers.contains(len)) {
            integers.append(len);
            }
      }


void ExportMusicXml::calcDivMoveToTick(int t)
      {
      if (t < tick) {
#ifdef DEBUG_TICK
            qDebug("backup %d", tick - t);
#endif
            addInteger(tick - t);
            }
      else if (t > tick) {
#ifdef DEBUG_TICK
            qDebug("forward %d", t - tick);
#endif
            addInteger(t - tick);
            }
      tick = t;
      }


static bool isTwoNoteTremolo(Chord* chord)
      {
      return (chord->tremolo() && chord->tremolo()->twoNotes());
      }






void ExportMusicXml::calcDivisions()
      {
            integers.clear();
      primes.clear();
      integers.append(MScore::division);
      primes.append(2);
      primes.append(3);
      primes.append(5);

      const QList<Part*>& il = _score->parts();

      for (int idx = 0; idx < il.size(); ++idx) {

            Part* part = il.at(idx);
            tick = 0;

            int staves = part->nstaves();
            int strack = _score->staffIdx(part) * VOICES;
            int etrack = strack + staves * VOICES;

            for (MeasureBase* mb = _score->measures()->first(); mb; mb = mb->next()) {

                  if (mb->type() != ElementType::MEASURE)
                        continue;
                  Measure* m = (Measure*)mb;

                  for (int st = strack; st < etrack; ++st) {
                                                                                                
                        int sstaff = (staves > 1) ? st - strack + VOICES : 0;
                        sstaff /= VOICES;

                        for (Segment* seg = m->first(); seg; seg = seg->next()) {

                              Element* el = seg->element(st);
                              if (!el)
                                    continue;

                                                            if (el->type() == ElementType::BAR_LINE && static_cast<BarLine*>(el)->barLineType() == BarLineType::START_REPEAT)
                                    continue;

                              if (tick != seg->tick())
                                    calcDivMoveToTick(seg->tick());

                              if (el->isChordRest()) {
                                    int l = static_cast<ChordRest*>(el)->actualTicks();
                                    if (el->type() == ElementType::CHORD) {
                                          if (isTwoNoteTremolo(static_cast<Chord*>(el)))
                                                l /= 2;
                                          }
#ifdef DEBUG_TICK
                                    qDebug("chordrest %d", l);
#endif
                                    addInteger(l);
                                    tick += l;
                                    }
                              }
                        }
                                    calcDivMoveToTick(m->tick() + m->ticks());
                  }
            }

            for (int u = 0; u < primes.count(); u++)
            while (canDivideBy(primes[u]))
                  divideBy(primes[u]);

      div = MScore::division / integers[0];
#ifdef DEBUG_TICK
      qDebug("divisions=%d div=%d", integers[0], div);
#endif
      }

#if 0

static void writePageFormat(const PageFormat* pf, XmlWriter& xml, double conversion)
      {
      xml.stag("page-layout");

      xml.tag("page-height", pf->size().height() * conversion);
      xml.tag("page-width", pf->size().width() * conversion);
      QString type("both");
      if (pf->twosided()) {
            type = "even";
            xml.stag(QString("page-margins type=\"%1\"").arg(type));
            xml.tag("left-margin",   pf->evenLeftMargin() * conversion);
            xml.tag("right-margin",  pf->evenRightMargin() * conversion);
            xml.tag("top-margin",    pf->evenTopMargin() * conversion);
            xml.tag("bottom-margin", pf->evenBottomMargin() * conversion);
            xml.etag();
            type = "odd";
            }
      xml.stag(QString("page-margins type=\"%1\"").arg(type));
      xml.tag("left-margin",   pf->oddLeftMargin() * conversion);
      xml.tag("right-margin",  pf->oddRightMargin() * conversion);
      xml.tag("top-margin",    pf->oddTopMargin() * conversion);
      xml.tag("bottom-margin", pf->oddBottomMargin() * conversion);
      xml.etag();

      xml.etag();
      }
#endif



static void defaults(XmlWriter& xml, Score* s, double& millimeters, const int& tenths)
      {
      xml.stag("defaults");
      xml.stag("scaling");
      xml.tag("millimeters", millimeters);
      xml.tag("tenths", tenths);
      xml.etag();

            
                        
            xml.tagE(QString("word-font font-family=\"%1\" font-size=\"%2\"").arg(s->styleSt(Sid::staffTextFontFace)).arg(s->styleD(Sid::staffTextFontSize)));
      xml.tagE(QString("lyric-font font-family=\"%1\" font-size=\"%2\"").arg(s->styleSt(Sid::lyricsOddFontFace)).arg(s->styleD(Sid::lyricsOddFontSize)));
      xml.etag();
      }



static void creditWords(XmlWriter& xml, Score* s, double x, double y, QString just, QString val, const QList<TextFragment>& words)
      {
      const QString mtf = s->styleSt(Sid::MusicalTextFont);
      CharFormat defFmt;
      defFmt.setFontFamily(s->styleSt(Sid::staffTextFontFace));
      defFmt.setFontSize(s->styleD(Sid::staffTextFontSize));

            xml.stag("credit page=\"1\"");
      QString attr = QString(" default-x=\"%1\"").arg(x);
      attr += QString(" default-y=\"%1\"").arg(y);
      attr += " justify=\"" + just + "\"";
      attr += " valign=\"" + val + "\"";
      MScoreTextToMXML mttm("credit-words", attr, defFmt, mtf);
      mttm.writeTextFragments(words, xml);
      xml.etag();
      }


static double parentHeight(const Element* element)
      {
      const Element* parent = element->parent();

      if (!parent)
            return 0;

      if (parent->type() == ElementType::VBOX) {
            return parent->height();
            }

      return 0;
      }


void ExportMusicXml::credits(XmlWriter& xml)
      {
      const MeasureBase* measure = _score->measures()->first();
      QString rights = _score->metaTag("copyright");

            const double h  = getTenthsFromInches(_score->styleD(Sid::pageHeight));
      const double w  = getTenthsFromInches(_score->styleD(Sid::pageWidth));
      const double lm = getTenthsFromInches(_score->styleD(Sid::pageOddLeftMargin));
      const double rm = getTenthsFromInches(_score->styleD(Sid::pagePrintableWidth) - _score->styleD(Sid::pageOddLeftMargin));
            const double bm = getTenthsFromInches(_score->styleD(Sid::pageOddBottomMargin));
      
            if (measure) {
            for (const Element* element : measure->el()) {
                  if (element->type() == ElementType::TEXT) {
                        const Text* text = (const Text*)element;
                        const double ph = getTenthsFromDots(parentHeight(text));

                        double tx = w / 2;
                        double ty = h - getTenthsFromDots(text->pagePos().y());

                        Align al = text->align();
                        QString just;
                        QString val;

                        if (al & Align::RIGHT) {
                              just = "right";
                              tx   = w - rm;
                              }
                        else if (al & Align::HCENTER) {
                              just = "center";
                                                            }
                        else {
                              just = "left";
                              tx   = lm;
                              }

                        if (al & Align::BOTTOM) {
                              val = "bottom";
                              ty -= ph;
                              }
                        else if (al & Align::VCENTER) {
                              val = "middle";
                              ty -= ph / 2;
                              }
                        else if (al & Align::BASELINE) {
                              val = "baseline";
                              ty -= ph / 2;
                              }
                        else {
                              val = "top";
                                                            }

                        creditWords(xml, _score, tx, ty, just, val, text->fragmentList());
                        }
                  }
            }

      if (!rights.isEmpty()) {
                                    TextFragment f(XmlWriter::xmlString(rights));
            f.changeFormat(FormatId::FontFamily, _score->styleSt(Sid::footerFontFace));
            f.changeFormat(FormatId::FontSize, _score->styleD(Sid::footerFontSize));
            QList<TextFragment> list;
            list.append(f);
            creditWords(xml, _score, w / 2, bm, "center", "bottom", list);
            }
      }


static int alterTab[12] = { 0,   1,   0,   1,   0,  0,   1,   0,   1,   0,   1,   0 };
static char noteTab[12] = { 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B' };

static void midipitch2xml(int pitch, char& c, int& alter, int& octave)
      {
            c      = noteTab[pitch % 12];
      alter  = alterTab[pitch % 12];
      octave = pitch / 12 - 1;
            }


static void tabpitch2xml(const int pitch, const int tpc, QString& s, int& alter, int& octave)
      {
      s      = tpc2stepName(tpc);
      alter  = tpc2alterByKey(tpc, Key::C);
      octave = (pitch - alter) / 12 - 1;
      if (alter < -2 || 2 < alter)
            qDebug("tabpitch2xml(pitch %d, tpc %d) problem:  step %s, alter %d, octave %d",
                   pitch, tpc, qPrintable(s), alter, octave);
      
      }



static void pitch2xml(const Note* note, QString& s, int& alter, int& octave)
      {

      const Staff* st = note->staff();
      const Instrument* instr = st->part()->instrument();         const Interval intval = instr->transpose();

      s      = tpc2stepName(note->tpc());
      alter  = tpc2alterByKey(note->tpc(), Key::C);
                  octave = (note->pitch() - intval.chromatic - alter) / 12 - 1;

                                          int tick        = note->chord()->tick();
      ClefType ct     = st->clef(tick);
      if (ct == ClefType::PERC || ct == ClefType::PERC2) {
            alter = 0;
            octave = line2pitch(note->line(), ct, Key::C) / 12 - 1;
            }

            int ottava = 0;
      switch (note->ppitch() - note->pitch()) {
            case  24: ottava =  2; break;
            case  12: ottava =  1; break;
            case   0: ottava =  0; break;
            case -12: ottava = -1; break;
            case -24: ottava = -2; break;
            default:  qDebug("pitch2xml() tick=%d pitch()=%d ppitch()=%d",
                             tick, note->pitch(), note->ppitch());
            }
      octave += ottava;

                  }


static void unpitch2xml(const Note* note, QString& s, int& octave)
      {
      static char table1[]  = "FEDCBAG";

      int tick        = note->chord()->tick();
      Staff* st       = note->staff();
      ClefType ct     = st->clef(tick);
            int clefOffset  = ClefInfo::pitchOffset(ct) - ClefInfo::pitchOffset(ClefType::G);
                  int line5g      = note->line() - clefOffset;
                                                      if (st->lines(0) == 1) line5g += 8;
      if (st->lines(0) == 3) line5g += 2;
            int stepIdx     = (line5g + 700) % 7;
            s               = table1[stepIdx];
                  octave =(3 - line5g + 700) / 7 + 5 - 100;
                  }


static QString tick2xml(const int ticks, int* dots)
      {
      TDuration t;
      t.setVal(ticks);
      *dots = t.dots();
      return t.name();
      }


static Volta* findVolta(Measure* m, bool left)
      {
      int stick = m->tick();
      int etick = m->tick() + m->ticks();
      auto spanners = m->score()->spannerMap().findOverlapping(stick, etick);
      for (auto i : spanners) {
            Spanner* el = i.value;
            if (el->type() != ElementType::VOLTA)
                  continue;
            if (left && el->tick() == stick)
                  return (Volta*) el;
            if (!left && el->tick2() == etick)
                  return (Volta*) el;
            }
      return 0;
      }


static void ending(XmlWriter& xml, Volta* v, bool left)
      {
      QString number = "";
      QString type = "";
      for (int i : v->endings()) {
            if (!number.isEmpty())
                  number += ", ";
            number += QString("%1").arg(i);
            }
      if (left) {
            type = "start";
            }
      else {
            Volta::Type st = v->voltaType();
            switch (st) {
                  case Volta::Type::OPEN:
                        type = "discontinue";
                        break;
                  case Volta::Type::CLOSED:
                        type = "stop";
                        break;
                  default:
                        qDebug("unknown volta subtype %d", int(st));
                        type = "unknown";
                        break;
                  }
            }
      QString voltaXml = QString("ending number=\"%1\" type=\"%2\"").arg(number).arg(type);
      voltaXml += addPositioningAttributes(v, left);
      xml.tagE(voltaXml);
      }


void ExportMusicXml::barlineLeft(Measure* m)
      {
      bool rs = m->repeatStart();
      Volta* volta = findVolta(m, true);
      if (!rs && !volta) return;
      attr.doAttr(xml, false);
      xml.stag(QString("barline location=\"left\""));
      if (rs)
            xml.tag("bar-style", QString("heavy-light"));
      if (volta)
            ending(xml, volta, true);
      if (rs)
            xml.tagE("repeat direction=\"forward\"");
      xml.etag();
      }


void ExportMusicXml::barlineRight(Measure* m)
      {
      const Measure* mmR1 = m->mmRest1();       const Measure* mmRLst = mmR1->isMMRest() ? mmR1->mmRestLast() : 0;             BarLineType bst = m == mmRLst ? mmR1->endBarLineType() : m->endBarLineType();
      bool visible = m->endBarLineVisible();

      bool needBarStyle = (bst != BarLineType::NORMAL && bst != BarLineType::START_REPEAT) || !visible;
      Volta* volta = findVolta(m, false);
            QString special = "";
      if (bst == BarLineType::NORMAL) {
            const BarLine* bl = m->endBarLine();
            if (bl && !bl->spanStaff()) {
                  if (bl->spanFrom() == BARLINE_SPAN_TICK1_FROM && bl->spanTo() == BARLINE_SPAN_TICK1_TO)
                        special = "tick";
                  if (bl->spanFrom() == BARLINE_SPAN_TICK2_FROM && bl->spanTo() == BARLINE_SPAN_TICK2_TO)
                        special = "tick";
                  if (bl->spanFrom() == BARLINE_SPAN_SHORT1_FROM && bl->spanTo() == BARLINE_SPAN_SHORT1_TO)
                        special = "short";
                  if (bl->spanFrom() == BARLINE_SPAN_SHORT2_FROM && bl->spanTo() == BARLINE_SPAN_SHORT2_FROM)
                        special = "short";
                  }
            }
      if (!needBarStyle && !volta && special.isEmpty())
            return;
      xml.stag(QString("barline location=\"right\""));
      if (needBarStyle) {
            if (!visible) {
                  xml.tag("bar-style", QString("none"));
                  }
            else {
                  switch (bst) {
                        case BarLineType::DOUBLE:
                              xml.tag("bar-style", QString("light-light"));
                              break;
                        case BarLineType::END_REPEAT:
                              xml.tag("bar-style", QString("light-heavy"));
                              break;
                        case BarLineType::BROKEN:
                              xml.tag("bar-style", QString("dashed"));
                              break;
                        case BarLineType::DOTTED:
                              xml.tag("bar-style", QString("dotted"));
                              break;
                        case BarLineType::END:
                              xml.tag("bar-style", QString("light-heavy"));
                              break;
                        default:
                              qDebug("ExportMusicXml::bar(): bar subtype %d not supported", int(bst));
                              break;
                        }
                  }
            }
      else if (!special.isEmpty()) {
            xml.tag("bar-style", special);
            }
      if (volta)
            ending(xml, volta, false);
      if (bst == BarLineType::END_REPEAT)
            {
            if (m->repeatCount() > 2) {
                  xml.tagE(QString("repeat direction=\"backward\" times=\"%1\"").arg(m->repeatCount()));
                  } else {
                  xml.tagE("repeat direction=\"backward\"");
                  }
            }
      xml.etag();
      }


void ExportMusicXml::moveToTick(int t)
      {
            if (t < tick) {
#ifdef DEBUG_TICK
            qDebug(" -> backup");
#endif
            attr.doAttr(xml, false);
            xml.stag("backup");
            xml.tag("duration", (tick - t) / div);
            xml.etag();
            }
      else if (t > tick) {
#ifdef DEBUG_TICK
            qDebug(" -> forward");
#endif
            attr.doAttr(xml, false);
            xml.stag("forward");
            xml.tag("duration", (t - tick) / div);
            xml.etag();
            }
      tick = t;
      }


void ExportMusicXml::timesig(TimeSig* tsig)
      {
      TimeSigType st = tsig->timeSigType();
      Fraction ts = tsig->sig();
      int z = ts.numerator();
      int n = ts.denominator();
      QString ns = tsig->numeratorString();

      attr.doAttr(xml, true);
      QString tagName = "time";
      if (st == TimeSigType::FOUR_FOUR)
            tagName += " symbol=\"common\"";
      else if (st == TimeSigType::ALLA_BREVE)
            tagName += " symbol=\"cut\"";
      if (!tsig->visible())
            tagName += " print-object=\"no\"";
      tagName += color2xml(tsig);
      xml.stag(tagName);

      QRegExp rx("^\\d+(\\+\\d+)+$");       if (rx.exactMatch(ns))
                        xml.tag("beats", ns);
      else
                        xml.tag("beats", z);
      xml.tag("beat-type", n);
      xml.etag();
      }


static double accSymId2alter(SymId id)
      {
      double res = 0;
      switch (id) {
            case SymId::accidentalDoubleFlat:                      res = -2;   break;
            case SymId::accidentalThreeQuarterTonesFlatZimmermann: res = -1.5; break;
            case SymId::accidentalFlat:                            res = -1;   break;
            case SymId::accidentalQuarterToneFlatStein:            res = -0.5; break;
            case SymId::accidentalNatural:                         res =  0;   break;
            case SymId::accidentalQuarterToneSharpStein:           res =  0.5; break;
            case SymId::accidentalSharp:                           res =  1;   break;
            case SymId::accidentalThreeQuarterTonesSharpStein:     res =  1.5; break;
            case SymId::accidentalDoubleSharp:                     res =  2;   break;
            default: qDebug("accSymId2alter: unsupported sym %s", Sym::id2name(id));
            }
      return res;
      }


void ExportMusicXml::keysig(const KeySig* ks, ClefType ct, int staff, bool visible)
      {
      static char table2[]  = "CDEFGAB";
      int po = ClefInfo::pitchOffset(ct);             
      QString tagName = "key";
      if (staff)
            tagName += QString(" number=\"%1\"").arg(staff);
      if (!visible)
            tagName += " print-object=\"no\"";
      tagName += color2xml(ks);
      attr.doAttr(xml, true);
      xml.stag(tagName);

      const KeySigEvent kse = ks->keySigEvent();
      const QList<KeySym> keysyms = kse.keySymbols();
      if (kse.custom() && !kse.isAtonal() && keysyms.size() > 0) {

                                    
                        QMap<qreal, KeySym> map;
            for (const KeySym& ksym : keysyms) {
                  map.insert(ksym.spos.x(), ksym);
                  }
                        for (const KeySym& ksym : map) {
                  int line = static_cast<int>(round(2 * ksym.spos.y()));
                  int step = (po - line) % 7;
                                                      xml.tag("key-step", QString(QChar(table2[step])));
                  xml.tag("key-alter", accSymId2alter(ksym.sym));
                  xml.tag("key-accidental", accSymId2MxmlString(ksym.sym));
                  }
            }
      else {
                        xml.tag("fifths", static_cast<int>(kse.key()));
            switch (kse.mode()) {
                  case KeyMode::NONE:     xml.tag("mode", "none"); break;
                  case KeyMode::MAJOR:    xml.tag("mode", "major"); break;
                  case KeyMode::MINOR:    xml.tag("mode", "minor"); break;
                  case KeyMode::UNKNOWN:
                  default:
                        if (kse.custom())
                              xml.tag("mode", "none");
                  }
            }
      xml.etag();
      }


void ExportMusicXml::clef(int staff, const Clef* clef)
      {
      ClefType ct = clef->clefType();
      clefDebug("ExportMusicXml::clef(staff %d, clef %hhd)", staff, ct);

      QString tagName = "clef";
      if (staff)
            tagName += QString(" number=\"%1\"").arg(staff);
      tagName += color2xml(clef);
      attr.doAttr(xml, true);
      xml.stag(tagName);

      QString sign = ClefInfo::sign(ct);
      int line   = ClefInfo::line(ct);
      xml.tag("sign", sign);
      xml.tag("line", line);
      if (ClefInfo::octChng(ct))
            xml.tag("clef-octave-change", ClefInfo::octChng(ct));
      xml.etag();
      }



static void tupletStartStop(ChordRest* cr, Notations& notations, XmlWriter& xml)
      {
      Tuplet* t = cr->tuplet();
      if (!t) return;
      if (cr == t->elements().front()) {
            notations.tag(xml);
            QString tupletTag = "tuplet type=\"start\"";
            tupletTag += " bracket=";
            tupletTag += t->hasBracket() ? "\"yes\"" : "\"no\"";
            if (t->numberType() == TupletNumberType::SHOW_RELATION)
                  tupletTag += " show-number=\"both\"";
            if (t->numberType() == TupletNumberType::NO_TEXT)
                  tupletTag += " show-number=\"none\"";
            xml.tagE(tupletTag);
            }
      if (cr == t->elements().back()) {
            notations.tag(xml);
            xml.tagE("tuplet type=\"stop\"");
            }
      }


int ExportMusicXml::findTrill(const Trill* tr) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (trills[i] == tr) return i;
      return -1;
      }


void ExportMusicXml::wavyLineStartStop(Chord* chord, Notations& notations, Ornaments& ornaments,
                                       TrillHash& trillStart, TrillHash& trillStop)
      {
      if (trillStop.contains(chord)) {
            const Trill* tr = trillStop.value(chord);
            int n = findTrill(tr);
            if (n >= 0)
                                    trills[n] = 0;
            else {
                                    n = findTrill(0);
                  if (n >= 0)
                        trills[n] = tr;
                  else
                        qDebug("too many overlapping trills (chord %p staff %d tick %d)",
                               chord, chord->staffIdx(), chord->tick());
                  }
            if (n >= 0) {
                  notations.tag(xml);
                  ornaments.tag(xml);
                  QString trillXml = QString("wavy-line type=\"stop\" number=\"%1\"").arg(n + 1);
                  trillXml += addPositioningAttributes(tr, false);
                  xml.tagE(trillXml);
                  }
            trillStop.remove(chord);
            }
      if (trillStart.contains(chord)) {
            const Trill* tr = trillStart.value(chord);
            int n = findTrill(tr);
            if (n >= 0)
                  qDebug("wavyLineStartStop error");
            else {
                  n = findTrill(0);
                  if (n >= 0) {
                        trills[n] = tr;
                                                notations.tag(xml);
                        ornaments.tag(xml);
                        xml.tagE("trill-mark");
                        QString tagName = "wavy-line type=\"start\"";
                        tagName += QString(" number=\"%1\"").arg(n + 1);
                        tagName += color2xml(tr);
                        tagName += addPositioningAttributes(tr, true);
                        xml.tagE(tagName);
                        }
                  else
                        qDebug("too many overlapping trills (chord %p staff %d tick %d)",
                               chord, chord->staffIdx(), chord->tick());
                  trillStart.remove(chord);
                  }
            }
      }


static Breath* hasBreathMark(Chord* ch)
      {
      int tick = ch->tick() + ch->actualTicks();
      Segment* s = ch->measure()->findSegment(SegmentType::Breath, tick);
      return s ? static_cast<Breath*>(s->element(ch->track())) : 0;
      }


static void tremoloSingleStartStop(Chord* chord, Notations& notations, Ornaments& ornaments, XmlWriter& xml)
      {
      if (chord->tremolo()) {
            Tremolo* tr = chord->tremolo();
            int count = 0;
            TremoloType st = tr->tremoloType();
            QString type = "";

            if (chord->tremoloChordType() == TremoloChordType::TremoloSingle) {
                  type = "single";
                  switch (st) {
                        case TremoloType::R8:  count = 1; break;
                        case TremoloType::R16: count = 2; break;
                        case TremoloType::R32: count = 3; break;
                        case TremoloType::R64: count = 4; break;
                        default: qDebug("unknown tremolo single %d", int(st)); break;
                        }
                  }
            else if (chord->tremoloChordType() == TremoloChordType::TremoloFirstNote) {
                  type = "start";
                  switch (st) {
                        case TremoloType::C8:  count = 1; break;
                        case TremoloType::C16: count = 2; break;
                        case TremoloType::C32: count = 3; break;
                        case TremoloType::C64: count = 4; break;
                        default: qDebug("unknown tremolo double %d", int(st)); break;
                        }
                  }
            else if (chord->tremoloChordType() == TremoloChordType::TremoloSecondNote) {
                  type = "stop";
                  switch (st) {
                        case TremoloType::C8:  count = 1; break;
                        case TremoloType::C16: count = 2; break;
                        case TremoloType::C32: count = 3; break;
                        case TremoloType::C64: count = 4; break;
                        default: qDebug("unknown tremolo double %d", int(st)); break;
                        }
                  }
            else qDebug("unknown tremolo subtype %d", int(st));


            if (type != "" && count > 0) {
                  notations.tag(xml);
                  ornaments.tag(xml);
                  QString tagName = "tremolo";
                  tagName += QString(" type=\"%1\"").arg(type);
                  if (type == "single" || type == "start")
                        tagName += color2xml(tr);
                  xml.tag(tagName, count);
                  }
            }
      }



static void fermatas(const QVector<Element*>& cra, XmlWriter& xml, Notations& notations)
      {
      for (const Element* e : cra) {
            if (!e->isFermata())
                  continue;
            const Fermata* a = toFermata(e);
            notations.tag(xml);
            QString tagName = "fermata";
            tagName += QString(" type=\"%1\"").arg(a->placement() == Placement::ABOVE ? "upright" : "inverted");
            tagName += color2xml(a);
            SymId id = a->symId();
            if (id == SymId::fermataAbove || id == SymId::fermataBelow)
                  xml.tagE(tagName);
                                    else if (id == SymId::fermataShortAbove || id == SymId::fermataShortBelow
                     || id == SymId::fermataShortHenzeAbove || id == SymId::fermataShortHenzeBelow
                     || id == SymId::fermataVeryShortAbove || id == SymId::fermataVeryShortBelow)
                  xml.tag(tagName, "angled");
                                    else if (id == SymId::fermataLongAbove || id == SymId::fermataLongBelow
                     || id == SymId::fermataLongHenzeAbove || id == SymId::fermataLongHenzeBelow
                     || id == SymId::fermataVeryLongAbove || id == SymId::fermataVeryLongBelow)
                  xml.tag(tagName, "square");
            }
      }


static QString symIdToArtic(const SymId sid)
      {
      switch (sid) {
            case SymId::articAccentAbove:
            case SymId::articAccentBelow:
                  return "accent";
                  break;

            case SymId::articStaccatoAbove:
            case SymId::articStaccatoBelow:
            case SymId::articAccentStaccatoAbove:
            case SymId::articAccentStaccatoBelow:
            case SymId::articMarcatoStaccatoAbove:
            case SymId::articMarcatoStaccatoBelow:
                  return "staccato";
                  break;

            case SymId::articStaccatissimoAbove:
            case SymId::articStaccatissimoBelow:
            case SymId::articStaccatissimoStrokeAbove:
            case SymId::articStaccatissimoStrokeBelow:
            case SymId::articStaccatissimoWedgeAbove:
            case SymId::articStaccatissimoWedgeBelow:
                  return "staccatissimo";
                  break;

            case SymId::articTenutoAbove:
            case SymId::articTenutoBelow:
                  return "tenuto";
                  break;

            case SymId::articMarcatoAbove:
            case SymId::articMarcatoBelow:
                  return "strong-accent";
                  break;

            case SymId::articTenutoStaccatoAbove:
            case SymId::articTenutoStaccatoBelow:
                  return "detached-legato";
                  break;

            default:
                  ;                         break;
            }

      return "";
      }


static QString symIdToOrnam(const SymId sid)
      {
      switch (sid) {
            case SymId::ornamentTurnInverted:
                  return "inverted-turn";
                  break;
            case SymId::ornamentTurn:
                  return "turn";
                  break;
            case SymId::ornamentTrill:
                  return "trill-mark";
                  break;
            case SymId::ornamentMordentInverted:
                  return "mordent";
                                    break;
            case SymId::ornamentMordent:
                                    return "inverted-mordent";
                  break;
            case SymId::ornamentTremblement:
                  return "inverted-mordent long=\"yes\"";
                  break;
            case SymId::ornamentPrallMordent:
                  return "mordent long=\"yes\"";
                  break;
            case SymId::ornamentUpPrall:
                  return "inverted-mordent long=\"yes\" approach=\"below\"";
                  break;
            case SymId::ornamentPrecompMordentUpperPrefix:
                  return "inverted-mordent long=\"yes\" approach=\"above\"";
                  break;
            case SymId::ornamentUpMordent:
                  return "mordent long=\"yes\" approach=\"below\"";
                  break;
            case SymId::ornamentDownMordent:
                  return "mordent long=\"yes\" approach=\"above\"";
                  break;
            case SymId::ornamentPrallDown:
                  return "inverted-mordent long=\"yes\" departure=\"below\"";
                  break;
            case SymId::ornamentPrallUp:
                  return "inverted-mordent long=\"yes\" departure=\"above\"";
                  break;
            case SymId::ornamentLinePrall:
                                    return "inverted-mordent long=\"yes\" approach=\"above\"";
                  break;
            case SymId::ornamentPrecompSlide:
                  return "schleifer";
                  break;

            default:
                  ;                   break;
            }

      return "";
      }


static QString symIdToTechn(const SymId sid)
      {
      switch (sid) {
            case SymId::brassMuteClosed:
                  return "stopped";
                  break;
            case SymId::stringsHarmonic:
                  return "x";                   break;
            case SymId::stringsUpBow:
                  return "up-bow";
                  break;
            case SymId::stringsDownBow:
                  return "down-bow";
                  break;
            case SymId::pluckedSnapPizzicatoAbove:
                  return "snap-pizzicato";
                  break;
            case SymId::brassMuteOpen:
                  return "open-string";
                  break;
            case SymId::stringsThumbPosition:
                  return "thumb-position";
                  break;
            default:
                  ;                   break;
            }

      return "";
      }


static void writeChordLines(const Chord* const chord, XmlWriter& xml, Notations& notations, Articulations& articulations)
      {
      for (Element* e : chord->el()) {
            qDebug("chordAttributes: el %p type %d (%s)", e, int(e->type()), e->name());
            if (e->type() == ElementType::CHORDLINE) {
                  ChordLine const* const cl = static_cast<ChordLine*>(e);
                  QString subtype;
                  switch (cl->chordLineType()) {
                        case ChordLineType::FALL:
                              subtype = "falloff";
                              break;
                        case ChordLineType::DOIT:
                              subtype = "doit";
                              break;
                        case ChordLineType::PLOP:
                              subtype = "plop";
                              break;
                        case ChordLineType::SCOOP:
                              subtype = "scoop";
                              break;
                        default:
                              qDebug("unknown ChordLine subtype %d", int(cl->chordLineType()));
                        }
                  if (subtype != "") {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE(subtype);
                        }
                  }
            }
      }


void ExportMusicXml::chordAttributes(Chord* chord, Notations& notations, Technical& technical,
                                     TrillHash& trillStart, TrillHash& trillStop)
      {
      QVector<Element*> fl;
      for (Element* e : chord->segment()->annotations()) {
            if (e->track() == chord->track() && e->isFermata())
                  fl.push_back(e);
            }
      fermatas(fl, xml, notations);

      const QVector<Articulation*> na = chord->articulations();
            Articulations articulations;
      for (const Articulation* a : na) {
            auto sid = a->symId();
            auto mxmlArtic = symIdToArtic(sid);

            if (mxmlArtic != "") {
                  if (sid == SymId::articMarcatoAbove || sid == SymId::articMarcatoBelow) {
                        if (a->up())
                              mxmlArtic += " type=\"up\"";
                        else
                              mxmlArtic += " type=\"down\"";
                        }

                  notations.tag(xml);
                  articulations.tag(xml);
                  xml.tagE(mxmlArtic);
                  }
            }

      if (Breath* b = hasBreathMark(chord)) {
            notations.tag(xml);
            articulations.tag(xml);
            xml.tagE(b->isCaesura() ? "caesura" : "breath-mark");
            }

      writeChordLines(chord, xml, notations, articulations);

      articulations.etag(xml);

            Ornaments ornaments;
      for (const Articulation* a : na) {
            auto sid = a->symId();
            auto mxmlOrnam = symIdToOrnam(sid);

            if (mxmlOrnam != "") {
                  notations.tag(xml);
                  ornaments.tag(xml);
                  xml.tagE(mxmlOrnam);
                  }
            }

      tremoloSingleStartStop(chord, notations, ornaments, xml);
      wavyLineStartStop(chord, notations, ornaments, trillStart, trillStop);
      ornaments.etag(xml);

            for (const Articulation* a : na) {
            auto sid = a->symId();

            if (sid == SymId::stringsHarmonic) {
                  notations.tag(xml);
                  technical.tag(xml);
                  xml.stag("harmonic");
                  xml.tagE("natural");
                  xml.etag();
                  }
            else {
                  auto mxmlTechn = symIdToTechn(sid);
                  if (mxmlTechn != "") {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tagE(mxmlTechn);
                        }
                  }
            }

            for (const Articulation* a : na) {
            auto sid = a->symId();
            if (symIdToArtic(sid) == ""
                && symIdToOrnam(sid) == ""
                && symIdToTechn(sid) == "") {
                  qDebug("unknown chord attribute %s", qPrintable(a->userName()));
                  }
            }
      }



static void arpeggiate(Arpeggio* arp, bool front, bool back, XmlWriter& xml, Notations& notations)
      {
      QString tagName = "";
      switch (arp->arpeggioType()) {
            case ArpeggioType::NORMAL:
                  notations.tag(xml);
                  tagName = "arpeggiate";
                  break;
            case ArpeggioType::UP:                      case ArpeggioType::UP_STRAIGHT:                   notations.tag(xml);
                  tagName = "arpeggiate direction=\"up\"";
                  break;
            case ArpeggioType::DOWN:                      case ArpeggioType::DOWN_STRAIGHT:                   notations.tag(xml);
                  tagName = "arpeggiate direction=\"down\"";
                  break;
            case ArpeggioType::BRACKET:
                  if (front) {
                        notations.tag(xml);
                        tagName = "non-arpeggiate type=\"bottom\"";
                        }
                  if (back) {
                        notations.tag(xml);
                        tagName = "non-arpeggiate type=\"top\"";
                        }
                  break;
            default:
                  qDebug("unknown arpeggio subtype %d", int(arp->arpeggioType()));
                  break;
            }

      tagName += addPositioningAttributes(arp);
      if (tagName != "")
            xml.tagE(tagName);
      }




static int determineTupletNormalTicks(ChordRest const* const chord)
      {
      Tuplet const* const t = chord->tuplet();
      if (!t)
            return 0;
      
      for (unsigned int i = 1; i < t->elements().size(); ++i)
            if (t->elements().at(0)->duration().ticks() != t->elements().at(i)->duration().ticks())
                  return t->baseLen().ticks();
      if (t->elements().size() != (unsigned)(t->ratio().numerator()))
            return t->baseLen().ticks();
      return 0;
      }



static void writeBeam(XmlWriter& xml, ChordRest* cr, Beam* b)
      {
      const auto& elements = b->elements();
      int idx = elements.indexOf(cr);
      if (idx == -1) {
            qDebug("Beam::writeMusicXml(): cannot find ChordRest");
            return;
            }
      int blp = -1;       int blc = -1;       int bln = -1;             for (int i = idx - 1; blp == -1 && i >= 0; --i) {
            ChordRest* crst = elements[i];
            if (crst->type() == ElementType::CHORD)
                  blp = (static_cast<Chord*>(crst))->beams();
            }
            if (cr->type() == ElementType::CHORD)
            blc = (static_cast<Chord*>(cr))->beams();
            for (int i = idx + 1; bln == -1 && i < elements.size(); ++i) {
            ChordRest* crst = elements[i];
            if (crst->type() == ElementType::CHORD)
                  bln = (static_cast<Chord*>(crst))->beams();
            }
      for (int i = 1; i <= blc; ++i) {
            QString s;
            if (blp < i && bln >= i) s = "begin";
            else if (blp < i && bln < i) {
                  if (bln > 0) s = "forward hook";
                  else if (blp > 0) s = "backward hook";
                  }
            else if (blp >= i && bln < i)
                  s = "end";
            else if (blp >= i && bln >= i)
                  s = "continue";
            if (s != "")
                  xml.tag(QString("beam number=\"%1\"").arg(i), s);
            }
      }


static QString instrId(int partNr, int instrNr)
      {
      return QString("id=\"P%1-I%2\"").arg(partNr).arg(instrNr);
      }


static void writeNotehead(XmlWriter& xml, const Note* const note)
      {
      QString noteheadTagname = QString("notehead");
      noteheadTagname += color2xml(note);
      bool leftParenthesis, rightParenthesis = false;
      for (Element* elem : note->el()) {
            if (elem->type() == ElementType::SYMBOL) {
                  Symbol* s = static_cast<Symbol*>(elem);
                  if (s->sym() == SymId::noteheadParenthesisLeft)
                        leftParenthesis = true;
                  else if (s->sym() == SymId::noteheadParenthesisRight)
                        rightParenthesis = true;
                  }
            }
      if (rightParenthesis && leftParenthesis)
            noteheadTagname += " parentheses=\"yes\"";
      if (note->headType() == NoteHead::Type::HEAD_QUARTER)
            noteheadTagname += " filled=\"yes\"";
      else if ((note->headType() == NoteHead::Type::HEAD_HALF) || (note->headType() == NoteHead::Type::HEAD_WHOLE))
            noteheadTagname += " filled=\"no\"";
      if (note->headGroup() == NoteHead::Group::HEAD_SLASH)
            xml.tag(noteheadTagname, "slash");
      else if (note->headGroup() == NoteHead::Group::HEAD_TRIANGLE_UP)
            xml.tag(noteheadTagname, "triangle");
      else if (note->headGroup() == NoteHead::Group::HEAD_DIAMOND)
            xml.tag(noteheadTagname, "diamond");
      else if (note->headGroup() == NoteHead::Group::HEAD_PLUS)
            xml.tag(noteheadTagname, "cross");
      else if (note->headGroup() == NoteHead::Group::HEAD_CROSS)
            xml.tag(noteheadTagname, "x");
      else if (note->headGroup() == NoteHead::Group::HEAD_XCIRCLE)
            xml.tag(noteheadTagname, "circle-x");
      else if (note->headGroup() == NoteHead::Group::HEAD_TRIANGLE_DOWN)
            xml.tag(noteheadTagname, "inverted triangle");
      else if (note->headGroup() == NoteHead::Group::HEAD_SLASHED1)
            xml.tag(noteheadTagname, "slashed");
      else if (note->headGroup() == NoteHead::Group::HEAD_SLASHED2)
            xml.tag(noteheadTagname, "back slashed");
      else if (note->headGroup() == NoteHead::Group::HEAD_DO)
            xml.tag(noteheadTagname, "do");
      else if (note->headGroup() == NoteHead::Group::HEAD_RE)
            xml.tag(noteheadTagname, "re");
      else if (note->headGroup() == NoteHead::Group::HEAD_MI)
            xml.tag(noteheadTagname, "mi");
      else if (note->headGroup() == NoteHead::Group::HEAD_FA && !note->chord()->up())
            xml.tag(noteheadTagname, "fa");
      else if (note->headGroup() == NoteHead::Group::HEAD_FA && note->chord()->up())
            xml.tag(noteheadTagname, "fa up");
      else if (note->headGroup() == NoteHead::Group::HEAD_LA)
            xml.tag(noteheadTagname, "la");
      else if (note->headGroup() == NoteHead::Group::HEAD_TI)
            xml.tag(noteheadTagname, "ti");
      else if (note->headGroup() == NoteHead::Group::HEAD_SOL)
            xml.tag(noteheadTagname, "so");
      else if (note->color() != MScore::defaultColor)
            xml.tag(noteheadTagname, "normal");
      else if (rightParenthesis && leftParenthesis)
            xml.tag(noteheadTagname, "normal");
      else if (note->headType() != NoteHead::Type::HEAD_AUTO)
            xml.tag(noteheadTagname, "normal");
      }


static void writeFingering(XmlWriter& xml, Notations& notations, Technical& technical, const Note* const note)
      {
      for (const Element* e : note->el()) {
            if (e->type() == ElementType::FINGERING) {
                  Text* f = (Text*)e;
                  notations.tag(xml);
                  technical.tag(xml);
                  QString t = MScoreTextToMXML::toPlainText(f->xmlText());
                  if (f->subStyleId() == SubStyleId::RH_GUITAR_FINGERING)
                        xml.tag("pluck", t);
                  else if (f->subStyleId() == SubStyleId::LH_GUITAR_FINGERING)
                        xml.tag("fingering", t);
                  else if (f->subStyleId() == SubStyleId::FINGERING) {
                                                                                                if (t == "p" || t == "i" || t == "m" || t == "a" || t == "c")
                              xml.tag("pluck", t);
                        else
                              xml.tag("fingering", t);
                        }
                  else if (f->subStyleId() == SubStyleId::STRING_NUMBER) {
                        bool ok;
                        int i = t.toInt(&ok);
                        if (ok) {
                              if (i == 0)
                                    xml.tagE("open-string");
                              else if (i > 0)
                                    xml.tag("string", t);
                              }
                        if (!ok || i < 0)
                              qDebug("invalid string number '%s'", qPrintable(t));
                        }
                  else
                        qDebug("unknown fingering style");
                  }
            else {
                                    }
            }
      }


static int stretchCorrActTicks(const Note* const note)
      {
            const Fraction str = note->chord()->staff()->timeStretch(note->chord()->tick());
            return note->chord()->actualTicks() * str.numerator() / str.denominator();
      }


static int tremoloCorrection(const Note* const note)
      {
      int tremCorr = 1;
      if (isTwoNoteTremolo(note->chord())) tremCorr = 2;
      return tremCorr;
      }


static void writeTypeAndDots(XmlWriter& xml, const Note* const note)
      {
            int dots = 0;
      Tuplet* t = note->chord()->tuplet();
      int actNotes = 1;
      int nrmNotes = 1;
      if (t) {
            actNotes = t->ratio().numerator();
            nrmNotes = t->ratio().denominator();
            }

      const auto strActTicks = stretchCorrActTicks(note);
      QString s = tick2xml(strActTicks * actNotes * tremoloCorrection(note) / nrmNotes, &dots);
      if (s.isEmpty())
            qDebug("no note type found for ticks %d", strActTicks);

      if (note->small())
            xml.tag("type size=\"cue\"", s);
      else
            xml.tag("type", s);
      for (int ni = dots; ni > 0; ni--)
            xml.tagE("dot");
      }


static void writeTimeModification(XmlWriter& xml, const Note* const note)
      {
                  if (tremoloCorrection(note) == 2) {
            xml.stag("time-modification");
            xml.tag("actual-notes", 2);
            xml.tag("normal-notes", 1);
            xml.etag();
            }

            const auto t = note->chord()->tuplet();
      if (t) {
            auto actNotes = t->ratio().numerator();
            auto nrmNotes = t->ratio().denominator();
            auto nrmTicks = determineTupletNormalTicks(note->chord());
                        xml.stag("time-modification");
            xml.tag("actual-notes", actNotes);
            xml.tag("normal-notes", nrmNotes);
                        if (nrmTicks > 0) {
                  int nrmDots = 0;
                  QString nrmType = tick2xml(nrmTicks, &nrmDots);
                  if (nrmType.isEmpty())
                        qDebug("no note type found for ticks %d", nrmTicks);
                  else {
                        xml.tag("normal-type", nrmType);
                        for (int ni = nrmDots; ni > 0; ni--)
                              xml.tagE("normal-dot");
                        }
                  }
            xml.etag();
            }
      }


static void writePitch(XmlWriter& xml, const Note* const note, const bool useDrumset)
      {
            QString step;
      int alter = 0;
      int octave = 0;
      const auto chord = note->chord();
      if (chord->staff() && chord->staff()->isTabStaff(0)) {
            tabpitch2xml(note->pitch(), note->tpc(), step, alter, octave);
            }
      else {
            if (!useDrumset) {
                  pitch2xml(note, step, alter, octave);
                  }
            else {
                  unpitch2xml(note, step, octave);
                  }
            }
      xml.stag(useDrumset ? "unpitched" : "pitch");
      xml.tag(useDrumset  ? "display-step" : "step", step);
            auto acc = note->accidental();
      double alter2 = 0.0;
      if (acc) {
            switch (acc->accidentalType()) {
                  case AccidentalType::MIRRORED_FLAT:  alter2 = -0.5; break;
                  case AccidentalType::SHARP_SLASH:    alter2 = 0.5;  break;
                  case AccidentalType::MIRRORED_FLAT2: alter2 = -1.5; break;
                  case AccidentalType::SHARP_SLASH4:   alter2 = 1.5;  break;
                  default:                                             break;
                  }
            }
      if (alter && !alter2)
            xml.tag("alter", alter);
      if (!alter && alter2)
            xml.tag("alter", alter2);
            xml.tag(useDrumset ? "display-octave" : "octave", octave);
      xml.etag();
      }


static void writeAccidental(XmlWriter& xml, const Note* const note)
      {
      auto acc = note->accidental();
      if (acc) {
            QString s = accidentalType2MxmlString(acc->accidentalType());
            if (s != "") {
                  if (note->accidental()->bracket() != AccidentalBracket::NONE)
                        xml.tag("accidental parentheses=\"yes\"", s);
                  else
                        xml.tag("accidental", s);
                  }
            }
      }


static QString notePosition(const ExportMusicXml* const expMxml, const Note* const note)
      {
      QString res;

      if (preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT)) {
            const double pageHeight  = expMxml->getTenthsFromInches(expMxml->score()->styleD(Sid::pageHeight));

            const auto chord = note->chord();

            double measureX = expMxml->getTenthsFromDots(chord->measure()->pagePos().x());
            double measureY = pageHeight - expMxml->getTenthsFromDots(chord->measure()->pagePos().y());
            double noteX = expMxml->getTenthsFromDots(note->pagePos().x());
            double noteY = pageHeight - expMxml->getTenthsFromDots(note->pagePos().y());

            res += QString(" default-x=\"%1\"").arg(QString::number(noteX - measureX,'f',2));
            res += QString(" default-y=\"%1\"").arg(QString::number(noteY - measureY,'f',2));
            }

      return res;
      }




void ExportMusicXml::chord(Chord* chord, int staff, const std::vector<Lyrics*>* ll, bool useDrumset)
      {
      Part* part = chord->score()->staff(chord->track() / VOICES)->part();
      int partNr = _score->parts().indexOf(part);
      int instNr = instrMap.value(part->instrument(tick), -1);
      
      std::vector<Note*> nl = chord->notes();
      bool grace = chord->isGrace();
      if (!grace) tick += chord->actualTicks();
#ifdef DEBUG_TICK
      qDebug("ExportMusicXml::chord() oldtick=%d", tick);
      qDebug("notetype=%d grace=%d", gracen, grace);
      qDebug(" newtick=%d", tick);
#endif

      for (Note* note : nl) {
            QString val;

            attr.doAttr(xml, false);
            QString noteTag = QString("note");

            noteTag += notePosition(this, note);

            if (!note->visible()) {
                  noteTag += QString(" print-object=\"no\"");
                  }
                        if (note->veloType() == Note::ValueType::USER_VAL) {
                  int velo = note->veloOffset();
                  noteTag += QString(" dynamics=\"%1\"").arg(QString::number(velo * 100.0 / 90.0,'f',2));
                  }
            xml.stag(noteTag);

            if (grace) {
                  if (note->noteType() == NoteType::ACCIACCATURA)
                        xml.tagE("grace slash=\"yes\"");
                  else
                        xml.tagE("grace");
                  }
            if (note != nl.front())
                  xml.tagE("chord");
            else if (note->chord()->small())                   xml.tagE("cue");

            writePitch(xml, note, useDrumset);

                        if (!grace)
                  xml.tag("duration", stretchCorrActTicks(note) / div);

            if (note->tieBack())
                  xml.tagE("tie type=\"stop\"");
            if (note->tieFor())
                  xml.tagE("tie type=\"start\"");

                        if (!useDrumset) {
                  if (instrMap.size() > 1 && instNr >= 0)
                        xml.tagE(QString("instrument %1").arg(instrId(partNr + 1, instNr + 1)));
                  }
            else
                  xml.tagE(QString("instrument %1").arg(instrId(partNr + 1, note->pitch() + 1)));

                                                int voice = (staff-1) * VOICES + note->chord()->voice() + 1;
            if (staff == 0)
                  voice += VOICES;

            xml.tag("voice", voice);

            writeTypeAndDots(xml, note);
            writeAccidental(xml, note);
            writeTimeModification(xml, note);

                        if (chord->noStem() || chord->measure()->slashStyle(chord->staffIdx())) {
                  xml.tag("stem", QString("none"));
                  }
            else if (note->chord()->stem()) {
                  xml.tag("stem", QString(note->chord()->up() ? "up" : "down"));
                  }

            writeNotehead(xml, note);

                        if (staff)
                  xml.tag("staff", staff + note->chord()->staffMove());

            if (note == nl.front() && chord->beam())
                  writeBeam(xml, chord, chord->beam());

            Notations notations;
            Technical technical;

            const Tie* tieBack = note->tieBack();
            if (tieBack) {
                  notations.tag(xml);
                  xml.tagE("tied type=\"stop\"");
                  }
            const Tie* tieFor = note->tieFor();
            if (tieFor) {
                  notations.tag(xml);
                  QString rest = slurTieLineStyle(tieFor);
                  xml.tagE(QString("tied type=\"start\"%1").arg(rest));
                  }

            if (note == nl.front()) {
                  if (!grace)
                        tupletStartStop(chord, notations, xml);

                  sh.doSlurs(chord, notations, xml);

                  chordAttributes(chord, notations, technical, trillStart, trillStop);
                  }

            writeFingering(xml, notations, technical, note);

                        if (chord->staff() && chord->staff()->isTabStaff(0))
                  if (note->fret() >= 0 && note->string() >= 0) {
                        notations.tag(xml);
                        technical.tag(xml);
                        xml.tag("string", note->string() + 1);
                        xml.tag("fret", note->fret());
                        }

            technical.etag(xml);
            if (chord->arpeggio()) {
                  arpeggiate(chord->arpeggio(), note == nl.front(), note == nl.back(), xml, notations);
                  }
            for (Spanner* spanner : note->spannerFor())
                  if (spanner->type() == ElementType::GLISSANDO) {
                        gh.doGlissandoStart(static_cast<Glissando*>(spanner), notations, xml);
                        }
            for (Spanner* spanner : note->spannerBack())
                  if (spanner->type() == ElementType::GLISSANDO) {
                        gh.doGlissandoStop(static_cast<Glissando*>(spanner), notations, xml);
                        }
                        
            notations.etag(xml);
                        if (!grace && (note == nl.front()) && ll)
                  lyrics(ll, chord->track());
            xml.etag();
            }
      }




void ExportMusicXml::rest(Rest* rest, int staff)
      {
      static char table2[]  = "CDEFGAB";
#ifdef DEBUG_TICK
      qDebug("ExportMusicXml::rest() oldtick=%d", tick);
#endif
      attr.doAttr(xml, false);

      QString noteTag = QString("note");
      noteTag += color2xml(rest);
      if (!rest->visible() ) {
            noteTag += QString(" print-object=\"no\"");
            }
      xml.stag(noteTag);

      int yOffsSt   = 0;
      int oct       = 0;
      int stp       = 0;
      ClefType clef = rest->staff()->clef(rest->tick());
      int po        = ClefInfo::pitchOffset(clef);

            
      if (clef != ClefType::TAB && clef != ClefType::TAB_SERIF && clef != ClefType::TAB4 && clef != ClefType::TAB4_SERIF) {
            double yOffsSp = rest->userOff().y() / rest->spatium();                          yOffsSt = -2 * int(yOffsSp > 0.0 ? yOffsSp + 0.5 : yOffsSp - 0.5); 
            po -= 4;                po += yOffsSt;             oct = po / 7;             stp = po % 7;             }

                  if (yOffsSt == 0) {
            xml.tagE("rest");
            }
      else {
            xml.stag("rest");
            xml.tag("display-step", QString(QChar(table2[stp])));
            xml.tag("display-octave", oct - 1);
            xml.etag();
            }

      TDuration d = rest->durationType();
      int tickLen = rest->actualTicks();
      if (d.type() == TDuration::DurationType::V_MEASURE) {
                        tickLen = rest->measure()->ticks();
            }
      tick += tickLen;
#ifdef DEBUG_TICK
      qDebug(" tickLen=%d newtick=%d", tickLen, tick);
#endif

      xml.tag("duration", tickLen / div);

                  int voice = (staff-1) * VOICES + rest->voice() + 1;
      if (staff == 0)
            voice += VOICES;
      xml.tag("voice", voice);

            if (d.type() != TDuration::DurationType::V_MEASURE) {
            QString s = d.name();
            int dots  = rest->dots();
            if (rest->small())
                  xml.tag("type size=\"cue\"", s);
            else
                  xml.tag("type", s);
            for (int i = dots; i > 0; i--)
                  xml.tagE("dot");
            }

      if (rest->tuplet()) {
            Tuplet* t = rest->tuplet();
            xml.stag("time-modification");
            xml.tag("actual-notes", t->ratio().numerator());
            xml.tag("normal-notes", t->ratio().denominator());
            int nrmTicks = determineTupletNormalTicks(rest);
            if (nrmTicks > 0) {
                  int nrmDots = 0;
                  QString nrmType = tick2xml(nrmTicks, &nrmDots);
                  if (nrmType.isEmpty())
                        qDebug("no note type found for ticks %d", nrmTicks);
                  else {
                        xml.tag("normal-type", nrmType);
                        for (int ni = nrmDots; ni > 0; ni--)
                              xml.tagE("normal-dot");
                        }
                  }
            xml.etag();
            }

      if (staff)
            xml.tag("staff", staff);

      Notations notations;
      QVector<Element*> fl;
      for (Element* e : rest->segment()->annotations()) {
            if (e->isFermata() && e->track() == rest->track())
                  fl.push_back(e);
            }
      fermatas(fl, xml, notations);

      tupletStartStop(rest, notations, xml);
      notations.etag(xml);

      xml.etag();
      }


static void directionTag(XmlWriter& xml, Attributes& attr, Element const* const el = 0)
      {
      attr.doAttr(xml, false);
      QString tagname = QString("direction");
      if (el) {
            
            const Element* pel = 0;
            const LineSegment* seg = 0;
            if (el->type() == ElementType::HAIRPIN || el->type() == ElementType::OTTAVA
                || el->type() == ElementType::PEDAL || el->type() == ElementType::TEXTLINE) {
                                                      const SLine* sl = static_cast<const SLine*>(el);
                  if (sl->spannerSegments().size() > 0) {
                        seg = (LineSegment*)sl->spannerSegments().at(0);
                        
                        pel = seg->parent();
                        }
                  }
            else if (el->type() == ElementType::DYNAMIC
                     || el->type() == ElementType::INSTRUMENT_CHANGE
                     || el->type() == ElementType::REHEARSAL_MARK
                     || el->type() == ElementType::STAFF_TEXT
                     || el->type() == ElementType::SYMBOL
                     || el->type() == ElementType::TEXT) {
                                                      for (const Element* e = el; e; e = e->parent()) {
                        if (e->type() == ElementType::SYSTEM) pel = e;
                        }
                  }
            else
                  qDebug("directionTag() element %p tp=%d (%s) not supported",
                         el, int(el->type()), el->name());

            

            if (pel && pel->type() == ElementType::SYSTEM) {
                  const System* sys = static_cast<const System*>(pel);
                  QRectF bb = sys->staff(el->staffIdx())->bbox();
                  

                  if (el->isHairpin() || el->isOttava() || el->isPedal() || el->isTextLine()) {
                                                                                                if (seg->pagePos().y() < sys->pagePos().y() + bb.y() + bb.height() / 2)
                              tagname += " placement=\"above\"";
                        else
                              tagname += " placement=\"below\"";
                        }
                  else if (el->isDynamic()) {
                        tagname += " placement=\"";
                        tagname += el->placement() == Placement::ABOVE
                              ? "above" : "below";
                        tagname += "\"";
                        }
                  else {
                        
                        if (el->y() + el->height() / 2 <  bb.height() / 2)
                              tagname += " placement=\"above\"";
                        else
                              tagname += " placement=\"below\"";
                        }
                  }             }
      xml.stag(tagname);
      }


static void directionETag(XmlWriter& xml, int staff, int offs = 0)
      {
      if (offs)
            xml.tag("offset", offs);
      if (staff)
            xml.tag("staff", staff);
      xml.etag();
      }


static void partGroupStart(XmlWriter& xml, int number, BracketType bracket)
      {
      xml.stag(QString("part-group type=\"start\" number=\"%1\"").arg(number));
      QString br = "";
      switch (bracket) {
            case BracketType::NO_BRACKET:
                  br = "none";
                  break;
            case BracketType::NORMAL:
                  br = "bracket";
                  break;
            case BracketType::BRACE:
                  br = "brace";
                  break;
            case BracketType::LINE:
                  br = "line";
                  break;
            case BracketType::SQUARE:
                  br = "square";
                  break;
            default:
                  qDebug("bracket subtype %d not understood", int(bracket));
            }
      if (br != "")
            xml.tag("group-symbol", br);
      xml.etag();
      }


static bool findUnit(TDuration::DurationType val, QString& unit)
      {
      unit = "";
      switch (val) {
            case TDuration::DurationType::V_HALF: unit = "half"; break;
            case TDuration::DurationType::V_QUARTER: unit = "quarter"; break;
            case TDuration::DurationType::V_EIGHTH: unit = "eighth"; break;
            default: qDebug("findUnit: unknown DurationType %d", int(val));
            }
      return true;
      }

static bool findMetronome(const QList<TextFragment>& list,
                          QList<TextFragment>& wordsLeft,                            bool& hasParen,                                QString& metroLeft,                            QString& metroRight,                           QList<TextFragment>& wordsRight                           )
      {
      QString words = MScoreTextToMXML::toPlainTextPlusSymbols(list);
            hasParen   = false;
      metroLeft  = "";
      metroRight = "";
      int metroPos = -1;         int metroLen = 0;    
      int indEq  = words.indexOf('=');
      if (indEq <= 0)
            return false;

      int len1 = 0;
      TDuration dur;

                  int pos1 = TempoText::findTempoDuration(words.left(indEq), len1, dur);
      QRegExp eq("\\s*=\\s*");
      int pos2 = eq.indexIn(words, pos1 + len1);
      if (pos1 != -1 && pos2 == pos1 + len1) {
            int len2 = eq.matchedLength();
            if (words.length() > pos2 + len2) {
                  QString s1 = words.mid(0, pos1);                       QString s2 = words.mid(pos1, len1);                    QString s3 = words.mid(pos2, len2);                    QString s4 = words.mid(pos2 + len2);                   

                                                      int len3 = 0;
                  QRegExp nmb("\\d+");
                  int pos3 = TempoText::findTempoDuration(s4, len3, dur);
                  if (pos3 == -1) {
                                                pos3 = nmb.indexIn(s4);
                        if (pos3 == 0)
                              len3 = nmb.matchedLength();
                        }
                  if (pos3 == -1)
                                                return false;

                  QString s5 = s4.mid(0, len3);                   QString s6 = s4.mid(len3);                      

                                                                        int lparen = s1.indexOf("(");
                  int rparen = s6.indexOf(")");
                  hasParen = (lparen == s1.length() - 1 && rparen == 0);

                  metroLeft = s2;
                  metroRight = s5;

                  metroPos = pos1;                                 metroLen = len1 + len2 + len3;                   if (hasParen) {
                        metroPos -= 1;                                   metroLen += 2;                                   }

                                                      int corrPos = metroPos;
                  for (int i = 0; i < metroPos; ++i)
                        if (words.at(i).isHighSurrogate())
                              --corrPos;
                  metroPos = corrPos;

                  
                  QList<TextFragment> mid;                   MScoreTextToMXML::split(list, metroPos, metroLen, wordsLeft, mid, wordsRight);
                  return true;
                  }
            }
      return false;
      }

static void beatUnit(XmlWriter& xml, const TDuration dur)
      {
      int dots = dur.dots();
      QString unit;
      findUnit(dur.type(), unit);
      xml.tag("beat-unit", unit);
      while (dots > 0) {
            xml.tagE("beat-unit-dot");
            --dots;
            }
      }

static void wordsMetrome(XmlWriter& xml, Score* s, TextBase const* const text)
      {
            const QList<TextFragment> list = text->fragmentList();
      QList<TextFragment>       wordsLeft;        bool hasParen;                              QString metroLeft;                          QString metroRight;                         QList<TextFragment>       wordsRight; 
            const QString mtf = s->styleSt(Sid::MusicalTextFont);
      CharFormat defFmt;
      defFmt.setFontFamily(s->styleSt(Sid::staffTextFontFace));
      defFmt.setFontSize(s->styleD(Sid::staffTextFontSize));

      if (findMetronome(list, wordsLeft, hasParen, metroLeft, metroRight, wordsRight)) {
            if (wordsLeft.size() > 0) {
                  xml.stag("direction-type");
                  QString attr;                   attr += addPositioningAttributes(text);
                  MScoreTextToMXML mttm("words", attr, defFmt, mtf);
                  mttm.writeTextFragments(wordsLeft, xml);
                  xml.etag();
                  }

            xml.stag("direction-type");
            QString tagName = QString("metronome parentheses=\"%1\"").arg(hasParen ? "yes" : "no");
            tagName += addPositioningAttributes(text);
            xml.stag(tagName);
            int len1 = 0;
            TDuration dur;
            TempoText::findTempoDuration(metroLeft, len1, dur);
            beatUnit(xml, dur);

            if (TempoText::findTempoDuration(metroRight, len1, dur) != -1)
                  beatUnit(xml, dur);
            else
                  xml.tag("per-minute", metroRight);

            xml.etag();
            xml.etag();

            if (wordsRight.size() > 0) {
                  xml.stag("direction-type");
                  QString attr;                   attr += addPositioningAttributes(text);
                  MScoreTextToMXML mttm("words", attr, defFmt, mtf);
                  mttm.writeTextFragments(wordsRight, xml);
                  xml.etag();
                  }
            }

      else {
            xml.stag("direction-type");
            QString attr;
            if (text->hasFrame()) {
                  if (text->circle())
                        attr = " enclosure=\"circle\"";
                  else
                        attr = " enclosure=\"rectangle\"";
                  }
            attr += addPositioningAttributes(text);
            MScoreTextToMXML mttm("words", attr, defFmt, mtf);
                        mttm.writeTextFragments(text->fragmentList(), xml);
            xml.etag();
            }
      }

void ExportMusicXml::tempoText(TempoText const* const text, int staff)
      {
      
      attr.doAttr(xml, false);
      xml.stag(QString("direction placement=\"%1\"").arg((text->parent()->y()-text->y() < 0.0) ? "below" : "above"));
      wordsMetrome(xml, _score, text);
      
      if (staff)
            xml.tag("staff", staff);
      xml.tagE(QString("sound tempo=\"%1\"").arg(QString::number(text->tempo()*60.0)));
      xml.etag();
      }


void ExportMusicXml::words(Text const* const text, int staff)
      {
      

      if (text->plainText() == "") {
                                    return;
            }

      directionTag(xml, attr, text);
      wordsMetrome(xml, _score, text);
      directionETag(xml, staff);
      }


void ExportMusicXml::rehearsal(RehearsalMark const* const rmk, int staff)
      {
      if (rmk->plainText() == "") {
                                    return;
            }

      directionTag(xml, attr, rmk);
      xml.stag("direction-type");
      QString attr;
      attr += addPositioningAttributes(rmk);
      if (!rmk->hasFrame()) attr = " enclosure=\"none\"";
            const QString mtf = _score->styleSt(Sid::MusicalTextFont);
      CharFormat defFmt;
      defFmt.setFontFamily(_score->styleSt(Sid::staffTextFontFace));
      defFmt.setFontSize(_score->styleD(Sid::staffTextFontSize));
            MScoreTextToMXML mttm("rehearsal", attr, defFmt, mtf);
      mttm.writeTextFragments(rmk->fragmentList(), xml);
      xml.etag();
      directionETag(xml, staff);
      }


int ExportMusicXml::findHairpin(const Hairpin* hp) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (hairpins[i] == hp) return i;
      return -1;
      }


void ExportMusicXml::hairpin(Hairpin const* const hp, int staff, int tick)
      {
      int n = findHairpin(hp);
      if (n >= 0)
            hairpins[n] = 0;
      else {
            n = findHairpin(0);
            if (n >= 0)
                  hairpins[n] = hp;
            else {
                  qDebug("too many overlapping hairpins (hp %p staff %d tick %d)", hp, staff, tick);
                  return;
                  }
            }

      directionTag(xml, attr, hp);
      xml.stag("direction-type");

      QString hairpinXml;
      if (hp->tick() == tick) {
            if (hp->hairpinType() == HairpinType::CRESC_HAIRPIN) {
                  if (hp->hairpinCircledTip())
                        hairpinXml = QString("wedge type=\"crescendo\" niente=\"yes\" number=\"%1\"").arg(n + 1);
                  else
                        hairpinXml = QString("wedge type=\"crescendo\" number=\"%1\"").arg(n + 1);
                  }
            else
                  hairpinXml = QString("wedge type=\"diminuendo\" number=\"%1\"").arg(n + 1);
            }
      else {
            if (hp->hairpinCircledTip() && hp->hairpinType() == HairpinType::DECRESC_HAIRPIN)
                  hairpinXml = QString("wedge type=\"stop\" niente=\"yes\" number=\"%1\"").arg(n + 1);
            else
                  hairpinXml = QString("wedge type=\"stop\" number=\"%1\"").arg(n + 1);

            }
      hairpinXml += addPositioningAttributes(hp, hp->tick() == tick);
      xml.tagE(hairpinXml);
      xml.etag();
      directionETag(xml, staff);
      }


int ExportMusicXml::findOttava(const Ottava* ot) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (ottavas[i] == ot) return i;
      return -1;
      }


void ExportMusicXml::ottava(Ottava const* const ot, int staff, int tick)
      {
      int n = findOttava(ot);
      if (n >= 0)
            ottavas[n] = 0;
      else {
            n = findOttava(0);
            if (n >= 0)
                  ottavas[n] = ot;
            else {
                  qDebug("too many overlapping ottavas (ot %p staff %d tick %d)", ot, staff, tick);
                  return;
                  }
            }

      directionTag(xml, attr, ot);
      xml.stag("direction-type");

      QString octaveShiftXml;
      OttavaType st = ot->ottavaType();
      if (ot->tick() == tick) {
            const char* sz = 0;
            const char* tp = 0;
            switch (st) {
                  case OttavaType::OTTAVA_8VA:
                        sz = "8";
                        tp = "down";
                        break;
                  case OttavaType::OTTAVA_15MA:
                        sz = "15";
                        tp = "down";
                        break;
                  case OttavaType::OTTAVA_8VB:
                        sz = "8";
                        tp = "up";
                        break;
                  case OttavaType::OTTAVA_15MB:
                        sz = "15";
                        tp = "up";
                        break;
                  default:
                        qDebug("ottava subtype %d not understood", int(st));
                  }
            if (sz && tp)
                  octaveShiftXml = QString("octave-shift type=\"%1\" size=\"%2\" number=\"%3\"").arg(tp).arg(sz).arg(n + 1);
            }
      else {
            if (st == OttavaType::OTTAVA_8VA || st == OttavaType::OTTAVA_8VB)
                  octaveShiftXml = QString("octave-shift type=\"stop\" size=\"8\" number=\"%1\"").arg(n + 1);
            else if (st == OttavaType::OTTAVA_15MA || st == OttavaType::OTTAVA_15MB)
                  octaveShiftXml = QString("octave-shift type=\"stop\" size=\"15\" number=\"%1\"").arg(n + 1);
            else
                  qDebug("ottava subtype %d not understood", int(st));
            }
      octaveShiftXml += addPositioningAttributes(ot, ot->tick() == tick);
      xml.tagE(octaveShiftXml);
      xml.etag();
      directionETag(xml, staff);
      }


void ExportMusicXml::pedal(Pedal const* const pd, int staff, int tick)
      {
      directionTag(xml, attr, pd);
      xml.stag("direction-type");
      QString pedalXml;
      if (pd->tick() == tick)
            pedalXml = "pedal type=\"start\" line=\"yes\"";
      else
            pedalXml = "pedal type=\"stop\" line=\"yes\"";
      pedalXml += addPositioningAttributes(pd, pd->tick() == tick);
      xml.tagE(pedalXml);
      xml.etag();
      directionETag(xml, staff);
      }


int ExportMusicXml::findBracket(const TextLine* tl) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (brackets[i] == tl) return i;
      return -1;
      }


void ExportMusicXml::textLine(TextLine const* const tl, int staff, int tick)
      {
      int n = findBracket(tl);
      if (n >= 0)
            brackets[n] = 0;
      else {
            n = findBracket(0);
            if (n >= 0)
                  brackets[n] = tl;
            else {
                  qDebug("too many overlapping textlines (tl %p staff %d tick %d)", tl, staff, tick);
                  return;
                  }
            }

      QString rest;
      QPointF p;

            bool dashes = tl->lineStyle() == Qt::DashLine && (tl->beginHookType() == HookType::NONE) && (tl->endHookType() == HookType::NONE);

      QString lineEnd = "none";
      QString type;
      bool hook = false;
      double hookHeight = 0.0;
      if (tl->tick() == tick) {
            if (!dashes) {
                  QString lineType;
                  switch (tl->lineStyle()) {
                        case Qt::SolidLine:
                              lineType = "solid";
                              break;
                        case Qt::DashLine:
                              lineType = "dashed";
                              break;
                        case Qt::DotLine:
                              lineType = "dotted";
                              break;
                        default:
                              lineType = "solid";
                        }
                  rest += QString(" line-type=\"%1\"").arg(lineType);
                  }
            hook       = tl->beginHookType() != HookType::NONE;
            hookHeight = tl->beginHookHeight().val();
            if (!tl->spannerSegments().empty())
                  p = tl->spannerSegments().first()->userOff();
                        type = "start";
            }
      else {
            hook = tl->endHookType() != HookType::NONE;
            hookHeight = tl->endHookHeight().val();
            if (!tl->spannerSegments().empty())
                  p = ((LineSegment*)tl->spannerSegments().last())->userOff2();
                        type = "stop";
            }

      if (hook) {
            if (hookHeight < 0.0) {
                  lineEnd = "up";
                  hookHeight *= -1.0;
                  }
            else
                  lineEnd = "down";
            rest += QString(" end-length=\"%1\"").arg(hookHeight * 10);
            }

      rest += addPositioningAttributes(tl, tl->tick() == tick);

      directionTag(xml, attr, tl);
      if (!tl->beginText().isEmpty() && tl->tick() == tick) {
            xml.stag("direction-type");
            xml.tag("words", tl->beginText());
            xml.etag();
            }
      xml.stag("direction-type");
      if (dashes)
            xml.tagE(QString("dashes type=\"%1\" number=\"%2\"").arg(type, QString::number(n + 1)));
      else
            xml.tagE(QString("bracket type=\"%1\" number=\"%2\" line-end=\"%3\"%4").arg(type, QString::number(n + 1), lineEnd, rest));
      xml.etag();
      
      directionETag(xml, staff);
      }



void ExportMusicXml::dynamic(Dynamic const* const dyn, int staff)
      {
      QSet<QString> set;       set << "f" << "ff" << "fff" << "ffff" << "fffff" << "ffffff"
          << "fp" << "fz"
          << "mf" << "mp"
          << "p" << "pp" << "ppp" << "pppp" << "ppppp" << "pppppp"
          << "rf" << "rfz"
          << "sf" << "sffz" << "sfp" << "sfpp" << "sfz";

      directionTag(xml, attr, dyn);

      xml.stag("direction-type");

      QString tagName = "dynamics";
      tagName += addPositioningAttributes(dyn);
      xml.stag(tagName);
      QString dynTypeName = dyn->dynamicTypeName();
      if (set.contains(dynTypeName)) {
            xml.tagE(dynTypeName);
            }
      else if (dynTypeName != "") {
            QString dynText = dynTypeName;
            if (dyn->dynamicType() == Dynamic::Type::OTHER)
                  dynText = dyn->plainText();
            xml.tag("other-dynamics", dynText);
            }
      xml.etag();

      xml.etag();

      
      if (staff)
            xml.tag("staff", staff);

      if (dyn->velocity() > 0)
            xml.tagE(QString("sound dynamics=\"%1\"").arg(QString::number(dyn->velocity() * 100.0 / 90.0, 'f', 2)));

      xml.etag();
      }



void ExportMusicXml::symbol(Symbol const* const sym, int staff)
      {
      QString name = Sym::id2name(sym->sym());
      QString mxmlName = "";
      if (name == "keyboardPedalPed")
            mxmlName = "pedal type=\"start\"";
      else if (name == "keyboardPedalUp")
            mxmlName = "pedal type=\"stop\"";
      else {
            qDebug("ExportMusicXml::symbol(): %s not supported", qPrintable(name));
            return;
            }
      directionTag(xml, attr, sym);
      mxmlName += addPositioningAttributes(sym);
      xml.stag("direction-type");
      xml.tagE(mxmlName);
      xml.etag();
      directionETag(xml, staff);
      }


void ExportMusicXml::lyrics(const std::vector<Lyrics*>* ll, const int trk)
      {
      for (const Lyrics* l :* ll) {
            if (l && !l->xmlText().isEmpty()) {
                  if ((l)->track() == trk) {
                        QString lyricXml = QString("lyric number=\"%1\"").arg((l)->no() + 1);
                        lyricXml += color2xml(l);
                        lyricXml += addPositioningAttributes(l);
                        xml.stag(lyricXml);
                        Lyrics::Syllabic syl = (l)->syllabic();
                        QString s = "";
                        switch (syl) {
                              case Lyrics::Syllabic::SINGLE: s = "single"; break;
                              case Lyrics::Syllabic::BEGIN:  s = "begin";  break;
                              case Lyrics::Syllabic::END:    s = "end";    break;
                              case Lyrics::Syllabic::MIDDLE: s = "middle"; break;
                              default:
                                    qDebug("unknown syllabic %d", int(syl));
                              }
                        xml.tag("syllabic", s);
                        QString attr;                                                 const QString mtf       = _score->styleSt(Sid::MusicalTextFont);
                        CharFormat defFmt;
                        defFmt.setFontFamily(_score->styleSt(Sid::lyricsEvenFontFace));
                        defFmt.setFontSize(_score->styleD(Sid::lyricsOddFontSize));
                                                MScoreTextToMXML mttm("text", attr, defFmt, mtf);
                        mttm.writeTextFragments(l->fragmentList(), xml);
#if 0
                        
#else
                        if (l->ticks())
                              xml.tagE("extend");
#endif
                        xml.etag();
                        }
                  }
            }
      }



static void directionJump(XmlWriter& xml, const Jump* const jp)
      {
      Jump::Type jtp = jp->jumpType();
      QString words = "";
      QString type  = "";
      QString sound = "";
      if (jtp == Jump::Type::DC) {
            if (jp->xmlText() == "")
                  words = "D.C.";
            else
                  words = jp->xmlText();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == Jump::Type::DC_AL_FINE) {
            if (jp->xmlText() == "")
                  words = "D.C. al Fine";
            else
                  words = jp->xmlText();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == Jump::Type::DC_AL_CODA) {
            if (jp->xmlText() == "")
                  words = "D.C. al Coda";
            else
                  words = jp->xmlText();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == Jump::Type::DS_AL_CODA) {
            if (jp->xmlText() == "")
                  words = "D.S. al Coda";
            else
                  words = jp->xmlText();
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else if (jtp == Jump::Type::DS_AL_FINE) {
            if (jp->xmlText() == "")
                  words = "D.S. al Fine";
            else
                  words = jp->xmlText();
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else if (jtp == Jump::Type::DS) {
            words = "D.S.";
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else
            qDebug("jump type=%d not implemented", int(jtp));
      if (sound != "") {
            xml.stag("direction placement=\"above\"");
            xml.stag("direction-type");
            QString positioning = "";
            positioning += addPositioningAttributes(jp);
            if (type != "") xml.tagE(type + positioning);
            if (words != "") xml.tag("words" + positioning, words);
            xml.etag();
            if (sound != "") xml.tagE(QString("sound ") + sound);
            xml.etag();
            }
      }


static void directionMarker(XmlWriter& xml, const Marker* const m)
      {
      Marker::Type mtp = m->markerType();
      QString words = "";
      QString type  = "";
      QString sound = "";
      if (mtp == Marker::Type::CODA) {
            type = "coda";
            if (m->label() == "")
                  sound = "coda=\"1\"";
            else
                                                      sound = "coda=\"coda\"";
            }
      else if (mtp == Marker::Type::SEGNO) {
            type = "segno";
            if (m->label() == "")
                  sound = "segno=\"1\"";
            else
                  sound = "segno=\"" + m->label() + "\"";
            }
      else if (mtp == Marker::Type::FINE) {
            words = "Fine";
            sound = "fine=\"yes\"";
            }
      else if (mtp == Marker::Type::TOCODA) {
            if (m->xmlText() == "")
                  words = "To Coda";
            else
                  words = m->xmlText();
            if (m->label() == "")
                  sound = "tocoda=\"1\"";
            else
                  sound = "tocoda=\"" + m->label() + "\"";
            }
      else
            qDebug("marker type=%d not implemented", int(mtp));
      if (sound != "") {
            xml.stag("direction placement=\"above\"");
            xml.stag("direction-type");
            QString positioning = "";
            positioning += addPositioningAttributes(m);
            if (type != "") xml.tagE(type + positioning);
            if (words != "") xml.tag("words" + positioning, words);
            xml.etag();
            if (sound != "") xml.tagE(QString("sound ") + sound);
            xml.etag();
            }
      }



static int findTrackForAnnotations(int track, Segment* seg)
      {
      if (seg->segmentType() != SegmentType::ChordRest)
            return -1;

      int staff = track / VOICES;
      int strack = staff * VOICES;            int etrack = strack + VOICES;     
      for (int i = strack; i < etrack; i++)
            if (seg->element(i))
                  return i;

      return -1;
      }


static void repeatAtMeasureStart(XmlWriter& xml, Attributes& attr, Measure* m, int strack, int etrack, int track)
      {
            for (Element* e : m->el()) {
            int wtrack = -1;             if (strack <= e->track() && e->track() < etrack)
                  wtrack = findTrackForAnnotations(e->track(), m->first(SegmentType::ChordRest));
            if (track != wtrack)
                  continue;
            switch (e->type()) {
                  case ElementType::MARKER:
                        {
                                                const Marker* const mk = static_cast<const Marker* const>(e);
                        Marker::Type mtp = mk->markerType();
                        if (   mtp == Marker::Type::SEGNO
                               || mtp == Marker::Type::CODA
                               ) {
                              qDebug(" -> handled");
                              attr.doAttr(xml, false);
                              directionMarker(xml, mk);
                              }
                        else if (   mtp == Marker::Type::FINE
                                    || mtp == Marker::Type::TOCODA
                                    ) {
                                                            }
                        else {
                              qDebug("repeatAtMeasureStart: marker %d not implemented", int(mtp));
                              }
                        }
                        break;
                  default:
                        qDebug("repeatAtMeasureStart: direction type %s at tick %d not implemented",
                               Element::name(e->type()), m->tick());
                        break;
                  }
            }
      }


static void repeatAtMeasureStop(XmlWriter& xml, Measure* m, int strack, int etrack, int track)
      {
      for (Element* e : m->el()) {
            int wtrack = -1;             if (strack <= e->track() && e->track() < etrack)
                  wtrack = findTrackForAnnotations(e->track(), m->first(SegmentType::ChordRest));
            if (track != wtrack)
                  continue;
            switch (e->type()) {
                  case ElementType::MARKER:
                        {
                                                const Marker* const mk = static_cast<const Marker* const>(e);
                        Marker::Type mtp = mk->markerType();
                        if (mtp == Marker::Type::FINE || mtp == Marker::Type::TOCODA) {
                              directionMarker(xml, mk);
                              }
                        else if (mtp == Marker::Type::SEGNO || mtp == Marker::Type::CODA) {
                                                            }
                        else {
                              qDebug("repeatAtMeasureStop: marker %d not implemented", int(mtp));
                              }
                        }
                        break;
                  case ElementType::JUMP:
                        directionJump(xml, static_cast<const Jump* const>(e));
                        break;
                  default:
                        qDebug("repeatAtMeasureStop: direction type %s at tick %d not implemented",
                               Element::name(e->type()), m->tick());
                        break;
                  }
            }
      }


void ExportMusicXml::work(const MeasureBase* )
      {
      QString workTitle  = _score->metaTag("workTitle");
      QString workNumber = _score->metaTag("workNumber");
      if (!(workTitle.isEmpty() && workNumber.isEmpty())) {
            xml.stag("work");
            if (!workNumber.isEmpty())
                  xml.tag("work-number", workNumber);
            if (!workTitle.isEmpty())
                  xml.tag("work-title", workTitle);
            xml.etag();
            }
      if (!_score->metaTag("movementNumber").isEmpty())
            xml.tag("movement-number", _score->metaTag("movementNumber"));
      if (!_score->metaTag("movementTitle").isEmpty())
            xml.tag("movement-title", _score->metaTag("movementTitle"));
      }

#if 0

static bool elementRighter(const Element* e1, const Element* e2)
      {
      return e1->x() < e2->x();
      }
#endif



static void measureStyle(XmlWriter& xml, Attributes& attr, Measure* m)
      {
      const Measure* mmR1 = m->mmRest1();
      if (m != mmR1 && m == mmR1->mmRestFirst()) {
            attr.doAttr(xml, true);
            xml.stag("measure-style");
            xml.tag("multiple-rest", mmR1->mmRestCount());
            xml.etag();
            }
      }


static const FretDiagram* findFretDiagram(int strack, int etrack, int track, Segment* seg)
      {
      if (seg->segmentType() == SegmentType::ChordRest) {
            for (const Element* e : seg->annotations()) {

                  int wtrack = -1; 
                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack && e->type() == ElementType::FRET_DIAGRAM)
                        return static_cast<const FretDiagram*>(e);
                  }
            }
      return 0;
      }



static void annotations(ExportMusicXml* exp, XmlWriter&, int strack, int etrack, int track, int sstaff, Segment* seg)
      {
      if (seg->segmentType() == SegmentType::ChordRest) {

            const FretDiagram* fd = findFretDiagram(strack, etrack, track, seg);
            
            for (const Element* e : seg->annotations()) {

                  int wtrack = -1; 
                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack) {
                        switch (e->type()) {
                              case ElementType::SYMBOL:
                                    exp->symbol(static_cast<const Symbol*>(e), sstaff);
                                    break;
                              case ElementType::TEMPO_TEXT:
                                    exp->tempoText(static_cast<const TempoText*>(e), sstaff);
                                    break;
                              case ElementType::STAFF_TEXT:
                              case ElementType::TEXT:
                              case ElementType::INSTRUMENT_CHANGE:
                                    exp->words(static_cast<const Text*>(e), sstaff);
                                    break;
                              case ElementType::DYNAMIC:
                                    exp->dynamic(static_cast<const Dynamic*>(e), sstaff);
                                    break;
                              case ElementType::HARMONY:
                                                                        exp->harmony(static_cast<const Harmony*>(e), fd );
                                    fd = 0;                                     break;
                              case ElementType::REHEARSAL_MARK:
                                    exp->rehearsal(static_cast<const RehearsalMark*>(e), sstaff);
                                    break;
                              case ElementType::FIGURED_BASS:                               case ElementType::FRET_DIAGRAM:                               case ElementType::JUMP:                                             break;
                              default:
                                    qDebug("annotations: direction type %s at tick %d not implemented",
                                           Element::name(e->type()), seg->tick());
                                    break;
                              }
                        }
                  }             if (fd)
                                    qDebug("annotations seg %p found fretboard diagram %p w/o harmony: cannot write",
                         seg, fd);
            }
      }


static void figuredBass(XmlWriter& xml, int strack, int etrack, int track, const ChordRest* cr, FigBassMap& fbMap, int divisions)
      {
      Segment* seg = cr->segment();
      if (seg->segmentType() == SegmentType::ChordRest) {
            for (const Element* e : seg->annotations()) {

                  int wtrack = -1; 
                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack) {
                        if (e->type() == ElementType::FIGURED_BASS) {
                              const FiguredBass* fb = static_cast<const FiguredBass*>(e);
                                                                                          bool extend = fb->ticks() > cr->actualTicks();
                              if (extend) {
                                                                                                            fbMap.insert(strack, fb);
                                    }
                              else
                                    fbMap.remove(strack);
                              int crEndTick = cr->tick() + cr->actualTicks();
                              int fbEndTick = fb->segment()->tick() + fb->ticks();
                              bool writeDuration = fb->ticks() < cr->actualTicks();
                              fb->writeMusicXML(xml, true, crEndTick, fbEndTick, writeDuration, divisions);

                                                            for (Segment* segNext = seg->next(); segNext && segNext->element(track) == NULL; segNext = segNext->next()) {
                                    for (Element* annot : segNext->annotations()) {
                                          if (annot->type() == ElementType::FIGURED_BASS && annot->track() == track) {
                                                const FiguredBass* fb = static_cast<const FiguredBass*>(annot);
                                                fb->writeMusicXML(xml, true, 0, 0, true, divisions);
                                                }
                                          }
                                    }
                                                            return;
                              }
                        }
                  }
                        if (fbMap.contains(strack)) {
                  const FiguredBass* fb = fbMap.value(strack);
                  int crEndTick = cr->tick() + cr->actualTicks();
                  int fbEndTick = fb->segment()->tick() + fb->ticks();
                  bool writeDuration = fb->ticks() < cr->actualTicks();
                  if (cr->tick() < fbEndTick) {
                                                fb->writeMusicXML(xml, false, crEndTick, fbEndTick, writeDuration, divisions);
                        }
                  if (fbEndTick <= crEndTick) {
                                                fbMap.remove(strack);
                        }
                  }
            }
      }



static void spannerStart(ExportMusicXml* exp, int strack, int etrack, int track, int sstaff, Segment* seg)
      {
      if (seg->segmentType() == SegmentType::ChordRest) {
            int stick = seg->tick();
            for (auto it = exp->score()->spanner().lower_bound(stick); it != exp->score()->spanner().upper_bound(stick); ++it) {
                  Spanner* e = it->second;

                  int wtrack = -1;                   if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack) {
                        switch (e->type()) {
                              case ElementType::HAIRPIN:
                                    exp->hairpin(static_cast<const Hairpin*>(e), sstaff, seg->tick());
                                    break;
                              case ElementType::OTTAVA:
                                    exp->ottava(static_cast<const Ottava*>(e), sstaff, seg->tick());
                                    break;
                              case ElementType::PEDAL:
                                    exp->pedal(static_cast<const Pedal*>(e), sstaff, seg->tick());
                                    break;
                              case ElementType::TEXTLINE:
                                    exp->textLine(static_cast<const TextLine*>(e), sstaff, seg->tick());
                                    break;
                              case ElementType::TRILL:
                                                                        break;
                              case ElementType::SLUR:
                                                                        break;
                              default:
                                    qDebug("spannerStart: direction type %s at tick %d not implemented",
                                           Element::name(e->type()), seg->tick());
                                    break;
                              }
                        }
                  }             }
      }



static void spannerStop(ExportMusicXml* exp, int strack, int tick2, int sstaff, QSet<const Spanner*>& stopped)
      {
      for (auto it : exp->score()->spanner()) {
            Spanner* e = it.second;

            if (e->tick2() != tick2 || e->track() != strack)
                  continue;

            if (!stopped.contains(e)) {
                  stopped.insert(e);
                  switch (e->type()) {
                        case ElementType::HAIRPIN:
                              exp->hairpin(static_cast<const Hairpin*>(e), sstaff, -1);
                              break;
                        case ElementType::OTTAVA:
                              exp->ottava(static_cast<const Ottava*>(e), sstaff, -1);
                              break;
                        case ElementType::PEDAL:
                              exp->pedal(static_cast<const Pedal*>(e), sstaff, -1);
                              break;
                        case ElementType::TEXTLINE:
                              exp->textLine(static_cast<const TextLine*>(e), sstaff, -1);
                              break;
                        case ElementType::TRILL:
                                                            break;
                        case ElementType::SLUR:
                                                            break;
                        default:
                              qDebug("spannerStop: direction type %s at tick2 %d not implemented",
                                     Element::name(e->type()), tick2);
                              break;
                        }
                  }
            }       }




void ExportMusicXml::keysigTimesig(const Measure* m, const Part* p)
      {
      int strack = p->startTrack();
      int etrack = p->endTrack();
      
            QMap<int, KeySig*> keysigs;       for (Segment* seg = m->first(); seg; seg = seg->next()) {
            if (seg->tick() > m->tick())
                  break;
            for (int t = strack; t < etrack; t += VOICES) {
                  Element* el = seg->element(t);
                  if (!el)
                        continue;
                  if (el->type() == ElementType::KEYSIG) {
                                                int st = (t - strack) / VOICES;
                        if (!el->generated())
                              keysigs[st] = static_cast<KeySig*>(el);
                        }
                  }
            }

      
            if (!keysigs.isEmpty()) {
                                    int nstaves = p->nstaves();
            bool singleKey = true;
                        for (int i = 0; i < nstaves; i++)
                  if (!keysigs.contains(i))
                        singleKey = false;
                        if (singleKey)
                  for (int i = 1; i < nstaves; i++)
                        if (!(keysigs.value(i)->key() == keysigs.value(0)->key()))
                              singleKey = false;

                                    if (singleKey) {
                                    keysig(keysigs.value(0), p->staff(0)->clef(m->tick()), 0, keysigs.value(0)->visible());
                  }
            else {
                                    for (int st : keysigs.keys())
                        keysig(keysigs.value(st), p->staff(st)->clef(m->tick()), st + 1, keysigs.value(st)->visible());
                  }
            }
      else {
                        if (m->tick() == 0) {
                                                      KeySig* ks = new KeySig(_score);
                  ks->setKey(Key::C);
                  keysig(ks, p->staff(0)->clef(m->tick()));
                  delete ks;
                  }
            }

      TimeSig* tsig = 0;
      for (Segment* seg = m->first(); seg; seg = seg->next()) {
            if (seg->tick() > m->tick())
                  break;
            Element* el = seg->element(strack);
            if (el && el->type() == ElementType::TIMESIG)
                  tsig = (TimeSig*) el;
            }
      if (tsig)
            timesig(tsig);
      }


static void identification(XmlWriter& xml, Score const* const score)
      {
      xml.stag("identification");

      QStringList creators;
            creators << "arranger" << "composer" << "lyricist" << "poet" << "translator";
      for (QString type : creators) {
            QString creator = score->metaTag(type);
            if (!creator.isEmpty())
                  xml.tag(QString("creator type=\"%1\"").arg(type), creator);
            }

      if (!score->metaTag("copyright").isEmpty())
            xml.tag("rights", score->metaTag("copyright"));

      xml.stag("encoding");

      if (MScore::debugMode) {
            xml.tag("software", QString("MuseScore 0.7.0"));
            xml.tag("encoding-date", QString("2007-09-10"));
            }
      else {
            xml.tag("software", QString("MuseScore ") + QString(VERSION));
            xml.tag("encoding-date", QDate::currentDate().toString(Qt::ISODate));
            }

            xml.tagE("supports element=\"accidental\" type=\"yes\"");
      xml.tagE("supports element=\"beam\" type=\"yes\"");
                  if (preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT)
          && preferences.musicxmlExportBreaks() == MusicxmlExportBreaks::ALL) {
            xml.tagE("supports element=\"print\" attribute=\"new-page\" type=\"yes\" value=\"yes\"");
            xml.tagE("supports element=\"print\" attribute=\"new-system\" type=\"yes\" value=\"yes\"");
            }
      else {
            xml.tagE("supports element=\"print\" attribute=\"new-page\" type=\"no\"");
            xml.tagE("supports element=\"print\" attribute=\"new-system\" type=\"no\"");
            }
      xml.tagE("supports element=\"stem\" type=\"yes\"");

      xml.etag();

      if (!score->metaTag("source").isEmpty())
            xml.tag("source", score->metaTag("source"));

      xml.etag();
      }


static int findPartGroupNumber(int* partGroupEnd)
      {
            for (int number = 0; number < MAX_PART_GROUPS; ++number)
            if (partGroupEnd[number] == -1)
                  return number;
      qDebug("no free part group number");
      return MAX_PART_GROUPS;
      }


static void scoreInstrument(XmlWriter& xml, const int partNr, const int instrNr, const QString& instrName)
      {
      xml.stag(QString("score-instrument %1").arg(instrId(partNr, instrNr)));
      xml.tag("instrument-name", instrName);
      xml.etag();
      }


static void midiInstrument(XmlWriter& xml, const int partNr, const int instrNr,
                           const Instrument* instr, const Score* score, const int unpitched = 0)
      {
      xml.stag(QString("midi-instrument %1").arg(instrId(partNr, instrNr)));
      int midiChannel = score->masterScore()->midiChannel(instr->channel(0)->channel);
      if (midiChannel >= 0 && midiChannel < 16)
            xml.tag("midi-channel", midiChannel + 1);
      int midiProgram = instr->channel(0)->program;
      if (midiProgram >= 0 && midiProgram < 128)
            xml.tag("midi-program", midiProgram + 1);
      if (unpitched > 0)
            xml.tag("midi-unpitched", unpitched);
      xml.tag("volume", (instr->channel(0)->volume / 127.0) * 100);        xml.tag("pan", int(((instr->channel(0)->pan - 63.5) / 63.5) * 90));       xml.etag();
      }




static void initInstrMap(MxmlInstrumentMap& im, const InstrumentList* il, const Score* )
      {
      im.clear();
      for (auto i = il->begin(); i != il->end(); ++i) {
            const Instrument* pinstr = i->second;
            if (!im.contains(pinstr))
                  im.insert(pinstr, im.size());
            }
      }


typedef QMap<int, const Instrument*> MxmlReverseInstrumentMap;



static void initReverseInstrMap(MxmlReverseInstrumentMap& rim, const MxmlInstrumentMap& im)
      {
      rim.clear();
      for (const Instrument* i : im.keys()) {
            int instNr = im.value(i);
            rim.insert(instNr, i);
            }
      }




void ExportMusicXml::print(Measure* m, int idx, int staffCount, int staves)
      {
      int currentSystem = NoSystem;
      Measure* previousMeasure = 0;

      for (MeasureBase* currentMeasureB = m->prev(); currentMeasureB; currentMeasureB = currentMeasureB->prev()) {
            if (currentMeasureB->type() == ElementType::MEASURE) {
                  previousMeasure = (Measure*) currentMeasureB;
                  break;
                  }
            }

      if (!previousMeasure)
            currentSystem = TopSystem;
      else {
            const auto mSystem = m->mmRest1()->system();
            const auto previousMeasureSystem = previousMeasure->mmRest1()->system();

            if (mSystem && previousMeasureSystem) {
                  if (mSystem->page() != previousMeasureSystem->page())
                        currentSystem = NewPage;
                  else if (mSystem != previousMeasureSystem)
                        currentSystem = NewSystem;
                  }
            }

      bool prevMeasLineBreak = false;
      bool prevMeasPageBreak = false;
      if (previousMeasure) {
            prevMeasLineBreak = previousMeasure->lineBreak();
            prevMeasPageBreak = previousMeasure->pageBreak();
            }

      if (currentSystem != NoSystem) {

                        QString newThing;                   if (preferences.musicxmlExportBreaks() == MusicxmlExportBreaks::ALL) {
                  if (currentSystem == NewSystem)
                        newThing = " new-system=\"yes\"";
                  else if (currentSystem == NewPage)
                        newThing = " new-page=\"yes\"";
                  }
            else if (preferences.musicxmlExportBreaks() == MusicxmlExportBreaks::MANUAL) {
                  if (currentSystem == NewSystem && prevMeasLineBreak)
                        newThing = " new-system=\"yes\"";
                  else if (currentSystem == NewPage && prevMeasPageBreak)
                        newThing = " new-page=\"yes\"";
                  }

                        bool doLayout = false;
            if (preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT)) {
                  if (currentSystem == TopSystem
                      || (preferences.musicxmlExportBreaks() == MusicxmlExportBreaks::ALL && newThing != "")) {
                        doLayout = true;
                        }
                  }

            if (doLayout) {
                  xml.stag(QString("print%1").arg(newThing));
                  const double pageWidth  = getTenthsFromInches(score()->styleD(Sid::pageWidth));
                  const double lm = getTenthsFromInches(score()->styleD(Sid::pageOddLeftMargin));
                  const double rm = getTenthsFromInches(score()->styleD(Sid::pageWidth)
                                                        - score()->styleD(Sid::pagePrintableWidth) - score()->styleD(Sid::pageOddLeftMargin));
                  const double tm = getTenthsFromInches(score()->styleD(Sid::pageOddTopMargin));

                  
                                                                                          const Measure* mmR1 = m->mmRest1();
                  const System* system = mmR1->system();

                                    if (idx == 0) {

                                                double systemLM = getTenthsFromDots(mmR1->pagePos().x() - system->page()->pagePos().x()) - lm;
                        double systemRM = pageWidth - rm - (getTenthsFromDots(system->bbox().width()) + lm);

                        xml.stag("system-layout");
                        xml.stag("system-margins");
                        xml.tag("left-margin", QString("%1").arg(QString::number(systemLM,'f',2)));
                        xml.tag("right-margin", QString("%1").arg(QString::number(systemRM,'f',2)) );
                        xml.etag();

                        if (currentSystem == NewPage || currentSystem == TopSystem) {
                              const double topSysDist = getTenthsFromDots(mmR1->pagePos().y()) - tm;
                              xml.tag("top-system-distance", QString("%1").arg(QString::number(topSysDist,'f',2)) );
                              }
                        if (currentSystem == NewSystem) {
                                                            const double sysDist = getTenthsFromDots(mmR1->pagePos().y()
                                                                       - previousMeasure->pagePos().y()
                                                                       - previousMeasure->bbox().height()
                                                                       + 2 * score()->spatium()
                                                                       );
                              xml.tag("system-distance",
                                      QString("%1").arg(QString::number(sysDist,'f',2)));
                              }

                        xml.etag();
                        }

                                    for (int staffIdx = (staffCount == 0) ? 1 : 0; staffIdx < staves; staffIdx++) {
                        xml.stag(QString("staff-layout number=\"%1\"").arg(staffIdx + 1));
                        const double staffDist = 0.0;
                        xml.tag("staff-distance", QString("%1").arg(QString::number(staffDist,'f',2)));
                        xml.etag();
                        }

                  xml.etag();
                  }
            else {
                                    if (newThing != "")
                        xml.tagE(QString("print%1").arg(newThing));
                  }

            } 
      }




void ExportMusicXml::findAndExportClef(Measure* m, const int staves, const int strack, const int etrack)
      {
      Measure* prevMeasure = m->prevMeasure();
      Measure* mmR         = m->mmRest();             int tick             = m->tick();
      Segment* cs1;
      Segment* cs2         = m->findSegment(SegmentType::Clef, tick);
      Segment* cs3;
      Segment* seg         = 0;

      if (prevMeasure)
            cs1 = prevMeasure->findSegment(SegmentType::Clef, tick);
      else
            cs1 = m->findSegment(SegmentType::HeaderClef, tick);

      if (mmR) {
            cs3 = mmR->findSegment(SegmentType::HeaderClef, tick);
            if (!cs3)
                  cs3 = mmR->findSegment(SegmentType::Clef, tick);
            }
      else
            cs3 = 0;

      if (cs1 && cs2) {
                                    seg = cs1;
            }
      else if (cs1)
            seg = cs1;
      else if (cs3) {
                                    seg = cs3;
            }
      else
            seg = cs2;
      clefDebug("exportxml: clef segments cs1=%p cs2=%p cs3=%p seg=%p", cs1, cs2, cs3, seg);

            if (seg) {
            for (int st = strack; st < etrack; st += VOICES) {
                                                                        
                  int sstaff = (staves > 1) ? st - strack + VOICES : 0;
                  sstaff /= VOICES;

                  Clef* cle = static_cast<Clef*>(seg->element(st));
                  if (cle) {
                        clefDebug("exportxml: clef at start measure ti=%d ct=%d gen=%d", tick, int(cle->clefType()), cle->generated());
                                                                        if (tick == 0 || !cle->generated()) {
                              clefDebug("exportxml: clef exported");
                              clef(sstaff, cle);
                              }
                        else {
                              clefDebug("exportxml: clef not exported");
                              }
                        }
                  }
            }
      }




typedef QSet<int> pitchSet;       
static void addChordPitchesToSet(const Chord* c, pitchSet& set)
      {
      for (const Note* note : c->notes()) {
            qDebug("chord %p note %p pitch %d", c, note, note->pitch() + 1);
            set.insert(note->pitch());
            }
      }

static void findPitchesUsed(const Part* part, pitchSet& set)
      {
      int strack = part->startTrack();
      int etrack = part->endTrack();

            for (const MeasureBase* mb = part->score()->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != ElementType::MEASURE)
                  continue;
            const Measure* m = static_cast<const Measure*>(mb);
            for (int st = strack; st < etrack; ++st) {
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
                        const Element* el = seg->element(st);
                        if (!el)
                              continue;
                        if (el->type() == ElementType::CHORD)
                              {
                                                            const Chord* c = static_cast<const Chord*>(el);
                              if (c) {
                                    for (const Chord* g : c->graceNotesBefore()) {
                                          addChordPitchesToSet(g, set);
                                          }
                                    addChordPitchesToSet(c, set);
                                    for (const Chord* g : c->graceNotesAfter()) {
                                          addChordPitchesToSet(g, set);
                                          }
                                    }
                              }
                        }
                  }
            }
      }




static void partList(XmlWriter& xml, Score* score, const QList<Part*>& il, MxmlInstrumentMap& instrMap)
      {
      xml.stag("part-list");
      int staffCount = 0;                                   int partGroupEnd[MAX_PART_GROUPS];                    for (int i = 0; i < MAX_PART_GROUPS; i++)
            partGroupEnd[i] = -1;
      for (int idx = 0; idx < il.size(); ++idx) {
            Part* part = il.at(idx);
            bool bracketFound = false;
                        for (int i = 0; i < part->nstaves(); i++) {
                  Staff* st = part->staff(i);
                  if (st) {
                        for (int j = 0; j < st->bracketLevels() + 1; j++) {
                              if (st->bracketType(j) != BracketType::NO_BRACKET) {
                                    bracketFound = true;
                                    if (i == 0) {
                                                                                                                              if (!(st->bracketSpan(j) == part->nstaves()
                                                && st->bracketType(j) == BracketType::BRACE)) {
                                                                                                int number = findPartGroupNumber(partGroupEnd);
                                                if (number < MAX_PART_GROUPS) {
                                                      partGroupStart(xml, number + 1, st->bracketType(j));
                                                      partGroupEnd[number] = staffCount + st->bracketSpan(j);
                                                      }
                                                }
                                          }
                                    else {
                                                                                    qDebug("bracket starting in staff %d not supported", i + 1);
                                          }
                                    }
                              }
                        }
                  }
                        if (!bracketFound && part->nstaves() > 1) {
                  int number = findPartGroupNumber(partGroupEnd);
                  if (number < MAX_PART_GROUPS) {
                        partGroupStart(xml, number + 1, BracketType::NO_BRACKET);
                        partGroupEnd[number] = idx + part->nstaves();
                        }
                  }

            xml.stag(QString("score-part id=\"P%1\"").arg(idx+1));
            initInstrMap(instrMap, part->instruments(), score);
                        if (part->longName() != "")
                  xml.tag("part-name", MScoreTextToMXML::toPlainText(part->longName()));
            else {
                  if (part->partName() != "") {
                                                                        xml.tag("part-name print-object=\"no\"", MScoreTextToMXML::toPlainText(part->partName()));
                        }
                  else
                                                xml.tag("part-name", "");
                  }
            if (!part->shortName().isEmpty())
                  xml.tag("part-abbreviation", MScoreTextToMXML::toPlainText(part->shortName()));

            if (part->instrument()->useDrumset()) {
                  const Drumset* drumset = part->instrument()->drumset();
                  pitchSet pitches;
                  findPitchesUsed(part, pitches);
                  for (int i = 0; i < 128; ++i) {
                        DrumInstrument di = drumset->drum(i);
                        if (di.notehead != NoteHead::Group::HEAD_INVALID)
                              scoreInstrument(xml, idx + 1, i + 1, di.name);
                        else if (pitches.contains(i))
                              scoreInstrument(xml, idx + 1, i + 1, QString("Instrument %1").arg(i + 1));
                        }
                  int midiPort = part->midiPort() + 1;
                  if (midiPort >= 1 && midiPort <= 16)
                        xml.tag(QString("midi-device port=\"%1\"").arg(midiPort), "");

                  for (int i = 0; i < 128; ++i) {
                        DrumInstrument di = drumset->drum(i);
                        if (di.notehead != NoteHead::Group::HEAD_INVALID || pitches.contains(i))
                              midiInstrument(xml, idx + 1, i + 1, part->instrument(), score, i + 1);
                        }
                  }
            else {
                  MxmlReverseInstrumentMap rim;
                  initReverseInstrMap(rim, instrMap);
                  for (int instNr : rim.keys()) {
                        scoreInstrument(xml, idx + 1, instNr + 1, MScoreTextToMXML::toPlainText(rim.value(instNr)->trackName()));
                        }
                  for (auto ii = rim.constBegin(); ii != rim.constEnd(); ii++) {
                        int instNr = ii.key();
                        int midiPort = part->midiPort() + 1;
                        if (ii.value()->channel().size() > 0)
                              midiPort = score->masterScore()->midiMapping(ii.value()->channel(0)->channel)->port + 1;
                        if (midiPort >= 1 && midiPort <= 16)
                              xml.tag(QString("midi-device %1 port=\"%2\"").arg(instrId(idx+1, instNr + 1)).arg(midiPort), "");
                        else
                              xml.tag(QString("midi-device %1").arg(instrId(idx+1, instNr + 1)), "");
                        midiInstrument(xml, idx + 1, instNr + 1, rim.value(instNr), score);
                        }
                  }

            xml.etag();
            staffCount += part->nstaves();
            for (int i = MAX_PART_GROUPS - 1; i >= 0; i--) {
                  int end = partGroupEnd[i];
                  if (end >= 0) {
                        if (staffCount >= end) {
                              xml.tagE(QString("part-group type=\"stop\" number=\"%1\"").arg(i + 1));
                              partGroupEnd[i] = -1;
                              }
                        }
                  }
            }
      xml.etag();

      }




void ExportMusicXml::writeElement(Element* el, const Measure* m, int sstaff, bool useDrumset)
      {
      switch (el->type()) {

            case ElementType::CLEF:
                  {
                                                                                                                              const Clef* cle = static_cast<const Clef*>(el);
                  int ti = cle->segment()->tick();
                  clefDebug("exportxml: clef in measure ti=%d ct=%d gen=%d", ti, int(cle->clefType()), el->generated());
                  if (el->generated()) {
                        clefDebug("exportxml: generated clef not exported");
                        break;
                        }
                  if (!el->generated() && ti != m->tick() && ti != m->endTick())
                        clef(sstaff, cle);
                  else {
                        clefDebug("exportxml: clef not exported");
                        }
                  }
                  break;

            case ElementType::KEYSIG:
                                    break;

            case ElementType::TIMESIG:
                                    break;

            case ElementType::CHORD:
                  {
                  Chord* c = static_cast<Chord*>(el);
                  const auto ll = &c->lyrics();
                                    if (c) {
                        for (Chord* g : c->graceNotesBefore()) {
                              chord(g, sstaff, ll, useDrumset);
                              }
                        chord(c, sstaff, ll, useDrumset);
                        for (Chord* g : c->graceNotesAfter()) {
                              chord(g, sstaff, ll, useDrumset);
                              }
                        }
                  break;
                  }
            case ElementType::REST:
                  rest((Rest*)el, sstaff);
                  break;

            case ElementType::BAR_LINE:
                                                                                                                                                                  break;
            case ElementType::BREATH:
                                    break;

            default:
                  qDebug("ExportMusicXml::write unknown segment type %s", el->name());
                  break;
            }

      }




static void writeStaffDetails(XmlWriter& xml, const Part* part)
      {
      const Instrument* instrument = part->instrument();
      int staves = part->nstaves();

                        for (int i = 0; i < staves; i++) {
            Staff* st = part->staff(i);
            if (st->lines(0) != 5 || st->isTabStaff(0)) {
                  if (staves > 1)
                        xml.stag(QString("staff-details number=\"%1\"").arg(i+1));
                  else
                        xml.stag("staff-details");
                  xml.tag("staff-lines", st->lines(0));
                  if (st->isTabStaff(0) && instrument->stringData()) {
                        QList<instrString> l = instrument->stringData()->stringList();
                        for (int i = 0; i < l.size(); i++) {
                              char step  = ' ';
                              int alter  = 0;
                              int octave = 0;
                              midipitch2xml(l.at(i).pitch, step, alter, octave);
                              xml.stag(QString("staff-tuning line=\"%1\"").arg(i+1));
                              xml.tag("tuning-step", QString("%1").arg(step));
                              if (alter)
                                    xml.tag("tuning-alter", alter);
                              xml.tag("tuning-octave", octave);
                              xml.etag();
                              }
                        }
                  xml.etag();
                  }
            }
      }




static void writeInstrumentDetails(XmlWriter& xml, const Part* part)
      {
      const Instrument* instrument = part->instrument();

            if (instrument->transpose().chromatic) {                    xml.stag("transpose");
            xml.tag("diatonic",  instrument->transpose().diatonic % 7);
            xml.tag("chromatic", instrument->transpose().chromatic % 12);
            int octaveChange = instrument->transpose().chromatic / 12;
            if (octaveChange != 0)
                  xml.tag("octave-change", octaveChange);
            xml.etag();
            }
      }




void ExportMusicXml::write(QIODevice* dev)
      {
                              bool concertPitch = score()->styleB(Sid::concertPitch);
      if (concertPitch) {
            score()->startCmd();
            score()->undo(new ChangeStyleVal(score(), Sid::concertPitch, false));
            score()->doLayout();                }

      calcDivisions();

      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            brackets[i] = 0;
            hairpins[i] = 0;
            ottavas[i] = 0;
            trills[i] = 0;
            }

      xml.setDevice(dev);
      xml.setCodec("UTF-8");
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml << "<!DOCTYPE score-partwise PUBLIC \"-      xml.stag("score-partwise version=\"3.1\"");

      const MeasureBase* measure = _score->measures()->first();
      work(measure);

      identification(xml, _score);

      if (preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT)) {
            defaults(xml, _score, millimeters, tenths);
            credits(xml);
            }

      const QList<Part*>& il = _score->parts();
      partList(xml, _score, il, instrMap);

      int staffCount = 0;

      for (int idx = 0; idx < il.size(); ++idx) {
            Part* part = il.at(idx);
            tick = 0;
            xml.stag(QString("part id=\"P%1\"").arg(idx+1));

            int staves = part->nstaves();
            int strack = part->startTrack();
            int etrack = part->endTrack();

            trillStart.clear();
            trillStop.clear();
            initInstrMap(instrMap, part->instruments(), _score);

            int measureNo = 1;                      int irregularMeasureNo = 1;             int pickupMeasureNo = 1;    
            FigBassMap fbMap;           
            for (MeasureBase* mb = _score->measures()->first(); mb; mb = mb->next()) {
                  if (mb->type() != ElementType::MEASURE)
                        continue;
                  Measure* m = static_cast<Measure*>(mb);


                                    QString measureTag = "measure number=";
                  if ((irregularMeasureNo + measureNo) == 2 && m->irregular()) {
                        measureTag += "\"0\" implicit=\"yes\"";
                        pickupMeasureNo++;
                        }
                  else if (m->irregular())
                        measureTag += QString("\"X%1\" implicit=\"yes\"").arg(irregularMeasureNo++);
                  else
                        measureTag += QString("\"%1\"").arg(measureNo++);
                  const bool isFirstActualMeasure = (irregularMeasureNo + measureNo + pickupMeasureNo) == 4;

                  if (preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT))
                        measureTag += QString(" width=\"%1\"").arg(QString::number(m->bbox().width() / DPMM / millimeters * tenths,'f',2));

                  xml.stag(measureTag);

                  print(m, idx, staffCount, staves);

                  attr.start();

                  findTrills(m, strack, etrack, trillStart, trillStop);

                                    barlineLeft(m);

                                    if (isFirstActualMeasure) {
                        attr.doAttr(xml, true);
                        xml.tag("divisions", MScore::division / div);
                        }

                                    keysigTimesig(m, part);

                                    if (isFirstActualMeasure) {
                        if (staves > 1)
                              xml.tag("staves", staves);
                        if (instrMap.size() > 1)
                              xml.tag("instruments", instrMap.size());
                        }

                                    findAndExportClef(m, staves, strack, etrack);

                                    if (isFirstActualMeasure) {
                        writeStaffDetails(xml, part);
                        writeInstrumentDetails(xml, part);
                        }

                                    measureStyle(xml, attr, m);

                                                      QSet<const Spanner*> spannersStopped;

                                                      if (idx == 0)
                        repeatAtMeasureStart(xml, attr, m, strack, etrack, strack);

                  for (int st = strack; st < etrack; ++st) {
                                                                                                
                        int sstaff = (staves > 1) ? st - strack + VOICES : 0;
                        sstaff /= VOICES;
                        for (Segment* seg = m->first(); seg; seg = seg->next()) {
                              Element* el = seg->element(st);
                              if (!el) {
                                    continue;
                                    }
                                                            if (el->type() == ElementType::BAR_LINE && static_cast<BarLine*>(el)->barLineType() == BarLineType::START_REPEAT)
                                    continue;

                                                            if (tick != seg->tick()) {
                                    attr.doAttr(xml, false);
                                    moveToTick(seg->tick());
                                    }

                                                            if (el->isChordRest()) {
                                    attr.doAttr(xml, false);
                                    annotations(this, xml, strack, etrack, st, sstaff, seg);
                                                                        for (Segment* seg1 = seg->next(); seg1; seg1 = seg1->next()) {
                                          if (seg1->isChordRestType()) {
                                                Element* el1 = seg1->element(st);
                                                if (el1)                                                       break;
                                                for (Element* annot : seg1->annotations()) {
                                                      if (annot->type() == ElementType::HARMONY && annot->track() == st)
                                                            harmony(static_cast<Harmony*>(annot), 0, (seg1->tick() - seg->tick()) / div);
                                                      }
                                                }
                                          }
                                    figuredBass(xml, strack, etrack, st, static_cast<const ChordRest*>(el), fbMap, div);
                                    spannerStart(this, strack, etrack, st, sstaff, seg);
                                    }

                                                            writeElement(el, m, sstaff, part->instrument()->useDrumset());

                                                            if (el->isChordRest()) {
                                    int spannerStaff = (st / VOICES) * VOICES;
                                    spannerStop(this, spannerStaff, tick, sstaff, spannersStopped);
                                    }

                              }                         attr.stop(xml);
                        }                   #ifdef DEBUG_TICK
                  qDebug("end of measure");
#endif
                  moveToTick(m->tick() + m->ticks());
                  if (idx == 0)
                        repeatAtMeasureStop(xml, m, strack, etrack, strack);
                                                      barlineRight(m);
                  xml.etag();
                  }
            staffCount += staves;
            xml.etag();
            }

      xml.etag();

      if (concertPitch) {
                        score()->endCmd(true);                    }
      }




bool saveXml(Score* score, const QString& name)
      {
      QFile f(name);
      if (!f.open(QIODevice::WriteOnly))
            return false;
      ExportMusicXml em(score);
      em.write(&f);
      return f.error() == QFile::NoError;
      }






bool saveMxl(Score* score, const QString& name)
      {
      MQZipWriter uz(name);

      QFileInfo fi(name);
#if 0
      QDateTime dt;
      if (MScore::debugMode)
            dt = QDateTime(QDate(2007, 9, 10), QTime(12, 0));
      else
            dt = QDateTime::currentDateTime();
#endif
      QString fn = fi.completeBaseName() + ".xml";

      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);
      XmlWriter xml(score);
      xml.setDevice(&cbuf);
      xml.setCodec("UTF-8");
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml.stag("container");
      xml.stag("rootfiles");
      xml.stag(QString("rootfile full-path=\"%1\"").arg(fn));
      xml.etag();
      xml.etag();
      xml.etag();
      cbuf.seek(0);
            uz.addFile("META-INF/container.xml", cbuf.data());

      QBuffer dbuf;
      dbuf.open(QIODevice::ReadWrite);
      ExportMusicXml em(score);
      em.write(&dbuf);
      dbuf.seek(0);
      uz.addFile(fn, dbuf.data());
      uz.close();
      return true;
      }

double ExportMusicXml::getTenthsFromInches(double inches) const
      {
      return inches * INCH / millimeters * tenths;
      }

double ExportMusicXml::getTenthsFromDots(double dots) const
      {
      return dots / DPMM / millimeters * tenths;
      }


void ExportMusicXml::harmony(Harmony const* const h, FretDiagram const* const fd, int offset)
      {
                                                                        int rootTpc = h->rootTpc();
      if (rootTpc != Tpc::TPC_INVALID) {
            QString tagName = "harmony";
            bool frame = h->hasFrame();
            tagName += QString(" print-frame=\"%1\"").arg(frame ? "yes" : "no");             tagName += color2xml(h);
            xml.stag(tagName);
            xml.stag("root");
            xml.tag("root-step", tpc2stepName(rootTpc));
            int alter = int(tpc2alter(rootTpc));
            if (alter)
                  xml.tag("root-alter", alter);
            xml.etag();

            if (!h->xmlKind().isEmpty()) {
                  QString s = "kind";
                  QString kindText = h->musicXmlText();
                  if (h->musicXmlText() != "")
                        s += " text=\"" + kindText + "\"";
                  if (h->xmlSymbols() == "yes")
                        s += " use-symbols=\"yes\"";
                  if (h->xmlParens() == "yes")
                        s += " parentheses-degrees=\"yes\"";
                  xml.tag(s, h->xmlKind());
                  QStringList l = h->xmlDegrees();
                  if (!l.isEmpty()) {
                        for (QString tag : l) {
                              QString degreeText;
                              if (h->xmlKind().startsWith("suspended")
                                  && tag.startsWith("add") && tag[3].isDigit()
                                  && !kindText.isEmpty() && kindText[0].isDigit()) {
                                                                                                            int tagDegree = tag.mid(3).toInt();
                                    QString kindTextExtension;
                                    for (int i = 0; i < kindText.length() && kindText[i].isDigit(); ++i)
                                          kindTextExtension[i] = kindText[i];
                                    int kindExtension = kindTextExtension.toInt();
                                    if (tagDegree <= kindExtension && (tagDegree & 1) && (kindExtension & 1))
                                          degreeText = " text=\"\"";
                                    }
                              xml.stag("degree");
                              int alter = 0;
                              int idx = 3;
                              if (tag[idx] == '#') {
                                    alter = 1;
                                    ++idx;
                                    }
                              else if (tag[idx] == 'b') {
                                    alter = -1;
                                    ++idx;
                                    }
                              xml.tag(QString("degree-value%1").arg(degreeText), tag.mid(idx));
                              xml.tag("degree-alter", alter);                                   if (tag.startsWith("add"))
                                    xml.tag(QString("degree-type%1").arg(degreeText), "add");
                              else if (tag.startsWith("sub"))
                                    xml.tag("degree-type", "subtract");
                              else if (tag.startsWith("alt"))
                                    xml.tag("degree-type", "alter");
                              xml.etag();
                              }
                        }
                  }
            else {
                  if (h->extensionName() == 0)
                        xml.tag("kind", "");
                  else
                        xml.tag(QString("kind text=\"%1\"").arg(h->extensionName()), "");
                  }

            int baseTpc = h->baseTpc();
            if (baseTpc != Tpc::TPC_INVALID) {
                  xml.stag("bass");
                  xml.tag("bass-step", tpc2stepName(baseTpc));
                  int alter = int(tpc2alter(baseTpc));
                  if (alter) {
                        xml.tag("bass-alter", alter);
                        }
                  xml.etag();
                  }
            if (offset > 0)
                  xml.tag("offset", offset);
            if (fd)
                  fd->writeMusicXML(xml);

            xml.etag();
            }
      else {
                                                            if (h->hasFrame())
                  xml.stag(QString("harmony print-frame=\"yes\""));                 else
                  xml.stag(QString("harmony print-frame=\"no\""));                  xml.stag("root");
            xml.tag("root-step text=\"\"", "C");
            xml.etag();                   QString k = "kind text=\"" + h->hTextName() + "\"";
            xml.tag(k, "none");
            xml.etag();       #if 0
                        xml.stag("direction");
            xml.stag("direction-type");
            xml.tag("words", h->text());
            xml.etag();
            xml.etag();
#endif
            }
#if 0
            xml.tag(QString("kind text=\"%1\"").arg(h->extensionName()), extension);
      for (int i = 0; i < h->numberOfDegrees(); i++) {
            HDegree hd = h->degree(i);
            HDegreeType tp = hd.type();
            if (tp == HDegreeType::ADD || tp == HDegreeType::ALTER || tp == HDegreeType::SUBTRACT) {
                  xml.stag("degree");
                  xml.tag("degree-value", hd.value());
                  xml.tag("degree-alter", hd.alter());
                  switch (tp) {
                        case HDegreeType::ADD:
                              xml.tag("degree-type", "add");
                              break;
                        case HDegreeType::ALTER:
                              xml.tag("degree-type", "alter");
                              break;
                        case HDegreeType::SUBTRACT:
                              xml.tag("degree-type", "subtract");
                              break;
                        default:
                              break;
                        }
                  xml.etag();
                  }
            }
#endif
      }

}

