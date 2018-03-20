
#pragma once

#include <array>

#include "Common/CommonTypes.h"
#include "Common/MathUtil.h"
#include "Core/HW/DSPHLE/UCodes/UCodes.h"

namespace DSP
{
namespace HLE
{
class DSPHLE;

class ZeldaAudioRenderer
{
public:
  void PrepareFrame();
  void AddVoice(u16 voice_id);
  void FinalizeFrame();

  void SetFlags(u32 flags) { m_flags = flags; }
  void SetSineTable(std::array<s16, 0x80>&& sine_table) { m_sine_table = sine_table; }
  void SetConstPatterns(std::array<s16, 0x100>&& patterns) { m_const_patterns = patterns; }
  void SetResamplingCoeffs(std::array<s16, 0x100>&& coeffs) { m_resampling_coeffs = coeffs; }
  void SetAfcCoeffs(std::array<s16, 0x20>&& coeffs) { m_afc_coeffs = coeffs; }
  void SetVPBBaseAddress(u32 addr) { m_vpb_base_addr = addr; }
  void SetReverbPBBaseAddress(u32 addr) { m_reverb_pb_base_addr = addr; }
  void SetOutputVolume(u16 volume) { m_output_volume = volume; }
  void SetOutputLeftBufferAddr(u32 addr) { m_output_lbuf_addr = addr; }
  void SetOutputRightBufferAddr(u32 addr) { m_output_rbuf_addr = addr; }
  void SetARAMBaseAddr(u32 addr) { m_aram_base_addr = addr; }
  void DoState(PointerWrap& p);

private:
  struct VPB;

    u32 m_flags;

  
      template <size_t N, size_t B>
  void ApplyVolumeInPlace(std::array<s16, N>* buf, u16 vol)
  {
    for (size_t i = 0; i < N; ++i)
    {
      s32 tmp = (u32)(*buf)[i] * (u32)vol;
      tmp >>= 16 - B;

      (*buf)[i] = (s16)MathUtil::Clamp(tmp, -0x8000, 0x7FFF);
    }
  }
  template <size_t N>
  void ApplyVolumeInPlace_1_15(std::array<s16, N>* buf, u16 vol)
  {
    ApplyVolumeInPlace<N, 1>(buf, vol);
  }
  template <size_t N>
  void ApplyVolumeInPlace_4_12(std::array<s16, N>* buf, u16 vol)
  {
    ApplyVolumeInPlace<N, 4>(buf, vol);
  }

            template <size_t N>
  s32 AddBuffersWithVolumeRamp(std::array<s16, N>* dst, const std::array<s16, N>& src, s32 vol,
                               s32 step)
  {
    if (!vol && !step)
      return vol;

    for (size_t i = 0; i < N; ++i)
    {
      (*dst)[i] += ((vol >> 16) * src[i]) >> 16;
      vol += step;
    }

    return vol;
  }

      void AddBuffersWithVolume(s16* dst, const s16* src, size_t count, u16 vol)
  {
    while (count--)
    {
      s32 vol_src = ((s32)*src++ * (s32)vol) >> 15;
      *dst++ += MathUtil::Clamp(vol_src, -0x8000, 0x7FFF);
    }
  }

    bool m_prepared = false;

    u32 m_output_lbuf_addr = 0;
  u32 m_output_rbuf_addr = 0;

    u16 m_output_volume = 0;

    typedef std::array<s16, 0x50> MixingBuffer;
  MixingBuffer m_buf_front_left{};
  MixingBuffer m_buf_front_right{};
  MixingBuffer m_buf_back_left{};
  MixingBuffer m_buf_back_right{};
  MixingBuffer m_buf_front_left_reverb{};
  MixingBuffer m_buf_front_right_reverb{};
  MixingBuffer m_buf_back_left_reverb{};
  MixingBuffer m_buf_back_right_reverb{};
  MixingBuffer m_buf_unk0_reverb{};
  MixingBuffer m_buf_unk1_reverb{};
  MixingBuffer m_buf_unk0{};
  MixingBuffer m_buf_unk1{};
  MixingBuffer m_buf_unk2{};

      MixingBuffer* BufferForID(u16 buffer_id);

    u32 m_vpb_base_addr;
  void FetchVPB(u16 voice_id, VPB* vpb);
  void StoreVPB(u16 voice_id, VPB* vpb);

      std::array<s16, 0x80> m_sine_table{};

    std::array<s16, 0x100> m_const_patterns{};

      void LoadInputSamples(MixingBuffer* buffer, VPB* vpb);

      u16 NeededRawSamplesCount(const VPB& vpb);

      void Resample(VPB* vpb, const s16* src, MixingBuffer* dst);

    std::array<s16, 0x100> m_resampling_coeffs{};

        u32 m_aram_base_addr = 0;
  void* GetARAMPtr() const;

      template <typename T>
  void DownloadPCMSamplesFromARAM(s16* dst, VPB* vpb, u16 requested_samples_count);

      void DownloadAFCSamplesFromARAM(s16* dst, VPB* vpb, u16 requested_samples_count);
  void DecodeAFC(VPB* vpb, s16* dst, size_t block_count);
  std::array<s16, 0x20> m_afc_coeffs{};

      void DownloadRawSamplesFromMRAM(s16* dst, VPB* vpb, u16 requested_samples_count);

        void ApplyReverb(bool post_rendering);
  std::array<u16, 4> m_reverb_pb_frames_count{};
  std::array<s16, 8> m_buf_unk0_reverb_last8{};
  std::array<s16, 8> m_buf_unk1_reverb_last8{};
  std::array<s16, 8> m_buf_front_left_reverb_last8{};
  std::array<s16, 8> m_buf_front_right_reverb_last8{};
  u32 m_reverb_pb_base_addr = 0;
};

class ZeldaUCode : public UCodeInterface
{
public:
  ZeldaUCode(DSPHLE* dsphle, u32 crc);
  virtual ~ZeldaUCode();

  void Initialize() override;
  void HandleMail(u32 mail) override;
  void Update() override;

  void DoState(PointerWrap& p) override;

private:
      u32 m_flags;

    void HandleMailDefault(u32 mail);
  void HandleMailLight(u32 mail);

                enum class MailState : u32
  {
    WAITING,
    RENDERING,
    WRITING_CMD,
    HALTED,
  };
  MailState m_mail_current_state = MailState::WAITING;
  u32 m_mail_expected_cmd_mails = 0;

      void SetMailState(MailState new_state);

            u32 m_sync_max_voice_id = 0;
  std::array<u16, 256> m_sync_voice_skip_flags{};
  bool m_sync_flags_second_half = false;

        std::array<u32, 64> m_cmd_buffer{};
  u32 m_read_offset = 0;
  u32 m_write_offset = 0;
  u32 m_pending_commands_count = 0;
  bool m_cmd_can_execute = true;

    u32 Read32();

    void Write32(u32 val);

            void RunPendingCommands();

    enum class CommandAck : u32
  {
    STANDARD,
    DONE_RENDERING,
  };
  void SendCommandAck(CommandAck ack_type, u16 sync_value);

    u32 m_rendering_requested_frames = 0;
  u16 m_rendering_voices_per_frame = 0;
  u32 m_rendering_curr_frame = 0;
  u32 m_rendering_curr_voice = 0;

  bool RenderingInProgress() const
  {
    return m_rendering_curr_frame != m_rendering_requested_frames;
  }
  void RenderAudio();

    ZeldaAudioRenderer m_renderer;
};
}  }  