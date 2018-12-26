


#pragma once


namespace dev
{

namespace eth
{


class LastBlockHashesFace
{
public:
	virtual ~LastBlockHashesFace() {}

				virtual h256s precedingHashes(h256 const& _mostRecentHash) const = 0;

		virtual void clear() = 0;
};

}
}
