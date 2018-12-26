

#include <libdevcore/JsonUtils.h>
#include <json_spirit/JsonSpiritHeaders.h>
#include <set>
#include <string>
#include <ostream>

void dev::validateFieldNames(json_spirit::mObject const& _obj, std::set<std::string> const& _allowedFields)
{
	for (auto const& elm: _obj)
		if (_allowedFields.find(elm.first) == _allowedFields.end())
		{
			std::string const comment = "Unknown field in config: " + elm.first;
			std::cerr << comment << "\n";
			BOOST_THROW_EXCEPTION(UnknownField() << errinfo_comment(comment));
		}
}
