


#include "Log.h"

#ifdef __APPLE__
#include <pthread.h>
#endif

#include <boost/core/null_deleter.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/attributes/function.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/exception_handler.hpp>

namespace dev
{
namespace
{
struct ThreadLocalLogName
{
    ThreadLocalLogName(std::string const& _name) { m_name.reset(new std::string(_name)); }
    boost::thread_specific_ptr<std::string> m_name;
};

ThreadLocalLogName g_logThreadName("main");
}  
std::string getThreadName()
{
#if defined(__GLIBC__) || defined(__APPLE__)
    char buffer[128];
    pthread_getname_np(pthread_self(), buffer, 127);
    buffer[127] = 0;
    return buffer;
#else
    return g_logThreadName.m_name.get() ? *g_logThreadName.m_name.get() : "<unknown>";
#endif
}

void setThreadName(std::string const& _n)
{
#if defined(__GLIBC__)
    pthread_setname_np(pthread_self(), _n.c_str());
#elif defined(__APPLE__)
    pthread_setname_np(_n.c_str());
#else
    g_logThreadName.m_name.reset(new std::string(_n));
#endif
}

BOOST_LOG_ATTRIBUTE_KEYWORD(channel, "Channel", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(context, "Context", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(threadName, "ThreadName", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)

void setupLogging(LoggingOptions const& _options)
{
    auto sink = boost::make_shared<
        boost::log::sinks::asynchronous_sink<boost::log::sinks::text_ostream_backend>>();

    boost::shared_ptr<std::ostream> stream{&std::cout, boost::null_deleter{}};
    sink->locked_backend()->add_stream(stream);
    sink->set_filter([_options](boost::log::attribute_value_set const& _set) {
        if (_set["Severity"].extract<int>() > _options.verbosity)
            return false;

        auto const messageChannel = _set[channel];
        return (_options.includeChannels.empty() ||
                   contains(_options.includeChannels, messageChannel)) &&
               !contains(_options.excludeChannels, messageChannel);
    });

    namespace expr = boost::log::expressions;
    sink->set_formatter(expr::stream
                        << EthViolet << expr::format_date_time(timestamp, "%Y-%m-%d %H:%M:%S")
                        << EthReset " " EthNavy << threadName << EthReset " " << channel
                        << expr::if_(expr::has_attr(
                               context))[expr::stream << " " EthNavy << context << EthReset]
                        << " " << expr::smessage);

    boost::log::core::get()->add_sink(sink);

    boost::log::core::get()->add_global_attribute(
        "ThreadName", boost::log::attributes::make_function(&getThreadName));
    boost::log::core::get()->add_global_attribute(
        "TimeStamp", boost::log::attributes::local_clock());

    boost::log::core::get()->set_exception_handler(
        boost::log::make_exception_handler<std::exception>([](std::exception const& _ex) {
        std::cerr << "Exception from the logging library: " << _ex.what() << '\n';
    }));
}

}  