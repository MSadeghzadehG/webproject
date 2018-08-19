


#include "Test.h"
#include <jsonrpccpp/common/errors.h>
#include <jsonrpccpp/common/exception.h>
#include <libdevcore/CommonJS.h>
#include <libethereum/ChainParams.h>
#include <libethereum/ClientTest.h>

using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace dev::rpc;
using namespace jsonrpc;

Test::Test(eth::Client& _eth): m_eth(_eth) {}

namespace
{
string logEntriesToLogHash(eth::LogEntries const& _logs)
{
    RLPStream s;
    s.appendList(_logs.size());
    for (eth::LogEntry const& l : _logs)
        l.streamRLP(s);
    return toJS(sha3(s.out()));
}
}

string Test::test_getLogHash(string const& _txHash)
{
    try
    {
        h256 txHash;
        try
        {
            txHash = h256(_txHash);
        }
        catch (BadHexCharacter const&)
        {
            throw JsonRpcException(Errors::ERROR_RPC_INVALID_PARAMS);
        }

        if (m_eth.blockChain().isKnownTransaction(txHash))
        {
            LocalisedTransaction t = m_eth.localisedTransaction(txHash);
            BlockReceipts const& blockReceipts = m_eth.blockChain().receipts(t.blockHash());
            if (blockReceipts.receipts.size() != 0)
                return logEntriesToLogHash(blockReceipts.receipts[t.transactionIndex()].log());
        }
        return toJS(dev::EmptyListSHA3);
    }
    catch (std::exception const& ex)
    {
        cwarn << ex.what();
        throw JsonRpcException(Errors::ERROR_RPC_INTERNAL_ERROR);
    }
}

bool Test::test_setChainParams(Json::Value const& param1)
{
    try
    {
        Json::FastWriter fastWriter;
        std::string output = fastWriter.write(param1);
        asClientTest(m_eth).setChainParams(output);
        asClientTest(m_eth).completeSync();      }
    catch (std::exception const&)
    {
        BOOST_THROW_EXCEPTION(JsonRpcException(Errors::ERROR_RPC_INTERNAL_ERROR));
    }

    return true;
}

bool Test::test_mineBlocks(int _number)
{
    try
    {
        asClientTest(m_eth).mineBlocks(_number);
    }
    catch (std::exception const&)
    {
        BOOST_THROW_EXCEPTION(JsonRpcException(Errors::ERROR_RPC_INTERNAL_ERROR));
    }

    return true;
}

bool Test::test_modifyTimestamp(int _timestamp)
{
        try
    {
        asClientTest(m_eth).modifyTimestamp(_timestamp);
    }
    catch (std::exception const&)
    {
        BOOST_THROW_EXCEPTION(JsonRpcException(Errors::ERROR_RPC_INTERNAL_ERROR));
    }
    return true;
}

bool Test::test_addBlock(std::string const& _rlp)
{
    try
    {
        asClientTest(m_eth).addBlock(_rlp);
    }
    catch (std::exception const&)
    {
        BOOST_THROW_EXCEPTION(JsonRpcException(Errors::ERROR_RPC_INTERNAL_ERROR));
    }
    return true;
}

bool Test::test_rewindToBlock(int _number)
{
    try
    {
        m_eth.rewind(_number);
        asClientTest(m_eth).completeSync();
    }
    catch (std::exception const&)
    {
        BOOST_THROW_EXCEPTION(JsonRpcException(Errors::ERROR_RPC_INTERNAL_ERROR));
    }
    return true;
}
