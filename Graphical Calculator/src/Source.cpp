#include <glad/glad.h>//should always be first
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>//OpenGL Mathematics
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_stdlib.h>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string> 
#include <future>

#include "../include/Function.h"

#include <myGL/Shader.h>
#include <myGL/VAO.h>
#include <myGL/VBO.h>
#include <myGL/EBO.h>
#include <myGL/Time.h>
#include <myGL/Texture2D.h>

#include <ft2build.h>
#include FT_FREETYPE_H


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void showFPS();
std::string getFPS_str(int precision);
void RenderText(const Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color);
void RenderAxisNumbers(const Shader& shader, glm::vec2 center, glm::vec2 size, float scale, glm::vec3 color);

constexpr int SCR_WIDTH = 800;
constexpr int SCR_HEIGHT = 600;

//mouse last positions
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

bool wheel_scrolled = false;
bool mouse_unpressed = false;

//assign values for update thresholds
/* currently unused
float last_updated_mouse_x = lastX;
float last_updated_mouse_y = lastY;
constexpr float mouse_move_update_threshold = 10.0f; //px

float last_updated_size_x = 20.0f;
float last_updated_size_y = 20.0f;
constexpr float size_update_threshold = 0.1f; // %/100
*/

constexpr float defaultMarkerSize = 0.002f;
float markerSize = defaultMarkerSize;

bool leftPressed = false;

Function function(SCR_WIDTH, SCR_HEIGHT);
std::size_t func_points_count = function.points.size();

float last_centerx = 0;

struct Character {
    GLuint TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};
std::map<GLchar, Character> Characters;
GLuint characterVAO, characterVBO;

bool update_func = true;

namespace RenderAxisNumbersPrecision {
    int xprecision = Function::float_precision;
    int yprecision = Function::float_precision;
    auto xformatting = std::fixed;
    auto yformatting = std::fixed;

    void updatePrecision() {
        xprecision = Function::float_precision;
        yprecision = Function::float_precision;
        xformatting = std::fixed;
        yformatting = std::fixed;
        const glm::vec2 f_size = function.getSize();
        if (f_size.x<2.0f) {
            xprecision += 1;
            if (f_size.x<0.4f) {
                xprecision += 1;
                if (f_size.x<0.01f) {
                    xprecision = 0;
                    xformatting = std::scientific;
                }
            }
        } else if (f_size.x>500.0f) {
            xprecision = 0;
            if (f_size.x>10000.0f) {
                xformatting = std::scientific;
            }
        }

        if (f_size.y<2.0f) {
            yprecision += 1;
            if (f_size.y<0.4f) {
                yprecision += 1;
                if (f_size.y<0.01f) {
                    yprecision = 0;
                    yformatting = std::scientific;
                }
            }
        } else if (f_size.y>500.0f) {
            yprecision = 0;
            if (f_size.y>10000.0f) {
                yformatting = std::scientific;
            }
        }
    }
}

