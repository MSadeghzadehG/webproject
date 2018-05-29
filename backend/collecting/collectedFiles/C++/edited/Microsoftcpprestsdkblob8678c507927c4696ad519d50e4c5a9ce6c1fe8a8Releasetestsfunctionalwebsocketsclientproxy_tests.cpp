

#include "stdafx.h"

#if defined(__cplusplus_winrt) || !defined(_M_ARM)

using namespace web::websockets;
using namespace web::websockets::client;

using namespace tests::functional::websocket::utilities;

namespace tests { namespace functional { namespace websocket { namespace client {

SUITE(proxy_tests)
{

#ifdef __cplusplus_winrt
TEST_FIXTURE(uri_address, no_proxy_options_on_winrt)
{
    websocket_client_config config;
    config.set_proxy(web::web_proxy::use_auto_discovery);
    websocket_client client(config);
    VERIFY_THROWS(client.connect(m_uri).wait(), websocket_exception);
}
#endif

#ifndef __cplusplus_winrt
TEST_FIXTURE(uri_address, proxy_with_credentials, "Ignore:Android", "390")
{
    web::web_proxy proxy(U("http:    web::credentials cred(U("artur"), U("fred"));     proxy.set_credentials(cred);
    websocket_client_config config;
    config.set_proxy(proxy);

    websocket_client client(config);

    try
    {
        client.connect(U("wss:        const auto text = std::string("hello");
        websocket_outgoing_message msg;
        msg.set_utf8_message(text);
        client.send(msg).wait();
        auto response = client.receive().get();
        VERIFY_ARE_EQUAL(text, response.extract_string().get());
        client.close().wait();
    }
    catch (websocket_exception const& e)
    {
        if (e.error_code().value() == 12007)
        {
                                                return;
        }
        else if (e.error_code().value() == 9 || e.error_code().value() == 5)
        {
                        return;
        }
        throw;
    }
}
#endif

} 
}}}}

#endif
