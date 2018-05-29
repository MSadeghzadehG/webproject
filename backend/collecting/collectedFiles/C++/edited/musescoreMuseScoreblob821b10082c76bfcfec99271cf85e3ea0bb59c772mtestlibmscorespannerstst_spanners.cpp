
#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/chord.h"
#include "libmscore/excerpt.h"
#include "libmscore/glissando.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/lyrics.h"
#include "libmscore/measure.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"
#include "libmscore/score.h"
#include "libmscore/system.h"
#include "libmscore/undo.h"

#define DIR QString("libmscore/spanners/")

using namespace Ms;


class TestSpanners : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void spanners01();                  void spanners02();                  void spanners03();                  void spanners04();                  void spanners05();                  void spanners06();                  void spanners07();                  void spanners09();                  void spanners10();                  void spanners11();                  void spanners12();                  void spanners14();                  };


void TestSpanners::initTestCase()
      {
      initMTest();
      }


void TestSpanners::spanners01()
      {
      EditData    dropData(0);
      Glissando*  gliss;

      MasterScore* score = readScore(DIR + "glissando01.mscx");
      QVERIFY(score);

                  Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      Segment*    seg   = msr->findSegment(SegmentType::ChordRest, 0);
      QVERIFY(seg);
      Ms::Chord*      chord = static_cast<Ms::Chord*>(seg->element(0));
      QVERIFY(chord && chord->type() == ElementType::CHORD);
      Note*       note  = chord->upNote();
      QVERIFY(note);
            gliss             = new Glissando(score);       dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

                  msr   = msr->nextMeasure();
      QVERIFY(msr);
      seg   = msr->first();
      QVERIFY(seg);
      chord = static_cast<Ms::Chord*>(seg->element(0));         QVERIFY(chord && chord->type() == ElementType::CHORD);
      note  = chord->upNote();
      QVERIFY(note);
            gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

                  msr   = msr->nextMeasure();
      QVERIFY(msr);
      seg   = msr->first();
      QVERIFY(seg);
      chord = static_cast<Ms::Chord*>(seg->element(4));         QVERIFY(chord && chord->type() == ElementType::CHORD);
      note  = chord->upNote();
      QVERIFY(note);
            gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

                  msr   = msr->nextMeasure();
      QVERIFY(msr);
      seg   = msr->first();
      QVERIFY(seg);
      chord = static_cast<Ms::Chord*>(seg->element(0));         QVERIFY(chord && chord->type() == ElementType::CHORD);
      note  = chord->upNote();
      QVERIFY(note);
            gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

                  msr   = msr->nextMeasure()->nextMeasure();
      QVERIFY(msr);
      seg   = msr->first();
      QVERIFY(seg);
      chord = static_cast<Ms::Chord*>(seg->element(0));         QVERIFY(chord && chord->type() == ElementType::CHORD);
      note  = chord->upNote();
      QVERIFY(note);
            gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      QVERIFY(saveCompareScore(score, "glissando01.mscx", DIR + "glissando01-ref.mscx"));
      delete score;
      }


void TestSpanners::spanners02()
      {
      MasterScore* score = readScore(DIR + "glissando-crossstaff01.mscx");
      QVERIFY(score);

      QVERIFY(saveCompareScore(score, "glissando-crsossstaff01.mscx", DIR + "glissando-crossstaff01-ref.mscx"));
      delete score;
      }


void TestSpanners::spanners03()
      {
      EditData    dropData(0);
      Glissando*  gliss;

      MasterScore* score = readScore(DIR + "glissando-graces01.mscx");
      QVERIFY(score);

                  Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      Segment*    seg   = msr->findSegment(SegmentType::ChordRest, 0);
      QVERIFY(seg);
      Ms::Chord*      chord = static_cast<Ms::Chord*>(seg->element(0));
      QVERIFY(chord && chord->type() == ElementType::CHORD);
      Note*       note  = chord->upNote();
      QVERIFY(note);
            gliss             = new Glissando(score);       dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

                  Ms::Chord*      grace = chord->graceNotesAfter().last();
      QVERIFY(grace && grace->type() == ElementType::CHORD);
      note              = grace->upNote();
      QVERIFY(note);
      gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

                  seg               = seg->nextCR(0);
      QVERIFY(seg);
      chord             = static_cast<Ms::Chord*>(seg->element(0));
      QVERIFY(chord && chord->type() == ElementType::CHORD);
      note              = chord->upNote();
      QVERIFY(note);
      gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

                  seg               = seg->nextCR(0);
      QVERIFY(seg);
      chord             = static_cast<Ms::Chord*>(seg->element(0));
      QVERIFY(chord && chord->type() == ElementType::CHORD);
            grace             = chord->graceNotesBefore().last();
      QVERIFY(grace && grace->type() == ElementType::CHORD);
      note              = grace->upNote();
      QVERIFY(note);
      gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      QVERIFY(saveCompareScore(score, "glissando-graces01.mscx", DIR + "glissando-graces01-ref.mscx"));
      delete score;
      }


