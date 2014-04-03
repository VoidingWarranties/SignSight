#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <unistd.h>

#include <list>
#include <vector>
#include <iostream>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "GUI/GUI.hpp"
#include "GUI/Draw.hpp"
#include "ImageProcessing/ImageProcessing.hpp"


void print_usage(char* name)
{
    std::cerr << "\n"
              << "Usage: " << name << " [-f <video_file> | -d <image_directory_path>] -o <output_directory>\n"
              << "\tIf no options are specified video device 0 is used by default.\n"
              << "Hot keys:\n"
              << "\tESC - exit the program\n"
              << "\ty - confirm selected sub-image as a street sign and save the sub-image to the output directory\n"
              << "\tn - reject selected sub-image as a street sign and do NOT save the sub-image\n"
              << std::endl;
}

int main(int argc, char** argv)
{
    bool video_flag = true;
    char* video_path = NULL;
    bool dir_flag = false;
    char* dir_path = NULL;
    char* output_path = NULL;

    opterr = 0;
    char c;

    while ((c = getopt(argc, argv, "f:d:o:")) != -1) {
        switch (c) {
            case 'f':
                video_flag = true;
                dir_flag = false;
                video_path = strdup(optarg);
                break;
            case 'd':
                dir_flag = true;
                video_flag = false;
                dir_path = strdup(optarg);
                break;
            case 'o':
                output_path = strdup(optarg);
                break;
            case '?':
                print_usage(argv[0]);
                return 1;
                break;
            default:
                return 1;
        }
    }
    if ((!(video_flag ^ dir_flag)) || (! output_path)) {
        print_usage(argv[0]);
        return 1;
    }

    cv::VideoCapture cam;
    std::list<char*> file_paths;
    if (video_flag) {
        cam.open(video_path);
    } else if (dir_flag) {
        DIR* dir = opendir(dir_path);
        if (! dir) {
            std::cerr << "Directory " << dir_path << " does not exist!" << std::endl;
            return 1;
        }
        dirent* file;
        while ((file = readdir(dir)) != NULL) {
            if (file && file->d_type == DT_REG) {
                char* buffer = new char[strlen(dir_path) + strlen(file->d_name) + 2]();
                strcpy(buffer, dir_path);
                buffer[strlen(dir_path)] = '/';
                strcpy(buffer + strlen(dir_path) + 1, file->d_name);
                file_paths.push_back(buffer);
            }
        }
        if (file_paths.size() == 0) {
            std::cerr << "No images found in directory!" << std::endl;
            return 1;
        }
    }

    std::list<char*>::const_iterator file_itr = file_paths.begin();
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
            char* sub_img_output_path = new char[strlen(output_path) + 10];
            strcpy(sub_img_output_path, output_path);
            sub_img_output_path[strlen(output_path)] = '/';
            for (size_t digit = 4, tmp_sign_index = sign_index; digit > 0; --digit) {
                sub_img_output_path[strlen(output_path) + digit] = (tmp_sign_index % 10) + '0';
                tmp_sign_index /= 10;
            }
            strcpy(sub_img_output_path + strlen(output_path) + 5, ".jpg");

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
                    goto exit_loop;
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
                }
            }
        }
        if (dir_flag) {
            ++file_itr;
            if (file_itr == file_paths.end()) {
                std::cout << "No more images in the directory." << std::endl;
                break;
            }
        }
    }
    exit_loop:

    delete [] video_path;
    delete [] dir_path;
    delete [] output_path;

    return 0;
}