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

#include "../myGL/Shader.h"
#include "../myGL/VAO.h"
#include "../myGL/VBO.h"
#include "../myGL/EBO.h"
#include "../myGL/Time.h"
#include "../myGL/Texture2D.h"

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
std::function<void()> ViewpointUpdateShaderCallback;

float SCR_WIDTH = 800;
float SCR_HEIGHT = 600;

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

//mouse last positions
//float lastX = SCR_WIDTH / 2.0f;
//float lastY = SCR_HEIGHT / 2.0f;
//bool firstMouse = true;
//bool wheel_scrolled = false;
//bool leftPressed = false;

//assign values for update thresholds
/* currently unused
float last_updated_mouse_x = mouse.pos.x;
float last_updated_mouse_y = mouse.pos.y;
constexpr float mouse_move_update_threshold = 10.0f; //px

float last_updated_size_x = 20.0f;
float last_updated_size_y = 20.0f;
constexpr float size_update_threshold = 0.1f; // %/100
*/

//constexpr float defaultMarkerSize = 0.002f;
//float markerSize = defaultMarkerSize;


struct Character {
    GLuint TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};
std::map<GLchar, Character> Characters;
GLuint characterVAO, characterVBO;

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

bool update_func = true;

bool need_update_shaders = false;

//bool viewpoint_updated = false;

