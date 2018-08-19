
#pragma once




#include <array>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Common/CommonTypes.h"
#include "Common/Swap.h"

namespace DiscIO
{
enum class BlobType
{
  PLAIN,
  DRIVE,
  DIRECTORY,
  GCZ,
  CISO,
  WBFS,
  TGC
};

class BlobReader
{
public:
  virtual ~BlobReader() {}
  virtual BlobType GetBlobType() const = 0;
  virtual u64 GetRawSize() const = 0;
  virtual u64 GetDataSize() const = 0;

    virtual bool Read(u64 offset, u64 size, u8* out_ptr) = 0;
  template <typename T>
  std::optional<T> ReadSwapped(u64 offset)
  {
    T temp;
    if (!Read(offset, sizeof(T), reinterpret_cast<u8*>(&temp)))
      return {};
    return Common::FromBigEndian(temp);
  }

  virtual bool SupportsReadWiiDecrypted() const { return false; }
  virtual bool ReadWiiDecrypted(u64 offset, u64 size, u8* out_ptr, u64 partition_offset)
  {
    return false;
  }

protected:
  BlobReader() {}
};

class SectorReader : public BlobReader
{
public:
  virtual ~SectorReader() = 0;

  bool Read(u64 offset, u64 size, u8* out_ptr) override;

protected:
  void SetSectorSize(int blocksize);
  int GetSectorSize() const { return m_block_size; }
            void SetChunkSize(int blocks);
  int GetChunkSize() const { return m_chunk_blocks; }
    virtual bool GetBlock(u64 block_num, u8* out) = 0;

        virtual bool ReadMultipleAlignedBlocks(u64 block_num, u64 num_blocks, u8* out_ptr);

private:
  struct Cache
  {
    std::vector<u8> data;
    u64 block_idx = 0;
    u32 num_blocks = 0;

                        u32 lru_sreg = 0;

    void Reset()
    {
      block_idx = 0;
      num_blocks = 0;
      lru_sreg = 0;
    }
    void Fill(u64 block, u32 count)
    {
      block_idx = block;
      num_blocks = count;
                              MarkUsed();
    }
    bool Contains(u64 block) const { return block >= block_idx && block - block_idx < num_blocks; }
    void MarkUsed() { lru_sreg |= 0x80000000; }
    void ShiftLRU() { lru_sreg >>= 1; }
    bool IsLessRecentlyUsedThan(const Cache& other) const { return lru_sreg < other.lru_sreg; }
  };

      const Cache* FindCacheLine(u64 block_num);

    Cache* GetEmptyCacheLine();

        const Cache* GetCacheLine(u64 block_num);

          u32 ReadChunk(u8* buffer, u64 chunk_num);

  static constexpr int CACHE_LINES = 32;
  u32 m_block_size = 0;      u32 m_chunk_blocks = 1;    std::array<Cache, CACHE_LINES> m_cache;
};

std::unique_ptr<BlobReader> CreateBlobReader(const std::string& filename);

typedef bool (*CompressCB)(const std::string& text, float percent, void* arg);

bool CompressFileToBlob(const std::string& infile_path, const std::string& outfile_path,
                        u32 sub_type = 0, int sector_size = 16384, CompressCB callback = nullptr,
                        void* arg = nullptr);
bool DecompressBlobToFile(const std::string& infile_path, const std::string& outfile_path,
                          CompressCB callback = nullptr, void* arg = nullptr);

}  