


#include "arpeggio.h"
#include "glissando.h"
#include "chord.h"
#include "ledgerline.h"
#include "note.h"
#include "notedot.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "system.h"
#include "style.h"
#include "sym.h"
#include "xml.h"
#include "accidental.h"

namespace Ms {

static const qreal      GLISS_PALETTE_WIDTH           = 4.0;
static const qreal      GLISS_PALETTE_HEIGHT          = 4.0;



void GlissandoSegment::layout()
      {
      if (staff())
            setMag(staff()->mag(tick()));
      QRectF r = QRectF(0.0, 0.0, pos2().x(), pos2().y()).normalized();
      qreal lw = glissando()->lineWidth() * .5;
      setbbox(r.adjusted(-lw, -lw, lw, lw));
      }


void GlissandoSegment::draw(QPainter* painter) const
      {
      painter->save();
      qreal _spatium = spatium();

      QPen pen(glissando()->curColor());
      pen.setWidthF(glissando()->lineWidth());
      pen.setCapStyle(Qt::RoundCap);
      painter->setPen(pen);

            qreal w     = pos2().x();
      qreal h     = pos2().y();
      qreal l     = sqrt(w * w + h * h);
      qreal wi    = asin(-h / l) * 180.0 / M_PI;
      qreal scale = painter->worldTransform().m11();
      painter->rotate(-wi);

      if (glissando()->glissandoType() == GlissandoType::STRAIGHT) {
            painter->drawLine(QLineF(0.0, 0.0, l, 0.0));
            }
      else if (glissando()->glissandoType() == GlissandoType::WAVY) {
            QRectF b = symBbox(SymId::wiggleTrill);
            qreal w  = symAdvance(SymId::wiggleTrill);
            int n    = (int)(l / w);                  qreal x  = (l - n*w) * 0.5;               std::vector<SymId> ids;
            for (int i = 0; i < n; ++i)
                  ids.push_back(SymId::wiggleTrill);
                        score()->scoreFont()->draw(ids, painter, magS(), QPointF(x, -(b.y() + b.height()*0.5) ), scale);
            }

      if (glissando()->showText()) {
            QFont f(glissando()->fontFace());
            f.setPointSizeF(glissando()->fontSize() * MScore::pixelRatio * _spatium / SPATIUM20);
            f.setBold(glissando()->fontBold());
            f.setItalic(glissando()->fontItalic());
            f.setUnderline(glissando()->fontUnderline());
            QFontMetricsF fm(f);
            QRectF r = fm.boundingRect(glissando()->text());

                        if (r.width() < l) {
                  qreal yOffset = r.height() + r.y();                                           yOffset += _spatium * (glissando()->glissandoType() == GlissandoType::WAVY ? 0.4 : 0.1);
                  painter->setFont(f);
                  qreal x = (l - r.width()) * 0.5;
                  painter->drawText(QPointF(x, -yOffset), glissando()->text());
                  }
            }
      painter->restore();
      }


QVariant GlissandoSegment::getProperty(Pid id) const
      {
      switch (id) {
                        case Pid::GLISS_TYPE:
            case Pid::GLISS_TEXT:
            case Pid::GLISS_SHOW_TEXT:
            case Pid::GLISSANDO_STYLE:
            case Pid::PLAY:
            case Pid::FONT_FACE:
            case Pid::FONT_SIZE:
            case Pid::FONT_BOLD:
            case Pid::FONT_ITALIC:
            case Pid::FONT_UNDERLINE:
            case Pid::LINE_WIDTH:
                  return glissando()->getProperty(id);
            default:
                  return LineSegment::getProperty(id);
            }
      }


bool GlissandoSegment::setProperty(Pid id, const QVariant& v)
      {
      switch (id) {
            case Pid::GLISS_TYPE:
            case Pid::GLISS_TEXT:
            case Pid::GLISS_SHOW_TEXT:
            case Pid::GLISSANDO_STYLE:
            case Pid::PLAY:
            case Pid::FONT_FACE:
            case Pid::FONT_SIZE:
            case Pid::FONT_BOLD:
            case Pid::FONT_ITALIC:
            case Pid::FONT_UNDERLINE:
            case Pid::LINE_WIDTH:
                  return glissando()->setProperty(id, v);
            default:
                  return LineSegment::setProperty(id, v);
            }
      }


QVariant GlissandoSegment::propertyDefault(Pid id) const
      {
      switch (id) {
            case Pid::GLISS_TYPE:
            case Pid::GLISS_TEXT:
            case Pid::GLISS_SHOW_TEXT:
            case Pid::GLISSANDO_STYLE:
            case Pid::PLAY:
            case Pid::FONT_FACE:
            case Pid::FONT_SIZE:
            case Pid::FONT_BOLD:
            case Pid::FONT_ITALIC:
            case Pid::FONT_UNDERLINE:
            case Pid::LINE_WIDTH:
                  return glissando()->propertyDefault(id);
            default:
                  return LineSegment::propertyDefault(id);
            }
      }


PropertyFlags& GlissandoSegment::propertyFlags(Pid id)
      {
      switch (id) {
            case Pid::FONT_FACE:
            case Pid::FONT_SIZE:
            case Pid::FONT_BOLD:
            case Pid::FONT_ITALIC:
            case Pid::FONT_UNDERLINE:
            case Pid::LINE_WIDTH:
                  return glissando()->propertyFlags(id);

            default:
                  return LineSegment::propertyFlags(id);
            }
      }


void GlissandoSegment::setPropertyFlags(Pid id, PropertyFlags f)
      {
      switch (id) {
            case Pid::FONT_FACE:
            case Pid::FONT_SIZE:
            case Pid::FONT_BOLD:
            case Pid::FONT_ITALIC:
            case Pid::FONT_UNDERLINE:
            case Pid::LINE_WIDTH:
                  glissando()->setPropertyFlags(id, f);
                  break;

            default:
                  LineSegment::setPropertyFlags(id, f);
                  break;
            }
      }


Sid GlissandoSegment::getPropertyStyle(Pid id) const
      {
      switch (id) {
            case Pid::FONT_FACE:
            case Pid::FONT_SIZE:
            case Pid::FONT_BOLD:
            case Pid::FONT_ITALIC:
            case Pid::FONT_UNDERLINE:
            case Pid::LINE_WIDTH:
                  return glissando()->getPropertyStyle(id);

            default:
                  return LineSegment::getPropertyStyle(id);
            }
      }


Glissando::Glissando(Score* s)
  : SLine(s, ElementFlag::MOVABLE | ElementFlag::SELECTABLE)
      {
      setAnchor(Spanner::Anchor::NOTE);
      setDiagonal(true);

      initSubStyle(SubStyleId::GLISSANDO);

      resetProperty(Pid::GLISS_SHOW_TEXT);
      resetProperty(Pid::PLAY);
      resetProperty(Pid::GLISSANDO_STYLE);
      resetProperty(Pid::GLISS_TYPE);
      resetProperty(Pid::GLISS_TEXT);
      }

Glissando::Glissando(const Glissando& g)
   : SLine(g)
      {
      _text           = g._text;
      _fontFace       = g._fontFace;
      _fontSize       = g._fontSize;
      _glissandoType  = g._glissandoType;
      _glissandoStyle = g._glissandoStyle;
      _showText       = g._showText;
      _playGlissando  = g._playGlissando;
      _fontBold       = g._fontBold;
      _fontItalic     = g._fontItalic;
      _fontUnderline  = g._fontUnderline;
      }


LineSegment* Glissando::createLineSegment()
      {
      GlissandoSegment* seg = new GlissandoSegment(score());
      seg->setFlag(ElementFlag::ON_STAFF, false);
      seg->setTrack(track());
      seg->setColor(color());
      return seg;
      }


void Glissando::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      func(data, this);
            for (SpannerSegment* seg : segments) {
            if (!seg->parent() || !seg->parent()->isSystem())
                  seg->scanElements(data, func, all);
            }
      }


