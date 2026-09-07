#include <iostream>
#include <string>

int main() {
    std::string url = "file:///home/hery/Documents/Repositories/Pop_DAW/audio.wav";
    // Check if it starts with file:// and print local path
    if (url.substr(0, 7) == "file://") {
        std::cout << url.substr(7) << std::endl;
    }
    return 0;
}
