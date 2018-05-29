



#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"
#include "libmscore/barline.h"
#include "libmscore/beam.h"
#include "libmscore/bracket.h"
#include "libmscore/chord.h"
#include "libmscore/clef.h"
#include "config.h"
#include "libmscore/dynamic.h"
#include "libmscore/element.h"
#include <fstream>
#include "libmscore/glissando.h"
#include "globals.h"
#include <iostream>
using std::cout;
#include "libmscore/hairpin.h"
#include "libmscore/harmony.h"
#include "libmscore/key.h"
#include "libmscore/keysig.h"
#include "libmscore/lyrics.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "libmscore/ottava.h"
#include "libmscore/page.h"
#include "libmscore/part.h"
#include "libmscore/pedal.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/repeat.h"
#include "libmscore/rest.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include <stdio.h>
#include <string.h>
#include <sstream>
#include "libmscore/style.h"
#include "libmscore/sym.h"
#include "libmscore/tempotext.h"
#include "libmscore/text.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/tuplet.h"
#include "libmscore/volta.h"
#include "libmscore/marker.h"
#include "libmscore/jump.h"
#include "musescore.h"

namespace Ms {

static  const int MAX_SLURS = 8;
static  const int BRACKSTAVES=64;
static  const int MAXPARTGROUPS = 8;
static const int VERSES = 8;


class ExportLy {
  Score* score;
  QFile f;
  QTextStream os;
  int level;          int curTicks;
  MScore::Direction stemDirection;
  int indx;
  bool partial; 
  int  timedenom, z1, z2, z3, z4;   int barlen, wholemeasurerest;
  QString wholemeasuretext;
  bool pickup;
  bool rehearsalnumbers;
  bool donefirst;   bool graceswitch, gracebeam;
  int gracecount;
  int prevpitch, staffpitch, chordpitch;
  int measurenumber, lastind, taktnr, staffInd;
  bool repeatactive;
  bool firstalt,secondalt;
  enum voltatype {startending, endending, startrepeat, endrepeat, bothrepeat, doublebar, brokenbar, endbar, none};
  struct  voltareg { voltatype voltart; int barno; };
  struct voltareg  voltarray[255];
  int tupletcount;
  bool pianostaff;
  bool slur;
  const Slur* slurre[MAX_SLURS];
  bool started[MAX_SLURS];
  int phraseslur;
  int slurstack;
  int findSlur(const Slur* s) const;
  const char *relativ, *staffrelativ;
  bool voiceActive[VOICES];
  int prevElTick;
  bool ottvaswitch, jumpswitch;
  char privateRehearsalMark;

  struct lybrackets
  {
    bool piano;
    bool bracestart,brakstart, braceend, brakend;
    int braceno, brakno;
  };

  struct lybrackets lybracks[BRACKSTAVES];
  void bracktest();

  struct staffnameinfo
  {
    QString voicename[VOICES];
    QString  staffid, partname, partshort;
    bool simultaneousvoices;
    int numberofvoices;
  };

  struct staffnameinfo staffname[32];

  QString cleannote, prevnote;

  struct InstructionAnchor
  {
    Element* instruct;      Element* anchor;        bool     start;         int      tick;        };

  int nextAnchor;
  struct InstructionAnchor anker;
  struct InstructionAnchor anchors[1024];

  struct glisstablelem
  {
    Chord* chord;
    int tick;
    QString glisstext;
    int type;
  };
  int glisscount;
  struct glisstablelem glisstable[99];

  QString voicebuffer;
  QTextStream out;
  QString scorebuffer;
  QTextStream scorout;
  
  bool nochord;
  int chordcount;
  void chordName(struct InstructionAnchor chordanchor);

  struct chordData
  {
    QString chrName;
    QString extName;
    int alt;
    QString bsnName;
    int bsnAlt;
    int ticklen;
    int tickpos;
  };

  struct chordData thisHarmony;
  struct chordData prevHarmony;
  void resetChordData(struct chordData&);
  QString chord2Name(int ch);

  struct chordPost   {
    struct chordData cd;
    struct chordPost * next;
    struct chordPost * prev;
  };
  struct chordPost cp;
  struct chordPost * chordHead;
  struct chordPost * chordThis;


      struct lyricsData
  {
    QString verselyrics[VERSES];
    QString voicename[VERSES];
    QString staffname;
    int tick[VERSES];
    int segmentnumber[VERSES];
  };

  struct lyricsRecord
  {
    int numberofverses;
    struct lyricsData lyrdat;
    struct lyricsRecord * next;
    struct lyricsRecord *prev;
  };

    struct lyricsRecord * thisLyrics;
  struct lyricsRecord * headOfLyrics;
  struct lyricsRecord * tailOfLyrics;


  void storeChord(struct InstructionAnchor chAnk);
  void chordInsertList(chordPost *);
  void printChordList();
  void cleanupChordList();
  void writeFingering (int&, QString fingering[5]);
  void findLyrics();
  void newLyricsRecord();
  void cleanupLyrics();
  void writeLyrics();
  void connectLyricsToStaff();
  void findGraceNotes(Note*,bool&, int);
  void setOctave(int&, int&, int (&foo)[12]);
  bool arpeggioTest(Chord* chord);
  bool glissandotest(Chord*);
  bool findNoteSymbol(Note*, QString &);
  void buildGlissandoList(int strack, int etrack);
  void writeStringInstruction(int &, QString stringarr[10]);
  void findFingerAndStringno(Note* note, int&, int&, QString (&finger)[5], QString (&strng)[10]);
  struct jumpOrMarkerLM
  {
    Marker* marker;
    int measurenum;
    bool start;
  };

  int lastJumpOrMarker;
  struct jumpOrMarkerLM  jumpOrMarkerList[100];

  void writeLilyHeader();
  void writeLilyMacros();
  void writePageFormat();
  void writeScoreTitles();
  void initJumpOrMarkerLMs();
  void resetJumpOrMarkerLM(struct jumpOrMarkerLM &mlm);
  void removeJumpOrMarkerLM(int);
  void preserveJumpOrMarker(Element *, int, bool);
  void printJumpOrMarker(int mnum, bool start);

  void anchortest();
  void voltatest();
  void jumptest();
  void storeAnchor(struct InstructionAnchor);
  void initAnchors();
  void removeAnchor(int);
  void resetAnchor(struct InstructionAnchor &ank);
  bool findMatchInMeasure(int, Staff*, Measure*, int, int, bool);
  bool findMatchInPart(int, Staff*, Score*, int, int, bool);

  void jumpAtMeasureStop(Measure*);
  void findMarkerAtMeasureStart(Measure*);
  void writeMeasuRestNum();
  void writeTremolo(Chord *);

  void writeSymbol(QString);
  void tempoText(TempoText *);
  void words(Text *);
  void hairpin(Hairpin* hp, int tick);
  void ottava(Ottava* ot, int tick);
  void pedal(Pedal* pd, int tick);
  void dynamic(Dynamic*, int);
  void textLine(Element*, int, bool);
  void findTextProperties(Text* , QString&, int &);
  bool textspannerdown;
    bool textspanswitch;
    void buildInstructionListPart(int strack, int etrack);
  void buildInstructionList(Measure* m, int strack, int etrack);
  void handleElement(Element* el);
  void handlePreInstruction(Element * el);
  void instructionJump(Jump*);
  void instructionMarker(Marker*);
  QString primitiveJump(Jump* );
  QString primitiveMarker(Marker*);
  int checkJumpOrMarker(int, bool, Element*&);
  void writeCombinedMarker(int, Element* );
  QString flatInInstrName(QString);

  void indent();   void indentF();   int getLen(int ticks, int* dots);
  void writeLen(int);
  void writeChordLen(int ticks);
  QString tpc2name(int tpc);
  QString tpc2purename(int tpc);

