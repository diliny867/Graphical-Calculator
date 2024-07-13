#pragma once

#include <glm/glm.hpp>

namespace Application {
    class Mouse {
    public:
        Mouse(const float x, const float y):pos({x,y}){}
        Mouse():Mouse(0,0){}
        glm::vec2 pos;
        bool leftPressed = false;
        bool rightPressed = false;
        bool wheelScrolled = false;
        bool firstInput = true;
    };

    class MouseDot {
        //constexpr float defaultMarkerSize = 0.002f;
        //float markerSize = defaultMarkerSize;
    public:
        bool funcCaptured = false;
        Function* func = nullptr;
        std::thread* thread;
        bool byDistance = true;
        glm::vec2 screenPos = {0,0};
        glm::vec3 color = {1,1,1};
    };
}
