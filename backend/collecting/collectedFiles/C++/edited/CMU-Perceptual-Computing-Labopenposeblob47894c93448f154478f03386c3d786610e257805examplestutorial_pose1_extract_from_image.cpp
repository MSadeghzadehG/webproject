                        
#include <gflags/gflags.h>
#ifndef GFLAGS_GFLAGS_H_
    namespace gflags = google;
#endif
#include <openpose/core/headers.hpp>
#include <openpose/filestream/headers.hpp>
#include <openpose/gui/headers.hpp>
#include <openpose/pose/headers.hpp>
#include <openpose/utilities/headers.hpp>

DEFINE_int32(logging_level,             3,              "The logging level. Integer in the range [0, 255]. 0 will output any log() message, while"
                                                        " 255 will not output any. Current OpenPose library messages are in the range 0-4: 1 for"
                                                        " low priority messages and 4 for important ones.");
DEFINE_string(image_path,               "examples/media/COCO_val2014_000000000192.jpg",     "Process the desired image.");
DEFINE_string(model_pose,               "COCO",         "Model to be used. E.g. `COCO` (18 keypoints), `MPI` (15 keypoints, ~10% faster), "
                                                        "`MPI_4_layers` (15 keypoints, even faster but less accurate).");
DEFINE_string(model_folder,             "models/",      "Folder path (absolute or relative) where the models (pose, face, ...) are located.");
DEFINE_string(net_resolution,           "-1x368",       "Multiples of 16. If it is increased, the accuracy potentially increases. If it is"
                                                        " decreased, the speed increases. For maximum speed-accuracy balance, it should keep the"
                                                        " closest aspect ratio possible to the images or videos to be processed. Using `-1` in"
                                                        " any of the dimensions, OP will choose the optimal aspect ratio depending on the user's"
                                                        " input value. E.g. the default `-1x368` is equivalent to `656x368` in 16:9 resolutions,"
                                                        " e.g. full HD (1980x1080) and HD (1280x720) resolutions.");
DEFINE_string(output_resolution,        "-1x-1",        "The image resolution (display and output). Use \"-1x-1\" to force the program to use the"
                                                        " input image resolution.");
DEFINE_int32(num_gpu_start,             0,              "GPU device start number.");
DEFINE_double(scale_gap,                0.3,            "Scale gap between scales. No effect unless scale_number > 1. Initial scale is always 1."
                                                        " If you want to change the initial scale, you actually want to multiply the"
                                                        " `net_resolution` by your desired initial scale.");
DEFINE_int32(scale_number,              1,              "Number of scales to average.");
DEFINE_bool(disable_blending,           false,          "If enabled, it will render the results (keypoint skeletons or heatmaps) on a black"
                                                        " background, instead of being rendered into the original image. Related: `part_to_show`,"
                                                        " `alpha_pose`, and `alpha_pose`.");
DEFINE_double(render_threshold,         0.05,           "Only estimated keypoints whose score confidences are higher than this threshold will be"
                                                        " rendered. Generally, a high threshold (> 0.5) will only render very clear body parts;"
                                                        " while small thresholds (~0.1) will also output guessed and occluded keypoints, but also"
                                                        " more false positives (i.e. wrong detections).");
DEFINE_double(alpha_pose,               0.6,            "Blending factor (range 0-1) for the body part rendering. 1 will show it completely, 0 will"
                                                        " hide it. Only valid for GPU rendering.");

int openPoseTutorialPose1()
{
    try
    {
        op::log("Starting OpenPose demo...", op::Priority::High);
        const auto timerBegin = std::chrono::high_resolution_clock::now();

                                                op::check(0 <= FLAGS_logging_level && FLAGS_logging_level <= 255, "Wrong logging_level value.",
                  __LINE__, __FUNCTION__, __FILE__);
        op::ConfigureLog::setPriorityThreshold((op::Priority)FLAGS_logging_level);
        op::log("", op::Priority::Low, __LINE__, __FUNCTION__, __FILE__);
                        const auto outputSize = op::flagsToPoint(FLAGS_output_resolution, "-1x-1");
                const auto netInputSize = op::flagsToPoint(FLAGS_net_resolution, "-1x368");
                const auto poseModel = op::flagsToPoseModel(FLAGS_model_pose);
                if (FLAGS_alpha_pose < 0. || FLAGS_alpha_pose > 1.)
            op::error("Alpha value for blending must be in the range [0,1].", __LINE__, __FUNCTION__, __FILE__);
        if (FLAGS_scale_gap <= 0. && FLAGS_scale_number > 1)
            op::error("Incompatible flag configuration: scale_gap must be greater than 0 or scale_number = 1.",
                      __LINE__, __FUNCTION__, __FILE__);
                op::log("", op::Priority::Low, __LINE__, __FUNCTION__, __FILE__);
                op::ScaleAndSizeExtractor scaleAndSizeExtractor(netInputSize, outputSize, FLAGS_scale_number, FLAGS_scale_gap);
        op::CvMatToOpInput cvMatToOpInput{poseModel};
        op::CvMatToOpOutput cvMatToOpOutput;
        op::PoseExtractorCaffe poseExtractorCaffe{poseModel, FLAGS_model_folder, FLAGS_num_gpu_start};
        op::PoseCpuRenderer poseRenderer{poseModel, (float)FLAGS_render_threshold, !FLAGS_disable_blending,
                                         (float)FLAGS_alpha_pose};
        op::OpOutputToCvMat opOutputToCvMat;
        op::FrameDisplayer frameDisplayer{"OpenPose Tutorial - Example 1", outputSize};
                poseExtractorCaffe.initializationOnThread();
        poseRenderer.initializationOnThread();

                                cv::Mat inputImage = op::loadImage(FLAGS_image_path, CV_LOAD_IMAGE_COLOR);
        if(inputImage.empty())
            op::error("Could not open or find the image: " + FLAGS_image_path, __LINE__, __FUNCTION__, __FILE__);
        const op::Point<int> imageSize{inputImage.cols, inputImage.rows};
                std::vector<double> scaleInputToNetInputs;
        std::vector<op::Point<int>> netInputSizes;
        double scaleInputToOutput;
        op::Point<int> outputResolution;
        std::tie(scaleInputToNetInputs, netInputSizes, scaleInputToOutput, outputResolution)
            = scaleAndSizeExtractor.extract(imageSize);
                const auto netInputArray = cvMatToOpInput.createArray(inputImage, scaleInputToNetInputs, netInputSizes);
        auto outputArray = cvMatToOpOutput.createArray(inputImage, scaleInputToOutput, outputResolution);
                poseExtractorCaffe.forwardPass(netInputArray, imageSize, scaleInputToNetInputs);
        const auto poseKeypoints = poseExtractorCaffe.getPoseKeypoints();
                poseRenderer.renderPose(outputArray, poseKeypoints, scaleInputToOutput);
                auto outputImage = opOutputToCvMat.formatToCvMat(outputArray);

                        frameDisplayer.displayFrame(outputImage, 0);                 const auto now = std::chrono::high_resolution_clock::now();
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

        return openPoseTutorialPose1();
}
