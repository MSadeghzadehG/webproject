// Copyright 2014 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "common/common_types.h"

namespace Log {

/// Specifies the severity or level of detail of the log message.
enum class Level : u8 {
    Trace,    ///< Extremely detailed and repetitive debugging information that is likely to
              ///  pollute logs.
    Debug,    ///< Less detailed debugging information.
    Info,     ///< Status information from important points during execution.
    Warning,  ///< Minor or potential problems found during execution of a task.
    Error,    ///< Major problems found during execution of a task that prevent it from being
              ///  completed.
    Critical, ///< Major problems during execution that threathen the stability of the entire
              ///  application.

    Count ///< Total number of logging levels
};

typedef u8 ClassType;

/**
 * Specifies the sub-system that generated the log message.
 *
 * @note If you add a new entry here, also add a corresponding one to `ALL_LOG_CLASSES` in
 * backend.cpp.
 */
enum class Class : ClassType {
    Log,               ///< Messages about the log system itself
    Common,            ///< Library routines
    Common_Filesystem, ///< Filesystem interface library
    Common_Memory,     ///< Memory mapping and management functions
    Core,              ///< LLE emulation core
    Core_ARM,          ///< ARM CPU core
    Core_Timing,       ///< CoreTiming functions
    Config,            ///< Emulator configuration (including commandline)
    Debug,             ///< Debugging tools
    Debug_Emulated,    ///< Debug messages from the emulated programs
    Debug_GPU,         ///< GPU debugging tools
    Debug_Breakpoint,  ///< Logging breakpoints and watchpoints
    Debug_GDBStub,     ///< GDB Stub
    Kernel,            ///< The HLE implementation of the CTR kernel
    Kernel_SVC,        ///< Kernel system calls
    Service,           ///< HLE implementation of system services. Each major service
                       ///  should have its own subclass.
    Service_ACC,       ///< The ACC (Accounts) service
    Service_AM,        ///< The AM (Applet manager) service
    Service_AOC,       ///< The AOC (AddOn Content) service
    Service_APM,       ///< The APM (Performance) service
    Service_Audio,     ///< The Audio (Audio control) service
    Service_Friend,    ///< The friend service
    Service_FS,        ///< The FS (Filesystem) service
    Service_HID,       ///< The HID (Human interface device) service
    Service_LM,        ///< The LM (Logger) service
    Service_NIFM,      ///< The NIFM (Network interface) service
    Service_NS,        ///< The NS services
    Service_NVDRV,     ///< The NVDRV (Nvidia driver) service
    Service_PCTL,      ///< The PCTL (Parental control) service
    Service_SET,       ///< The SET (Settings) service
    Service_SM,        ///< The SM (Service manager) service
    Service_Time,      ///< The time service
    Service_VI,        ///< The VI (Video interface) service
    HW,                ///< Low-level hardware emulation
    HW_Memory,         ///< Memory-map and address translation
    HW_LCD,            ///< LCD register emulation
    HW_GPU,            ///< GPU control emulation
    HW_AES,            ///< AES engine emulation
    IPC,               ///< IPC interface
    Frontend,          ///< Emulator UI
    Render,            ///< Emulator video output and hardware acceleration
    Render_Software,   ///< Software renderer backend
    Render_OpenGL,     ///< OpenGL backend
    Audio,             ///< Audio emulation
    Audio_DSP,         ///< The HLE implementation of the DSP
    Audio_Sink,        ///< Emulator audio output backend
    Loader,            ///< ROM loader
    Input,             ///< Input emulation
    Network,           ///< Network emulation
    WebService,        ///< Interface to Citra Web Services
    Count              ///< Total number of logging classes
};

/// Logs a message to the global logger.
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

} // namespace Log

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
