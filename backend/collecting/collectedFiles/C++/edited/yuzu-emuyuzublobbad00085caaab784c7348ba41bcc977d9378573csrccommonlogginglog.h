
#pragma once

#include <fmt/format.h>
#include "common/common_types.h"

namespace Log {

enum class Level : u8 {
    Trace,                      Debug,        Info,         Warning,      Error,                      Critical,               
    Count };

typedef u8 ClassType;


enum class Class : ClassType {
    Log,                   Common,                Common_Filesystem,     Common_Memory,         Core,                  Core_ARM,              Core_Timing,           Config,                Debug,                 Debug_Emulated,        Debug_GPU,             Debug_Breakpoint,      Debug_GDBStub,         Kernel,                Kernel_SVC,            Service,                                      Service_ACC,           Service_AM,            Service_AOC,           Service_APM,           Service_Audio,         Service_Fatal,         Service_Friend,        Service_FS,            Service_HID,           Service_LM,            Service_NFP,           Service_NIFM,          Service_NS,            Service_NVDRV,         Service_PCTL,          Service_PREPO,         Service_SET,           Service_SM,            Service_SPL,           Service_SSL,           Service_Time,          Service_VI,            HW,                    HW_Memory,             HW_LCD,                HW_GPU,                HW_AES,                IPC,                   Frontend,              Render,                Render_Software,       Render_OpenGL,         Audio,                 Audio_DSP,             Audio_Sink,            Loader,                Input,                 Network,               WebService,            Count              };

void FmtLogMessageImpl(Class log_class, Level log_level, const char* filename,
                       unsigned int line_num, const char* function, const char* format,
                       const fmt::format_args& args);

template <typename... Args>
void FmtLogMessage(Class log_class, Level log_level, const char* filename, unsigned int line_num,
                   const char* function, const char* format, const Args&... args) {
    FmtLogMessageImpl(log_class, log_level, filename, line_num, function, format,
                      fmt::make_args(args...));
}

} 
#ifdef _DEBUG
#define NGLOG_TRACE(log_class, ...)                                                                \
    ::Log::FmtLogMessage(::Log::Class::log_class, ::Log::Level::Trace, __FILE__, __LINE__,         \
                         __func__, __VA_ARGS__)
#else
#define NGLOG_TRACE(log_class, fmt, ...) (void(0))
#endif

#define NGLOG_DEBUG(log_class, ...)                                                                \
    ::Log::FmtLogMessage(::Log::Class::log_class, ::Log::Level::Debug, __FILE__, __LINE__,         \
                         __func__, __VA_ARGS__)
#define NGLOG_INFO(log_class, ...)                                                                 \
    ::Log::FmtLogMessage(::Log::Class::log_class, ::Log::Level::Info, __FILE__, __LINE__,          \
                         __func__, __VA_ARGS__)
#define NGLOG_WARNING(log_class, ...)                                                              \
    ::Log::FmtLogMessage(::Log::Class::log_class, ::Log::Level::Warning, __FILE__, __LINE__,       \
                         __func__, __VA_ARGS__)
#define NGLOG_ERROR(log_class, ...)                                                                \
    ::Log::FmtLogMessage(::Log::Class::log_class, ::Log::Level::Error, __FILE__, __LINE__,         \
                         __func__, __VA_ARGS__)
#define NGLOG_CRITICAL(log_class, ...)                                                             \
    ::Log::FmtLogMessage(::Log::Class::log_class, ::Log::Level::Critical, __FILE__, __LINE__,      \
                         __func__, __VA_ARGS__)
