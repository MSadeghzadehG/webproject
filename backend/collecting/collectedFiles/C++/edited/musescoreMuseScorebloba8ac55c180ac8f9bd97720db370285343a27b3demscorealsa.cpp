
#include "config.h"

#ifdef USE_ALSA
#include <sys/time.h>
#include "alsa.h"
#include "libmscore/score.h"
#include "musescore.h"
#include "preferences.h"
#include "seq.h"
#include "alsamidi.h"
#include "libmscore/utils.h"

namespace Ms {


AlsaDriver::AlsaDriver(QString s, unsigned rate,
   snd_pcm_uframes_t frsize, unsigned nfrags)
      {
      _name        = s;
      _play_handle = 0;
      _play_hwpar  = 0;
      _play_swpar  = 0;
      _play_npfd   = 0;
      _rate        = rate;
      _frsize      = frsize;
      _nfrags      = nfrags;
      _stat        = -1;
      _play_nchan  = 2;
      }


bool AlsaDriver::init()
      {
      if (snd_pcm_open(&_play_handle, _name.toLatin1().data(), SND_PCM_STREAM_PLAYBACK, 0) < 0) {
            _play_handle = 0;
            qDebug ("Alsa_driver: Cannot open PCM device %s for playback.",
               _name.toLatin1().data());
            return false;
            }

      
      if (snd_pcm_hw_params_malloc (&_play_hwpar) < 0) {
            qDebug ("Alsa_driver: can't allocate playback hw params");
            return false;
            }

      if (snd_pcm_sw_params_malloc (&_play_swpar) < 0) {
            qDebug ("Alsa_driver: can't allocate playback sw params");
            return false;
            }
      if (!setHwpar(_play_handle, _play_hwpar))
            return false;
      if (!setSwpar(_play_handle, _play_swpar))
            return false;
      int dir;
      unsigned rate = _rate;
      if (snd_pcm_hw_params_get_rate (_play_hwpar, &_rate, &dir) || (rate != _rate) || dir) {
            qDebug ("Alsa_driver: can't get requested sample rate for playback.");
            return false;
            }

      snd_pcm_hw_params_get_format (_play_hwpar, &_play_format);
      snd_pcm_hw_params_get_access (_play_hwpar, &_play_access);

      switch (_play_format) {
            case SND_PCM_FORMAT_S32_LE:
                  _play_func  = play_32le;
                  _clear_func = clear_32le;
                  break;
            case SND_PCM_FORMAT_S24_3LE:
                  _play_func  = play_24le;
                  _clear_func = clear_24le;
                  break;
            case SND_PCM_FORMAT_S16_LE:
                  _play_func  = play_16le;
                  _clear_func = clear_16le;
                  break;
            default:
                  qDebug ("Alsa_driver: can't handle playback sample format.");
                  return false;
            }
      _play_npfd = snd_pcm_poll_descriptors_count (_play_handle);
      if (_play_npfd > MAXPFD) {
            qDebug ("Alsa_driver: interface requires more than %d pollfd", MAXPFD);
            return false;
            }
      _stat = 0;
      return true;
      }


AlsaDriver::~AlsaDriver()
      {
      snd_pcm_sw_params_free (_play_swpar);
      snd_pcm_hw_params_free (_play_hwpar);

      if (_play_handle)
            snd_pcm_close(_play_handle);
      }


bool AlsaDriver::pcmStart()
      {
      snd_pcm_sframes_t n = snd_pcm_avail_update(_play_handle);
      if (unsigned(n) != _frsize * _nfrags) {
            qDebug("Alsa_driver: %ld != %ld full buffer not available at start.", n, _frsize * _nfrags);
            return false;
            }
      if (mmappedInterface) {
            for (unsigned i = 0; i < _nfrags; i++) {
                  playInit (_frsize);
                  for (unsigned j = 0; j < _play_nchan; j++)
                        clearChan(j, _frsize);
                  snd_pcm_mmap_commit(_play_handle, _play_offs, _frsize);
                  }
            }
      int err = snd_pcm_start(_play_handle);
      if (err < 0) {
            qDebug ("Alsa_driver: pcmStart: pcm_start: %s.", snd_strerror (err));
            return false;
            }
      return true;
      }


int AlsaDriver::pcmStop()
      {
      int err;
      if (_play_handle && ((err = snd_pcm_drop (_play_handle)) < 0)) {
            qDebug ("Alsa_driver: pcm_drop(play): %s", snd_strerror (err));
            return -1;
            }
      return 0;
      }


snd_pcm_sframes_t AlsaDriver::pcmWait()
      {
      _stat = 0;
      _xrun = false;
      bool need_play = true;

      while (need_play) {
            if (need_play)
                  snd_pcm_poll_descriptors(_play_handle, _pfd, _play_npfd);

            errno = 0;
                        if (poll(_pfd, _play_npfd, -1) < 0) {
                  if (errno == EINTR) {
                        _stat = 1;
                        return 0;
                        }
                  qDebug ("Alsa_driver: poll(): %s.", strerror (errno));
                  _stat = 2;
                  return 0;
                  }
            int play_to = 0;
            if (need_play) {
                  for (int i = 0; i < _play_npfd; i++) {
                        if (_pfd[i].revents & POLLERR) {
                              _xrun = true;
                              _stat |= 4;
                              }
                        if (_pfd[i].revents == 0)
                              play_to++;
                        }
                  if (!play_to)
                        need_play = false;
                  }

            if ((play_to && (play_to == _play_npfd))) {
                  qDebug ("Alsa_driver: poll timed out.");
                  _stat |= 16;
                  return 0;
                  }
            }
      snd_pcm_sframes_t play_av = snd_pcm_avail_update(_play_handle);
      if (play_av < 0) {
            _xrun = true;
            _stat |= 64;
            }
      if (_xrun) {
            recover();
            return 0;
            }
      return play_av;
      }


int AlsaDriver::playInit(snd_pcm_uframes_t len)
      {
      int err;
      const snd_pcm_channel_area_t* a;

      if ((err = snd_pcm_mmap_begin (_play_handle, &a, &_play_offs, &len)) < 0) {
            qDebug ("Alsa_driver: snd_pcm_mmap_begin(play): %s.", snd_strerror (err));
            return -1;
            }
      _play_step = (a->step) >> 3;
      for (unsigned i = 0; i < _play_nchan; i++, a++) {
            _play_ptr[i] = (char *)a->addr + ((a->first + a->step * _play_offs) >> 3);
            }
      return len;
      }


void AlsaDriver::printinfo()
      {
      qDebug("\n  nchan  : %d", _play_nchan);
      qDebug("  rate   : %d", _rate);
      qDebug("  frsize : %ld", _frsize);
      qDebug("  nfrags : %d", _nfrags);
      qDebug("  format : %s", snd_pcm_format_name (_play_format));
      }


bool AlsaDriver::setHwpar(snd_pcm_t* handle, snd_pcm_hw_params_t* hwpar)
      {
      int err;

      if ((err = snd_pcm_hw_params_any(handle, hwpar)) < 0) {
            qDebug("Alsa_driver: no hw configurations available: %s.", snd_strerror (err));
            return false;
            }

      if ((err = snd_pcm_hw_params_set_periods_integer (handle, hwpar)) < 0) {
            qDebug("Alsa_driver: can't set period size to integral value.");
            return false;
            }

      mmappedInterface = true;
      if (((err = snd_pcm_hw_params_set_access (handle, hwpar, SND_PCM_ACCESS_MMAP_NONINTERLEAVED)) < 0)
         && ((err = snd_pcm_hw_params_set_access (handle, hwpar, SND_PCM_ACCESS_MMAP_INTERLEAVED)) < 0)) {
            mmappedInterface = false;
            if (MScore::debugMode)
                  qDebug("Alsa_driver: the interface doesn't support mmap-based access.");
            if (((err = snd_pcm_hw_params_set_access (handle, hwpar, SND_PCM_ACCESS_RW_NONINTERLEAVED)) < 0)
               && ((err = snd_pcm_hw_params_set_access (handle, hwpar, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)) {
                  qDebug("Alsa_driver: the interface doesn't support rw-based access.");
                  return false;
                  }
            }

      if (((err = snd_pcm_hw_params_set_format(handle, hwpar, SND_PCM_FORMAT_S16)) < 0)
         && ((err = snd_pcm_hw_params_set_format(handle, hwpar, SND_PCM_FORMAT_S24_3LE)) < 0)
         && ((err = snd_pcm_hw_params_set_format(handle, hwpar, SND_PCM_FORMAT_S32)) < 0)) {
            qDebug("Alsa_driver: the interface doesn't support 32, 24 or 16 bit access.");
            return false;
            }

      if ((err = snd_pcm_hw_params_set_rate(handle, hwpar, _rate, 0)) < 0) {
            qDebug("Alsa_driver: can't set sample rate to %u.", _rate);
            return false;
            }

      if ((err = snd_pcm_hw_params_set_channels(handle, hwpar, _play_nchan)) < 0) {
            qDebug("Alsa_driver: can't set channel count to %u.",
               _play_nchan);
            return false;
            }

      int dir = 0;
            if ((err = snd_pcm_hw_params_set_periods(handle, hwpar, _nfrags, 0)) < 0) {
            qDebug("Alsa_driver: can't set periods to %u.", _nfrags);
            return false;
            }

      dir = 0;
      if ((err = snd_pcm_hw_params_set_period_size_near(handle, hwpar, &_frsize, &dir)) < 0) {
            qDebug("Alsa_driver: can't set period size to %lu: %s",
               _frsize, snd_strerror(err));
            return false;
            }

      snd_pcm_uframes_t  n = _frsize * _nfrags;
      if ((err = snd_pcm_hw_params_set_buffer_size_near (handle, hwpar, &n)) < 0) {
            qDebug("Alsa_driver: can't set buffer length to %lu.", _frsize * _nfrags);
            return false;
            }
      if (n != _frsize * _nfrags) {
            qDebug("Alsa_driver: buffer size requested %lu got %lu, _frsize %lu _nfrags %d",
               _frsize * _nfrags, n, _frsize, _nfrags);
            if (n < _frsize * _nfrags)
                  return false;
            }

      if ((err = snd_pcm_hw_params (handle, hwpar)) < 0) {
            qDebug("Alsa_driver: can't set hardware parameters.");
            return false;
            }
      return true;
      }


bool AlsaDriver::setSwpar(snd_pcm_t *handle, snd_pcm_sw_params_t *swpar)
      {
      int err;

      snd_pcm_sw_params_current(handle, swpar);
      if ((err = snd_pcm_sw_params_set_silence_size(handle, swpar, 0)) < 0) {
            qDebug("AlsaDriver: can't set timestamp mode to %u.",
               SND_PCM_TSTAMP_MMAP);
            return false;
            }

      if ((err = snd_pcm_sw_params_set_avail_min(handle, swpar, _frsize)) < 0) {
            qDebug("AlsaDriver: can't set availmin to %lu.", _frsize);
            return false;
            }
      if ((err = snd_pcm_sw_params(handle, swpar)) < 0) {
            qDebug ("Alsa_driver: can't set software parameters.");
            return false;
            }
      return true;
      }


bool AlsaDriver::recover()
      {
      int err;
      snd_pcm_status_t* stat;

      snd_pcm_status_alloca (&stat);

      if ((err = snd_pcm_status (_play_handle, stat)) < 0) {
            qDebug("Alsa_driver: recover: pcm_status(): %s",  snd_strerror (err));
            return false;
            }
      if (snd_pcm_status_get_state (stat) == SND_PCM_STATE_XRUN) {
            struct timeval tnow, trig;
            gettimeofday (&tnow, 0);
            snd_pcm_status_get_trigger_tstamp (stat, &trig);
            qDebug("AlsaDriver: recover: stat = %02x, xrun of at least %8.3lf ms", _stat,
               1e3 * tnow.tv_sec - 1e3 * trig.tv_sec + 1e-3 * tnow.tv_usec - 1e-3 * trig.tv_usec);
            }

      if (pcmStop()) {
            qDebug("AlsaDriver: recover: pcmStop failed");
            return false;
            }
      if (_play_handle && ((err = snd_pcm_prepare (_play_handle)) < 0)) {
            qDebug("Alsa_driver: recover: pcm_prepare(play): %s", snd_strerror (err));
            return false;
            }
      return pcmStart();
      }


char* AlsaDriver::play_16le (const float* src, char* dst, int step, int nfrm)
      {
      while (nfrm--) {
            short d;
            float s = *src++;
            if (s >  1)
                  d = 0x7fff;
            else if (s < -1)
                  d = 0x8001;
            else
                  d = (short)(0x7fff * s);
            *((short*) dst) = d;
            dst += step;
            }
      return dst;
      }


char* AlsaDriver::play_24le(const float* src, char* dst, int step, int nfrm)
      {
      float   s;
      int     d;

      while (nfrm--) {
            s = *src++;
            if (s >  1)
                  d = 0x007fffff;
            else if (s < -1)
                  d = 0x00800001;
            else
                  d = (int)(0x007fffff * s);
            dst [0] = d;
            dst [1] = d >> 8;
            dst [2] = d >> 16;
            dst += step;
            }
      return dst;
      }


char* AlsaDriver::play_32le(const float* src, char* dst, int step, int nfrm)
      {
      float   s;
      int     d;

      while (nfrm--) {
            s = *src++;
            if (s >  1)
                  d = 0x007fffff;
            else if (s < -1)
                  d = 0x00800001;
            else
                  d = (int)(0x007fffff * s);
            *((int *) dst) = d << 8;
            dst += step;
            }
      return dst;
      }


char* AlsaDriver::clear_16le (char* dst, int step, int nfrm)
      {
      while (nfrm--) {
            *((short int *) dst) = 0;
            dst += step;
            }
      return dst;
      }


char* AlsaDriver::clear_24le(char* dst, int step, int nfrm)
      {
      while (nfrm--) {
            dst [0] = 0;
            dst [1] = 0;
            dst [2] = 0;
            dst += step;
            }
      return dst;
      }


char* AlsaDriver::clear_32le(char* dst, int step, int nfrm)
      {
      while (nfrm--) {
            *((int *) dst) = 0;
            dst += step;
            }
      return dst;
      }


void AlsaDriver::write(int n, float* l, float* r)
      {
      for (;;) {
            int err = snd_pcm_wait(_play_handle, -1);
            if (err < 0) {
                  recover();
                  continue;
                  }
            int avail = snd_pcm_avail_update(_play_handle);
            if (avail < 0) {
                  qDebug("AlsaDriver::write: snd_pcm_avail_update() (%s)", snd_strerror(avail));
                  recover();
                  continue;
                  }
            else if (avail >= n)
                  break;
            }
      if (mmappedInterface) {
            playInit(n);
            _play_ptr[0] = _play_func(l, _play_ptr[0], _play_step, n);
            _play_ptr[1] = _play_func(r, _play_ptr[1], _play_step, n);
            snd_pcm_mmap_commit(_play_handle, _play_offs, n);
            }
      else {
                                                int err;
            if (_play_access == SND_PCM_ACCESS_RW_NONINTERLEAVED) {
                                                                        short lbuffer[n];
                  short rbuffer[n];
                  _play_func(l, (char*)lbuffer, 2, n);
                  _play_func(r, (char*)rbuffer, 2, n);
                  void* bp[2];
                  bp[0] = lbuffer;
                  bp[1] = rbuffer;
                  if ((err = snd_pcm_writen(_play_handle, bp, n)) < 0)
                        qDebug("AlsaDriver::write(): failed (%s)", snd_strerror(err));
                  }
            else if (_play_access == SND_PCM_ACCESS_RW_INTERLEAVED) {
                  short buffer[n * 2];
                  _play_func(l, (char*)buffer, 4, n);
                  _play_func(r, (char*)(buffer + 1), 4, n);
                  if ((err = snd_pcm_writei(_play_handle, buffer, n)) < 0)
                        qDebug("AlsaDriver::write(): failed (%s)", snd_strerror(err));
                  }
            else {
                  qDebug("AlsaDriver::write(): unsupported access type %d", _play_access);
                  return;
                  }
            }
      }


AlsaAudio::AlsaAudio(Seq* s)
   : Driver(s)
      {
      alsa       = 0;
      state      = Transport::STOP;
      seekflag   = false;
      midiDriver = 0;
      }


int AlsaAudio::sampleRate() const
      {
      if (alsa)
            return alsa->sampleRate();
      else
            return preferences.getInt(PREF_IO_ALSA_SAMPLERATE);
      }


AlsaAudio::~AlsaAudio()
      {
      stop();
      delete alsa;
      }


bool AlsaAudio::init(bool )
      {
      alsa = new AlsaDriver(
         preferences.getString(PREF_IO_ALSA_DEVICE),
         preferences.getInt(PREF_IO_ALSA_SAMPLERATE),
         preferences.getInt(PREF_IO_ALSA_PERIODSIZE),
         preferences.getInt(PREF_IO_ALSA_FRAGMENTS));
      if (!alsa->init()) {
            delete alsa;
            alsa = 0;
            qDebug("init ALSA audio driver failed");
            return false;
            }

      midiDriver = new AlsaMidiDriver(seq);
      if (!midiDriver->init()) {
            delete midiDriver;
            midiDriver = 0;
            return false;
            }
      return true;
      }


static void* alsaLoop(void* alsa)
      {
      ((AlsaAudio*)alsa)->alsaLoop();
      return 0;
      }


void AlsaAudio::alsaLoop()
      {
                        struct sched_param rt_param;
      memset(&rt_param, 0, sizeof(rt_param));
      rt_param.sched_priority = 50;
      int rv = pthread_setschedparam(pthread_self(), SCHED_FIFO, &rt_param);
      if (rv == -1)
            perror("Set realtime scheduler failed");

      if (!alsa->pcmStart()) {
            alsa->pcmStop();
            runAlsa = 0;
            return;
            }
      int size = alsa->fsize();
      float buffer[size * 2];
      runAlsa = 2;
      while (runAlsa == 2) {
            seq->process(size, buffer);
            float l[size];
            float r[size];
            float* lp = l;
            float* rp = r;
            float* sp = buffer;
            for (int i = 0; i < size; ++i) {
                  *lp++ = *sp++;
                  *rp++ = *sp++;
                  }
            alsa->write(size, l, r);
            }
      alsa->pcmStop();
      runAlsa = 0;
      }


bool AlsaAudio::start(bool)
      {
      pthread_attr_t* attributes = (pthread_attr_t*) malloc(sizeof(pthread_attr_t));
      pthread_attr_init(attributes);
      if (pthread_create(&thread, attributes, Ms::alsaLoop, this))
            perror("creating thread failed:");
      pthread_attr_destroy(attributes);
      return true;
      }


bool AlsaAudio::stop()
      {
      if (runAlsa == 2) {
            runAlsa = 1;
            int i = 0;
            for (;i < 4; ++i) {
                  if (runAlsa == 0)
                        break;
                  sleep(1);
                  }
            pthread_cancel(thread);
            pthread_join(thread, 0);
            }
      return true;
      }


void AlsaAudio::startTransport()
      {
      state = Transport::PLAY;
      }


void AlsaAudio::stopTransport()
      {
      state = Transport::STOP;
      }


Transport AlsaAudio::getState()
      {
      return state;
      }


void AlsaAudio::midiRead()
      {
      midiDriver->read();
      }


void AlsaAudio::updateOutPortCount(int maxport)
      {
      static_cast<AlsaMidiDriver*>(midiDriver)->updateInPortCount(maxport);
      }
}

#endif

