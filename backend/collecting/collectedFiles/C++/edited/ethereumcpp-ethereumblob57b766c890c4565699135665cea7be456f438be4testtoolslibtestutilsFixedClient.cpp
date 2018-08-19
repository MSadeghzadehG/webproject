


#include "FixedClient.h"

using namespace dev;
using namespace dev::eth;
using namespace dev::test;

Block FixedClient::block(h256 const& _h) const
{
	ReadGuard l(x_stateDB);
	Block ret(bc(), m_block.db());
	ret.populateFromChain(bc(), _h);
	return ret;
}
