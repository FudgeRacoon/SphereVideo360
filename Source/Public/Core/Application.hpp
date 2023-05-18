#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <iostream>
#include <functional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <Core/Defines.h>

struct WindowDesc
{
    /**
     * @brief The window width in screen coordinates.
     */
    uint16_t m_window_width     = DEFAULT_WINDOW_WIDTH;

    /**
     * @brief The window height in screen coordinates.
     */
    uint16_t m_window_height    = DEFAULT_WINDOW_HEIGHT;

    /**
     * @brief Is the window fullscreen or not?
     */
    bool m_is_window_fullscreen = DEFAULT_WINDOW_FULLSCREEN;

    /**
     * @brief The initial window title.
     */
    const char* m_window_title   = DEFAULT_WINDOW_TITLE;
};

struct ApplicationHints
{
    /**
     * @brief The client API major version.
     */
    uint16_t m_context_major_version = DEFAULT_CONTEXT_MAJOR_VERSION;

    /**
     * @brief The client API minor version.
     */
    uint16_t m_context_minor_version = DEFAULT_CONTEXT_MINOR_VERSION;
};

class Application
{
private:
    using DefaultCallback     = std::function<void(void)>;
    using KeyboardCallback    = std::function<void(GLFWwindow*, int, int, int, int)>;
    using MouseCursorCallback = std::function<void(GLFWwindow*, double, double)>;
    using MouseScrollCallback = std::function<void(GLFWwindow*, double, double)>;
    using MouseButtonCallback = std::function<void(GLFWwindow*, int, int, int)>;

private:
    /**
     * @brief The application owned window.
     */
    GLFWwindow* m_window;

private:
    DefaultCallback m_on_start_callback;
    DefaultCallback m_on_update_callback;
    DefaultCallback m_on_terminate_callback;

public:
    /**
     * @brief Default constructor.
     */
    Application(ApplicationHints application_hints);

public:
    /**
     * @brief Default destructor.
     */
    ~Application(void);

public:
    /**
     * @brief Checks wether the application is running or not.
     * 
     * @returns True if the appplication is running, otherwise false.
     */
    bool IsRunning(void);

public:
    /**
     * @brief Get the current time since application initialization.
     * 
     * @returns double storing the time since application initialization.
     */
    double GetTime(void);

public:
    /**
     * @brief Sets the key callback of the specified window, which is called
     * when a key is pressed, repeated or released.
     * 
     * @param fn The function to be called. 
     */
    void SetKeyboardCallback(GLFWkeyfun fn);

    /**
     * @brief Sets the cursor position callback of the specified window, which
     * is called when the cursor is moved.
     * 
     * @param fn The function to be called.
     */
    void SetMouseCursorCallback(GLFWcursorposfun fn);

    /**
     * @brief Sets the key callback of the specified window, which is called
     * when a key is pressed, repeated or released.
     * 
     * @param fn The function to be called.
     */
    void SetMouseScrollCallback(GLFWscrollfun fn);

    /**
     * @brief Sets the key callback of the specified window, which is called
     * when a key is pressed, repeated or released.
     * 
     * @param fn The function to be called.
     */
    void SetMouseButtonCallback(GLFWmousebuttonfun fn);

public:
    /**
     * @brief Initializes the application by creating a window and a valid
     * opengl context associated with the window.
     * 
     * @param window_description The window description.
     */
    void Init(WindowDesc window_description);

    /**
     * @brief Updates the current frame of the application.
     */
    void Tick(void);

    /**
     * @brief Polls the current inputs in the application.
     */
    void Poll(void);

public:
    /**
     * @brief Sets the function to be called on application start.
     * 
     * @param fn The function to be called.
     */
    void OnStart(DefaultCallback fn);

    /**
     * @brief Sets the function to be called on application update.
     * 
     * @param fn The function to be called.
     */
    void OnUpdate(DefaultCallback fn);

    /**
     * @brief Sets the function to be called on application termination.
     * 
     * @param fn The function to be called.
     */
    void OnTerminate(DefaultCallback fn);
};

#endif