void Glissando::layout()
      {
      qreal _spatium = spatium();

      if (score() == gscore || !startElement() || !endElement()) {              if (spannerSegments().empty())
                  add(createLineSegment());
            LineSegment* s = frontSegment();
            s->setPos(QPointF());
            s->setPos2(QPointF(_spatium * GLISS_PALETTE_WIDTH, -_spatium * GLISS_PALETTE_HEIGHT));
            s->layout();
            return;
            }
      if (spannerSegments().empty()) {
            qDebug("no segments");
            return;
            }
      SLine::layout();
      setPos(0.0, 0.0);

      Note*       anchor1     = toNote(startElement());
      Note*       anchor2     = toNote(endElement());
      Chord*      cr1         = anchor1->chord();
      Chord*      cr2         = anchor2->chord();
      GlissandoSegment* segm1 = toGlissandoSegment(frontSegment());
      GlissandoSegment* segm2 = toGlissandoSegment(backSegment());

                  
      
                  QPointF offs1 = QPointF(anchor1->headWidth() * 0.5, 0.0);
      QPointF offs2 = QPointF(anchor2->headWidth() * 0.5, 0.0);

      
      int upDown = (0 < (anchor2->pitch() - anchor1->pitch())) - ((anchor2->pitch() - anchor1->pitch()) < 0);
                  if (cr1->staff()->isTabStaff(cr1->tick())) {
            qreal yOff = cr1->staff()->lineDistance(cr1->tick()) * 0.4 * _spatium;
            offs1.ry() += yOff * upDown;
            offs2.ry() -= yOff * upDown;
            }
            else {
            if (anchor1->line() == anchor2->line()) {
                  offs1.ry() += _spatium * 0.25 * upDown;
                  offs2.ry() -= _spatium * 0.25 * upDown;
                  }
            }

            segm1->setPos (segm1->ipos()  + offs1);
      segm1->setPos2(segm1->ipos2() - offs1);
            segm2->setPos2(segm2->ipos2() + offs2);

                  if (cr2->segment()->measure() == cr2->segment()->system()->firstMeasure() && cr2->rtick() == 0
                           && !(cr2->noteType() == NoteType::GRACE8_AFTER
            || cr2->noteType() == NoteType::GRACE16_AFTER || cr2->noteType() == NoteType::GRACE32_AFTER)
                  && !(cr1->parent() == cr2))
            {
            segm2->rxpos() -= GLISS_STARTOFSYSTEM_WIDTH * _spatium;
            segm2->rxpos2()+= GLISS_STARTOFSYSTEM_WIDTH * _spatium;
            }

                  
            qreal xTot = 0.0;
      for (SpannerSegment* segm : spannerSegments())
            xTot += segm->ipos2().x();
      qreal y0   = segm1->ipos().y();
      qreal yTot = segm2->ipos().y() + segm2->ipos2().y() - y0;
      qreal ratio = yTot / xTot;
            qreal xCurr = 0.0;
      qreal yCurr;
      for (int i = 0; i < spannerSegments().count()-1; i++) {
            SpannerSegment* segm = segmentAt(i);
            xCurr += segm->ipos2().x();
            yCurr = y0 + ratio * xCurr;
            segm->rypos2() = yCurr - segm->ipos().y();                               segm = segmentAt(i+1);
            segm->rypos2() += segm->ipos().y() - yCurr;                  segm->rypos() = yCurr;                                       }

      
            offs1 *= -1.0;                int dots = cr1->dots();
      LedgerLine * ledLin = cr1->ledgerLines();
                  offs1.rx() += (dots && anchor1->dot(dots-1) ? anchor1->dot(dots-1)->pos().x() + anchor1->dot(dots-1)->width()
                  : (ledLin ? ledLin->pos().x() + ledLin->width() : anchor1->headWidth()) );

            offs2 *= -1.0;                if (Arpeggio* a = cr2->arpeggio())
            offs2.rx() += a->pos().x() + a->userOff().x();
      else if (Accidental* a = anchor2->accidental())
            offs2.rx() += a->pos().x() + a->userOff().x();
      else if ( (ledLin = cr2->ledgerLines()) != nullptr)
            offs2.rx() += ledLin->pos().x();

            offs1.rx() += _spatium * 0.25;
      offs2.rx() -= _spatium * 0.25;

            offs1.ry() = segm1->ipos2().y() * offs1.x() / segm1->ipos2().x();
      segm1->setPos(segm1->ipos() + offs1);
      segm1->setPos2(segm1->ipos2() - offs1);
            offs2.ry() = segm2->ipos2().y() * offs2.x() / segm2->ipos2().x();
      segm2->setPos2(segm2->ipos2() + offs2);

      for (SpannerSegment* segm : spannerSegments())
            segm->layout();

            QPointF anchor2PagePos = anchor2->pagePos();
      QPointF system2PagePos = cr2->segment()->system()->pagePos();
      QPointF anchor2SystPos = anchor2PagePos - system2PagePos;
      QRectF r = QRectF(anchor2SystPos - segm2->pos(), anchor2SystPos - segm2->pos() - segm2->pos2()).normalized();
      qreal lw = lineWidth() * .5;
      setbbox(r.adjusted(-lw, -lw, lw, lw));
      }