int main() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//opengl versions
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//set to core profile

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL application", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_cursor_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, mouse_scroll_callback);
    glfwSwapInterval(0);//VSync
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//capture mouse

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    //glEnable(GL_DEPTH_TEST); //enable z-buffer
	//glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    { //FreeType
        FT_Library ft;
        // All functions return a value different than 0 whenever an error occurred
        if (FT_Init_FreeType(&ft)) {
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return -1;
        }

        // find path to font
        const std::string font_name = "resources/Arial.ttf";

        // load font as face
        FT_Face face;
        if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
            return -1;
        } else {
            // set size to load glyphs as
            FT_Set_Pixel_Sizes(face, 0, 48);

            // disable byte-alignment restriction
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            // load first 128 characters of ASCII set
            for (unsigned char c = 0; c < 128; c++) {
                // Load character glyph 
                if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                    std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                    continue;
                }
                // generate texture
                const Texture2D char_texture((GLsizei)face->glyph->bitmap.width, (GLsizei)face->glyph->bitmap.rows, GL_RED, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
                // store character for later use
                Character character = {
                    char_texture.id,
                    glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                    glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                    static_cast<unsigned int>(face->glyph->advance.x)
                };
                Characters.insert(std::pair<char, Character>(c, character));
            }
            Texture2D::unbind();
           // glBindTexture(GL_TEXTURE_2D, 0);
        }
        // destroy FreeType once we're finished
        FT_Done_Face(face);
        FT_Done_FreeType(ft);


        // configure VAO/VBO for texture quads
        VAO::generate(characterVAO);
        VBO::generate(characterVBO);
        VAO::bind(characterVAO);
        VBO::bind(characterVBO);
        VBO::setData(characterVBO, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        VAO::addAttrib(characterVAO, 0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        VBO::unbind();
        VAO::unbind();
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    const ImGuiIO& imGuiIO = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    Time::Init();

    //Shader shader("shaders/shader_vs.glsl", "shaders/shader_fs.glsl");
	//Shader funcShader("resources/shaders/shader_vs.glsl", "resources/shaders/shader_gs.glsl", "resources/shaders/shader_fs.glsl");
	Shader funcShader("resources/shaders/shader2_vs.glsl", "resources/shaders/shader2_gs.glsl", "resources/shaders/shader2_fs.glsl");
	Shader coordAxisShader("resources/shaders/coordAxisShader_vs.glsl", "resources/shaders/coordAxisShader_gs.glsl", "resources/shaders/coordAxisShader_fs.glsl");
	Shader coordAxisNumbersShader("resources/shaders/coordAxisNumbersShader_vs.glsl", "resources/shaders/coordAxisNumbersShader_fs.glsl");

    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(SCR_WIDTH), 0.0f, static_cast<float>(SCR_HEIGHT));
    coordAxisNumbersShader.use();
    coordAxisNumbersShader.setMat4("projection", projection);

    GLuint vbo;
    VBO::generate(vbo, static_cast<GLsizeiptr>(func_points_count*sizeof(glm::vec2)), function.points.data(), GL_DYNAMIC_DRAW);
    VBO::bind(vbo);
    GLuint vao;
    VAO::generate(vao);
    VAO::bind(vao);
    VAO::addAttrib(vao, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    //GLuint ebo;
    //EBO::generate(ebo, sizeof(indices), indices, GL_STATIC_DRAW);
    
    std::string inputData = "sin(x)+2";
    function.setFunction(inputData);
    function.recalculatePoints();

    funcShader.use();
    //constexpr float markerSize = 0.0f;
    //funcShader.setFloat("markerRadius", markerSize);
    funcShader.setInt("pointCount", static_cast<int>(func_points_count));
    funcShader.setFloat("ycenter", 0.0f);
    funcShader.setVec3("color", glm::vec3(1.0f)); //desmos color: glm::vec3(199.0f, 68.0f, 64.0f)/255.0f
    //glUniform2fv(glGetUniformLocation(funcShader.id, "vPoints"), static_cast<GLsizei>(function.points.size()), reinterpret_cast<float*>(function.points.data()));

    coordAxisShader.use();
    coordAxisShader.setVec2("center", 0.0f, 0.0f);
    
    glm::vec3 coordAxisColor = glm::vec3(0.5f);
    coordAxisShader.setVec3("color", coordAxisColor);

    constexpr float buttonsInc = 0.15f;

    auto remap_vbo = [](GLuint _vbo) {
        VBO::bind(_vbo);
        void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(ptr, function.points.data(), func_points_count*sizeof(glm::vec2)); //copy points data to vbo
        glUnmapBuffer(GL_ARRAY_BUFFER);
        //VBO::setSubData(_vbo, 0, static_cast<GLsizeiptr>(func_points_count*sizeof(glm::vec2)), function.points.data()); //what is faster?
    };

    //function.recalculatePoints();
    //std::thread th(std::ref(remap_vbo), vbo); //cant do this, because different thread cant execute callback on main thread

    std::vector<std::future<void>> futures;
    futures.push_back({ std::async(std::launch::async, [] {function.recalculatePoints(); }) });
    bool need_remap_vbo = false;

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        Time::Update();
        //showFPS(); //print fps

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Settings");
        {
            if (ImGui::Button("Home")) {
                function.setSize(10.0f, 10.0f);
                function.setCenter(0.0f, 0.0f);
                RenderAxisNumbersPrecision::updatePrecision();
            }
            if (ImGui::Button("+")) {
                function.multSize(1.0f-buttonsInc, 1.0f-buttonsInc);
                RenderAxisNumbersPrecision::updatePrecision();
            }
            if (ImGui::Button("-")) {
                function.multSize(1.0f+buttonsInc, 1.0f+buttonsInc);
                RenderAxisNumbersPrecision::updatePrecision();
            }
            if (ImGui::Button("X +")) {
                function.multSize(1.0f-buttonsInc, 1.0f);
                RenderAxisNumbersPrecision::updatePrecision();
            }
            if (ImGui::Button("X -")) {
                function.multSize(1.0f+buttonsInc, 1.0f);
                RenderAxisNumbersPrecision::updatePrecision();
            }
            if (ImGui::Button("Y +")) {
                function.multSize(1.0f, 1.0f-buttonsInc);
                RenderAxisNumbersPrecision::updatePrecision();
            }
            if (ImGui::Button("Y -")) {
                function.multSize(1.0f, 1.0f+buttonsInc);
                RenderAxisNumbersPrecision::updatePrecision();
            }
            if (ImGui::InputText("Input function", &inputData)) {
                function.setFunction(inputData);
                futures.push_back({ std::async(std::launch::async, [] {function.recalculatePoints(); }) });
            }
        }
        ImGui::End();

        if(imGuiIO.WantCaptureMouse) {
            update_func = false;
        }else {
            update_func = true;
        }

        if (function.needs_update) {
            futures.push_back({ std::async(std::launch::async, [] {function.recalculatePoints(); }) });
            function.needs_update = false;
        }
        for(auto it = futures.begin();it<futures.end();) {
	        if(it->_Is_ready()) { //if future is ready, remap vbo and remove the future from vector
                it = futures.erase(it);
                need_remap_vbo = true;
	        }else {
                ++it;
	        }
        }
        if(need_remap_vbo) {
            coordAxisShader.use();
            coordAxisShader.setVec2("center", function.getCenter().x, function.getCenter().y);
            funcShader.use();
            funcShader.setFloat("ycenter", function.getCenter().y);
            remap_vbo(vbo);
            need_remap_vbo = false;
        }

    	//if (function.needs_update) {
    	//	coordAxisShader.use();
    	//	coordAxisShader.setVec2("center", function.getCenter().x, function.getCenter().y);
    	//	funcShader.use();
        //    funcShader.setFloat("ycenter", function.getCenter().y);
        //    //std::cout<<mouse_unpressed<<std::endl;
        //    //glm::vec2 curr_func_size = function.getSize();
        //    if (abs(lastX-last_updated_mouse_x)>=mouse_move_update_threshold || abs(lastY-last_updated_mouse_y)>=mouse_move_update_threshold || wheel_scrolled || mouse_unpressed
        //        /*|| abs(curr_func_size.x-last_updated_size_x)>=size_update_threshold || abs(curr_func_size.y-last_updated_size_y)>=size_update_threshold*/) {
        //        function.recalculatePoints();
        //        recalculatePointsCallback(vbo);
        //        last_updated_mouse_x = lastX;
        //        last_updated_mouse_y = lastY;
        //        //last_updated_size_x = curr_func_size.x;
        //        //last_updated_size_y = curr_func_size.y;
        //        last_centerx = function.getCenter().x;
        //        wheel_scrolled = false;
        //        mouse_unpressed = false;
        //        funcShader.setFloat("xmove", 0.0f);
        //    } else {
        //        funcShader.setFloat("xmove", function.getCenter().x-last_centerx);
        //    }
    	//	function.needs_update = false;
    	//}


    	glLineWidth(1);
        VAO::bind(vao);
        coordAxisShader.use();
        glDrawArrays(GL_POINTS, 0, 1);

        RenderAxisNumbers(coordAxisNumbersShader, function.getCenter(), function.getSize(), 0.25f, glm::vec3(0.7f));

        glLineWidth(3);
        VAO::bind(vao);
        funcShader.use();
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, static_cast<GLsizei>(func_points_count));
        

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        RenderText(coordAxisNumbersShader, "FPS: "+getFPS_str(2), SCR_WIDTH*0.03f, SCR_HEIGHT*0.97f, 0.25f, glm::vec3(0.4f)); //render fps

    	glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}

