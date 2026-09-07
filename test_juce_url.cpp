#include <iostream>
#include <string>

int main() {
    std::string fileUrl = "/home/hery/Documents/Repositories/Pop_DAW/audio.wav";
    // Actually, I can't test juce::URL easily without linking JUCE.
    // Let's just write the fix for EngineController.cpp to handle both file:// and absolute paths!
    return 0;
}
