


#include <libdevcore/SwarmHash.h>

#include <libdevcore/SHA3.h>

using namespace std;
using namespace dev;

namespace
{

bytes toLittleEndian(size_t _size)
{
	bytes encoded(8);
	for (size_t i = 0; i < 8; ++i)
		encoded[i] = (_size >> (8 * i)) & 0xff;
	return encoded;
}

h256 swarmHashSimple(bytesConstRef _data, size_t _size)
{
	return keccak256(toLittleEndian(_size) + _data.toBytes());
}

h256 swarmHashIntermediate(string const& _input, size_t _offset, size_t _length)
{
	bytesConstRef ref;
	bytes innerNodes;
	if (_length <= 0x1000)
		ref = bytesConstRef(_input).cropped(_offset, _length);
	else
	{
		size_t maxRepresentedSize = 0x1000;
		while (maxRepresentedSize * (0x1000 / 32) < _length)
			maxRepresentedSize *= (0x1000 / 32);
		for (size_t i = 0; i < _length; i += maxRepresentedSize)
		{
			size_t size = std::min(maxRepresentedSize, _length - i);
			innerNodes += swarmHashIntermediate(_input, _offset + i, size).asBytes();
		}
		ref = bytesConstRef(&innerNodes);
	}
	return swarmHashSimple(ref, _length);
}

}

h256 dev::swarmHash(string const& _input)
{
	return swarmHashIntermediate(_input, 0, _input.size());
}
