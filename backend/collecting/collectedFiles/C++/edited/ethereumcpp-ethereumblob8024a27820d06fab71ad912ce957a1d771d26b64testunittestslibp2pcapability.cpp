


#include <boost/test/unit_test.hpp>
#include <chrono>
#include <thread>
#include <libp2p/Common.h>
#include <libp2p/Host.h>
#include <libp2p/Session.h>
#include <libp2p/Capability.h>
#include <libp2p/HostCapability.h>
#include <test/tools/libtesteth/TestOutputHelper.h>
#include <test/tools/libtesteth/Options.h>

using namespace std;
using namespace dev;
using namespace dev::test;
using namespace dev::p2p;

struct P2PFixture: public TestOutputHelperFixture
{
    P2PFixture() { dev::p2p::NodeIPEndpoint::test_allowLocal = true; }
    ~P2PFixture() { dev::p2p::NodeIPEndpoint::test_allowLocal = false; }
};

class TestCapability: public Capability
{
public:
    TestCapability(std::shared_ptr<SessionFace> _s, HostCapabilityFace* _h, unsigned _idOffset, CapDesc const&): Capability(_s, _h, _idOffset), m_cntReceivedMessages(0), m_testSum(0) {}
    virtual ~TestCapability() {}
    int countReceivedMessages() { return m_cntReceivedMessages; }
    int testSum() { return m_testSum; }
    static std::string name() { return "test"; }
    static u256 version() { return 2; }
    static unsigned messageCount() { return UserPacket + 1; }
    void sendTestMessage(int _i) { RLPStream s; sealAndSend(prep(s, UserPacket, 1) << _i); }

protected:
    virtual bool interpret(unsigned _id, RLP const& _r) override;

    int m_cntReceivedMessages;
    int m_testSum;
};

bool TestCapability::interpret(unsigned _id, RLP const& _r) 
{
        ++m_cntReceivedMessages;
    m_testSum += _r[0].toInt();
    BOOST_ASSERT(_id == UserPacket);
    return (_id == UserPacket);
}

class TestHostCapability: public HostCapability<TestCapability>, public Worker
{
public:
    TestHostCapability(): Worker("test") {}
    virtual ~TestHostCapability() {}

    void sendTestMessage(NodeID const& _id, int _x)
    {
        for (auto i: peerSessions())
            if (_id == i.second->id)
                capabilityFromSession<TestCapability>(*i.first)->sendTestMessage(_x);
    }

    std::pair<int, int> retrieveTestData(NodeID const& _id)
    { 
        int cnt = 0;
        int checksum = 0;
        for (auto i: peerSessions())
            if (_id == i.second->id)
            {
                cnt += capabilityFromSession<TestCapability>(*i.first)->countReceivedMessages();
                checksum += capabilityFromSession<TestCapability>(*i.first)->testSum();
            }

        return std::pair<int, int>(cnt, checksum);
    }
};

BOOST_FIXTURE_TEST_SUITE(p2pCapability, P2PFixture)

BOOST_AUTO_TEST_CASE(capability)
{
    cnote << "Testing Capability...";

    int const step = 10;
    const char* const localhost = "127.0.0.1";
    NetworkPreferences prefs1(localhost, 0, false);
    NetworkPreferences prefs2(localhost, 0, false);
    Host host1("Test", prefs1);
    Host host2("Test", prefs2);
    auto thc1 = host1.registerCapability(make_shared<TestHostCapability>());
    auto thc2 = host2.registerCapability(make_shared<TestHostCapability>());
    host1.start();	
    host2.start();
    auto port1 = host1.listenPort();
    auto port2 = host2.listenPort();
    BOOST_REQUIRE(port1);
    BOOST_REQUIRE(port2);	
    BOOST_REQUIRE_NE(port1, port2);

    for (unsigned i = 0; i < 3000; i += step)
    {
        this_thread::sleep_for(chrono::milliseconds(step));

        if (host1.isStarted() && host2.isStarted())
            break;
    }

    BOOST_REQUIRE(host1.isStarted() && host2.isStarted());
    host1.requirePeer(host2.id(), NodeIPEndpoint(bi::address::from_string(localhost), port2, port2));

        for (unsigned i = 0; i < 12000; i += step)
    {
        this_thread::sleep_for(chrono::milliseconds(step));

        if ((host1.peerCount() > 0) && (host2.peerCount() > 0))
            break;
    }

    BOOST_REQUIRE(host1.peerCount() > 0 && host2.peerCount() > 0);

    int const target = 64;
    int checksum = 0;
    for (int i = 0; i < target; checksum += i++)
        thc2->sendTestMessage(host1.id(), i);

    this_thread::sleep_for(chrono::seconds(target / 64 + 1));
    std::pair<int, int> testData = thc1->retrieveTestData(host2.id());
    BOOST_REQUIRE_EQUAL(target, testData.first);
    BOOST_REQUIRE_EQUAL(checksum, testData.second);
}

BOOST_AUTO_TEST_SUITE_END()


