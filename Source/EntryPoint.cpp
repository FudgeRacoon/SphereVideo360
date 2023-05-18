#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>

#include <vector>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Core/Application.hpp>
#include <Core/VideoReader.hpp>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
}

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}

#define GL_ERR(__FUNC__)                  \
    {                                     \
        __FUNC__;                         \
        glCheckError_(__FILE__, __LINE__); \
    }

const char* vert_shader = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoords;\n"
    "\n"
    "out vec2 TexCoords;\n"
    "\n"
    "uniform mat4 model_matrix;\n"
    "uniform mat4 view_matrix;\n"
    "uniform mat4 proj_matrix;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position = proj_matrix * view_matrix * model_matrix * vec4(aPos, 1.0);\n" 
    "    TexCoords = aTexCoords;\n"
    "}\n";

const char* frag_shader =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "\n"
    "in vec2 TexCoords;\n"
    "\n"
    "uniform sampler2D tex;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(tex, TexCoords);\n"
    "}\n";

void draw_pixel(uint32_t color_tex_id, int x, int y, float r, float g, float b, float a)
{
    float pixel_data[] = { r, g, b, a };

    GL_ERR(glBindTexture(GL_TEXTURE_2D, color_tex_id))
    GL_ERR(glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, GL_RGBA, GL_FLOAT, pixel_data))

    GL_ERR(glBindTexture(GL_TEXTURE_2D, 0))
}

