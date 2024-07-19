#pragma once

#include "Function.h"

#include <glad/glad.h>//should always be first
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <queue>
#include <list>
#include <future>

#include "Common.h"

namespace Application{
    class FuncData {
    public:
        Function function;
        std::string inputData;
        std::queue<std::future<void>> futures;
        glm::vec3 color = glm::vec3(1.0f);
        GLuint vbo = 0;
        GLuint vao = 0;
        bool show = true;
        bool needRemapVBO = false;
    };
    class FunctionSystem {
    private:
        std::mutex m;
        glm::vec2 screen = {800,600};
        glm::vec2 center = {0,0};
        glm::vec2 size = {10,10};

        inline static FunctionSystem* instance = nullptr;
    public:
        int numbersFloatPrecision = 1;

        inline static bool allDirty = false;

        ExprStrParser::Parser exprStrParser;

        std::vector<FuncData*> functions;

        void SetScreen(const glm::vec2& size);
        glm::vec2 GetScreen();
        void SetSize(const glm::vec2& size);
        void IncSize(const glm::vec2& delta);
        void MultSize(const glm::vec2& delta);
        glm::vec2 GetSize();
        void SetCenter(const glm::vec2& center);
        void IncCenter(const glm::vec2& delta);
        void MultCenter(const glm::vec2& delta);
        glm::vec2 GetCenter();
        glm::vec2 GetCenterNDC();

        FunctionSystem();
        FunctionSystem(const glm::vec2& screenSize);

        inline static FunctionSystem* GetInstance() {
            return instance;
        }
    };
}