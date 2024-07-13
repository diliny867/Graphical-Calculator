#pragma once

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
#include <queue>


#include "../myGL/Shader.h"
#include "../myGL/VAO.h"
#include "../myGL/VBO.h"
//#include "../myGL/EBO.h" //unused
#include "../myGL/Time.h"
#include "../myGL/Texture2D.h"


#include "../include/Function.h"
#include "../include/TextRender.h"
#include "../include/Mouse.h"


namespace Application {
    //callbacks and some other functions
    //namespace GLFWCallbacks {
    //    extern void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    //    extern void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos);
    //    extern void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
    //    extern void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
    //}
    namespace RenderAxisNumbersPrecision { //for calculation of precision for number on number axis
        inline int xprecision = Function::numbers_float_precision;
        inline int yprecision = Function::numbers_float_precision;
        inline auto xformatting = std::fixed;
        inline auto yformatting = std::fixed;

        extern void updatePrecision();
    }

    class App {
    private:
        void processInput(GLFWwindow* window);
        void showFPS();
        std::string getFPS_str(int precision);
        std::function<void()> ViewpointUpdateShaderCallback;

        
        void RenderAxisNumbers(const Shader& shader, glm::vec2 center, glm::vec2 size, float scale, glm::vec3 color);

        glm::vec2 screenSize{900, 900};
        Mouse mouse{screenSize.x/2.0f, screenSize.y/2.0f};
        MouseDot mouseDot{};

        class FuncData {
        public:
            Function function;
            std::string inputData;
            std::queue<std::future<void>> futures;
            glm::vec3 color = glm::vec3(1.0f);
            GLuint vbo = 0;
            GLuint vao = 0;
            bool show = true;
            bool need_remap_vbo = false;
        };
        std::vector<FuncData*> functions;

        bool updateFunctions = true;
        bool needUpdateShaders = false;

        std::atomic_bool appOn = true;
        std::mutex m;

        GLFWwindow* appWindow = nullptr;
        ImGuiIO* imGuiIO = nullptr;

        struct {
            Shader funcShader;
            Shader coordAxisShader;
            Shader coordAxisNumbersShader;
            Shader textShader;
            Shader mouseDotShader;
        } shaders = {};

        struct {
            const float buttonsInc = 0.15f;
        } imGuiData = {};

        struct {
            GLuint vao;
            GLuint vbo;
            GLuint mouseDotVAO;
            GLuint mouseDotVBO;
        } mainGlObjects = {};

        void CreateImGuiMenu();
        void CreateMouseDot();

        int GLInit();
        int Init();

        void Cleanup();

        inline static App* instance = nullptr;
    public:
        inline static App* GetInstance() {
            return instance;
        }
        struct Callbacks{
            static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
            static void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos);
            static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
            static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        };
        inline glm::vec2 GetScreenSize() {
            return screenSize;
        }
        inline Mouse& GetMouse() {
            return mouse;
        }
        int Run();
    };

}