void showFPS() {
    std::cout << getFPS_str(5) << std::endl;
}

std::string getFPS_str(const int precision) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << 1/Time::deltaTime;
    return ss.str();
}


void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouse_cursor_callback(GLFWwindow* window, const double xpos, const double ypos) {

	const float xposIn = static_cast<float>(xpos);
	const float yposIn = static_cast<float>(ypos);

    if (firstMouse) {
        lastX = xposIn;
        lastY = yposIn;
        firstMouse = false;
    }

    if (update_func) {
        if (leftPressed) {
            const float deltaX = xposIn-lastX;
            const float deltaY = yposIn-lastY;
        	function.incCenter(deltaX/(SCR_WIDTH/2.0f), deltaY/(SCR_HEIGHT/2.0f));
        }
    }

    lastX = xposIn;
    lastY = yposIn;
}

void mouse_button_callback(GLFWwindow* window, const int button, const int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        leftPressed = true;
    } else if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
        leftPressed = false;
        function.needs_update = true;
        mouse_unpressed = true;
    }
}

void mouse_scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset) {
    wheel_scrolled = true;
    if (update_func) {
        function.incCenter( -(lastX/static_cast<float>(SCR_WIDTH) -function.getCenterNDC().x)*(static_cast<float>(yoffset)/10.0f),
							-(lastY/static_cast<float>(SCR_HEIGHT)-function.getCenterNDC().y)*(static_cast<float>(yoffset)/10.0f));
        function.multSize((1.0f-static_cast<float>(yoffset)/20.0f), (1.0f-static_cast<float>(yoffset)/20.0f));

        RenderAxisNumbersPrecision::updatePrecision();
    }
}