  void writeScore();
  void stemDir(Chord *);
  void writeVoiceMeasure(MeasureBase*, Staff*, int, int);
  void writeKeySig(int);
  void writeTimeSig(TimeSig*);
  void writeClef(int);
  void writeChord(Chord*, bool);
  void writeRest(int, int);
  void findVolta();
  void findStartRepNoBarline(int &i, Measure*);
  void writeBarline(Measure *);
  int  voltaCheckBar(Measure *, int);
  void writeVolta(int, int);
  void findTuplets(ChordRest*);
  void writeArticulation(ChordRest*);
  void writeScoreBlock();
  void checkSlur(Chord*, bool);
  void doSlurStart(Chord*, bool);
  void doSlurStop(Chord*);
  void initBrackets();
  void brackRegister(int, int, int, bool, bool);
  void findBrackets();

public:
  ExportLy(Score* s)
  {
    score  = s;
    level  = 0;
    curTicks = MScore::division;
    slur   = false;
    stemDirection = MScore::AUTO;
  }
  bool write(const QString& name);
};


int numval(int num)
{  if (num <0) return -num;
  return num;
}



void ExportLy::initBrackets()
{
  for (int i = 0; i < BRACKSTAVES; ++i)         {
      lybracks[i].piano=false;
      lybracks[i].bracestart=false;
      lybracks[i].brakstart=false;
      lybracks[i].braceend=false;
      lybracks[i].brakend=false;
      lybracks[i].braceno=0;
      lybracks[i].brakno=0;
    }
}




void ExportLy::brackRegister(int brnumber, int bratype, int staffnr, bool start, bool end)

{
  QString br = "";
  switch(bratype)
    {
    case BRACKET_NORMAL:
      if (start) lybracks[staffnr].brakstart=true;
      if (end) lybracks[staffnr].brakend=true;
      lybracks[staffnr].brakno=brnumber;
      break;
    case BRACKET_BRACE:
      if (start) lybracks[staffnr].bracestart=true;
      if (end) lybracks[staffnr].braceend=true;
      lybracks[staffnr].braceno=brnumber;
      break;
    case -1: 	           lybracks[staffnr].piano=true;
      if (start) lybracks[staffnr].bracestart=true;
      if (end) lybracks[staffnr].braceend=true;
      lybracks[staffnr].braceno=brnumber;
      break;
    default:
      qDebug("bracket subtype %d not understood\n", bratype);
    }
}



void ExportLy::findBrackets()
{
  initBrackets();
  char groupnumber;
  groupnumber=1;
  const QList<Part*>& il = score->parts();  
  for (int partnumber = 0; partnumber < il.size(); ++partnumber)      {
      Part* part = il.at(partnumber);
      if (part->nstaves() == 2) pianostaff=true;
      for (int stavno = 0; stavno < part->nstaves(); stavno++) 	{
	  if (pianostaff)
	    {
	      if (stavno==0)
		{
		  brackRegister(groupnumber, -1, partnumber+stavno, true, false);
		}
	      if (stavno==1)
		{
		  brackRegister(groupnumber, -1, partnumber+stavno, false, true);
		  pianostaff=false;
		}
	    }
	  else 	    {
	      Staff* st = part->staff(stavno);
	      if (st)
		{
		  for (int braclev= 0; braclev < st->bracketLevels(); braclev++) 		    {
		      if (st->bracket(braclev) != NO_BRACKET) 			{
			  groupnumber++;
			  if (groupnumber < MAXPARTGROUPS)
			    { 			      brackRegister(groupnumber, st->bracket(braclev), partnumber, true, false);
			      brackRegister(groupnumber,st->bracket(braclev), partnumber-1+st->bracketSpan(braclev), false, true);
			    }
			}		    }		}	    } 	}    }}


void ExportLy::bracktest()
      {
      for (int i = 0; i < 10; i++) {
            qDebug("stavnr: %d braceno: %d brackno %d\n", i, lybracks[i].braceno, lybracks[i].brakno);
            }
      }



void ExportLy::instructionJump(Jump* jp)
{
  JumpType jtp = jp->jumpType();
  QString words = "\n    \\once\\override Score.RehearsalMark #'self-alignment-X = #RIGHT \n      ";

  if (jtp == JumpType::DC)
    words += "\\mark \"Da capo\" ";
  else if (jtp == JumpType::DC_AL_FINE)
    words += "\\DCalfine ";
  else if (jtp == JumpType::DC_AL_CODA)
    words += "\\DCalcoda";
  else if (jtp == JumpType::DS_AL_CODA)
    words += "\\DSalcoda";
  else if (jtp == JumpType::DS_AL_FINE)
    words += "\\DSalfine";
  else if (jtp == JumpType::DS)
    words += "\\mark \\markup{Dal segno \\raise #2 \\halign#-1 \\musicglyph #\"scripts.segno\"}";
  else
    qDebug("jump type=%d not implemented\n", jtp);
  out <<  words << " ";
}




void ExportLy::instructionMarker(Marker* m)
{
  MarkerType mtp = m->markerType();
  QString words = "";
  if (mtp == MarkerType::CODA)
      words = "\\theCoda ";
  else if (mtp == MarkerType::CODETTA)
     	words = "\\codetta";
  else if (mtp == MarkerType::SEGNO)
      words = "\\thesegno";
  else if (mtp == MarkerType::FINE)
	words = "\\fine";
  else if (mtp == MarkerType::TOCODA)
	words = "\\gotocoda ";
  else if (mtp == MarkerType::VARCODA)
	words = "\\varcodasign ";
  else if (mtp == MarkerType::USER)
	qDebug("unknown user marker\n");
  else
    qDebug("marker type=%d not implemented\n", mtp);

  out <<  words << " ";

}




QString ExportLy::primitiveJump(Jump* jp)
{
  JumpType jtp = jp->jumpType();
  QString words = "";

  cout << "primitivejump\n";

  if (jtp == JumpType::DC)
     	words = "Da capo";
  else if (jtp == JumpType::DC_AL_FINE)
      words = "D.C. al fine";
  else if (jtp == JumpType::DC_AL_CODA)
	words = "D.C. al coda";
  else if (jtp == JumpType::DS_AL_CODA)
	words = "D.S. al coda";
  else if (jtp == JumpType::DS_AL_FINE)
	words = "D.S. al fine";
  else if (jtp == JumpType::DS)
      words = "Dal segnoX \\musicglyph #\"scripts.segno\"";
  else
    qDebug("jump type=%d not implemented\n", jtp);
  return  words;
}


QString ExportLy::primitiveMarker(Marker* m)
{
  MarkerType mtp = m->markerType();
  QString words = "";
  if (mtp == MarkerType::CODA)         words = "\\line{\\halign #-0.75 \\musicglyph #\"scripts.coda\" \\musicglyph #\"scripts.coda\"}";
  else if (mtp == MarkerType::CODETTA)
     	words = "\\line {\\musicglyph #\"scripts.coda\" \\hspace #-1.3 \\musicglyph #\"scripts.coda\"} } \n";
  else if (mtp == MarkerType::SEGNO)
      words = "\\musicglyph #\"scripts.segno\"";
  else if (mtp == MarkerType::FINE)
	words =  "{\"Fine\"} \\mark \\markup {\\musicglyph #\"scripts.ufermata\" } \\bar \"\bar \"||\" } \n";
  else if (mtp == MarkerType::TOCODA)
	words = "\\musicglyph #\"scripts.coda\"";
  else if (mtp == MarkerType::VARCODA)
    words = "\\musicglyph#\"scripts.varcoda\"";
  else if (mtp == MarkerType::USER)
	qDebug("unknown user marker\n");
  else
    qDebug("marker type=%d not implemented\n", mtp);
  return words;
}




void ExportLy::writeSymbol(QString name)
{
     
  if (wholemeasurerest > 0) writeMeasuRestNum();

  if (name == "clef eight")
      out << "^\\markup \\tiny\\roman 8 ";
  else if (name == "pedal ped")
    out << " \\sustainOn ";
  else if (name == "pedalasterisk")
    out << " \\sustainOff ";
  else if (name == "scripts.trill")
    out << "\\trill ";
  else if (name == "scripts.flageolet")
    out << "\\flageolet ";
  else if (name == "rcomma")
    out << "\\mark \\markup {\\musicglyph #\"scripts.rcomma\"} ";
  else if (name == "lcomma")
    out << "\\mark \\markup {\\musicglyph #\"scripts.lcomma\"} ";
  else
    out << "^\\markup{\\musicglyph #\"" << name << "\"} ";
                                          }


void ExportLy::resetChordData(struct chordData &CD)
{
  CD.chrName="";
  CD.extName="";
  CD.alt=0;
  CD.bsnName="";
  CD.tickpos=0;
  CD.bsnAlt=0;
  CD.ticklen=0;
}


void ExportLy::cleanupChordList()
{
  chordPost * next;
  chordThis = chordHead;
  if (chordThis == 0)
        return;
  next = chordThis->next;

  while (next !=NULL)
    {
      next->prev = NULL;
      delete chordThis;
      chordThis = next;
      next = next->next;
    }
  delete chordThis;
}


void ExportLy::writeChordLen(int ticks)
{
  int dots = 0;
  int len = getLen(ticks, &dots);

  switch (len)
    {
    case -5:
      os << "1*5/4";       break;
    case -4:
      os << "2*5 ";       break;
    case -3:
      os << "1.*2 ";
      break;
    case -2:      os << "1*4 ";
      break;
    case -1:       os << "1*2";
      break;
    default:
      os << len;
      for (int i = 0; i < dots; ++i)
	os << ".";
      break;
    }
}

void ExportLy::printChordList()
{
  chordThis = chordHead;
  if (chordThis == 0)         return;

  struct chordPost * next;
  next = chordThis->next;
  int i=0;
  int dots=0;
  int lilylen=0;

  while (next != NULL)
    {
      i++;

            if ((i==1) and (chordThis->cd.tickpos > 0))
	{
	  int factor=1;
	  	  if (timedenom == 2) factor=2; else factor = 1;
	  int measnum = chordThis->cd.tickpos / (z1 * MScore::division * factor);
	  qDebug("Measnum chord: \n");
	  int surplus = chordThis->cd.tickpos % MScore::division;
	  if (measnum == 0) surplus = chordThis->cd.tickpos;
	  level++;
	  indentF();
	  if (surplus > 0)
	    {
	      lilylen= getLen(surplus, &dots);
	      level++;
	      indentF();
	      os << "s" << lilylen;
	      while (dots>0)
		{
		  os<< ".";
		  dots--;
		}
	      os << " ";
	    }
	  if (measnum > 0 ) os << "s1*" << measnum<< " \n";
	}      else
	{
	  	  chordThis->cd.ticklen =  next->cd.tickpos - chordThis->cd.tickpos;
	  chordThis=next;
	  next=next->next;
	}
    }
  if (next == NULL)
     chordThis->cd.ticklen= 480;


  chordThis = chordHead;
  next = chordThis;
  
  indentF();


  while (next != NULL)
    {
      next=next->next;
      dots=0;
      lilylen=0;
      i++;
            os << chordThis->cd.chrName;
      curTicks=0;

      writeChordLen(chordThis->cd.ticklen); 
      while (dots > 0)
	{
	  os << ".";
	  dots--;
	}

      if (chordThis->cd.extName !="")
	os << ":" << chordThis->cd.extName;

      if (chordThis->cd.bsnName !="")
	os << "/" << chordThis->cd.bsnName;
      if (chordThis->cd.bsnAlt > 0)
	os << chordThis->cd.bsnAlt;
      os << " ";
      chordThis=next;
    }  os << "}%%end of chordlist \n\n";
}


QString ExportLy::chord2Name(int ch)
      {
      const char names[] = "fcgdaeb";
      return QString(names[(ch + 1) % 7]);
      }


void ExportLy::chordInsertList(chordPost * newchord)
{

  if (chordHead == NULL)     {
      chordcount++;
      chordHead = newchord;
      newchord->prev = NULL;
      newchord->next = NULL;
    }
  else     {
      chordcount++;
      chordThis = chordHead;
      while ((newchord->cd.tickpos >= chordThis->cd.tickpos) && (chordThis->next != NULL))
	{
	  chordThis = chordThis->next;
	}
      if ((chordThis->next == NULL) && (chordThis->cd.tickpos <= newchord->cd.tickpos)) 	{
	  	  chordThis->next = newchord;
	  newchord->prev = chordThis;
	}
      else
		{
	  newchord->next = chordThis;
	  newchord->prev = chordThis->prev;
	  if (chordHead != chordThis)
	    {
	      chordThis = chordThis->prev;
	      chordThis->next = newchord;
	    }
	  else 	    {
	      chordThis->prev = newchord;
	      chordHead = newchord;
	    }
	}    }}
void ExportLy::storeChord(struct InstructionAnchor chordanchor)
{
  cout << "chords!!!\n";
    chordPost * aux;
  aux = new chordPost();
  resetChordData(aux->cd);
  aux->next = NULL;
  aux->prev = NULL;

    Harmony* harmelm = (Harmony*) chordanchor.instruct;
  int  chordroot = harmelm->rootTpc();
  QString n, app;

  if (chordroot != INVALID_TPC)
    {
      if (nochord == true) nochord = false;
      aux->cd.chrName = chord2Name(chordroot);
      n=thisHarmony.chrName;

      aux->cd.tickpos = harmelm->parent()->type() == Element::SEGMENT
         ? static_cast<Segment*>(harmelm->parent())->tick() : 0;

      if (!harmelm->xmlKind().isEmpty())
	{
	  aux->cd.extName = harmelm->extensionName();
	  aux->cd.extName = aux->cd.extName.toLower();
	}

      int alter = tpc2alter(chordroot);
      if (alter==1) app = "is";
      else
	{
	  if (alter == -1)
	    {
	      if (n == "e") app = "s";
	      else app = "es";
	    }
	}
      aux->cd.chrName = aux->cd.chrName + app;

      int  bassnote = harmelm->baseTpc();
      if (bassnote != INVALID_TPC)
	{
	  aux->cd.bsnName = chord2Name(bassnote);
	  int alter = tpc2alter(bassnote);
	  n=aux->cd.bsnName;

	  if (alter==1) app = "is";
	  else if (alter == -1)
	  {
	    if (n=="e")  app =  "s"; else app = "es";
	  }

	  aux->cd.bsnName = n + app;
	  aux->cd.bsnAlt=alter;
	}             chordInsertList(aux);
    }  else
    storeAnchor(anker);
}



void ExportLy::tempoText(TempoText* text)
      {
	QString temptekst = text->text();
	double met = text->tempo();
	int metronome;
	metronome = (int) (met * 60);
	out << "\\tempo \""  << text->text() << "\" " <<  timedenom << " = " << metronome << "  ";
      }




void ExportLy::words(Text* text)
     {
       QString style;
       int size;
       findTextProperties(text,style,size);
                     out << "^\\markup {" << style<< " \"" << text->text() << "\"} ";
           }




void ExportLy::hairpin(Hairpin* hp, int tick)
{       	int art=2;
	art=hp->hairpinType();
	if (hp->tick() == tick)
	  {
	    if (art == 0) 	      out << "\\< ";
	    if (art == 1) 	      out << "\\> ";
	    if (art > 1 ) out << "\\!x ";
	  }
      }


void ExportLy::ottava(Ottava* ot, int tick)
{
  int st = ot->ottavaType();
  if (ot->tick() == tick)
    {
      switch(st) {
      case 0:
	out << "\\ottva ";
	break;
      case 1:
	out << "\\ottva \\once\\override TextSpanner #'(bound-details left text) = \"15va\" \n";
	indent();
	break;
      case 2:
	out << "\\ottvabassa ";
	break;
      case 3:
	out << "\\ottvabassa \\once \\override TextSpanner #'(bound-details left text) = \"15vb\"  \n";
	indent();
	break;
      default:
	qDebug("ottava subtype %d not understood\n", st);
      }
    }
  else {
     	  out << "\\ottvaend ";
        }
}



void ExportLy::pedal(Pedal* pd, int tick)
      {
      if (pd->tick() == tick)
	out << "\\sustainOn ";
      else
	out << "\\sustainOff ";
      }



void ExportLy::dynamic(Dynamic* dyn, int nop)
{
  QString t = dyn->text();
  if (t == "p" || t == "pp" || t == "ppp" || t == "pppp" || t == "ppppp" || t == "pppppp"
      || t == "f" ||
      t == "ff" || t == "fff" || t == "ffff" || t == "fffff" || t == "ffffff"
      || t == "mp" || t == "mf" || t == "sf" || t == "sfp" || t == "sfpp" || t == "fp"
      || t == "rf" || t == "rfz" || t == "sfz" || t == "sffz" || t == "fz" || t == "sff")
    {
	switch(nop)
	    {
	    case 0:
		out << "\\" << t << " ";
		break;
	    case 1:
		out << "_\\markup\{\\dynamic " << t.toLatin1().data() << " \\halign #-2 ";
		break;
	    case 2:
		out <<  " \\dynamic " << t.toLatin1().data() << " } ";
		break;
	    default:
		out << "\\" << t.toLatin1().data() << " ";
		break;
	    }

    }
  else if (t == "m" || t == "z")
    {
      out << "\\"<< t.toLatin1().data() << " ";
    }
    else
      out << "_\\markup{\""<< t.toLatin1().data() << "\"} ";
}

void ExportLy::findTextProperties(Text* tekst, QString &tekststyle, int &fontsize)
{
  QFont fontprops=tekst->font();
  fontsize= fontprops.pointSizeF();
  switch (fontprops.style())
    {
    case QFont::StyleNormal :
      tekststyle = "\\upright ";
      break;
    case QFont::StyleItalic :
    case QFont::StyleOblique:
      tekststyle = "\\italic";
      break;
    default :
      tekststyle = "\\upright ";
      break;
    }
  switch (fontprops.weight())
    {
    case QFont::Light:
    case QFont::Normal:
      break;
    case QFont::DemiBold:
    case QFont::Bold:
    case QFont::Black:
      tekststyle += "\\bold ";
      break;
    default:
      break;
    }
}

void ExportLy::textLine(Element* instruction, int tick, bool pre)
{
  qDebug("textline\n");
  QString rest;
  QPointF point;
  QString lineEnd = "none";
  QString type;
    QString lineType;
  int fontsize=0;
  TextLine* tekstlinje = (TextLine *) instruction;
  bool post = false;
  if (pre == false) post = true;

    if (tekstlinje->tick() == tick)
    {
      if (pre)
	{
	  switch (tekstlinje->lineStyle())
	    {
	    case Qt::DashDotLine:
	    case Qt::DashDotDotLine:
	    case Qt::DashLine:
	      out << " \\once\\override TextSpanner  #'style = #'dashed-line \n";
	      indent();
	      break;
	    case Qt::DotLine:
	      out << " \\once\\override TextSpanner  #'style = #'dotted-line \n";
	      indent();
	      break;
	    default:
	      break;
	    }
	  if (tekstlinje->endHook())
	    {
	      double h = tekstlinje->endHookHeight().val();
	      if (h < 0.0)
		{
		  out << "\\once\\override TextSpanner #'(bound-details right text) = \\markup{ \\draw-line #'(0 . 1) }\n";
		  indent();
		}
	      else
		{
		  out << "\\once\\override TextSpanner #'(bound-details right text) = \\markup{ \\draw-line #'(0 . -1) }\n";
		  indent();
		}
	    }
	  if (tekstlinje->beginText())
	    {
	      QString linetext = tekstlinje->beginText()->text();
	      Text* tekst = (Text*) tekstlinje->beginText();
	      QString tekststyle = "";
	      findTextProperties(tekst, tekststyle, fontsize);
	      out << "\\once\\override TextSpanner #'(bound-details left text) = \\markup{";
	      out << tekststyle<< "\"" << linetext <<"\"} \n";
	      indent();
	    }
	  point = tekstlinje->frontSegment()->userOff();
	  if (point.y() > 0.0) 	    {
	      out <<"\\textSpannerDown ";
	      textspannerdown=true;
	    }
	  else if (textspannerdown)
	    {
	      out << "\\textSpannerNeutral ";
	      textspannerdown = false;
	    }
	}
      else if (post) 	{
	  out << "\\startTextSpan ";
	  textspanswitch=true;
	}    }#if 0   else if  (sl->tick2() == tick)      {
      if (pre)
	{
	  out << "\\stopTextSpan ";
	  textspanswitch=false;
	   	  	  	  	}
      else if (post)
	{
	  	}
    }#endif
}

