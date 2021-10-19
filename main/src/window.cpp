#include "window.h"

Window* Window::windowSingleton = nullptr;

Window::Window(const char* title_, s32 width_, s32 height_, s32 windowpos_x, s32 windowpos_y)
    : m_title(ers::StringView(title_)), m_width(width_), m_height(height_), m_previousWidth(width_), m_previousHeight(height_)
{
    ERS_PANICF(windowSingleton == nullptr, "%s", "Only one instance of the Window class is allowed at the moment.");
    windowSingleton = this;

    m_windowSizeChanged = false;
    
    memset(&m_keys[0], 0, sizeof(m_keys));
    memset(&m_mouseButtons[0], 0, sizeof(m_mouseButtons));

    m_previousFrame = 0.0;
    m_accumulatedTime = 0.0;
    
    memset(&m_mouseCursor, 0, sizeof(m_mouseCursor));
    memset(&m_mouseWheel, 0, sizeof(m_mouseWheel));

    ERS_PANICF(glfwInit(), "%s", "Failed to initialize GLFW.");

    // GLFW OpenGL boilerplate.
#ifdef __EMSCRIPTEN__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    // Create a window.
    m_window = glfwCreateWindow(
        m_width,
        m_height,
        m_title.data,
        nullptr,
        nullptr);
    ERS_PANICF(m_window, "%s", "GLFW: Failed to create window.");

    glfwMakeContextCurrent(m_window);
    glfwSetWindowPos(m_window, windowpos_x, windowpos_y);
    glfwSetFramebufferSizeCallback(m_window, Window::framebuffer_size_callback);
    glfwSetKeyCallback(m_window, Window::key_callback);
    glfwSetCursorPosCallback(m_window, Window::cursor_pos_callback);
    glfwSetMouseButtonCallback(m_window, Window::mouse_button_callback);
    glfwSetScrollCallback(m_window, Window::scroll_callback);
    glfwSwapInterval(1);

    #ifndef __EMSCRIPTEN__  
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwDestroyWindow(m_window);
	    glfwTerminate();
        fprintf(stderr, "%s", "Failed to initialiaze GLAD.");
        exit(EXIT_FAILURE);
    }
    #endif
    
    glViewport(0, 0, m_width, m_height);
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
	glfwTerminate();
    windowSingleton = nullptr;
}

void Window::Init() { }
// void Window::ProcessInput() { }
void Window::Update() { }
// void Window::Draw() { }
void Window::Cleanup() { }

void Window::Loop()
{
    m_currentFrame = glfwGetTime();
    m_deltaTime = m_currentFrame - m_previousFrame;
    m_previousFrame = m_currentFrame;
    
    glfwPollEvents();
    updateInputInternal(); 
    Update();

    glfwSwapBuffers(m_window);
}

void Window::StaticLoop() 
{ 
    windowSingleton->Loop();
}

void Window::Run()
{         
    Init();   
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(Window::StaticLoop, 0, true);
#else
    while(!glfwWindowShouldClose(m_window)) 
        Loop();
#endif
}

void Window::SetMousePosition(f32 xpos, f32 ypos)
{
    glfwSetCursorPos(m_window, (f32)xpos, (f32)ypos);
} 

void Window::SetMousePosition(const ers::vec2& pos)
{
    glfwSetCursorPos(m_window, (f32)pos.x(), (f32)pos.y());
} 

ers::vec2 Window::GetMousePosition()
{
    return ers::vec2((f32)m_mouseCursor.x, (f32)m_mouseCursor.y);
}

ers::vec2 Window::GetMouseDelta()
{
    return ers::vec2((f32)m_mouseCursor.dx, (f32)m_mouseCursor.dy);
}

f32 Window::GetScrollDelta()
{
    return (f32)m_mouseWheel.dy;
}

bool Window::KeyPressed(s32 i)
{
    return m_keys[i].pressed;
}

bool Window::KeyReleased(s32 i)
{
    return m_keys[i].released;
}

bool Window::KeyHeld(s32 i)
{
    return m_keys[i].held;
}

bool Window::MouseButtonPressed(s32 i)
{
    return m_mouseButtons[i].pressed;
}

bool Window::MouseButtonReleased(s32 i)
{
    return m_mouseButtons[i].released;
}

bool Window::MouseButtonHeld(s32 i)
{
    return m_mouseButtons[i].held;
}

ers::StringView Window::GetTitle()
{
    return m_title;
}  

void Window::SetWindowSize(s32 new_width, s32 new_height)
{
    m_width = new_width;
    m_height = new_height;
    glViewport(0, 0, m_width, m_height);
}

ers::vec2 Window::GetWindowSize()
{
    return ers::vec2((f32)m_width, (f32)m_height);
}

