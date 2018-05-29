


#include <boost/test/unit_test.hpp>
#include <libethashseal/Ethash.h>
#include <libethcore/BasicAuthority.h>
#include <test/tools/libtesteth/TestOutputHelper.h>
#include <test/tools/libtesteth/Options.h>

using namespace std;
using namespace dev;
using namespace dev::test;
using namespace dev::eth;
using namespace boost;

void TestOutputHelper::initTest(size_t _maxTests)
{
	Ethash::init();
	NoProof::init();
	BasicAuthority::init();
	m_currentTestName = "n/a";
	m_currentTestFileName = string();
	m_execTimeResults = std::vector<execTimeName>();
	m_timer = Timer();
	m_timer.restart();
	m_currentTestCaseName = boost::unit_test::framework::current_test_case().p_name;
	if (!Options::get().createRandomTest)
		std::cout << "Test Case \"" + m_currentTestCaseName + "\": \n";
	m_maxTests = _maxTests;
	m_currTest = 0;
}

bool TestOutputHelper::checkTest(std::string const& _testName)
{
	if (test::Options::get().singleTest && test::Options::get().singleTestName != _testName)
		return false;

	m_currentTestName = _testName;
	return true;
}

void TestOutputHelper::showProgress()
{
	m_currTest++;
	int m_testsPerProgs = std::max(1, (int)(m_maxTests / 4));
	if (!test::Options::get().createRandomTest && (m_currTest % m_testsPerProgs == 0 || m_currTest ==  m_maxTests))
	{
		int percent = int(m_currTest*100/m_maxTests);
		std::cout << percent << "%";
		if (percent != 100)
			std::cout << "...";
		std::cout << "\n";
	}
}

void TestOutputHelper::finishTest()
{
	if (Options::get().exectimelog)
	{
		execTimeName res;
		res.first = m_timer.elapsed();
		res.second = caseName();
		std::cout << res.second + " time: " + toString(res.first) << "\n";
		m_execTimeResults.push_back(res);
	}
}

void TestOutputHelper::printTestExecStats()
{
	if (Options::get().exectimelog)
	{
		std::cout << std::left;
		std::sort(m_execTimeResults.begin(), m_execTimeResults.end(), [](execTimeName _a, execTimeName _b) { return (_b.first < _a.first); });
		for (size_t i = 0; i < m_execTimeResults.size(); i++)
			std::cout << setw(45) << m_execTimeResults[i].second << setw(25) << " time: " + toString(m_execTimeResults[i].first) << "\n";
	}
}
