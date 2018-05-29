#ifndef OPENPOSE_UTILITIES_PROFILER_HPP
#define OPENPOSE_UTILITIES_PROFILER_HPP

#include <string>
#include <openpose/core/macros.hpp>


                        
namespace op
{
    class OP_API Profiler
    {
    public:
        static unsigned long long DEFAULT_X;

                static void setDefaultX(const unsigned long long defaultX);

        static const std::string timerInit(const int line, const std::string& function, const std::string& file);

        static void timerEnd(const std::string& key);

        static void printAveragedTimeMsOnIterationX(const std::string& key, const int line,
                                                    const std::string& function, const std::string& file,
                                                    const unsigned long long x = DEFAULT_X);

        static void printAveragedTimeMsEveryXIterations(const std::string& key, const int line,
                                                        const std::string& function, const std::string& file,
                                                        const unsigned long long x = DEFAULT_X);

        static void profileGpuMemory(const int line, const std::string& function, const std::string& file);
    };
}

#endif 