void ExportLy::anchortest()
{
      int i;
      for (i=0; i<nextAnchor ; i++)
	{
	  Element * instruction = anchors[i].instruct;
	  ElementType instructiontype = instruction ->type();
	  qDebug("anker nr: %d ", i);
	  switch(instructiontype)
	    {
	    case Element::STAFF_TEXT:
	      qDebug("STAFF_TEXT ");
	      qDebug("\n");
	      break;
	    case Element::TEXT:
	      qDebug("TEXT ");
	      qDebug("\n");
	      break;
	    case Element::MARKER:
	      qDebug("MARKER\n");
	      break;
	    case Element::JUMP:
	      qDebug("JUMP\n");
	      break;
	    case Element::SYMBOL:
	      qDebug("SYMBOL\n");
	      break;
	    case Element::TEMPO_TEXT:
	      qDebug("TEMPOTEXT MEASURE\n");
	      break;
	    case Element::DYNAMIC:
	      qDebug("Dynamic\n");
	      break;
	    case Element::HARMONY:
	      qDebug("akkordnavn. \n");
	      break;
	    case Element::HAIRPIN:
	      qDebug("hairpin \n");
	      break;
	    case Element::PEDAL:
	      qDebug("pedal\n");
	      break;
	    case Element::TEXTLINE:
	      qDebug("textline\n");
	      break;
	    case Element::OTTAVA:
	      qDebug("ottava\n");
	      break;
	    default: break;
	    }
	}
      qDebug("Anchortest finished\n");
}




void ExportLy::jumptest()
{
  qDebug("at jumptest A lastjump %d\n", lastJumpOrMarker);
      int i;
      for (i=0; i<lastJumpOrMarker; i++)
	{
	  qDebug("jumptest 1\n");
	  Element * merke = jumpOrMarkerList[i].marker;
	  qDebug("jumptest 2\n");
	  ElementType instructiontype = merke->type();
	  qDebug("jumptest 3\n");
	  qDebug("jumptest 4\n");
	  qDebug("marker nr: %d ", i);
	  switch(instructiontype)
	    {
	    case Element::STAFF_TEXT:
	      qDebug("STAFF_TEXT ");
	      qDebug("\n");
	      break;
	    case Element::TEXT:
	      qDebug("TEXT ");
	      qDebug("\n");
	      break;
	    case Element::MARKER:
	      qDebug("MARKER\n");
	      break;
	    case Element::JUMP:
	      qDebug("JUMP\n");
	      break;
	    case Element::SYMBOL:
	      qDebug("SYMBOL\n");
	      break;
	    case Element::TEMPO_TEXT:
	      qDebug("TEMPOTEXT MEASURE\n");
	      break;
	    case Element::DYNAMIC:
	      qDebug("Dynamic\n");
	      break;
	    case Element::HARMONY:
	      qDebug("akkordnavn. \n");
	      break;
	    case Element::HAIRPIN:
	      qDebug("hairpin \n");
	      break;
	    case Element::PEDAL:
	      qDebug("pedal\n");
	      break;
	    case Element::TEXTLINE:
	      qDebug("textline\n");
	      break;
	    case Element::OTTAVA:
	      qDebug("ottava\n");
	      break;
	    default:
	      break;
	    }
	}
}


void ExportLy::initAnchors()
{
  int i;
  for (i=0; i<1024; i++)
    resetAnchor(anchors[i]);
}



void ExportLy::resetAnchor(struct InstructionAnchor &ank)
{
  ank.instruct=0;
  ank.anchor=0;
  ank.start=false;
  ank.tick=0;
}

void ExportLy::removeAnchor(int ankind)
{
  int i;
  resetAnchor(anchors[ankind]);
  for (i=ankind; i<=nextAnchor; i++)
    anchors[i]=anchors[i+1];
  resetAnchor(anchors[nextAnchor]);
  nextAnchor=nextAnchor-1;
}


void ExportLy::storeAnchor(struct InstructionAnchor a)
      {
	if (nextAnchor < 1024)
	  {
	    anchors[nextAnchor++] = a;
	  }
	else
	  qDebug("InstructionHandler: too many instructions\n");
	resetAnchor(anker);
      }

void ExportLy::writeCombinedMarker(int foundJoM, Element* elm)
{
  out << "\\mark\\markup\\column \{ \n";
  level++;
  indent();
  if (foundJoM == Element::MARKER)
    {
      QString primark = primitiveMarker((Marker*)elm);
      out << primark << "\n";
    }
  if (foundJoM == Element::JUMP)
    out << primitiveJump((Jump*) elm) << "\n";
  indent();
  out << "\\box\\bold \"";
  if (rehearsalnumbers)
    out << (int)privateRehearsalMark;   else
    out << privateRehearsalMark << "\" \n";
  indent();
  level--;
  indent();
  out << "} \n";
  indent();
  out << "\\set Score.rehearsalMark = #" << (int)privateRehearsalMark-63 << "\n";
  indent();
}





void ExportLy::handlePreInstruction(Element * el)
{
  int i = 0;
  int foundJoM = 0;
  Text* tekst;
  for (i = 0; i <= nextAnchor; i++)     {
      if  ((anchors[i].anchor != 0) && (anchors[i].anchor == el))
	{
	  Element * instruction = anchors[i].instruct;
	  ElementType instructiontype = instruction->type();

	  switch(instructiontype)
	    {
	    case Element::STAFF_TEXT:
	    case Element::REHEARSAL_MARK:
	      {
		    tekst = (Text*) instruction;
		    if (wholemeasurerest >=1) writeMeasuRestNum();
		    bool ok = false;
		    		    QString c;
		    c=tekst->text();
		    		    if (ok) rehearsalnumbers=true;
		    Element* elm = 0;
    		    foundJoM = checkJumpOrMarker(measurenumber, true, elm); 		      if (foundJoM)
			writeCombinedMarker(foundJoM,elm);
		      else
		        out << "\\mark\\default ";		      privateRehearsalMark++;
		      removeAnchor(i); 		break;
	      }
	    case Element::OTTAVA:
	      if (wholemeasurerest >=1) writeMeasuRestNum();
	      ottvaswitch=true;
	      ottava((Ottava*) instruction, anchors[i].tick);
	      removeAnchor(i);
	      break;
	    case Element::TEMPO_TEXT:
	      tempoText((TempoText*) instruction);
	      removeAnchor(i);
	      break;
	    case Element::TEXTLINE:
	      textLine(instruction, anchors[i].tick, true);
	      break;
	    default: break;
	    }	}    }}



void ExportLy::handleElement(Element* el)
{
  int i = 0;
  Symbol * sym;
  QString name;
  for (i = 0; i<=nextAnchor; i++)    {
	if (anchors[i].anchor != 0 and anchors[i].anchor==el) 	    {
		Element* instruction = anchors[i].instruct;
		ElementType instructiontype = instruction->type();

		switch(instructiontype)
		    {
		    case Element::MARKER:
			qDebug("MARKER\n");
			instructionMarker((Marker*) instruction);
			break;
		    case Element::JUMP:
			qDebug("JUMP\n");
			instructionJump((Jump*) instruction);
			break;
		    case Element::SYMBOL:
			{
			    cout << "symbol in anchorlist tick: " << anchors[i].tick << "  \n";
			    sym = (Symbol*) instruction;
			    name = Sym::id2name(sym->sym());
			    writeSymbol(name);
			    break;
			}
		    case Element::TEMPO_TEXT:
									break;
		    case Element::STAFF_TEXT:
		    case Element::TEXT:
			cout << "anchored text \n";
			if (wholemeasurerest)
			    {
				Text* wmtx = (Text*) instruction;
				wholemeasuretext = wmtx->text();
			    }
			else
			    words((Text*) instruction);
			break;
		    case Element::DYNAMIC:
			{
			    int nextorprev=0;

			    if ((anchors[i+1].anchor != 0) and (anchors[i+1].anchor==el))
				{

				    Element* nextinstruct = anchors[i+1].instruct;
				    ElementType nextinstrtype = nextinstruct->type();
				    if (nextinstrtype == Element::DYNAMIC)
				    nextorprev = 1;
				}
			    else if ((anchors[i-1].anchor != 0) and (anchors[i-1].anchor==el))
				{
				    Element* previnstruct = anchors[i-1].instruct;
				    ElementType previnstrtype = previnstruct->type();
				    if (previnstrtype == Element::DYNAMIC)
					nextorprev=2;
				}
			    dynamic((Dynamic*) instruction, nextorprev);
			    break;
			}
		    case Element::HAIRPIN:
			hairpin((Hairpin*) instruction, anchors[i].tick);
			break;
		    case Element::HARMONY:
			words((Text*) instruction);
			break;
		    case Element::PEDAL:
			pedal((Pedal*) instruction, anchors[i].tick);
			break;
		    case Element::TEXTLINE:
			textLine(instruction, anchors[i].tick, false);
			break;
		    case Element::OTTAVA:
			break;
		    default:
			qDebug("post-InstructionHandler::handleElement: direction type %s at tick %d not implemented\n",
			       Element::name(instruction->type()), anchors[i].tick);
			break;
		    }
			    }
    } }




void ExportLy::resetJumpOrMarkerLM(struct jumpOrMarkerLM &mlm)
{
  mlm.marker=0;
  mlm.measurenum=0;
  mlm.start=false;
}

void ExportLy::initJumpOrMarkerLMs()
{
  int i;
  for (i=0; i<100; i++)
    resetJumpOrMarkerLM(jumpOrMarkerList[i]);
}

void ExportLy::removeJumpOrMarkerLM(int markerind)
{
  int i;
  resetJumpOrMarkerLM(jumpOrMarkerList[markerind]);
  for (i=markerind; i<=nextAnchor; i++)
    jumpOrMarkerList[i]=jumpOrMarkerList[i+1];
  resetJumpOrMarkerLM(jumpOrMarkerList[lastJumpOrMarker]);
  lastJumpOrMarker=lastJumpOrMarker-1;
}



void ExportLy::preserveJumpOrMarker(Element* dir, int mnum, bool start)
{
  jumpswitch=true;
  jumpOrMarkerLM mlm;
  Marker* ma = (Marker*) dir;
  mlm.marker = ma;
  mlm.measurenum = mnum;
  mlm.start = start;
  if (lastJumpOrMarker < 100)
    {
      lastJumpOrMarker++;
      jumpOrMarkerList[lastJumpOrMarker] = mlm;
    }
  else
    qDebug("PreserveMarker: Too many marksorjumps\n");
}



int ExportLy::checkJumpOrMarker(int mnum, bool start, Element* &moj)
{
  cout << "checkjumpormarker\n";

  int tp=0;
  int i=0;

  if (start) mnum--;   cout << "mnum: " << mnum << "\n";

  while (jumpOrMarkerList[i].measurenum < mnum)
    {
      ++i;
      if (jumpOrMarkerList[i].measurenum ==0 )
	goto endofcheck;
    }

  while ((jumpOrMarkerList[i].measurenum == mnum) and (i < 100))
    {
      cout << "found measure  " << jumpOrMarkerList[i].start << "\n";
      if (jumpOrMarkerList[i].start == true)
	{
	  moj = jumpOrMarkerList[i].marker;
	  tp = moj->type();
	  cout << "moj->type: " << tp << "\n";
    	}
      i++;
      cout << i << "\n";
                }
 endofcheck:
  cout << "checkjumpormarker, type: " << tp << "\n";
  return tp;
}


void ExportLy::printJumpOrMarker(int mnum, bool start)
{
  cout << "printjumpormarker 1\n";

  int i=0;
  while (jumpOrMarkerList[i].measurenum < mnum)
    i++;

  cout << "test 2\n";

  while ((jumpOrMarkerList[i].measurenum == mnum) and (i < 100))
    {
      cout << "test 3\n";

      if (jumpOrMarkerList[i].start == start)
	{
	  cout << "test 4\n";

	  Element* moj = jumpOrMarkerList[i].marker;
	  int tp = moj->type();
	  if (tp == Element::MARKER)
	    {
	      cout << "test 5\n";
	      Marker* ma = (Marker*) moj;
	      instructionMarker(ma);
	    }
	  else if (tp ==Element::JUMP)
	    {
	      cout << "test 6\n";
	      Jump* jp = (Jump*) moj;
	      instructionJump(jp);
    	    }
    	  cout << "test 7\n";
    	}
      i++;
      cout << i << "\n";
                }
  cout << "test 8\n";
}





void ExportLy::findMarkerAtMeasureStart(Measure* m)
{
   for (auto ci = m->el()->begin(); ci != m->el()->end(); ++ci)
     {
       Element* dir = *ci;
       int tp = dir->type();
       if (tp == Element::MARKER)
  	 { 	   Marker* ma = (Marker*) dir;
	   MarkerType mtp = ma->markerType();
	   	   if (!(mtp == MarkerType::FINE || mtp == MarkerType::TOCODA))
	     {
	       cout << "marker found at measure: " << measurenumber << "\n";
	       	       preserveJumpOrMarker(dir, measurenumber, true); 	     }
  	 }
     }
}


void ExportLy::jumpAtMeasureStop(Measure* m)
      {
			for (auto ci = m->el()->begin(); ci != m->el()->end(); ++ci)
	  {
	    Element* dir = *ci;
	    int tp = dir->type();
	    bool end; 	    	    end=false;

	    if (tp == Element::JUMP)
	      {
				Jump* jp = (Jump*) dir;
				instructionJump(jp);
															      	preserveJumpOrMarker(dir, measurenumber, end);
	      }
	    else if (tp == Element::MARKER)
	      {
		Marker* ma = (Marker*) dir;
		MarkerType mtp = ma->markerType();
				if (mtp == MarkerType::FINE || mtp == MarkerType::TOCODA)
		  {
		    		    instructionMarker(ma);
		    		    preserveJumpOrMarker(dir, measurenumber, end);
		  }
	      }
	  }
      }



