
#include "preferences.h"
#include "libmscore/score.h"
#include "musescore.h"
#include "seq.h"
#include "pa.h"

#ifdef USE_ALSA
#include "alsa.h"
#include "alsamidi.h"
#endif

#include <portaudio.h>
#include "mididriver.h"

#ifdef USE_PORTMIDI
#include "pm.h"
#endif

namespace Ms {

static PaStream* stream;


int paCallback(const void*, void* out, long unsigned frames,
   const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void *)
      {
      seq->setInitialMillisecondTimestampWithLatency();
      seq->process((unsigned)frames, (float*)out);
      return 0;
      }


Portaudio::Portaudio(Seq* s)
   : Driver(s)
      {
      _sampleRate = 48000;          initialized = false;
      state       = Transport::STOP;
      seekflag    = false;
      midiDriver  = 0;
      }


Portaudio::~Portaudio()
      {
      if (initialized) {
            PaError err = Pa_CloseStream(stream);
            if (err != paNoError)
                  qDebug("Portaudio close stream failed: %s", Pa_GetErrorText(err));
            Pa_Terminate();
            }
      }


bool Portaudio::init(bool)
      {
      PaError err = Pa_Initialize();
      if (err != paNoError) {
            qDebug("Portaudio initialize failed: %s", Pa_GetErrorText(err));
            return false;
            }
      initialized = true;
      if (MScore::debugMode)
            qDebug("using PortAudio Version: %s", Pa_GetVersionText());

      PaDeviceIndex idx = preferences.getInt(PREF_IO_PORTAUDIO_DEVICE);
      if (idx < 0) {
            idx = Pa_GetDefaultOutputDevice();
            qDebug("No device selected.  PortAudio detected %d devices.  Will use the default device (index %d).", Pa_GetDeviceCount(), idx);
            }

      const PaDeviceInfo* di = Pa_GetDeviceInfo(idx);

            if (di == nullptr || di->maxOutputChannels < 1)
            di = Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());

      if (!di)
            return false;          _sampleRate = int(di->defaultSampleRate);

      
      struct PaStreamParameters out;
      memset(&out, 0, sizeof(out));

      out.device           = idx;
      out.channelCount     = 2;
      out.sampleFormat     = paFloat32;
      out.suggestedLatency = di->defaultLowOutputLatency;
      out.hostApiSpecificStreamInfo = 0;

      err = Pa_OpenStream(&stream, 0, &out, double(_sampleRate), 0, 0, paCallback, (void*)this);
      if (err != paNoError) {
                        out.device = Pa_GetDefaultOutputDevice();
            err = Pa_OpenStream(&stream, 0, &out, double(_sampleRate), 0, 0, paCallback, (void*)this);
            if (err != paNoError) {
                  qDebug("Portaudio open stream %d failed: %s", idx, Pa_GetErrorText(err));
                  return false;
                  }
            }
      const PaStreamInfo* si = Pa_GetStreamInfo(stream);
      if (si)
            _sampleRate = int(si->sampleRate);
#ifdef USE_ALSA
      midiDriver = new AlsaMidiDriver(seq);
#endif
#ifdef USE_PORTMIDI
      midiDriver = new PortMidiDriver(seq);
#endif
      if (midiDriver && !midiDriver->init()) {
            qDebug("Init midi driver failed");
            delete midiDriver;
            midiDriver = 0;
#ifdef USE_PORTMIDI
            return true;                  #else
            return false;
#endif
            }
      return true;
      }


QStringList Portaudio::apiList() const
      {
      QStringList al;

      PaHostApiIndex apis = Pa_GetHostApiCount();
      for (PaHostApiIndex i = 0; i < apis; ++i) {
            const PaHostApiInfo* info = Pa_GetHostApiInfo(i);
            if (info)
                  al.append(QString::fromLocal8Bit(info->name));
            }
      return al;
      }


QStringList Portaudio::deviceList(int apiIdx)
      {
      QStringList dl;
      const PaHostApiInfo* info = Pa_GetHostApiInfo(apiIdx);
      if (info) {
            for (int i = 0; i < info->deviceCount; ++i) {
                  PaDeviceIndex idx = Pa_HostApiDeviceIndexToDeviceIndex(apiIdx, i);
                  const PaDeviceInfo* di = Pa_GetDeviceInfo(idx);
                  if (di)
                        dl.append(QString::fromLocal8Bit(di->name));
                  }
            }
      return dl;
      }


int Portaudio::deviceIndex(int apiIdx, int apiDevIdx)
      {
      return Pa_HostApiDeviceIndexToDeviceIndex(apiIdx, apiDevIdx);
      }


bool Portaudio::start(bool)
      {
      PaError err = Pa_StartStream(stream);
      if (err != paNoError) {
            qDebug("Portaudio: start stream failed: %s", Pa_GetErrorText(err));
            return false;
            }
      return true;
      }


