#include <unistd.h>
#include <cstdlib>

#include <iostream>

void print_usage(char* name)
{
    std::cout << "Usage: " << name << " [-v <video_device_index> | -f <image_directory_path>]"
              << "\n\n"
              << "       if no options are specified video device 0 is used by default."
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

    return 0;
}
