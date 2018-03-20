


#pragma once
#include <test/tools/libtesteth/TestSuite.h>
#include <boost/filesystem/path.hpp>

namespace dev
{
namespace test
{

class StateTestSuite: public TestSuite
{
public:
	json_spirit::mValue doTests(json_spirit::mValue const& _input, bool _fillin) const override;
	boost::filesystem::path suiteFolder() const override;
	boost::filesystem::path suiteFillerFolder() const override;
};

}
}
