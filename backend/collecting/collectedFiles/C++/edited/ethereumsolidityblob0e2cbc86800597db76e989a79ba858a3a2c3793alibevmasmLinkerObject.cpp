


#include <libevmasm/LinkerObject.h>
#include <libdevcore/CommonData.h>

using namespace dev;
using namespace dev::eth;
using namespace std;

void LinkerObject::append(LinkerObject const& _other)
{
	for (auto const& ref: _other.linkReferences)
		linkReferences[ref.first + bytecode.size()] = ref.second;
	bytecode += _other.bytecode;
}

void LinkerObject::link(map<string, h160> const& _libraryAddresses)
{
	std::map<size_t, std::string> remainingRefs;
	for (auto const& linkRef: linkReferences)
		if (h160 const* address = matchLibrary(linkRef.second, _libraryAddresses))
			copy(address->data(), address->data() + 20, bytecode.begin() + linkRef.first);
		else
			remainingRefs.insert(linkRef);
	linkReferences.swap(remainingRefs);
}

string LinkerObject::toHex() const
{
	string hex = dev::toHex(bytecode);
	for (auto const& ref: linkReferences)
	{
		size_t pos = ref.first * 2;
		string const& name = ref.second;
		hex[pos] = hex[pos + 1] = hex[pos + 38] = hex[pos + 39] = '_';
		for (size_t i = 0; i < 36; ++i)
			hex[pos + 2 + i] = i < name.size() ? name[i] : '_';
	}
	return hex;
}

h160 const*
LinkerObject::matchLibrary(
	string const& _linkRefName,
	map<string, h160> const& _libraryAddresses
)
{
	auto it = _libraryAddresses.find(_linkRefName);
	if (it != _libraryAddresses.end())
		return &it->second;
			size_t colon = _linkRefName.find(':');
	if (colon == string::npos)
		return nullptr;
	it = _libraryAddresses.find(_linkRefName.substr(colon + 1));
	if (it != _libraryAddresses.end())
		return &it->second;
	return nullptr;
}
