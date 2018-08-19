
#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__



#include "globals.h"

namespace Ms {

extern QString mscoreGlobalShare;

enum class SessionStart : char {
      EMPTY, LAST, NEW, SCORE
      };

enum {
      RMIDI_REWIND,
      RMIDI_TOGGLE_PLAY,
      RMIDI_PLAY,
      RMIDI_STOP,
      RMIDI_NOTE1,
      RMIDI_NOTE2,
      RMIDI_NOTE4,
      RMIDI_NOTE8,
      RMIDI_NOTE16,
      RMIDI_NOTE32,
      RMIDI_NOTE64,
      RMIDI_REST,
      RMIDI_DOT,
      RMIDI_DOTDOT,
      RMIDI_TIE,
      RMIDI_UNDO,
      RMIDI_NOTE_EDIT_MODE,
      RMIDI_REALTIME_ADVANCE,
      MIDI_REMOTES
      };

enum class MuseScoreStyleType : char {
      DARK_FUSION = 0,
      LIGHT_FUSION
      };

enum class MusicxmlExportBreaks : char {
      ALL, MANUAL, NO
      };

#define PREF_APP_AUTOSAVE_AUTOSAVETIME                      "application/autosave/autosaveTime"
#define PREF_APP_AUTOSAVE_USEAUTOSAVE                       "application/autosave/useAutosave"
#define PREF_APP_PATHS_INSTRUMENTLIST1                      "application/paths/instrumentList1"
#define PREF_APP_PATHS_INSTRUMENTLIST2                      "application/paths/instrumentList2"
#define PREF_APP_PATHS_MYIMAGES                             "application/paths/myImages"
#define PREF_APP_PATHS_MYPLUGINS                            "application/paths/myPlugins"
#define PREF_APP_PATHS_MYSCORES                             "application/paths/myScores"
#define PREF_APP_PATHS_MYSHORTCUTS                          "application/paths/myShortcuts"
#define PREF_APP_PATHS_MYSOUNDFONTS                         "application/paths/mySoundfonts"
#define PREF_APP_PATHS_MYSTYLES                             "application/paths/myStyles"
#define PREF_APP_PATHS_MYTEMPLATES                          "application/paths/myTemplates"
#define PREF_APP_PLAYBACK_FOLLOWSONG                        "application/playback/followSong"
#define PREF_APP_PLAYBACK_PANPLAYBACK                       "application/playback/panPlayback"
#define PREF_APP_PLAYBACK_PLAYREPEATS                       "application/playback/playRepeats"
#define PREF_APP_USESINGLEPALETTE                           "application/useSinglePalette"
#define PREF_APP_STARTUP_SESSIONSTART                       "application/startup/sessionStart"
#define PREF_APP_STARTUP_STARTSCORE                         "application/startup/startScore"
#define PREF_APP_WORKSPACE                                  "application/workspace"
#define PREF_EXPORT_AUDIO_SAMPLERATE                        "export/audio/sampleRate"
#define PREF_EXPORT_MP3_BITRATE                             "export/mp3/bitRate"
#define PREF_EXPORT_MUSICXML_EXPORTLAYOUT                   "export/musicXML/exportLayout"
#define PREF_EXPORT_MUSICXML_EXPORTBREAKS                   "export/musicXML/exportBreaks"
#define PREF_EXPORT_PDF_DPI                                 "export/pdf/dpi"
#define PREF_EXPORT_PNG_RESOLUTION                          "export/png/resolution"
#define PREF_EXPORT_PNG_USETRANSPARENCY                     "export/png/useTransparency"
#define PREF_IMPORT_GUITARPRO_CHARSET                       "import/guitarpro/charset"
#define PREF_IMPORT_MUSICXML_IMPORTBREAKS                   "import/musicXML/importBreaks"
#define PREF_IMPORT_MUSICXML_IMPORTLAYOUT                   "import/musicXML/importLayout"
#define PREF_IMPORT_OVERTURE_CHARSET                        "import/overture/charset"
#define PREF_IMPORT_STYLE_STYLEFILE                         "import/style/styleFile"
#define PREF_IO_ALSA_DEVICE                                 "io/alsa/device"
#define PREF_IO_ALSA_FRAGMENTS                              "io/alsa/fragments"
#define PREF_IO_ALSA_PERIODSIZE                             "io/alsa/periodSize"
#define PREF_IO_ALSA_SAMPLERATE                             "io/alsa/sampleRate"
#define PREF_IO_ALSA_USEALSAAUDIO                           "io/alsa/useAlsaAudio"
#define PREF_IO_JACK_REMEMBERLASTCONNECTIONS                "io/jack/rememberLastConnections"
#define PREF_IO_JACK_TIMEBASEMASTER                         "io/jack/timebaseMaster"
#define PREF_IO_JACK_USEJACKAUDIO                           "io/jack/useJackAudio"
#define PREF_IO_JACK_USEJACKMIDI                            "io/jack/useJackMIDI"
#define PREF_IO_JACK_USEJACKTRANSPORT                       "io/jack/useJackTransport"
#define PREF_IO_MIDI_ADVANCEONRELEASE                       "io/midi/advanceOnRelease"
#define PREF_IO_MIDI_ENABLEINPUT                            "io/midi/enableInput"
#define PREF_IO_MIDI_EXPANDREPEATS                          "io/midi/expandRepeats"
#define PREF_IO_MIDI_EXPORTRPNS                             "io/midi/exportRPNs"
#define PREF_IO_MIDI_REALTIMEDELAY                          "io/midi/realtimeDelay"
#define PREF_IO_MIDI_REMOTE                                 "io/midi/remote"
#define PREF_IO_MIDI_SHORTESTNOTE                           "io/midi/shortestNote"
#define PREF_IO_MIDI_SHOWCONTROLSINMIXER                    "io/midi/showControlsInMixer"
#define PREF_IO_MIDI_USEREMOTECONTROL                       "io/midi/useRemoteControl"
#define PREF_IO_OSC_PORTNUMBER                              "io/osc/portNumber"
#define PREF_IO_OSC_USEREMOTECONTROL                        "io/osc/useRemoteControl"
#define PREF_IO_PORTAUDIO_DEVICE                            "io/portAudio/device"
#define PREF_IO_PORTAUDIO_USEPORTAUDIO                      "io/portAudio/usePortAudio"
#define PREF_IO_PORTMIDI_INPUTBUFFERCOUNT                   "io/portMidi/inputBufferCount"
#define PREF_IO_PORTMIDI_INPUTDEVICE                        "io/portMidi/inputDevice"
#define PREF_IO_PORTMIDI_OUTPUTBUFFERCOUNT                  "io/portMidi/outputBufferCount"
#define PREF_IO_PORTMIDI_OUTPUTDEVICE                       "io/portMidi/outputDevice"
#define PREF_IO_PORTMIDI_OUTPUTLATENCYMILLISECONDS          "io/portMidi/outputLatencyMilliseconds"
#define PREF_IO_PULSEAUDIO_USEPULSEAUDIO                    "io/pulseAudio/usePulseAudio"
#define PREF_SCORE_CHORD_PLAYONADDNOTE                      "score/chord/playOnAddNote"
#define PREF_SCORE_MAGNIFICATION                            "score/magnification"
#define PREF_SCORE_NOTE_PLAYONCLICK                         "score/note/playOnClick"
#define PREF_SCORE_NOTE_DEFAULTPLAYDURATION                 "score/note/defaultPlayDuration"
#define PREF_SCORE_NOTE_WARNPITCHRANGE                      "score/note/warnPitchRange"
#define PREF_SCORE_STYLE_DEFAULTSTYLEFILE                   "score/style/defaultStyleFile"
#define PREF_SCORE_STYLE_PARTSTYLEFILE                      "score/style/partStyleFile"
#define PREF_UI_CANVAS_BG_USECOLOR                          "ui/canvas/background/useColor"
#define PREF_UI_CANVAS_FG_USECOLOR                          "ui/canvas/foreground/useColor"
#define PREF_UI_CANVAS_BG_COLOR                             "ui/canvas/background/color"
#define PREF_UI_CANVAS_FG_COLOR                             "ui/canvas/foreground/color"
#define PREF_UI_CANVAS_BG_WALLPAPER                         "ui/canvas/background/wallpaper"
#define PREF_UI_CANVAS_FG_WALLPAPER                         "ui/canvas/foreground/wallpaper"
#define PREF_UI_CANVAS_MISC_ANTIALIASEDDRAWING              "ui/canvas/misc/antialiasedDrawing"
#define PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY              "ui/canvas/misc/selectionProximity"
#define PREF_UI_CANVAS_SCROLL_VERTICALORIENTATION           "ui/canvas/scroll/verticalOrientation"
#define PREF_UI_CANVAS_SCROLL_LIMITSCROLLAREA               "ui/canvas/scroll/limitScrollArea"
#define PREF_UI_APP_STARTUP_CHECKUPDATE                     "ui/application/startup/checkUpdate"
#define PREF_UI_APP_STARTUP_SHOWNAVIGATOR                   "ui/application/startup/showNavigator"
#define PREF_UI_APP_STARTUP_SHOWPLAYPANEL                   "ui/application/startup/showPlayPanel"
#define PREF_UI_APP_STARTUP_SHOWSPLASHSCREEN                "ui/application/startup/showSplashScreen"
#define PREF_UI_APP_STARTUP_SHOWSTARTCENTER                 "ui/application/startup/showStartCenter"
#define PREF_UI_APP_GLOBALSTYLE                             "ui/application/globalStyle"
#define PREF_UI_APP_LANGUAGE                                "ui/application/language"
#define PREF_UI_APP_RASTER_HORIZONTAL                       "ui/application/raster/horizontal"
#define PREF_UI_APP_RASTER_VERTICAL                         "ui/application/raster/vertical"
#define PREF_UI_APP_SHOWSTATUSBAR                           "ui/application/showStatusBar"
#define PREF_UI_APP_USENATIVEDIALOGS                        "ui/application/useNativeDialogs"
#define PREF_UI_PIANO_HIGHLIGHTCOLOR                        "ui/piano/highlightColor"
#define PREF_UI_SCORE_NOTE_DROPCOLOR                        "ui/score/note/dropColor"
#define PREF_UI_SCORE_DEFAULTCOLOR                          "ui/score/defaultColor"
#define PREF_UI_SCORE_FRAMEMARGINCOLOR                      "ui/score/frameMarginColor"
#define PREF_UI_SCORE_LAYOUTBREAKCOLOR                      "ui/score/layoutBreakColor"
#define PREF_UI_SCORE_VOICE1_COLOR                          "ui/score/voice1/color"
#define PREF_UI_SCORE_VOICE2_COLOR                          "ui/score/voice2/color"
#define PREF_UI_SCORE_VOICE3_COLOR                          "ui/score/voice3/color"
#define PREF_UI_SCORE_VOICE4_COLOR                          "ui/score/voice4/color"
#define PREF_UI_THEME_ICONHEIGHT                            "ui/theme/iconHeight"
#define PREF_UI_THEME_ICONWIDTH                             "ui/theme/iconWidth"


class Preference {
   private:
      QVariant _defaultValue = 0;

