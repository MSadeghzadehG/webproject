


#pragma once
#include <test/tools/libtestutils/Common.h>
#include <test/tools/libtesteth/JsonSpiritHeaders.h>

namespace dev
{
namespace test
{

class TestOutputHelper
{
public:
	static TestOutputHelper& get()
	{
		static TestOutputHelper instance;
		return instance;
	}
	TestOutputHelper(TestOutputHelper const&) = delete;
	void operator=(TestOutputHelper const&) = delete;

	void initTest(size_t _maxTests = 1);
		void showProgress();
	void finishTest();

		bool checkTest(std::string const& _testName);
	void setCurrentTestFile(boost::filesystem::path const& _name) { m_currentTestFileName = _name; }
	void setCurrentTestName(std::string const& _name) { m_currentTestName = _name; }
	std::string const& testName() { return m_currentTestName; }
	std::string const& caseName() { return m_currentTestCaseName; }
	boost::filesystem::path const& testFile() { return m_currentTestFileName; }
	void printTestExecStats();

private:
	TestOutputHelper() {}
	Timer m_timer;
	size_t m_currTest;
	size_t m_maxTests;
	std::string m_currentTestName;
	std::string m_currentTestCaseName;
	boost::filesystem::path m_currentTestFileName;
	typedef std::pair<double, std::string> execTimeName;
	std::vector<execTimeName> m_execTimeResults;
};

class TestOutputHelperFixture
{
public:
	TestOutputHelperFixture() { TestOutputHelper::get().initTest(); }
	~TestOutputHelperFixture() { TestOutputHelper::get().finishTest(); }
};

} } 