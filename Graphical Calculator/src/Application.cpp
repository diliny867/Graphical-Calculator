#include "../include/Application.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_stdlib.h>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "../myGL/VAO.h"
#include "../myGL/VBO.h"
#include "../myGL/Time.h"
#include "../myGL/Texture2D.h"

#include "../include/TextRender.h"

using namespace Application;

void App::pushNewFunction() { //creates and adds new empty function with random color
    std::lock_guard lg(m);
    functions.push_back(new FuncData());
    VBO::generate(functions.back()->vbo, static_cast<GLsizeiptr>((Function::calcPointsCount+2)*sizeof(glm::vec2)), functions.back()->function.points.data(), GL_DYNAMIC_DRAW);
    VAO::generate(functions.back()->vao);
    VAO::bind(functions.back()->vao);
    VAO::addAttrib(functions.back()->vao, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    functions.back()->color = glm::vec3(static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX));
    functions.back()->function.setFunction("");
}
void App::eraseFunction(const decltype(functions)::iterator& it) {
    std::lock_guard lg(m);
    VBO::deleteIt((*it)->vbo);
    VAO::deleteIt((*it)->vao);
    delete *it;
    functions.erase(it);
}

void App::drawFunctions() {
    glLineWidth(3);
    shaders.funcShader.use();
    for (std::size_t i = 0; i<functions.size(); i++) { //check if function recalculated and draw it
        auto& function = functions[i];
        if (!function->show) { continue; }
        if (Function::allDirty || function->function.dirty) {
            function->futures.push({ std::async(std::launch::async, [&, i] {function->function.recalculatePoints(); }) });
            function->function.dirty = false;
        }
        //if future is ready, remap vbo and remove the future from vector
        while (!function->futures.empty() && function->futures.front()._Is_ready()) { //with while it is a bit better at combattin artifacts, when changing zoom(size)
            function->futures.pop();
            function->needRemapVBO = true;
        }
        if (function->needRemapVBO) {
            VBO::bind(function->vbo);
            void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
            memcpy(ptr, function->function.points.data(), (Function::calcPointsCount+2)*sizeof(glm::vec2)); //copy points data to vbo
            glUnmapBuffer(GL_ARRAY_BUFFER);
            function->needRemapVBO = false;
        }

        VAO::bind(function->vao);
        shaders.funcShader.setVec3("color", function->color);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 1, static_cast<GLsizei>(Function::calcPointsCount+2)); // 2 side points are offscreen
    }
    Function::allDirty = false;
}
void App::drawMouseDot() {
    if (mouseDot.funcCaptured) { //draw mouse dot and value at it
        VAO::bind(glData.mouseDotVAO);
        VBO::setData(glData.mouseDotVBO, sizeof(glm::vec2), &mouseDot.screenPos, GL_STATIC_DRAW);
        glPointSize(7);
        shaders.mouseDotShader.use();
        glDrawArrays(GL_POINTS, 0, 1);
        const std::string dotText = std::to_string(Function::xsize*(mouseDot.screenPos.x-Function::xcenter)) + " " + std::to_string(mouseDot.func->calcAtScrPos(mouseDot.screenPos));
        TextRender::RenderText(shaders.textShader, dotText, screenSize.x*(mouseDot.screenPos.x+1.0f)*0.5f+10.0f, screenSize.y*(mouseDot.screenPos.y+1.0f)*0.5f, 0.3f, glm::vec3(0.7f));
    }
}

