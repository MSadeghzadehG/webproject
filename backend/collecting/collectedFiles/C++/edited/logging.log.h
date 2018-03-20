
#pragma once

#include "common/common_types.h"

namespace Log {

enum class Level : u8 {
    Trace,                      Debug,        Info,         Warning,      Error,                      Critical,               
    Count };

typedef u8 ClassType;


enum class Class : ClassType {
    Log,                   Common,                Common_Filesystem,     Common_Memory,         Core,                  Core_ARM,              Core_Timing,           Config,                Debug,                 Debug_Emulated,        Debug_GPU,             Debug_Breakpoint,      Debug_GDBStub,         Kernel,                Kernel_SVC,            Service,                                      Service_ACC,           Service_AM,            Service_AOC,           Service_APM,           Service_Audio,         Service_Friend,        Service_FS,            Service_HID,           Service_LM,            Service_NIFM,          Service_NS,            Service_NVDRV,         Service_PCTL,          Service_SET,           Service_SM,            Service_Time,          Service_VI,            HW,                    HW_Memory,             HW_LCD,                HW_GPU,                HW_AES,                IPC,                   Frontend,              Render,                Render_Software,       Render_OpenGL,         Audio,                 Audio_DSP,             Audio_Sink,            Loader,                Input,                 Network,               WebService,            Count              };

void LogMessage(Class log_class, Level log_level, const char* filename, unsigned int line_nr,
                const char* function,
#ifdef _MSC_VER
                _Printf_format_string_
#endif
                const char* format,
                ...)
#ifdef __GNUC__
    __attribute__((format(printf, 6, 7)))
#endif
    ;

} 
#define LOG_GENERIC(log_class, log_level, ...)                                                     \
    ::Log::LogMessage(log_class, log_level, __FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef _DEBUG
#define LOG_TRACE(log_class, ...)                                                                  \
    LOG_GENERIC(::Log::Class::log_class, ::Log::Level::Trace, __VA_ARGS__)
#else
#define LOG_TRACE(log_class, ...) (void(0))
#endif

#define LOG_DEBUG(log_class, ...)                                                                  \
    LOG_GENERIC(::Log::Class::log_class, ::Log::Level::Debug, __VA_ARGS__)
#define LOG_INFO(log_class, ...)                                                                   \
    LOG_GENERIC(::Log::Class::log_class, ::Log::Level::Info, __VA_ARGS__)
#define LOG_WARNING(log_class, ...)                                                                \
    LOG_GENERIC(::Log::Class::log_class, ::Log::Level::Warning, __VA_ARGS__)
#define LOG_ERROR(log_class, ...)                                                                  \
    LOG_GENERIC(::Log::Class::log_class, ::Log::Level::Error, __VA_ARGS__)
#define LOG_CRITICAL(log_class, ...)                                                               \
    LOG_GENERIC(::Log::Class::log_class, ::Log::Level::Critical, __VA_ARGS__)
