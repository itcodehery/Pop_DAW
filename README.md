# Pop - DAP

A digital audio playground for Linux.

Pop is a multi-track audio environment for recording, playing back VST3/LV2 instruments, and experimenting with sound. It is not intended to be a traditional DAW or a replacement for one.

Built with C++20 on [Tracktion Engine](https://github.com/Tracktion/tracktion_engine) and [JUCE](https://juce.com). Audio backend support for PipeWire, JACK, and ALSA.

## Building

### System Dependencies

**Debian / Ubuntu:**
```bash
sudo apt install build-essential cmake git pkg-config \
    libasound2-dev libjack-jackd2-dev libfreetype-dev \
    libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev \
    libxi-dev libgl1-mesa-dev libcurl4-openssl-dev \
    libwebkit2gtk-4.1-dev ladspa-sdk
```

**Arch Linux:**
```bash
sudo pacman -S --needed base-devel cmake git pkgconf \
    alsa-lib jack2 freetype2 libx11 libxrandr libxinerama \
    libxcursor libxi mesa curl webkit2gtk-4.1 ladspa
```

### Clone & Build

```bash
git clone --recurse-submodules https://github.com/hery/Pop_DAW.git
cd Pop_DAW
cmake --preset debug
cmake --build build/debug -j$(nproc)
```

## License

GPLv3 — see [LICENSE](LICENSE) for details.
