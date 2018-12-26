                        
#include <gflags/gflags.h>
#ifndef GFLAGS_GFLAGS_H_
    namespace gflags = google;
#endif
#include <openpose/core/headers.hpp>
#include <openpose/gui/headers.hpp>
#include <openpose/producer/headers.hpp>
#include <openpose/thread/headers.hpp>
#include <openpose/utilities/headers.hpp>

DEFINE_int32(logging_level,             3,              "The logging level. Integer in the range [0, 255]. 0 will output any log() message, while"
                                                        " 255 will not output any. Current OpenPose library messages are in the range 0-4: 1 for"
                                                        " low priority messages and 4 for important ones.");
DEFINE_int32(camera,                    -1,             "The camera index for cv::VideoCapture. Integer in the range [0, 9]. Select a negative"
                                                        " number (by default), to auto-detect and open the first available camera.");
DEFINE_string(camera_resolution,        "-1x-1",        "Set the camera resolution (either `--camera` or `--flir_camera`). `-1x-1` will use the"
                                                        " default 1280x720 for `--camera`, or the maximum flir camera resolution available for"
                                                        " `--flir_camera`");
DEFINE_double(camera_fps,               30.0,           "Frame rate for the webcam (also used when saving video). Set this value to the minimum"
                                                        " value between the OpenPose displayed speed and the webcam real frame rate.");
DEFINE_string(video,                    "",             "Use a video file instead of the camera. Use `examples/media/video.avi` for our default"
                                                        " example video.");
DEFINE_string(image_dir,                "",             "Process a directory of images. Use `examples/media/` for our default example folder with 20"
                                                        " images. Read all standard formats (jpg, png, bmp, etc.).");
DEFINE_bool(flir_camera,                false,          "Whether to use FLIR (Point-Grey) stereo camera.");
DEFINE_string(ip_camera,                "",             "String with the IP camera URL. It supports protocols like RTSP and HTTP.");
DEFINE_bool(process_real_time,          false,          "Enable to keep the original source frame rate (e.g. for video). If the processing time is"
                                                        " too long, it will skip frames. If it is too fast, it will slow it down.");
DEFINE_string(camera_parameter_folder,  "models/cameraParameters/flir/", "String with the folder where the camera parameters are located.");
DEFINE_string(output_resolution,        "-1x-1",        "The image resolution (display and output). Use \"-1x-1\" to force the program to use the"
                                                        " input image resolution.");
DEFINE_int32(3d_views,                  1,              "Complementary option to `--image_dir` or `--video`. OpenPose will read as many images per"
                                                        " iteration, allowing tasks such as stereo camera processing (`--3d`). Note that"
                                                        " `--camera_parameters_folder` must be set. OpenPose must find as many `xml` files in the"
                                                        " parameter folder as this number indicates.");
DEFINE_bool(fullscreen,                 false,          "Run in full-screen mode (press f during runtime to toggle).");

class WUserClass : public op::Worker<std::shared_ptr<std::vector<op::Datum>>>
{
public:
    WUserClass()
    {
            }

    void initializationOnThread() {}

    void work(std::shared_ptr<std::vector<op::Datum>>& datumsPtr)
    {
        try
        {
                                                        if (datumsPtr != nullptr)
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

int openPoseTutorialThread2()
{
    op::log("OpenPose Library Tutorial - Example 3.", op::Priority::High);
                            op::check(0 <= FLAGS_logging_level && FLAGS_logging_level <= 255, "Wrong logging_level value.",
              __LINE__, __FUNCTION__, __FILE__);
    op::ConfigureLog::setPriorityThreshold((op::Priority)FLAGS_logging_level);
            const auto outputSize = op::flagsToPoint(FLAGS_output_resolution, "-1x-1");
        const auto producerSharedPtr = op::flagsToProducer(FLAGS_image_dir, FLAGS_video, FLAGS_ip_camera, FLAGS_camera,
                                                       FLAGS_flir_camera, FLAGS_camera_resolution, FLAGS_camera_fps,
                                                       FLAGS_camera_parameter_folder,
                                                       (unsigned int) FLAGS_3d_views);
    const auto displayProducerFpsMode = (FLAGS_process_real_time
                                      ? op::ProducerFpsMode::OriginalFps : op::ProducerFpsMode::RetrievalFps);
    producerSharedPtr->setProducerFpsMode(displayProducerFpsMode);
    op::log("", op::Priority::Low, __LINE__, __FUNCTION__, __FILE__);
        auto videoSeekSharedPtr = std::make_shared<std::pair<std::atomic<bool>, std::atomic<int>>>();
    videoSeekSharedPtr->first = false;
    videoSeekSharedPtr->second = 0;
    const op::Point<int> producerSize{(int)producerSharedPtr->get(CV_CAP_PROP_FRAME_WIDTH),
                                (int)producerSharedPtr->get(CV_CAP_PROP_FRAME_HEIGHT)};
        typedef std::vector<op::Datum> TypedefDatumsNoPtr;
    typedef std::shared_ptr<TypedefDatumsNoPtr> TypedefDatums;
    op::ThreadManager<TypedefDatums> threadManager;
            auto DatumProducer = std::make_shared<op::DatumProducer<TypedefDatumsNoPtr>>(producerSharedPtr);
    auto wDatumProducer = std::make_shared<op::WDatumProducer<TypedefDatums, TypedefDatumsNoPtr>>(DatumProducer);
        auto wUserClass = std::make_shared<WUserClass>();
        auto gui = std::make_shared<op::Gui>(outputSize, FLAGS_fullscreen, threadManager.getIsRunningSharedPtr());
    auto wGui = std::make_shared<op::WGui<TypedefDatums>>(gui);

                                                            auto threadId = 0ull;
    auto queueIn = 0ull;
    auto queueOut = 1ull;
    threadManager.add(threadId++, wDatumProducer, queueIn++, queueOut++);       threadManager.add(threadId++, wUserClass, queueIn++, queueOut++);           threadManager.add(threadId++, wGui, queueIn++, queueOut++);             
                            
                                    
        op::log("Starting thread(s)", op::Priority::High);
                threadManager.exec();                                                      
            op::log("Example 2 successfully finished.", op::Priority::High);
        return 0;
}

int main(int argc, char *argv[])
{
        gflags::ParseCommandLineFlags(&argc, &argv, true);

        return openPoseTutorialThread2();
}
