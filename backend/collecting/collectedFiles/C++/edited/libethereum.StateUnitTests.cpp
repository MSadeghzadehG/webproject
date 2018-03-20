

#include <test/tools/libtesteth/TestHelper.h>
#include <libethereum/BlockChain.h>
#include <libethereum/Block.h>
#include <libethcore/BasicAuthority.h>
#include <libethereum/Defaults.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

namespace dev
{
namespace test
{

BOOST_FIXTURE_TEST_SUITE(StateUnitTests, TestOutputHelperFixture)

BOOST_AUTO_TEST_CASE(Basic)
{
	Block s(Block::Null);
}

BOOST_AUTO_TEST_CASE(LoadAccountCode)
{
	Address addr{"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};
	State s{0};
	s.createContract(addr);
	uint8_t codeData[] = {'c', 'o', 'd', 'e'};
	s.setCode(addr, {std::begin(codeData), std::end(codeData)});
	s.commit(State::CommitBehaviour::RemoveEmptyAccounts);

	auto& loadedCode = s.code(addr);
	BOOST_CHECK(std::equal(
			std::begin(codeData), std::end(codeData), std::begin(loadedCode)
	));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