void TestSpanners::spanners04()
      {
      MasterScore* score = readScore(DIR + "glissando-cloning01.mscx");
      QVERIFY(score);

                  Staff* oldStaff   = score->staff(0);
      Staff* newStaff   = new Staff(score);
      newStaff->setPart(oldStaff->part());
      newStaff->initFromStaffType(oldStaff->staffType(0));
      newStaff->setDefaultClefType(ClefTypeList(ClefType::G));

      KeySigEvent ke;
      ke.setKey(Key::C);
      newStaff->setKey(0, ke);

      score->undoInsertStaff(newStaff, 1, false);
      Excerpt::cloneStaff(oldStaff, newStaff);

      QVERIFY(saveCompareScore(score, "glissando-cloning01.mscx", DIR + "glissando-cloning01-ref.mscx"));
      delete score;
      }


void TestSpanners::spanners05()
      {
      MasterScore* score = readScore(DIR + "glissando-cloning02.mscx");
      QVERIFY(score);

                  QList<Part*> parts;
      parts.append(score->parts().at(0));
      Score* nscore = new Score(score);

      Excerpt* ex = new Excerpt(score);
      ex->setPartScore(nscore);
      ex->setTitle(parts.front()->longName());
      ex->setParts(parts);
      Excerpt::createExcerpt(ex);
      QVERIFY(nscore);


      score->Score::undo(new AddExcerpt(ex));

      QVERIFY(saveCompareScore(score, "glissando-cloning02.mscx", DIR + "glissando-cloning02-ref.mscx"));
      delete score;
      }


void TestSpanners::spanners06()
      {
      EditData    dropData(0);
      Glissando*  gliss;

      MasterScore* score = readScore(DIR + "glissando-cloning03.mscx");
      QVERIFY(score);

            Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      Segment*    seg   = msr->findSegment(SegmentType::ChordRest, 0);
      QVERIFY(seg);
      Ms::Chord*      chord = static_cast<Ms::Chord*>(seg->element(0));
      QVERIFY(chord && chord->type() == ElementType::CHORD);
      Note*       note  = chord->upNote();
      QVERIFY(note);
            gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      QVERIFY(saveCompareScore(score, "glissando-cloning03.mscx", DIR + "glissando-cloning03-ref.mscx"));
      delete score;
      }


void TestSpanners::spanners07()
      {
      EditData    dropData(0);
      Glissando*  gliss;

      MasterScore* score = readScore(DIR + "glissando-cloning04.mscx");
      QVERIFY(score);

            Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      Segment*    seg   = msr->findSegment(SegmentType::ChordRest, 0);
      QVERIFY(seg);
      Ms::Chord*      chord = static_cast<Ms::Chord*>(seg->element(0));
      QVERIFY(chord && chord->type() == ElementType::CHORD);
      Note*       note  = chord->upNote();
      QVERIFY(note);
            gliss             = new Glissando(score);
      dropData.pos      = note->pagePos();
      dropData.element  = gliss;
      note->drop(dropData);

      QVERIFY(saveCompareScore(score, "glissando-cloning04.mscx", DIR + "glissando-cloning04-ref.mscx"));
      delete score;
      }
#if 0

void TestSpanners::spanners08()
      {
      MasterScore* score = readScore(DIR + "lyricsline01.mscx");
      QVERIFY(score);

            System* sys = score->systems().at(0);
      QVERIFY(sys->spannerSegments().size() == 1);
      QVERIFY(score->unmanagedSpanners().size() == 1);

            Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      Segment*    seg   = msr->findSegment(SegmentType::ChordRest, 0);
      QVERIFY(seg);
      Ms::Chord*      chord = static_cast<Ms::Chord*>(seg->element(0));
      QVERIFY(chord && chord->type() == ElementType::CHORD);
      QVERIFY(chord->lyrics().size() > 0);
      Lyrics*     lyr   = chord->lyrics(0, Element::Placement::BELOW);
      score->startCmd();
      score->undoRemoveElement(lyr);
      score->endCmd();

            QVERIFY(sys->spannerSegments().size() == 0);
      QVERIFY(score->unmanagedSpanners().size() == 0);

            QVERIFY(saveCompareScore(score, "lyricsline01.mscx", DIR + "lyricsline01-ref.mscx"));

            score->undoStack()->undo();
      score->doLayout();

            QVERIFY(sys->spannerSegments().size() == 1);
      QVERIFY(score->unmanagedSpanners().size() == 1);

            QVERIFY(saveCompareScore(score, "lyricsline01.mscx", DIR + "lyricsline01.mscx"));
      delete score;
      }
