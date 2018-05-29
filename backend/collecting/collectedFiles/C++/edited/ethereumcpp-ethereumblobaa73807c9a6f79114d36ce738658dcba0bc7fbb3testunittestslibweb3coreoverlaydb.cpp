


#include <libdevcore/DBImpl.h>
#include <libdevcore/OverlayDB.h>
#include <libdevcore/TransientDirectory.h>
#include <test/tools/libtesteth/TestOutputHelper.h>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace dev;
using namespace dev::test;

BOOST_FIXTURE_TEST_SUITE(OverlayDBTests, TestOutputHelperFixture)

BOOST_AUTO_TEST_CASE(basicUsage)
{
    TransientDirectory td;
    std::unique_ptr<db::DBImpl> db(new db::DBImpl(td.path()));
    BOOST_REQUIRE(db);

    OverlayDB odb(std::move(db));
    BOOST_CHECK(!odb.get().size());

        odb.commit();

    string const value = "\x43";
    BOOST_CHECK(!odb.get().size());

    odb.insert(h256(42), &value);
    BOOST_CHECK(odb.get().size());
    BOOST_CHECK(odb.exists(h256(42)));
    BOOST_CHECK_EQUAL(odb.lookup(h256(42)), value);

    odb.commit();
    BOOST_CHECK(!odb.get().size());
    BOOST_CHECK(odb.exists(h256(42)));
    BOOST_CHECK_EQUAL(odb.lookup(h256(42)), value);

    odb.insert(h256(41), &value);
    odb.commit();
    BOOST_CHECK(!odb.get().size());
    BOOST_CHECK(odb.exists(h256(41)));
    BOOST_CHECK_EQUAL(odb.lookup(h256(41)), value);
}

BOOST_AUTO_TEST_CASE(auxMem)
{
    TransientDirectory td;
    std::unique_ptr<db::DBImpl> db(new db::DBImpl(td.path()));
    BOOST_REQUIRE(db);

    OverlayDB odb(std::move(db));

    string const value = "\x43";
    bytes valueAux = fromHex("44");

    odb.insert(h256(42), &value);
    odb.insert(h256(0), &value);
    odb.insert(h256(numeric_limits<u256>::max()), &value);

    odb.insertAux(h256(42), &valueAux);
    odb.insertAux(h256(0), &valueAux);
    odb.insertAux(h256(numeric_limits<u256>::max()), &valueAux);

    odb.commit();

    BOOST_CHECK(!odb.get().size());

    BOOST_CHECK(odb.exists(h256(42)));
    BOOST_CHECK_EQUAL(odb.lookup(h256(42)), value);

    BOOST_CHECK(odb.exists(h256(0)));
    BOOST_CHECK_EQUAL(odb.lookup(h256(0)), value);

    BOOST_CHECK(odb.exists(h256(std::numeric_limits<u256>::max())));
    BOOST_CHECK_EQUAL(odb.lookup(h256(std::numeric_limits<u256>::max())), value);

    BOOST_CHECK(odb.lookupAux(h256(42)) == valueAux);
    BOOST_CHECK(odb.lookupAux(h256(0)) == valueAux);
    BOOST_CHECK(odb.lookupAux(h256(std::numeric_limits<u256>::max())) == valueAux);
}

BOOST_AUTO_TEST_CASE(rollback)
{
    TransientDirectory td;
    std::unique_ptr<db::DBImpl> db(new db::DBImpl(td.path()));
    BOOST_REQUIRE(db);

    OverlayDB odb(std::move(db));
    bytes value = fromHex("42");

    odb.insert(h256(43), &value);
    BOOST_CHECK(odb.get().size());
    odb.rollback();
    BOOST_CHECK(!odb.get().size());
}

BOOST_AUTO_TEST_SUITE_END()
