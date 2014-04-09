#include <cstdlib>
#include <cstring>
#include <dirent.h>

#include <list>
#include <string>
#include <iostream>

#include <opencv2/imgproc/imgproc.hpp>

#include "GUI.hpp"
#include "Draw.hpp"
#include "ImageProcessing.hpp"


cv::Mat processingFunction(cv::Mat& image);

void print_usage(char* name)
{
    std::cerr << "Usage: " << name << " [-v <dev index> | -f <in file> | -d <in dir>]\n"
              << "       video device 0 is used by default if no options are specified\n"
              << "  <dev index> - input video device\n"
              << "    <in file> - input video file\n"
              << "     <in dir> - input directory containing only image files\n"
              << "Hot keys:\n"
              << "          ESC - exit the program\n"
              << "   LEFT ARROW - previous image (only when the -d option is used)\n"
              << "  RIGHT ARROW - next image (only when the -d option is used)"
              << std::endl;
}

void print_missing_option(char* name, const std::string& arg)
{
    std::cerr << name << ": missing option after '" << arg << "'" << std::endl;
}

int main(int argc, char** argv)
{
    bool video_dev_flag = false;
    int video_index;
    bool video_file_flag = false;
    std::string video_path;
    bool dir_flag = false;
    std::string dir_path;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-v") {
            video_dev_flag = true;
            if (++i == argc || strlen(argv[i]) <= 0) {
                print_missing_option(argv[0], arg);
                return 1;
            }
            video_index = atoi(argv[i]);
        } else if (arg == "-f") {
            video_file_flag = true;
            if (++i == argc || strlen(argv[i]) <= 0) {
                print_missing_option(argv[0], arg);
                return 1;
            }
            video_path = std::string(argv[i]);
        } else if (arg == "-d") {
            dir_flag = true;
            if (++i == argc || strlen(argv[i]) <= 0) {
                print_missing_option(argv[0], arg);
                return 1;
            }
            dir_path = std::string(argv[i]);
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else {
            std::cerr << argv[0] << ": invalid argument '" << arg << "'" << std::endl;
            return 1;
        }
    }

    if (!video_dev_flag && !video_file_flag && !dir_flag) {
        video_dev_flag = true;
        video_index = 0;
    }

    if ((video_dev_flag || video_file_flag || !dir_flag) && (video_dev_flag || !video_file_flag || dir_flag) && (!video_dev_flag || video_file_flag || dir_flag)) {
        std::cerr << argv[0] << ": need to specify one and only one of the arguments '-v', '-f', or '-d'" << std::endl;
        return 1;
    }

    // Build a list of files in the specified directory
    std::list<std::string> file_paths;
    if (dir_flag) {
        DIR* dir = opendir(dir_path.c_str());
        if (! dir) {
            std::cerr << "Directory " << dir_path << " does not exist!" << std::endl;
            return 1;
        }
        // Iterate over all files in the specified directory.
        dirent* file;
        while ((file = readdir(dir)) != NULL) {
            if (file && file->d_type == DT_REG) {
                file_paths.push_back(dir_path + "/" + file->d_name);
            }
        }
    }

    int return_val = displayImageFeed(video_dev_flag, video_index, video_file_flag, video_path, dir_flag, file_paths, processingFunction);

    return return_val;
}

cv::Mat processingFunction(cv::Mat& image)
{
    // Resize the image if it is very large.
    while (image.rows > 1000 || image.cols > 1000) {
        cv::resize(image, image, cv::Size(image.cols / 2, image.rows / 2));
    }

    std::vector<std::vector<cv::Point> > contours = segmentForeground(image);
    for (size_t i = 0; i < contours.size(); ++i) {
        cv::Rect bounding_rect = scaleRectWithLimits(cv::boundingRect(contours[i]), cv::Point(0,0), cv::Point(image.cols,image.rows), default_scale);
        cv::rectangle(image, bounding_rect.tl(), bounding_rect.br(), cv::Scalar(0,255,0), 2);
    }

    return image;
}
