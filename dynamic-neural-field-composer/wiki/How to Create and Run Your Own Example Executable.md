This guide will walk you through creating and running your own custom example executable in the Dynamic Neural Field Composer project.

## Project Structure
The project follows this structure:
``` 
dynamic-neural-field-composer/
├── src/                          # Main source code
├── include/                      # Header files
├── examples/                     # Example executables
│   ├── ex_field_couplings.cpp
│   ├── ex_two_robot_team.cpp
│   └── ... (other examples)
├── CMakeLists.txt               # Main build configuration
└── build/                       # Build directory (created during build)
```

## Step 1: Create Your Example File
1. Navigate to the examples directory;
1. Create your new example file ```my_custom_example.cpp```; 
1. Based on the provided examples create your DNF architecture.

## Step 2: Update CMakeLists.txt
1. Open the main CMakeLists.txt file;
2. Add your executable to the build system:
``` cmake
   # Add this near the end of the file, after the main executable
   add_executable(my_custom_example "examples/my_custom_example.cpp")
```
**Note:** Replace `my_custom_example` with your desired executable name, and update the path to match your filename.

3. Build and run.

## Tips and Best Practices
1. **Use meaningful names** for your executables and elements
2. **Start simple** - begin with basic elements and gradually add complexity
3. **Check existing examples** in the directory for reference `examples/`
4. **Use appropriate simulation parameters** - start with proven values from existing examples and use the GUI to facilitate parameter selection

## Troubleshooting
**Build errors:**
- Make sure all includes are correct
- Verify CMakeLists.txt syntax
- Check that all dependencies are properly linked

**Runtime errors:**
- Ensure simulation is initialized with `sim->init()` before running steps
- Verify element parameters are within valid ranges
- Check that element connections are properly established


Now you're ready to create and run your own Dynamic Neural Field examples! Start with simple configurations and gradually build up to more complex scenarios.
