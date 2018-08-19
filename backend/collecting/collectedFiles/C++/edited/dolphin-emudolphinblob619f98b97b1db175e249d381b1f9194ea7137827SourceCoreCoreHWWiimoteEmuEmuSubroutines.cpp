




#include <fstream>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

#include "Common/ChunkFile.h"
#include "Common/CommonTypes.h"
#include "Common/FileUtil.h"
#include "Common/Logging/Log.h"
#include "Common/MsgHandler.h"
#include "Common/Swap.h"
#include "Core/Core.h"
#include "Core/HW/WiimoteCommon/WiimoteHid.h"
#include "Core/HW/WiimoteEmu/Attachment/Attachment.h"
#include "Core/HW/WiimoteEmu/WiimoteEmu.h"
#include "Core/HW/WiimoteReal/WiimoteReal.h"
#include "InputCommon/ControllerEmu/ControlGroup/Extension.h"

namespace WiimoteEmu
{
void Wiimote::ReportMode(const wm_report_mode* const dr)
{
          
    m_reporting_auto = dr->continuous;    m_reporting_mode = dr->mode;
  
    
  if (dr->mode > 0x37)
    PanicAlert("Wiimote: Unsupported Reporting mode.");
  else if (dr->mode < RT_REPORT_CORE)
    PanicAlert("Wiimote: Reporting mode < 0x30.");
}


void Wiimote::HidOutputReport(const wm_report* const sr, const bool send_ack)
{
  DEBUG_LOG(WIIMOTE, "HidOutputReport (page: %i, cid: 0x%02x, wm: 0x%02x)", m_index,
            m_reporting_channel, sr->wm);

      m_rumble_on = sr->rumble;

  switch (sr->wm)
  {
  case RT_RUMBLE:          return;      break;

  case RT_LEDS:          m_status.leds = sr->data[0] >> 4;
    break;

  case RT_REPORT_MODE:      ReportMode(reinterpret_cast<const wm_report_mode*>(sr->data));
    break;

  case RT_IR_PIXEL_CLOCK:              if (false == sr->ack)
      return;
    break;

  case RT_SPEAKER_ENABLE:              m_status.speaker = sr->enable;
    if (false == sr->ack)
      return;
    break;

  case RT_REQUEST_STATUS:      if (WIIMOTE_SRC_EMU & g_wiimote_sources[m_index])
      RequestStatus(reinterpret_cast<const wm_request_status*>(sr->data));
    return;      break;

  case RT_WRITE_DATA:      WriteData(reinterpret_cast<const wm_write_data*>(sr->data));
    break;

  case RT_READ_DATA:      if (WIIMOTE_SRC_EMU & g_wiimote_sources[m_index])
      ReadData(reinterpret_cast<const wm_read_data*>(sr->data));
    return;      break;

  case RT_WRITE_SPEAKER_DATA:      if (WIIMOTE_SRC_EMU & g_wiimote_sources[m_index] && !m_speaker_mute)
      Wiimote::SpeakerData(reinterpret_cast<const wm_speaker_data*>(sr->data));
    return;      break;

  case RT_SPEAKER_MUTE:      m_speaker_mute = sr->enable;
    if (false == sr->ack)
      return;
    break;

  case RT_IR_LOGIC:                  m_status.ir = sr->enable;
    if (false == sr->ack)
      return;
    break;

  default:
    PanicAlert("HidOutputReport: Unknown channel 0x%02x", sr->wm);
    return;      break;
  }

    if (send_ack && WIIMOTE_SRC_EMU & g_wiimote_sources[m_index])
    SendAck(sr->wm);
}


void Wiimote::SendAck(u8 report_id)
{
  u8 data[6];

  data[0] = 0xA1;
  data[1] = RT_ACK_DATA;

  wm_acknowledge* ack = reinterpret_cast<wm_acknowledge*>(data + 2);

  ack->buttons = m_status.buttons;
  ack->reportID = report_id;
  ack->errorID = 0;

  Core::Callback_WiimoteInterruptChannel(m_index, m_reporting_channel, data, sizeof(data));
}

void Wiimote::HandleExtensionSwap()
{
    if (m_extension->active_extension != m_extension->switch_extension)
  {
        if ((m_extension->active_extension > 0) && m_extension->switch_extension)
                  m_extension->active_extension = 0;
    else
            m_extension->active_extension = m_extension->switch_extension;

        ((WiimoteEmu::Attachment*)m_extension->attachments[m_extension->active_extension].get())
        ->Reset();
  }
}

void Wiimote::RequestStatus(const wm_request_status* const rs)
{
  HandleExtensionSwap();

    m_status.extension = m_extension->active_extension ? 1 : 0;

    u8 data[8];
  data[0] = 0xA1;
  data[1] = RT_STATUS_REPORT;

    *reinterpret_cast<wm_status_report*>(data + 2) = m_status;

    if (WIIMOTE_SRC_REAL & g_wiimote_sources[m_index] && (m_extension->switch_extension <= 0))
  {
    using namespace WiimoteReal;

    std::lock_guard<std::mutex> lk(g_wiimotes_mutex);

    if (g_wiimotes[m_index])
    {
      wm_request_status rpt = {};
      g_wiimotes[m_index]->QueueReport(RT_REQUEST_STATUS, &rpt, sizeof(rpt));
    }

    return;
  }

    Core::Callback_WiimoteInterruptChannel(m_index, m_reporting_channel, data, sizeof(data));
}


void Wiimote::WriteData(const wm_write_data* const wd)
{
  u32 address = Common::swap24(wd->address);

    address &= ~0x010000;

  if (wd->size > 16)
  {
    PanicAlert("WriteData: size is > 16 bytes");
    return;
  }

  switch (wd->space)
  {
  case WS_EEPROM:
  {
    
    if (address + wd->size > WIIMOTE_EEPROM_SIZE)
    {
      ERROR_LOG(WIIMOTE, "WriteData: address + size out of bounds!");
      PanicAlert("WriteData: address + size out of bounds!");
      return;
    }
    memcpy(m_eeprom + address, wd->data, wd->size);

        if (address >= 0x0FCA && address < 0x12C0)
    {
            std::ofstream file;
      File::OpenFStream(file, File::GetUserPath(D_SESSION_WIIROOT_IDX) + "/mii.bin",
                        std::ios::binary | std::ios::out);
      file.write((char*)m_eeprom + 0x0FCA, 0x02f0);
      file.close();
    }
  }
  break;

  case WS_REGS1:
  case WS_REGS2:
  {
    
        if (0xA4 == (address >> 16))
      address &= 0xFF00FF;

    const u8 region_offset = (u8)address;
    void* region_ptr = nullptr;
    int region_size = 0;

    switch (address >> 16)
    {
        case 0xa2:
      region_ptr = &m_reg_speaker;
      region_size = WIIMOTE_REG_SPEAKER_SIZE;
      break;

        case 0xa4:
      region_ptr = (void*)&m_reg_ext;
      region_size = WIIMOTE_REG_EXT_SIZE;
      break;

        case 0xa6:
      region_ptr = &m_reg_motion_plus;
      region_size = WIIMOTE_REG_EXT_SIZE;
      break;

        case 0xB0:
      region_ptr = &m_reg_ir;
      region_size = WIIMOTE_REG_IR_SIZE;
      break;
    }

    if (region_ptr && (region_offset + wd->size <= region_size))
    {
      memcpy((u8*)region_ptr + region_offset, wd->data, wd->size);
    }
    else
      return;  
    if (&m_reg_ext == region_ptr)
    {
                  if (address >= 0xa40040 && address <= 0xa4004c)
        WiimoteGenerateKey(&m_ext_key, m_reg_ext.encryption_key);
    }
    else if (&m_reg_motion_plus == region_ptr)
    {
            if (0x55 == m_reg_motion_plus.activated)
      {
                m_reg_motion_plus.activated = 0;

        RequestStatus();
      }
    }
  }
  break;

  default:
    PanicAlert("WriteData: unimplemented parameters!");
    break;
  }
}


void Wiimote::ReadData(const wm_read_data* const rd)
{
  u32 address = Common::swap24(rd->address);
  u16 size = Common::swap16(rd->size);

    address &= 0xFEFFFF;

      if (WIIMOTE_SRC_REAL & g_wiimote_sources[m_index] &&
      ((0xA4 != (address >> 16)) || (m_extension->switch_extension <= 0)))
  {
    WiimoteReal::InterruptChannel(m_index, m_reporting_channel, ((u8*)rd) - 2,
                                  sizeof(wm_read_data) + 2);  
        return;
  }

  ReadRequest rr;
  u8* const block = new u8[size];

  switch (rd->space)
  {
  case WS_EEPROM:
  {
        if (address + size >= WIIMOTE_EEPROM_FREE_SIZE)
    {
      if (address + size > WIIMOTE_EEPROM_SIZE)
      {
        PanicAlert("ReadData: address + size out of bounds");
        delete[] block;
        return;
      }
            size = 0;
    }

        if (address >= 0x0FCA && address < 0x12C0)
    {
            std::ifstream file;
      File::OpenFStream(file, (File::GetUserPath(D_SESSION_WIIROOT_IDX) + "/mii.bin").c_str(),
                        std::ios::binary | std::ios::in);
      file.read((char*)m_eeprom + 0x0FCA, 0x02f0);
      file.close();
    }

        memcpy(block, m_eeprom + address, size);
  }
  break;

  case WS_REGS1:
  case WS_REGS2:
  {
    
        if (0xA4 == (address >> 16))
      address &= 0xFF00FF;

    const u8 region_offset = (u8)address;
    void* region_ptr = nullptr;
    int region_size = 0;

    switch (address >> 16)
    {
        case 0xa2:
      region_ptr = &m_reg_speaker;
      region_size = WIIMOTE_REG_SPEAKER_SIZE;
      break;

        case 0xa4:
      region_ptr = (void*)&m_reg_ext;
      region_size = WIIMOTE_REG_EXT_SIZE;
      break;

        case 0xa6:
            region_ptr = &m_reg_motion_plus;
      region_size = WIIMOTE_REG_EXT_SIZE;
      break;

        case 0xb0:
      region_ptr = &m_reg_ir;
      region_size = WIIMOTE_REG_IR_SIZE;
      break;
    }

    if (region_ptr && (region_offset + size <= region_size))
    {
      memcpy(block, (u8*)region_ptr + region_offset, size);
    }
    else
      size = 0;  
    if (&m_reg_ext == region_ptr)
    {
                  if (0xaa == m_reg_ext.encryption)
        WiimoteEncrypt(&m_ext_key, block, address & 0xffff, (u8)size);
    }
  }
  break;

  default:
    PanicAlert("WmReadData: unimplemented parameters (size: %i, address: 0x%x)!", size, rd->space);
    break;
  }

    rr.address = Common::swap24(rd->address);
  rr.size = size;
    rr.position = 0;
  rr.data = block;

    SendReadDataReply(rr);

    if (rr.size)
    m_read_requests.push(rr);
  else
    delete[] rr.data;
}

void Wiimote::SendReadDataReply(ReadRequest& request)
{
  u8 data[23];
  data[0] = 0xA1;
  data[1] = RT_READ_DATA_REPLY;

  wm_read_data_reply* const reply = reinterpret_cast<wm_read_data_reply*>(data + 2);
  reply->buttons = m_status.buttons;
  reply->address = Common::swap16(request.address);

              if (0 == request.size)
  {
    reply->size = 0x0f;
    reply->error = 0x08;

    memset(reply->data, 0, sizeof(reply->data));
  }
  else
  {
            const int amt = std::min((unsigned int)16, request.size);

        reply->error = 0;

        reply->size = amt - 1;

        memset(reply->data, 0, sizeof(reply->data));

        memcpy(reply->data, request.data + request.position, amt);

        request.size -= amt;
    request.position += amt;
    request.address += amt;
  }

    Core::Callback_WiimoteInterruptChannel(m_index, m_reporting_channel, data, sizeof(data));
}

void Wiimote::DoState(PointerWrap& p)
{
  p.Do(m_extension->active_extension);
  p.Do(m_extension->switch_extension);

  p.Do(m_accel);
  p.Do(m_index);
  p.Do(ir_sin);
  p.Do(ir_cos);
  p.Do(m_rumble_on);
  p.Do(m_speaker_mute);
  p.Do(m_reporting_auto);
  p.Do(m_reporting_mode);
  p.Do(m_reporting_channel);
  p.Do(m_shake_step);
  p.Do(m_sensor_bar_on_top);
  p.Do(m_status);
  p.Do(m_adpcm_state);
  p.Do(m_ext_key);
  p.DoArray(m_eeprom);
  p.Do(m_reg_motion_plus);
  p.Do(m_reg_ir);
  p.Do(m_reg_ext);
  p.Do(m_reg_speaker);

    {
    u32 size = 0;
    if (p.mode == PointerWrap::MODE_READ)
    {
            while (!m_read_requests.empty())
      {
        delete[] m_read_requests.front().data;
        m_read_requests.pop();
      }

      p.Do(size);
      while (size--)
      {
        ReadRequest tmp;
        p.Do(tmp.address);
        p.Do(tmp.position);
        p.Do(tmp.size);
        tmp.data = new u8[tmp.size];
        p.DoArray(tmp.data, tmp.size);
        m_read_requests.push(tmp);
      }
    }
    else
    {
      std::queue<ReadRequest> tmp_queue(m_read_requests);
      size = (u32)(m_read_requests.size());
      p.Do(size);
      while (!tmp_queue.empty())
      {
        ReadRequest tmp = tmp_queue.front();
        p.Do(tmp.address);
        p.Do(tmp.position);
        p.Do(tmp.size);
        p.DoArray(tmp.data, tmp.size);
        tmp_queue.pop();
      }
    }
  }
  p.DoMarker("Wiimote");

  if (p.GetMode() == PointerWrap::MODE_READ)
    RealState();
}

void Wiimote::RealState()
{
  using namespace WiimoteReal;

  if (g_wiimotes[m_index])
  {
    g_wiimotes[m_index]->SetChannel(m_reporting_channel);
    g_wiimotes[m_index]->EnableDataReporting(m_reporting_mode);
  }
}
}