void RenderAxisNumbers(const Shader& shader, const glm::vec2 center, const glm::vec2 size, const float scale, const glm::vec3 color) {
	const glm::vec2 interval = size/5.0f;

    std::stringstream ss;

    for(int i = -10;i<10;i++) {
        ss.str("");
        ss << RenderAxisNumbersPrecision::xformatting << std::setprecision(RenderAxisNumbersPrecision::xprecision) << interval.x*(static_cast<float>(i)-center.x*10.0f)/2.0f;
        RenderText(shader, ss.str(), static_cast<float>(i+10)/20.0f*SCR_WIDTH, (1.0f-(center.y+1.0f)/2.0f)*SCR_HEIGHT, scale , color);
        ss.str("");
        ss << RenderAxisNumbersPrecision::yformatting << std::setprecision(RenderAxisNumbersPrecision::yprecision) << interval.y*(static_cast<float>(i)+center.y*10.0f)/2.0f;
        RenderText(shader, ss.str(), ((center.x+1.0f)/2.0f)*SCR_WIDTH, static_cast<float>(i+10)/20.0f*SCR_HEIGHT, scale, color);
    }
}

void RenderText(const Shader& shader, std::string text, float x, float y, const float scale, const glm::vec3 color) {
    // activate corresponding render state	
    shader.use();
    shader.setVec3("textColor", color.x, color.y, color.z);
    //glUniform3f(glGetUniformLocation(shader.id, "textColor"), color.x, color.y, color.z);
    Texture2D::deactivate();
    //glActiveTexture(GL_TEXTURE0);
    VAO::bind(characterVAO);

    // iterate through all characters
    for (std::string::const_iterator c = text.begin(); c < text.end(); ++c) {
	    const Character ch = Characters[*c];

        const float xpos = x + static_cast<float>(ch.Bearing.x) * scale;
        const float ypos = y - static_cast<float>(ch.Size.y - ch.Bearing.y) * scale;

	    const float w = static_cast<float>(ch.Size.x) * scale;
	    const float h = static_cast<float>(ch.Size.y) * scale;
        // update VBO for each character
        const float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // update content of VBO memory
        VBO::bind(characterVBO);
        VBO::setSubData(characterVBO, 0, sizeof(vertices), vertices);
        //glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
        VBO::unbind();

        // render quad with character
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += static_cast<float>(ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    VAO::unbind();
    Texture2D::deactivate();
    //glActiveTexture(GL_TEXTURE0);
}


void framebuffer_size_callback(GLFWwindow* window, const int width, const int height) {
    glViewport(0, 0, width, height);//0,0 - left bottom
}