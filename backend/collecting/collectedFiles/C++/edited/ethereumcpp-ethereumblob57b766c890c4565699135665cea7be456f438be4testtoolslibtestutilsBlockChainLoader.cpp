


#include <json/writer.h>
#include <libethashseal/Ethash.h>
#include <libethereum/BlockChain.h>
#include "BlockChainLoader.h"
#include "Common.h"
using namespace std;
using namespace dev;
using namespace dev::test;
using namespace dev::eth;

BlockChainLoader::BlockChainLoader(Json::Value const& _json, eth::Network _sealEngineNetwork):
	m_block(Block::Null)
{
		bytes genesisBlock = fromHex(_json["genesisRLP"].asString());

	Json::FastWriter a;
	m_bc.reset(new BlockChain(ChainParams(genesisInfo(_sealEngineNetwork), genesisBlock, jsonToAccountMap(a.write(_json["pre"]))), m_dir.path(), WithExisting::Kill));

		m_block = m_bc->genesisBlock(State::openDB(m_dir.path(), m_bc->genesisHash(), WithExisting::Kill));

	assert(m_block.rootHash() == m_bc->info().stateRoot());

		for (auto const& block: _json["blocks"])
	{
		bytes rlp = fromHex(block["rlp"].asString());
		m_bc->import(rlp, state().db());
	}

		m_block.sync(*m_bc);
}
