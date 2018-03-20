
#pragma once

#include "VMFace.h"
#include <boost/program_options/options_description.hpp>

namespace dev
{
namespace eth
{
enum class VMKind
{
    Interpreter,
    JIT,
    Hera,
    Legacy,
};

std::vector<std::pair<std::string, std::string>>& evmcOptions() noexcept;

boost::program_options::options_description vmProgramOptions(
    unsigned _lineLength = boost::program_options::options_description::m_default_line_length);

class VMFactory
{
public:
	VMFactory() = delete;
	~VMFactory() = delete;

		static std::unique_ptr<VMFace> create();

		static std::unique_ptr<VMFace> create(VMKind _kind);

		static void setKind(VMKind _kind);
};

}
}
