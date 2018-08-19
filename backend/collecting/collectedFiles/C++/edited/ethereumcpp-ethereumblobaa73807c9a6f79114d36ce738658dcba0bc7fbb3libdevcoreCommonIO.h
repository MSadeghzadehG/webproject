


#pragma once

#include <map>
#include <set>
#include <unordered_set>
#include <array>
#include <list>
#include <vector>
#include <sstream>
#include <string>
#include <iosfwd>
#include <chrono>
#include "Common.h"
#include "CommonData.h"
#include <boost/filesystem.hpp>

namespace dev
{

std::string getPassword(std::string const& _prompt);

bytes contents(boost::filesystem::path const& _file);
bytesSec contentsSec(boost::filesystem::path const& _file);
std::string contentsString(boost::filesystem::path const& _file);

void writeFile(boost::filesystem::path const& _file, bytesConstRef _data, bool _writeDeleteRename = false);
inline void writeFile(boost::filesystem::path const& _file, bytes const& _data, bool _writeDeleteRename = false) { writeFile(_file, bytesConstRef(&_data), _writeDeleteRename); }

void copyDirectory(boost::filesystem::path const& _srcDir, boost::filesystem::path const& _dstDir);

std::string memDump(bytes const& _bytes, unsigned _width = 8, bool _html = false);


template <class T> struct StreamOut { static std::ostream& bypass(std::ostream& _out, T const& _t) { _out << _t; return _out; } };
template <> struct StreamOut<uint8_t> { static std::ostream& bypass(std::ostream& _out, uint8_t const& _t) { _out << (int)_t; return _out; } };

inline std::ostream& operator<<(std::ostream& _out, bytes const& _e) { _out << toHexPrefixed(_e); return _out; }
template <class T> inline std::ostream& operator<<(std::ostream& _out, std::vector<T> const& _e);
template <class T, std::size_t Z> inline std::ostream& operator<<(std::ostream& _out, std::array<T, Z> const& _e);
template <class T, class U> inline std::ostream& operator<<(std::ostream& _out, std::set<T, U> const& _e);
template <class T, class U> inline std::ostream& operator<<(std::ostream& _out, std::unordered_set<T, U> const& _e);

#if defined(_WIN32)
template <class T> inline std::string toString(std::chrono::time_point<T> const& _e, std::string const& _format = "%Y-%m-%d %H:%M:%S")
#else
template <class T> inline std::string toString(std::chrono::time_point<T> const& _e, std::string const& _format = "%F %T")
#endif
{
    unsigned long milliSecondsSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>(_e.time_since_epoch()).count();
    auto const durationSinceEpoch = std::chrono::milliseconds(milliSecondsSinceEpoch);
    std::chrono::time_point<std::chrono::system_clock> const tpAfterDuration(durationSinceEpoch);

    tm timeValue;
    auto time = std::chrono::system_clock::to_time_t(tpAfterDuration);
#if defined(_WIN32)
    gmtime_s(&timeValue, &time);
#else
    gmtime_r(&time, &timeValue);
#endif

    unsigned const millisRemainder = milliSecondsSinceEpoch % 1000;
    char buffer[1024];
    if (strftime(buffer, sizeof(buffer), _format.c_str(), &timeValue))
        return std::string(buffer) + "." + (millisRemainder < 1 ? "000" : millisRemainder < 10 ? "00" : millisRemainder < 100 ? "0" : "") + std::to_string(millisRemainder) + "Z";
    return std::string();
}

template <class T>
inline std::ostream& streamout(std::ostream& _out, std::vector<T> const& _e)
{
    _out << "[";
    if (!_e.empty())
    {
        StreamOut<T>::bypass(_out, _e.front());
        for (auto i = ++_e.begin(); i != _e.end(); ++i)
            StreamOut<T>::bypass(_out << ",", *i);
    }
    _out << "]";
    return _out;
}

template <class T> inline std::ostream& operator<<(std::ostream& _out, std::vector<T> const& _e) { streamout(_out, _e); return _out; } 
template <class T, std::size_t Z>
inline std::ostream& streamout(std::ostream& _out, std::array<T, Z> const& _e) {
    _out << "[";
    if (!_e.empty())
    {
        StreamOut<T>::bypass(_out, _e.front());
        auto i = _e.begin();
        for (++i; i != _e.end(); ++i)
            StreamOut<T>::bypass(_out << ",", *i);
    }
    _out << "]";
    return _out;
}
template <class T, std::size_t Z> inline std::ostream& operator<<(std::ostream& _out, std::array<T, Z> const& _e) { streamout(_out, _e); return _out; }

template <class T>
std::ostream& streamout(std::ostream& _out, std::set<T> const& _v)
{
    if (_v.empty())
        return _out << "{}";
    int i = 0;
    for (auto p: _v)
        _out << (!(i++) ? "{ " : ", ") << p;
    return _out << " }";
}
template <class T> inline std::ostream& operator<<(std::ostream& _out, std::set<T> const& _e) { streamout(_out, _e); return _out; }

template <class T>
std::ostream& streamout(std::ostream& _out, std::unordered_set<T> const& _v)
{
    if (_v.empty())
        return _out << "{}";
    int i = 0;
    for (auto p: _v)
        _out << (!(i++) ? "{ " : ", ") << p;
    return _out << " }";
}
template <class T> inline std::ostream& operator<<(std::ostream& _out, std::unordered_set<T> const& _e) { streamout(_out, _e); return _out; }


template <class _T>
inline std::string toString(_T const& _t)
{
    std::ostringstream o;
    o << _t;
    return o.str();
}

template <>
inline std::string toString<std::string>(std::string const& _s)
{
    return _s;
}

template <>
inline std::string toString<uint8_t>(uint8_t const& _u)
{
    std::ostringstream o;
    o << static_cast<uint16_t>(_u);
    return o.str();
}
}
