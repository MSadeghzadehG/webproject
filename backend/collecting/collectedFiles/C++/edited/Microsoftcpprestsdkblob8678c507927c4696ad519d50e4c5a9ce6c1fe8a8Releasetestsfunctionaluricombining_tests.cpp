

#include "stdafx.h"

using namespace web;
using namespace utility;

namespace tests { namespace functional { namespace uri_tests {

SUITE(combining_tests)
{

TEST(append_path)
{
    utility::string_t uri_str = U("http:    uri_builder ub(uri_str);
    uri combined = ub.append_path(U("/baz")).to_uri();

    VERIFY_ARE_EQUAL(uri(U("http:}

TEST(append_empty_path)
{
    utility::string_t uri_str(U("http:    uri u = uri_str;
    uri_builder ub(u);
    uri combined = ub.append_path(U("")).to_uri();

    VERIFY_ARE_EQUAL(u, combined);
}

TEST(append_query)
{
    utility::string_t uri_str(U("http:    uri_builder ub(uri_str);
    uri combined = ub.append_query(uri(U("http:    
    VERIFY_ARE_EQUAL(U("http:}

TEST(append_empty_query)
{
    utility::string_t uri_str(U("http:    uri u(uri_str);
    uri_builder ub(u);
    uri combined = ub.append_query(U("")).to_uri();

    VERIFY_ARE_EQUAL(u, combined);
}

TEST(append)
{
    utility::string_t uri_str(U("http:    uri_builder ub(uri_str);
    uri combined = ub.append(U("http:    
    VERIFY_ARE_EQUAL(U("http:    VERIFY_ARE_EQUAL(U("/path1/path2?key1=value2&key2=value3"), combined.resource().to_string());
}

TEST(append_empty)
{
    utility::string_t uri_str(U("http:    uri u(uri_str);
    uri_builder ub(u);
    uri combined = ub.append(U("")).to_uri();

    VERIFY_ARE_EQUAL(u, combined);
}

} 
}}}
