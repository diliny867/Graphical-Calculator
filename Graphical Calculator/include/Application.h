#pragma once

#include <glad/glad.h>//should always be first
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>//OpenGL Mathematics

#include <imgui/imgui.h>

#include <string> 
#include <future>
#include <queue>

#include "../myGL/Shader.h"

#include "../include/FunctionSystem.h"
#include "../include/Function.h"
#include "../include/Mouse.h"


namespace Application {
    namespace RenderAxisNumbersPrecision { //for calculation of precision for number on number axis
        inline int xprecision = Function::numbersFloatPrecision;
        inline int yprecision = Function::numbersFloatPrecision;
        inline auto xformatting = std::fixed;
        inline auto yformatting = std::fixed;

        extern void UpdatePrecision();
    }

    class App {
    private:
        std::atomic_bool appOn = true;
        std::mutex m;

        glm::vec2 screenSize{900, 900};

        Mouse mouse{screenSize.x/2.0f, screenSize.y/2.0f};
        MouseDot mouseDot{};

        FunctionSystem functionSystem = FunctionSystem(screenSize);

        bool updateFunctions = true;
        bool needUpdateShaders = false;
        bool viewPointUpdated = false;

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
            GLuint coordAxisVAO;
            GLuint mouseDotVAO;
            GLuint mouseDotVBO;
            glm::mat4 projection;
        } glData = {};

        struct Callbacks{
            static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
            static void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos);
            static void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
            static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        };

        void processInput(GLFWwindow* window);
        void showFPS();
        std::string getFPS_str(int precision);

        void renderAxisNumbers(const Shader& shader, glm::vec2 center, glm::vec2 size, float scale, glm::vec3 color);

        void pushNewFunction();
        void eraseFunction(const decltype(functionSystem.functions)::iterator& it); // because i can

        void drawFunctions();
        void drawMouseDot();

        void createImGuiMenu();
        void createMouseDot();

        int initGL();
        void initShaders();
        int init();

        inline static App* instance = nullptr;
    public:
        App();
        inline static App* GetInstance() {
            return instance;
        }
        inline void SetDirty() {
            needUpdateShaders = true;
            FunctionSystem::allDirty = true;
            RenderAxisNumbersPrecision::UpdatePrecision();
        }
        inline glm::vec2 GetScreenSize() {
            return screenSize;
        }
        inline Mouse& GetMouse() {
            return mouse;
        }
        int Run();

        ~App();
    };
}