void App::createImGuiMenu() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Menu");
    { //draw GUI with ImGUI
        if (ImGui::Button("Home")) {
            Function::setSize(10.0f, 10.0f);
            Function::setCenter(0.0f, 0.0f);
            SetDirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("+")) {
            Function::multSize(1.0f-imGuiData.buttonsInc, 1.0f-imGuiData.buttonsInc);
            SetDirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("-")) {
            Function::multSize(1.0f+imGuiData.buttonsInc, 1.0f+imGuiData.buttonsInc);
            SetDirty();
        }
        if (ImGui::Button("X +")) {
            Function::multSize(1.0f-imGuiData.buttonsInc, 1.0f);
            SetDirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("X -")) {
            Function::multSize(1.0f+imGuiData.buttonsInc, 1.0f);
            SetDirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("Y +")) {
            Function::multSize(1.0f, 1.0f-imGuiData.buttonsInc);
            SetDirty();
        }
        ImGui::SameLine();
        if (ImGui::Button("Y -")) {
            Function::multSize(1.0f, 1.0f+imGuiData.buttonsInc);
            SetDirty();
        }
        //ImGui::Text("\n"); //somehow this makes indent smaller than " "
        ImGui::Checkbox("Mouse Dot by distance", &mouseDot.byDistance);
        ImGui::SameLine();
        if(ImGui::ColorEdit3("Mouse Dot color", value_ptr(mouseDot.color), ImGuiColorEditFlags_NoInputs)) {
            shaders.mouseDotShader.use();
            shaders.mouseDotShader.setVec3("color", mouseDot.color);
        }
        if (ImGui::Button("Add new function")) {
            pushNewFunction();
        }
        ImGui::SameLine();
        if (ImGui::Button("Remove last function")) {
            if (!functions.empty()) {
                eraseFunction(std::prev(functions.end()));
            }
        }
        for (std::size_t i = 0; i<functions.size(); i++) {
            auto& function = functions[i];
            auto& fFunction = function->function;
            ImGui::PushID((int)i);
            if (ImGui::Button("-")) {
                eraseFunction(functions.begin()+i);
                continue;
            }
            ImGui::SameLine();
            if (function->show) {
                if (ImGui::Button("hide")) {
                    function->show = false;
                }
            } else {
                if (ImGui::Button("show")) {
                    function->show = true;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Variables")) {
                if (!fFunction.exprStrParser.GetArgs().empty()) {
                    ImGui::OpenPopup("Variables");
                }
            }
            if (ImGui::BeginPopup("Variables")) {
                for (auto&[argName, argValue]: fFunction.exprStrParser.GetArgs()) {
                    ImGui::PushID((int)i+1);
                    if (ImGui::InputDouble(argName.c_str(), &argValue)) {
                        fFunction.exprStrParser.SetArgs(argName,argValue);
                        fFunction.dirty = true;
                    }
                    ImGui::PopID();
                    ImGui::SameLine();
                    const float inc = abs(argValue/10.0f);
                    if (ImGui::DragScalar(argName.c_str(), ImGuiDataType_Double, &argValue, inc==0.0f?0.001f:(inc>1000.0f?1000.0f:inc))) {
                        fFunction.exprStrParser.SetArgs(argName,argValue);
                        fFunction.dirty = true;
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::ColorEdit3("", value_ptr(function->color), ImGuiColorEditFlags_NoInputs);
            ImGui::SameLine();
            if (ImGui::InputText("", &function->inputData)) {
                //auto lastArgs = fFunction.exprStrParser.GetArgs();
                fFunction.setFunction(function->inputData);

                //for (auto& arg: fFunction.exprStrParser.GetArgs()) {
                //    fFunction.exprStrParser.SetArgs(arg.first, lastArgs[arg.first]);
                //}
                function->futures.push({ std::async(std::launch::async, [&, i] {fFunction.recalculatePoints(); }) });
            }
            ImGui::PopID();
        }
    }
    ImGui::End();
}

glm::vec2 getClosestPointOfLine(const glm::vec2& point, const glm::vec2& lineX1, const glm::vec2& lineX2) {
    const glm::vec2 lx1Tolx2 = lineX2 - lineX1;
    const glm::vec2 lx1ToP = point - lineX1;
    const float t = glm::dot(lx1ToP, lx1Tolx2) / glm::dot(lx1Tolx2, lx1Tolx2);
    return lineX1 + lx1Tolx2 * t; //basically lerp
};
void App::createMouseDot() {
    constexpr auto getFinalOnScreenPoint = [](const glm::vec2& target, const glm::vec2& middle, const glm::vec2& prev, const glm::vec2& next) -> glm::vec2 {
        const auto p1 = getClosestPointOfLine(target, middle, prev);
        const auto p2 = getClosestPointOfLine(target, middle, next);
        if(glm::distance(target, p1) < glm::distance(target, p2)) {
            return p1;
        } else{
            return p2;
        }
    };
    shaders.mouseDotShader.use();
    shaders.mouseDotShader.setVec3("color", mouseDot.color);
    mouseDot.thread = new std::thread([&]() { //thread that calculates poin for mouse
        bool lastLeftPressed = false;
        while (appOn) {
            glm::vec2 scrOffsetY = glm::vec2{0, Function::ycenter};
            if (mouse.leftPressed) {
                if (imGuiIO->WantCaptureMouse) {continue;}
                const glm::vec2 mousePos = glm::vec2(mouse.pos.x / screenSize.x * 2.0f - 1.0f, -(mouse.pos.y / screenSize.y * 2.0f - 1.0f)) + scrOffsetY;
                glm::vec2 closestPoint = { 4,4 };
                if (mouseDot.funcCaptured) {
                    if (mouseDot.byDistance) {
                        std::size_t closestAt = 0;
                        //for (const glm::vec2 point : mouseDot.func->points) {
                        for(std::size_t i = 0; i < mouseDot.func->points.size(); i++) {
                            const glm::vec2& point = mouseDot.func->points[i];
                            if (glm::distance(mousePos, point) < glm::distance(mousePos, closestPoint)) {
                                closestPoint = point;
                                closestAt = i;
                            }
                        }
                        //mouseDot.screenPos = closestPoint;
                        if(closestAt == 0 || closestAt == mouseDot.func->points.size() - 1) {
                            mouseDot.screenPos = closestPoint - scrOffsetY;
                        } else{
                            mouseDot.screenPos = getFinalOnScreenPoint(closestPoint, 
								mouseDot.func->points[closestAt], mouseDot.func->points[closestAt-1], mouseDot.func->points[closestAt+1]) - scrOffsetY;
                        }
                    }else {
                        mouseDot.screenPos = mouseDot.func->calcPointScrPos(mousePos);
                    }
                }else {
                    if (lastLeftPressed && !mouseDot.funcCaptured) { continue; }
                    std::lock_guard lg(m);
                    for(const auto& func: functions) {
                        if(func->show) {
                            std::size_t closestAt = 0;
                            //for(const auto& point: func->function.points) {
                            for(std::size_t i = 0; i < func->function.points.size(); i++) {
                                const glm::vec2& point = func->function.points[i];
                                if(glm::distance(mousePos, point) < glm::distance(mousePos, closestPoint)) {
                                    closestPoint = point;
                                    mouseDot.func = &func->function;
                                    closestAt = i;
                                }
                            }
                            if(glm::distance(mousePos, closestPoint)<0.025f) { //capture function
                                //mouseDot.screenPos = closestPoint;
                                if(closestAt == 0 || closestAt == mouseDot.func->points.size() - 1) {
                                    mouseDot.screenPos = closestPoint - scrOffsetY;
                                } else{
                                    mouseDot.screenPos = getFinalOnScreenPoint(closestPoint, 
										mouseDot.func->points[closestAt], mouseDot.func->points[closestAt-1], mouseDot.func->points[closestAt+1]) - scrOffsetY;
                                }
                                mouseDot.funcCaptured = true;
                                break;
                            }
                        }
                    }
                    lastLeftPressed = true;
                }
            }else {
                lastLeftPressed = false;
                mouseDot.funcCaptured = false;
            }
        }
    });
}

int App::initGL() {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);//opengl versions
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//set to core profile

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    appWindow = glfwCreateWindow(screenSize.x, screenSize.y, "Graphical Calculator", NULL, NULL);
    if (appWindow == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(appWindow);
    glfwSetFramebufferSizeCallback(appWindow, App::Callbacks::framebufferSizeCallback);
    glfwSetCursorPosCallback(appWindow, App::Callbacks::mouseCursorCallback);
    glfwSetMouseButtonCallback(appWindow, App::Callbacks::mouseButtonCallback);
    glfwSetScrollCallback(appWindow, App::Callbacks::mouseScrollCallback);
    glfwSwapInterval(0);//VSync
    glfwSetInputMode(appWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);//capture mouse

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //glEnable(GL_DEPTH_TEST); //enable z-buffer
    //glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable(GL_LINE_SMOOTH);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    return 0;
}

void App::initShaders() {
    shaders.funcShader = Shader("resources/shaders/function_vs.glsl", "resources/shaders/function_gs.glsl", "resources/shaders/function_fs.glsl");
    shaders.coordAxisShader = Shader("resources/shaders/coordAxisShader_vs.glsl", "resources/shaders/coordAxisShader_gs.glsl", "resources/shaders/coordAxisShader_fs.glsl");
    shaders.coordAxisNumbersShader = Shader("resources/shaders/coordAxisNumbersShader_vs.glsl", "resources/shaders/coordAxisNumbersShader_fs.glsl");
    shaders.textShader = Shader("resources/shaders/textShader_vs.glsl", "resources/shaders/textShader_fs.glsl");
    shaders.mouseDotShader = Shader("resources/shaders/mouseDot_vs.glsl","resources/shaders/mouseDot_fs.glsl");

    //setup projection and shader data
    glData.projection = glm::ortho(0.0f, screenSize.x, 0.0f, screenSize.y);
    shaders.coordAxisNumbersShader.use();
    shaders.coordAxisNumbersShader.setMat4("projection", glData.projection);
    shaders.coordAxisNumbersShader.setVec2("center", Function::xcenter, Function::ycenter);
    shaders.coordAxisNumbersShader.setVec2("size", Function::xsize, Function::ysize);
    shaders.coordAxisNumbersShader.setVec2("updateOffset", 0.0f, 0.0f);
    shaders.textShader.use();
    shaders.textShader.setMat4("projection", glData.projection);

    //setup function shader
    shaders.funcShader.use();
    //constexpr float markerSize = 0.0f;
    //funcShader.setFloat("markerRadius", markerSize);
    shaders.funcShader.setVec2("resolution", screenSize.x, screenSize.y);
    shaders.funcShader.setFloat("ycenter", 0.0f);
    shaders.funcShader.setVec3("color", glm::vec3(1.0f)); //desmos color: glm::vec3(199.0f, 68.0f, 64.0f)/255.0f
    //glUniform2fv(glGetUniformLocation(funcShader.id, "vPoints"), static_cast<GLsizei>(function.points.size()), reinterpret_cast<float*>(function.points.data()));

    //setup coordinate axis shader
    shaders.coordAxisShader.use();
    shaders.coordAxisShader.setVec2("center", 0.0f, 0.0f);
    shaders.coordAxisShader.setVec2("size", Function::xsize, Function::ysize);
    //coordAxisShader.setFloat("gridSize", 10.0f);
    glm::vec3 coordAxisCenterColor = glm::vec3(0.5f);
    glm::vec3 coordAxisGridColor = glm::vec3(0.2f);
    shaders.coordAxisShader.setVec3("centerColor", coordAxisCenterColor);
    shaders.coordAxisShader.setVec3("gridColor", coordAxisGridColor);
}

int App::init() {
    srand(static_cast<unsigned int>(time(NULL)));
    Time::Init();

    int error;
    error = initGL();
    if(error != 0) {
        return error;
    }
    error = TextRender::Init();
    if(error != 0) {
        return error;
    }

    IMGUI_CHECKVERSION(); //init imgui
    ImGui::CreateContext();
    imGuiIO = &ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui_ImplGlfw_InitForOpenGL(appWindow, true);

    initShaders();

    //generate some buffers for GPU
    VAO::generate(glData.coordAxisVAO);
    VAO::bind(glData.coordAxisVAO);
    VAO::addAttrib(glData.coordAxisVAO, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    VBO::generate(glData.mouseDotVBO, sizeof(glm::vec2), NULL, GL_STATIC_DRAW);
    VAO::generate(glData.mouseDotVAO);
    VAO::bind(glData.mouseDotVAO);
    VAO::addAttrib(glData.mouseDotVAO, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    //add first function
    pushNewFunction();

    createMouseDot();

    return 0;
}

App::App() {
    instance = this;
}

App::~App() {
	ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    mouseDot.thread->join();

    glfwTerminate();
}

int App::Run() {
    int error;
    error = init();
    if(error != 0) {
        return error;
    }

    while (!glfwWindowShouldClose(appWindow)) {
        processInput(appWindow);

        Time::Update();
        //showFPS(); //print fps

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        createImGuiMenu();

        if (imGuiIO->WantCaptureMouse || mouseDot.funcCaptured) {
            updateFunctions = false;
        } else {
            updateFunctions = true;
        }

        if(viewPointUpdated) {
            shaders.funcShader.use();
            shaders.funcShader.setVec2("resolution", screenSize.x, screenSize.y);
            glData.projection = glm::ortho(0.0f, screenSize.x, 0.0f, screenSize.y);
            shaders.coordAxisNumbersShader.use();
            shaders.coordAxisNumbersShader.setMat4("projection", glData.projection);
            shaders.textShader.use();
            shaders.textShader.setMat4("projection", glData.projection);
            viewPointUpdated = false;
        }

        if (needUpdateShaders) { //update shaders
            shaders.coordAxisShader.use();
            shaders.coordAxisShader.setVec2("center", Function::xcenter, Function::ycenter);
            shaders.coordAxisShader.setVec2("size", Function::xsize, Function::ysize);
            //coordAxisNumbersShader.use();
            //coordAxisNumbersShader.setVec2("center", Function::xcenter, Function::ycenter);
            //coordAxisNumbersShader.setVec2("size", Function::xsize, Function::ysize);
            shaders.funcShader.use();
            shaders.funcShader.setFloat("ycenter", Function::ycenter);
            needUpdateShaders = false;
        }

        glLineWidth(1);//draw coord axes
        VAO::bind(glData.coordAxisVAO);
        shaders.coordAxisShader.use();
        glDrawArrays(GL_POINTS, 0, 1);

        VAO::bind(glData.coordAxisVAO);
        shaders.coordAxisShader.use();

        renderAxisNumbers(shaders.coordAxisNumbersShader, Function::getCenter(), Function::getSize(), 0.25f, glm::vec3(0.7f)); //render numbers on number axis

        drawFunctions();

        drawMouseDot();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        TextRender::RenderText(shaders.textShader, "FPS: "+getFPS_str(2), screenSize.x*0.03f, screenSize.y*0.97f, 0.25f, glm::vec3(0.4f)); //render fps

        glfwSwapBuffers(appWindow);
        glfwPollEvents();
    }
    appOn = false;

    return 0;
}


void App::showFPS() {
    std::cout << getFPS_str(5) << std::endl;
}

std::string App::getFPS_str(const int precision) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << 1/Time::deltaTime;
    return ss.str();
}


void App::processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void App::renderAxisNumbers(const Shader& shader, const glm::vec2 center, const glm::vec2 size, const float scale, const glm::vec3 color) {
    const glm::vec2 interval = size/10.0f;

    std::stringstream ss;

    for (int i = -11; i<11; i++) {
        ss.str("");
        ss << RenderAxisNumbersPrecision::xformatting << std::setprecision(RenderAxisNumbersPrecision::xprecision) << interval.x*(static_cast<float>(i)-center.x*10.0f);
        TextRender::RenderText(shader, ss.str(), static_cast<float>(i+10)/10.0f*screenSize.x/2.0f, (1.0f-(center.y+1.0f)/2.0f)*screenSize.y, scale, color);
        ss.str("");
        ss << RenderAxisNumbersPrecision::yformatting << std::setprecision(RenderAxisNumbersPrecision::yprecision) << interval.y*(static_cast<float>(i)+center.y*10.0f);
        TextRender::RenderText(shader, ss.str(), ((center.x+1.0f)/2.0f)*screenSize.x, static_cast<float>(i+10)/10.0f*screenSize.y/2.0f, scale, color);
    }
}

void Application::RenderAxisNumbersPrecision::UpdatePrecision() {
    xprecision = Function::numbersFloatPrecision;
    yprecision = Function::numbersFloatPrecision;
    xformatting = std::fixed;
    yformatting = std::fixed;
    const glm::vec2 f_size = Function::getSize();
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
