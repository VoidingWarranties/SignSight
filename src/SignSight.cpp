#include <unistd.h>
#include <cstdlib>
#include <dirent.h>

#include <opencv2/highgui/highgui.hpp>

#include <list>
#include <iostream>


int displayOutput(bool video_flag, int video_index, bool dir_flag, char* dir_path);

void print_usage(char* name)
{
    std::cerr << "Usage: " << name << " [-v <video_device_index> | -f <image_directory_path>]"
              << "\n\n"
              << "       If no options are specified video device 0 is used by default."
              << std::endl;
}

int main(int argc, char** argv)
{
    bool video_flag = true;
    int video_index = 0;
    bool dir_flag = false;
    char* dir_path = NULL;

    opterr = 0;
    char c;

    while ((c = getopt(argc, argv, "v:f:")) != -1) {
        switch (c) {
            case 'v':
                video_flag = true;
                dir_flag = false;
                video_index = atoi(optarg);
                break;
            case 'f':
                dir_flag = true;
                video_flag = false;
                dir_path = optarg;
                break;
            case '?':
                print_usage(argv[0]);
                return 1;
                break;
            default:
                return 1;
        }
        if (video_flag && dir_flag) {
            print_usage(argv[0]);
            return 1;
        }
    }

    int return_val = displayOutput(video_flag, video_index, dir_flag, dir_path);

    return return_val;
}

int displayOutput(bool video_flag, int video_index, bool dir_flag, char* dir_path)
{
    // The right and left arrow key codes seem to differ from system to system.
    // Change these values as necessary
    const int RIGHT_ARROW_KEY = 65363;
    const int LEFT_ARROW_KEY = 65361;


    cv::namedWindow("Output");
    cv::Mat image;

    if (video_flag) {
        cv::VideoCapture cam(video_index);

        while (true) {
            cam >> image;
            if (! image.data) {
                std::cerr << "Error reading camera data. Did someone unplug it?" << std::endl;
            }
            cv::imshow("Output", image);

            int key = cv::waitKey(10);
            if (key == 'q') {
                break;
            }
        }
    } else if (dir_flag) {
        std::list<char*> file_list;
        DIR* dir = opendir(dir_path);
        if (! dir) {
            std::cerr << "Directory " << dir_path << " does not exist!" << std::endl;
            return 1;
        }
        // Iterate over all files in the specified directory.
        dirent* file;
        while ((file = readdir(dir)) != NULL) {
            if (file && file->d_type == DT_REG) {
                // Form the path by concatenating dir_path with each file name
                char* buffer = new char[strlen(dir_path) + strlen(file->d_name) + 2]();
                strcpy(buffer, dir_path);
                buffer[strlen(dir_path)] = '/';
                strcpy(buffer + strlen(dir_path) + 1, file->d_name);
                // add the buffer to the end of the doubly linked list
                file_list.push_back(buffer);
            }
        }
        std::list<char*>::const_iterator file_itr = file_list.begin();
        while (true) {
            image = cv::imread(*file_itr);
            if (! image.data) {
                std::cerr << "Error reading image file " << *file_itr << std::endl;
                return 1;
            }

            cv::imshow("Output", image);

            int key = cv::waitKey();
            if (key == 'q') {
                break;
            } else if (key == RIGHT_ARROW_KEY) {
                ++file_itr;
                if (file_itr == file_list.end()) {
                    file_itr = file_list.begin();
                }
            } else if (key == LEFT_ARROW_KEY) {
                --file_itr;
                if (file_itr == file_list.end()) {
                    --file_itr;
                }
            }
        }
    }

    return 0;
}