bool ExportLy::findMatchInMeasure(int tick, Staff* stf, Measure* m, int strack, int etrack, bool rehearsalmark)
{
  int iter=0;
  bool  found = false;

  for (int st = strack; st < etrack; ++st)
    {
      for (Segment* seg = m->first(); seg; seg = seg->next())
	{
	  iter ++;
	  Element* el = seg->element(st);
	  if (!el) continue;

	  if ((el->isChordRest()) and ((el->staff() == stf) or (rehearsalmark==true)) && ((seg->tick() >= tick)))
	    {
	      if (seg->tick() > tick) tick=prevElTick;
	      anker.anchor=el;
	      found=true;
	      anker.tick=tick;
	      anker.start=true;
	      goto fertig;
	    }
	    prevElTick = seg->tick();
	 }
    }
 fertig:
 return found;
}



bool ExportLy::findMatchInPart(int tick, Staff* stav, Score* sc, int starttrack, int endtrack, bool rehearsalmark)
{

  bool found=false;
  for (MeasureBase* mb = sc->measures()->first(); mb; mb = mb->next())
    {
      if (mb->type() != Element::MEASURE)
	continue;
      Measure* m = (Measure*)mb;
      found = findMatchInMeasure(tick, stav, m, starttrack, endtrack, rehearsalmark);
      if (found) break;
     }
return found;
}


void ExportLy::buildInstructionListPart(int strack, int etrack)
{

    prevElTick=0;
#if 0   foreach(Element* instruction, *(score->gel()))
    {
      bool found=false;
      bool rehearsalm=false;
      switch(instruction->type())
	{
	case Element::JUMP:
            break;
	case Element::MARKER:
 break;
	case Element::HAIRPIN:
	case Element::HARMONY:
	case Element::OTTAVA:
	case Element::PEDAL:
	case Element::DYNAMIC:
	case Element::TEXT:
	case Element::TEXTLINE:
	  {
	    SLine* sl = (SLine*) instruction;
	    Text* tekst = (Text*) instruction;
	    	    	    if (tekst->subtypeName() == "RehearsalMark")
	      {
		rehearsalm=true;
		qDebug("found rehearsalmark in part\n");
	      }
	    	    found=findMatchInPart(sl->tick(), sl->staff(), score, strack, etrack, rehearsalm);
	    if (found)
	      {
		anker.instruct=instruction;
		storeAnchor(anker);
	      }
	    	    if (found)
	      {
		anker.instruct=instruction;
		storeAnchor(anker);
	      }
	    break;
	  } 	default:
	  	  	  break;
	}
    }#endif

    for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next())
    {
      if (mb->type() != Element::MEASURE)
	continue;
      Measure* m = (Measure*)mb;
      buildInstructionList(m, strack, etrack);
    }
}


void ExportLy::buildInstructionList(Measure* m, int strack, int etrack)
{

    for (auto ci = m->el()->begin(); ci != m->el()->end(); ++ci)
    {
      bool found=false;

      Element* instruction = *ci;
      switch(instruction->type())
	{
	case Element::DYNAMIC:
	case Element::SYMBOL:
	case Element::TEMPO_TEXT:
	case Element::TEXT:
	case Element::HAIRPIN:
	  	case Element::OTTAVA:
	case Element::PEDAL:
	case Element::STAFF_TEXT:
#if 0 	  { 	    	    	    if (instruction->subtypeName() == "RehearsalMark") rehearsal=true;
	    found = findMatchInMeasure(instruction->tick(), instruction->staff(), m, strack, etrack, rehearsal);
	  if (found)
	    {
	      anker.instruct=instruction;
	      storeAnchor(anker);
	    }
       }
#endif
	  break;
	case Element::HARMONY:
	  {
          Harmony* h = static_cast<Harmony*>(instruction);
          int tick = h->parent()->type() == Element::SEGMENT
             ? static_cast<Segment*>(h->parent())->tick() : 0;
	    found = findMatchInMeasure(tick, instruction->staff(), m, strack, etrack, false);
	    if ((found) && (staffInd == 0)) 	      {
		anker.instruct=instruction;
		storeChord(anker);
		resetAnchor(anker);
	      }
	    break;
	  }
	 default:
	   break;
	}
    }
}

void ExportLy::buildGlissandoList(int strack, int etrack)
{
            for (MeasureBase* mb = score->measures()->first(); mb; mb = mb->next())
    {
      if (mb->type() != Element::MEASURE)
	continue;
      Measure* m = (Measure*)mb;
      for (int st = strack; st < etrack; ++st)
       	{
	  for (Segment* seg = m->first(); seg; seg = seg->next())
	    {
	      Element* el = seg->element(st);	      if (!el) continue;

	      if (el->type() == Element::CHORD)
		{
		 Chord* cd = (Chord*)el;
		  if (cd->glissando())
		    {
		      glisscount++;
		      		      Element* prevel = seg->prev()->element(st); 		      Chord* prevchord = (Chord*)prevel;
		      glisstable[glisscount].chord = prevchord;
		      glisstable[glisscount].type = int(cd->glissando()->glissandoType());
		      glisstable[glisscount].glisstext = cd->glissando()->text();
		      glisstable[glisscount].tick = prevchord->tick();
		    }
		}
	    }
	 }
    }
}




void ExportLy::indent()
{
  for (int i = 0; i < level; ++i)
    out << "    ";
}



void ExportLy::indentF()
{
      for (int i = 0; i < level; ++i)
	    os << "    ";
}



void ExportLy::findTuplets(ChordRest* cr)
{
      Tuplet* t = cr->tuplet();

      if (t) {
            if (tupletcount == 0) {
                  int actNotes   = t->ratio().numerator();
                  int nrmNotes   = t->ratio().denominator();
                  int baselength = t->duration().ticks() / nrmNotes;
                  int thislength = cr->duration().ticks();
		  tupletcount    = nrmNotes * baselength - thislength;
                  out << "\\times " <<  nrmNotes << "/" << actNotes << "{" ;
                  }
            else if (tupletcount > 1) {
                  int thislength = cr->duration().ticks();
                  tupletcount    = tupletcount - thislength;
                  if (tupletcount == 0)
                        tupletcount = -1;
                  }
            }
      }

int ExportLy::voltaCheckBar(Measure* meas, int i)
{

  int barlinetype = meas->endBarLineType();

  switch(barlinetype)
    {
    case START_REPEAT:
      i++;
      voltarray[i].voltart=startrepeat;
      voltarray[i].barno=taktnr;
      break;
    case END_REPEAT:
      i++;
      voltarray[i].voltart=endrepeat;
      voltarray[i].barno=taktnr;
      break;
    case END_START_REPEAT:
      i++;
      voltarray[i].voltart=bothrepeat;
      voltarray[i].barno=taktnr;
      break;
    case END_BAR:
      i++;
      voltarray[i].voltart=endbar;
      voltarray[i].barno=taktnr;
      break;
    case DOUBLE_BAR:
      i++;
      voltarray[i].voltart=doublebar;
      voltarray[i].barno=taktnr;
      break;
    case BROKEN_BAR:
    case DOTTED_BAR:
      i++;
      voltarray[i].voltart=brokenbar;
      voltarray[i].barno=taktnr;
      break;
    default:
      break;
    }
          if (meas->repeatFlags() == RepeatStart)
    {
            if ((voltarray[i].barno != taktnr-1) and (voltarray[i].voltart != startrepeat) and ( voltarray[i].voltart != bothrepeat ))
	{
	  i++;
	  voltarray[i].voltart=startrepeat;
	  voltarray[i].barno=taktnr-1; 	}
    }

  return i;
}

void ExportLy::findStartRepNoBarline(int &i, Measure* m)
{
   for (Segment* seg = m->first(); seg; seg = seg->next())
    {
      if (seg->segmentType() == SegmentType::StartRepeatBarLine)
	{
	  i++; 	  voltarray[i].voltart = startrepeat;
	  voltarray[i].barno = taktnr-1;
	  break;
	}
    }
}




void  ExportLy::findVolta()
{
  taktnr=0;
  lastind=0;
  int i=0;

  for (i=0; i<255; i++)
    {
      voltarray[i].voltart=none;
      voltarray[i].barno=0;
    }

  i=0;

  for (MeasureBase * m=score->first(); m; m=m->next())
    {      if (m->type() != Element::MEASURE )
	continue;

      ++taktnr; 
                        Measure* meas = (Measure*)m;
      findStartRepNoBarline(i,meas);

#if 0       foreach(Element* el, *(m->score()->gel()))
		{
	  if (el->type() == Element::VOLTA)
	    {
	      Volta* v = (Volta*) el;

	      if (v->tick() == m->tick()) 		{
		  i++;
		  		  		  		  		  		  		  		  		  		  		  		  		  		  		  		  		  voltarray[i].voltart = startending;
		  voltarray[i].barno=taktnr-1; 		}
#if 0 	      if (v->tick2() == m->tick() + m->ticks()) 		{
		  i++;
		  voltarray[i].voltart = endending;
		  voltarray[i].barno=taktnr;		  		  		  		  		  		  		}
#endif
	    }	}#endif
      i=voltaCheckBar((Measure *) m, i);
    }  lastind=i;

}
void ExportLy::voltatest()
{
  int i=0;
  for (i=0; i<lastind; i++)
    {
      qDebug("iter: %d\n", i);
      switch(voltarray[i].voltart)
	{
	case startrepeat:
	  qDebug("startrepeat, bar %d\n", voltarray[i].barno);
	  break;
	case endrepeat:
	  qDebug("endrepeat, bar %d\n", voltarray[i].barno);
	  break;
	case bothrepeat:
	  qDebug("bothrepeat, bar %d\n", voltarray[i].barno);
	  break;
	case endbar:
	  qDebug("endbar, bar %d\n", voltarray[i].barno);
	  break;
	case doublebar:
	  qDebug("doublebar, bar %d\n", voltarray[i].barno);
	  break;
	case startending:
	  qDebug("startending, bar %d\n", voltarray[i].barno);
	  break;
	case endending:
	  qDebug("endending, bar %d\n", voltarray[i].barno);
	  break;
	default:
	  break;
	}

    }
}



bool MuseScore::saveLilypond(Score* score, const QString& name)
{
  ExportLy em(score);
  return em.write(name);
}



void ExportLy::writeClef(int clef)
{
  out << "\\clef ";
  switch(clef) {
  case ClefType::G:      out << "treble\n";         break;
  case ClefType::F:      out << "bass\n";           break;
  case ClefType::G1:     out << "\"treble^8\"\n";   break;
  case ClefType::G2:     out << "\"treble^15\"\n";  break;
  case ClefType::G3:     out << "\"treble_8\"\n";   break;
  case ClefType::F8:     out << "\"bass_8\"\n";     break;
  case ClefType::F15:    out << "\"bass_15\"\n";    break;
  case ClefType::F_B:    out << "bass\n";           break;
  case ClefType::F_C:    out << "bass\n";           break;
  case ClefType::C1:     out <<  "soprano\n";       break;
  case ClefType::C2:     out <<  "mezzo-soprano\n"; break;
  case ClefType::C3:     out <<  "alto\n";          break;
  case ClefType::C4:     out <<  "tenor\n";         break;
  case ClefType::TAB2:
  case ClefType::TAB:    out <<  "tab\n";           break;
  case ClefType::PERC:   out <<  "percussion\n";    break;
  }

}


void ExportLy::writeTimeSig(TimeSig* sig)
{
  int st     = sig->timeSigType();
  Fraction f = sig->sig();
  timedenom  = f.denominator();
  z1         = f.numerator();

    if (st == TSIG_ALLA_BREVE)
    {
      z1=2;
      timedenom=2;
          }
  indent();
  out << "\\time " << z1 << "/" << timedenom << " ";
}


void ExportLy::writeKeySig(int st)
{
  st = char(st & 0xff);
  out << "\\key ";
  switch(st) {
  case 7:  out << "cis"; break;
  case 6:  out << "fis"; break;
  case 5:  out << "b";   break;
  case 4:  out << "e";   break;
  case 3:  out << "a";   break;
  case 2:  out << "d";   break;
  case 1:  out << "g";   break;
  case 0:  out << "c";   break;
  case -7: out << "ces"; break;
  case -6: out << "ges"; break;
  case -5: out << "des"; break;
  case -4: out << "as";  break;
  case -3: out << "es";  break;
  case -2: out << "bes"; break;
  case -1: out << "f";   break;
  default:
    qDebug("illegal key %d\n", st);
    break;
  }
  out << " \\major \n";
}


QString ExportLy::tpc2name(int tpc)
{
  const char names[] = "fcgdaeb";
  int acc   = ((tpc+1) / 7) - 2;
  QString s(names[(tpc + 1) % 7]);
  switch(acc) {
  case -2: s += "eses"; break;
  case -1: s += "es";  break;
  case  1: s += "is";  break;
  case  2: s += "isis"; break;
  case  0: break;
  default: s += "??"; break;
  }
  return s;
}



QString ExportLy::tpc2purename(int tpc)
{
  const char names[] = "fcgdaeb";
  QString s(names[(tpc + 1) % 7]);
  return s;
}



int ExportLy::findSlur(const Slur* s) const
{
  for (int i = 0; i < 8; ++i)
    if (slurre[i] == s) return i;
  return -1;
}


void ExportLy::doSlurStart(Chord* chord, bool nextisrest)
{
#if 0   int slurcount=0;
  for(const Spanner* sp = chord->spannerFor(); sp; sp = sp->next())
    {
      if (sp->type() != Element::SLUR)
            continue;
      const Slur* s = static_cast<const Slur*>(sp);

      slurcount++;

      int i = findSlur(s);

      if (i >= 0)
	{
	  slurstack++;
	  slurre[i] = 0;
	  started[i] = false;
	  if (s->slurDirection() == MScore::UP) out << "^";
	  if (s->slurDirection() == MScore::DOWN) out << "_";
	  if (slurcount==2)
	    {
	      phraseslur=slurstack;
	      out <<"\\";
	    }
	  if (nextisrest)
	    {
	      out << "\\laissezVibrer " ;
	    }
	    else
	      out << "(";

	}
      else
	{
	  i = findSlur(0);
	  if (i >= 0)
	    {
	      slurstack++;
	      slurre[i] = s;
	      started[i] = true;
	      if (s->slurDirection() == MScore::UP) out << "^";
	      if (s->slurDirection() == MScore::DOWN) out << "_";
	      if (slurcount==2)
		{
		  phraseslur=slurstack;
		  out <<"\\";
		}

	      if (nextisrest)
	     {
	       out << "\\laissezVibrer " ;
	     }
	     else
	      out << "(";
	    }
	  else
	    qDebug("no free slur slot");
	}
    }
#endif

}


