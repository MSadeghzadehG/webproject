
#include "VideoCommon/BPStructs.h"

#include <cmath>
#include <cstring>
#include <string>

#include "Common/Logging/Log.h"
#include "Common/StringUtil.h"
#include "Common/Thread.h"
#include "Core/ConfigManager.h"
#include "Core/CoreTiming.h"
#include "Core/FifoPlayer/FifoPlayer.h"
#include "Core/FifoPlayer/FifoRecorder.h"
#include "Core/HW/Memmap.h"
#include "Core/HW/VideoInterface.h"

#include "VideoCommon/BPFunctions.h"
#include "VideoCommon/BPMemory.h"
#include "VideoCommon/BoundingBox.h"
#include "VideoCommon/Fifo.h"
#include "VideoCommon/GeometryShaderManager.h"
#include "VideoCommon/PerfQueryBase.h"
#include "VideoCommon/PixelEngine.h"
#include "VideoCommon/PixelShaderManager.h"
#include "VideoCommon/RenderBase.h"
#include "VideoCommon/TextureCacheBase.h"
#include "VideoCommon/TextureDecoder.h"
#include "VideoCommon/VertexShaderManager.h"
#include "VideoCommon/VideoBackendBase.h"
#include "VideoCommon/VideoCommon.h"
#include "VideoCommon/VideoConfig.h"

using namespace BPFunctions;

static const float s_gammaLUT[] = {1.0f, 1.7f, 2.2f, 1.0f};

void BPInit()
{
  memset(&bpmem, 0, sizeof(bpmem));
  bpmem.bpMask = 0xFFFFFF;
}

