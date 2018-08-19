


#include <test/tools/libtesteth/TestHelper.h>
#include <test/tools/libtesteth/TestOutputHelper.h>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace dev;
using namespace dev::test;

BOOST_FIXTURE_TEST_SUITE(TestHelperSuite, TestOutputHelperFixture)

BOOST_AUTO_TEST_CASE(translateNetworks_gtHomestead)
{
    set<string> networks = {"Frontier", ">Homestead"};
    networks = test::translateNetworks(networks);
    BOOST_REQUIRE(networks.count("Frontier") > 0);
    BOOST_REQUIRE(networks.count("Homestead") == 0);
    for (auto const& net : test::getNetworks())
    {
        if (net != eth::Network::FrontierTest && net != eth::Network::HomesteadTest)
            BOOST_REQUIRE(networks.count(test::netIdToString(net)) > 0);
    }
}

BOOST_AUTO_TEST_CASE(translateNetworks_geHomestead)
{
    set<string> networks = {"Frontier", ">=Homestead"};
    networks = test::translateNetworks(networks);
    for (auto const& net : test::getNetworks())
        BOOST_REQUIRE(networks.count(test::netIdToString(net)) > 0);
}

BOOST_AUTO_TEST_CASE(translateNetworks_ltHomestead)
{
    set<string> networks = {"<Homestead"};
    networks = test::translateNetworks(networks);
    BOOST_REQUIRE(networks.count("Frontier") > 0);
    for (auto const& net : test::getNetworks())
    {
        if (net != eth::Network::FrontierTest)
            BOOST_REQUIRE(networks.count(test::netIdToString(net)) == 0);
    }
}

BOOST_AUTO_TEST_CASE(translateNetworks_ltTest)
{
    set<string> networks = {"<=EIP150", "<EIP158"};
    networks = test::translateNetworks(networks);
    BOOST_REQUIRE(networks.count("Frontier") > 0);
    BOOST_REQUIRE(networks.count("Homestead") > 0);
    BOOST_REQUIRE(networks.count("EIP150") > 0);
    BOOST_REQUIRE(networks.count("EIP158") == 0);
    BOOST_REQUIRE(networks.count("Byzantium") == 0);
}

BOOST_AUTO_TEST_CASE(translateNetworks_leHomestead)
{
    set<string> networks = {"<=Homestead"};
    networks = test::translateNetworks(networks);
    BOOST_REQUIRE(networks.count("Frontier") > 0);
    BOOST_REQUIRE(networks.count("Homestead") > 0);
    for (auto const& net : test::getNetworks())
    {
        if (net != eth::Network::FrontierTest && net != eth::Network::HomesteadTest)
            BOOST_REQUIRE(networks.count(test::netIdToString(net)) == 0);
    }
}

BOOST_AUTO_TEST_CASE(translateNetworks_leFrontier)
{
    set<string> networks = {"<=Frontier"};
    networks = test::translateNetworks(networks);
    BOOST_REQUIRE(networks.count("Frontier") > 0);
    for (auto const& net : test::getNetworks())
    {
        if (net != eth::Network::FrontierTest)
            BOOST_REQUIRE(networks.count(test::netIdToString(net)) == 0);
    }
}

BOOST_AUTO_TEST_SUITE_END()
