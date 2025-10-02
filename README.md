<img src="./dynamic-neural-field-composer/resources/images/logo.png" alt="logo" >

Dynamic Neural Field Composer - A C++ application designed for developing and simulating dynamic neural field architectures
===============================================

## Description

Dynamic Neural Field Composer leverages modern C++ standards to offer a robust framework for the creation, composition, and real-time visualization of neural field dynamics. This tool is equipped with a graphical user interface that enhances user interaction, making it easier to monitor and adjust neural field behaviors and properties dynamically.

## Functionalities

- **Flexible Element Management**: Dynamically add, modify, and remove neural field elements during simulations.
- **Advanced Simulation Controls**: Initiate, pause, and step through simulations with precise timing control.
- **Interactive GUI**: Utilize integrated GUI controls for managing simulations and visualizing data in real time.
- **Data Export and Logging**: Export neural field data and configurations, facilitating detailed analysis and reproducibility.
- **Extensible Architecture**: Easily extend with new neural field models, learning rules, or simulation protocols.
- **Learning and Adaptation**: Implement learning algorithms to adjust synaptic weights based on predefined or dynamic rules.
- **Plotting and Visualization**: Integrated plotting tools to visualize neural activity and simulation metrics.
- **Centroid Monitoring**: Track and display the centroid of neural activations to study stability and activity patterns.

## Project Dependencies

- **C++20**: Ensures compatibility with modern C++ features for optimal performance.
- **CMake 3.15+**: For building and configuring the project.
- **VCPKG**: Manages C++ libraries, facilitating the integration of external tools like ImGui and ImPlot.
- **DirectX 12 (Windows)** / **OpenGL (Linux)**: Essential for rendering graphical content.
- **GLFW (Linux)**: Supports window management and various operating functions on Linux.

The rest of the dependencies are installed automatically when running `build.bat`.  However, you must have defined the `VCPKG_ROOT` directory.

- **Dear ImGui**: Provides the foundational GUI components.
- **ImPlot**: Enables advanced plotting capabilities within ImGui interfaces.
- **nlohmann_json**: Supports JSON for configuration and saving of simulation states.

## Building and Installing

The project includes platform-specific scripts to facilitate building and installation:
- `build.bat`, `build.sh`: Compile the project using CMake configurations.
- `install.bat`, `install.sh`: Install the library to your system via CMake.

**Note**:
In a Linux machine you might have to:
1. Create a build directory inside the project folder ```mkdir build```;
2. Set ```VCPKG_ROOT``` directory as an environment variable ```export VCPKG_ROOT=/opt/vcpkg```;
3. Confirm you have at least GCC 13+ ```g++ --version```, if not:
    -   Install GCC 13
    ```bash
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test
    sudo apt update
    sudo apt install gcc-13 g++-13
    ```
    - Set it as default (optional):
    ```bash
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 100
    sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-13 100
    ```
Before running the ```build.sh```.

## Integration into Your CMake Project

Post-installation, integrate the library into your CMake projects:

```cmake
find_package(dynamic-neural-field-composer REQUIRED)

# Define your project target
add_executable(your_target src/main.cpp)

# Link against Dynamic Neural Field Composer
target_link_libraries(your_target PRIVATE dynamic-neural-field-composer)
```

## Getting started

1. Clone this repository to your local machine using Git.
2. Run the ```build.bat``` file. This will install all the necessary dependencies and build the project. Make sure you have VCPKG installed and the VCPKG_ROOT environment variable defined.
3. You can run the example executables to see the library in action.

The best way to get familiar with the library is to take a look at the ```examples``` folder!

## Contributing

Contributions to the Dynamic Neural Field Composer are welcomed. Whether it involves fixing bugs, adding new features, or improving the documentation, your help is appreciated to make this project even better.
