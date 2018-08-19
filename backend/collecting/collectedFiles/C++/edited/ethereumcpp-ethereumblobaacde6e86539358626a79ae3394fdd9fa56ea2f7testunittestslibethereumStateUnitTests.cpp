

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

class AddressRangeTestFixture : public TestOutputHelperFixture
{
public:
    AddressRangeTestFixture()
    {
                for (unsigned i = 0; i < addressCount; ++i)
        {
            Address addr{i};
            hashToAddress[sha3(addr)] = addr;
        }

                for (auto const& hashAndAddr : hashToAddress)
            state.addBalance(hashAndAddr.second, 100);
        state.commit(State::CommitBehaviour::RemoveEmptyAccounts);
    }

    State state{0};
    unsigned const addressCount = 10;
    std::map<h256, Address> hashToAddress;
};

BOOST_FIXTURE_TEST_SUITE(StateAddressRangeTests, AddressRangeTestFixture)


BOOST_AUTO_TEST_CASE(addressesReturnsAllAddresses)
{
    std::pair<State::AddressMap, h256> addressesAndNextKey =
        state.addresses(h256{}, addressCount * 2);
    State::AddressMap addresses = addressesAndNextKey.first;

    BOOST_CHECK_EQUAL(addresses.size(), addressCount);
    BOOST_CHECK(addresses == hashToAddress);
    BOOST_CHECK_EQUAL(addressesAndNextKey.second, h256{});
}

BOOST_AUTO_TEST_CASE(addressesReturnsNoMoreThanRequested)
{
    int maxResults = 3;
    std::pair<State::AddressMap, h256> addressesAndNextKey = state.addresses(h256{}, maxResults);
    State::AddressMap& addresses = addressesAndNextKey.first;
    h256& nextKey = addressesAndNextKey.second;

    BOOST_CHECK_EQUAL(addresses.size(), maxResults);
    auto itHashToAddressEnd = std::next(hashToAddress.begin(), maxResults);
    BOOST_CHECK(addresses == State::AddressMap(hashToAddress.begin(), itHashToAddressEnd));
    BOOST_CHECK_EQUAL(nextKey, itHashToAddressEnd->first);

        std::pair<State::AddressMap, h256> addressesAndNextKey2 = state.addresses(nextKey, maxResults);
    State::AddressMap& addresses2 = addressesAndNextKey2.first;
    BOOST_CHECK_EQUAL(addresses2.size(), maxResults);
    auto itHashToAddressEnd2 = std::next(itHashToAddressEnd, maxResults);
    BOOST_CHECK(addresses2 == State::AddressMap(itHashToAddressEnd, itHashToAddressEnd2));
}

BOOST_AUTO_TEST_CASE(addressesDoesntReturnDeletedInCache)
{
        unsigned deleteCount = 3;
    auto it = hashToAddress.begin();
    for (unsigned i = 0; i < deleteCount; ++i, ++it)
        state.kill(it->second);
    
    std::pair<State::AddressMap, h256> addressesAndNextKey =
        state.addresses(h256{}, addressCount * 2);
    State::AddressMap& addresses = addressesAndNextKey.first;
    BOOST_CHECK_EQUAL(addresses.size(), addressCount - deleteCount);
    BOOST_CHECK(addresses == State::AddressMap(it, hashToAddress.end()));
}

BOOST_AUTO_TEST_CASE(addressesReturnsCreatedInCache)
{
        unsigned createCount = 3;
    std::map<h256, Address> newHashToAddress;
    for (unsigned i = addressCount; i < addressCount + createCount; ++i)
    {
        Address addr{i};
        newHashToAddress[sha3(addr)] = addr;
    }

        for (auto const& hashAndAddr : newHashToAddress)
        state.addBalance(hashAndAddr.second, 100);
    
    std::pair<State::AddressMap, h256> addressesAndNextKey =
        state.addresses(newHashToAddress.begin()->first, addressCount + createCount);
    State::AddressMap& addresses = addressesAndNextKey.first;
    for (auto const& hashAndAddr : newHashToAddress)
        BOOST_CHECK(addresses.find(hashAndAddr.first) != addresses.end());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

}
}