void ExportLy::doSlurStop(Chord* chord)
{
#if 0   for(const Spanner* sp = chord->spannerBack(); sp; sp = sp->next())
    {
    if (sp->type() != Element::SLUR)
          continue;
    const Slur* s = static_cast<const Slur*>(sp);

            int i = findSlur(s);
      if (i < 0)
	{
	  	  i = findSlur(0);
	  if (i >= 0)
	    {
	      slurre[i] = s;
	      started[i] = false;
	      if (slurstack == phraseslur)
		{
		  phraseslur=0;
		  out << "\\";
		}
	      slurstack--;
	      out << ")";  	    }
	  else
	    qDebug("no free slur slot");
	}
    }
#endif
      for (int i = 0; i < 8; ++i) {
            if (slurre[i]) {
#if 0                   if  (slurre[i]->endElement() == chord) {
                        if (started[i]) {
                              slurre[i] = 0;
                              started[i] = false;
                              if (phraseslur == slurstack) {
                                    out << "\\";
     	                              phraseslur = 0;
     	                              }
                              slurstack--;
                              out << ")";                               }
                        }
#endif
	            }
            }
      }

void ExportLy::checkSlur(Chord* chord, bool nextisrest)
{
    for (int i = 0; i < 8; ++i)
    {
      slurre[i] = 0;
      started[i] = false;
     }
  doSlurStop(chord);
  doSlurStart(chord, nextisrest);
}



void ExportLy::writeArticulation(ChordRest* c)
{
  foreach(Articulation* a, c->articulations())
    {
      switch(a->articulationType())
	{
	case Articulation_Fermata:
        if (a->up())
	      out << "\\fermata ";
        else
	      out << "_\\fermata ";
	  break;
	case Articulation_Thumb:
	  out << "\\thumb ";
	  break;
	case Articulation_Sforzatoaccent:
	  out << "-> ";
	  break;
	case Articulation_Espressivo:
	  out << "\\espressivo ";
	  break;
	case Articulation_Staccato:
	  out << "-. ";
	  break;
	case Articulation_Staccatissimo:
        if (a->up())
	      out << "-| ";
        else
	      out << "_| ";
	  break;
	case Articulation_Tenuto:
	  out << "-- ";
	  break;
	case Articulation_Portato:
        if (a->up())
	      out << "-_ ";
        else
	      out << "__ ";
	  break;
	case Articulation_Marcato:
        if (a->up())
	      out << "-^ ";
        else
	      out << "_^ ";
	  break;
	case Articulation_Ouvert:
	  out << "\\open ";
	  break;
	case Articulation_Plusstop:
	  out << "-+ ";
	  break;
	case Articulation_Upbow:
	  out << "\\upbow ";
	  break;
	case Articulation_Downbow:
	  out << "\\downbow ";
	  break;
	case Articulation_Reverseturn:
	  out << "\\reverseturn ";
	  break;
	case Articulation_Turn:
	  out << "\\turn ";
	  break;
	case Articulation_Trill:
	  out << "\\trill ";
	  break;
	case Articulation_Prall:
	  out << "\\prall ";
	  break;
	case Articulation_Mordent:
	  out << "\\mordent ";
	  break;
	case Articulation_PrallPrall:
	  out << "\\prallprall ";
	  break;
	case Articulation_PrallMordent:
	  out << "\\prallmordent ";
	  break;
	case Articulation_UpPrall:
	  out << "\\prallup ";
	  break;
	case Articulation_DownPrall:
	  out << "\\pralldown ";
	  break;
	case Articulation_UpMordent:
	  out << "\\upmordent ";
	  break;
	case Articulation_DownMordent:
	  out << "\\downmordent ";
	  break;
	default:
	  qDebug("unsupported note attribute %d\n", int(a->articulationType()));
	  break;
	}    }}


void ExportLy::writeTremolo(Chord * chord)
{
  if (chord->tremolo())
    {
      Tremolo * tr = chord->tremolo();
      int st = tr->tremoloType();
      switch (st)
	{
	case TREMOLO_R8:
	  out << ":8 ";
	  break;
	case TREMOLO_R16:
	  out << ":16 ";
	  break;
	case TREMOLO_R32:
	  out << ":32 ";
	  break;
	case TREMOLO_R64:
	  out << ":64 ";
	  break;
	default:
	  qDebug("unknown tremolo %d\n", st);
	  break;
	}
    }
}



void ExportLy::findFingerAndStringno(Note* note, int &fingix, int &stringix, QString (&fingarray)[5], QString (&stringarray)[10])
      {
      foreach (const Element* e, note->el()) {
            if (e->type() == Element::FINGERING) {
                  const Text* text = static_cast<const Text*>(e);
                  if (text->textStyleType() == TEXT_STYLE_FINGERING) {
	                  fingix++;
      	            Text* f = (Text*)e;
	                  fingarray[fingix] = f->text();
	                  }
                  else if (text->textStyleType() == TEXT_STYLE_STRING_NUMBER) {
                        stringix++;
                        Text * s = (Text*)e;
                        stringarray[stringix] = s->text();
                        }
                  }
            }
      }

void ExportLy::writeStringInstruction(int &strgix, QString stringarr[10])
{
  if (strgix > 0)
    {        for (int i=0; i < strgix; i++)
	out << "\\" << stringarr[strgix];
    }
  strgix = 0;
}


void ExportLy::writeFingering (int &fingr,   QString fingering[5])
{
  if (fingr > 0)
	{
	  if (fingr == 1) out << "-" << fingering[1] << " ";
	  else if (fingr >1)
	    {
	      out << "^\\markup {\\finger \"";
	      out << fingering[1] << " - " << fingering[2] << "\"} ";
	    }
	}
  fingr=0;
}


void ExportLy::stemDir(Chord * chord)
{
    if (chord->beam() == 0 || chord->beam()->elements().front() == chord)
    {
      MScore::Direction d = chord->stemDirection();
      if (d != stemDirection)
	{
	  stemDirection = d;
	  if ((d == MScore::UP) and (graceswitch == true))
	    out << "\\stemUp ";
	  else if ((d == MScore::DOWN)  and (graceswitch == true))
	    out << "\\stemDown ";
	  	  	  	  	  	  	  	}
    }
}
void ExportLy::findGraceNotes(Note *note, bool &chordstart, int streng)
{
  NoteType gracen;
  gracen = note->noteType();
  switch(gracen)
    {
    case NOTE_INVALID:
    case NOTE_NORMAL:
      if (graceswitch==true)
	{
	  graceswitch=false;
	  gracebeam=false;
	  if (gracecount > 1) out << " ] "; 	  out << " } \\stemNeutral "; 	  gracecount=0;
	}
      if ((chordstart) or (streng > 0))
	{
	  out << "<";
	  chordstart=false;
	}
      break;
    case NOTE_ACCIACCATURA:
    case NOTE_APPOGGIATURA:
    case NOTE_GRACE4:
    case NOTE_GRACE16:
    case NOTE_GRACE32:
      if (graceswitch==false)
	{
	  out << "\\grace{\\stemUp "; 	  graceswitch=true;
	  gracebeam=false;
	  gracecount=0;
	}
      gracecount++;
      break;
    } }
void ExportLy::setOctave(int &purepitch, int &pitchidx, int (&pitchlist)[12])
{
  int oktavdiff=prevpitch - purepitch;
  int oktreit=numval(oktavdiff);
  while (oktreit > 0)
    {
      if ((oktavdiff < -6) or ((prevnote=="b") and (oktavdiff < -5)))
	{ 	  out << "'";
	  oktavdiff=oktavdiff+12;
	}
      else if ((oktavdiff > 6)  or ((prevnote=="f") and (oktavdiff > 5)))
	{	  out << ",";
	  oktavdiff=oktavdiff-12;
	}
      oktreit=oktreit-12;
    }
  prevpitch=purepitch;
  pitchlist[pitchidx]=purepitch;
  pitchidx++;
}

bool ExportLy::arpeggioTest(Chord* chord)
{
  bool arp=false;
  if (chord->arpeggio())
    {
      arp=true;
      int subtype = int(chord->arpeggio()->arpeggioType());
      switch (subtype)
	{
	case 0:
	  out << "\\arpeggioNormal ";
	  break;
	case 1:
	  out << "\\arpeggioArrowUp ";
	  break;
	case 2:
	  out << "\\arpeggioArrowDown ";
	  break;
	default:
	  qDebug("unknown arpeggio subtype %d\n", subtype);
	  break;
	}
    }
  return arp;
}


bool ExportLy::glissandotest(Chord* chord)
{
  bool gliss=false;
  int i=0;
  for (i=0; i < glisscount; i++)
    {
      if (glisstable[i].chord == chord)
	{
	  if (glisstable[i].type == 1)
	    {
	      out << "\\once\\override Glissando #'style = #'trill \n";
	      indent();
	    }
	  gliss=true;
	}
    }
  return gliss;
}



bool ExportLy::findNoteSymbol(Note* n, QString& symbolname)
      {
      symbolname = "";

      foreach(const Element* symbol, n->el()) {
            if (symbol->type() == Element::SYMBOL) {
                  const Symbol* symb = static_cast<const Symbol*>(symbol);
                  symbolname = Sym::id2name(symb->sym());
                  return true;                   }
            }
      return false;
      }

void ExportLy::writeChord(Chord* c, bool nextisrest)
{
  int  purepitch;
  QString purename, chordnote;
  int pitchlist[12];
  QString fingering[5];
  QString stringno[10];
  bool tie=false;
  bool symb=false;
  QList<Note*> nl = c->notes();
  bool chordstart=false;
  int fing=0;
  int streng=0;
  bool gliss=false;
  QString glisstext;
  QString symbolname;

  int j=0;
  for (j=0; j<12; j++) pitchlist[j]=0;

  stemDir(c);

  if (nl.size() > 1) chordstart = true;

  int  pitchidx=0;
  bool arpeggioswitch=false;
  arpeggioswitch=arpeggioTest(c);

  gliss = glissandotest(c);
  int iter=0;
  for (QList<Note*>::iterator notesinchord = nl.begin();;)
    {
	  iter++;
      Note* n = *notesinchord;
            if (fing>0)  writeFingering(fing,fingering);
      if (streng>0) writeStringInstruction(streng,stringno);

            findFingerAndStringno(n, fing, streng, fingering, stringno);

      if (iter == 1) findTuplets(n->chord());

      findGraceNotes(n, chordstart, streng);
      symb = findNoteSymbol(n, symbolname);

      if (n->tieFor()) tie=true;

      if (gracecount==2) out << " [ ";


      out << tpc2name(n->tpc()).toUtf8().data();  
      if ((chordstart) and (symb))
	{
	  cout << "symbol in chord\n";
	  writeSymbol(symbolname);
	}

      purepitch = n->pitch();
      purename = tpc2name(n->tpc());        prevnote=cleannote;                   cleannote=tpc2purename(n->tpc());
      if (purename.contains("eses")==1)  purepitch=purepitch+2;
      else if (purename.contains("es")==1)  purepitch=purepitch+1;
      else if (purename.contains("isis")==1) purepitch=purepitch-2;
      else if (purename.contains("is")==1) purepitch=purepitch-1;

      setOctave(purepitch, pitchidx, pitchlist);

      if (notesinchord == nl.begin())
	{
	  chordpitch=prevpitch;
	  chordnote=cleannote;
	}

      ++notesinchord;       if (notesinchord == nl.end())
	break;
      out << " ";
    } 
  if ((nl.size() > 1) or (streng > 0))
    {
            if (fing   > 0) writeFingering(fing, fingering);
      if (streng > 0) writeStringInstruction(streng,stringno);
      out << ">";       cleannote=chordnote;
                }

  int ix=0;
  prevpitch=pitchlist[0];
   while (pitchlist[ix] !=0)
     {
       if (pitchlist[ix]<prevpitch) prevpitch=pitchlist[ix];
       ix++;
     }

  writeLen(c->actualTicks());

  if ((symb) and (nl.size() == 1))
    writeSymbol(symbolname);

  if (arpeggioswitch)
    {
      out << "\\arpeggio ";
      arpeggioswitch=false;
    }


    if (nl.size() == 1)
    writeFingering(fing, fingering);

  writeTremolo(c);

  if (gliss)
    {
      out << "\\glissando ";
      if (glisstable[glisscount].glisstext !="")
	out << "^\\markup{" << glisstable[glisscount].glisstext << "} ";
          }

  if (tie)
    {
      out << "~";
      tie=false;
    }

  writeArticulation(c);
  checkSlur(c, nextisrest);

  out << " ";

}


int ExportLy::getLen(int l, int* dots)
{
  int len  = 4;

  if (l == 16 * MScore::division)     len=-2;
  else if (l == 12 * MScore::division)     len=-3;
  else if (l == 10 * MScore::division)     len=-4;
  else if (l == 8 * MScore::division)     len = -1;
  else if (l == 7 * MScore::division)     {
      len = 1;
      *dots = 2;
    }
  else if (l == 6 * MScore::division)     {
      len  = 1;
      *dots = 1;
    }
  else if (l == 5 * MScore::division)       len = -5;
  else if (l == 4 * MScore::division)     len = 1;
  else if (l == 3 * MScore::division)     {
      len = 2;
      *dots = 1;
    }
  else if (l == ((MScore::division/2)*7))     {
      len = 2;
      *dots=2;
    }
  else if (l == 2 * MScore::division)
    len = 2;
  else if (l == MScore::division)             len = 4;
  else if (l == MScore::division *3 /2)       {
      len=4;
      *dots=1;
    }
  else if (l == ((MScore::division/4)*7))     {
      len = 4;
      *dots=2;
    }
  else if (l == MScore::division / 2)         len = 8;
  else if (l == MScore::division*3 /4)     {
      len = 8;
      *dots=1;
    }
  else if (l == ((MScore::division/8)*7))     {
      len = 8;
      *dots=2;
    }
  else if (l == MScore::division / 4)
    len = 16;
  else if (l == MScore::division / 8)
    len = 32;
  else if (l == MScore::division * 3 /8)     {
      len = 16;
      *dots = 1;
    }
  else if (l == ((MScore::division/16)*7))     {
      len = 16;
      *dots=2;
    }
  else if (l == MScore::division / 16)
    len = 64;
  else if (l == MScore::division /32)
    len = 128;
      else if (l == ((MScore::division  * 8)/3))
     len = 1;
  else if (l == MScore::division * 4 /3)
     len = 2;
  else if (l == (MScore::division * 2)/3)
    len = 4;
  else if (l == MScore::division /3)
    len = 8;
  else if (l == MScore::division /(3*2))
    len = 16;
  else if (l == MScore::division /3*4)
    len = 32;
  else if (l == MScore::division/3*8)
    len = 64;
  else if (l == 0)
    len = 1;
  else qDebug("measure: %d, unsupported len %d (%d,%d)\n", measurenumber, l, l/MScore::division, l % MScore::division);
  return len;
}


