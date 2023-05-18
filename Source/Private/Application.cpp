#include <Core/Application.hpp>

Application::Application(ApplicationHints application_hints)
{
    if(glfwInit() == GLFW_FALSE)
    {
        std::cerr << "ERROR::GLFW::Failed to initialize" << std::endl;
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE,        GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, application_hints.m_context_major_version);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, application_hints.m_context_minor_version);

    m_on_start_callback = nullptr;
    m_on_update_callback = nullptr;
    m_on_terminate_callback = nullptr;
}

Application::~Application(void)
{
    glfwDestroyWindow(m_window);

    glfwTerminate();
}

double Application::GetTime(void)
{
    return glfwGetTime();
}

bool Application::IsRunning(void)
{
    return !glfwWindowShouldClose(m_window);
}

void Application::SetKeyboardCallback(GLFWkeyfun fn)
{
    glfwSetKeyCallback(m_window, fn);
}
void Application::SetMouseCursorCallback(GLFWcursorposfun fn)
{
    glfwSetCursorPosCallback(m_window, fn);
}
void Application::SetMouseScrollCallback(GLFWscrollfun fn)
{
    glfwSetScrollCallback(m_window, fn);
}
void Application::SetMouseButtonCallback(GLFWmousebuttonfun fn)
{
    glfwSetMouseButtonCallback(m_window, fn);
}

void Application::Init(WindowDesc window_description)
{
    m_window = glfwCreateWindow(
        window_description.m_window_width, 
        window_description.m_window_height, 
        window_description.m_window_title, NULL, NULL);

    if (!m_window) 
    {
        const char* description;
        int code = glfwGetError(&description);

        if (description)
            std::cerr << "ERROR::GLFW::code: " 
                      << code << " description: " << description << std::endl;
        
        glfwTerminate();

        exit(EXIT_FAILURE);
    }

    glfwSwapInterval(1);    
    glfwMakeContextCurrent(m_window);

    if(glewInit() != GLEW_OK)
    {
        std::cerr << "ERROR::GLEW::failed to initialize" << std::endl;

        glfwDestroyWindow(m_window);
        glfwTerminate();

        exit(EXIT_FAILURE);
    }

    if(m_on_start_callback)
        m_on_start_callback();
}
void Application::Tick(void)
{
    if(m_on_update_callback)
        m_on_update_callback();

    glfwSwapBuffers(m_window);
}
void Application::Poll(void)
{
    glfwPollEvents();
}

void Application::OnStart(DefaultCallback fn)
{
    m_on_start_callback = fn;
}
void Application::OnUpdate(DefaultCallback fn)
{
    m_on_update_callback = fn;
}
void Application::OnTerminate(DefaultCallback fn)
{
    m_on_terminate_callback = fn;
}