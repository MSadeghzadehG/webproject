
#include "Core/Core.h"

#include <atomic>
#include <cstring>
#include <locale>
#include <mutex>
#include <queue>
#include <utility>
#include <variant>

#ifdef _WIN32
#include <windows.h>
#endif

#include "AudioCommon/AudioCommon.h"

#include "Common/CPUDetect.h"
#include "Common/CommonPaths.h"
#include "Common/CommonTypes.h"
#include "Common/FileUtil.h"
#include "Common/Flag.h"
#include "Common/Logging/LogManager.h"
#include "Common/MemoryUtil.h"
#include "Common/MsgHandler.h"
#include "Common/ScopeGuard.h"
#include "Common/StringUtil.h"
#include "Common/Thread.h"
#include "Common/Timer.h"

#include "Core/Analytics.h"
#include "Core/BootManager.h"
#include "Core/ConfigManager.h"
#include "Core/CoreTiming.h"
#include "Core/DSPEmulator.h"
#include "Core/Host.h"
#include "Core/MemTools.h"
#ifdef USE_MEMORYWATCHER
#include "Core/MemoryWatcher.h"
#endif
#include "Core/Boot/Boot.h"
#include "Core/FifoPlayer/FifoPlayer.h"
#include "Core/HLE/HLE.h"
#include "Core/HW/CPU.h"
#include "Core/HW/DSP.h"
#include "Core/HW/EXI/EXI.h"
#include "Core/HW/GCKeyboard.h"
#include "Core/HW/GCPad.h"
#include "Core/HW/HW.h"
#include "Core/HW/SystemTimers.h"
#include "Core/HW/VideoInterface.h"
#include "Core/HW/Wiimote.h"
#include "Core/IOS/IOS.h"
#include "Core/Movie.h"
#include "Core/NetPlayClient.h"
#include "Core/NetPlayProto.h"
#include "Core/PatchEngine.h"
#include "Core/PowerPC/JitInterface.h"
#include "Core/PowerPC/PowerPC.h"
#include "Core/State.h"
#include "Core/WiiRoot.h"

#ifdef USE_GDBSTUB
#include "Core/PowerPC/GDBStub.h"
#endif

#include "InputCommon/ControllerInterface/ControllerInterface.h"
#include "InputCommon/GCAdapter.h"

#include "VideoCommon/Fifo.h"
#include "VideoCommon/OnScreenDisplay.h"
#include "VideoCommon/RenderBase.h"
#include "VideoCommon/VideoBackendBase.h"

#if defined __ANDROID__ || defined __APPLE__
#include <pthread.h>
#else  #define ThreadLocalStorage thread_local
#endif