static void BPWritten(const BPCmd& bp)
{
  

    g_video_backend->CheckInvalidState();

  if (((s32*)&bpmem)[bp.address] == bp.newvalue)
  {
    if (!(bp.address == BPMEM_TRIGGER_EFB_COPY || bp.address == BPMEM_CLEARBBOX1 ||
          bp.address == BPMEM_CLEARBBOX2 || bp.address == BPMEM_SETDRAWDONE ||
          bp.address == BPMEM_PE_TOKEN_ID || bp.address == BPMEM_PE_TOKEN_INT_ID ||
          bp.address == BPMEM_LOADTLUT0 || bp.address == BPMEM_LOADTLUT1 ||
          bp.address == BPMEM_TEXINVALIDATE || bp.address == BPMEM_PRELOAD_MODE ||
          bp.address == BPMEM_CLEAR_PIXEL_PERF))
    {
      return;
    }
  }

  FlushPipeline();

  ((u32*)&bpmem)[bp.address] = bp.newvalue;

  switch (bp.address)
  {
  case BPMEM_GENMODE:      PRIM_LOG("genmode: texgen=%d, col=%d, multisampling=%d, tev=%d, cullmode=%d, ind=%d, zfeeze=%d",
             (u32)bpmem.genMode.numtexgens, (u32)bpmem.genMode.numcolchans,
             (u32)bpmem.genMode.multisampling, (u32)bpmem.genMode.numtevstages + 1,
             (u32)bpmem.genMode.cullmode, (u32)bpmem.genMode.numindstages,
             (u32)bpmem.genMode.zfreeze);

    if (bp.changes)
      PixelShaderManager::SetGenModeChanged();

        if (bp.changes & 0xC000)
      SetGenerationMode();
    return;
  case BPMEM_IND_MTXA:    case BPMEM_IND_MTXB:
  case BPMEM_IND_MTXC:
  case BPMEM_IND_MTXA + 3:
  case BPMEM_IND_MTXB + 3:
  case BPMEM_IND_MTXC + 3:
  case BPMEM_IND_MTXA + 6:
  case BPMEM_IND_MTXB + 6:
  case BPMEM_IND_MTXC + 6:
    if (bp.changes)
      PixelShaderManager::SetIndMatrixChanged((bp.address - BPMEM_IND_MTXA) / 3);
    return;
  case BPMEM_RAS1_SS0:      if (bp.changes)
      PixelShaderManager::SetIndTexScaleChanged(false);
    return;
  case BPMEM_RAS1_SS1:      if (bp.changes)
      PixelShaderManager::SetIndTexScaleChanged(true);
    return;
        case BPMEM_SCISSORTL:        case BPMEM_SCISSORBR:        case BPMEM_SCISSOROFFSET:      SetScissor();
    SetViewport();
    VertexShaderManager::SetViewportChanged();
    GeometryShaderManager::SetViewportChanged();
    return;
  case BPMEM_LINEPTWIDTH:      GeometryShaderManager::SetLinePtWidthChanged();
    return;
  case BPMEM_ZMODE:      PRIM_LOG("zmode: test=%u, func=%u, upd=%u", bpmem.zmode.testenable.Value(),
             bpmem.zmode.func.Value(), bpmem.zmode.updateenable.Value());
    SetDepthMode();
    PixelShaderManager::SetZModeControl();
    return;
  case BPMEM_BLENDMODE:      if (bp.changes & 0xFFFF)
    {
      PRIM_LOG("blendmode: en=%u, open=%u, colupd=%u, alphaupd=%u, dst=%u, src=%u, sub=%u, mode=%u",
               bpmem.blendmode.blendenable.Value(), bpmem.blendmode.logicopenable.Value(),
               bpmem.blendmode.colorupdate.Value(), bpmem.blendmode.alphaupdate.Value(),
               bpmem.blendmode.dstfactor.Value(), bpmem.blendmode.srcfactor.Value(),
               bpmem.blendmode.subtract.Value(), bpmem.blendmode.logicmode.Value());

      SetBlendMode();

      PixelShaderManager::SetBlendModeChanged();
    }
    return;
  case BPMEM_CONSTANTALPHA:      PRIM_LOG("constalpha: alp=%d, en=%d", bpmem.dstalpha.alpha.Value(),
             bpmem.dstalpha.enable.Value());
    if (bp.changes)
    {
      PixelShaderManager::SetAlpha();
      PixelShaderManager::SetDestAlphaChanged();
    }
    if (bp.changes & 0x100)
      SetBlendMode();
    return;

          case BPMEM_SETDRAWDONE:
    switch (bp.newvalue & 0xFF)
    {
    case 0x02:
      if (!Fifo::UseDeterministicGPUThread())
        PixelEngine::SetFinish();        DEBUG_LOG(VIDEO, "GXSetDrawDone SetPEFinish (value: 0x%02X)", (bp.newvalue & 0xFFFF));
      return;

    default:
      WARN_LOG(VIDEO, "GXSetDrawDone ??? (value 0x%02X)", (bp.newvalue & 0xFFFF));
      return;
    }
    return;
  case BPMEM_PE_TOKEN_ID:      if (!Fifo::UseDeterministicGPUThread())
      PixelEngine::SetToken(static_cast<u16>(bp.newvalue & 0xFFFF), false);
    DEBUG_LOG(VIDEO, "SetPEToken 0x%04x", (bp.newvalue & 0xFFFF));
    return;
  case BPMEM_PE_TOKEN_INT_ID:      if (!Fifo::UseDeterministicGPUThread())
      PixelEngine::SetToken(static_cast<u16>(bp.newvalue & 0xFFFF), true);
    DEBUG_LOG(VIDEO, "SetPEToken + INT 0x%04x", (bp.newvalue & 0xFFFF));
    return;

            case BPMEM_TRIGGER_EFB_COPY:    {
            
    u32 destAddr = bpmem.copyTexDest << 5;
    u32 destStride = bpmem.copyMipMapStrideChannels << 5;

    EFBRectangle srcRect;
    srcRect.left = static_cast<int>(bpmem.copyTexSrcXY.x);
    srcRect.top = static_cast<int>(bpmem.copyTexSrcXY.y);

                srcRect.right = static_cast<int>(bpmem.copyTexSrcXY.x + bpmem.copyTexSrcWH.x + 1);
    srcRect.bottom = static_cast<int>(bpmem.copyTexSrcXY.y + bpmem.copyTexSrcWH.y + 1);

    UPE_Copy PE_copy = bpmem.triggerEFBCopy;

        if (PE_copy.copy_to_xfb == 0)
    {
                  static constexpr CopyFilterCoefficients::Values filter_coefficients = {
          {0, 0, 21, 22, 21, 0, 0}};
      bool is_depth_copy = bpmem.zcontrol.pixel_format == PEControl::Z24;
      g_texture_cache->CopyRenderTargetToTexture(
          destAddr, PE_copy.tp_realFormat(), srcRect.GetWidth(), srcRect.GetHeight(), destStride,
          is_depth_copy, srcRect, !!PE_copy.intensity_fmt, !!PE_copy.half_scale, 1.0f, 1.0f,
          bpmem.triggerEFBCopy.clamp_top, bpmem.triggerEFBCopy.clamp_bottom, filter_coefficients);
    }
    else
    {
                  
      BoundingBox::active = false;
      PixelShaderManager::SetBoundingBoxActive(false);

      float yScale;
      if (PE_copy.scale_invert)
        yScale = 256.0f / static_cast<float>(bpmem.dispcopyyscale);
      else
        yScale = static_cast<float>(bpmem.dispcopyyscale) / 256.0f;

      float num_xfb_lines = 1.0f + bpmem.copyTexSrcWH.y * yScale;

      u32 height = static_cast<u32>(num_xfb_lines);

      DEBUG_LOG(VIDEO,
                "RenderToXFB: destAddr: %08x | srcRect {%d %d %d %d} | fbWidth: %u | "
                "fbStride: %u | fbHeight: %u | yScale: %f",
                destAddr, srcRect.left, srcRect.top, srcRect.right, srcRect.bottom,
                bpmem.copyTexSrcWH.x + 1, destStride, height, yScale);

      bool is_depth_copy = bpmem.zcontrol.pixel_format == PEControl::Z24;
      g_texture_cache->CopyRenderTargetToTexture(
          destAddr, EFBCopyFormat::XFB, srcRect.GetWidth(), height, destStride, is_depth_copy,
          srcRect, false, false, yScale, s_gammaLUT[PE_copy.gamma], bpmem.triggerEFBCopy.clamp_top,
          bpmem.triggerEFBCopy.clamp_bottom, bpmem.copyfilter.GetCoefficients());

            g_renderer->RenderToXFB(destAddr, srcRect, destStride, height, s_gammaLUT[PE_copy.gamma]);

      if (g_ActiveConfig.bImmediateXFB)
      {
                g_renderer->Swap(destAddr, destStride / 2, destStride / 2, height, srcRect,
                         CoreTiming::GetTicks());
      }
      else
      {
        if (FifoPlayer::GetInstance().IsRunningWithFakeVideoInterfaceUpdates())
        {
          VideoInterface::FakeVIUpdate(destAddr, srcRect.GetWidth(), height);
        }
      }
    }

        if (PE_copy.clear)
    {
      ClearScreen(srcRect);
    }

    return;
  }
  case BPMEM_LOADTLUT0:      return;
  case BPMEM_LOADTLUT1:    {
    u32 tlutTMemAddr = (bp.newvalue & 0x3FF) << 9;
    u32 tlutXferCount = (bp.newvalue & 0x1FFC00) >> 5;
    u32 addr = bpmem.tmem_config.tlut_src << 5;

        if (!SConfig::GetInstance().bWii)
      addr = addr & 0x01FFFFFF;

    Memory::CopyFromEmu(texMem + tlutTMemAddr, addr, tlutXferCount);

    if (g_bRecordFifoData)
      FifoRecorder::GetInstance().UseMemory(addr, tlutXferCount, MemoryUpdate::TMEM);

    TextureCacheBase::InvalidateAllBindPoints();

    return;
  }
  case BPMEM_FOGRANGE:    case BPMEM_FOGRANGE + 1:
  case BPMEM_FOGRANGE + 2:
  case BPMEM_FOGRANGE + 3:
  case BPMEM_FOGRANGE + 4:
  case BPMEM_FOGRANGE + 5:
    if (bp.changes)
      PixelShaderManager::SetFogRangeAdjustChanged();
    return;
  case BPMEM_FOGPARAM0:
  case BPMEM_FOGBMAGNITUDE:
  case BPMEM_FOGBEXPONENT:
  case BPMEM_FOGPARAM3:
    if (bp.changes)
      PixelShaderManager::SetFogParamChanged();
    return;
  case BPMEM_FOGCOLOR:      if (bp.changes)
      PixelShaderManager::SetFogColorChanged();
    return;
  case BPMEM_ALPHACOMPARE:      PRIM_LOG("alphacmp: ref0=%d, ref1=%d, comp0=%d, comp1=%d, logic=%d", (int)bpmem.alpha_test.ref0,
             (int)bpmem.alpha_test.ref1, (int)bpmem.alpha_test.comp0, (int)bpmem.alpha_test.comp1,
             (int)bpmem.alpha_test.logic);
    if (bp.changes & 0xFFFF)
      PixelShaderManager::SetAlpha();
    if (bp.changes)
    {
      PixelShaderManager::SetAlphaTestChanged();
      SetBlendMode();
    }
    return;
  case BPMEM_BIAS:      PRIM_LOG("ztex bias=0x%x", bpmem.ztex1.bias.Value());
    if (bp.changes)
      PixelShaderManager::SetZTextureBias();
    return;
  case BPMEM_ZTEX2:    {
    if (bp.changes & 3)
      PixelShaderManager::SetZTextureTypeChanged();
    if (bp.changes & 12)
      PixelShaderManager::SetZTextureOpChanged();
#if defined(_DEBUG) || defined(DEBUGFAST)
    const char* pzop[] = {"DISABLE", "ADD", "REPLACE", "?"};
    const char* pztype[] = {"Z8", "Z16", "Z24", "?"};
    PRIM_LOG("ztex op=%s, type=%s", pzop[bpmem.ztex2.op], pztype[bpmem.ztex2.type]);
#endif
  }
    return;
            case BPMEM_DISPLAYCOPYFILTER:        case BPMEM_DISPLAYCOPYFILTER + 1:    case BPMEM_DISPLAYCOPYFILTER + 2:    case BPMEM_DISPLAYCOPYFILTER + 3:    case BPMEM_COPYFILTER0:              case BPMEM_COPYFILTER1:                return;
        case BPMEM_FIELDMASK:    case BPMEM_FIELDMODE:          return;
        case BPMEM_BUSCLOCK0:     case BPMEM_BUSCLOCK1:     case BPMEM_PERF0_TRI:     case BPMEM_PERF0_QUAD:    case BPMEM_PERF1:           break;
        case BPMEM_EFB_TL:      case BPMEM_EFB_BR:      case BPMEM_EFB_ADDR:      return;
        case BPMEM_CLEAR_AR:    case BPMEM_CLEAR_GB:    case BPMEM_CLEAR_Z:       return;
        case BPMEM_CLEARBBOX1:
  case BPMEM_CLEARBBOX2:
  {
    u8 offset = bp.address & 2;
    BoundingBox::active = true;
    PixelShaderManager::SetBoundingBoxActive(true);

    if (g_ActiveConfig.backend_info.bSupportsBBox && g_ActiveConfig.bBBoxEnable)
    {
      g_renderer->BBoxWrite(offset, bp.newvalue & 0x3ff);
      g_renderer->BBoxWrite(offset + 1, bp.newvalue >> 10);
    }
  }
    return;
  case BPMEM_TEXINVALIDATE:
        TextureCacheBase::InvalidateAllBindPoints();
    return;

  case BPMEM_ZCOMPARE:      OnPixelFormatChange();
    if (bp.changes & 7)
      SetBlendMode();      PixelShaderManager::SetZModeControl();
    return;

  case BPMEM_MIPMAP_STRIDE:    case BPMEM_COPYYSCALE:     
  
  case BPMEM_IREF:
  {
    if (bp.changes)
      PixelShaderManager::SetTevIndirectChanged();
    return;
  }

  case BPMEM_TEV_KSEL:        case BPMEM_TEV_KSEL + 1:    case BPMEM_TEV_KSEL + 2:    case BPMEM_TEV_KSEL + 3:    case BPMEM_TEV_KSEL + 4:    case BPMEM_TEV_KSEL + 5:    case BPMEM_TEV_KSEL + 6:    case BPMEM_TEV_KSEL + 7:      PixelShaderManager::SetTevKSel(bp.address - BPMEM_TEV_KSEL, bp.newvalue);
    return;

  
  case BPMEM_BP_MASK:

  case BPMEM_IND_IMASK:    case BPMEM_REVBITS:        return;

  case BPMEM_CLEAR_PIXEL_PERF:
            if (PerfQueryBase::ShouldEmulate())
      g_perf_query->ResetQuery();
    return;

  case BPMEM_PRELOAD_ADDR:
  case BPMEM_PRELOAD_TMEMEVEN:
  case BPMEM_PRELOAD_TMEMODD:      return;

  case BPMEM_PRELOAD_MODE:          if (bp.newvalue != 0)
    {
                  
      BPS_TmemConfig& tmem_cfg = bpmem.tmem_config;
      u32 src_addr = tmem_cfg.preload_addr << 5;        u32 bytes_read = 0;
      u32 tmem_addr_even = tmem_cfg.preload_tmem_even * TMEM_LINE_SIZE;

      if (tmem_cfg.preload_tile_info.type != 3)
      {
        bytes_read = tmem_cfg.preload_tile_info.count * TMEM_LINE_SIZE;
        if (tmem_addr_even + bytes_read > TMEM_SIZE)
          bytes_read = TMEM_SIZE - tmem_addr_even;

        Memory::CopyFromEmu(texMem + tmem_addr_even, src_addr, bytes_read);
      }
      else        {
        u8* src_ptr = Memory::GetPointer(src_addr);

                        u32 tmem_addr_odd = tmem_cfg.preload_tmem_odd * TMEM_LINE_SIZE;

        for (u32 i = 0; i < tmem_cfg.preload_tile_info.count; ++i)
        {
          if (tmem_addr_even + TMEM_LINE_SIZE > TMEM_SIZE ||
              tmem_addr_odd + TMEM_LINE_SIZE > TMEM_SIZE)
            break;

          memcpy(texMem + tmem_addr_even, src_ptr + bytes_read, TMEM_LINE_SIZE);
          memcpy(texMem + tmem_addr_odd, src_ptr + bytes_read + TMEM_LINE_SIZE, TMEM_LINE_SIZE);
          tmem_addr_even += TMEM_LINE_SIZE;
          tmem_addr_odd += TMEM_LINE_SIZE;
          bytes_read += TMEM_LINE_SIZE * 2;
        }
      }

      if (g_bRecordFifoData)
        FifoRecorder::GetInstance().UseMemory(src_addr, bytes_read, MemoryUpdate::TMEM);

      TextureCacheBase::InvalidateAllBindPoints();
    }
    return;

                      case BPMEM_TEV_COLOR_RA:
  case BPMEM_TEV_COLOR_RA + 2:
  case BPMEM_TEV_COLOR_RA + 4:
  case BPMEM_TEV_COLOR_RA + 6:
  {
    int num = (bp.address >> 1) & 0x3;
    if (bpmem.tevregs[num].type_ra)
    {
      PixelShaderManager::SetTevKonstColor(num, 0, (s32)bpmem.tevregs[num].red);
      PixelShaderManager::SetTevKonstColor(num, 3, (s32)bpmem.tevregs[num].alpha);
    }
    else
    {
      PixelShaderManager::SetTevColor(num, 0, (s32)bpmem.tevregs[num].red);
      PixelShaderManager::SetTevColor(num, 3, (s32)bpmem.tevregs[num].alpha);
    }
    return;
  }

  case BPMEM_TEV_COLOR_BG:
  case BPMEM_TEV_COLOR_BG + 2:
  case BPMEM_TEV_COLOR_BG + 4:
  case BPMEM_TEV_COLOR_BG + 6:
  {
    int num = (bp.address >> 1) & 0x3;
    if (bpmem.tevregs[num].type_bg)
    {
      PixelShaderManager::SetTevKonstColor(num, 1, (s32)bpmem.tevregs[num].green);
      PixelShaderManager::SetTevKonstColor(num, 2, (s32)bpmem.tevregs[num].blue);
    }
    else
    {
      PixelShaderManager::SetTevColor(num, 1, (s32)bpmem.tevregs[num].green);
      PixelShaderManager::SetTevColor(num, 2, (s32)bpmem.tevregs[num].blue);
    }
    return;
  }

  default:
    break;
  }

  switch (bp.address & 0xFC)    {
        case BPMEM_TREF:
  case BPMEM_TREF + 4:
    PixelShaderManager::SetTevOrder(bp.address - BPMEM_TREF, bp.newvalue);
    return;
        case BPMEM_SU_SSIZE:    case BPMEM_SU_SSIZE + 4:
  case BPMEM_SU_SSIZE + 8:
  case BPMEM_SU_SSIZE + 12:
    if (bp.changes)
    {
      PixelShaderManager::SetTexCoordChanged((bp.address - BPMEM_SU_SSIZE) >> 1);
      GeometryShaderManager::SetTexCoordChanged((bp.address - BPMEM_SU_SSIZE) >> 1);
    }
    return;
            case BPMEM_TX_SETMODE0:    case BPMEM_TX_SETMODE0_4:
    TextureCacheBase::InvalidateAllBindPoints();
    return;

  case BPMEM_TX_SETMODE1:
  case BPMEM_TX_SETMODE1_4:
    TextureCacheBase::InvalidateAllBindPoints();
    return;
                case BPMEM_TX_SETIMAGE0:
  case BPMEM_TX_SETIMAGE0_4:
  case BPMEM_TX_SETIMAGE1:
  case BPMEM_TX_SETIMAGE1_4:
  case BPMEM_TX_SETIMAGE2:
  case BPMEM_TX_SETIMAGE2_4:
  case BPMEM_TX_SETIMAGE3:
  case BPMEM_TX_SETIMAGE3_4:
    TextureCacheBase::InvalidateAllBindPoints();
    return;
          case BPMEM_TX_SETTLUT:
  case BPMEM_TX_SETTLUT_4:
    TextureCacheBase::InvalidateAllBindPoints();
    return;

  default:
    break;
  }

  switch (bp.address & 0xF0)
  {
        case BPMEM_IND_CMD:
    PixelShaderManager::SetTevIndirectChanged();
    return;
            case BPMEM_TEV_COLOR_ENV:    case BPMEM_TEV_COLOR_ENV + 16:
    PixelShaderManager::SetTevCombiner((bp.address - BPMEM_TEV_COLOR_ENV) >> 1,
                                       (bp.address - BPMEM_TEV_COLOR_ENV) & 1, bp.newvalue);
    return;
  default:
    break;
  }

  WARN_LOG(VIDEO, "Unknown BP opcode: address = 0x%08x value = 0x%08x", bp.address, bp.newvalue);
}