bool Portaudio::stop()
      {
      PaError err = Pa_StopStream(stream);            if (err != paNoError) {
            qDebug("Portaudio: stop failed: %s", Pa_GetErrorText(err));
            return false;
            }
      return true;
      }


int Portaudio::framePos() const
      {
      return 0;
      }


void Portaudio::startTransport()
      {
      state = Transport::PLAY;
      }


void Portaudio::stopTransport()
      {
      state = Transport::STOP;
      }


Transport Portaudio::getState()
      {
      return state;
      }


void Portaudio::midiRead()
      {
      if (midiDriver)
            midiDriver->read();
      }


#ifdef USE_PORTMIDI

#define less128(__less) ((__less >=0 && __less <= 127) ? __less : 0)

void Portaudio::putEvent(const NPlayEvent& e, unsigned framePos)
      {
      PortMidiDriver* portMidiDriver = static_cast<PortMidiDriver*>(midiDriver);
      if (!portMidiDriver || !portMidiDriver->getOutputStream() || !portMidiDriver->canOutput())
            return;

      int portIdx = seq->score()->midiPort(e.channel());
      int chan    = seq->score()->midiChannel(e.channel());

      if (portIdx < 0 ) {
            qDebug("Portaudio::putEvent: invalid port %d", portIdx);
            return;
            }

      if (midiOutputTrace) {
            int a     = e.dataA();
            int b     = e.dataB();
            qDebug("MidiOut<%d>: Portaudio: %02x %02x %02x, chan: %i", portIdx, e.type(), a, b, chan);
            }

      switch(e.type()) {
            case ME_NOTEON:
            case ME_NOTEOFF:
            case ME_POLYAFTER:
            case ME_CONTROLLER:
                                    if (e.dataA() == CTRL_PROGRAM) {
                                                long msg = Pm_Message(ME_PROGRAM | chan, less128(e.dataB()), 0);
                        PmError error = Pm_WriteShort(portMidiDriver->getOutputStream(), seq->getCurrentMillisecondTimestampWithLatency(framePos), msg);
                        if (error != pmNoError) {
                              qDebug("Portaudio: error %d", error);
                              return;
                              }
                        break;
                        }
                              case ME_PITCHBEND:
                  {
                  long msg = Pm_Message(e.type() | chan, less128(e.dataA()), less128(e.dataB()));
                  PmError error = Pm_WriteShort(portMidiDriver->getOutputStream(), seq->getCurrentMillisecondTimestampWithLatency(framePos), msg);
                  if (error != pmNoError) {
                        qDebug("Portaudio: error %d", error);
                        return;
                        }
                  }
                  break;

            case ME_PROGRAM:
            case ME_AFTERTOUCH:
                  {
                  long msg = Pm_Message(e.type() | chan, less128(e.dataA()), 0);
                  PmError error = Pm_WriteShort(portMidiDriver->getOutputStream(), seq->getCurrentMillisecondTimestampWithLatency(framePos), msg);
                  if (error != pmNoError) {
                        qDebug("Portaudio: error %d", error);
                        return;
                        }
                  }
                  break;
            case ME_SONGPOS:
            case ME_CLOCK:
            case ME_START:
            case ME_CONTINUE:
            case ME_STOP:
                  qDebug("Portaudio: event type %x not supported", e.type());
                  break;
            }
      }
#endif


int Portaudio::currentApi() const
      {
      PaDeviceIndex idx = preferences.getInt(PREF_IO_PORTAUDIO_DEVICE);
      if (idx < 0)
            idx = Pa_GetDefaultOutputDevice();

      for (int api = 0; api < Pa_GetHostApiCount(); ++api) {
            const PaHostApiInfo* info = Pa_GetHostApiInfo(api);
            if (info) {
                  for (int k = 0; k < info->deviceCount; ++k) {
                        PaDeviceIndex i = Pa_HostApiDeviceIndexToDeviceIndex(api, k);
                        if (i == idx)
                              return api;
                        }
                  }
            }
      qDebug("Portaudio: no current api found for device %d", idx);
      return -1;
      }


int Portaudio::currentDevice() const
      {
      PaDeviceIndex idx = preferences.getInt(PREF_IO_PORTAUDIO_DEVICE);
      if (idx < 0)
            idx = Pa_GetDefaultOutputDevice();

      for (int api = 0; api < Pa_GetHostApiCount(); ++api) {
            const PaHostApiInfo* info = Pa_GetHostApiInfo(api);
            if (info) {
                  for (int k = 0; k < info->deviceCount; ++k) {
                        PaDeviceIndex i = Pa_HostApiDeviceIndexToDeviceIndex(api, k);
                        if (i == idx)
                              return k;
                        }
                  }
            }
      qDebug("Portaudio: no current ApiDevice found for device %d", idx);
      return -1;
      }
}

