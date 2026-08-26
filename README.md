![Pop DAW](pics/popdaw_banner.png)

# Pop - DAP

A digital audio playground for Linux.

Pop is a multi-track audio environment for recording, playing back VST3/LV2 instruments, and experimenting with sound. Built with **Qt6 / QML** for a sleek, GPU-accelerated UI and **Tracktion Engine** for rock-solid audio architecture.

Pop features a user-configurable, hot-reloadable QML interface that completely decouples the frontend from the headless audio backend.

## Features

- **Modular UI:** Beautiful, retro-futuristic dark pastel UI powered by Qt Quick Controls 2.
- **Audio Engine:** Headless Tracktion Engine integration providing high-performance playback.
- **Plugin Support:** Native VST3 and LV2 hosting with a built-in Plugin Scanner.
- **Timeline:** Drag-and-drop audio clips (`.wav`, `.mp3`, `.flac`) directly into track lanes.
- **Mixer:** Dynamic channel strips with real-time level metering, mute, solo, and volume controls.
- **Audio Routing:** Native support for PipeWire, JACK, and ALSA via JUCE.

## Building

### System Dependencies

**Debian / Ubuntu:**
```bash
sudo apt install build-essential cmake git pkg-config \
    libasound2-dev libjack-jackd2-dev libfreetype-dev \
    libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev \
    libxi-dev libgl1-mesa-dev libcurl4-openssl-dev \
    libwebkit2gtk-4.1-dev ladspa-sdk \
    qt6-base-dev qt6-declarative-dev
```

**Arch Linux:**
```bash
sudo pacman -S --needed base-devel cmake git pkgconf \
    alsa-lib jack2 freetype2 libx11 libxrandr libxinerama \
    libxcursor libxi mesa curl webkit2gtk-4.1 ladspa \
    qt6-base qt6-declarative
```

### Clone & Build

```bash
git clone --recurse-submodules https://github.com/hery/Pop_DAW.git
cd Pop_DAW
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Running

```bash
./PopDAW
```

## License

GPLv3 — see [LICENSE](LICENSE) for details.
