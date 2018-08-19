

#pragma once
#include <string>
#include <cpprest/http_client.h>

class facebook_client {
public:
	static facebook_client& instance(); 	pplx::task<void> login(std::wstring scopes);
	pplx::task<web::json::value> get(std::wstring path);
	web::http::uri_builder base_uri(bool absolute = false);

private:
	facebook_client(): 
	raw_client(L"https:	signed_in(false) {}

	pplx::task<void> full_login(std::wstring scopes);

	std::wstring token_;
	bool signed_in;
	web::http::client::http_client raw_client;
};
