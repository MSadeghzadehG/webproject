

#include <algorithm>
#include <array>
#include <cstring>
#include <functional>
#include <map>
#include <set>
#include <utility>

#include "Common/CommonTypes.h"
#include "Common/JitRegister.h"
#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/PowerPC/JitCommon/JitBase.h"
#include "Core/PowerPC/PPCSymbolDB.h"
#include "Core/PowerPC/PowerPC.h"

#ifdef _WIN32
#include <windows.h>
#endif

using namespace Gen;

bool JitBlock::OverlapsPhysicalRange(u32 address, u32 length) const
{
  return physical_addresses.lower_bound(address) !=
         physical_addresses.lower_bound(address + length);
}

JitBaseBlockCache::JitBaseBlockCache(JitBase& jit) : m_jit{jit}
{
}

JitBaseBlockCache::~JitBaseBlockCache() = default;

void JitBaseBlockCache::Init()
{
  JitRegister::Init(SConfig::GetInstance().m_perfDir);

  Clear();
}

void JitBaseBlockCache::Shutdown()
{
  JitRegister::Shutdown();
}

void JitBaseBlockCache::Clear()
{
#if defined(_DEBUG) || defined(DEBUGFAST)
  Core::DisplayMessage("Clearing code cache.", 3000);
#endif
  m_jit.js.fifoWriteAddresses.clear();
  m_jit.js.pairedQuantizeAddresses.clear();
  for (auto& e : block_map)
  {
    DestroyBlock(e.second);
  }
  block_map.clear();
  links_to.clear();
  block_range_map.clear();

  valid_block.ClearAll();

  fast_block_map.fill(nullptr);
}

void JitBaseBlockCache::Reset()
{
  Shutdown();
  Init();
}

JitBlock** JitBaseBlockCache::GetFastBlockMap()
{
  return fast_block_map.data();
}

void JitBaseBlockCache::RunOnBlocks(std::function<void(const JitBlock&)> f)
{
  for (const auto& e : block_map)
    f(e.second);
}

JitBlock* JitBaseBlockCache::AllocateBlock(u32 em_address)
{
  u32 physicalAddress = PowerPC::JitCache_TranslateAddress(em_address).address;
  JitBlock& b = block_map.emplace(physicalAddress, JitBlock())->second;
  b.effectiveAddress = em_address;
  b.physicalAddress = physicalAddress;
  b.msrBits = MSR & JIT_CACHE_MSR_MASK;
  b.linkData.clear();
  b.fast_block_map_index = 0;
  return &b;
}

void JitBaseBlockCache::FinalizeBlock(JitBlock& block, bool block_link,
                                      const std::set<u32>& physical_addresses)
{
  size_t index = FastLookupIndexForAddress(block.effectiveAddress);
  fast_block_map[index] = &block;
  block.fast_block_map_index = index;

  block.physical_addresses = physical_addresses;

  u32 range_mask = ~(BLOCK_RANGE_MAP_ELEMENTS - 1);
  for (u32 addr : physical_addresses)
  {
    valid_block.Set(addr / 32);
    block_range_map[addr & range_mask].insert(&block);
  }

  if (block_link)
  {
    for (const auto& e : block.linkData)
    {
      links_to.emplace(e.exitAddress, &block);
    }

    LinkBlock(block);
  }

  Symbol* symbol = nullptr;
  if (JitRegister::IsEnabled() &&
      (symbol = g_symbolDB.GetSymbolFromAddr(block.effectiveAddress)) != nullptr)
  {
    JitRegister::Register(block.checkedEntry, block.codeSize, "JIT_PPC_%s_%08x",
                          symbol->function_name.c_str(), block.physicalAddress);
  }
  else
  {
    JitRegister::Register(block.checkedEntry, block.codeSize, "JIT_PPC_%08x",
                          block.physicalAddress);
  }
}

JitBlock* JitBaseBlockCache::GetBlockFromStartAddress(u32 addr, u32 msr)
{
  u32 translated_addr = addr;
  if (UReg_MSR(msr).IR)
  {
    auto translated = PowerPC::JitCache_TranslateAddress(addr);
    if (!translated.valid)
    {
      return nullptr;
    }
    translated_addr = translated.address;
  }

  auto iter = block_map.equal_range(translated_addr);
  for (; iter.first != iter.second; iter.first++)
  {
    JitBlock& b = iter.first->second;
    if (b.effectiveAddress == addr && b.msrBits == (msr & JIT_CACHE_MSR_MASK))
      return &b;
  }

  return nullptr;
}

const u8* JitBaseBlockCache::Dispatch()
{
  JitBlock* block = fast_block_map[FastLookupIndexForAddress(PC)];

  if (!block || block->effectiveAddress != PC || block->msrBits != (MSR & JIT_CACHE_MSR_MASK))
    block = MoveBlockIntoFastCache(PC, MSR & JIT_CACHE_MSR_MASK);

  if (!block)
    return nullptr;

  return block->normalEntry;
}

