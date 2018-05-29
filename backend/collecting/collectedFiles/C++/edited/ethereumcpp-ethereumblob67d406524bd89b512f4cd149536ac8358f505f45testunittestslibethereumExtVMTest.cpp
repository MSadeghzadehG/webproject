

#include <test/tools/libtesteth/BlockChainHelper.h>
#include <test/tools/libtesteth/TestHelper.h>
#include <test/tools/libtestutils/TestLastBlockHashes.h>

#include <libethereum/Block.h>
#include <libethereum/ExtVM.h>

using namespace dev;
using namespace dev::eth;
using namespace dev::test;

class ExtVMTestFixture : public TestOutputHelperFixture
{
public:
    ExtVMTestFixture()
      : networkSelector(eth::Network::ConstantinopleTransitionTest),
        testBlockchain(TestBlockChain::defaultGenesisBlock()),
        genesisBlock(testBlockchain.testGenesis()),
        genesisDB(genesisBlock.state().db()),
        blockchain(testBlockchain.getInterface())
    {
        TestBlock testBlock;
                testBlock.mine(testBlockchain);
        testBlockchain.addBlock(testBlock);

                testBlock.mine(testBlockchain);
        testBlockchain.addBlock(testBlock);
    }

    NetworkSelector networkSelector;
    TestBlockChain testBlockchain;
    TestBlock const& genesisBlock;
    OverlayDB const& genesisDB;
    BlockChain const& blockchain;
};

BOOST_FIXTURE_TEST_SUITE(ExtVmSuite, ExtVMTestFixture)

BOOST_AUTO_TEST_CASE(BlockhashOutOfBoundsRetunsZero)
{
    Block block = blockchain.genesisBlock(genesisDB);
    block.sync(blockchain);

    TestLastBlockHashes lastBlockHashes({});
    EnvInfo envInfo(block.info(), lastBlockHashes, 0);
    Address addr("0xa94f5374fce5edbc8e2a8697c15331677e6ebf0b");
    ExtVM extVM(block.mutableState(), envInfo, *blockchain.sealEngine(), addr, addr, addr, 0, 0, {},
        {}, {}, 0, false, false);

    BOOST_CHECK_EQUAL(extVM.blockHash(100), h256());
}

BOOST_AUTO_TEST_CASE(BlockhashBeforeConstantinopleReliesOnLastHashes)
{
    Block block = blockchain.genesisBlock(genesisDB);
    block.sync(blockchain);

    h256s lastHashes{h256("0xaaabbbccc"), h256("0xdddeeefff")};
    TestLastBlockHashes lastBlockHashes(lastHashes);
    EnvInfo envInfo(block.info(), lastBlockHashes, 0);
    Address addr("0xa94f5374fce5edbc8e2a8697c15331677e6ebf0b");
    ExtVM extVM(block.mutableState(), envInfo, *blockchain.sealEngine(), addr, addr, addr, 0, 0, {},
        {}, {}, 0, false, false);
    h256 hash = extVM.blockHash(1);
    BOOST_REQUIRE_EQUAL(hash, lastHashes[0]);
}

BOOST_AUTO_TEST_CASE(BlockhashDoesntNeedLastHashesInConstantinople)
{
            TestBlock testBlock;
    for (int i = 0; i < 256; ++i)
    {
        testBlock.mine(testBlockchain);
        testBlockchain.addBlock(testBlock);
    }

    Block block = blockchain.genesisBlock(genesisDB);
    block.sync(blockchain);

    TestLastBlockHashes lastBlockHashes({});
    EnvInfo envInfo(block.info(), lastBlockHashes, 0);
    Address addr("0xa94f5374fce5edbc8e2a8697c15331677e6ebf0b");
    ExtVM extVM(block.mutableState(), envInfo, *blockchain.sealEngine(), addr, addr, addr, 0, 0, {},
        {}, {}, 0, false, false);

        BOOST_CHECK_EQUAL(extVM.blockHash(1), h256());

    h256 hash = extVM.blockHash(200);
    BOOST_REQUIRE_EQUAL(hash, blockchain.numberHash(200));
}


BOOST_AUTO_TEST_SUITE_END()
