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

#include "../include/Function.h"
#include "../include/FunctionSystem.h"

#include "../myGL/Shader.h"
#include "../myGL/VAO.h"
#include "../myGL/VBO.h"
//#include "../myGL/EBO.h" //unused
#include "../myGL/Time.h"
#include "../myGL/Texture2D.h"

#include <ft2build.h>
#include <stack>

#include FT_FREETYPE_H

//callbacks and some other functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_cursor_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void showFPS();
std::string getFPS_str(int precision);
void RenderText(const Shader& shader, std::string text, float x, float y, float scale, glm::vec3 color);
void RenderAxisNumbers(const Shader& shader, glm::vec2 center, glm::vec2 size, float scale, glm::vec3 color);
std::function<void()> ViewpointUpdateShaderCallback;

float SCR_WIDTH = 900;
float SCR_HEIGHT = 900;

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
Mouse mouse(SCR_WIDTH/2.0f, SCR_HEIGHT/2.0f);


struct Character {
    GLuint TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};
std::map<GLchar, Character> Characters;
GLuint characterVAO, characterVBO;

FunctionSystem MainFunctionSystem = FunctionSystem({SCR_WIDTH,SCR_HEIGHT});

std::vector<FuncData*> functions;

bool updateFunc = true;

bool needUpdateShaders = false;

glm::mat4 projection;

std::atomic_bool appOn = true;
std::mutex m;

namespace RenderAxisNumbersPrecision { //for calculation of precision for number on number axis
    int xprecision = MainFunctionSystem.numbersFloatPrecision;
    int yprecision = MainFunctionSystem.numbersFloatPrecision;
    auto xformatting = std::fixed;
    auto yformatting = std::fixed;