std::atomic_bool appOn = true;
std::mutex m;

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

    //Shader funcShader("resources/shaders/shader_vs.glsl", "resources/shaders/shader_gs.glsl", "resources/shaders/shader_fs.glsl");
    Shader funcShader("resources/shaders/shader2_vs.glsl", "resources/shaders/shader2_gs.glsl", "resources/shaders/shader2_fs.glsl");
    Shader coordAxisShader("resources/shaders/coordAxisShader_vs.glsl", "resources/shaders/coordAxisShader_gs.glsl", "resources/shaders/coordAxisShader_fs.glsl");
    Shader coordAxisNumbersShader("resources/shaders/coordAxisNumbersShader_vs.glsl", "resources/shaders/coordAxisNumbersShader_fs.glsl");
    Shader textShader("resources/shaders/textShader_vs.glsl", "resources/shaders/textShader_fs.glsl");
    Shader mouseDotShader("resources/shaders/mouseDot_vs.glsl","resources/shaders/mouseDot_fs.glsl");

    glm::mat4 projection = glm::ortho(0.0f, SCR_WIDTH, 0.0f, SCR_HEIGHT);
    coordAxisNumbersShader.use();
    coordAxisNumbersShader.setMat4("projection", projection);
    coordAxisNumbersShader.setVec2("center", Function::xcenter, Function::ycenter);
    coordAxisNumbersShader.setVec2("size", Function::xsize, Function::ysize);
    coordAxisNumbersShader.setVec2("updateOffset", 0.0f, 0.0f);
    textShader.use();
    textShader.setMat4("projection", projection);


    //GLuint vbo;
    //VBO::generate(vbo, static_cast<GLsizeiptr>(Function::calc_points_count*sizeof(glm::vec2)), NULL, GL_DYNAMIC_DRAW);
    //VBO::bind(vbo);
    GLuint vao;
    VAO::generate(vao);
    VAO::bind(vao);
    VAO::addAttrib(vao, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    //GLuint ebo;
    //EBO::generate(ebo, sizeof(indices), indices, GL_STATIC_DRAW);

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
    funcShader.setFloat("ycenter", 0.0f);
    funcShader.setVec3("color", glm::vec3(1.0f)); //desmos color: glm::vec3(199.0f, 68.0f, 64.0f)/255.0f
    //glUniform2fv(glGetUniformLocation(funcShader.id, "vPoints"), static_cast<GLsizei>(function.points.size()), reinterpret_cast<float*>(function.points.data()));

    coordAxisShader.use();
    coordAxisShader.setVec2("center", 0.0f, 0.0f);
    coordAxisShader.setVec2("size", Function::xsize, Function::ysize);
    //coordAxisShader.setFloat("gridSize", 10.0f);

    glm::vec3 coordAxisCenterColor = glm::vec3(0.5f);
    glm::vec3 coordAxisGridColor = glm::vec3(0.2f);
    coordAxisShader.setVec3("centerColor", coordAxisCenterColor);
    coordAxisShader.setVec3("gridColor", coordAxisGridColor);

    ViewpointUpdateShaderCallback = [&]() {
        funcShader.use();
        funcShader.setVec2("resolution", SCR_WIDTH, SCR_HEIGHT);
        projection = glm::ortho(0.0f, SCR_WIDTH, 0.0f, SCR_HEIGHT);
        coordAxisNumbersShader.use();
        coordAxisNumbersShader.setMat4("projection", projection);
        textShader.use();
        textShader.setMat4("projection", projection);
    };

    constexpr float buttonsInc = 0.15f;

    //add first function
    functions.push_back(new FuncData());
    VBO::generate(functions.back()->vbo, static_cast<GLsizeiptr>(Function::calc_points_count*sizeof(glm::vec2)), functions.back()->function.points.data(), GL_DYNAMIC_DRAW);
    VAO::generate(functions.back()->vao);
    VAO::bind(functions.back()->vao);
    VAO::addAttrib(functions.back()->vao, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    //functions.back()->color = glm::vec3(1.0f);
    functions.back()->color = glm::vec3(static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX));
    std::string func_clear("");
    functions.back()->function.setFunction(func_clear);

    MouseDot mouseDot;
    mouseDotShader.use();
    mouseDotShader.setVec3("color", mouseDot.color);
    std::thread mouseDotThread([&]() {
        bool lastLeftPressed = false;
        while (appOn) {
            if (mouse.leftPressed) {
                if (imGuiIO.WantCaptureMouse) {continue;}
                const glm::vec2 mousePos = glm::vec2(mouse.pos.x / SCR_WIDTH * 2.0f - 1.0f, -(mouse.pos.y / SCR_HEIGHT * 2.0f - 1.0f));
                glm::vec2 closestPoint = { 4,4 };
                if (mouseDot.funcCaptured) {
                    if (mouseDot.byDistance) {
                        for (const glm::vec2 point : mouseDot.func->points) {
                            const glm::vec2 pointPos = { point.x,point.y - Function::ycenter };
                            if (glm::distance(mousePos, pointPos) < glm::distance(mousePos, closestPoint)) {
                                closestPoint = pointPos;
                            }
                        }
                        mouseDot.screenPos = closestPoint;
                    }else {
                        mouseDot.screenPos = mouseDot.func->calcPointScrPos(mousePos);
                    }
                }else {
                    if (lastLeftPressed && !mouseDot.funcCaptured) { continue; }
                    std::lock_guard lg(m);
                    for(const auto& func: functions) {
	                    if(func->show) {
		                    for(const auto& point: func->function.points) {
                                const glm::vec2 pointPos = { point.x,point.y - Function::ycenter };
			                    if(glm::distance(mousePos, pointPos) < glm::distance(mousePos, closestPoint)) {
                                    closestPoint = pointPos;
                                    mouseDot.func = &func->function;
			                    }
		                    }
                            if(glm::distance(mousePos, closestPoint)<0.025f) {
                                mouseDot.screenPos = closestPoint;
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
        {
            if (ImGui::Button("Home")) {
                Function::setSize(10.0f, 10.0f);
                Function::setCenter(0.0f, 0.0f);
                need_update_shaders = true;
                Function::needs_update = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }
            ImGui::SameLine();
            if (ImGui::Button("+")) {
                Function::multSize(1.0f-buttonsInc, 1.0f-buttonsInc);
                need_update_shaders = true;
                Function::needs_update = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }
            ImGui::SameLine();
            if (ImGui::Button("-")) {
                Function::multSize(1.0f+buttonsInc, 1.0f+buttonsInc);
                need_update_shaders = true;
                Function::needs_update = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }
            if (ImGui::Button("X +")) {
                Function::multSize(1.0f-buttonsInc, 1.0f);
                need_update_shaders = true;
                Function::needs_update = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }
            ImGui::SameLine();
            if (ImGui::Button("X -")) {
                Function::multSize(1.0f+buttonsInc, 1.0f);
                need_update_shaders = true;
                Function::needs_update = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }
            ImGui::SameLine();
            if (ImGui::Button("Y +")) {
                Function::multSize(1.0f, 1.0f-buttonsInc);
                need_update_shaders = true;
                Function::needs_update = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }
            ImGui::SameLine();
            if (ImGui::Button("Y -")) {
                Function::multSize(1.0f, 1.0f+buttonsInc);
                need_update_shaders = true;
                Function::needs_update = true;
                RenderAxisNumbersPrecision::updatePrecision();
            }
            //ImGui::Text("\n"); //somehow this makes indent smaller than " "
            ImGui::Checkbox("Mouse Dot by distance", &mouseDot.byDistance);
            ImGui::SameLine();
            if(ImGui::ColorEdit3("Mouse Dot color", value_ptr(mouseDot.color), ImGuiColorEditFlags_NoInputs)) {
                mouseDotShader.use();
                mouseDotShader.setVec3("color", mouseDot.color);
            }
            if (ImGui::Button("Add new function")) {
                std::lock_guard lg(m);
                functions.push_back(new FuncData());
                VBO::generate(functions.back()->vbo, static_cast<GLsizeiptr>(Function::calc_points_count*sizeof(glm::vec2)), functions.back()->function.points.data(), GL_DYNAMIC_DRAW);
                VAO::generate(functions.back()->vao);
                VAO::bind(functions.back()->vao);
                VAO::addAttrib(functions.back()->vao, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
                functions.back()->color = glm::vec3(static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX));
                functions.back()->function.setFunction(func_clear);
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove last function")) {
                if (!functions.empty()) {
                    std::lock_guard lg(m);
                    VBO::deleteIt(functions.back()->vbo);
                    VAO::deleteIt(functions.back()->vao);
                    functions.pop_back();
                }
            }
            std::string blank_indent; //this is stupid, but works
            for (std::size_t i = 0; i<functions.size(); i++) {
                blank_indent.push_back('\r');
                if (ImGui::Button((blank_indent+"-").c_str())) {
                    VBO::deleteIt(functions.at(i)->vbo);
                    VAO::deleteIt(functions.at(i)->vao);
                    functions.erase(functions.begin()+i);
                    continue;
                }
                ImGui::SameLine();
                if (functions[i]->show) {
                    if (ImGui::Button((blank_indent+"hide").c_str())) {
                        functions[i]->show = false;
                    }
                } else {
                    if (ImGui::Button((blank_indent+"show").c_str())) {
                        functions[i]->show = true;
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button((blank_indent+"Variables").c_str())) {
                    if (!functions[i]->function.expr_str_parser.get_args().empty()) {
                        ImGui::OpenPopup((blank_indent+"Variables").c_str());
                    }
                }
                if (ImGui::BeginPopup((blank_indent+"Variables").c_str())) {
                    for (auto& arg: functions[i]->function.expr_str_parser.get_args()) {
                        if (ImGui::InputFloat((blank_indent+arg.first).c_str(), &arg.second)) {
                            functions[i]->function.expr_str_parser.set_args(arg);
                            functions[i]->function.needs_personal_update = true;
                        }
                        ImGui::SameLine();
                        const float inc = abs(arg.second/10.0f);
                        if (ImGui::DragFloat(arg.first.c_str(), &arg.second, inc==0.0f?0.001f:(inc>1000.0f?1000.0f:inc))) {
                            functions[i]->function.expr_str_parser.set_args(arg);
                            functions[i]->function.needs_personal_update = true;
                        }
                    }
                    ImGui::EndPopup();
                }
                ImGui::SameLine();
                ImGui::ColorEdit3(blank_indent.c_str(), value_ptr(functions[i]->color), ImGuiColorEditFlags_NoInputs);
                ImGui::SameLine();
                if (ImGui::InputText(blank_indent.c_str(), &functions[i]->inputData)) {
                    auto last_args = functions[i]->function.expr_str_parser.get_args();
                    functions[i]->function.setFunction(functions[i]->inputData);
                    for (auto& arg: functions[i]->function.expr_str_parser.get_args()) {
                        functions[i]->function.expr_str_parser.set_args(arg.first, last_args[arg.first]);
                    }
                    //functions[i]->futures.push_back({ std::async(std::launch::async, [&, i] {functions[i]->function.recalculatePoints(); }) });
                    functions[i]->futures.push({ std::async(std::launch::async, [&, i] {functions[i]->function.recalculatePoints(); }) });
                }
            }
        }
        ImGui::End();

        if (imGuiIO.WantCaptureMouse || mouseDot.funcCaptured) {
            update_func = false;
        } else {
            update_func = true;
        }

        if (need_update_shaders) {
            coordAxisShader.use();
            coordAxisShader.setVec2("center", Function::xcenter, Function::ycenter);
            coordAxisShader.setVec2("size", Function::xsize, Function::ysize);
        	//coordAxisNumbersShader.use();
            //coordAxisNumbersShader.setVec2("center", Function::xcenter, Function::ycenter);
            //coordAxisNumbersShader.setVec2("size", Function::xsize, Function::ysize);
            funcShader.use();
            funcShader.setFloat("ycenter", Function::ycenter);
            need_update_shaders = false;
        }

        glLineWidth(1);
        VAO::bind(vao);
        coordAxisShader.use();
        glDrawArrays(GL_POINTS, 0, 1);

        VAO::bind(vao);
        coordAxisShader.use();
        //if (viewpoint_updated) {
        //    funcShader.use();
        //    funcShader.setVec2("resolution", SCR_WIDTH, SCR_HEIGHT);
        //    projection = glm::ortho(0.0f, SCR_WIDTH, 0.0f, SCR_HEIGHT);
        //    coordAxisNumbersShader.use();
        //    coordAxisNumbersShader.setMat4("projection", projection);
        //    textShader.use();
        //    textShader.setMat4("projection", projection);
        //}
        RenderAxisNumbers(coordAxisNumbersShader, Function::getCenter(), Function::getSize(), 0.25f, glm::vec3(0.7f));

        glLineWidth(3);
        funcShader.use();
        for (std::size_t i = 0; i<functions.size(); i++) {
            if (!functions[i]->show) { continue; }
            if (Function::needs_update) {
                //functions[i]->futures.push_back({ std::async(std::launch::async, [&, i] {functions[i]->function.recalculatePoints(); }) });
                functions[i]->futures.push({ std::async(std::launch::async, [&, i] {functions[i]->function.recalculatePoints(); }) });
            } else if (functions[i]->function.needs_personal_update) {
                functions[i]->futures.push({ std::async(std::launch::async, [&, i] {functions[i]->function.recalculatePoints(); }) });
                functions[i]->function.needs_personal_update = false;
            }
            while (!functions[i]->futures.empty() && functions[i]->futures.front()._Is_ready()) { //helps a bit to combat artifacts, when changing zoom(size)
                functions[i]->futures.pop();
                functions[i]->need_remap_vbo = true;
            }
            //for (auto it = functions[i]->futures.begin(); it<functions[i]->futures.end();) {
            //    if (it->_Is_ready()) { //if future is ready, remap vbo and remove the future from vector
            //        it = functions[i]->futures.erase(it);
            //        functions[i]->need_remap_vbo = true;
            //    } else {
            //        ++it;
            //    }
            //}
            if (functions[i]->need_remap_vbo) {
                VBO::bind(functions[i]->vbo);
                void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
                memcpy(ptr, functions[i]->function.points.data(), Function::calc_points_count*sizeof(glm::vec2)); //copy points data to vbo
                glUnmapBuffer(GL_ARRAY_BUFFER);
                functions[i]->need_remap_vbo = false;
            }

            VAO::bind(functions[i]->vao);
            funcShader.setVec3("color", functions[i]->color);
            glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, static_cast<GLsizei>(Function::calc_points_count));
        }
        Function::needs_update = false;

        if (mouseDot.funcCaptured) {
            VAO::bind(mouseDotVAO);
            VBO::setData(mouseDotVBO, sizeof(glm::vec2), &mouseDot.screenPos, GL_STATIC_DRAW);
            glPointSize(7);
            mouseDotShader.use();
            glDrawArrays(GL_POINTS, 0, 1);
            //std::stringstream ss;
            //ss << RenderAxisNumbersPrecision::xformatting << std::setprecision(RenderAxisNumbersPrecision::xprecision) << Function::xsize*(mouseDot.screenPos.x-Function::xcenter) << " ";
            //ss << RenderAxisNumbersPrecision::yformatting << std::setprecision(RenderAxisNumbersPrecision::yprecision) << Function::ysize*(mouseDot.func->expr_str_parser.calculate(mouseDot.screenPos.y+Function::ycenter));
            //std::lock_guard lg(mouseDot.func->m);
            //const std::string dotText = std::to_string(Function::xsize*(mouseDot.screenPos.x-Function::xcenter)) + " " + std::to_string(Function::ysize*(mouseDot.func->expr_str_parser.calculate(mouseDot.screenPos.y+Function::ycenter)));
            const std::string dotText = std::to_string(Function::xsize*(mouseDot.screenPos.x-Function::xcenter)) + " " + std::to_string(mouseDot.func->calcAtScrPos(mouseDot.screenPos));
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

    if (update_func) {
        if (mouse.leftPressed) {
            const float deltaX = xposIn-mouse.pos.x;
            const float deltaY = yposIn-mouse.pos.y;
            Function::incCenter(deltaX/(SCR_WIDTH/2.0f), deltaY/(SCR_HEIGHT/2.0f));
            need_update_shaders = true;
            Function::needs_update = true;
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
        need_update_shaders = true;
        Function::needs_update = true;
    }
}

void mouse_scroll_callback(GLFWwindow* window, const double xoffset, const double yoffset) {
    mouse.wheelScrolled = true;
    if (update_func) {
        const glm::vec2 mousePos = glm::vec2(mouse.pos.x/SCR_WIDTH*2.0f-1.0f, mouse.pos.y/SCR_HEIGHT*2.0f-1.0f);
        const glm::vec2 lastValue = (-Function::getCenter()+mousePos)*Function::getSize();
	    //const glm::vec2 lastCenterValue = (Function::getCenter()+glm::vec2(mouse.pos.x/SCR_WIDTH*2-1, mouse.pos.y/SCR_HEIGHT*2-1))*Function::getSize();
        //Function::incCenter(-(mouse.pos.x/SCR_WIDTH -Function::getCenterNDC().x)*(static_cast<float>(yoffset)/10.0f),
		//					-(mouse.pos.y/SCR_HEIGHT-Function::getCenterNDC().y)*(static_cast<float>(yoffset)/10.0f));
        Function::multSize((1.0f-static_cast<float>(yoffset)/20.0f), (1.0f-static_cast<float>(yoffset)/20.0f));
        Function::setCenter(-lastValue/Function::getSize()+mousePos);
        need_update_shaders = true;
        Function::needs_update = true;
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
        RenderText(shader, ss.str(), static_cast<float>(i+10)/10.0f*SCR_WIDTH/2.0f, (1.0f-(center.y+1.0f)/2.0f)*SCR_HEIGHT, scale, color);
        ss.str("");
        //shader.setVec2("updateOffset", 0.0f, -1.0f);
        ss << RenderAxisNumbersPrecision::yformatting << std::setprecision(RenderAxisNumbersPrecision::yprecision) << interval.y*(static_cast<float>(i)+center.y*10.0f);
        RenderText(shader, ss.str(), ((center.x+1.0f)/2.0f)*SCR_WIDTH, static_cast<float>(i+10)/10.0f*SCR_HEIGHT/2.0f, scale, color);
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
    //viewpoint_updated = true;
    SCR_WIDTH = static_cast<float>(width);
    SCR_HEIGHT = static_cast<float>(height);
    glViewport(0, 0, width, height);
    //0,0 - left bottom
    ViewpointUpdateShaderCallback();
}
