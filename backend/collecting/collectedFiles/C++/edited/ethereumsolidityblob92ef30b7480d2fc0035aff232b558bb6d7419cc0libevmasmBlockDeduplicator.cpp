


#include <libevmasm/BlockDeduplicator.h>

#include <libevmasm/AssemblyItem.h>
#include <libevmasm/SemanticInformation.h>

#include <functional>
#include <set>

using namespace std;
using namespace dev;
using namespace dev::eth;


bool BlockDeduplicator::deduplicate()
{
		
			AssemblyItem pushSelf(PushTag, u256(-4));
	if (
		std::count(m_items.cbegin(), m_items.cend(), pushSelf.tag()) ||
		std::count(m_items.cbegin(), m_items.cend(), pushSelf.pushTag())
	)
		return false;

	function<bool(size_t, size_t)> comparator = [&](size_t _i, size_t _j)
	{
		if (_i == _j)
			return false;

						AssemblyItem pushFirstTag(pushSelf);
		AssemblyItem pushSecondTag(pushSelf);

		if (_i < m_items.size() && m_items.at(_i).type() == Tag)
			pushFirstTag = m_items.at(_i).pushTag();
		if (_j < m_items.size() && m_items.at(_j).type() == Tag)
			pushSecondTag = m_items.at(_j).pushTag();

		BlockIterator first(m_items.begin() + _i, m_items.end(), &pushFirstTag, &pushSelf);
		BlockIterator second(m_items.begin() + _j, m_items.end(), &pushSecondTag, &pushSelf);
		BlockIterator end(m_items.end(), m_items.end());

		if (first != end && (*first).type() == Tag)
			++first;
		if (second != end && (*second).type() == Tag)
			++second;

		return std::lexicographical_compare(first, end, second, end);
	};

	size_t iterations = 0;
	for (; ; ++iterations)
	{
				set<size_t, function<bool(size_t, size_t)>> blocksSeen(comparator);
		for (size_t i = 0; i < m_items.size(); ++i)
		{
			if (m_items.at(i).type() != Tag)
				continue;
			auto it = blocksSeen.find(i);
			if (it == blocksSeen.end())
				blocksSeen.insert(i);
			else
				m_replacedTags[m_items.at(i).data()] = m_items.at(*it).data();
		}

		if (!applyTagReplacement(m_items, m_replacedTags))
			break;
	}
	return iterations > 0;
}

bool BlockDeduplicator::applyTagReplacement(
	AssemblyItems& _items,
	map<u256, u256> const& _replacements,
	size_t _subId
)
{
	bool changed = false;
	for (AssemblyItem& item: _items)
		if (item.type() == PushTag)
		{
			size_t subId;
			size_t tagId;
			tie(subId, tagId) = item.splitForeignPushTag();
			if (subId != _subId)
				continue;
			auto it = _replacements.find(tagId);
			if (it != _replacements.end())
			{
				changed = true;
				item.setPushTagSubIdAndTag(subId, size_t(it->second));
			}
		}
	return changed;
}

BlockDeduplicator::BlockIterator& BlockDeduplicator::BlockIterator::operator++()
{
	if (it == end)
		return *this;
	if (SemanticInformation::altersControlFlow(*it) && *it != AssemblyItem(Instruction::JUMPI))
		it = end;
	else
	{
		++it;
		while (it != end && it->type() == Tag)
			++it;
	}
	return *this;
}

AssemblyItem const& BlockDeduplicator::BlockIterator::operator*() const
{
	if (replaceItem && replaceWith && *it == *replaceItem)
		return *replaceWith;
	else
		return *it;
}