#endif

void TestSpanners::spanners09()
      {
      MasterScore* score = readScore(DIR + "lyricsline02.mscx");
      QVERIFY(score);

            Measure* msr   = score->firstMeasure();
      QVERIFY(msr);
      msr = msr->nextMeasure();
      QVERIFY(msr);
      score->startCmd();
      score->select(msr);
      score->cmdTimeDelete();
      score->endCmd();
      QVERIFY(saveCompareScore(score, "lyricsline02.mscx", DIR + "lyricsline02-ref.mscx"));

            score->undoStack()->undo(&ed);
      score->doLayout();       QVERIFY(saveCompareScore(score, "lyricsline02.mscx", DIR + "lyricsline02.mscx"));
      delete score;
      }


void TestSpanners::spanners10()
      {
      MasterScore* score = readScore(DIR + "lyricsline03.mscx");
      QVERIFY(score);

            Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      msr = msr->nextMeasure();
      QVERIFY(msr);
      score->startCmd();
      score->select(msr);
      score->cmdTimeDelete();
      score->endCmd();
      QVERIFY(saveCompareScore(score, "lyricsline03.mscx", DIR + "lyricsline03-ref.mscx"));

            score->undoStack()->undo(&ed);
      score->doLayout();       QVERIFY(saveCompareScore(score, "lyricsline03.mscx", DIR + "lyricsline03.mscx"));
      delete score;
      }


void TestSpanners::spanners11()
      {
      MasterScore* score = readScore(DIR + "lyricsline04.mscx");
      QVERIFY(score);

            Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      msr = msr->nextMeasure();
      QVERIFY(msr);
      score->startCmd();
      score->select(msr);
      score->cmdTimeDelete();
      score->endCmd();
      QVERIFY(saveCompareScore(score, "lyricsline04.mscx", DIR + "lyricsline04-ref.mscx"));

            score->undoStack()->undo(&ed);
      score->doLayout();       QVERIFY(saveCompareScore(score, "lyricsline04.mscx", DIR + "lyricsline04.mscx"));
      delete score;
      }


void TestSpanners::spanners12()
      {
      MasterScore* score = readScore(DIR + "lyricsline05.mscx");
      QVERIFY(score);

            Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      msr = msr->nextMeasure();
      QVERIFY(msr);
      score->startCmd();
      score->select(msr);
      score->cmdTimeDelete();
      score->endCmd();
      QVERIFY(saveCompareScore(score, "lyricsline05.mscx", DIR + "lyricsline05-ref.mscx"));

            score->undoStack()->undo(&ed);
      score->doLayout();       QVERIFY(saveCompareScore(score, "lyricsline05.mscx", DIR + "lyricsline05.mscx"));
      delete score;
      }
#if 0

void TestSpanners::spanners13()
      {
      EditData          dropData(0);
      LayoutBreak*      brk;

      MasterScore* score = readScore(DIR + "lyricsline06.mscx");
      QVERIFY(score);

            Measure*    msr   = score->firstMeasure();
      QVERIFY(msr);
      brk               = new LayoutBreak(score);
      brk->setLayoutBreakType(LayoutBreak::Type::LINE);
      dropData.pos      = msr->pagePos();
      dropData.element  = brk;
      score->startCmd();
      msr->drop(dropData);
      score->endCmd();
            for (System* sys : score->systems())
            QVERIFY(sys->spannerSegments().size() == 1);
      QVERIFY(saveCompareScore(score, "lyricsline06.mscx", DIR + "lyricsline06-ref.mscx"));

            score->undoStack()->undo();
      score->doLayout();            QVERIFY(saveCompareScore(score, "lyricsline06.mscx", DIR + "lyricsline06.mscx"));
      delete score;
      }
#endif


void TestSpanners::spanners14()
      {
      MasterScore* score = readScore(DIR + "glissando-cloning05.mscx");
      QVERIFY(score);

                  QList<Part*> parts;
      parts.append(score->parts().at(0));
      Score* nscore = new Score(score);

      Excerpt* ex = new Excerpt(score);
      ex->setPartScore(nscore);
      ex->setTitle(parts.front()->longName());
      ex->setParts(parts);
      Excerpt::createExcerpt(ex);
      QVERIFY(nscore);


      score->Score::undo(new AddExcerpt(ex));

      QVERIFY(saveCompareScore(score, "glissando-cloning05.mscx", DIR + "glissando-cloning05-ref.mscx"));
      delete score;
      }



QTEST_MAIN(TestSpanners)
#include "tst_spanners.moc"