void Glissando::write(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      if (_showText && !_text.isEmpty())
            xml.tag("text", _text);

      for (auto id : { Pid::GLISS_TYPE, Pid::PLAY, Pid::GLISSANDO_STYLE } )
            writeProperty(xml, id);

      SLine::writeProperties(xml);
      xml.etag();
      }


void Glissando::read(XmlReader& e)
      {
      qDeleteAll(spannerSegments());
      spannerSegments().clear();
      e.addSpanner(e.intAttribute("id", -1), this);

      _showText = false;
      while (e.readNextStartElement()) {
            const QStringRef& tag = e.name();
            if (tag == "text") {
                  _showText = true;
                  _text = e.readElementText();
                  }
            else if (tag == "subtype")
                  _glissandoType = GlissandoType(e.readInt());
            else if (tag == "glissandoStyle")
                  setProperty(Pid::GLISSANDO_STYLE, Ms::getProperty(Pid::GLISSANDO_STYLE, e));
            else if (tag == "play")
                  setPlayGlissando(e.readBool());
            else if (readStyledProperty(e, tag))
                  ;
            else if (!SLine::readProperties(e))
                  e.unknown();
            }
      }


Note* Glissando::guessInitialNote(Chord* chord)
      {
      switch (chord->noteType()) {
                        case NoteType::ACCIACCATURA:
            case NoteType::APPOGGIATURA:
            case NoteType::GRACE4:
            case NoteType::GRACE16:
            case NoteType::GRACE32:
                                    if (chord->parent() && chord->parent()->isChord())
                        chord = toChord(chord->parent());
                  else
                        return 0;
                  break;
                        case NoteType::GRACE8_AFTER:
            case NoteType::GRACE16_AFTER:
            case NoteType::GRACE32_AFTER:
                  if (chord->parent() && chord->parent()->isChord())
                        return toChord(chord->parent())->upNote();
                  else                                                  return nullptr;
            case NoteType::NORMAL:
                  {
                                    QVector<Chord*> graces = chord->graceNotesBefore();
                  if (graces.size() > 0)
                        return graces.last()->upNote();
                  }
                  break;                                    default:
                  break;
            }

      
            if (!chord->parent()->isSegment())
            return 0;

      int         chordTrack  = chord->track();
      Segment*    segm        = chord->segment();
      Part*       part        = chord->part();
      if (segm != nullptr)
            segm = segm->prev1();
      while (segm) {
                        if (segm->segmentType() == SegmentType::ChordRest) {
                  Chord* target = nullptr;
                                    if (segm->element(chordTrack) && segm->element(chordTrack)->isChord())
                        target = toChord(segm->element(chordTrack));
                  else {                                     for (Element* currChord : segm->elist()) {
                              if (currChord && currChord->isChord() && toChord(currChord)->part() == part) {
                                    target = toChord(currChord);
                                    break;
                                    }
                              }
                        }
                                    if (target) {
                                                QVector<Chord*>graces = target->graceNotesAfter();
                        if (graces.size() > 0)
                              return graces.last()->upNote();
                        return target->upNote();                              }
                  }
            segm = segm->prev1();
            }
      qDebug("no first note for glissando found");
      return 0;
      }