void ExportLy::writeLen(int ticks)
{
  int dots = 0;
  int len = getLen(ticks, &dots);

  if (ticks != curTicks)
    {
      switch (len)
	{
	case -5:
	  out << "1*5/4";
	  break;
	case -4:
	  out << "2*5 ";
	  break;
	case -3:
	  out << "1.*2 ";
	  break;
	case -2:	  out << "\\longa ";
	    break;
	case -1: 	  out << "\\breve";
	  break;
	default:
	  out << len;
	  for (int i = 0; i < dots; ++i)
	    out << ".";
	  break;
	}
      curTicks = ticks;
      if (dots>0)
	curTicks = -1;     }
}


void ExportLy::writeRest(int l, int type)
{
  if (type == 1)     {
      out << "R";
      curTicks = -1;       writeLen(l);
      wholemeasurerest=1;
     }
  else if (type == 2)     {
      curTicks = -1;
      out << "s";
      writeLen(l);
    }
  else     {
      out << "r";
     writeLen(l);
    }
  out << " ";
}

void ExportLy::writeMeasuRestNum()
{
  if (wholemeasurerest >1) out << "*" << wholemeasurerest << " ";
  if (wholemeasuretext != "")
    {
      out << "^\\markup{" << wholemeasuretext << "} \n";
      indent();
    }
  out << " | % \n";
  indent();
  wholemeasurerest=0;
  wholemeasuretext= "";
  curTicks = -9;
}

void ExportLy::writeVolta(int measurenumber, int lastind)
{
  bool utgang=false;
  int i=0;

  if (pickup)
    measurenumber--;
  while ((voltarray[i].barno < measurenumber) and (i<=lastind))
    {
            i++;
    }

  if (measurenumber==voltarray[i].barno)
    {
      while (utgang==false)
	{
	  switch(voltarray[i].voltart)
	    {
	    case startrepeat:
	      if (wholemeasurerest > 0) writeMeasuRestNum();
	      indent();
	      out << "\\repeat volta 2 { %startrep \n";
	      firstalt=false;
	      secondalt=false;
	      repeatactive=true;
	      curTicks=-1;
	      break;
	    case endrepeat:
	      if ((repeatactive==true) and (secondalt==false))
		{
		  if (wholemeasurerest > 0) writeMeasuRestNum();
		  out << "} % end of repeatactive\n";
		  curTicks=-1;
		  		}
	      indent();
	      break;
	    case bothrepeat:
	      if (firstalt==false)
		{
		  if (wholemeasurerest > 0) writeMeasuRestNum();
		  out << "} % end of repeat (both)\n";
		  indent();
		  out << "\\repeat volta 2 { % bothrep \n";
		  firstalt=false;
		  secondalt=false;
		  repeatactive=true;
		  curTicks=-1;
		}
	      break;
	    case doublebar:
	      if (wholemeasurerest > 0) writeMeasuRestNum();
	      out << "\n";
	      indent();
	      out << "\\bar \"||\"";
	      curTicks=-1;
	      break;
	    case startending:
	      if (firstalt==false)
		{
		  if (wholemeasurerest > 0) writeMeasuRestNum();
		  out << "} % end of repeat except alternate endings\n";
		  indent();
		  out << "\\alternative{ {  ";
		  firstalt=true;
		  curTicks=-1;
		}
	      else
		{
		  if (wholemeasurerest > 0) writeMeasuRestNum();		  out << "{ ";
		  indent();
		  firstalt=false;
		  secondalt=true;
		  curTicks=-1;
		}
	      break;
	    case endending:
	      if (firstalt)
		{
		  if (wholemeasurerest > 0) writeMeasuRestNum();
		  out << "} %close alt1\n";
		  secondalt=true;
		  repeatactive=true;
		  curTicks=-1;
		}
	      else
		{
		  if (wholemeasurerest > 0) writeMeasuRestNum();
		  out << "} } %close alternatives\n";
		  secondalt=false;
		  firstalt=true;
		  repeatactive=false;
		  curTicks=-1;
		}
	      break;
	    case endbar:
	      if (wholemeasurerest > 0) writeMeasuRestNum();
	      out << "\\bar \"|.\"";
	      curTicks=-1;
	      break;
          default:
	    	    break;
	    }
	  if (voltarray[i+1].barno==measurenumber)
	    {
	      i++;
	    }
	  else utgang=true;
	}    }}


static void checkIfNextIsRest(MeasureBase* mb, Segment* s, bool &nextisrest, int track)
{
  nextisrest = false;
  Segment* nextseg = s->next();
  Element*  nextelem;
  nextelem= nextseg->element(track);

  while (!(nextseg->segmentType() == SegmentType::EndBarLine))    {
            if (nextseg->isChordRest())	break;
      nextseg = nextseg->next();
      nextelem = nextseg->element(track);     }

      while ((nextelem==0) and (!(nextseg->segmentType() == SegmentType::EndBarLine)))
    {
      nextseg = nextseg->next();
      nextelem = nextseg->element(track);
    }

      if ((nextseg->segmentType() != SegmentType::EndBarLine) &&  (nextseg->isChordRest()))
    {
                  if ((!(nextelem == 0 || nextelem->generated())))
	{
	  if (nextelem->type() == Element::REST)
	    {
	      nextisrest=true;
	    }
	}
    }
  else     {
            if (mb->next()) 	{
	  mb = mb->next();
	  if (mb->type() == Element::MEASURE)
	    {
	      Measure* meas = (Measure*) mb;
	      for(Segment* s = meas->first(); s; s = s->next())
		{
		  if (s->isChordRest())
		    {
		      Element* elem = s->element(track);
		      if (!(elem == 0 ||  elem->generated()))
			{
			  if (elem->type() == Element::REST)
			    {
			      nextisrest=true;
			    }
			  else if (elem->type() == Element::CHORD)
			    {
			      			    }
			}
		      break; 		    }
		}
	    }
	}
      else nextisrest=false;
    }
}




void ExportLy::newLyricsRecord()
{
  lyricsRecord* lyrrec;
  lyrrec = new lyricsRecord();

  for (int i = 0; i < VERSES; i++)
    {
      lyrrec->lyrdat.tick[i]=0;
      lyrrec->lyrdat.verselyrics[i] = "";
      lyrrec->lyrdat.segmentnumber[i] = 0;
    }
  lyrrec->lyrdat.staffname = staffname[staffInd].staffid;
  lyrrec->numberofverses=-1;
  lyrrec->next = NULL;
  lyrrec->prev = NULL;

  if (tailOfLyrics != NULL)
    {
      lyrrec->prev = tailOfLyrics;
      tailOfLyrics->next = lyrrec;

    }

  tailOfLyrics = lyrrec;
  thisLyrics = lyrrec;

  if (headOfLyrics == NULL)  headOfLyrics = lyrrec;
}

void ExportLy::findLyrics()
{
  int verse = 0;
  int track = 0;
  int vox = 0;
  int prevverse =  0;

  for (int staffno=0; staffno < staffInd; staffno++)
    {
      newLyricsRecord();
      for (MeasureBase* mb = score->first(); mb; mb = mb->next())
	{
	  if (mb->type() != Element::MEASURE)
	    continue;
	  Measure* meas = (Measure*)mb;

        SegmentType st = SegmentType::ChordRest | SegmentType::Grace;
	  for(Segment* seg = meas->first(st); seg; seg = seg->next(st))
	    {
        const QList<Lyrics*>* lyrlist = seg->lyricsList(staffno*VOICES);
            if (!lyrlist)
                  continue;

	      foreach(const Lyrics* lix, *lyrlist)
		{
		  if (lix)
		    {
		      verse = (lix)->no();
		      if ((verse - prevverse) > 1)
			{
			  thisLyrics->lyrdat.verselyrics[verse-1] += "__ _ ";
			}
		      track = (lix)->track();
		      vox = track - (staffno*VOICES);

		      thisLyrics->lyrdat.segmentnumber[verse]++;
		      thisLyrics->lyrdat.tick[verse] = (lix)->segment()->tick();

		      if (verse > thisLyrics->numberofverses)
			{
			  thisLyrics->numberofverses = verse;
			  if (verse > 0)
			    {
			      int segdiff = (thisLyrics->lyrdat.segmentnumber[verse-1] -  thisLyrics->lyrdat.segmentnumber[verse]);
			      if (segdiff > 0)
				{
				  for (int i = 0; i < segdiff; i++)
				    thisLyrics->lyrdat.verselyrics[verse] += " _ ";
				  thisLyrics->lyrdat.segmentnumber[verse] += segdiff;
				}
			    }
			}

		      QString lyriks = (lix)->text();

                    if (lyriks.contains('"'))
                 lyriks = "\"" + lyriks.replace("\"","\\\"") + "\"";

		      lyriks = lyriks.replace(" ", "_"); 		      thisLyrics->lyrdat.verselyrics[verse] += lyriks;

		      thisLyrics->lyrdat.staffname =  staffname[staffno].staffid;
		      thisLyrics->lyrdat.voicename[verse] = staffname[staffno].voicename[vox];

		      thisLyrics->lyrdat.tick[verse] = (lix)->segment()->tick();

		      int syl   = (lix)->syllabic();
		      switch(syl)
			{
			case Lyrics::SINGLE:
			  thisLyrics ->lyrdat.verselyrics[verse] += " ";
			  break;
			case Lyrics::BEGIN:
			  thisLyrics->lyrdat.verselyrics[verse] +=  " -- ";
			  break;
			case Lyrics::END:
			  thisLyrics->lyrdat.verselyrics[verse] += "  ";
			  break;
			case Lyrics::MIDDLE:
			  thisLyrics->lyrdat.verselyrics[verse] += " -- ";
			  break;
			default:
			  qDebug("unknown syllabic %d\n", syl);
			}		      cout << " lyrics endtick: " << (lix)->endTick() << "\n";
		      if ((lix)->ticks() > 0) 			{
			  cout << " _ ";
			  thisLyrics->lyrdat.verselyrics[verse] += " _ ";
			}
		    } 		  prevverse = verse;
		} 		if (verse < thisLyrics->numberofverses)
		  thisLyrics->lyrdat.verselyrics[thisLyrics->numberofverses] += "__ _ ";
	    } 	}     } }
void ExportLy::writeLyrics()
{

  thisLyrics = headOfLyrics;
  tailOfLyrics->next = NULL;  int staffi=0;
  int stanza=0;

  while (thisLyrics != NULL)
    {
      staffi=0;
      while (staffname[staffi].staffid != "laststaff")
	{
	  for (int j=0; j< staffname[staffi].numberofvoices; j++)
	    {
		  stanza=0;
	      for (int ix = 0; ix < thisLyrics->numberofverses+1; ix++)		{
		  if ((thisLyrics->lyrdat.staffname == staffname[staffi].staffid)
		      and (thisLyrics->lyrdat.voicename[ix] == staffname[staffi].voicename[j]))
		    {
		      indentF();
		      stanza++;
		      char verseno = (ix + 65);
		      os << "  " << thisLyrics->lyrdat.staffname;
		      os << "verse" << verseno << " = \\lyricmode { \\set stanza = \" " << stanza << ". \" ";
		      os << thisLyrics->lyrdat.verselyrics[ix] << "}\n";
		    }
		}
	    }
	  staffi++;
	}
             thisLyrics = thisLyrics->next;
    }
  thisLyrics = headOfLyrics;
}




void ExportLy::connectLyricsToStaff()
{
  
  thisLyrics =headOfLyrics;
  while (thisLyrics != NULL)
    {
      for (int j=0; j< staffname[indx].numberofvoices; j++)
	{
	  for (int ix = 0; ix <= thisLyrics->numberofverses; ix++)	    {
	      if (thisLyrics->lyrdat.staffname == staffname[indx].staffid)
		{
		  if (thisLyrics->lyrdat.voicename[ix] == staffname[indx].voicename[j])
		    {
		      indentF();
		      char verseno = ix + 65;
		      os << " \\context Lyrics = " << staffname[indx].staffid;
		      os << "verse"<< verseno;
		      os <<  "\\lyricsto ";
		      os << thisLyrics->lyrdat.voicename[ix] << "  \\";
		      os << thisLyrics->lyrdat.staffname << "verse" << verseno << "\n";;
		    }
		}
	    }
	}

            thisLyrics = thisLyrics->next;
    }
  os << "\n";
}
void ExportLy::cleanupLyrics()
{
  thisLyrics=headOfLyrics;
  while (thisLyrics !=NULL)
    {
      headOfLyrics=headOfLyrics->next;
      delete thisLyrics;
      thisLyrics=headOfLyrics;
    }
}



QString ExportLy::flatInInstrName(QString name)
{
    int pt = 0;
  QChar kar;
  int unum;
  bool flat=false;
  QString newname="";
  for (pt = 0; pt < name.size(); pt++)
    {
      kar=name.at(pt);
      unum = kar.unicode();
      if (unum < 256)
	{
	  newname.append(kar);
	}
      else if (unum == 57613)
					{
	  newname.append("\\smaller \\flat ");
	  flat=true;
	}
    }
  if (flat)
    {
      newname.prepend("\\markup{");
      newname.append("}");
    }
  else newname = "";
  return newname;
}