   public:
      Preference(QVariant defaultValue) : _defaultValue(defaultValue) {}

      QVariant defaultValue() const {return _defaultValue;}
      };


class Preferences {

                        std::unordered_map<std::string, Preference> _allPreferences;
                  std::unordered_map<std::string, QVariant> _inMemorySettings;
      bool _storeInMemoryOnly = false;
      bool _returnDefaultValues = false;
      bool _initialized = false;
      QSettings* _settings; 
      QSettings* settings() const;
                  QVariant get(const QString key) const;
      bool has(const QString key) const;
      void set(const QString key, QVariant value, bool temporary = false);
      void remove(const QString key);

      QVariant preference(const QString key) const;
      void checkIfKeyExists(const QString key) const;

   public:
      Preferences();
      ~Preferences();
      void init(bool storeInMemoryOnly = false);
      void save();

            QVariant defaultValue(const QString key) const;
      bool getBool(const QString key) const;
      QColor getColor(const QString key) const;
      QString getString(const QString key) const;
      int getInt(const QString key) const;
      double getDouble(const QString key) const;

            void revertToDefaultValue(const QString key);
            void setReturnDefaultValues(bool returnDefaultValues);
      void setPreference(const QString key, QVariant value);
                        void setTemporaryPreference(const QString key, QVariant value);

      
      SessionStart sessionStart() const;
      MusicxmlExportBreaks musicxmlExportBreaks() const;
      MuseScoreStyleType globalStyle() const;
      bool isThemeDark() const;

      template<typename T>
      void setCustomPreference(const QString key, T t)
            {
            set(key, QVariant::fromValue<T>(t));
            }

            MidiRemote midiRemote(int recordId) const;
      void updateMidiRemote(int recordId, MidiRemoteType type, int data);
      void clearMidiRemote(int recordId);
      };

extern Preferences preferences;

template<typename T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
inline QDataStream &operator<<(QDataStream &out, const T &val)
{
    return out << static_cast<int>(val);
}

template<typename T, typename std::enable_if<std::is_enum<T>::value>::type* = nullptr>
inline QDataStream &operator>>(QDataStream &in, T &val)
{
    int tmp;
    in >> tmp;
    val = static_cast<T>(tmp);
    return in;
}


} 
Q_DECLARE_METATYPE(Ms::SessionStart);
Q_DECLARE_METATYPE(Ms::MusicxmlExportBreaks);
Q_DECLARE_METATYPE(Ms::MuseScoreStyleType);

#endif
