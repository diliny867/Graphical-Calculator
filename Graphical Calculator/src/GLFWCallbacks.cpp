#include "../include/Application.h"

using namespace Application;

void App::Callbacks::mouseCursorCallback(GLFWwindow*window,double xpos,double ypos) {
    auto app = App::GetInstance();
    auto fs = FunctionSystem::GetInstance();
    Mouse& mouse = app->mouse;

    const float xposIn = static_cast<float>(xpos);
    const float yposIn = static_cast<float>(ypos);

    if(mouse.firstInput) {
        mouse.pos.x = xposIn;
        mouse.pos.y = yposIn;
        mouse.firstInput = false;
    }

    if(app->updateFunctions) {
        if(mouse.leftPressed) {
            const float deltaX = xposIn-mouse.pos.x;
            const float deltaY = yposIn-mouse.pos.y;
            fs->IncCenter({deltaX/(app->screenSize.x/2.0f), deltaY/(app->screenSize.y/2.0f)});
            app->needUpdateShaders = true;
            FunctionSystem::allDirty = true;
        }
    }

    mouse.pos.x = xposIn;
    mouse.pos.y = yposIn;
}

void App::Callbacks::mouseButtonCallback(GLFWwindow*window,int button,int action,int mods) {
    auto app = App::GetInstance();
    Mouse& mouse = app->mouse;
    if(action == GLFW_PRESS) {
        switch(button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            mouse.leftPressed = true;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            mouse.rightPressed = true;
            break;
        }
    } else if(action == GLFW_RELEASE) {
        switch(button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            mouse.leftPressed = false;
            app->needUpdateShaders = true;
            FunctionSystem::allDirty = true;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            mouse.rightPressed = false;
            break;
        }
    }
}

void App::Callbacks::mouseScrollCallback(GLFWwindow* window, const double xoffset, const double yoffset) {
    auto app = App::GetInstance();
    auto fs = FunctionSystem::GetInstance();
    Mouse& mouse = app->mouse;
    mouse.wheelScrolled = true;
    if(app->updateFunctions) {//scale functions
        const glm::vec2 mousePos = glm::vec2(mouse.pos.x/app->screenSize.x*2.0f-1.0f, mouse.pos.y/app->screenSize.y*2.0f-1.0f);
        const glm::vec2 lastValue = (-fs->GetCenter()+mousePos)*fs->GetSize();
        //const glm::vec2 lastCenterValue = (Function::getCenter()+glm::vec2(mouse.pos.x/screenSize.x*2-1, mouse.pos.y/screenSize.y*2-1))*Function::getSize();
        //Function::incCenter(-(mouse.pos.x/screenSize.x -Function::getCenterNDC().x)*(static_cast<float>(yoffset)/10.0f),
        //					-(mouse.pos.y/screenSize.y-Function::getCenterNDC().y)*(static_cast<float>(yoffset)/10.0f));
        fs->MultSize({(1.0f-static_cast<float>(yoffset)/20.0f), (1.0f-static_cast<float>(yoffset)/20.0f)});
        fs->SetCenter(-lastValue/fs->GetSize()+mousePos);
        app->SetDirty();
    }
}

void App::Callbacks::framebufferSizeCallback(GLFWwindow* window, const int width, const int height) {
    auto app = App::GetInstance();
    app->screenSize.x = static_cast<float>(width);
    app->screenSize.y = static_cast<float>(height);
    glViewport(0, 0, width, height);
    //0,0 - left bottom
    app->viewPointUpdated = true;
    //app->ViewpointUpdateShaderCallback();
}
