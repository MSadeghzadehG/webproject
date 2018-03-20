#include <ctime> #include <fstream> #include <iostream> #include <stdexcept> #include <openpose/utilities/errorAndLog.hpp>

namespace op
{
        bool checkIfErrorHas(const ErrorMode errorMode)
    {
        for (const auto& sErrorMode : ConfigureError::getErrorModes())
            if (sErrorMode == errorMode || sErrorMode == ErrorMode::All)
                return true;
        return false;
    }

    bool checkIfLoggingHas(const LogMode loggingMode)
    {
        for (const auto& sLoggingMode : ConfigureLog::getLogModes())
            if (sLoggingMode == loggingMode || sLoggingMode == LogMode::All)
                return true;
        return false;
    }

    std::string createFullMessage(const std::string& message, const int line = -1, const std::string& function = "",
                                  const std::string& file = "")
    {
        const auto hasMessage = (!message.empty());
        const auto hasLocation = (line != -1 || !function.empty() || !file.empty());

        std::string fullMessage;

        if (hasMessage)
        {
            fullMessage += message;
                                                                                                            
            if (hasLocation)
            {
                if (*message.crbegin() != '.')
                    fullMessage += " in ";
                else
                    fullMessage += " In ";
            }
        }

        if (hasLocation)
            fullMessage += file + ":" + function + "():" + std::to_string(line);

        else if (!hasMessage)             fullMessage += "[Undefined]";

        return fullMessage;
    }

    std::string getTime()
    {
                std::time_t rawtime;
        struct std::tm timeStruct;
        std::time(&rawtime);
        timeStruct = *localtime(&rawtime);

                                
                timeStruct.tm_mon++;
        timeStruct.tm_year += 1900;
        return std::to_string(timeStruct.tm_year) + '_' + std::to_string(timeStruct.tm_mon)
               + '_' + std::to_string(timeStruct.tm_mday) + "___" + std::to_string(timeStruct.tm_hour)
               + '_' + std::to_string(timeStruct.tm_min) + '_' + std::to_string(timeStruct.tm_sec);
    }

    void fileLogging(const std::string& message)
    {
        std::string fileToOpen{"errorLogging.txt"};

                std::ifstream in{fileToOpen, std::ios::binary | std::ios::ate};
        const auto currentSizeBytes = in.tellg();
        in.close();

                const auto maxLogSize = 15 * 1024 * 1024;         std::ofstream loggingFile{fileToOpen,
                                  (currentSizeBytes < maxLogSize ? std::ios_base::app : std::ios_base::trunc)};

                loggingFile << getTime();
        loggingFile << "\n";
        loggingFile << message;
        loggingFile << "\n\n\n\n\n";

        loggingFile.close();
    }





        void error(const std::string& message, const int line, const std::string& function, const std::string& file)
    {
        const std::string errorInit = "\nError:\n";
        const std::string errorEnum = "- ";

                std::string errorMessageToPropagate;
        std::string errorMessageToPrint;
                if (message.size() < errorInit.size() || message.substr(0, errorInit.size()) != errorInit)
        {
            errorMessageToPrint = errorInit + createFullMessage(message) + "\n\nComing from:\n" + errorEnum
                                + createFullMessage("", line, function, file);
            errorMessageToPropagate = errorMessageToPrint + "\n";
        }
                else
        {
            errorMessageToPrint = errorEnum + createFullMessage("", line, function, file);
            errorMessageToPropagate = createFullMessage(message.substr(0, message.size()-1)) + "\n"
                                    + errorMessageToPrint + "\n";
        }

                if (checkIfErrorHas(ErrorMode::StdCerr))
            std::cerr << errorMessageToPrint << std::endl;

                if (checkIfErrorHas(ErrorMode::FileLogging))
            fileLogging(errorMessageToPrint);

                if (checkIfErrorHas(ErrorMode::StdRuntimeError))
            throw std::runtime_error{errorMessageToPropagate};
    }

    void log(const std::string& message, const Priority priority, const int line, const std::string& function,
             const std::string& file)
    {
        if (priority >= ConfigureLog::getPriorityThreshold())
        {
            const auto infoMessage = createFullMessage(message, line, function, file);

                        if (checkIfLoggingHas(LogMode::StdCout))
                std::cout << infoMessage << std::endl;

                        if (checkIfLoggingHas(LogMode::FileLogging))
                fileLogging(infoMessage);
        }
    }





            std::vector<ErrorMode> sErrorModes              {ErrorMode::StdCerr, ErrorMode::StdRuntimeError};
    std::mutex sErrorModesMutex                     {};

    std::vector<ErrorMode> ConfigureError::getErrorModes()
    {
        const std::lock_guard<std::mutex> lock{sErrorModesMutex};
        return sErrorModes;
    }

    void ConfigureError::setErrorModes(const std::vector<ErrorMode>& errorModes)
    {
        const std::lock_guard<std::mutex> lock{sErrorModesMutex};
        sErrorModes = errorModes;
    }





        std::atomic<Priority> sPriorityThreshold        {Priority::High};
        std::vector<LogMode> sLoggingModes              {LogMode::StdCout};
    std::mutex sConfigureLogMutex                   {};

    Priority ConfigureLog::getPriorityThreshold()
    {
        return sPriorityThreshold;
    }

    const std::vector<LogMode>& ConfigureLog::getLogModes()
    {
        const std::lock_guard<std::mutex> lock{sConfigureLogMutex};
        return sLoggingModes;
    }

    void ConfigureLog::setPriorityThreshold(const Priority priorityThreshold)
    {
        sPriorityThreshold = priorityThreshold;
    }

    void ConfigureLog::setLogModes(const std::vector<LogMode>& loggingModes)
    {
        const std::lock_guard<std::mutex> lock{sConfigureLogMutex};
        sLoggingModes = loggingModes;
    }
}
