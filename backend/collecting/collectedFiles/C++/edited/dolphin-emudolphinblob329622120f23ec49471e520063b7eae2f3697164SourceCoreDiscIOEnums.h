
#pragma once

#include <string>

#include "Common/CommonTypes.h"

namespace DiscIO
{

enum class Platform
{
  GameCubeDisc = 0,
  WiiDisc,
  WiiWAD,
  ELFOrDOL,
  NumberOfPlatforms
};

enum class Country
{
  Europe = 0,
  Japan,
  USA,
  Australia,
  France,
  Germany,
  Italy,
  Korea,
  Netherlands,
  Russia,
  Spain,
  Taiwan,
  World,
  Unknown,
  NumberOfCountries
};

enum class Region
{
  NTSC_J = 0,     NTSC_U = 1,     PAL = 2,        Unknown = 3,    NTSC_K = 4    };

enum class Language
{
  Japanese = 0,
  English = 1,
  German = 2,
  French = 3,
  Spanish = 4,
  Italian = 5,
  Dutch = 6,
  SimplifiedChinese = 7,
  TraditionalChinese = 8,
  Korean = 9,
  Unknown
};

std::string GetName(Country country, bool translate);
std::string GetName(Language language, bool translate);

bool IsDisc(Platform volume_type);
bool IsWii(Platform volume_type);
bool IsNTSC(Region region);

Country TypicalCountryForRegion(Region region);
Region RegionSwitchGC(u8 country_code);
Region RegionSwitchWii(u8 country_code);
Country CountrySwitch(u8 country_code);

Region GetSysMenuRegion(u16 title_version);
std::string GetSysMenuVersionString(u16 title_version);

const std::string& GetCompanyFromID(const std::string& company_id);
}
