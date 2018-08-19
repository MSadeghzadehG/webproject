


#include <liblll/Compiler.h>
#include <liblll/Parser.h>
#include <liblll/CompilerState.h>
#include <liblll/CodeFragment.h>

using namespace std;
using namespace dev;
using namespace dev::eth;

bytes dev::eth::compileLLL(string const& _src, dev::solidity::EVMVersion _evmVersion, bool _opt, std::vector<std::string>* _errors, dev::eth::ReadCallback const& _readFile)
{
	try
	{
		CompilerState cs;
		cs.populateStandard();
		auto assembly = CodeFragment::compile(_src, cs, _readFile).assembly(cs);
		if (_opt)
			assembly = assembly.optimise(true, _evmVersion);
		bytes ret = assembly.assemble().bytecode;
		for (auto i: cs.treesToKill)
			killBigints(i);
		return ret;
	}
	catch (Exception const& _e)
	{
		if (_errors)
		{
			_errors->push_back("Parse error.");
			_errors->push_back(boost::diagnostic_information(_e));
		}
	}
	catch (std::exception const& _e)
	{
		if (_errors)
		{
			_errors->push_back("Parse exception.");
			_errors->push_back(boost::diagnostic_information(_e));
		}
	}
	catch (...)
	{
		if (_errors)
			_errors->push_back("Internal compiler exception.");
	}
	return bytes();
}

std::string dev::eth::compileLLLToAsm(std::string const& _src, EVMVersion _evmVersion, bool _opt, std::vector<std::string>* _errors, ReadCallback const& _readFile)
{
	try
	{
		CompilerState cs;
		cs.populateStandard();
		auto assembly = CodeFragment::compile(_src, cs, _readFile).assembly(cs);
		if (_opt)
			assembly = assembly.optimise(true, _evmVersion);
		string ret = assembly.assemblyString();
		for (auto i: cs.treesToKill)
			killBigints(i);
		return ret;
	}
	catch (Exception const& _e)
	{
		if (_errors)
		{
			_errors->push_back("Parse error.");
			_errors->push_back(boost::diagnostic_information(_e));
		}
	}
	catch (std::exception const& _e)
	{
		if (_errors) {
			_errors->push_back("Parse exception.");
			_errors->push_back(boost::diagnostic_information(_e));
		}
	}
	catch (...)
	{
		if (_errors)
			_errors->push_back("Internal compiler exception.");
	}
	return string();
}

string dev::eth::parseLLL(string const& _src)
{
	sp::utree o;

	try
	{
		parseTreeLLL(_src, o);
	}
	catch (...)
	{
		killBigints(o);
		return string();
	}

	ostringstream ret;
	debugOutAST(ret, o);
	killBigints(o);
	return ret.str();
}