Note* Glissando::guessFinalNote(Chord* chord)
      {
      switch (chord->noteType()) {
                                                            case NoteType::ACCIACCATURA:
            case NoteType::APPOGGIATURA:
            case NoteType::GRACE4:
            case NoteType::GRACE16:
            case NoteType::GRACE32:
                  if (chord->parent() && chord->parent()->isChord())
                        return toChord(chord->parent())->upNote();
                  else                                                  return nullptr;
                                    case NoteType::GRACE8_AFTER:
            case NoteType::GRACE16_AFTER:
            case NoteType::GRACE32_AFTER:
                                    if (chord->parent() && chord->parent()->isChord())
                        chord = toChord(chord->parent());
                  else
                        return 0;
                  break;
            case NoteType::NORMAL:
                  {
                                    QVector<Chord*>graces = chord->graceNotesAfter();
                  if (graces.size() > 0)
                        return graces.first()->upNote();
                  }
                  break;
            default:
                  break;
            }

      
            if (!chord->parent()->isSegment())
            return 0;

            Segment*    segm        = chord->score()->tick2rightSegment(chord->tick() + chord->actualTicks());
      int         chordTrack  = chord->track();
      Part*       part        = chord->part();
      while (segm) {
                        if (segm->segmentType() == SegmentType::ChordRest) {
                  Chord* target = nullptr;

                                    if (segm->element(chordTrack) && segm->element(chordTrack)->isChord())
                        target = toChord(segm->element(chordTrack));
                  else {                                      for (Element* currChord : segm->elist()) {
                              if (currChord && currChord->isChord() && toChord(currChord)->part() == part) {
                                    target = toChord(currChord);
                                    break;
                                    }
                              }
                        }

                                    if (target) {
                                                QVector<Chord*>graces = target->graceNotesBefore();
                        if (graces.size() > 0)
                              return graces.first()->upNote();
                        return target->upNote();                              }
                  }
            segm = segm->next1();
            }
      qDebug("no second note for glissando found");
      return 0;
      }


