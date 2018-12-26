
#pragma once

#include <string>

#include "Common/CommonTypes.h"

namespace DiscIO
{

enum class Platform
{
  GAMECUBE_DISC = 0,
  WII_DISC,
  WII_WAD,
  ELF_DOL,
  NUMBER_OF_PLATFORMS
};

enum class Country
{
  COUNTRY_EUROPE = 0,
  COUNTRY_JAPAN,
  COUNTRY_USA,
  COUNTRY_AUSTRALIA,
  COUNTRY_FRANCE,
  COUNTRY_GERMANY,
  COUNTRY_ITALY,
  COUNTRY_KOREA,
  COUNTRY_NETHERLANDS,
  COUNTRY_RUSSIA,
  COUNTRY_SPAIN,
  COUNTRY_TAIWAN,
  COUNTRY_WORLD,
  COUNTRY_UNKNOWN,
  NUMBER_OF_COUNTRIES
};

enum class Region
{
  NTSC_J = 0,            NTSC_U = 1,            PAL = 2,               UNKNOWN_REGION = 3,    NTSC_K = 4           };

enum class Language
{
  LANGUAGE_JAPANESE = 0,
  LANGUAGE_ENGLISH = 1,
  LANGUAGE_GERMAN = 2,
  LANGUAGE_FRENCH = 3,
  LANGUAGE_SPANISH = 4,
  LANGUAGE_ITALIAN = 5,
  LANGUAGE_DUTCH = 6,
  LANGUAGE_SIMPLIFIED_CHINESE = 7,
  LANGUAGE_TRADITIONAL_CHINESE = 8,
  LANGUAGE_KOREAN = 9,
  LANGUAGE_UNKNOWN
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
