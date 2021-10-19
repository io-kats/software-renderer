#ifndef WINDOW_H
#define WINDOW_H

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <GLES3/gl3.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#else
#include <glad/glad.h>
#include <glfw3.h>
#endif

#include <cstring>
#include "ers/macros.h"
#include "ers/common.h"
#include "ers/vec.h"

// Helper structs for the Window class.
struct MouseCursor
{
    f64 x_prev, y_prev;
    f64 x, y;
    f64 dx, dy;
};

struct MouseWheel
{
    f64 dx_cache, dy_cache;
    f64 dx, dy;
};

struct HardwareSwitch
{
    bool previous_state;
    bool current_state;
	bool pressed;
	bool held;
	bool released;
};

// Absolute bare-bones GLFW wrapper, using:
// - OpenGL 3.3 with the Glad loader from https://glad.dav1d.de/ (Windows/Linux) or 
// - OpenGL 3.0 ES with emscripten for the browser
// for context/window creation. No abstracted backend yet.
// It does not fully abstract GLFW away either, i.e. I sometimes depend on GLFW outside of this class as well.
// The user is meant to inherit from this class and provide their own functionality by overriding
// the 3 virtual functions listed below.
class Window
{
public:
    Window(const char* title, s32 width_, s32 height_, s32 windowpos_x, s32 windowpos_y);
    ~Window();

    void Loop();

    // Workaround for emscripten's main loop.
    static void StaticLoop();
    
    // Start the loop.
    void Run();

    // User-defined functions.
    // Before loop.
    // 1. Initialize user-defined members
    virtual void Init(); 
    
    // Inside loop.
    // 2. Update user-defined members, poll inputs etc.
    virtual void Update();

    // After loop.
    // 3. Cleanup as necessary before shutdown.
    virtual void Cleanup(); 

    // Helper methods.
    void SetMousePosition(float xpos, float ypos);
    void SetMousePosition(const ers::vec2& pos);
    ers::vec2 GetMousePosition();
    ers::vec2 GetMouseDelta();
    float GetScrollDelta();
   
    bool KeyPressed(s32 i);
    bool KeyReleased(s32 i);
    bool KeyHeld(s32 i);
    bool MouseButtonPressed(s32 i);
    bool MouseButtonReleased(s32 i);
    bool MouseButtonHeld(s32 i);

    ers::StringView GetTitle();

    void SetWindowSize(s32 new_width, s32 new_height);
    ers::vec2 GetWindowSize();
    s32 GetWindowWidth();
    s32 GetWindowHeight();

    bool WindowSizeChanged();
    GLFWwindow* GetWindowObject();

    f64 GetDeltaTime(); // Returns the time difference between frames.
    f64 GetCurrentFrameTime(); // Returns the time at the start of the current frame.
    f64 GetPreviousFrameTime(); // Returns the time at the start of the previous frame.
    f64 GetCurrentTime(); // Returns the time at the point of calling this method.

private:  
    // Workaround for GLFW callback functions.
    static Window* windowSingleton;
    
    ers::StringView m_title;
    s32 m_width;
    s32 m_height;
    s32 m_previousWidth;
    s32 m_previousHeight;
    bool m_windowSizeChanged;

    GLFWwindow* m_window;
    MouseCursor m_mouseCursor;
    MouseWheel m_mouseWheel;

    HardwareSwitch m_keys[GLFW_KEY_LAST + 1];
    HardwareSwitch m_mouseButtons[GLFW_MOUSE_BUTTON_LAST + 1];

    f64 m_deltaTime;
    f64 m_currentFrame;
    f64 m_previousFrame;
    f64 m_accumulatedTime;

    void updateInputInternal();

    // GLFW callback functions.
    static void framebuffer_size_callback(GLFWwindow* window, s32 width, s32 height);
    static void key_callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mode);
    static void cursor_pos_callback(GLFWwindow* window, f64 xpos, f64 ypos);
    static void mouse_button_callback(GLFWwindow* window, s32 button, s32 action, s32 mods);
    static void scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset);
};

#endif // WINDOW_H