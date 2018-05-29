

#include "stdafx.h"

using namespace web::http;
using namespace web::http::client;

using namespace tests::functional::http::utilities;

namespace tests { namespace functional { namespace http { namespace client {

void test_connection(test_http_server *p_server, http_client *p_client, const utility::string_t &path)
{
    p_server->next_request().then([path](test_request *p_request)
    {
        http_asserts::assert_test_request_equals(p_request, methods::GET, path);
        VERIFY_ARE_EQUAL(0u, p_request->reply(200));
    });
    http_asserts::assert_response_equals(p_client->request(methods::GET).get(), status_codes::OK);
}

void test_connection(test_http_server *p_server, http_client *p_client, const utility::string_t &request_path, const utility::string_t &expected_path)
{
    p_server->next_request().then([expected_path](test_request *p_request)
    {
        http_asserts::assert_test_request_equals(p_request, methods::GET, expected_path);
        VERIFY_ARE_EQUAL(0u, p_request->reply(200));
    });
    http_asserts::assert_response_equals(p_client->request(methods::GET, request_path).get(), status_codes::OK);
}

}}}}