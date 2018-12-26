
#include <gtest/gtest.h>

#include "TestFixture.h"

namespace glslangtest {
namespace {

using PreprocessingTest = GlslangTest<::testing::TestWithParam<std::string>>;

TEST_P(PreprocessingTest, FromFile)
{
    loadFilePreprocessAndCheck(GLSLANG_TEST_DIRECTORY, GetParam());
}

INSTANTIATE_TEST_CASE_P(
    Glsl, PreprocessingTest,
    ::testing::ValuesIn(std::vector<std::string>({
        "preprocessor.cpp_style_line_directive.vert",
        "preprocessor.cpp_style___FILE__.vert",
        "preprocessor.edge_cases.vert",
        "preprocessor.errors.vert",
        "preprocessor.extensions.vert",
        "preprocessor.function_macro.vert",
        "preprocessor.include.enabled.vert",
        "preprocessor.include.disabled.vert",
        "preprocessor.line.vert",
        "preprocessor.line.frag",
        "preprocessor.pragma.vert",
        "preprocessor.simple.vert",
        "preprocessor.success_if_parse_would_fail.vert",
        "preprocessor.defined.vert",
        "preprocessor.many.endif.vert",
        "preprocessor.eof_missing.vert",
    })),
    FileNameAsCustomTestSuffix
);

}  }  