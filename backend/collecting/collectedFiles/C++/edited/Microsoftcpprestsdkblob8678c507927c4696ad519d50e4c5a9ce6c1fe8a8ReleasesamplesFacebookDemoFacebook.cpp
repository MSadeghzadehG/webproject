

#include "pch.h"
#include "facebook.h"

using namespace pplx;
using namespace web;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Security::Authentication::Web;
using namespace Windows::Storage;

const std::wstring application_id(L"insert your application ID");
const std::wstring application_token(L"insert your application token");

facebook_client& facebook_client::instance()
{
	static facebook_client c;
	return c;
}

pplx::task<void> facebook_client::login(std::wstring scopes)
{
		auto ls = ApplicationData::Current->LocalSettings->CreateContainer("LoginDetailsCache",
		ApplicationDataCreateDisposition::Always);

	if(ls->Values->HasKey("facebookToken")) {
		token_ = dynamic_cast<String^>(ls->Values->Lookup("facebookToken"))->Data();
	}

	if(!token_.empty()) {
				using namespace http;

		uri_builder tokendbg_uri(L"/debug_token");
		tokendbg_uri.append_query(L"input_token", token_);
		tokendbg_uri.append_query(L"access_token", application_token);

		http_request request(methods::GET);
		request.set_request_uri(tokendbg_uri.to_string());
		request.headers().add(header_names::accept, L"application/json");

		return raw_client.request(request)
		.then([](http_response response){
			return response.extract_json();
		}).then([=](json::value v) -> task<void> {
			auto obj = v[L"data"];

			if(obj[L"is_valid"].as_bool()) {
								signed_in = true;
				return create_task([](){}); 			}

						return full_login(scopes);
		}, task_continuation_context::use_current());
	}

		return full_login(scopes);
}

pplx::task<void> facebook_client::full_login(std::wstring scopes) 
{
		http::uri_builder login_uri(L"https:	login_uri.append_query(L"client_id", application_id); 	login_uri.append_query(L"redirect_uri", L"https:	login_uri.append_query(L"scope", scopes);
	login_uri.append_query(L"display", L"popup");
	login_uri.append_query(L"response_type", L"token");

	return create_task(WebAuthenticationBroker::AuthenticateAsync(
		WebAuthenticationOptions::None, ref new Uri(ref new String(login_uri.to_string().c_str())),
		ref new Uri("https:	.then([=](WebAuthenticationResult^ result) {
		if(result->ResponseStatus == WebAuthenticationStatus::Success)
		{
			signed_in = true;

			std::wstring response(result->ResponseData->Data());  			auto start = response.find(L"access_token=");
			start += 13;
			auto end = response.find('&');

			token_ = response.substr(start, end-start);

						auto ls = ApplicationData::Current->LocalSettings->CreateContainer("LoginDetailsCache",
				ApplicationDataCreateDisposition::Always);

			ls->Values->Insert("facebookToken", ref new String(token_.c_str()));
		}
	});
}

task<json::value> facebook_client::get(std::wstring path)
{
	using namespace http;
	
	http_request request(methods::GET);

	request.set_request_uri(base_uri().append_path(path).to_uri());
	request.headers().add(header_names::accept, L"application/json");

	return raw_client.request(request)
	.then([](http_response response) {
		return response.extract_json();
	});
}

http::uri_builder facebook_client::base_uri(bool absolute)
{
	http::uri_builder ret;

	if(absolute)
		ret.set_path(L"https:
	ret.append_query(L"access_token", token_);
	return ret;
}