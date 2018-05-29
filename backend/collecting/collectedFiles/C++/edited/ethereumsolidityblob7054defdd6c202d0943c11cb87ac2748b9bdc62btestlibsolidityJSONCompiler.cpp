


#include <string>
#include <boost/test/unit_test.hpp>
#include <libdevcore/JSON.h>
#include <libsolidity/interface/Version.h>
#include <libsolc/libsolc.h>

#include <test/Metadata.h>
#include <test/Options.h>

using namespace std;

namespace dev
{
namespace solidity
{
namespace test
{

namespace
{

Json::Value compileSingle(string const& _input)
{
	string output(compileJSON(_input.c_str(), dev::test::Options::get().optimize));
	Json::Value ret;
	BOOST_REQUIRE(jsonParseStrict(output, ret));
	return ret;
}

Json::Value compileMulti(string const& _input, bool _callback)
{
	string output(
		_callback ?
		compileJSONCallback(_input.c_str(), dev::test::Options::get().optimize, NULL) :
		compileJSONMulti(_input.c_str(), dev::test::Options::get().optimize)
	);
	Json::Value ret;
	BOOST_REQUIRE(jsonParseStrict(output, ret));
	return ret;
}

Json::Value compile(string const& _input)
{
	string output(compileStandard(_input.c_str(), NULL));
	Json::Value ret;
	BOOST_REQUIRE(jsonParseStrict(output, ret));
	return ret;
}

} 
BOOST_AUTO_TEST_SUITE(JSONCompiler)

BOOST_AUTO_TEST_CASE(read_version)
{
	string output(version());
	BOOST_CHECK(output.find(VersionString) == 0);
}

BOOST_AUTO_TEST_CASE(read_license)
{
	string output(license());
	BOOST_CHECK(output.find("GNU GENERAL PUBLIC LICENSE") != string::npos);
}

BOOST_AUTO_TEST_CASE(basic_compilation)
{
	char const* input = R"(
	{
		"sources": {
			"fileA": "contract A { }"
		}
	}
	)";
	Json::Value result = compileMulti(input, false);
	BOOST_CHECK(result.isObject());

		BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(result),
		dev::jsonCompactPrint(compileMulti(input, true))
	);

	BOOST_CHECK(result["contracts"].isObject());
	BOOST_CHECK(result["contracts"]["fileA:A"].isObject());
	Json::Value contract = result["contracts"]["fileA:A"];
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["interface"].isString());
	BOOST_CHECK_EQUAL(contract["interface"].asString(), "[]");
	BOOST_CHECK(contract["bytecode"].isString());
	BOOST_CHECK_EQUAL(
		dev::test::bytecodeSansMetadata(contract["bytecode"].asString()),
		"6080604052348015600f57600080fd5b50603580601d6000396000f3006080604052600080fd00"
	);
	BOOST_CHECK(contract["runtimeBytecode"].isString());
	BOOST_CHECK_EQUAL(
		dev::test::bytecodeSansMetadata(contract["runtimeBytecode"].asString()),
		"6080604052600080fd00"
	);
	BOOST_CHECK(contract["functionHashes"].isObject());
	BOOST_CHECK(contract["gasEstimates"].isObject());
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(contract["gasEstimates"]),
		"{\"creation\":[66,10600],\"external\":{},\"internal\":{}}"
	);
	BOOST_CHECK(contract["metadata"].isString());
	BOOST_CHECK(dev::test::isValidMetadata(contract["metadata"].asString()));
	BOOST_CHECK(result["sources"].isObject());
	BOOST_CHECK(result["sources"]["fileA"].isObject());
	BOOST_CHECK(result["sources"]["fileA"]["AST"].isObject());
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(result["sources"]["fileA"]["AST"]),
		"{\"attributes\":{\"absolutePath\":\"fileA\",\"exportedSymbols\":{\"A\":[1]}},"
		"\"children\":[{\"attributes\":{\"baseContracts\":[null],\"contractDependencies\":[null],"
		"\"contractKind\":\"contract\",\"documentation\":null,\"fullyImplemented\":true,\"linearizedBaseContracts\":[1],"
		"\"name\":\"A\",\"nodes\":[null],\"scope\":2},\"id\":1,\"name\":\"ContractDefinition\","
		"\"src\":\"0:14:0\"}],\"id\":2,\"name\":\"SourceUnit\",\"src\":\"0:14:0\"}"
	);
}

BOOST_AUTO_TEST_CASE(single_compilation)
{
	Json::Value result = compileSingle("contract A { }");
	BOOST_CHECK(result.isObject());

	BOOST_CHECK(result["contracts"].isObject());
	BOOST_CHECK(result["contracts"][":A"].isObject());
	Json::Value contract = result["contracts"][":A"];
	BOOST_CHECK(contract.isObject());
	BOOST_CHECK(contract["interface"].isString());
	BOOST_CHECK_EQUAL(contract["interface"].asString(), "[]");
	BOOST_CHECK(contract["bytecode"].isString());
	BOOST_CHECK_EQUAL(
		dev::test::bytecodeSansMetadata(contract["bytecode"].asString()),
		"6080604052348015600f57600080fd5b50603580601d6000396000f3006080604052600080fd00"
	);
	BOOST_CHECK(contract["runtimeBytecode"].isString());
	BOOST_CHECK_EQUAL(
		dev::test::bytecodeSansMetadata(contract["runtimeBytecode"].asString()),
		"6080604052600080fd00"
	);
	BOOST_CHECK(contract["functionHashes"].isObject());
	BOOST_CHECK(contract["gasEstimates"].isObject());
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(contract["gasEstimates"]),
		"{\"creation\":[66,10600],\"external\":{},\"internal\":{}}"
	);
	BOOST_CHECK(contract["metadata"].isString());
	BOOST_CHECK(dev::test::isValidMetadata(contract["metadata"].asString()));
	BOOST_CHECK(result["sources"].isObject());
	BOOST_CHECK(result["sources"][""].isObject());
	BOOST_CHECK(result["sources"][""]["AST"].isObject());
	BOOST_CHECK_EQUAL(
		dev::jsonCompactPrint(result["sources"][""]["AST"]),
		"{\"attributes\":{\"absolutePath\":\"\",\"exportedSymbols\":{\"A\":[1]}},"
		"\"children\":[{\"attributes\":{\"baseContracts\":[null],\"contractDependencies\":[null],"
		"\"contractKind\":\"contract\",\"documentation\":null,\"fullyImplemented\":true,\"linearizedBaseContracts\":[1],"
		"\"name\":\"A\",\"nodes\":[null],\"scope\":2},\"id\":1,\"name\":\"ContractDefinition\","
		"\"src\":\"0:14:0\"}],\"id\":2,\"name\":\"SourceUnit\",\"src\":\"0:14:0\"}"
	);
}

BOOST_AUTO_TEST_CASE(standard_compilation)
{
	char const* input = R"(
	{
		"language": "Solidity",
		"sources": {
			"fileA": {
				"content": "contract A { }"
			}
		}
	}
	)";
	Json::Value result = compile(input);
	BOOST_CHECK(result.isObject());

		BOOST_CHECK(result.isMember("sources"));
	BOOST_CHECK(result.isMember("contracts"));
}

BOOST_AUTO_TEST_SUITE_END()

}
}
} 