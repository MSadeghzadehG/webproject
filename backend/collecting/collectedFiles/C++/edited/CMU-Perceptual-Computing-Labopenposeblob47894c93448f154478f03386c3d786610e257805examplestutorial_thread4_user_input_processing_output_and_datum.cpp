                        
#include <gflags/gflags.h>
#ifndef GFLAGS_GFLAGS_H_
    namespace gflags = google;
#endif
#include <openpose/headers.hpp>

DEFINE_int32(logging_level,             3,              "The logging level. Integer in the range [0, 255]. 0 will output any log() message, while"
                                                        " 255 will not output any. Current OpenPose library messages are in the range 0-4: 1 for"
                                                        " low priority messages and 4 for important ones.");
DEFINE_string(image_dir,                "examples/media/",      "Process a directory of images. Read all standard formats (jpg, png, bmp, etc.).");
DEFINE_bool(fullscreen,                 false,          "Run in full-screen mode (press f during runtime to toggle).");


struct UserDatum : public op::Datum
{
    bool boolThatUserNeedsForSomeReason;

    UserDatum(const bool boolThatUserNeedsForSomeReason_ = false) :
        boolThatUserNeedsForSomeReason{boolThatUserNeedsForSomeReason_}
    {}
};


class WUserInput : public op::WorkerProducer<std::shared_ptr<std::vector<UserDatum>>>
{
public:
    WUserInput(const std::string& directoryPath) :
        mImageFiles{op::getFilesOnDirectory(directoryPath, "jpg")},
                        mCounter{0}
    {
        if (mImageFiles.empty())
            op::error("No images found on: " + directoryPath, __LINE__, __FUNCTION__, __FILE__);
    }

    void initializationOnThread() {}

    std::shared_ptr<std::vector<UserDatum>> workProducer()
    {
        try
        {
                        if (mImageFiles.size() <= mCounter)
            {
                op::log("Last frame read and added to queue. Closing program after it is processed.",
                        op::Priority::High);
                                                this->stop();
                return nullptr;
            }
            else
            {
                                auto datumsPtr = std::make_shared<std::vector<UserDatum>>();
                datumsPtr->emplace_back();
                auto& datum = datumsPtr->at(0);

                                datum.cvInputData = cv::imread(mImageFiles.at(mCounter++));

                                if (datum.cvInputData.empty())
                {
                    op::log("Empty frame detected on path: " + mImageFiles.at(mCounter-1) + ". Closing program.",
                        op::Priority::High);
                    this->stop();
                    datumsPtr = nullptr;
                }

                return datumsPtr;
            }
        }
        catch (const std::exception& e)
        {
            op::log("Some kind of unexpected error happened.");
            this->stop();
            op::error(e.what(), __LINE__, __FUNCTION__, __FILE__);
            return nullptr;
        }
    }

private:
    const std::vector<std::string> mImageFiles;
    unsigned long long mCounter;
};

class WUserPostProcessing : public op::Worker<std::shared_ptr<std::vector<UserDatum>>>
{
public:
    WUserPostProcessing()
    {
            }

    void initializationOnThread() {}

    void work(std::shared_ptr<std::vector<UserDatum>>& datumsPtr)
    {
                                        try
        {
            if (datumsPtr != nullptr && !datumsPtr->empty())
                for (auto& datum : *datumsPtr)
                    cv::bitwise_not(datum.cvInputData, datum.cvOutputData);
        }
        catch (const std::exception& e)
        {
            op::log("Some kind of unexpected error happened.");
            this->stop();
            op::error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }
};

class WUserOutput : public op::WorkerConsumer<std::shared_ptr<std::vector<UserDatum>>>
{
public:
    void initializationOnThread() {}

    void workConsumer(const std::shared_ptr<std::vector<UserDatum>>& datumsPtr)
    {
        try
        {
                                                        if (datumsPtr != nullptr && !datumsPtr->empty())
            {
                cv::imshow("User worker GUI", datumsPtr->at(0).cvOutputData);
                                cv::waitKey(1);
            }
        }
        catch (const std::exception& e)
        {
            op::log("Some kind of unexpected error happened.");
            this->stop();
            op::error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        }
    }
};

int openPoseTutorialThread4()
{
    try
    {
        op::log("Starting OpenPose demo...", op::Priority::High);
        const auto timerBegin = std::chrono::high_resolution_clock::now();

                                                op::check(0 <= FLAGS_logging_level && FLAGS_logging_level <= 255, "Wrong logging_level value.",
                  __LINE__, __FUNCTION__, __FILE__);
        op::ConfigureLog::setPriorityThreshold((op::Priority)FLAGS_logging_level);
                typedef std::shared_ptr<std::vector<UserDatum>> TypedefDatums;
        typedef std::shared_ptr<op::Worker<TypedefDatums>> TypedefWorker;
        op::ThreadManager<TypedefDatums> threadManager;
                        TypedefWorker wUserInput = std::make_shared<WUserInput>(FLAGS_image_dir);
                TypedefWorker wUserProcessing = std::make_shared<WUserPostProcessing>();
                TypedefWorker wUserOutput = std::make_shared<WUserOutput>();

                                                                                                auto threadId = 0ull;
        auto queueIn = 0ull;
        auto queueOut = 1ull;
        threadManager.add(threadId++, wUserInput, queueIn++, queueOut++);               threadManager.add(threadId++, wUserProcessing, queueIn++, queueOut++);          threadManager.add(threadId++, wUserOutput, queueIn++, queueOut++);      
                op::log("Starting thread(s)...", op::Priority::High);
                            threadManager.exec();                                                                                                      
                        const auto now = std::chrono::high_resolution_clock::now();
        const auto totalTimeSec = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(now-timerBegin).count()
                                * 1e-9;
        const auto message = "OpenPose demo successfully finished. Total time: "
                           + std::to_string(totalTimeSec) + " seconds.";
        op::log(message, op::Priority::High);
                return 0;
    }
    catch (const std::exception& e)
    {
        op::error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        return -1;
    }
}

int main(int argc, char *argv[])
{
        gflags::ParseCommandLineFlags(&argc, &argv, true);

        return openPoseTutorialThread4();
}
