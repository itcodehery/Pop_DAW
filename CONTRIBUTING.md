# Contributing to Pop

Welcome to the Pop project! We're glad you're interested in contributing to our Digital Audio Playground.

## Code of Conduct

Please treat everyone with respect. Constructive feedback is welcome, but personal attacks are not.

## Development Workflow

1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/my-new-feature`).
3. Commit your changes (`git commit -am 'Add some feature'`).
4. Push to the branch (`git push origin feature/my-new-feature`).
5. Create a new Pull Request.

## Coding Style

*   **Standard:** C++20
*   **Formatting:** We use `clang-format` with the provided `.clang-format` file (based on LLVM with Allman braces and 4-space indentation). Please format your code before submitting a pull request.
*   **Documentation:** Add comments for any non-obvious logic.
*   **JUCE & Tracktion Engine:** Try to follow the conventions used by JUCE and Tracktion Engine when working with their APIs.

## Building

See the `README.md` for detailed instructions on installing dependencies and building the project using CMake.