void JitBaseBlockCache::InvalidateICache(u32 address, u32 length, bool forced)
{
  auto translated = PowerPC::JitCache_TranslateAddress(address);
  if (!translated.valid)
    return;
  u32 pAddr = translated.address;

    bool destroy_block = true;
  if (length == 32)
  {
    if (!valid_block.Test(pAddr / 32))
      destroy_block = false;
    else
      valid_block.Clear(pAddr / 32);
  }

  if (destroy_block)
  {
        ErasePhysicalRange(pAddr, length);

                    if (!forced)
    {
      for (u32 i = address; i < address + length; i += 4)
      {
        m_jit.js.fifoWriteAddresses.erase(i);
        m_jit.js.pairedQuantizeAddresses.erase(i);
      }
    }
  }
}

void JitBaseBlockCache::ErasePhysicalRange(u32 address, u32 length)
{
    u32 range_mask = ~(BLOCK_RANGE_MAP_ELEMENTS - 1);
  auto start = block_range_map.lower_bound(address & range_mask);
  auto end = block_range_map.lower_bound(address + length);
  while (start != end)
  {
        auto iter = start->second.begin();
    while (iter != start->second.end())
    {
      JitBlock* block = *iter;
      if (block->OverlapsPhysicalRange(address, length))
      {
                        for (u32 addr : block->physical_addresses)
          if ((addr & range_mask) != start->first)
            block_range_map[addr & range_mask].erase(block);

                DestroyBlock(*block);
        auto block_map_iter = block_map.equal_range(block->physicalAddress);
        while (block_map_iter.first != block_map_iter.second)
        {
          if (&block_map_iter.first->second == block)
          {
            block_map.erase(block_map_iter.first);
            break;
          }
          block_map_iter.first++;
        }
        iter = start->second.erase(iter);
      }
      else
      {
        iter++;
      }
    }

        if (start->second.empty())
      start = block_range_map.erase(start);
    else
      start++;
  }
}

u32* JitBaseBlockCache::GetBlockBitSet() const
{
  return valid_block.m_valid_block.get();
}

void JitBaseBlockCache::WriteDestroyBlock(const JitBlock& block)
{
}


void JitBaseBlockCache::LinkBlockExits(JitBlock& block)
{
  for (auto& e : block.linkData)
  {
    if (!e.linkStatus)
    {
      JitBlock* destinationBlock = GetBlockFromStartAddress(e.exitAddress, block.msrBits);
      if (destinationBlock)
      {
        WriteLinkBlock(e, destinationBlock);
        e.linkStatus = true;
      }
    }
  }
}

void JitBaseBlockCache::LinkBlock(JitBlock& block)
{
  LinkBlockExits(block);
  auto ppp = links_to.equal_range(block.effectiveAddress);

  for (auto iter = ppp.first; iter != ppp.second; ++iter)
  {
    JitBlock& b2 = *iter->second;
    if (block.msrBits == b2.msrBits)
      LinkBlockExits(b2);
  }
}

void JitBaseBlockCache::UnlinkBlock(const JitBlock& block)
{
    for (auto& e : block.linkData)
  {
    WriteLinkBlock(e, nullptr);
  }

    auto ppp = links_to.equal_range(block.effectiveAddress);
  for (auto iter = ppp.first; iter != ppp.second; ++iter)
  {
    JitBlock& sourceBlock = *iter->second;
    if (sourceBlock.msrBits != block.msrBits)
      continue;

    for (auto& e : sourceBlock.linkData)
    {
      if (e.exitAddress == block.effectiveAddress)
      {
        WriteLinkBlock(e, nullptr);
        e.linkStatus = false;
      }
    }
  }
}

void JitBaseBlockCache::DestroyBlock(JitBlock& block)
{
  if (fast_block_map[block.fast_block_map_index] == &block)
    fast_block_map[block.fast_block_map_index] = nullptr;

  UnlinkBlock(block);

    for (const auto& e : block.linkData)
  {
    auto it = links_to.equal_range(e.exitAddress);
    while (it.first != it.second)
    {
      if (it.first->second == &block)
        it.first = links_to.erase(it.first);
      else
        it.first++;
    }
  }

    WriteDestroyBlock(block);
}

JitBlock* JitBaseBlockCache::MoveBlockIntoFastCache(u32 addr, u32 msr)
{
  JitBlock* block = GetBlockFromStartAddress(addr, msr);

  if (!block)
    return nullptr;

    if (fast_block_map[block->fast_block_map_index] == block)
    fast_block_map[block->fast_block_map_index] = nullptr;

    size_t index = FastLookupIndexForAddress(addr);
  fast_block_map[index] = block;
  block->fast_block_map_index = index;

  return block;
}

size_t JitBaseBlockCache::FastLookupIndexForAddress(u32 address)
{
  return (address >> 2) & FAST_BLOCK_MAP_MASK;
}
