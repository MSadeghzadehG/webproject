
#pragma once

#include <array>
#include <cstring>
#include <functional>
#include <string>
#include <type_traits>
#include "video_core/regs.h"

namespace GLShader {

enum Attributes {
    ATTRIBUTE_POSITION,
    ATTRIBUTE_COLOR,
    ATTRIBUTE_TEXCOORD0,
    ATTRIBUTE_TEXCOORD1,
    ATTRIBUTE_TEXCOORD2,
    ATTRIBUTE_TEXCOORD0_W,
    ATTRIBUTE_NORMQUAT,
    ATTRIBUTE_VIEW,
};


union PicaShaderConfig {

        static PicaShaderConfig BuildFromRegs(const Pica::Regs& regs);

    bool TevStageUpdatesCombinerBufferColor(unsigned stage_index) const {
        return (stage_index < 4) && (state.combiner_buffer_input & (1 << stage_index));
    }

    bool TevStageUpdatesCombinerBufferAlpha(unsigned stage_index) const {
        return (stage_index < 4) && ((state.combiner_buffer_input >> 4) & (1 << stage_index));
    }

    bool operator==(const PicaShaderConfig& o) const {
        return std::memcmp(&state, &o.state, sizeof(PicaShaderConfig::State)) == 0;
    };

                        struct TevStageConfigRaw {
        u32 sources_raw;
        u32 modifiers_raw;
        u32 ops_raw;
        u32 scales_raw;
        explicit operator Pica::TexturingRegs::TevStageConfig() const noexcept {
            Pica::TexturingRegs::TevStageConfig stage;
            stage.sources_raw = sources_raw;
            stage.modifiers_raw = modifiers_raw;
            stage.ops_raw = ops_raw;
            stage.const_color = 0;
            stage.scales_raw = scales_raw;
            return stage;
        }
    };

    struct State {
        Pica::FramebufferRegs::CompareFunc alpha_test_func;
        Pica::RasterizerRegs::ScissorMode scissor_test_mode;
        Pica::TexturingRegs::TextureConfig::TextureType texture0_type;
        bool texture2_use_coord1;
        std::array<TevStageConfigRaw, 6> tev_stages;
        u8 combiner_buffer_input;

        Pica::RasterizerRegs::DepthBuffering depthmap_enable;
        Pica::TexturingRegs::FogMode fog_mode;
        bool fog_flip;

        struct {
            struct {
                unsigned num;
                bool directional;
                bool two_sided_diffuse;
                bool dist_atten_enable;
                bool spot_atten_enable;
                bool geometric_factor_0;
                bool geometric_factor_1;
            } light[8];

            bool enable;
            unsigned src_num;
            Pica::LightingRegs::LightingBumpMode bump_mode;
            unsigned bump_selector;
            bool bump_renorm;
            bool clamp_highlights;

            Pica::LightingRegs::LightingConfig config;
            Pica::LightingRegs::LightingFresnelSelector fresnel_selector;

            struct {
                bool enable;
                bool abs_input;
                Pica::LightingRegs::LightingLutInput type;
                float scale;
            } lut_d0, lut_d1, lut_sp, lut_fr, lut_rr, lut_rg, lut_rb;
        } lighting;

        struct {
            bool enable;
            u32 coord;
            Pica::TexturingRegs::ProcTexClamp u_clamp, v_clamp;
            Pica::TexturingRegs::ProcTexCombiner color_combiner, alpha_combiner;
            bool separate_alpha;
            bool noise_enable;
            Pica::TexturingRegs::ProcTexShift u_shift, v_shift;
            u32 lut_width;
            u32 lut_offset;
            Pica::TexturingRegs::ProcTexFilter lut_filter;
        } proctex;

    } state;
};


std::string GenerateVertexShader();


std::string GenerateFragmentShader(const PicaShaderConfig& config);

} 
namespace std {
template <>
struct hash<GLShader::PicaShaderConfig> {
    size_t operator()(const GLShader::PicaShaderConfig& k) const {
        return Common::ComputeStructHash64(k.state);
    }
};
} 