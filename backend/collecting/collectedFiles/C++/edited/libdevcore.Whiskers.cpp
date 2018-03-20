


#include <libdevcore/Whiskers.h>

#include <libdevcore/Assertions.h>

#include <boost/regex.hpp>

using namespace std;
using namespace dev;

Whiskers::Whiskers(string const& _template):
m_template(_template)
{
}

Whiskers& Whiskers::operator ()(string const& _parameter, string const& _value)
{
	assertThrow(
		m_parameters.count(_parameter) == 0,
		WhiskersError,
		_parameter + " already set."
	);
	assertThrow(
		m_listParameters.count(_parameter) == 0,
		WhiskersError,
		_parameter + " already set as list parameter."
	);
	m_parameters[_parameter] = _value;

	return *this;
}

Whiskers& Whiskers::operator ()(
	string const& _listParameter,
	vector<map<string, string>> const& _values
)
{
	assertThrow(
		m_listParameters.count(_listParameter) == 0,
		WhiskersError,
		_listParameter + " already set."
	);
	assertThrow(
		m_parameters.count(_listParameter) == 0,
		WhiskersError,
		_listParameter + " already set as value parameter."
	);
	m_listParameters[_listParameter] = _values;

	return *this;
}

string Whiskers::render() const
{
	return replace(m_template, m_parameters, m_listParameters);
}

string Whiskers::replace(
	string const& _template,
	StringMap const& _parameters,
	map<string, vector<StringMap>> const& _listParameters
)
{
	using namespace boost;
	static regex listOrTag("<([^#/>]+)>|<#([^>]+)>(.*?)</\\2>");
	return regex_replace(_template, listOrTag, [&](match_results<string::const_iterator> _match) -> string
	{
		string tagName(_match[1]);
		if (!tagName.empty())
		{
			assertThrow(
				_parameters.count(tagName),
				WhiskersError,
				"Value for tag " + tagName + " not provided.\n" +
				"Template:\n" +
				_template
			);
			return _parameters.at(tagName);
		}
		else
		{
			string listName(_match[2]);
			string templ(_match[3]);
			assertThrow(!listName.empty(), WhiskersError, "");
			assertThrow(
				_listParameters.count(listName),
				WhiskersError, "List parameter " + listName + " not set."
			);
			string replacement;
			for (auto const& parameters: _listParameters.at(listName))
				replacement += replace(templ, joinMaps(_parameters, parameters));
			return replacement;
		}
	});
}

Whiskers::StringMap Whiskers::joinMaps(
	Whiskers::StringMap const& _a,
	Whiskers::StringMap const& _b
)
{
	Whiskers::StringMap ret = _a;
	for (auto const& x: _b)
		assertThrow(
			ret.insert(x).second,
			WhiskersError,
			"Parameter collision"
		);
	return ret;
}

