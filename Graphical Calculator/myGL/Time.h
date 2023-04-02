#pragma once

#include <GLFW/glfw3.h>
#include <mutex>

class Time
{
private:
    inline static std::once_flag onceFlag;
public:
    Time() = delete;
    inline static double time;
    inline static double lastTime;
    inline static double deltaTime;
    static void Init() {
        std::call_once(onceFlag, []
            {
                time = 0.0;
                lastTime = 0.0;
                deltaTime = 0.0;
            });
    }
    static void Update() {
        time = glfwGetTime();
        deltaTime = time - lastTime;
        lastTime = time;
    }
};