void ExportLy::writeVoiceMeasure(MeasureBase* mb, Staff* staff, int staffInd, int voice)

{
  int i=0;
  char cvoicenum, cstaffnum;
  bool  barempty=true;
  bool nextisrest=false;
  Measure* m = (Measure*) mb;

    if ((m->no() > 0) and (wholemeasurerest==0) and (textspanswitch==false))
    {
      indent();
      out << " | % " << m->no() << "\n" ;
    }
  measurenumber=m->no()+1;

            

   if ((measurenumber==1) and (donefirst==false))
         {
      donefirst=true;
      level=0;
      indent();
      cvoicenum=voice+65;
      cstaffnum= staffInd+65;
            staffname[staffInd].voicename[voice] = staffname[staffInd].partshort;
      staffname[staffInd].voicename[voice].append("voice");
      staffname[staffInd].voicename[voice].append(cstaffnum);
      staffname[staffInd].voicename[voice].append(cvoicenum);
      staffname[staffInd].voicename[voice].prepend("A");
      staffname[staffInd].voicename[voice].remove(QRegExp("[0-9]"));
      staffname[staffInd].voicename[voice].remove(QChar('.'));
      staffname[staffInd].voicename[voice].remove(QChar(' '));

      out << staffname[staffInd].voicename[voice];
      out << " = \\relative c" << relativ;
      indent();
      out << "{\n";
      level++;
      indent();
      if (voice==0)
	{
	  QString flatpartn="";
	  QString flatshortn="";

	  cout << "X" << staffname[staffInd].partname.toUtf8().data() << "x\n";

	  flatpartn = flatInInstrName(staffname[staffInd].partname);
	  flatshortn = flatInInstrName(staffname[staffInd].partshort);

	  out <<"\\set Staff.instrumentName = ";

	  cout << "F" << flatpartn.toUtf8().data() << "f\n";

	  if (flatpartn == "")
	    out<< "#\"" << staffname[staffInd].partname << "\"";
	  else
	    out << flatpartn;
	  out << "\n";

	  indent();
	  out << "\\set Staff.shortInstrumentName = ";
	  if (flatshortn =="")
	    out << "#\"" << staffname[staffInd].partshort << "\"";
	  else
	    out << flatshortn;
	  out << "\n";

	  indent();
	  writeClef(staff->clef(0));
	  indent();
	  out << "%staffkeysig\n";
	  indent();
	  	  writeKeySig(staff->keys()->key(0).accidentalType());
	}

      cout << "pianostaff: " << pianostaff << "\n";

      if (pianostaff==false)
			{
	  switch(voice)
	    {
	    case 0: break;
	      	      	    case 1:
	      out <<"\\voiceTwo" <<"\n\n";
	      break;
	    case 2:
	      out <<"\\voiceThree" <<"\n\n";
	      break;
	    case 3:
	      out <<"\\voiceFour" <<"\n\n";
	      break;
	    }
	}

                  i=0;
      while ((voltarray[i].voltart != startrepeat) and (voltarray[i].voltart != endrepeat)
	     and (voltarray[i].voltart !=bothrepeat) and (i<=lastind))
	{
	  i++;
	}

      if (i<=lastind)
	{
	  if ((voltarray[i].voltart==endrepeat) or (voltarray[i].voltart==bothrepeat))
	    {
	      indent();
	      out << "\\repeat volta 2 { \n";
	      repeatactive=true;
	    }
	}
    }
   if (wholemeasurerest < 1) indent();
   int tick = m->tick();
   int measuretick=0;
   Element* e;

   for(Segment* s = m->first(); s; s = s->next())
     {
              int track = staffInd * VOICES + voice;
       e = s->element(track);

       if (!(e == 0 || e->generated()))
	 {
	   voiceActive[voice] = true;
	   barempty = false;
	 }
       else
         continue;

       handlePreInstruction(e);        barlen=m->ticks();
              switch(e->type())
	 {
	 case Element::CLEF:
	   if (wholemeasurerest >=1) writeMeasuRestNum();
	   writeClef(static_cast<Clef*>(e)->clefType());
	   indent();
	   break;
	 case Element::TIMESIG:
	   {
		 if (wholemeasurerest >=1)
		       writeMeasuRestNum();
		 out << "%bartimesig: \n";
		 writeTimeSig((TimeSig*)e);
		 out << "\n";

		 int nombarlen=z1*MScore::division;

		 if (timedenom==8) nombarlen=nombarlen/2;
		 if (timedenom == 2) nombarlen = 2*nombarlen;

		 if ((barlen<nombarlen) and (measurenumber==1) and (voice == 0))
		       {
			     pickup = true;
           partial = true;
			     indent();
			     const SigEvent ev(m->score()->sigmap()->timesig(m->tick()));
      	   out << "\\partial " << ev.timesig().denominator() << "*" << ev.timesig().numerator() << "\n";
		       }
		 curTicks=-1; 		 indent();
		 break;
	   }
	 case Element::KEYSIG:
	     {
		 if (wholemeasurerest >=1) writeMeasuRestNum();

		 out << "%barkeysig: \n";
		 		 
		 
		 		 		 		 		 		 		 
		 KeySig* ksig= (KeySig*) e;
		 int keytick = ksig->tick();
		 cout << "at tick: " << keytick << "\n";
		 KeyList* kl = score->staff(staffInd)-> keys();
		 KeySigEvent key = kl->key(keytick);
		 		 		 			 cout << "barkeysig: " << key.accidentalType() << " measureno: " << measurenumber << "\n";
			 indent();
			 writeKeySig(key.accidentalType());
		 
		 indent();
		 curTicks=-1; 		 break;
	     }
	 case Element::CHORD:
	     {
		 if (wholemeasurerest >=1) writeMeasuRestNum();
		 int ntick = static_cast<Chord*>(e)->tick() - tick;
		 if (ntick > 0)
		     {
			 writeRest(ntick, 2);			 curTicks=-1;
		     }
		 tick += ntick;
		 measuretick=measuretick+ntick;
		 checkIfNextIsRest(mb, s, nextisrest, track);
		 writeChord((Chord*)e, nextisrest);
		 tick += ((Chord*)e)->actualTicks();
		 measuretick=measuretick+((Chord*)e)->actualTicks();
		 break;
	     }
	 case Element::REST:
	   {
	     bool articul=false;
	     findTuplets((ChordRest *) e);

	     QList<Articulation*> a;
	     ChordRest * CR = (ChordRest*) e;

	     a = CR->articulations();

	     if (!(a.isEmpty()) ) articul = true;

	     int l = ((Rest*)e)->actualTicks();
	     int mlen=((Rest*)e)->segment()->measure()->ticks();

	     int nombarl=z1*MScore::division;

	     if (((l==mlen) || (l==0)) and (mlen ==nombarl))  	       {
		 if (wholemeasurerest > 0)
		   {
		     if (articul)
		       {
			 writeMeasuRestNum();
			 writeRest(l,0);
		         writeArticulation((ChordRest*) e);
		       }
		     else
		     wholemeasurerest++;
		   }
		 else
		   {
		     		     l = ((Rest*)e)->segment()->measure()->ticks();
		     if (articul)
		       {
			 writeRest(l,0);
			 writeArticulation((ChordRest*) e);
		       }
		     else
		       writeRest(l, 1); 		   }
	       }
	     else
	       {
		 if (wholemeasurerest >=1)
		   writeMeasuRestNum();
		 writeRest(l, 0);		 if (articul) writeArticulation((ChordRest*) e);
	       }
	     tick += l;
	     measuretick=measuretick+l;
	  } 	  break;
	case Element::MARKER:
	  qDebug("ordinary elements: Marker found\n");
	  break;
	case Element::BREATH:
	  out << "\\breathe ";
	  break;
	default:
	  	  break;
	} 
       handleElement(e); 
      if (tupletcount==-1)
	{
	  out << " } ";
	  tupletcount=0;
	}
    } 
   barlen=m->ticks();
   if (barempty == true)
       {
      if ((pickup) and (measurenumber==1) and (voice == 0))
	{

    const SigEvent ev(m->score()->sigmap()->timesig(m->tick()));
    out << "\\partial " << ev.timesig().denominator() << "*" << ev.timesig().numerator() << "\n";
	  indent();
	  writeRest(barlen,2);
	  out << "\n";
	}      else 	{
	  writeRest(barlen, 2);
	  curTicks=-1;
	}
    }   else      {
              if ((measuretick < barlen) and (measurenumber>0))
	 {
	   	   int negative=barlen-measuretick;
	   curTicks=-1;
	   writeRest(negative, 2);
	   curTicks=-1;
	 }
     }
   int mno;
   if (!partial)
     mno = measurenumber +1;
   else
     mno = measurenumber;
   writeVolta(mno, lastind);
} 



void ExportLy::writeScore()
{
    firstalt=false;
  secondalt=false;
  tupletcount=0;
  char  cpartnum;
  chordpitch=41;
  repeatactive=false;
  staffInd = 0;
  graceswitch=false;
  int voice=0;
  cleannote="c";
  prevnote="c";
  gracecount=0;
  donefirst=false;
  lastJumpOrMarker = 0;
  initJumpOrMarkerLMs();
  wholemeasuretext = "";
  glisscount = 0;
  textspanswitch = false;
  textspannerdown=false;
  headOfLyrics = NULL;
  tailOfLyrics = NULL;
  privateRehearsalMark='A';


  foreach(Part* part, score->parts())
    {
      nextAnchor=0;
      initAnchors();
      resetAnchor(anker);

      int n = part->staves()->size();
      staffname[staffInd].partname  = part->longName().toPlainText();
      staffname[staffInd].partshort = part->shortName().toPlainText();
      curTicks=-1;
      pickup=false;

      if (part->nstaves()==2)
	pianostaff = true;
      else
	pianostaff = false;

      int strack = score->staffIdx(part) * VOICES;
      int etrack = strack + n* VOICES;

      buildInstructionListPart(strack, etrack);
      buildGlissandoList(strack,etrack);


                              
      foreach(Staff* staff, *part->staves())
	{

	  out << "\n";
    relativ="";
	  switch(staff->clef(0))
	    {
	    case ClefType::G:
	      relativ="'";
	      staffpitch=12*5;
	      break;
	    case ClefType::TAB:
	    case ClefType::PERC:
	    case ClefType::PERC2:
	    case ClefType::G3:
	    case ClefType::F:
	      relativ="";
	      staffpitch=12*4;
	      break;
	    case ClefType::G1:
	    case ClefType::G2:
	      relativ="''";
	      staffpitch=12*6;
	      break;
	    case ClefType::F_B:
	    case ClefType::F_C:
	    case ClefType::F8:
	      relativ=",";
	      staffpitch=12*3;
	      break;
	    case ClefType::F15:
	      relativ=",,";
	      staffpitch=12*2;
	      break;
	    case ClefType::C1:
	    case ClefType::C2:
	    case ClefType::C3:
	    case ClefType::C4:
	      relativ="'";
	      staffpitch=12*5;
	      break;
          default:                  break;
	    }

	  staffrelativ=relativ;

	  cpartnum = staffInd + 65;
	  staffname[staffInd].staffid = staffname[staffInd].partshort;
	  staffname[staffInd].staffid.append("part");
	  staffname[staffInd].staffid.append(cpartnum);
	  staffname[staffInd].staffid.prepend("A");
	  staffname[staffInd].staffid.remove(QRegExp("[0-9]"));
	  staffname[staffInd].staffid.remove(QChar('.'));
	  staffname[staffInd].staffid.remove(QChar(' '));

	  findVolta();
	  	  
	  for (voice = 0; voice < VOICES; ++voice)  voiceActive[voice] = false;

	  for (voice = 0; voice < VOICES; ++voice)
	    {
	      prevpitch=staffpitch;
	      relativ=staffrelativ;
	      donefirst=false;
	      partial=0;

	      	      for (MeasureBase* m = score->first(); m; m = m->next())
		{
		  if (m->type() != Element::MEASURE)
		    continue;

		  if (staffInd == 0)
		    findMarkerAtMeasureStart((Measure*) m );
		  		  
		  writeVoiceMeasure(m, staff, staffInd, voice); 
		  if (staffInd == 0)
		    jumpAtMeasureStop( (Measure*) m);
		  		  		}
	      level--;
	      indent();
	      out << "\\bar \"|.\" \n"; 	      level=0;
	      indent();
	      out << "}% end of last bar in partorvoice\n\n";
	      if (voiceActive[voice])
		{
		  scorout<< voicebuffer;
		}
	      voicebuffer = " \n";
	    } 
	  int voiceno=0;

	  for (voice = 0; voice < VOICES; ++voice)
	    if (voiceActive[voice]) voiceno++;

	  if (voiceno == 1)
	    staffname[staffInd].simultaneousvoices=false;

	  if (voiceno>1) 	    {
	      level=0;
	      indent();
	      out << staffname[staffInd].staffid << " =  << \n";
	      staffname[staffInd].simultaneousvoices=true;
	      level++;
	      indent();
	      out << "\\mergeDifferentlyHeadedOn\n";
	      indent();
              out << "\\mergeDifferentlyDottedOn \n";
	      ++level;

	      for (voice = 0; voice < voiceno; voice++)
		{
		  if (voiceActive[voice])
		    {
		      		      		      indent();
		      out << "\\context Voice = " << staffname[staffInd].voicename[voice] ;
		      if ((voice == 0) and (pianostaff ==false))
			out << "{\\voiceOne ";
		      out << "\\" << staffname[staffInd].voicename[voice];
		      if ((voice == 0) and (pianostaff == false))
			out << "}";
		      if (voice < voiceno-1) out << "\\\\ \n";
		      else out <<"\n";
		    }
		}

	      indent();
	      out << ">> \n\n";
	      level=0;
	      indent();
	      scorout<< voicebuffer;
	      voicebuffer = " \n";
	    }
	  staffname[staffInd].numberofvoices=voiceno;
	  ++staffInd;
	}
      staffname[staffInd].staffid="laststaff";
      if (n > 1)
	{
	  --level;
	  indent();
	}
    }}

