


#pragma once

#include <string>

namespace dev
{


class TransientDirectory
{
public:
	TransientDirectory();
	TransientDirectory(std::string const& _path);
	~TransientDirectory();

	std::string const& path() const { return m_path; }

private:
	std::string m_path;
};

}
