#include <cstdlib>
#include <cstring>

#include <list>
#include <string>
#include <vector>
#include <iostream>

#include <boost/filesystem.hpp>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "GUI.hpp"
#include "Draw.hpp"
#include "ImageProcessing.hpp"


void print_usage(char* name)
{
    std::cerr << "Usage: " << name << " -f <in file> | -d <in dir> -o <out dir>\n"
              << "  <in file> - input video file\n"
              << "   <in dir> - input directory containing only image files\n"
              << "  <out dir> - directory where sub-images will be saved\n"
              << "Hot keys:\n"
              << "        ESC - exit the program\n"
              << "          y - confirm selected sub-image as a sign and save the sub-image to the output directory\n"
              << "          n - reject selected sub-image as a sign and do NOT save the sub-image\n"
              << "          s - skip frame (rejects all signs in frame)"
              << std::endl;
}

void print_missing_option(char* name, const std::string& arg)
{
    std::cerr << name << ": missing option after '" << arg << "'" << std::endl;
}

int main(int argc, char** argv)
{
    bool video_flag = false;
    std::string video_path;
    bool dir_flag = false;
    std::string dir_path;
    bool output_flag = false;
    std::string output_path;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-f") {
            video_flag = true;
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
        } else if (arg == "-o") {
            output_flag = true;
            if (++i == argc || strlen(argv[i]) <= 0) {
                print_missing_option(argv[0], arg);
                return 1;
            }
            output_path = std::string(argv[i]);
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else {
            std::cerr << argv[0] << ": invalid argument '" << arg << "'" << std::endl;
            return 1;
        }
    }

    if (!(video_flag ^ dir_flag)) {
        std::cerr << argv[0] << ": need to specify either '-f' or '-d' argument, but not both" << std::endl;
        return 1;
    }
    if (!output_flag) {
        std::cerr << argv[0] << ": missing required argument '-o'" << std::endl;
        return 1;
    }

    cv::VideoCapture cam;
    std::list<std::string> file_paths;
    if (video_flag) {
        cam.open(video_path);
    } else if (dir_flag) {
        boost::filesystem::path dir(dir_path);
        if (! boost::filesystem::exists(dir)) {
            std::cerr << "'" << dir_path << "' does not exist" << std::endl;
            return 2;
        }
        if (! boost::filesystem::is_directory(dir)) {
            std::cerr << "'" << dir_path << "' is not a directory" << std::endl;
        }
        boost::filesystem::directory_iterator end_dir_itr;
        for (boost::filesystem::directory_iterator dir_itr(dir); dir_itr != end_dir_itr; ++dir_itr) {
            if (boost::filesystem::is_regular_file(dir_itr->status())) {
                file_paths.push_back(dir_itr->path().generic_string());
            }
        }
        if (file_paths.size() == 0) {
            std::cerr << "No images found in directory!" << std::endl;
            return 1;
        }
    }

    std::list<std::string>::const_iterator file_itr = file_paths.begin();
    size_t sign_index = 0;
    while (true) {
        cv::Mat image;
        if (video_flag) {
            cam >> image;
        } else if (dir_flag) {
            image = cv::imread(*file_itr);
        }
        if (! image.data) {
            std::cerr << "Error reading from device or file." << std::endl;
            return 1;
        }
        while (image.rows > 1000 || image.cols > 1000) {
            cv::resize(image, image, cv::Size(image.cols / 2, image.rows / 2));
        }
        cv::imshow("Output", image);

        cv::Mat display_image;
        image.copyTo(display_image);
        std::vector<std::vector<cv::Point> > contours = segmentForeground(image);
        std::vector<cv::Rect> bounding_rects;
        for (size_t i = 0; i < contours.size(); ++i) {
            bounding_rects.push_back(scaleRectWithLimits(cv::boundingRect(contours[i]), cv::Point(0,0), cv::Point(image.cols,image.rows), default_scale));
            cv::rectangle(display_image, bounding_rects[i], cv::Scalar(0,255,255), 2);
        }

        for (size_t i = 0; i < contours.size(); ++i) {
            // Construct file output path.
            std::string sub_img_output_path = output_path + "/" + std::to_string(sign_index) + ".jpg";

            // Redraw the current rectangle.
            cv::rectangle(display_image, bounding_rects[i], cv::Scalar(255,0,0), 2);
            cv::imshow("Output", display_image);

            // Create a sub image of the area inside the rectangle and display it.
            cv::Mat sub_image(image,
                              cv::Range(bounding_rects[i].tl().y, bounding_rects[i].br().y),
                              cv::Range(bounding_rects[i].tl().x, bounding_rects[i].br().x));
            cv::namedWindow("Sign");
            cv::imshow("Sign", sub_image);

            while (true) {
                char key = cv::waitKey();
                if (key == ESCAPE_KEY) {
                    goto exit_video_loop;
                } else if (key == 'n') {
                    cv::rectangle(display_image, bounding_rects[i], cv::Scalar(0,0,255), 2);
                    break;
                } else if (key == 'y') {
                    ++sign_index;
                    if (! cv::imwrite(sub_img_output_path, sub_image)) {
                        std::cerr << "Error writing to file " << sub_img_output_path << std::endl;
                        return 1;
                    }
                    cv::rectangle(display_image, bounding_rects[i], cv::Scalar(0,255,0), 2);
                    break;
                } else if (key == 's') {
                    goto exit_frame_loop;
                }
            }
        }
        exit_frame_loop:
        if (dir_flag) {
            ++file_itr;
            if (file_itr == file_paths.end()) {
                std::cout << "No more images in the directory." << std::endl;
                break;
            }
        }
    }
    exit_video_loop:

    return 0;
}