void ExportLy::writeScoreBlock()
{
  thisLyrics = headOfLyrics;

  if (nochord==false)     {
      os << "theChords = \\chordmode { \n";
      printChordList();
      cleanupChordList();
      level--;
    }

  
  level=0;
  os << "\n\\score { \n";
  level++;
  indentF();
  os << "<< \n";

  indx=0;
  while (staffname[indx].staffid!="laststaff")
    {
      if (lybracks[indx].brakstart)
	{
	  ++level;
	  indentF();
	  os << "\\context StaffGroup = " << (char)(lybracks[indx].brakno + 64) << "<< \n";
	}

      if (lybracks[indx].bracestart)
	{
	  ++level;
	  indentF();
	  if (lybracks[indx].piano)
	    {
	      os << "\\context PianoStaff <<\n";
	      indentF();
	      os << "\\set PianoStaff.instrumentName=\"Piano\" \n";
	      pianostaff=true;
	    }
	  else
	    os << "\\context GrandStaff = " << (char)(lybracks[indx].braceno + 64) << "<< \n";
	}

      if ((nochord == false) && (indx==0)) 	{
	  indentF();
	  os << "\\new ChordNames { \\theChords } \n";
	}


      ++level;
      indentF();
      os << "\\context Staff = " << staffname[indx].staffid << " << \n";
      ++level;
      indentF();
      os << "\\";
      if (staffname[indx].simultaneousvoices)
	os << staffname[indx].staffid << "\n";
      else
	{
	  	  os << "context Voice = "  << staffname[indx].voicename[0] << " \\";
	  os << staffname[indx].voicename[0] << "\n"; 	}

      if (lybracks[indx].piano)
	{
	  indentF();
	  os << "\\set Staff.instrumentName = #\"\"\n";
	  indentF();
	  os << "\\set Staff.shortInstrumentName = #\"\"\n";
	}

      --level;
      indentF();
      os << ">>\n\n"; 
      connectLyricsToStaff();

      if (((lybracks[indx].brakstart) and (lybracks[indx].brakend)) or ((lybracks[indx].bracestart) and (lybracks[indx].braceend)))
	{
	  	  indentF();
	  os << "\\override StaffGroup.SystemStartBracket #'collapse-height = #1 \n";
	  indentF();
	  os << "\\override Score.SystemStartBar #'collapse-height = #1 \n";
	}

      if (lybracks[indx].brakend)
	{  --level;
	  indentF();
	  os << ">> %end of StaffGroup" << (char)(lybracks[indx].brakno + 64) << "\n\n";
	}
      if (lybracks[indx].braceend)
	{
	  --level;
	  indentF();
	  if (lybracks[indx].piano)
	    os << ">> %end of PianoStaff" << (char)(lybracks[indx].braceno + 64) << "\n";
	  else
	    os << ">> %end of GrandStaff" << (char)(lybracks[indx].braceno + 64) << "\n";
	}


      --level;
      ++indx;

    }
  cleanupLyrics();

  os << "\n";

  os << "\n"
  "      \\set Score.skipBars = ##t\n"
  "      %%\\set Score.melismaBusyProperties = #'()\n"
  "      \\override Score.BarNumber #'break-visibility = #end-of-line-invisible %%every bar is numbered.!!!\n"
  "      %% remove previous line to get barnumbers only at beginning of system.\n"
  "       #(set-accidental-style 'modern-cautionary)\n";
  if (rehearsalnumbers) os <<  "      \\set Score.markFormatter = #format-mark-box-numbers %%boxed rehearsal-numbers \n";
  else  os <<  "      \\set Score.markFormatter = #format-mark-box-letters %%boxed rehearsal-marks\n";
  if ((timedenom == 2) and (z1 == 2))
    {os << "%% "; }
  os << "       \\override Score.TimeSignature #'style = #'() %%makes timesigs always numerical\n"
  "      %% remove previous line to get cut-time/alla breve or common time \n";

os <<
  "      \\set Score.pedalSustainStyle = #'mixed \n"
  "       %% make spanners comprise the note it end on, so that there is no doubt that this note is included.\n"
  "       \\override Score.TrillSpanner #'(bound-details right padding) = #-2\n"
  "      \\override Score.TextSpanner #'(bound-details right padding) = #-1\n"
  "      %% Lilypond's normal textspanners are too weak:  \n"
  "      \\override Score.TextSpanner #'dash-period = #1\n"
  "      \\override Score.TextSpanner #'dash-fraction = #0.5\n"
  "      %% lilypond chordname font, like mscore jazzfont, is both far too big and extremely ugly (olagunde@start.no):\n"
  "      \\override Score.ChordName #'font-family = #'roman \n"
  "      \\override Score.ChordName #'font-size =#0 \n"
  "      %% In my experience the normal thing in printed scores is maj7 and not the triangle. (olagunde):\n"
  "      \\set Score.majorSevenSymbol = \\markup {maj7}\n"
  "  >>\n\n"
  "  %% Boosey and Hawkes, and Peters, have barlines spanning all staff-groups in a score,\n"
  "  %% Eulenburg and Philharmonia, like Lilypond, have no barlines between staffgroups.\n"
  "  %% If you want the Eulenburg/Lilypond style, comment out the following line:\n"
  "  \\layout {\\context {\\Score \\consists Span_bar_engraver}}\n"
  "}%% end of score-block \n\n";

  if (((pianostaff) and (indx==2)) or (indx < 2))
    os << "#(set-global-staff-size 20)\n";
  else if (indx > 2)
    os << "#(set-global-staff-size 14)\n";
}



void ExportLy::writeLilyMacros()
{
  if ((jumpswitch) || (ottvaswitch))
    {
      os<< " %%---------------MSCORE'S LILYPOND MACROS: -------------------------\n\n";
    }

  if (ottvaswitch)
    {
      os << " %%-----------------replacement for the \\ottava command--------------------\n\n";

                                                            
      os << "ottva =\n  "
	"{  %% for explanation, see mscore source file exportly.cpp \n"
	"   \\once\\override TextSpanner #'(bound-details left text) = \"8va\" \n"
	"   \\once\\override TextSpanner #'(bound-details right text) = \\markup{ \\draw-line #'(0 . -1) }\n"
	"   #(ly:export (make-event-chord (list (make-span-event 'TextSpanEvent START)))) \n"
	"}\n"
	"\n"

	"ottvaend ={ #(ly:export (make-event-chord (list (make-span-event 'TextSpanEvent STOP)))) \n"
	"   \\textSpannerNeutral} \n"

	"ottvabassa = \n"
	"{   \n"
	"   \\once \\override TextSpanner #'(bound-details left text) = \"8vb\"  \n"
	"   \\textSpannerDown \n"
        "   \\once \\override TextSpanner #'(bound-details right text) = \\markup{ \\draw-line #'(0 . 1) } \n"
	"   #(ly:export (make-event-chord (list (make-span-event 'TextSpanEvent START)))) \n"
	"}\n"
	"\n"

	"%%------------------end ottava macros ---------------------\n\n";
 }

  if (jumpswitch)
    {
      os << "   %%------------------coda---segno---macros--------------------\n"

	"   %%                 modified from lsr-snippets. Work in progress:       \n"

	"   %% These macros presupposes a difference between the use of the       \n"
	"   %% Coda-sign telling us to jump to the coda (\\gotocoda), and the   \n"
	"   %% Coda-sign telling us that this is actually the Coda (\\theCoda).  \n"
	"   %% This goes well if you use the mscore text: \"To Coda\" as a mark of \n"
	"   %% of where to jump from, and the codawheel as the mark of where to jump to\n"
	"   %% Otherwise (using codawheel for both) you have to edit the lilypond-file by hand.\n"

	"   gotocoda     = \\mark \\markup {\\musicglyph #\"scripts.coda\"}               \n"
	"   thecodasign  = \\mark \\markup {\\musicglyph #\"scripts.coda\" \"Coda\"}     \n"
	"   thesegno     = \\mark \\markup {\\musicglyph #\"scripts.segno\"}              \n"
	"   varcodasign  = \\mark \\markup {\\musicglyph #\"scripts.varcoda\"}            \n"
	"   Radjust      =  \\once \\override Score.RehearsalMark #'self-alignment-X = #RIGHT \n"
	"   blankClefKey = {\\once \\override Staff.KeySignature #'break-visibility = #all-invisible \n"
	"		    \\once \\override Staff.Clef #'break-visibility = #all-invisible   \n"
	"                 } \n"
	"   codetta     = {\\mark \\markup \\line {\\musicglyph #\"scripts.coda\" \\hspace #-1.3 \\musicglyph #\"scripts.coda\"} } \n"
	"   fine        = {\\Radjust \\mark \\markup {\"Fine\"} \\mark \\markup {\\musicglyph #\"scripts.ufermata\" } \n"
	"		  \\bar \"||\" } \n"
	"   DCalfine    = {\\Radjust \\mark \\markup {\"D.C. al fine\"} \\bar \"||\" \\blankClefKey \\stopStaff \\cadenzaOn } \n"
	"   DCalcoda    = {\\Radjust \\mark \\markup {\"D.C. al coda\"} \\bar \"||\" \\blankClefKey \\stopStaff \\cadenzaOn }  \n"
	"   DSalfine    = {\\Radjust \\mark \\markup {\"D.S. al fine\"} \\bar \"||\" \\blankClefKey \\stopStaff \\cadenzaOn } \n"
	"   DSalcoda    = {\\Radjust \\mark \\markup {\"D.S. al coda\"} \\bar \"||\" \\blankClefKey \\stopStaff \\cadenzaOn } \n"
	"   showClefKey = {\\once \\override Staff.KeySignature #'break-visibility = #all-visible \n"
	"               \\once \\override Staff.Clef #'break-visibility = #all-visible \n"
	"		 } \n"
	"   resumeStaff = {\\cadenzaOff \\startStaff % Resume bar count and show staff lines again \n"
	"		  \\partial 32 s32 % Add a whee bit of staff before the clef! \n"
	"		  \\bar \"\" \n"
	"		 } \n"
	"   %%   whitespace between D.S./D.C. and the Coda: \n"
	"   codaspace = {\\repeat unfold 2 {s4 s4 s4 s4 \\noBreak \\bar \"\" }}  \n"
	"   theCoda   = {\\noBreak \\codaspace \\resumeStaff \\showClefKey \\thecodasign} \n"

	" %% -------------------end-of-coda-segno-macros------------------  \n\n ";
    }

 if ((jumpswitch) || ottvaswitch)
   {
     os << "%% --------------END MSCORE LILYPOND-MACROS------------------------\n\n\n\n\n";
   }
} 


void ExportLy::writeLilyHeader()
{
  os << "%=============================================\n"
    "%   created by MuseScore Version: " << VERSION << "\n"
    "%          " << QDate::currentDate().toString(Qt::SystemLocaleLongDate);
  os << "\n";
  os <<"%=============================================\n"
    "\n"
    "\\version \"2.12.0\"\n\n";     
  os << "\n\n";
}

      void ExportLy::writePageFormat()
{
  const PageFormat* pf = score->pageFormat();
  os << "#(set-default-paper-size ";
      os << "\"" << QString(pf->paperSize()->name).toLower() << "\"";

  if (pf->size().width() > pf->size().height()) os << " 'landscape";

  os << ")\n\n";

    
  double lw = pf->printableWidth();
  os << "\\paper {\n";
  os <<  "  line-width    = " << lw * INCH << "\\mm\n";
  os <<  "  left-margin   = " << pf->evenLeftMargin() * INCH << "\\mm\n";
  os <<  "  top-margin    = " << pf->evenTopMargin() * INCH << "\\mm\n";
  os <<  "  bottom-margin = " << pf->evenBottomMargin() * INCH << "\\mm\n";
  os <<  "  %%indent = 0 \\mm \n";
  os <<  "  %%set to ##t if your score is less than one page: \n";
  os <<  "  ragged-last-bottom = ##t \n";
  os <<  "  ragged-bottom = ##f  \n";
  os <<  "  %% in orchestral scores you probably want the two bold slashes \n";
  os <<  "  %% separating the systems: so uncomment the following line: \n";
  os <<  "  %% system-separator-markup = \\slashSeparator \n";
  os <<  "  }\n\n";
}


void ExportLy::writeScoreTitles()
{
#if 0 
  os << "\\header {\n";

  ++level;
  const MeasureBase* m = score->first();
  foreach(const Element* e, *m->el()) {
    if (e->type() != TEXT)
      continue;
    QString s = ((Text*)e)->text();
    indentF();
    switch(e->subtype()) {
    case TEXT_TITLE:
      os << "title = ";
      break;
    case TEXT_SUBTITLE:
      os << "subtitle = ";
      break;
    case TEXT_COMPOSER:
      os << "composer = ";
      break;
    case TEXT_POET:
      os << "poet = ";
      break;
    default:
      qDebug("text-type %d not supported\n", e->subtype());
      os << "subtitle = ";
      break;
    }
    os << "\"" << s << "\"\n";
  }

  if (!score->metaTag("Copyright").isEmpty())
    {
      indentF();
      os << "copyright = \"" << score->metaTag("Copyright") << "\"\n";
    }

  indentF();
  os << "}\n";
#endif
}



bool ExportLy::write(const QString& name)
{
    pianostaff=false;
  rehearsalnumbers=false;
  wholemeasurerest=0;
  f.setFileName(name);
  if (!f.open(QIODevice::WriteOnly))
    return false;
  os.setDevice(&f);
  os.setCodec("utf8");
  out.setCodec("utf8");
  out.setString(&voicebuffer);
  voicebuffer = "";
  scorout.setCodec("utf8");
  scorout.setString(&scorebuffer);
  scorebuffer = "";
  chordHead=NULL;
  chordcount = 0;
  slurstack=0;
  phraseslur=0;
  ottvaswitch = false;
  jumpswitch = false;
  nochord = true;

  writeLilyHeader();

  writeScore();

  findLyrics();

  writeLilyMacros();

  writePageFormat();

  writeScoreTitles();

  findBrackets();

  os << scorebuffer;
  scorebuffer = "";

  writeLyrics();

  writeScoreBlock();

  f.close();
  return f.error() == QFile::NoError;
}










}