    void updatePrecision() {
        xprecision = MainFunctionSystem.numbersFloatPrecision;
        yprecision = MainFunctionSystem.numbersFloatPrecision;
        xformatting = std::fixed;
        yformatting = std::fixed;
        const glm::vec2 f_size = MainFunctionSystem.size;
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

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Graphical Calculator", NULL, NULL);
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
    //glEnable(GL_LINE_SMOOTH);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    

    srand(static_cast<unsigned int>(time(NULL)));

    { //FreeType (thx https://learnopengl.com, copied from it)
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

    IMGUI_CHECKVERSION(); //init imgui
    ImGui::CreateContext();
    const ImGuiIO& imGuiIO = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    Time::Init();

    Function::mainFunctionSystem = &MainFunctionSystem;

    Shader funcShader("resources/shaders/function_vs.glsl", "resources/shaders/function_gs.glsl", "resources/shaders/function_fs.glsl");
    Shader coordAxisShader("resources/shaders/coordAxisShader_vs.glsl", "resources/shaders/coordAxisShader_gs.glsl", "resources/shaders/coordAxisShader_fs.glsl");
    Shader coordAxisNumbersShader("resources/shaders/coordAxisNumbersShader_vs.glsl", "resources/shaders/coordAxisNumbersShader_fs.glsl");
    Shader textShader("resources/shaders/textShader_vs.glsl", "resources/shaders/textShader_fs.glsl");
    Shader mouseDotShader("resources/shaders/mouseDot_vs.glsl","resources/shaders/mouseDot_fs.glsl");

    projection = glm::ortho(0.0f, SCR_WIDTH, 0.0f, SCR_HEIGHT); //setup projection and shader data
    coordAxisNumbersShader.use();
    coordAxisNumbersShader.setMat4("projection", projection);
    coordAxisNumbersShader.setVec2("center", MainFunctionSystem.center);
    coordAxisNumbersShader.setVec2("size", MainFunctionSystem.size);
    coordAxisNumbersShader.setVec2("updateOffset", 0.0f, 0.0f);
    textShader.use();
    textShader.setMat4("projection", projection);

    GLuint coordAxisVAO;
    VAO::generate(coordAxisVAO);

    GLuint mouseDotVBO;
    VBO::generate(mouseDotVBO, sizeof(glm::vec2), NULL, GL_STATIC_DRAW);
    GLuint mouseDotVAO;
    VAO::generate(mouseDotVAO);
    VAO::bind(mouseDotVAO);
    VAO::addAttrib(mouseDotVAO, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    funcShader.use();
    //constexpr float markerSize = 0.0f;
    //funcShader.setFloat("markerRadius", markerSize);
    funcShader.setVec2("resolution", SCR_WIDTH, SCR_HEIGHT);
    funcShader.setVec2("center", 0.0f, 0.0f);
    funcShader.setVec3("color", glm::vec3(1.0f)); //desmos color: glm::vec3(199.0f, 68.0f, 64.0f)/255.0f
    funcShader.setMat4("projection", projection);
    //glUniform2fv(glGetUniformLocation(funcShader.id, "vPoints"), static_cast<GLsizei>(function.points.size()), reinterpret_cast<float*>(function.points.data()));

    coordAxisShader.use();
    coordAxisShader.setVec2("center", 0.0f, 0.0f);
    coordAxisShader.setVec2("size", MainFunctionSystem.size);
    //coordAxisShader.setFloat("gridSize", 10.0f);

    glm::vec3 coordAxisCenterColor = glm::vec3(0.5f);
    glm::vec3 coordAxisGridColor = glm::vec3(0.2f);
    coordAxisShader.setVec3("centerColor", coordAxisCenterColor);
    coordAxisShader.setVec3("gridColor", coordAxisGridColor);

    mouseDotShader.use();
    mouseDotShader.setMat4("projection", projection);

    ViewpointUpdateShaderCallback = [&]() { //set up callback
        projection = glm::ortho(0.0f, SCR_WIDTH, 0.0f, SCR_HEIGHT);
        //const glm::vec2 halfScreen = glm::vec2(SCR_WIDTH*0.5f, SCR_HEIGHT*0.5f);
        //const glm::vec2 deltaMouse = mouse.pos - halfScreen;
        //const glm::vec2 translation = halfScreen;
        //zoomLevel = 1.0f;
        //scrDimensions.left = -translation.x * zoomLevel + translation.x;
        //scrDimensions.right = translation.x * zoomLevel + translation.x;
        //scrDimensions.bottom = -translation.y * zoomLevel + translation.y;
        //scrDimensions.top = translation.y * zoomLevel + translation.y;
        //glm::mat4 projection = glm::ortho(scrDimensions.left, scrDimensions.right, scrDimensions.bottom, scrDimensions.top);

    	funcShader.use();
        funcShader.setVec2("resolution", SCR_WIDTH, SCR_HEIGHT);
        funcShader.setMat4("projection", projection);
        coordAxisNumbersShader.use();
        coordAxisNumbersShader.setMat4("projection", projection);
        textShader.use();
        textShader.setMat4("projection", projection);
        mouseDotShader.use();
        mouseDotShader.setMat4("projection", projection);
    };

    constexpr float buttonsInc = 0.15f;

    //add first function
    MainFunctionSystem.functions.push_back(new FuncData()); //add initial blank function
    VBO::generate(MainFunctionSystem.functions.back()->vbo, static_cast<GLsizeiptr>(MainFunctionSystem.functions.back()->function.points.size*sizeof(glm::vec2)), MainFunctionSystem.functions.back()->function.points.data, GL_DYNAMIC_DRAW);
    VAO::generate(MainFunctionSystem.functions.back()->vao);
    VAO::bind(MainFunctionSystem.functions.back()->vao);
    VAO::addAttrib(MainFunctionSystem.functions.back()->vao, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    //MainFunctionSystem.functions.back()->color = glm::vec3(1.0f);
    MainFunctionSystem.functions.back()->color = glm::vec3(static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX));
    std::string func_clear("");
    MainFunctionSystem.exprStrParser.Parse(func_clear);
    MainFunctionSystem.functions.back()->function.SetFunction(MainFunctionSystem.exprStrParser.CopyExpression());

    MouseDot mouseDot;
    mouseDotShader.use();
    mouseDotShader.setVec3("color", mouseDot.color);
    std::thread mouseDotThread([&]() { //thread that calculates poin for mouse
        bool lastLeftPressed = false;
        while (appOn) {
            if (mouse.leftPressed) {
                if (imGuiIO.WantCaptureMouse) {continue;}
                const glm::vec2 mousePos = glm::vec2(mouse.pos.x / SCR_WIDTH * 2.0f - 1.0f, -(mouse.pos.y / SCR_HEIGHT * 2.0f - 1.0f));
                glm::vec2 closestPoint = { 4,4 };
                if (mouseDot.funcCaptured) {
                    if (mouseDot.byDistance) {
                        for(std::size_t i=0; i<mouseDot.func->points.size; i++) {
                            const glm::vec2& point = mouseDot.func->points.data[i];
                            const glm::vec2 pointPos = { point.x,point.y - MainFunctionSystem.center.y };
                            if (glm::distance(mousePos, pointPos) < glm::distance(mousePos, closestPoint)) {
                                closestPoint = pointPos;
                            }
                        }
                        mouseDot.screenPos = closestPoint;
                    }else {
                        mouseDot.screenPos = mouseDot.func->CalcPointScrPos(mousePos); //TODO: FIX
                    }
                }else {
                    if (lastLeftPressed && !mouseDot.funcCaptured) { continue; }
                    std::lock_guard lg(m);
                    for(FuncData* func: MainFunctionSystem.functions) {
	                    if(func->show && func->function.points.size>0) {
                            float distanceTmp;
                            closestPoint = func->function.points.data[0];
                            float distanceToClosestPoint = glm::distance(mousePos, closestPoint);
                            for(std::size_t i=1; i<func->function.points.size; i++) {
                                const glm::vec2& point = func->function.points.data[i];
                                const glm::vec2 pointPos = { point.x,point.y - MainFunctionSystem.center.y };
                                distanceTmp = glm::distance(mousePos, pointPos);
			                    if(distanceTmp < distanceToClosestPoint) {
                                    closestPoint = pointPos;
                                    distanceToClosestPoint = distanceTmp;
			                    }
		                    }
                            if(distanceToClosestPoint < 0.025f) { //capture function
                                mouseDot.screenPos = closestPoint;
                                mouseDot.func = &func->function;
                                mouseDot.funcCaptured = true;
                                break;
                            }
	                    }
                    }
                }
                lastLeftPressed = true;
            }else {
                lastLeftPressed = false;
                mouseDot.funcCaptured = false;
            }
        }
    });


    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        Time::Update();
        //showFPS(); //print fps

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Menu");
        { //draw GUI with ImGUI
            if (ImGui::Button("Home")) {
                MainFunctionSystem.size = {10.0f, 10.0f};
                MainFunctionSystem.center = {0.0f, 0.0f};
                needUpdateShaders = true;
                MainFunctionSystem.functionsNeedUpdate = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }
            ImGui::SameLine();
            if (ImGui::Button("+")) {
                MainFunctionSystem.size *= glm::vec2(1.0f-buttonsInc,1.0f-buttonsInc);
                needUpdateShaders = true;
                MainFunctionSystem.functionsNeedUpdate = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }
            ImGui::SameLine();
            if (ImGui::Button("-")) {
                MainFunctionSystem.size *= glm::vec2(1.0f+buttonsInc,1.0f+buttonsInc);
                needUpdateShaders = true;
                MainFunctionSystem.functionsNeedUpdate = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }

            ImGui::SameLine();
            ImGui::PushItemWidth(65);
            const double precisionDragInc = std::clamp(abs(MainFunctionSystem.funcPrecision*0.01), FunctionSystem::funcPrecisionMin*0.1, 2.0);
            if (ImGui::DragScalar("Precision", ImGuiDataType_Double, &MainFunctionSystem.funcPrecision, precisionDragInc, &FunctionSystem::funcPrecisionMin)) {
                needUpdateShaders = true;
                MainFunctionSystem.functionsNeedUpdate = true;
            }
            ImGui::PopItemWidth();

            if (ImGui::Button("X +")) {
                MainFunctionSystem.size.x *= 1.0f-buttonsInc;
                needUpdateShaders = true;
                MainFunctionSystem.functionsNeedUpdate = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }
            ImGui::SameLine();
            if (ImGui::Button("X -")) {
                MainFunctionSystem.size.x *= 1.0f+buttonsInc;
                needUpdateShaders = true;
                MainFunctionSystem.functionsNeedUpdate = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }
            ImGui::SameLine();
            if (ImGui::Button("Y +")) {
                MainFunctionSystem.size.y *= 1.0f-buttonsInc;
                needUpdateShaders = true;
                MainFunctionSystem.functionsNeedUpdate = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }
            ImGui::SameLine();
            if (ImGui::Button("Y -")) {
                MainFunctionSystem.size.y *= 1.0f+buttonsInc;
                needUpdateShaders = true;
                MainFunctionSystem.functionsNeedUpdate = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }
            ImGui::Checkbox("Mouse Dot by distance", &mouseDot.byDistance);
            ImGui::SameLine();
            if(ImGui::ColorEdit3("Mouse Dot color", value_ptr(mouseDot.color), ImGuiColorEditFlags_NoInputs)) {
                mouseDotShader.use();
                mouseDotShader.setVec3("color", mouseDot.color);
            }
            if (ImGui::Button("Add new function")) {
                std::lock_guard lg(m);
                MainFunctionSystem.functions.push_back(new FuncData());
                FuncData* currFunction = MainFunctionSystem.functions.back();
                VBO::generate(currFunction->vbo, static_cast<GLsizeiptr>(currFunction->function.points.size*sizeof(glm::vec2)), MainFunctionSystem.functions.back()->function.points.data, GL_DYNAMIC_DRAW);
                VAO::generate(currFunction->vao);
                VAO::bind(currFunction->vao);
                VAO::addAttrib(currFunction->vao, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
                currFunction->color = glm::vec3(static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX));
                MainFunctionSystem.exprStrParser.Parse(func_clear);
                currFunction->function.SetFunction(MainFunctionSystem.exprStrParser.CopyExpression());
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove last function")) {
                if (!MainFunctionSystem.functions.empty()) {
                    std::lock_guard lg(m);
                    FuncData* currFunction = MainFunctionSystem.functions.back();
                    VBO::deleteIt(currFunction->vbo);
                    VAO::deleteIt(currFunction->vao);
                    currFunction->function.FreePointsBuf(currFunction->function.points);
                    MainFunctionSystem.functions.pop_back();
                }
            }
            for (std::size_t i = 0; i<MainFunctionSystem.functions.size(); i++) {
                ImGui::PushID((int)i);
                FuncData* currFunction = MainFunctionSystem.functions.at(i);
                if (ImGui::Button("-")) {
                    VBO::deleteIt(currFunction->vbo);
                    VAO::deleteIt(currFunction->vao);
                    currFunction->function.FreePointsBuf(currFunction->function.points);
                    MainFunctionSystem.functions.erase(MainFunctionSystem.functions.begin()+i);
                    ImGui::PopID();
                    continue;
                }
                ImGui::SameLine();
                if (currFunction->show) {
                    if (ImGui::Button("hide")) {
                        currFunction->show = false;
                    }
                } else {
                    if (ImGui::Button("show")) {
                        currFunction->show = true;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Variables")) {
                    if (currFunction->function.functionExpression.GetArgs().size()>1) { //because the size is at least one (there is always x)
                        ImGui::OpenPopup("Variables");
                    }
                }
                if (ImGui::BeginPopup("Variables")) {
                    for (auto& [argName, argValue]: currFunction->function.functionExpression.GetArgs()) {
                        if(argName == "x"){ continue; }
                        ImGui::PushID((int)i+1);
                        if (ImGui::InputDouble(argName.c_str(), &argValue)) {
                            currFunction->function.functionExpression.SetArgs(argName,argValue);
                            currFunction->function.needsPersonalUpdate = true;
                        }
                        ImGui::PopID();
                        ImGui::SameLine();
                        const double inc = std::clamp(abs(argValue*0.1), 0.001, 1000.0);
                        if (ImGui::DragScalar(argName.c_str(), ImGuiDataType_Double, &argValue, inc)) {
                            currFunction->function.functionExpression.SetArgs(argName,argValue);
                            currFunction->function.needsPersonalUpdate = true;
                        }
                    }
                    ImGui::EndPopup();
                }
                ImGui::SameLine();
                ImGui::ColorEdit3("", value_ptr(currFunction->color), ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine();
                if (ImGui::InputText("", &currFunction->inputData)) {
                    MainFunctionSystem.exprStrParser.Parse(currFunction->inputData);
                    currFunction->function.SetFunction(MainFunctionSystem.exprStrParser.CopyExpression());
                    currFunction->futures.push_back({ std::async(std::launch::async, [=] {return currFunction->function.CalculatePoints(); }) });
                }
                ImGui::PopID();
            }
        }
        ImGui::End();

        if (imGuiIO.WantCaptureMouse || mouseDot.funcCaptured) {
            updateFunc = false;
        } else {
            updateFunc = true;
        }

        if (needUpdateShaders) { //update shaders
            coordAxisShader.use();
            coordAxisShader.setVec2("center", MainFunctionSystem.center);
            coordAxisShader.setVec2("size", MainFunctionSystem.size);
        	//coordAxisNumbersShader.use();
            //coordAxisNumbersShader.setVec2("center", Function::xcenter, Function::ycenter);
            //coordAxisNumbersShader.setVec2("size", Function::xsize, Function::ysize);
            funcShader.use();
            funcShader.setVec2("center", MainFunctionSystem.center);
            needUpdateShaders = false;
        }

        glLineWidth(1);//draw coord axes
        VAO::bind(coordAxisVAO);
        coordAxisShader.use();
        glDrawArrays(GL_POINTS, 0, 1);

        RenderAxisNumbers(coordAxisNumbersShader, MainFunctionSystem.center, MainFunctionSystem.size, 0.25f, glm::vec3(0.7f)); //render numbers on number axis

        glLineWidth(3);
        funcShader.use();
        for (FuncData* currFunction: MainFunctionSystem.functions) { //check if function recalculated and draw it
            if (!currFunction->show) { continue; }
            if (MainFunctionSystem.functionsNeedUpdate || currFunction->function.needsPersonalUpdate) {
                currFunction->futures.push_back({ std::async(std::launch::async, [=] {return currFunction->function.CalculatePoints(); }) });
                currFunction->function.needsPersonalUpdate = false;
            }

            for(auto rit = currFunction->futures.rbegin(); rit!=currFunction->futures.rend(); ++rit) { //to dispose of buffers from unnecessary futures
                if(rit->_Is_ready()) {
                    const PointsBuf points = rit->get();
                    currFunction->function.SetPoints(points);
                    currFunction->function.FreePointsBuf(points);

                	VBO::bind(currFunction->vbo);
                    VBO::setData(currFunction->vbo, currFunction->function.points.size*sizeof(glm::vec2), currFunction->function.points.data, GL_DYNAMIC_DRAW);

                    const auto end = --rit.base();
                    for(auto it = currFunction->futures.begin(); it != end; ){
                        if(it->_Is_ready()) {
							currFunction->function.FreePointsBuf(it->get());
							it = currFunction->futures.erase(it);
                        }else {
                            ++it;
                        }
                    }
                    currFunction->futures.erase(end);

                	break;
                }
            }

            VAO::bind(currFunction->vao);
            funcShader.setVec3("color", currFunction->color);
            glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, static_cast<GLsizei>(currFunction->function.points.size));
        }
        MainFunctionSystem.functionsNeedUpdate = false;

        if (mouseDot.funcCaptured) { //draw mouse dot and value at it
            VAO::bind(mouseDotVAO);
            VBO::setData(mouseDotVBO, sizeof(glm::vec2), glm::value_ptr(mouseDot.screenPos), GL_DYNAMIC_DRAW);
            glPointSize(7);
            mouseDotShader.use();
            glDrawArrays(GL_POINTS, 0, 1);
            //std::stringstream ss;
            //ss << RenderAxisNumbersPrecision::xformatting << std::setprecision(RenderAxisNumbersPrecision::xprecision) << Function::xsize*(mouseDot.screenPos.x-Function::xcenter) << " ";
            //ss << RenderAxisNumbersPrecision::yformatting << std::setprecision(RenderAxisNumbersPrecision::yprecision) << Function::ysize*(mouseDot.func->expr_str_parser.calculate(mouseDot.screenPos.y+Function::ycenter));
            //std::lock_guard lg(mouseDot.func->m);
            //const std::string dotText = std::to_string(Function::xsize*(mouseDot.screenPos.x-Function::xcenter)) + " " + std::to_string(Function::ysize*(mouseDot.func->expr_str_parser.calculate(mouseDot.screenPos.y+Function::ycenter)));
        	const std::string dotText = std::to_string(MainFunctionSystem.size.x*(mouseDot.screenPos.x-MainFunctionSystem.center.x)) + " " + std::to_string(mouseDot.func->CalcAtScrPos(mouseDot.screenPos));
        	RenderText(textShader, dotText, SCR_WIDTH*(mouseDot.screenPos.x+1.0f)*0.5f+10.0f, SCR_HEIGHT*(mouseDot.screenPos.y+1.0f)*0.5f, 0.3f, glm::vec3(0.7f));
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        RenderText(textShader, "FPS: "+getFPS_str(2), SCR_WIDTH*0.03f, SCR_HEIGHT*0.97f, 0.25f, glm::vec3(0.4f)); //render fps

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    appOn = false;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    mouseDotThread.join();

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

    if (mouse.firstInput) {
        mouse.pos.x = xposIn;
        mouse.pos.y = yposIn;
        mouse.firstInput = false;
    }

    if (updateFunc) {
        if (mouse.leftPressed) {
            const float deltaX = xposIn-mouse.pos.x;
            const float deltaY = yposIn-mouse.pos.y;
            MainFunctionSystem.center += glm::vec2(deltaX/(SCR_WIDTH/2.0f), deltaY/(SCR_HEIGHT/2.0f));
            needUpdateShaders = true;
            MainFunctionSystem.functionsNeedUpdate = true;
        }
    }

    mouse.pos.x = xposIn;
    mouse.pos.y = yposIn;
}

void mouse_button_callback(GLFWwindow* window, const int button, const int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse.leftPressed = true;
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouse.leftPressed = false;
        needUpdateShaders = true;
        MainFunctionSystem.functionsNeedUpdate = true;
    }
}

void mouse_scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset) {
    mouse.wheelScrolled = true;
    if (updateFunc) {//scale MainFunctionSystem.functions
        const glm::vec2 mousePos = glm::vec2(mouse.pos.x/SCR_WIDTH*2.0f-1.0f, mouse.pos.y/SCR_HEIGHT*2.0f-1.0f);
        const glm::vec2 lastValue = (-MainFunctionSystem.center+mousePos)*MainFunctionSystem.size;
	    //const glm::vec2 lastCenterValue = (Function::getCenter()+glm::vec2(mouse.pos.x/SCR_WIDTH*2-1, mouse.pos.y/SCR_HEIGHT*2-1))*Function::getSize();
        //Function::incCenter(-(mouse.pos.x/SCR_WIDTH -Function::getCenterNDC().x)*(static_cast<float>(yoffset)/10.0f),
		//					-(mouse.pos.y/SCR_HEIGHT-Function::getCenterNDC().y)*(static_cast<float>(yoffset)/10.0f));
        MainFunctionSystem.size *= glm::vec2((1.0f-static_cast<float>(yoffset)/20.0f), (1.0f-static_cast<float>(yoffset)/20.0f));
        MainFunctionSystem.center = -lastValue/MainFunctionSystem.size+mousePos;
        needUpdateShaders = true;
        MainFunctionSystem.functionsNeedUpdate = true;
        RenderAxisNumbersPrecision::updatePrecision();
    }
}

void RenderAxisNumbers(const Shader& shader, const glm::vec2 center, const glm::vec2 size, const float scale, const glm::vec3 color) {
    const glm::vec2 interval = size/10.0f;

    //const float xside_indent = SCR_WIDTH/80.0f;
    //const float yside_indent = SCR_HEIGHT/80.0f;

    std::stringstream ss;

    //shader.use();
    for (int i = -11; i<11; i++) {
        //shader.setInt("index", i);
        ss.str("");
        //shader.setVec2("updateOffset", 1.0f, 0.0f);
        ss << RenderAxisNumbersPrecision::xformatting << std::setprecision(RenderAxisNumbersPrecision::xprecision) << interval.x*(static_cast<float>(i)-center.x*10.0f);
        //RenderText(shader, ss.str(), static_cast<float>(i+10)/10.0f*SCR_WIDTH/2.0f, (1.0f-(center.y+1.0f)/2.0f)*SCR_HEIGHT, scale, color);
        RenderText(shader, ss.str(), static_cast<float>(i+10)*0.1f*SCR_WIDTH*0.5f, (1.0f-(center.y+1.0f)*0.5f)*SCR_HEIGHT, scale, color);
        ss.str("");
        //shader.setVec2("updateOffset", 0.0f, -1.0f);
        ss << RenderAxisNumbersPrecision::yformatting << std::setprecision(RenderAxisNumbersPrecision::yprecision) << interval.y*(static_cast<float>(i)+center.y*10.0f);
        //RenderText(shader, ss.str(), ((center.x+1.0f)/2.0f)*SCR_WIDTH, static_cast<float>(i+10)/10.0f*SCR_HEIGHT/2.0f, scale, color);
        RenderText(shader, ss.str(), ((center.x+1.0f)*0.5f)*SCR_WIDTH, static_cast<float>(i+10)*0.1f*SCR_HEIGHT*0.5f, scale, color);
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
    SCR_WIDTH = static_cast<float>(width);
    SCR_HEIGHT = static_cast<float>(height);
    glViewport(0, 0, width, height); //0,0 - left bottom
    ViewpointUpdateShaderCallback();
}