namespace Core
{
static bool s_wants_determinism;

static Common::Timer s_timer;
static std::atomic<u32> s_drawn_frame;
static std::atomic<u32> s_drawn_video;

static bool s_is_stopping = false;
static bool s_hardware_initialized = false;
static bool s_is_started = false;
static Common::Flag s_is_booting;
static void* s_window_handle = nullptr;
static std::thread s_emu_thread;
static StateChangedCallbackFunc s_on_state_changed_callback;

static std::thread s_cpu_thread;
static bool s_request_refresh_info = false;
static bool s_is_throttler_temp_disabled = false;
static bool s_frame_step = false;

struct HostJob
{
  std::function<void()> job;
  bool run_after_stop;
};
static std::mutex s_host_jobs_lock;
static std::queue<HostJob> s_host_jobs_queue;

#ifdef ThreadLocalStorage
static ThreadLocalStorage bool tls_is_cpu_thread = false;
#else
static pthread_key_t s_tls_is_cpu_key;
static pthread_once_t s_cpu_key_is_init = PTHREAD_ONCE_INIT;
static void InitIsCPUKey()
{
  pthread_key_create(&s_tls_is_cpu_key, nullptr);
}
#endif

static void EmuThread(std::unique_ptr<BootParameters> boot);

bool GetIsThrottlerTempDisabled()
{
  return s_is_throttler_temp_disabled;
}

void SetIsThrottlerTempDisabled(bool disable)
{
  s_is_throttler_temp_disabled = disable;
}

void FrameUpdateOnCPUThread()
{
  if (NetPlay::IsNetPlayRunning())
    NetPlayClient::SendTimeBase();
}


std::string StopMessage(bool main_thread, const std::string& message)
{
  return StringFromFormat("Stop [%s %i]\t%s", main_thread ? "Main Thread" : "Video Thread",
                          Common::CurrentThreadId(), message.c_str());
}

void DisplayMessage(const std::string& message, int time_in_ms)
{
  if (!IsRunning())
    return;

    for (const char& c : message)
  {
    if (!std::isprint(c, std::locale::classic()))
      return;
  }

  OSD::AddMessage(message, time_in_ms);
  Host_UpdateTitle(message);
}

bool IsRunning()
{
  return (GetState() != State::Uninitialized || s_hardware_initialized) && !s_is_stopping;
}

bool IsRunningAndStarted()
{
  return s_is_started && !s_is_stopping;
}

bool IsRunningInCurrentThread()
{
  return IsRunning() && IsCPUThread();
}

bool IsCPUThread()
{
#ifdef ThreadLocalStorage
  return tls_is_cpu_thread;
#else
      pthread_once(&s_cpu_key_is_init, InitIsCPUKey);
  return pthread_getspecific(s_tls_is_cpu_key);
#endif
}

bool IsGPUThread()
{
  const SConfig& _CoreParameter = SConfig::GetInstance();
  if (_CoreParameter.bCPUThread)
  {
    return (s_emu_thread.joinable() && (s_emu_thread.get_id() == std::this_thread::get_id()));
  }
  else
  {
    return IsCPUThread();
  }
}

bool WantsDeterminism()
{
  return s_wants_determinism;
}

bool Init(std::unique_ptr<BootParameters> boot)
{
  if (s_emu_thread.joinable())
  {
    if (IsRunning())
    {
      PanicAlertT("Emu Thread already running");
      return false;
    }

        s_emu_thread.join();
  }

    HostDispatchJobs();

  Core::UpdateWantDeterminism( true);

  INFO_LOG(BOOT, "Starting core = %s mode", SConfig::GetInstance().bWii ? "Wii" : "GameCube");
  INFO_LOG(BOOT, "CPU Thread separate = %s", SConfig::GetInstance().bCPUThread ? "Yes" : "No");

  Host_UpdateMainFrame();  
  s_window_handle = Host_GetRenderHandle();

    s_emu_thread = std::thread(EmuThread, std::move(boot));
  return true;
}

void Stop()  {
  if (GetState() == State::Stopping)
    return;

  const SConfig& _CoreParameter = SConfig::GetInstance();

  s_is_stopping = true;

    HostDispatchJobs();

  Fifo::EmulatorState(false);

  INFO_LOG(CONSOLE, "Stop [Main Thread]\t\t---- Shutting down ----");

    INFO_LOG(CONSOLE, "%s", StopMessage(true, "Stop CPU").c_str());
  CPU::Stop();

  if (_CoreParameter.bCPUThread)
  {
                INFO_LOG(CONSOLE, "%s", StopMessage(true, "Wait for Video Loop to exit ...").c_str());

    g_video_backend->Video_ExitLoop();
  }
#if defined(__LIBUSB__)
  GCAdapter::ResetRumble();
#endif

#ifdef USE_MEMORYWATCHER
  MemoryWatcher::Shutdown();
#endif
}

void DeclareAsCPUThread()
{
#ifdef ThreadLocalStorage
  tls_is_cpu_thread = true;
#else
      pthread_once(&s_cpu_key_is_init, InitIsCPUKey);
  pthread_setspecific(s_tls_is_cpu_key, (void*)true);
#endif
}

void UndeclareAsCPUThread()
{
#ifdef ThreadLocalStorage
  tls_is_cpu_thread = false;
#else
      pthread_once(&s_cpu_key_is_init, InitIsCPUKey);
  pthread_setspecific(s_tls_is_cpu_key, (void*)false);
#endif
}

static void CPUSetInitialExecutionState()
{
      QueueHostJob([]() {
    SetState(SConfig::GetInstance().bBootToPause ? State::Paused : State::Running);
    Host_UpdateDisasmDialog();
    Host_UpdateMainFrame();
    Host_Message(WM_USER_CREATE);
  });
}

static void CpuThread(const std::optional<std::string>& savestate_path, bool delete_savestate)
{
  DeclareAsCPUThread();

  const SConfig& _CoreParameter = SConfig::GetInstance();
  if (_CoreParameter.bCPUThread)
    Common::SetCurrentThreadName("CPU thread");
  else
    Common::SetCurrentThreadName("CPU-GPU thread");

    DolphinAnalytics::Instance()->ReportGameStart();

  if (_CoreParameter.bFastmem)
    EMM::InstallExceptionHandler();  
#ifdef USE_MEMORYWATCHER
  MemoryWatcher::Init();
#endif

  if (savestate_path)
  {
    ::State::LoadAs(*savestate_path);
    if (delete_savestate)
      File::Delete(*savestate_path);
  }

  s_is_started = true;
  CPUSetInitialExecutionState();

#ifdef USE_GDBSTUB
#ifndef _WIN32
  if (!_CoreParameter.gdb_socket.empty())
  {
    gdb_init_local(_CoreParameter.gdb_socket.data());
    gdb_break();
  }
  else
#endif
      if (_CoreParameter.iGDBPort > 0)
  {
    gdb_init(_CoreParameter.iGDBPort);
        gdb_break();
  }
#endif

    CPU::Run();

  s_is_started = false;

  if (_CoreParameter.bFastmem)
    EMM::UninstallExceptionHandler();
}

static void FifoPlayerThread(const std::optional<std::string>& savestate_path,
                             bool delete_savestate)
{
  DeclareAsCPUThread();

  const SConfig& _CoreParameter = SConfig::GetInstance();
  if (_CoreParameter.bCPUThread)
    Common::SetCurrentThreadName("FIFO player thread");
  else
    Common::SetCurrentThreadName("FIFO-GPU thread");

    if (auto cpu_core = FifoPlayer::GetInstance().GetCPUCore())
  {
    PowerPC::InjectExternalCPUCore(cpu_core.get());
    s_is_started = true;

    CPUSetInitialExecutionState();
    CPU::Run();

    s_is_started = false;
    PowerPC::InjectExternalCPUCore(nullptr);
    FifoPlayer::GetInstance().Close();
  }
  else
  {
        PanicAlert("FIFO file is invalid, cannot playback.");
    FifoPlayer::GetInstance().Close();
    return;
  }
}

static void EmuThread(std::unique_ptr<BootParameters> boot)
{
  const SConfig& core_parameter = SConfig::GetInstance();
  s_is_booting.Set();
  if (s_on_state_changed_callback)
    s_on_state_changed_callback(State::Starting);
  Common::ScopeGuard flag_guard{[] {
    s_is_booting.Clear();
    s_is_started = false;
    s_is_stopping = false;
    s_wants_determinism = false;

    if (s_on_state_changed_callback)
      s_on_state_changed_callback(State::Uninitialized);

    INFO_LOG(CONSOLE, "Stop\t\t---- Shutdown complete ----");
  }};

  Common::SetCurrentThreadName("Emuthread - Starting");

    DeclareAsCPUThread();
  s_frame_step = false;

  Movie::Init(*boot);
  Common::ScopeGuard movie_guard{Movie::Shutdown};

  HW::Init();
  Common::ScopeGuard hw_guard{[] {
        s_hardware_initialized = false;
    INFO_LOG(CONSOLE, "%s", StopMessage(false, "Shutting down HW").c_str());
    HW::Shutdown();
    INFO_LOG(CONSOLE, "%s", StopMessage(false, "HW shutdown").c_str());

        OSD::ClearMessages();

            BootManager::RestoreConfig();

    PatchEngine::Shutdown();
    HLE::Clear();
  }};

  if (!g_video_backend->Initialize(s_window_handle))
  {
    PanicAlert("Failed to initialize video backend!");
    return;
  }
  Common::ScopeGuard video_guard{[] { g_video_backend->Shutdown(); }};

  if (cpu_info.HTT)
    SConfig::GetInstance().bDSPThread = cpu_info.num_cores > 4;
  else
    SConfig::GetInstance().bDSPThread = cpu_info.num_cores > 2;

  if (!DSP::GetDSPEmulator()->Initialize(core_parameter.bWii, core_parameter.bDSPThread))
  {
    PanicAlert("Failed to initialize DSP emulation!");
    return;
  }

  bool init_controllers = false;
  if (!g_controller_interface.IsInit())
  {
    g_controller_interface.Initialize(s_window_handle);
    Pad::Initialize();
    Keyboard::Initialize();
    init_controllers = true;
  }
  else
  {
        Pad::LoadConfig();
    Keyboard::LoadConfig();
  }

  const std::optional<std::string> savestate_path = boot->savestate_path;
  const bool delete_savestate = boot->delete_savestate;

    if (core_parameter.bWii && !SConfig::GetInstance().m_bt_passthrough_enabled)
  {
    if (init_controllers)
    {
      Wiimote::Initialize(savestate_path ? Wiimote::InitializeMode::DO_WAIT_FOR_WIIMOTES :
                                           Wiimote::InitializeMode::DO_NOT_WAIT_FOR_WIIMOTES);
    }
    else
    {
      Wiimote::LoadConfig();
    }
  }

  Common::ScopeGuard controller_guard{[init_controllers] {
    if (!init_controllers)
      return;

    Wiimote::Shutdown();
    Keyboard::Shutdown();
    Pad::Shutdown();
    g_controller_interface.Shutdown();
  }};

  AudioCommon::InitSoundStream();
  Common::ScopeGuard audio_guard{AudioCommon::ShutdownSoundStream};

    s_hardware_initialized = true;
  s_is_booting.Clear();

    CPU::Break();

    PowerPC::SetMode(PowerPC::CoreMode::Interpreter);

    void (*cpuThreadFunc)(const std::optional<std::string>& savestate_path, bool delete_savestate);
  if (std::holds_alternative<BootParameters::DFF>(boot->parameters))
    cpuThreadFunc = FifoPlayerThread;
  else
    cpuThreadFunc = CpuThread;

  if (!CBoot::BootUp(std::move(boot)))
    return;

    Fifo::Prepare();

    if (core_parameter.iCPUCore != PowerPC::CORE_INTERPRETER &&
      (!core_parameter.bRunCompareServer || core_parameter.bRunCompareClient))
  {
    PowerPC::SetMode(PowerPC::CoreMode::JIT);
  }
  else
  {
    PowerPC::SetMode(PowerPC::CoreMode::Interpreter);
  }

    if (core_parameter.bCPUThread)
  {
            Common::SetCurrentThreadName("Video thread");
    UndeclareAsCPUThread();

        s_cpu_thread = std::thread(cpuThreadFunc, savestate_path, delete_savestate);

        Fifo::RunGpuLoop();

        INFO_LOG(CONSOLE, "%s", StopMessage(false, "Video Loop Ended").c_str());

        s_cpu_thread.join();
    INFO_LOG(CONSOLE, "%s", StopMessage(true, "CPU thread stopped.").c_str());
  }
  else    {
        cpuThreadFunc(savestate_path, delete_savestate);
  }

#ifdef USE_GDBSTUB
  INFO_LOG(CONSOLE, "%s", StopMessage(true, "Stopping GDB ...").c_str());
  gdb_deinit();
  INFO_LOG(CONSOLE, "%s", StopMessage(true, "GDB stopped.").c_str());
#endif
}


void SetState(State state)
{
    if (!IsRunningAndStarted())
    return;

  switch (state)
  {
  case State::Paused:
            CPU::EnableStepping(true);      Wiimote::Pause();
#if defined(__LIBUSB__)
    GCAdapter::ResetRumble();
#endif
    break;
  case State::Running:
    CPU::EnableStepping(false);
    Wiimote::Resume();
    break;
  default:
    PanicAlert("Invalid state");
    break;
  }

  if (s_on_state_changed_callback)
    s_on_state_changed_callback(GetState());
}

State GetState()
{
  if (s_is_stopping)
    return State::Stopping;

  if (s_hardware_initialized)
  {
    if (CPU::IsStepping() || s_frame_step)
      return State::Paused;

    return State::Running;
  }

  if (s_is_booting.IsSet())
    return State::Starting;

  return State::Uninitialized;
}

static std::string GenerateScreenshotFolderPath()
{
  const std::string& gameId = SConfig::GetInstance().GetGameID();
  std::string path = File::GetUserPath(D_SCREENSHOTS_IDX) + gameId + DIR_SEP_CHR;

  if (!File::CreateFullPath(path))
  {
        path = File::GetUserPath(D_SCREENSHOTS_IDX);
  }

  return path;
}

static std::string GenerateScreenshotName()
{
  std::string path = GenerateScreenshotFolderPath();

    path += SConfig::GetInstance().GetGameID();

  std::string name;
  for (int i = 1; File::Exists(name = StringFromFormat("%s-%d.png", path.c_str(), i)); ++i)
  {
      }

  return name;
}

void SaveScreenShot(bool wait_for_completion)
{
  const bool bPaused = GetState() == State::Paused;

  SetState(State::Paused);

  g_renderer->SaveScreenshot(GenerateScreenshotName(), wait_for_completion);

  if (!bPaused)
    SetState(State::Running);
}

void SaveScreenShot(const std::string& name, bool wait_for_completion)
{
  const bool bPaused = GetState() == State::Paused;

  SetState(State::Paused);

  std::string filePath = GenerateScreenshotFolderPath() + name + ".png";

  g_renderer->SaveScreenshot(filePath, wait_for_completion);

  if (!bPaused)
    SetState(State::Running);
}

void RequestRefreshInfo()
{
  s_request_refresh_info = true;
}

static bool PauseAndLock(bool do_lock, bool unpause_on_unlock)
{
    if (!IsRunning())
    return true;

  bool was_unpaused = true;
  if (do_lock)
  {
                was_unpaused = CPU::PauseAndLock(true);
  }

  ExpansionInterface::PauseAndLock(do_lock, false);

    DSP::GetDSPEmulator()->PauseAndLock(do_lock, false);

      Fifo::PauseAndLock(do_lock, false);

#if defined(__LIBUSB__)
  GCAdapter::ResetRumble();
#endif

      if (!do_lock)
  {
                    was_unpaused = CPU::PauseAndLock(false, unpause_on_unlock, true);
  }

  return was_unpaused;
}

void RunAsCPUThread(std::function<void()> function)
{
  const bool is_cpu_thread = IsCPUThread();
  bool was_unpaused = false;
  if (!is_cpu_thread)
    was_unpaused = PauseAndLock(true, true);

  function();

  if (!is_cpu_thread)
    PauseAndLock(false, was_unpaused);
}

void VideoThrottle()
{
    u32 ElapseTime = (u32)s_timer.GetTimeDifference();
  if ((ElapseTime >= 1000 && s_drawn_video.load() > 0) || s_request_refresh_info)
  {
    UpdateTitle();

        s_timer.Update();
    s_drawn_frame.store(0);
    s_drawn_video.store(0);
  }

  s_drawn_video++;
}

bool ShouldSkipFrame(int skipped)
{
  u32 TargetFPS = VideoInterface::GetTargetRefreshRate();
  if (SConfig::GetInstance().m_EmulationSpeed > 0.0f)
    TargetFPS = u32(TargetFPS * SConfig::GetInstance().m_EmulationSpeed);
  const u32 frames = s_drawn_frame.load();
  const bool fps_slow = !(s_timer.GetTimeDifference() < (frames + skipped) * 1000 / TargetFPS);

  return fps_slow;
}


void Callback_VideoCopiedToXFB(bool video_update)
{
  if (video_update)
    s_drawn_frame++;

  Movie::FrameUpdate();

  if (s_frame_step)
  {
    s_frame_step = false;
    CPU::Break();
    if (s_on_state_changed_callback)
      s_on_state_changed_callback(Core::GetState());
  }
}

void UpdateTitle()
{
  u32 ElapseTime = (u32)s_timer.GetTimeDifference();
  s_request_refresh_info = false;
  SConfig& _CoreParameter = SConfig::GetInstance();

  if (ElapseTime == 0)
    ElapseTime = 1;

  float FPS = (float)(s_drawn_frame.load() * 1000.0 / ElapseTime);
  float VPS = (float)(s_drawn_video.load() * 1000.0 / ElapseTime);
  float Speed = (float)(s_drawn_video.load() * (100 * 1000.0) /
                        (VideoInterface::GetTargetRefreshRate() * ElapseTime));

    std::string SSettings = StringFromFormat(
      "%s %s | %s | %s", PowerPC::GetCPUName(), _CoreParameter.bCPUThread ? "DC" : "SC",
      g_video_backend->GetDisplayName().c_str(), _CoreParameter.bDSPHLE ? "HLE" : "LLE");

  std::string SFPS;

  if (Movie::IsPlayingInput())
    SFPS = StringFromFormat("Input: %u/%u - VI: %u - FPS: %.0f - VPS: %.0f - %.0f%%",
                            (u32)Movie::GetCurrentInputCount(), (u32)Movie::GetTotalInputCount(),
                            (u32)Movie::GetCurrentFrame(), FPS, VPS, Speed);
  else if (Movie::IsRecordingInput())
    SFPS = StringFromFormat("Input: %u - VI: %u - FPS: %.0f - VPS: %.0f - %.0f%%",
                            (u32)Movie::GetCurrentInputCount(), (u32)Movie::GetCurrentFrame(), FPS,
                            VPS, Speed);
  else
  {
    SFPS = StringFromFormat("FPS: %.0f - VPS: %.0f - %.0f%%", FPS, VPS, Speed);
    if (SConfig::GetInstance().m_InterfaceExtendedFPSInfo)
    {
                        static u64 ticks = 0;
      static u64 idleTicks = 0;
      u64 newTicks = CoreTiming::GetTicks();
      u64 newIdleTicks = CoreTiming::GetIdleTicks();

      u64 diff = (newTicks - ticks) / 1000000;
      u64 idleDiff = (newIdleTicks - idleTicks) / 1000000;

      ticks = newTicks;
      idleTicks = newIdleTicks;

      float TicksPercentage =
          (float)diff / (float)(SystemTimers::GetTicksPerSecond() / 1000000) * 100;

      SFPS += StringFromFormat(" | CPU: ~%i MHz [Real: %i + IdleSkip: %i] / %i MHz (~%3.0f%%)",
                               (int)(diff), (int)(diff - idleDiff), (int)(idleDiff),
                               SystemTimers::GetTicksPerSecond() / 1000000, TicksPercentage);
    }
  }

  std::string message = StringFromFormat("%s | %s", SSettings.c_str(), SFPS.c_str());
  if (SConfig::GetInstance().m_show_active_title)
  {
    const std::string& title = SConfig::GetInstance().GetTitleDescription();
    if (!title.empty())
      message += " | " + title;
  }

    if (g_sound_stream)
  {
    Mixer* pMixer = g_sound_stream->GetMixer();
    pMixer->UpdateSpeed((float)Speed / 100);
  }

  Host_UpdateTitle(message);
}

void Shutdown()
{
              if (s_emu_thread.joinable())
    s_emu_thread.join();

    HostDispatchJobs();
}

void SetOnStateChangedCallback(StateChangedCallbackFunc callback)
{
  s_on_state_changed_callback = std::move(callback);
}

void UpdateWantDeterminism(bool initial)
{
        bool new_want_determinism = Movie::IsMovieActive() || NetPlay::IsNetPlayRunning();
  if (new_want_determinism != s_wants_determinism || initial)
  {
    NOTICE_LOG(COMMON, "Want determinism <- %s", new_want_determinism ? "true" : "false");

    RunAsCPUThread([&] {
      s_wants_determinism = new_want_determinism;
      const auto ios = IOS::HLE::GetIOS();
      if (ios)
        ios->UpdateWantDeterminism(new_want_determinism);
      Fifo::UpdateWantDeterminism(new_want_determinism);
                  JitInterface::ClearCache();

            if (!initial)
        Core::InitializeWiiRoot(s_wants_determinism);
    });
  }
}

void QueueHostJob(std::function<void()> job, bool run_during_stop)
{
  if (!job)
    return;

  bool send_message = false;
  {
    std::lock_guard<std::mutex> guard(s_host_jobs_lock);
    send_message = s_host_jobs_queue.empty();
    s_host_jobs_queue.emplace(HostJob{std::move(job), run_during_stop});
  }
    if (send_message)
    Host_Message(WM_USER_JOB_DISPATCH);
}

void HostDispatchJobs()
{
        std::unique_lock<std::mutex> guard(s_host_jobs_lock);
  while (!s_host_jobs_queue.empty())
  {
    HostJob job = std::move(s_host_jobs_queue.front());
    s_host_jobs_queue.pop();

                        if (!job.run_after_stop && !s_is_booting.IsSet() && !IsRunning())
      continue;

    guard.unlock();
    job.job();
    guard.lock();
  }
}

void DoFrameStep()
{
  if (GetState() == State::Paused)
  {
        s_frame_step = true;
    RequestRefreshInfo();
    SetState(State::Running);
  }
  else if (!s_frame_step)
  {
        SetState(State::Paused);
  }
}

}  