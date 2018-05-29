#ifndef _VERSIONS_INCLUDED_
#define _VERSIONS_INCLUDED_


typedef enum {
    EBadProfile           = 0,
    ENoProfile            = (1 << 0),     ECoreProfile          = (1 << 1),
    ECompatibilityProfile = (1 << 2),
    EEsProfile            = (1 << 3)
} EProfile;

namespace glslang {

inline const char* ProfileName(EProfile profile)
{
    switch (profile) {
    case ENoProfile:             return "none";
    case ECoreProfile:           return "core";
    case ECompatibilityProfile:  return "compatibility";
    case EEsProfile:             return "es";
    default:                     return "unknown profile";
    }
}

struct SpvVersion {
    SpvVersion() : spv(0), vulkan(0), openGl(0) {}
    unsigned int spv;     int vulkan;           int openGl;       };

typedef enum {
    EBhMissing = 0,
    EBhRequire,
    EBhEnable,
    EBhWarn,
    EBhDisable,
    EBhDisablePartial    } TExtensionBehavior;

const char* const E_GL_OES_texture_3D                   = "GL_OES_texture_3D";
const char* const E_GL_OES_standard_derivatives         = "GL_OES_standard_derivatives";
const char* const E_GL_EXT_frag_depth                   = "GL_EXT_frag_depth";
const char* const E_GL_OES_EGL_image_external           = "GL_OES_EGL_image_external";
const char* const E_GL_EXT_shader_texture_lod           = "GL_EXT_shader_texture_lod";

const char* const E_GL_ARB_texture_rectangle            = "GL_ARB_texture_rectangle";
const char* const E_GL_3DL_array_objects                = "GL_3DL_array_objects";
const char* const E_GL_ARB_shading_language_420pack     = "GL_ARB_shading_language_420pack";
const char* const E_GL_ARB_texture_gather               = "GL_ARB_texture_gather";
const char* const E_GL_ARB_gpu_shader5                  = "GL_ARB_gpu_shader5";
const char* const E_GL_ARB_separate_shader_objects      = "GL_ARB_separate_shader_objects";
const char* const E_GL_ARB_compute_shader               = "GL_ARB_compute_shader";
const char* const E_GL_ARB_tessellation_shader          = "GL_ARB_tessellation_shader";
const char* const E_GL_ARB_enhanced_layouts             = "GL_ARB_enhanced_layouts";
const char* const E_GL_ARB_texture_cube_map_array       = "GL_ARB_texture_cube_map_array";
const char* const E_GL_ARB_shader_texture_lod           = "GL_ARB_shader_texture_lod";
const char* const E_GL_ARB_explicit_attrib_location     = "GL_ARB_explicit_attrib_location";
const char* const E_GL_ARB_shader_image_load_store      = "GL_ARB_shader_image_load_store";
const char* const E_GL_ARB_shader_atomic_counters       = "GL_ARB_shader_atomic_counters";
const char* const E_GL_ARB_shader_draw_parameters       = "GL_ARB_shader_draw_parameters";
const char* const E_GL_ARB_shader_group_vote            = "GL_ARB_shader_group_vote";
const char* const E_GL_ARB_derivative_control           = "GL_ARB_derivative_control";
const char* const E_GL_ARB_shader_texture_image_samples = "GL_ARB_shader_texture_image_samples";
const char* const E_GL_ARB_viewport_array               = "GL_ARB_viewport_array";
const char* const E_GL_ARB_gpu_shader_int64             = "GL_ARB_gpu_shader_int64";
const char* const E_GL_ARB_shader_ballot                = "GL_ARB_shader_ballot";
const char* const E_GL_ARB_sparse_texture2              = "GL_ARB_sparse_texture2";
const char* const E_GL_ARB_sparse_texture_clamp         = "GL_ARB_sparse_texture_clamp";

const char* const E_GL_EXT_shader_non_constant_global_initializers = "GL_EXT_shader_non_constant_global_initializers";

const char* const E_GL_GOOGLE_cpp_style_line_directive          = "GL_GOOGLE_cpp_style_line_directive";
const char* const E_GL_GOOGLE_include_directive                 = "GL_GOOGLE_include_directive";

const char* const E_GL_ANDROID_extension_pack_es31a             = "GL_ANDROID_extension_pack_es31a";
const char* const E_GL_KHR_blend_equation_advanced              = "GL_KHR_blend_equation_advanced";
const char* const E_GL_OES_sample_variables                     = "GL_OES_sample_variables";
const char* const E_GL_OES_shader_image_atomic                  = "GL_OES_shader_image_atomic";
const char* const E_GL_OES_shader_multisample_interpolation     = "GL_OES_shader_multisample_interpolation";
const char* const E_GL_OES_texture_storage_multisample_2d_array = "GL_OES_texture_storage_multisample_2d_array";
const char* const E_GL_EXT_geometry_shader                      = "GL_EXT_geometry_shader";
const char* const E_GL_EXT_geometry_point_size                  = "GL_EXT_geometry_point_size";
const char* const E_GL_EXT_gpu_shader5                          = "GL_EXT_gpu_shader5";
const char* const E_GL_EXT_primitive_bounding_box               = "GL_EXT_primitive_bounding_box";
const char* const E_GL_EXT_shader_io_blocks                     = "GL_EXT_shader_io_blocks";
const char* const E_GL_EXT_tessellation_shader                  = "GL_EXT_tessellation_shader";
const char* const E_GL_EXT_tessellation_point_size              = "GL_EXT_tessellation_point_size";
const char* const E_GL_EXT_texture_buffer                       = "GL_EXT_texture_buffer";
const char* const E_GL_EXT_texture_cube_map_array               = "GL_EXT_texture_cube_map_array";

const char* const E_GL_OES_geometry_shader                      = "GL_OES_geometry_shader";
const char* const E_GL_OES_geometry_point_size                  = "GL_OES_geometry_point_size";
const char* const E_GL_OES_gpu_shader5                          = "GL_OES_gpu_shader5";
const char* const E_GL_OES_primitive_bounding_box               = "GL_OES_primitive_bounding_box";
const char* const E_GL_OES_shader_io_blocks                     = "GL_OES_shader_io_blocks";
const char* const E_GL_OES_tessellation_shader                  = "GL_OES_tessellation_shader";
const char* const E_GL_OES_tessellation_point_size              = "GL_OES_tessellation_point_size";
const char* const E_GL_OES_texture_buffer                       = "GL_OES_texture_buffer";
const char* const E_GL_OES_texture_cube_map_array               = "GL_OES_texture_cube_map_array";


const char* const AEP_geometry_shader[] = { E_GL_EXT_geometry_shader, E_GL_OES_geometry_shader };
const int Num_AEP_geometry_shader = sizeof(AEP_geometry_shader)/sizeof(AEP_geometry_shader[0]);

const char* const AEP_geometry_point_size[] = { E_GL_EXT_geometry_point_size, E_GL_OES_geometry_point_size };
const int Num_AEP_geometry_point_size = sizeof(AEP_geometry_point_size)/sizeof(AEP_geometry_point_size[0]);

const char* const AEP_gpu_shader5[] = { E_GL_EXT_gpu_shader5, E_GL_OES_gpu_shader5 };
const int Num_AEP_gpu_shader5 = sizeof(AEP_gpu_shader5)/sizeof(AEP_gpu_shader5[0]);

const char* const AEP_primitive_bounding_box[] = { E_GL_EXT_primitive_bounding_box, E_GL_OES_primitive_bounding_box };
const int Num_AEP_primitive_bounding_box = sizeof(AEP_primitive_bounding_box)/sizeof(AEP_primitive_bounding_box[0]);

const char* const AEP_shader_io_blocks[] = { E_GL_EXT_shader_io_blocks, E_GL_OES_shader_io_blocks };
const int Num_AEP_shader_io_blocks = sizeof(AEP_shader_io_blocks)/sizeof(AEP_shader_io_blocks[0]);

const char* const AEP_tessellation_shader[] = { E_GL_EXT_tessellation_shader, E_GL_OES_tessellation_shader };
const int Num_AEP_tessellation_shader = sizeof(AEP_tessellation_shader)/sizeof(AEP_tessellation_shader[0]);

const char* const AEP_tessellation_point_size[] = { E_GL_EXT_tessellation_point_size, E_GL_OES_tessellation_point_size };
const int Num_AEP_tessellation_point_size = sizeof(AEP_tessellation_point_size)/sizeof(AEP_tessellation_point_size[0]);

const char* const AEP_texture_buffer[] = { E_GL_EXT_texture_buffer, E_GL_OES_texture_buffer };
const int Num_AEP_texture_buffer = sizeof(AEP_texture_buffer)/sizeof(AEP_texture_buffer[0]);

const char* const AEP_texture_cube_map_array[] = { E_GL_EXT_texture_cube_map_array, E_GL_OES_texture_cube_map_array };
const int Num_AEP_texture_cube_map_array = sizeof(AEP_texture_cube_map_array)/sizeof(AEP_texture_cube_map_array[0]);

} 
#endif 