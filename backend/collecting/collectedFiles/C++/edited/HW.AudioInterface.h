

#pragma once

#include "Common/CommonTypes.h"

class PointerWrap;
namespace MMIO
{
class Mapping;
}

namespace AudioInterface
{
void Init();
void Shutdown();
void DoState(PointerWrap& p);
bool IsPlaying();

void RegisterMMIO(MMIO::Mapping* mmio, u32 base);

unsigned int GetAIDSampleRate();

void GenerateAISInterrupt();

}  