s32 Window::GetWindowWidth()
{
    return m_width;
}

s32 Window::GetWindowHeight()
{
    return m_height;
}

bool Window::WindowSizeChanged()
{
    return m_windowSizeChanged;
}

GLFWwindow* Window::GetWindowObject()
{
    return m_window;
}

f64 Window::GetDeltaTime()
{
    return m_deltaTime;
}

f64 Window::GetCurrentFrameTime()
{
    return m_currentFrame;
}

f64 Window::GetPreviousFrameTime()
{
    return m_previousFrame;
}

f64 Window::GetCurrentTime()
{
    return glfwGetTime();
}

void Window::updateInputInternal()
{
    // Keyboard.
    for (size_t key = 0; key <= GLFW_KEY_LAST; ++key)
    {
        HardwareSwitch& key_curr = m_keys[key];
        key_curr.pressed = false;
        key_curr.released = false;
        if (key_curr.current_state != key_curr.previous_state)
        {
            if (key_curr.current_state)
            {
                key_curr.pressed = !key_curr.held;
                key_curr.held = true;
            }
            else
            {
                key_curr.held = false;
                key_curr.released = true;
            }
        }
        key_curr.previous_state = key_curr.current_state;
    }

    // Mouse buttons.
    for (size_t button = 0; button <= GLFW_MOUSE_BUTTON_LAST; ++button)
    {
        HardwareSwitch& button_curr = m_mouseButtons[button];
        button_curr.pressed = false;
        button_curr.released = false;
        if (button_curr.current_state != button_curr.previous_state)
        {
            if (button_curr.current_state)
            {
                button_curr.pressed = !button_curr.held;
                button_curr.held = true;
            }
            else
            {
                button_curr.held = false;
                button_curr.released = true;
            }
        }
        button_curr.previous_state = button_curr.current_state;
    }

    // Mouse cursor position and difference.
    m_mouseCursor.dx = m_mouseCursor.x - m_mouseCursor.x_prev;
    m_mouseCursor.dy = m_mouseCursor.y - m_mouseCursor.y_prev;
    m_mouseCursor.x_prev = m_mouseCursor.x;
    m_mouseCursor.y_prev = m_mouseCursor.y;

    // Mouse wheel offset and difference.
    #ifdef __EMSCRIPTEN__
    m_mouseWheel.dx = -m_mouseWheel.dx_cache;
    m_mouseWheel.dy = -m_mouseWheel.dy_cache;
    #else
    m_mouseWheel.dx = m_mouseWheel.dx_cache;
    m_mouseWheel.dy = m_mouseWheel.dy_cache;
    #endif
    m_mouseWheel.dx_cache = 0.0;
    m_mouseWheel.dy_cache = 0.0;

    // Window size.
    m_windowSizeChanged = (m_previousWidth != m_width || m_previousHeight != m_height);
    m_previousWidth = m_width;
    m_previousHeight = m_height;
}

void Window::framebuffer_size_callback(GLFWwindow* window, s32 width, s32 height)
{
    ERS_UNUSED(window);
    glViewport(0, 0, width, height);
    windowSingleton->m_width = width;
    windowSingleton->m_height = height;
}

void Window::key_callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mode)
{
    ERS_UNUSED(window);
    ERS_UNUSED(mode);
    ERS_UNUSED(scancode);
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(windowSingleton->m_window, true);

    if (key >= 0 && key <= GLFW_KEY_LAST)
    {	        	
        if (action == GLFW_PRESS)
        {
            windowSingleton->m_keys[key].current_state = true;   
        }
        else if (action == GLFW_RELEASE)
        {
            windowSingleton->m_keys[key].current_state = false;	
        }
    }
}

void Window::mouse_button_callback(GLFWwindow* window, s32 button, s32 action, s32 mods)
{
    ERS_UNUSED(window);
    ERS_UNUSED(mods);
    if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
    {	        	
        if (action == GLFW_PRESS)
        {
            windowSingleton->m_mouseButtons[button].current_state = true;   
        }
        else if (action == GLFW_RELEASE)
        {
            windowSingleton->m_mouseButtons[button].current_state = false;	
        }
    }
}

void Window::cursor_pos_callback(GLFWwindow* window, f64 xpos, f64 ypos)
{
    ERS_UNUSED(window);
    windowSingleton->m_mouseCursor.x = xpos;
    windowSingleton->m_mouseCursor.y = ypos;
}

void Window::scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset)
{
    ERS_UNUSED(window);
    windowSingleton->m_mouseWheel.dx_cache += xoffset;
    windowSingleton->m_mouseWheel.dy_cache += yoffset;
}
