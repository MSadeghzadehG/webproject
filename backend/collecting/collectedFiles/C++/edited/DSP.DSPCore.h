
#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <memory>
#include <string>

#include "Core/DSP/DSPBreakpoints.h"
#include "Core/DSP/DSPCaptureLogger.h"

namespace DSP
{
class Accelerator;

namespace JIT
{
namespace x64
{
class DSPEmitter;
}
}

enum : u32
{
  DSP_IRAM_BYTE_SIZE = 0x2000,
  DSP_IRAM_SIZE = 0x1000,
  DSP_IRAM_MASK = 0x0fff
};

enum : u32
{
  DSP_IROM_BYTE_SIZE = 0x2000,
  DSP_IROM_SIZE = 0x1000,
  DSP_IROM_MASK = 0x0fff
};

enum : u32
{
  DSP_DRAM_BYTE_SIZE = 0x2000,
  DSP_DRAM_SIZE = 0x1000,
  DSP_DRAM_MASK = 0x0fff
};

enum : u32
{
  DSP_COEF_BYTE_SIZE = 0x1000,
  DSP_COEF_SIZE = 0x800,
  DSP_COEF_MASK = 0x7ff
};

enum : u16
{
  DSP_RESET_VECTOR = 0x8000
};

enum : u8
{
  DSP_STACK_DEPTH = 0x20,
  DSP_STACK_MASK = 0x1f
};

enum : u32
{
  DSP_CR_IMEM = 2,
  DSP_CR_DMEM = 0,
  DSP_CR_TO_CPU = 1,
  DSP_CR_FROM_CPU = 0
};

enum : int
{
    DSP_REG_AR0 = 0x00,
  DSP_REG_AR1 = 0x01,
  DSP_REG_AR2 = 0x02,
  DSP_REG_AR3 = 0x03,

    DSP_REG_IX0 = 0x04,
  DSP_REG_IX1 = 0x05,
  DSP_REG_IX2 = 0x06,
  DSP_REG_IX3 = 0x07,

    DSP_REG_WR0 = 0x08,
  DSP_REG_WR1 = 0x09,
  DSP_REG_WR2 = 0x0a,
  DSP_REG_WR3 = 0x0b,

    DSP_REG_ST0 = 0x0c,
  DSP_REG_ST1 = 0x0d,
  DSP_REG_ST2 = 0x0e,
  DSP_REG_ST3 = 0x0f,

    DSP_REG_CR = 0x12,
  DSP_REG_SR = 0x13,

    DSP_REG_PRODL = 0x14,
  DSP_REG_PRODM = 0x15,
  DSP_REG_PRODH = 0x16,
  DSP_REG_PRODM2 = 0x17,

  DSP_REG_AXL0 = 0x18,
  DSP_REG_AXL1 = 0x19,
  DSP_REG_AXH0 = 0x1a,
  DSP_REG_AXH1 = 0x1b,

    DSP_REG_ACC0 = 0x1c,
  DSP_REG_ACC1 = 0x1d,

  DSP_REG_ACL0 = 0x1c,    DSP_REG_ACL1 = 0x1d,
  DSP_REG_ACM0 = 0x1e,    DSP_REG_ACM1 = 0x1f,
  DSP_REG_ACH0 = 0x10,    DSP_REG_ACH1 = 0x11   };

enum : u32
{
  DSP_COEF_A1_0 = 0xa0,