void LoadBPReg(u32 value0)
{
  int regNum = value0 >> 24;
  int oldval = ((u32*)&bpmem)[regNum];
  int newval = (oldval & ~bpmem.bpMask) | (value0 & bpmem.bpMask);
  int changes = (oldval ^ newval) & 0xFFFFFF;

  BPCmd bp = {regNum, changes, newval};

    if (regNum != BPMEM_BP_MASK)
    bpmem.bpMask = 0xFFFFFF;

  BPWritten(bp);
}

void LoadBPRegPreprocess(u32 value0)
{
  int regNum = value0 >> 24;
    u32 newval = value0 & 0xffffff;
  switch (regNum)
  {
  case BPMEM_SETDRAWDONE:
    if ((newval & 0xff) == 0x02)
      PixelEngine::SetFinish();
    break;
  case BPMEM_PE_TOKEN_ID:
    PixelEngine::SetToken(newval & 0xffff, false);
    break;
  case BPMEM_PE_TOKEN_INT_ID:      PixelEngine::SetToken(newval & 0xffff, true);
    break;
  }
}

void GetBPRegInfo(const u8* data, std::string* name, std::string* desc)
{
  const char* no_yes[2] = {"No", "Yes"};

  u8 cmd = data[0];
  u32 cmddata = Common::swap32(data) & 0xFFFFFF;
  switch (cmd)
  {
#define SetRegName(reg)                                                                            \
  *name = #reg;                                                                                    \
  (void)(reg);

  case BPMEM_GENMODE:      SetRegName(BPMEM_GENMODE);
        break;

  case BPMEM_DISPLAYCOPYFILTER:          SetRegName(BPMEM_DISPLAYCOPYFILTER);
        break;

  case 0x02:    case 0x03:    case 0x04:          break;

  case BPMEM_IND_MTXA:    case BPMEM_IND_MTXA + 3:
  case BPMEM_IND_MTXA + 6:
    SetRegName(BPMEM_IND_MTXA);
        break;

  case BPMEM_IND_MTXB:    case BPMEM_IND_MTXB + 3:
  case BPMEM_IND_MTXB + 6:
    SetRegName(BPMEM_IND_MTXB);
        break;

  case BPMEM_IND_MTXC:    case BPMEM_IND_MTXC + 3:
  case BPMEM_IND_MTXC + 6:
    SetRegName(BPMEM_IND_MTXC);
        break;

  case BPMEM_IND_IMASK:      SetRegName(BPMEM_IND_IMASK);
        break;

  case BPMEM_IND_CMD:    case BPMEM_IND_CMD + 1:
  case BPMEM_IND_CMD + 2:
  case BPMEM_IND_CMD + 3:
  case BPMEM_IND_CMD + 4:
  case BPMEM_IND_CMD + 5:
  case BPMEM_IND_CMD + 6:
  case BPMEM_IND_CMD + 7:
  case BPMEM_IND_CMD + 8:
  case BPMEM_IND_CMD + 9:
  case BPMEM_IND_CMD + 10:
  case BPMEM_IND_CMD + 11:
  case BPMEM_IND_CMD + 12:
  case BPMEM_IND_CMD + 13:
  case BPMEM_IND_CMD + 14:
  case BPMEM_IND_CMD + 15:
    SetRegName(BPMEM_IND_CMD);
        break;

  case BPMEM_SCISSORTL:      SetRegName(BPMEM_SCISSORTL);
        break;

  case BPMEM_SCISSORBR:      SetRegName(BPMEM_SCISSORBR);
        break;

  case BPMEM_LINEPTWIDTH:      SetRegName(BPMEM_LINEPTWIDTH);
        break;

  case BPMEM_PERF0_TRI:      SetRegName(BPMEM_PERF0_TRI);
        break;

  case BPMEM_PERF0_QUAD:      SetRegName(BPMEM_PERF0_QUAD);
        break;

  case BPMEM_RAS1_SS0:      SetRegName(BPMEM_RAS1_SS0);
        break;

  case BPMEM_RAS1_SS1:      SetRegName(BPMEM_RAS1_SS1);
        break;

  case BPMEM_IREF:      SetRegName(BPMEM_IREF);
        break;

  case BPMEM_TREF:    case BPMEM_TREF + 1:
  case BPMEM_TREF + 2:
  case BPMEM_TREF + 3:
  case BPMEM_TREF + 4:
  case BPMEM_TREF + 5:
  case BPMEM_TREF + 6:
  case BPMEM_TREF + 7:
    SetRegName(BPMEM_TREF);
        break;

  case BPMEM_SU_SSIZE:    case BPMEM_SU_SSIZE + 2:
  case BPMEM_SU_SSIZE + 4:
  case BPMEM_SU_SSIZE + 6:
  case BPMEM_SU_SSIZE + 8:
  case BPMEM_SU_SSIZE + 10:
  case BPMEM_SU_SSIZE + 12:
  case BPMEM_SU_SSIZE + 14:
    SetRegName(BPMEM_SU_SSIZE);
        break;

  case BPMEM_SU_TSIZE:    case BPMEM_SU_TSIZE + 2:
  case BPMEM_SU_TSIZE + 4:
  case BPMEM_SU_TSIZE + 6:
  case BPMEM_SU_TSIZE + 8:
  case BPMEM_SU_TSIZE + 10:
  case BPMEM_SU_TSIZE + 12:
  case BPMEM_SU_TSIZE + 14:
    SetRegName(BPMEM_SU_TSIZE);
        break;

  case BPMEM_ZMODE:      SetRegName(BPMEM_ZMODE);
        break;

  case BPMEM_BLENDMODE:    {
    SetRegName(BPMEM_BLENDMODE);
    BlendMode mode;
    mode.hex = cmddata;
    const char* dstfactors[] = {"0",         "1",           "src_color", "1-src_color",
                                "src_alpha", "1-src_alpha", "dst_alpha", "1-dst_alpha"};
    const char* srcfactors[] = {"0",         "1",           "dst_color", "1-dst_color",
                                "src_alpha", "1-src_alpha", "dst_alpha", "1-dst_alpha"};
    const char* logicmodes[] = {"0",     "s & d",  "s & ~d",   "s",        "~s & d", "d",
                                "s ^ d", "s | d",  "~(s | d)", "~(s ^ d)", "~d",     "s | ~d",
                                "~s",    "~s | d", "~(s & d)", "1"};
    *desc = StringFromFormat(
        "Enable: %s\n"
        "Logic ops: %s\n"
        "Dither: %s\n"
        "Color write: %s\n"
        "Alpha write: %s\n"
        "Dest factor: %s\n"
        "Source factor: %s\n"
        "Subtract: %s\n"
        "Logic mode: %s\n",
        no_yes[mode.blendenable], no_yes[mode.logicopenable], no_yes[mode.dither],
        no_yes[mode.colorupdate], no_yes[mode.alphaupdate], dstfactors[mode.dstfactor],
        srcfactors[mode.srcfactor], no_yes[mode.subtract], logicmodes[mode.logicmode]);
  }
  break;

  case BPMEM_CONSTANTALPHA:      SetRegName(BPMEM_CONSTANTALPHA);
        break;

  case BPMEM_ZCOMPARE:    {
    SetRegName(BPMEM_ZCOMPARE);
    PEControl config;
    config.hex = cmddata;
    const char* pixel_formats[] = {"RGB8_Z24", "RGBA6_Z24", "RGB565_Z16", "Z24",
                                   "Y8",       "U8",        "V8",         "YUV420"};
    const char* zformats[] = {
        "linear",     "compressed (near)",     "compressed (mid)",     "compressed (far)",
        "inv linear", "compressed (inv near)", "compressed (inv mid)", "compressed (inv far)"};
    *desc = StringFromFormat("EFB pixel format: %s\n"
                             "Depth format: %s\n"
                             "Early depth test: %s\n",
                             pixel_formats[config.pixel_format], zformats[config.zformat],
                             no_yes[config.early_ztest]);
  }
  break;

  case BPMEM_FIELDMASK:      SetRegName(BPMEM_FIELDMASK);
        break;

  case BPMEM_SETDRAWDONE:      SetRegName(BPMEM_SETDRAWDONE);
        break;

  case BPMEM_BUSCLOCK0:      SetRegName(BPMEM_BUSCLOCK0);
        break;

  case BPMEM_PE_TOKEN_ID:      SetRegName(BPMEM_PE_TOKEN_ID);
        break;

  case BPMEM_PE_TOKEN_INT_ID:      SetRegName(BPMEM_PE_TOKEN_INT_ID);
        break;

  case BPMEM_EFB_TL:    {
    SetRegName(BPMEM_EFB_TL);
    X10Y10 left_top;
    left_top.hex = cmddata;
    *desc = StringFromFormat("Left: %d\nTop: %d", left_top.x, left_top.y);
  }
  break;

  case BPMEM_EFB_BR:    {
        SetRegName(BPMEM_EFB_BR);
    X10Y10 width_height;
    width_height.hex = cmddata;
    *desc = StringFromFormat("Width: %d\nHeight: %d", width_height.x + 1, width_height.y + 1);
  }
  break;

  case BPMEM_EFB_ADDR:      SetRegName(BPMEM_EFB_ADDR);
    *desc = StringFromFormat("Target address (32 byte aligned): 0x%06X", cmddata << 5);
    break;

  case BPMEM_MIPMAP_STRIDE:      SetRegName(BPMEM_MIPMAP_STRIDE);
        break;

  case BPMEM_COPYYSCALE:      SetRegName(BPMEM_COPYYSCALE);
    *desc = StringFromFormat("Scaling factor (XFB copy only): 0x%X (%f or inverted %f)", cmddata,
                             (float)cmddata / 256.f, 256.f / (float)cmddata);
    break;

  case BPMEM_CLEAR_AR:      SetRegName(BPMEM_CLEAR_AR);
    *desc = StringFromFormat("Alpha: 0x%02X\nRed: 0x%02X", (cmddata & 0xFF00) >> 8, cmddata & 0xFF);
    break;

  case BPMEM_CLEAR_GB:      SetRegName(BPMEM_CLEAR_GB);
    *desc =
        StringFromFormat("Green: 0x%02X\nBlue: 0x%02X", (cmddata & 0xFF00) >> 8, cmddata & 0xFF);
    break;

  case BPMEM_CLEAR_Z:      SetRegName(BPMEM_CLEAR_Z);
    *desc = StringFromFormat("Z value: 0x%06X", cmddata);
    break;

  case BPMEM_TRIGGER_EFB_COPY:    {
    SetRegName(BPMEM_TRIGGER_EFB_COPY);
    UPE_Copy copy;
    copy.Hex = cmddata;
    *desc = StringFromFormat(
        "Clamping: %s\n"
        "Converting from RGB to YUV: %s\n"
        "Target pixel format: 0x%X\n"
        "Gamma correction: %s\n"
        "Mipmap filter: %s\n"
        "Vertical scaling: %s\n"
        "Clear: %s\n"
        "Frame to field: 0x%01X\n"
        "Copy to XFB: %s\n"
        "Intensity format: %s\n"
        "Automatic color conversion: %s",
        (copy.clamp_top && copy.clamp_bottom) ?
            "Top and Bottom" :
            (copy.clamp_top) ? "Top only" : (copy.clamp_bottom) ? "Bottom only" : "None",
        no_yes[copy.yuv], static_cast<int>(copy.tp_realFormat()),
        (copy.gamma == 0) ?
            "1.0" :
            (copy.gamma == 1) ? "1.7" : (copy.gamma == 2) ? "2.2" : "Invalid value 0x3?",
        no_yes[copy.half_scale], no_yes[copy.scale_invert], no_yes[copy.clear],
        (u32)copy.frame_to_field, no_yes[copy.copy_to_xfb], no_yes[copy.intensity_fmt],
        no_yes[copy.auto_conv]);
  }
  break;

  case BPMEM_COPYFILTER0:      SetRegName(BPMEM_COPYFILTER0);
        break;

  case BPMEM_COPYFILTER1:      SetRegName(BPMEM_COPYFILTER1);
        break;

  case BPMEM_CLEARBBOX1:      SetRegName(BPMEM_CLEARBBOX1);
        break;

  case BPMEM_CLEARBBOX2:      SetRegName(BPMEM_CLEARBBOX2);
        break;

  case BPMEM_CLEAR_PIXEL_PERF:      SetRegName(BPMEM_CLEAR_PIXEL_PERF);
        break;

  case BPMEM_REVBITS:      SetRegName(BPMEM_REVBITS);
        break;

  case BPMEM_SCISSOROFFSET:      SetRegName(BPMEM_SCISSOROFFSET);
        break;

  case BPMEM_PRELOAD_ADDR:      SetRegName(BPMEM_PRELOAD_ADDR);
        break;

  case BPMEM_PRELOAD_TMEMEVEN:      SetRegName(BPMEM_PRELOAD_TMEMEVEN);
        break;

  case BPMEM_PRELOAD_TMEMODD:      SetRegName(BPMEM_PRELOAD_TMEMODD);
        break;

  case BPMEM_PRELOAD_MODE:      SetRegName(BPMEM_PRELOAD_MODE);
        break;

  case BPMEM_LOADTLUT0:      SetRegName(BPMEM_LOADTLUT0);
        break;

  case BPMEM_LOADTLUT1:      SetRegName(BPMEM_LOADTLUT1);
        break;

  case BPMEM_TEXINVALIDATE:      SetRegName(BPMEM_TEXINVALIDATE);
        break;

  case BPMEM_PERF1:      SetRegName(BPMEM_PERF1);
        break;

  case BPMEM_FIELDMODE:      SetRegName(BPMEM_FIELDMODE);
        break;

  case BPMEM_BUSCLOCK1:      SetRegName(BPMEM_BUSCLOCK1);
        break;

  case BPMEM_TX_SETMODE0:    case BPMEM_TX_SETMODE0 + 1:
  case BPMEM_TX_SETMODE0 + 2:
  case BPMEM_TX_SETMODE0 + 3:
    SetRegName(BPMEM_TX_SETMODE0);
        break;

  case BPMEM_TX_SETMODE1:    case BPMEM_TX_SETMODE1 + 1:
  case BPMEM_TX_SETMODE1 + 2:
  case BPMEM_TX_SETMODE1 + 3:
    SetRegName(BPMEM_TX_SETMODE1);
        break;

  case BPMEM_TX_SETIMAGE0:    case BPMEM_TX_SETIMAGE0 + 1:
  case BPMEM_TX_SETIMAGE0 + 2:
  case BPMEM_TX_SETIMAGE0 + 3:
  case BPMEM_TX_SETIMAGE0_4:    case BPMEM_TX_SETIMAGE0_4 + 1:
  case BPMEM_TX_SETIMAGE0_4 + 2:
  case BPMEM_TX_SETIMAGE0_4 + 3:
  {
    SetRegName(BPMEM_TX_SETIMAGE0);
    int texnum =
        (cmd < BPMEM_TX_SETIMAGE0_4) ? cmd - BPMEM_TX_SETIMAGE0 : cmd - BPMEM_TX_SETIMAGE0_4 + 4;
    TexImage0 teximg;
    teximg.hex = cmddata;
    *desc = StringFromFormat("Texture Unit: %i\n"
                             "Width: %i\n"
                             "Height: %i\n"
                             "Format: %x\n",
                             texnum, teximg.width + 1, teximg.height + 1, teximg.format);
  }
  break;

  case BPMEM_TX_SETIMAGE1:    case BPMEM_TX_SETIMAGE1 + 1:
  case BPMEM_TX_SETIMAGE1 + 2:
  case BPMEM_TX_SETIMAGE1 + 3:
  case BPMEM_TX_SETIMAGE1_4:    case BPMEM_TX_SETIMAGE1_4 + 1:
  case BPMEM_TX_SETIMAGE1_4 + 2:
  case BPMEM_TX_SETIMAGE1_4 + 3:
  {
    SetRegName(BPMEM_TX_SETIMAGE1);
    int texnum =
        (cmd < BPMEM_TX_SETIMAGE1_4) ? cmd - BPMEM_TX_SETIMAGE1 : cmd - BPMEM_TX_SETIMAGE1_4 + 4;
    TexImage1 teximg;
    teximg.hex = cmddata;
    *desc = StringFromFormat("Texture Unit: %i\n"
                             "Even TMEM Offset: %x\n"
                             "Even TMEM Width: %i\n"
                             "Even TMEM Height: %i\n"
                             "Cache is manually managed: %s\n",
                             texnum, teximg.tmem_even, teximg.cache_width, teximg.cache_height,
                             no_yes[teximg.image_type]);
  }
  break;

  case BPMEM_TX_SETIMAGE2:    case BPMEM_TX_SETIMAGE2 + 1:
  case BPMEM_TX_SETIMAGE2 + 2:
  case BPMEM_TX_SETIMAGE2 + 3:
  case BPMEM_TX_SETIMAGE2_4:    case BPMEM_TX_SETIMAGE2_4 + 1:
  case BPMEM_TX_SETIMAGE2_4 + 2:
  case BPMEM_TX_SETIMAGE2_4 + 3:
  {
    SetRegName(BPMEM_TX_SETIMAGE2);
    int texnum =
        (cmd < BPMEM_TX_SETIMAGE2_4) ? cmd - BPMEM_TX_SETIMAGE2 : cmd - BPMEM_TX_SETIMAGE2_4 + 4;
    TexImage2 teximg;
    teximg.hex = cmddata;
    *desc = StringFromFormat("Texture Unit: %i\n"
                             "Odd TMEM Offset: %x\n"
                             "Odd TMEM Width: %i\n"
                             "Odd TMEM Height: %i\n",
                             texnum, teximg.tmem_odd, teximg.cache_width, teximg.cache_height);
  }
  break;

  case BPMEM_TX_SETIMAGE3:    case BPMEM_TX_SETIMAGE3 + 1:
  case BPMEM_TX_SETIMAGE3 + 2:
  case BPMEM_TX_SETIMAGE3 + 3:
  case BPMEM_TX_SETIMAGE3_4:    case BPMEM_TX_SETIMAGE3_4 + 1:
  case BPMEM_TX_SETIMAGE3_4 + 2:
  case BPMEM_TX_SETIMAGE3_4 + 3:
  {
    SetRegName(BPMEM_TX_SETIMAGE3);
    int texnum =
        (cmd < BPMEM_TX_SETIMAGE3_4) ? cmd - BPMEM_TX_SETIMAGE3 : cmd - BPMEM_TX_SETIMAGE3_4 + 4;
    TexImage3 teximg;
    teximg.hex = cmddata;
    *desc = StringFromFormat("Texture %i source address (32 byte aligned): 0x%06X", texnum,
                             teximg.image_base << 5);
  }
  break;

  case BPMEM_TX_SETTLUT:    case BPMEM_TX_SETTLUT + 1:
  case BPMEM_TX_SETTLUT + 2:
  case BPMEM_TX_SETTLUT + 3:
  case BPMEM_TX_SETTLUT_4:    case BPMEM_TX_SETTLUT_4 + 1:
  case BPMEM_TX_SETTLUT_4 + 2:
  case BPMEM_TX_SETTLUT_4 + 3:
    SetRegName(BPMEM_TX_SETTLUT);
        break;

  case BPMEM_TEV_COLOR_ENV:    case BPMEM_TEV_COLOR_ENV + 2:
  case BPMEM_TEV_COLOR_ENV + 4:
  case BPMEM_TEV_COLOR_ENV + 8:
  case BPMEM_TEV_COLOR_ENV + 10:
  case BPMEM_TEV_COLOR_ENV + 12:
  case BPMEM_TEV_COLOR_ENV + 14:
  case BPMEM_TEV_COLOR_ENV + 16:
  case BPMEM_TEV_COLOR_ENV + 18:
  case BPMEM_TEV_COLOR_ENV + 20:
  case BPMEM_TEV_COLOR_ENV + 22:
  case BPMEM_TEV_COLOR_ENV + 24:
  case BPMEM_TEV_COLOR_ENV + 26:
  case BPMEM_TEV_COLOR_ENV + 28:
  case BPMEM_TEV_COLOR_ENV + 30:
  {
    SetRegName(BPMEM_TEV_COLOR_ENV);
    TevStageCombiner::ColorCombiner cc;
    cc.hex = cmddata;
    const char* tevin[] = {
        "prev.rgb", "prev.aaa", "c0.rgb",  "c0.aaa",  "c1.rgb", "c1.aaa", "c2.rgb",    "c2.aaa",
        "tex.rgb",  "tex.aaa",  "ras.rgb", "ras.aaa", "ONE",    "HALF",   "konst.rgb", "ZERO",
    };
    const char* tevbias[] = {"0", "+0.5", "-0.5", "compare"};
    const char* tevop[] = {"add", "sub"};
    const char* tevscale[] = {"1", "2", "4", "0.5"};
    const char* tevout[] = {"prev.rgb", "c0.rgb", "c1.rgb", "c2.rgb"};
    *desc = StringFromFormat("Tev stage: %d\n"
                             "a: %s\n"
                             "b: %s\n"
                             "c: %s\n"
                             "d: %s\n"
                             "Bias: %s\n"
                             "Op: %s\n"
                             "Clamp: %s\n"
                             "Scale factor: %s\n"
                             "Dest: %s\n",
                             (data[0] - BPMEM_TEV_COLOR_ENV) / 2, tevin[cc.a], tevin[cc.b],
                             tevin[cc.c], tevin[cc.d], tevbias[cc.bias], tevop[cc.op],
                             no_yes[cc.clamp], tevscale[cc.shift], tevout[cc.dest]);
    break;
  }

  case BPMEM_TEV_ALPHA_ENV:    case BPMEM_TEV_ALPHA_ENV + 2:
  case BPMEM_TEV_ALPHA_ENV + 4:
  case BPMEM_TEV_ALPHA_ENV + 6:
  case BPMEM_TEV_ALPHA_ENV + 8:
  case BPMEM_TEV_ALPHA_ENV + 10:
  case BPMEM_TEV_ALPHA_ENV + 12:
  case BPMEM_TEV_ALPHA_ENV + 14:
  case BPMEM_TEV_ALPHA_ENV + 16:
  case BPMEM_TEV_ALPHA_ENV + 18:
  case BPMEM_TEV_ALPHA_ENV + 20:
  case BPMEM_TEV_ALPHA_ENV + 22:
  case BPMEM_TEV_ALPHA_ENV + 24:
  case BPMEM_TEV_ALPHA_ENV + 26:
  case BPMEM_TEV_ALPHA_ENV + 28:
  case BPMEM_TEV_ALPHA_ENV + 30:
  {
    SetRegName(BPMEM_TEV_ALPHA_ENV);
    TevStageCombiner::AlphaCombiner ac;
    ac.hex = cmddata;
    const char* tevin[] = {
        "prev", "c0", "c1", "c2", "tex", "ras", "konst", "ZERO",
    };
    const char* tevbias[] = {"0", "+0.5", "-0.5", "compare"};
    const char* tevop[] = {"add", "sub"};
    const char* tevscale[] = {"1", "2", "4", "0.5"};
    const char* tevout[] = {"prev", "c0", "c1", "c2"};
    *desc =
        StringFromFormat("Tev stage: %d\n"
                         "a: %s\n"
                         "b: %s\n"
                         "c: %s\n"
                         "d: %s\n"
                         "Bias: %s\n"
                         "Op: %s\n"
                         "Clamp: %s\n"
                         "Scale factor: %s\n"
                         "Dest: %s\n"
                         "Ras sel: %d\n"
                         "Tex sel: %d\n",
                         (data[0] - BPMEM_TEV_ALPHA_ENV) / 2, tevin[ac.a], tevin[ac.b], tevin[ac.c],
                         tevin[ac.d], tevbias[ac.bias], tevop[ac.op], no_yes[ac.clamp],
                         tevscale[ac.shift], tevout[ac.dest], ac.rswap.Value(), ac.tswap.Value());
    break;
  }

  case BPMEM_TEV_COLOR_RA:        case BPMEM_TEV_COLOR_RA + 2:    case BPMEM_TEV_COLOR_RA + 4:    case BPMEM_TEV_COLOR_RA + 6:      SetRegName(BPMEM_TEV_COLOR_RA);
        break;

  case BPMEM_TEV_COLOR_BG:        case BPMEM_TEV_COLOR_BG + 2:    case BPMEM_TEV_COLOR_BG + 4:    case BPMEM_TEV_COLOR_BG + 6:      SetRegName(BPMEM_TEV_COLOR_BG);
        break;

  case BPMEM_FOGRANGE:    case BPMEM_FOGRANGE + 1:
  case BPMEM_FOGRANGE + 2:
  case BPMEM_FOGRANGE + 3:
  case BPMEM_FOGRANGE + 4:
  case BPMEM_FOGRANGE + 5:
    SetRegName(BPMEM_FOGRANGE);
        break;

  case BPMEM_FOGPARAM0:      SetRegName(BPMEM_FOGPARAM0);
        break;

  case BPMEM_FOGBMAGNITUDE:      SetRegName(BPMEM_FOGBMAGNITUDE);
        break;

  case BPMEM_FOGBEXPONENT:      SetRegName(BPMEM_FOGBEXPONENT);
        break;

  case BPMEM_FOGPARAM3:      SetRegName(BPMEM_FOGPARAM3);
        break;

  case BPMEM_FOGCOLOR:      SetRegName(BPMEM_FOGCOLOR);
        break;

  case BPMEM_ALPHACOMPARE:    {
    SetRegName(BPMEM_ALPHACOMPARE);
    AlphaTest test;
    test.hex = cmddata;
    const char* functions[] = {"NEVER",   "LESS",   "EQUAL",  "LEQUAL",
                               "GREATER", "NEQUAL", "GEQUAL", "ALWAYS"};
    const char* logic[] = {"AND", "OR", "XOR", "XNOR"};
    *desc = StringFromFormat("Test 1: %s (ref: %#02x)\n"
                             "Test 2: %s (ref: %#02x)\n"
                             "Logic: %s\n",
                             functions[test.comp0], (int)test.ref0, functions[test.comp1],
                             (int)test.ref1, logic[test.logic]);
    break;
  }

  case BPMEM_BIAS:      SetRegName(BPMEM_BIAS);
        break;

  case BPMEM_ZTEX2:      SetRegName(BPMEM_ZTEX2);
        break;

  case BPMEM_TEV_KSEL:    case BPMEM_TEV_KSEL + 1:
  case BPMEM_TEV_KSEL + 2:
  case BPMEM_TEV_KSEL + 3:
  case BPMEM_TEV_KSEL + 4:
  case BPMEM_TEV_KSEL + 5:
  case BPMEM_TEV_KSEL + 6:
  case BPMEM_TEV_KSEL + 7:
    SetRegName(BPMEM_TEV_KSEL);
        break;

#undef SetRegName
  }
}

void BPReload()
{
        SetGenerationMode();
  SetScissor();
  SetViewport();
  SetDepthMode();
  SetBlendMode();
  OnPixelFormatChange();
}