void init_gpu_program(uint32_t* program_id)
{
    uint32_t vert_id, frag_id;

    int success;
    char infoLog[512];
   
    // vertex Shader
    vert_id = glCreateShader(GL_VERTEX_SHADER);
    GL_ERR(glShaderSource(vert_id, 1, &vert_shader, nullptr))
    GL_ERR(glCompileShader(vert_id))
 
    GL_ERR(glGetShaderiv(vert_id, GL_COMPILE_STATUS, &success))
    if(!success)
    {
        GL_ERR(glGetShaderInfoLog(vert_id, 512, NULL, infoLog))
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };
  
    // fragment Shader
    frag_id = glCreateShader(GL_FRAGMENT_SHADER);
    GL_ERR(glShaderSource(frag_id, 1, &frag_shader, nullptr))
    GL_ERR(glCompileShader(frag_id))
 
    GL_ERR(glGetShaderiv(frag_id, GL_COMPILE_STATUS, &success))
    if(!success)
    {
        GL_ERR(glGetShaderInfoLog(frag_id, 512, NULL, infoLog))
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    };
  
    // shader Program
    *(program_id) = glCreateProgram();

    GL_ERR(glAttachShader(*(program_id), vert_id))
    GL_ERR(glAttachShader(*(program_id), frag_id))
    
    GL_ERR(glLinkProgram(*(program_id)))

    GL_ERR(glGetProgramiv(*(program_id), GL_LINK_STATUS, &success))
    if(!success)
    {
        GL_ERR(glGetProgramInfoLog(*(program_id), 512, NULL, infoLog))
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    
    GL_ERR(glDeleteShader(vert_id))
    GL_ERR(glDeleteShader(frag_id))
}

void init_screen_quad(uint32_t* vao_id, uint32_t* vbo_id, float** vertices_container)
{
    *(vertices_container) = new float[24];

    // Triangle 1
    (*(vertices_container)) [0] = -1.0f; (*(vertices_container)) [1] =  1.0f;  (*(vertices_container)) [2] = 0.0f; (*(vertices_container)) [3] = 1.0f;
    (*(vertices_container)) [4] = -1.0f; (*(vertices_container)) [5] = -1.0f;  (*(vertices_container)) [6] = 0.0f; (*(vertices_container)) [7] = 0.0f;
    (*(vertices_container)) [8] =  1.0f; (*(vertices_container)) [9] =  1.0f;  (*(vertices_container))[10] = 1.0f; (*(vertices_container))[11] = 1.0f;
     
    // Triangle 2
    (*(vertices_container))[12] = -1.0f; (*(vertices_container))[13] = -1.0f;  (*(vertices_container))[14] = 0.0f; (*(vertices_container))[15] = 0.0f;
    (*(vertices_container))[16] =  1.0f; (*(vertices_container))[17] = -1.0f;  (*(vertices_container))[18] = 1.0f; (*(vertices_container))[19] = 0.0f;
    (*(vertices_container))[20] =  1.0f; (*(vertices_container))[21] =  1.0f;  (*(vertices_container))[22] = 1.0f; (*(vertices_container))[23] = 1.0f;

    glGenVertexArrays(1, vao_id);
    glBindVertexArray(*vao_id);

    glGenBuffers(1, vbo_id);
    glBindBuffer(GL_ARRAY_BUFFER, *vbo_id);
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, *(vertices_container), GL_STATIC_DRAW);
    
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texture Coordinate
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

void init_framebuffer_object(uint32_t* fbo_id, uint32_t* color_tex_id, uint32_t* depth_tex_id, size_t width, size_t height)
{
    glGenFramebuffers(1, fbo_id);
    glBindFramebuffer(GL_FRAMEBUFFER, *(fbo_id));  

    // Color attachment texture
    glGenTextures(1, color_tex_id);
    glBindTexture(GL_TEXTURE_2D, *color_tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Depth attachment texture
    glGenTextures(1, depth_tex_id);
    glBindTexture(GL_TEXTURE_2D, *depth_tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *color_tex_id, 0); 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth_tex_id, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	    std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  
}

auto generate_uvspehre(int lon_count, int lat_count)
    -> std::tuple<std::vector<uint32_t>, std::vector<glm::vec3>, std::vector<glm::vec2>>
{   
    std::vector<uint32_t>  indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> tex_coords;

    float lon_step = 2 * M_PI / lon_count;
    float lat_step = M_PI / lat_count;

    float sectorAngle, stackAngle;

    float x, y, z, xy;
    float s, t;

    for(int i = 0; i <= lat_count; ++i)
    {
        stackAngle = M_PI / 2 - i * lat_step; 

        xy = 1 * cosf(stackAngle);          
        z = 1 * sinf(stackAngle);           

        for(int j = 0; j <= lon_count; ++j)
        {
            sectorAngle = j * lon_step;          

            x = xy * cosf(sectorAngle);           
            y = xy * sinf(sectorAngle);  
                       
            vertices.push_back(glm::vec3(x, y, z));

            s = (float)j / lon_count;
            t = (float)i / lat_count;

            tex_coords.push_back(glm::vec2(s, t));
        }
    }

    int k1, k2;

    for(int i = 0; i < lat_count; ++i)
    {
        k1 = i * (lon_count + 1);     // beginning of current stack
        k2 = k1 + lon_count + 1;      // beginning of next stack

        for(int j = 0; j < lon_count; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if(i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            // k1+1 => k2 => k2+1
            if(i != (lat_count-1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    return { indices, vertices, tex_coords };
}

void render_first_pass(uint32_t fbo_id, uint32_t color_tex_id)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);

    glEnable(GL_DEPTH_TEST);

    glClearColor(0.22f, 0.24f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void render_second_pass(uint32_t program_id, uint32_t color_tex_id, uint32_t vao_id)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(program_id);

    glBindVertexArray(vao_id);
    
    glBindTexture(GL_TEXTURE_2D, color_tex_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0); 
}

bool load_frame(const char* path, uint8_t** data, uint32_t* width, uint32_t* height)
{
    AVFormatContext* av_format_cxt = avformat_alloc_context();
    if(!av_format_cxt){
        std::cerr << "ERROR::AVFORMAT::Failed to create avformat context." << std::endl;
        return false;
    }

    if(avformat_open_input(&av_format_cxt, path, nullptr, nullptr) != 0){
        std::cerr << "ERROR::AVFORMAT::Failed to open video file." << std::endl;
        return false;
    }
    
    int video_stream_index = -1;

    const AVCodec* av_codec;
    AVCodecParameters* av_codec_params;

    for(int i = 0; i < av_format_cxt->nb_streams; i++){
        auto stream = av_format_cxt->streams[i];

        av_codec_params = stream->codecpar;
        av_codec = avcodec_find_decoder(av_codec_params->codec_id);  
        if(!av_codec)
            continue;

        if(av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream_index = i;
            *width = av_codec_params->width;
            *height = av_codec_params->height;
            //time_base = av_format_ctx->streams[i]->time_base;
            break;
        }
    }

    if (video_stream_index == -1) {
        printf("Couldn't find valid video stream inside file\n");
        return false;
    }

    // Set up a codec context for the decoder
    AVCodecContext* av_codec_ctx = avcodec_alloc_context3(av_codec);
    if (!av_codec_ctx) {
        printf("Couldn't create AVCodecContext\n");
        return false;
    }
    if (avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0) {
        printf("Couldn't initialize AVCodecContext\n");
        return false;
    }
    if (avcodec_open2(av_codec_ctx, av_codec, NULL) < 0) {
        printf("Couldn't open codec\n");
        return false;
    }

    AVFrame* av_frame = av_frame_alloc();
    AVPacket* av_packet = av_packet_alloc();

    av_frame = av_frame_alloc();
    if (!av_frame) {
        printf("Couldn't allocate AVFrame\n");
        return false;
    }
    av_packet = av_packet_alloc();
    if (!av_packet) {
        printf("Couldn't allocate AVPacket\n");
        return false;
    }

    while(av_read_frame(av_format_cxt, av_packet) >= 0){
        if(av_packet->stream_index != video_stream_index)
            continue;

        int response = avcodec_send_packet(av_codec_ctx, av_packet);
        if(response < 0){
            printf("Failed to decode packet\n");
            return false;
        }

        response = avcodec_receive_frame(av_codec_ctx, av_frame);
        if(response == AVERROR(EAGAIN) || response == AVERROR_EOF)
            continue;
        if(response <0){
            printf("Failed to decode packet\n");
            return false;
        }        
    }

    *data = new uint8_t[av_frame->width * av_frame->height * 4];

    for(int y = 0; y < av_frame->height; y++)
        for(int x = 0; x < av_frame->width; x++)
        {
            (*data)[(y * av_frame->width * 4) + x * 4    ] = av_frame->data[0][y * av_frame->linesize[0] + x];
            (*data)[(y * av_frame->width * 4) + x * 4 + 1] = av_frame->data[0][y * av_frame->linesize[0] + x];
            (*data)[(y * av_frame->width * 4) + x * 4 + 2] = av_frame->data[0][y * av_frame->linesize[0] + x];
            (*data)[(y * av_frame->width * 4) + x * 4 + 3] = av_frame->data[0][y * av_frame->linesize[0] + x];
            (*data)[(y * av_frame->width * 4) + x * 4 + 4] = 0xff;
        }


    return true;
}


double mouse_x_offset = 0.0;
double mouse_y_offset = 0.0;

double mouse_x_pos = 0.0;
double mouse_y_pos = 0.0;

bool mouse_pressed  = false;
bool mouse_released = false;

void mouse_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    mouse_x_offset = xoffset;
    mouse_y_offset = yoffset;
}
void mouse_cursor_callback(GLFWwindow *window, double xpos, double ypos)
{
    mouse_x_pos = xpos;
    mouse_y_pos = ypos;
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if(action == GLFW_PRESS)
    {
        mouse_pressed = true;
        mouse_released = false;
    }
    else if(action == GLFW_RELEASE)
    {
        mouse_pressed = false;
        mouse_released = true;
    }
}

glm::vec2 diff;
glm::vec2 prev_mouse_pos;

bool is_initial_press = true;

void mouse_drag_rotate(void)
{
    glm::vec2 current_pos(mouse_x_pos, mouse_y_pos);
    
    if(mouse_pressed)
    {
        if(is_initial_press)
        {
            is_initial_press = false;

            prev_mouse_pos.x = current_pos.x;
            prev_mouse_pos.y = current_pos.y;
        }

        diff = current_pos - prev_mouse_pos;
    }
    else if(mouse_released)
    {
        is_initial_press = true;

        diff = glm::vec2(0.0f);
        prev_mouse_pos = glm::vec2(0.0f);
    }
}

int main(int argc, char** args)
{
    uint32_t gpu_program_id          = 0;   // GPU program ID.
    uint32_t screen_quad_vao_id      = 0;   // Screen quad vertex array object ID.
    uint32_t screen_quad_vbo_id      = 0;   // Screen quad vertex buffer object ID.
    uint32_t framebuffer_object_id   = 0;   // Framebuffer Object ID.
    uint32_t color_buffer_texture_id = 0;   // Color buffer texture attachment ID.
    uint32_t depth_buffer_texture_id = 0;   // Depth buffer texture attachment ID.

    uint32_t uv_sphere_vao_id    = 0;       // UV Sphere vertex array object ID.
    uint32_t uv_sphere_ebo_id    = 0;       // UV Sphere index ebo ID.
    uint32_t uv_sphere_vx_vbo_id = 0;       // UV Sphere vertex vbo ID.
    uint32_t uv_sphere_uv_vbo_id = 0;       // UV Sphere texture coordinates vbo ID.

    uint32_t uv_sphere_tex_id = 0;          // UV Sphere texture ID.

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view  = glm::mat4(1.0f);
    glm::mat4 proj  = glm::mat4(1.0f);

    glm::vec3 uv_sphere_pos = { 0.0f, 0.0f, -3.0f };
    glm::vec3 uv_sphere_scl = { 10.0f, 10.0f, 10.0f };

    float camera_fov     = 45.0f;
    glm::vec3 camera_pos = {0.0f, 0.0f, 15.0f};

    std::vector<uint32_t>  ind;
    std::vector<glm::vec3> vxs;
    std::vector<glm::vec2> uvs;
   
    if(argc <= 1 || argc > 2)
    {
        printf("Invalid Arguments\n");
        return 1;
    }

    const char* video_path = args[1];
    
    VideoReaderState vr_state;
    if (!video_reader_open(&vr_state, video_path)){
        printf("Couldn't open video file (make sure you set a video file that exists)\n");
        return 1;
    }

    constexpr int ALIGNMENT = 128;
    const int frame_width = vr_state.width;
    const int frame_height = vr_state.height;
    uint8_t* frame_data = (uint8_t*)_aligned_malloc(frame_width * frame_height * 4, ALIGNMENT);

    std::tie(ind, vxs, uvs) = generate_uvspehre(32, 64);

    WindowDesc window_desc;
    Application app({3, 2});

    app.OnStart([&]() -> void
        {
            // Generate vertex buffer object for vertex positions.
            GL_ERR(glGenBuffers(1, &uv_sphere_vx_vbo_id))
            GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, uv_sphere_vx_vbo_id))
            GL_ERR(glBufferData(GL_ARRAY_BUFFER, vxs.size() * sizeof(glm::vec3), vxs.data(), GL_STATIC_DRAW))
            GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, 0))

            // Generate vertex buffer object for texture coordinates.
            GL_ERR(glGenBuffers(1, &uv_sphere_uv_vbo_id))
            GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, uv_sphere_uv_vbo_id))
            GL_ERR(glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW))
            GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, 0))

            // Generate index buffer object.
            GL_ERR(glGenBuffers(1, &uv_sphere_ebo_id))
            GL_ERR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uv_sphere_ebo_id))
            GL_ERR(glBufferData(GL_ELEMENT_ARRAY_BUFFER, ind.size() * sizeof(uint32_t), ind.data(), GL_STATIC_DRAW))

            // Generate vertex array object.
            GL_ERR(glGenVertexArrays(1, &uv_sphere_vao_id))
            GL_ERR(glBindVertexArray(uv_sphere_vao_id))

            // Set the layout of vertex positions.
            GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, uv_sphere_vx_vbo_id))
            GL_ERR(glEnableVertexAttribArray(0))
            GL_ERR(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0))
            GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, 0))

            // Set the layout of texture coordinates.
            GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, uv_sphere_uv_vbo_id))
            GL_ERR(glEnableVertexAttribArray(1))
            GL_ERR(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0))
            GL_ERR(glBindBuffer(GL_ARRAY_BUFFER, 0))

            GL_ERR(glBindVertexArray(0))

            // Generate texture for UV Shere.
            glGenTextures(1, &uv_sphere_tex_id);
            glBindTexture(GL_TEXTURE_2D, uv_sphere_tex_id);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glBindTexture(GL_TEXTURE_2D, 0);

            GL_ERR(glFrontFace(GL_CW))

            GL_ERR(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL))

            GL_ERR(glEnable(GL_CULL_FACE))
            GL_ERR(glEnable(GL_DEPTH_TEST))
        }
    );

    app.Init(window_desc);

    init_gpu_program(&gpu_program_id);

    app.SetMouseScrollCallback(mouse_scroll_callback);
    app.SetMouseCursorCallback(mouse_cursor_callback);
    app.SetMouseButtonCallback(mouse_button_callback);

    glm::vec2 curr_angle = glm::vec2(0.0f);

    app.OnUpdate([&]() -> void
        {
            int64_t pts;
            if (!video_reader_read_frame(&vr_state, &frame_data, &pts)) {
                printf("Couldn't load video frame\n");
            }

            static bool first_frame = true;
            if (first_frame) {
                glfwSetTime(0.0);
                first_frame = false;
            }

            double pt_in_seconds = pts * (double)vr_state.time_base.num / (double)vr_state.time_base.den;
            while (pt_in_seconds > glfwGetTime()) {
                glfwWaitEventsTimeout(pt_in_seconds - glfwGetTime());
            }

            glBindTexture(GL_TEXTURE_2D, uv_sphere_tex_id);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame_width, frame_height, GL_RGBA, GL_UNSIGNED_BYTE, frame_data);
            //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_data);

            GL_ERR(glClearColor(0.22f, 0.24f, 0.25f, 1.0f))
            GL_ERR(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))

            mouse_drag_rotate();

            curr_angle += (diff * 0.025f);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(curr_angle.x), glm::vec3(0.0, 1.0, 0.0));
            model = glm::rotate(model, glm::radians(curr_angle.y), glm::vec3(1.0, 0.0, 0.0));
            model = glm::scale(model, uv_sphere_scl);

            view = glm::lookAt(camera_pos, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            
            camera_fov += mouse_y_offset;

            proj = glm::perspective(glm::radians(45.0f), 
                (float)window_desc.m_window_width / (float)window_desc.m_window_height, 
                0.1f, 1000.0f);

            GL_ERR(glUseProgram(gpu_program_id))

            unsigned int model_matrix_id = glGetUniformLocation(gpu_program_id, "model_matrix");
            GL_ERR(glUniformMatrix4fv(model_matrix_id, 1, GL_FALSE, glm::value_ptr(model)))

            unsigned int view_matrix_id = glGetUniformLocation(gpu_program_id, "view_matrix");
            GL_ERR(glUniformMatrix4fv(view_matrix_id, 1, GL_FALSE, glm::value_ptr(view)))

            unsigned int proj_matrix_id = glGetUniformLocation(gpu_program_id, "proj_matrix");
            GL_ERR(glUniformMatrix4fv(proj_matrix_id, 1, GL_FALSE, glm::value_ptr(proj)))
            
            GL_ERR(glBindVertexArray(uv_sphere_vao_id))
            glBindTexture(GL_TEXTURE_2D, uv_sphere_tex_id);
            GL_ERR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uv_sphere_ebo_id))
            GL_ERR(glDrawElements(GL_TRIANGLES, ind.size(), GL_UNSIGNED_INT, nullptr))
            glBindTexture(GL_TEXTURE_2D, 0);
            GL_ERR(glBindVertexArray(0))

            mouse_y_offset = 0.0;
        }
    );

    while(app.IsRunning())
    {
        app.Tick();
        app.Poll();
    }

    video_reader_close(&vr_state);

    return EXIT_SUCCESS;
}