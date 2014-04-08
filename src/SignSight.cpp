#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <unistd.h>

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
    std::cerr << "\n"
              << "Usage: " << name << " [-v <video_device_index> | -f <video_file> | -d <image_directory_path>]\n"
              << "\tIf no options are specified video device 0 is used by default.\n"
              << "Hot keys:\n"
              << "\tESC - exit the program\n"
              << "\tLEFT ARROW - previous image (only when the -d option is used)\n"
              << "\tRIGHT ARROW - next image (only when the -d option is used)\n"
              << std::endl;
}

int main(int argc, char** argv)
{
    bool video_dev_flag = true;
    int video_index = 0;
    bool video_file_flag = false;
    std::string video_path;
    bool dir_flag = false;
    std::string dir_path;

    opterr = 0;
    char c;

    while ((c = getopt(argc, argv, "v:f:d:")) != -1) {
        switch (c) {
            case 'v':
                video_dev_flag = true;
                video_file_flag = false;
                dir_flag = false;
                video_index = atoi(optarg);
                break;
            case 'f':
                video_dev_flag = false;
                video_file_flag = true;
                dir_flag = false;
                video_path = std::string(optarg);
                break;
            case 'd':
                video_dev_flag = false;
                video_file_flag = false;
                dir_flag = true;
                dir_path = std::string(optarg);
                break;
            case '?':
                print_usage(argv[0]);
                return 1;
                break;
            default:
                return 1;
        }
    }
    if (((video_dev_flag && video_file_flag) || (video_dev_flag && dir_flag) || (video_file_flag && dir_flag)) && (video_dev_flag || video_file_flag || dir_flag)) {
        print_usage(argv[0]);
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