QVariant Glissando::getProperty(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::GLISS_TYPE:
                  return int(glissandoType());
            case Pid::GLISS_TEXT:
                  return text();
            case Pid::GLISS_SHOW_TEXT:
                  return showText();
            case Pid::GLISSANDO_STYLE:
                  return int(glissandoStyle());
            case Pid::PLAY:
                  return bool(playGlissando());
            case Pid::FONT_FACE:
                  return _fontFace;
            case Pid::FONT_SIZE:
                  return _fontSize;
            case Pid::FONT_BOLD:
                  return _fontBold;
            case Pid::FONT_ITALIC:
                  return _fontItalic;
            case Pid::FONT_UNDERLINE:
                  return _fontUnderline;
            default:
                  break;
            }
      return SLine::getProperty(propertyId);
      }


bool Glissando::setProperty(Pid propertyId, const QVariant& v)
      {
      switch (propertyId) {
            case Pid::GLISS_TYPE:
                  setGlissandoType(GlissandoType(v.toInt()));
                  break;
            case Pid::GLISS_TEXT:
                  setText(v.toString());
                  break;
            case Pid::GLISS_SHOW_TEXT:
                  setShowText(v.toBool());
                  break;
            case Pid::GLISSANDO_STYLE:
                 setGlissandoStyle(GlissandoStyle(v.toInt()));
                 break;
            case Pid::PLAY:
                 setPlayGlissando(v.toBool());
                 break;
            case Pid::FONT_FACE:
                  setFontFace(v.toString());
                  break;
            case Pid::FONT_SIZE:
                  setFontSize(v.toReal());
                  break;
            case Pid::FONT_BOLD:
                  setFontBold(v.toBool());
                  break;
            case Pid::FONT_ITALIC:
                  setFontItalic(v.toBool());
                  break;
            case Pid::FONT_UNDERLINE:
                  setFontUnderline(v.toBool());
                  break;
            default:
                  if (!SLine::setProperty(propertyId, v))
                        return false;
                  break;
            }
      score()->setLayoutAll();
      return true;
      }


QVariant Glissando::propertyDefault(Pid propertyId) const
      {
      switch (propertyId) {
            case Pid::GLISS_TYPE:
                  return int(GlissandoType::STRAIGHT);
            case Pid::GLISS_SHOW_TEXT:
                  return true;
            case Pid::GLISSANDO_STYLE:
                  return int(GlissandoStyle::CHROMATIC);
            case Pid::PLAY:
                  return true;
            default:
                  break;
            }
      return SLine::propertyDefault(propertyId);
      }
}

