# A bare Xvfb can drop an example's window connection

When `imgui_window_parameters.ini` is missing from the working directory, imgui-platform-kit's
`WindowParameters` (built with the default width and height of -1) calls `glfwInit`, reads the
primary monitor's video mode and calls `glfwTerminate`. `UserInterface::initialize` then calls
`glfwInit` again to open the window. So a fresh run makes two X connections in quick succession.

An X server started without `-noreset` resets whenever its last client disconnects. Under
`xvfb-run`, the application is the only client, so closing the first connection starts a reset,
and a second connection that arrives during the reset is dropped. The symptom is:

```text
GLFW Error 65550: X11: Failed to open display :<n>
Failed to initialize GLFW.
Segmentation fault (core dumped)
```

The segfault comes from the kit: `UserInterface::initialize` returns without a window when
`glfwInit` fails, and `render()` uses the null window.

Signs it is this and not a display-number clash:
- The message is the kit's `std::cerr` line, not the exception `WindowParameters` throws, so the
  first connection to the same display worked.
- Each Xvfb compiles its keymap about twice (count "XKEYBOARD keymap compiler" in the log), once
  at startup and once per reset.
- It only hits examples that start before any example has exited and written the ini file.

Fix: pass `-noreset` in Xvfb's server arguments (`xvfb-run --server-args="... -noreset"`). The
CI smoke-run does this. Locally, a desktop session's other clients keep the server alive, so it
does not show up there.
