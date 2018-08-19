


#include "JSON.h"

#include <sstream>
#include <map>
#include <memory>

using namespace std;

static_assert(
	(JSONCPP_VERSION_MAJOR == 1) && (JSONCPP_VERSION_MINOR == 8) && (JSONCPP_VERSION_PATCH == 4),
	"Unexpected jsoncpp version: " JSONCPP_VERSION_STRING ". Expecting 1.8.4."
);

namespace dev
{

namespace
{

class StreamWriterBuilder: public Json::StreamWriterBuilder
{
public:
	explicit StreamWriterBuilder(map<string, string> const& _settings)
	{
		for (auto const& iter :_settings)
			this->settings_[iter.first] = iter.second;
	}
};

class StrictModeCharReaderBuilder: public Json::CharReaderBuilder
{
public:
	StrictModeCharReaderBuilder()
	{
		Json::CharReaderBuilder::strictMode(&this->settings_);
	}
};

string print(Json::Value const& _input, Json::StreamWriterBuilder const& _builder)
{
	stringstream stream;
	unique_ptr<Json::StreamWriter> writer(_builder.newStreamWriter());
	writer->write(_input, &stream);
	return stream.str();
}

bool parse(Json::CharReaderBuilder& _builder, string const& _input, Json::Value& _json, string* _errs)
{
	unique_ptr<Json::CharReader> reader(_builder.newCharReader());
	return reader->parse(_input.c_str(), _input.c_str() + _input.length(), &_json, _errs);
}

} 
string jsonPrettyPrint(Json::Value const& _input)
{
	static map<string, string> settings{{"indentation", "  "}};
	static StreamWriterBuilder writerBuilder(settings);
	return print(_input, writerBuilder);
}

string jsonCompactPrint(Json::Value const& _input)
{
	static map<string, string> settings{{"indentation", ""}};
	static StreamWriterBuilder writerBuilder(settings);
	return print(_input, writerBuilder);
}

bool jsonParseStrict(string const& _input, Json::Value& _json, string* _errs )
{
	static StrictModeCharReaderBuilder readerBuilder;
	return parse(readerBuilder, _input, _json, _errs);
}

bool jsonParse(string const& _input, Json::Value& _json, string *_errs )
{
	static Json::CharReaderBuilder readerBuilder;
	return parse(readerBuilder, _input, _json, _errs);
}

} 