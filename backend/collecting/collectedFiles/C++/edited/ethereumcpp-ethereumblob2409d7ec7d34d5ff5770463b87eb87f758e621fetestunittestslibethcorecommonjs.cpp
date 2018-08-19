


#include <boost/test/unit_test.hpp>
#include <libdevcore/Log.h>
#include <libethcore/CommonJS.h>
#include <test/tools/libtesteth/TestOutputHelper.h>

using namespace std;
using namespace dev;
using namespace dev::eth;
using namespace dev::test;

BOOST_FIXTURE_TEST_SUITE(commonjs, TestOutputHelperFixture)

BOOST_AUTO_TEST_CASE(jsToPublic)
{
	KeyPair kp = KeyPair::create();
	string s = toJS(kp.pub());
	Public pub = dev::jsToPublic(s);
	BOOST_CHECK_EQUAL(kp.pub(), pub);
}

BOOST_AUTO_TEST_CASE(jsToAddress)
{
	KeyPair kp = KeyPair::create();
	string s = toJS(kp.address());
	Address address = dev::jsToAddress(s);
	BOOST_CHECK_EQUAL(kp.address(), address);
}

BOOST_AUTO_TEST_CASE(jsToSecret)
{
	KeyPair kp = KeyPair::create();
	string s = toJS(kp.secret().makeInsecure());
	Secret secret = dev::jsToSecret(s);
	BOOST_CHECK_EQUAL(kp.secret().makeInsecure(), secret.makeInsecure());
}

BOOST_AUTO_TEST_SUITE_END()