  DSP_DSCR = 0xc9,     DSP_DSPA = 0xcd,     DSP_DSBL = 0xcb,     DSP_DSMAH = 0xce,    DSP_DSMAL = 0xcf,  
  DSP_FORMAT = 0xd1,     DSP_ACUNK = 0xd2,      DSP_ACDATA1 = 0xd3,    DSP_ACSAH = 0xd4,      DSP_ACSAL = 0xd5,
  DSP_ACEAH = 0xd6,    DSP_ACEAL = 0xd7,
  DSP_ACCAH = 0xd8,    DSP_ACCAL = 0xd9,
  DSP_PRED_SCALE = 0xda,    DSP_YN1 = 0xdb,
  DSP_YN2 = 0xdc,
  DSP_ACCELERATOR = 0xdd,    DSP_GAIN = 0xde,
  DSP_ACUNK2 = 0xdf,  
  DSP_AMDM = 0xef,  
  DSP_DIRQ = 0xfb,    DSP_DMBH = 0xfc,    DSP_DMBL = 0xfd,    DSP_CMBH = 0xfe,    DSP_CMBL = 0xff   };

enum class StackRegister
{
  Call,
  Data,
  LoopAddress,
  LoopCounter
};

enum : u32
{
  CR_EXTERNAL_INT = 0x0002,
  CR_HALT = 0x0004,
  CR_INIT = 0x0400
};

enum : u16
{
  SR_CARRY = 0x0001,
  SR_OVERFLOW = 0x0002,
  SR_ARITH_ZERO = 0x0004,
  SR_SIGN = 0x0008,
  SR_OVER_S32 = 0x0010,    SR_TOP2BITS = 0x0020,    SR_LOGIC_ZERO = 0x0040,
  SR_OVERFLOW_STICKY =
      0x0080,    SR_100 = 0x0100,           SR_INT_ENABLE = 0x0200,    SR_400 = 0x0400,           SR_EXT_INT_ENABLE = 0x0800,    SR_1000 = 0x1000,              SR_MUL_MODIFY = 0x2000,        SR_40_MODE_BIT = 0x4000,                                      SR_MUL_UNSIGNED = 0x8000,                                   
    SR_CMP_MASK = 0x3f
};

enum : int
{
  EXP_STOVF = 1,    EXP_2 = 2,        EXP_3 = 3,        EXP_4 = 4,        EXP_ACCOV = 5,    EXP_6 = 6,        EXP_INT = 7     };

struct DSP_Regs
{
  u16 ar[4];
  u16 ix[4];
  u16 wr[4];
  u16 st[4];
  u16 cr;
  u16 sr;

  union
  {
    u64 val;
    struct
    {
      u16 l;
      u16 m;
      u16 h;
      u16 m2;      };
  } prod;

  union
  {
    u32 val;
    struct
    {
      u16 l;
      u16 h;
    };
  } ax[2];

  union
  {
    u64 val;
    struct
    {
      u16 l;
      u16 m;
      u16 h;
    };
  } ac[2];
};

struct SDSP
{
  DSP_Regs r;
  u16 pc;
#if PROFILE
  u16 err_pc;
#endif

          u16 cr;

  u8 reg_stack_ptr[4];
  u8 exceptions;    volatile bool external_interrupt_waiting;
  bool reset_dspjit_codespace;

              u16 reg_stack[4][DSP_STACK_DEPTH];

    u32 iram_crc;
  u64 step_counter;

    std::atomic<u32> mbox[2];

    std::array<u16, 256> ifx_regs;

  std::unique_ptr<Accelerator> accelerator;

      u16* iram;
  u16* dram;
  u16* irom;
  u16* coef;

    u8* cpu_ram;
};

extern SDSP g_dsp;
extern DSPBreakpoints g_dsp_breakpoints;
extern bool g_init_hax;
extern std::unique_ptr<JIT::x64::DSPEmitter> g_dsp_jit;
extern std::unique_ptr<DSPCaptureLogger> g_dsp_cap;

struct DSPInitOptions
{
    std::array<u16, DSP_IROM_SIZE> irom_contents;

    std::array<u16, DSP_COEF_SIZE> coef_contents;

      enum CoreType
  {
    CORE_INTERPRETER,
    CORE_JIT,
  };
  CoreType core_type;

      DSPCaptureLogger* capture_logger;

  DSPInitOptions() : core_type(CORE_JIT), capture_logger(new DefaultDSPCaptureLogger()) {}
};

bool DSPCore_Init(const DSPInitOptions& opts);

void DSPCore_Reset();
void DSPCore_Shutdown();  
void DSPCore_CheckExternalInterrupt();
void DSPCore_CheckExceptions();
void DSPCore_SetExternalInterrupt(bool val);

void DSPCore_SetException(u8 level);

enum class State
{
  Stopped,
  Running,
  Stepping,
};

int DSPCore_RunCycles(int cycles);

void DSPCore_SetState(State new_state);
State DSPCore_GetState();

void DSPCore_Step();

u16 DSPCore_ReadRegister(size_t reg);
void DSPCore_WriteRegister(size_t reg, u16 val);
}  