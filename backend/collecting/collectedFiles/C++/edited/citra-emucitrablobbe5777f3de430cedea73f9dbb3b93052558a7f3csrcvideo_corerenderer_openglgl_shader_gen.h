
#pragma once

#include <array>
#include <cstring>
#include <functional>
#include <string>
#include <type_traits>
#include <boost/optional.hpp>
#include "common/hash.h"
#include "video_core/regs.h"
#include "video_core/shader/shader.h"

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

struct PicaFSConfigState {
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
            bool shadow_enable;
        } light[8];

        bool enable;
        unsigned src_num;
        Pica::LightingRegs::LightingBumpMode bump_mode;
        unsigned bump_selector;
        bool bump_renorm;
        bool clamp_highlights;

        Pica::LightingRegs::LightingConfig config;
        bool enable_primary_alpha;
        bool enable_secondary_alpha;

        bool enable_shadow;
        bool shadow_primary;
        bool shadow_secondary;
        bool shadow_invert;
        bool shadow_alpha;
        unsigned shadow_selector;

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
};


struct PicaFSConfig : Common::HashableStruct<PicaFSConfigState> {

        static PicaFSConfig BuildFromRegs(const Pica::Regs& regs);

    bool TevStageUpdatesCombinerBufferColor(unsigned stage_index) const {
        return (stage_index < 4) && (state.combiner_buffer_input & (1 << stage_index));
    }

    bool TevStageUpdatesCombinerBufferAlpha(unsigned stage_index) const {
        return (stage_index < 4) && ((state.combiner_buffer_input >> 4) & (1 << stage_index));
    }
};


struct PicaShaderConfigCommon {
    void Init(const Pica::ShaderRegs& regs, Pica::Shader::ShaderSetup& setup);

    u64 program_hash;
    u64 swizzle_hash;
    u32 main_offset;
    bool sanitize_mul;

    u32 num_outputs;

        std::array<u32, 16> output_map;
};


struct PicaVSConfig : Common::HashableStruct<PicaShaderConfigCommon> {
    explicit PicaVSConfig(const Pica::Regs& regs, Pica::Shader::ShaderSetup& setup) {
        state.Init(regs.vs, setup);
    }
};

struct PicaGSConfigCommonRaw {
    void Init(const Pica::Regs& regs);

    u32 vs_output_attributes;
    u32 gs_output_attributes;

    struct SemanticMap {
        u32 attribute_index;
        u32 component_index;
    };

        std::array<SemanticMap, 24> semantic_maps;
};


struct PicaFixedGSConfig : Common::HashableStruct<PicaGSConfigCommonRaw> {
    explicit PicaFixedGSConfig(const Pica::Regs& regs) {
        state.Init(regs);
    }
};

struct PicaGSConfigRaw : PicaShaderConfigCommon, PicaGSConfigCommonRaw {
    void Init(const Pica::Regs& regs, Pica::Shader::ShaderSetup& setup);

    u32 num_inputs;
    u32 attributes_per_vertex;

        std::array<u32, 16> input_map;
};


struct PicaGSConfig : Common::HashableStruct<PicaGSConfigRaw> {
    explicit PicaGSConfig(const Pica::Regs& regs, Pica::Shader::ShaderSetup& setups) {
        state.Init(regs, setups);
    }
};


std::string GenerateTrivialVertexShader(bool separable_shader);


boost::optional<std::string> GenerateVertexShader(const Pica::Shader::ShaderSetup& setup,
                                                  const PicaVSConfig& config,
                                                  bool separable_shader);


std::string GenerateFixedGeometryShader(const PicaFixedGSConfig& config, bool separable_shader);


boost::optional<std::string> GenerateGeometryShader(const Pica::Shader::ShaderSetup& setup,
                                                    const PicaGSConfig& config,
                                                    bool separable_shader);


std::string GenerateFragmentShader(const PicaFSConfig& config, bool separable_shader);

} 
namespace std {
template <>
struct hash<GLShader::PicaFSConfig> {
    size_t operator()(const GLShader::PicaFSConfig& k) const {
        return k.Hash();
    }
};

template <>
struct hash<GLShader::PicaVSConfig> {
    size_t operator()(const GLShader::PicaVSConfig& k) const {
        return k.Hash();
    }
};

template <>
struct hash<GLShader::PicaFixedGSConfig> {
    size_t operator()(const GLShader::PicaFixedGSConfig& k) const {
        return k.Hash();
    }
};

template <>
struct hash<GLShader::PicaGSConfig> {
    size_t operator()(const GLShader::PicaGSConfig& k) const {
        return k.Hash();
    }
};
} 