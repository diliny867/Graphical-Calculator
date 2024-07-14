#include "../include/Application.h"

using namespace Application;

void App::createImGuiMenu() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Menu");
    { //draw GUI with ImGUI
        if (ImGui::Button("Home")) {
            Function::setSize(10.0f, 10.0f);
            Function::setCenter(0.0f, 0.0f);
            needUpdateShaders = true;
            Function::allDirty = true;
            RenderAxisNumbersPrecision::updatePrecision();
        }
        ImGui::SameLine();
        if (ImGui::Button("+")) {
            Function::multSize(1.0f-imGuiData.buttonsInc, 1.0f-imGuiData.buttonsInc);
            needUpdateShaders = true;
            Function::allDirty = true;
            RenderAxisNumbersPrecision::updatePrecision();
        }
        ImGui::SameLine();
        if (ImGui::Button("-")) {
            Function::multSize(1.0f+imGuiData.buttonsInc, 1.0f+imGuiData.buttonsInc);
            needUpdateShaders = true;
            Function::allDirty = true;
            RenderAxisNumbersPrecision::updatePrecision();
        }
        if (ImGui::Button("X +")) {
            Function::multSize(1.0f-imGuiData.buttonsInc, 1.0f);
            needUpdateShaders = true;
            Function::allDirty = true;
            RenderAxisNumbersPrecision::updatePrecision();
        }
        ImGui::SameLine();
        if (ImGui::Button("X -")) {
            Function::multSize(1.0f+imGuiData.buttonsInc, 1.0f);
            needUpdateShaders = true;
            Function::allDirty = true;
            RenderAxisNumbersPrecision::updatePrecision();
        }
        ImGui::SameLine();
        if (ImGui::Button("Y +")) {
            Function::multSize(1.0f, 1.0f-imGuiData.buttonsInc);
            needUpdateShaders = true;
            Function::allDirty = true;
            RenderAxisNumbersPrecision::updatePrecision();
        }
        ImGui::SameLine();
        if (ImGui::Button("Y -")) {
            Function::multSize(1.0f, 1.0f+imGuiData.buttonsInc);
            needUpdateShaders = true;
            Function::allDirty = true;
            RenderAxisNumbersPrecision::updatePrecision();
        }
        //ImGui::Text("\n"); //somehow this makes indent smaller than " "
        ImGui::Checkbox("Mouse Dot by distance", &mouseDot.byDistance);
        ImGui::SameLine();
        if(ImGui::ColorEdit3("Mouse Dot color", value_ptr(mouseDot.color), ImGuiColorEditFlags_NoInputs)) {
            shaders.mouseDotShader.use();
            shaders.mouseDotShader.setVec3("color", mouseDot.color);
        }
        if (ImGui::Button("Add new function")) {
            std::lock_guard lg(m);
            functions.push_back(new FuncData());
            VBO::generate(functions.back()->vbo, static_cast<GLsizeiptr>(Function::calcPointsCount*sizeof(glm::vec2)), functions.back()->function.points.data(), GL_DYNAMIC_DRAW);
            VAO::generate(functions.back()->vao);
            VAO::bind(functions.back()->vao);
            VAO::addAttrib(functions.back()->vao, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
            functions.back()->color = glm::vec3(static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX));
            functions.back()->function.setFunction("");
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
        for (std::size_t i = 0; i<functions.size(); i++) {
            const std::string id = std::to_string(i);
            if (ImGui::Button(("-##"+id).c_str())) {
                VBO::deleteIt(functions.at(i)->vbo);
                VAO::deleteIt(functions.at(i)->vao);
                functions.erase(functions.begin()+i);
                continue;
            }
            ImGui::SameLine();
            if (functions[i]->show) {
                if (ImGui::Button(("hide##"+id).c_str())) {
                    functions[i]->show = false;
                }
            } else {
                if (ImGui::Button(("show##"+id).c_str())) {
                    functions[i]->show = true;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button(("Variables##"+id).c_str())) {
                if (!functions[i]->function.exprStrParser.GetArgs().empty()) {
                    ImGui::OpenPopup(("Variables##"+id).c_str());
                }
            }
            if (ImGui::BeginPopup(("Variables##"+id).c_str())) {
                for (auto& arg: functions[i]->function.exprStrParser.GetArgs()) {
                    if (ImGui::InputDouble((arg.first+"##Input").c_str(), &arg.second)) {
                        functions[i]->function.exprStrParser.SetArgs(arg.first,arg.second);
                        functions[i]->function.dirty = true;
                    }
                    ImGui::SameLine();
                    const float inc = abs(arg.second/10.0f);
                    if (ImGui::DragScalar((arg.first+"##Drag").c_str(), ImGuiDataType_Double, &arg.second, inc==0.0f?0.001f:(inc>1000.0f?1000.0f:inc))) {
                        functions[i]->function.exprStrParser.SetArgs(arg.first,arg.second);
                        functions[i]->function.dirty = true;
                    }
                }
                ImGui::EndPopup();
            }
            ImGui::SameLine();
            ImGui::ColorEdit3(("##"+id).c_str(), value_ptr(functions[i]->color), ImGuiColorEditFlags_NoInputs);
            ImGui::SameLine();
            if (ImGui::InputText(("##"+id).c_str(), &functions[i]->inputData)) {
                auto last_args = functions[i]->function.exprStrParser.GetArgs();
                functions[i]->function.setFunction(functions[i]->inputData);

                for (auto& arg: functions[i]->function.exprStrParser.GetArgs()) {
                    functions[i]->function.exprStrParser.SetArgs(arg.first, last_args[arg.first]);
                }
                //functions[i]->futures.push_back({ std::async(std::launch::async, [&, i] {functions[i]->function.recalculatePoints(); }) });
                functions[i]->futures.push({ std::async(std::launch::async, [&, i] {functions[i]->function.recalculatePoints(); }) });
            }
        }
    }
    ImGui::End();
}

void App::createMouseDot() {
    shaders.mouseDotShader.use();
    shaders.mouseDotShader.setVec3("color", mouseDot.color);
    mouseDot.thread = new std::thread([&]() { //thread that calculates poin for mouse
        bool lastLeftPressed = false;
        while (appOn) {
            if (mouse.leftPressed) {
                if (imGuiIO->WantCaptureMouse) {continue;}
                const glm::vec2 mousePos = glm::vec2(mouse.pos.x / screenSize.x * 2.0f - 1.0f, -(mouse.pos.y / screenSize.y * 2.0f - 1.0f));
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
                            if(glm::distance(mousePos, closestPoint)<0.025f) { //capture function
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
}

int App::glInit() {
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

int App::init() {
    srand(static_cast<unsigned int>(time(NULL)));
    Time::Init();

    int error;
    error = glInit();
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

    shaders.funcShader = Shader("resources/shaders/function_vs.glsl", "resources/shaders/function_gs.glsl", "resources/shaders/function_fs.glsl");
    shaders.coordAxisShader = Shader("resources/shaders/coordAxisShader_vs.glsl", "resources/shaders/coordAxisShader_gs.glsl", "resources/shaders/coordAxisShader_fs.glsl");
    shaders.coordAxisNumbersShader = Shader("resources/shaders/coordAxisNumbersShader_vs.glsl", "resources/shaders/coordAxisNumbersShader_fs.glsl");
    shaders.textShader = Shader("resources/shaders/textShader_vs.glsl", "resources/shaders/textShader_fs.glsl");
    shaders.mouseDotShader = Shader("resources/shaders/mouseDot_vs.glsl","resources/shaders/mouseDot_fs.glsl");

    glm::mat4 projection = glm::ortho(0.0f, screenSize.x, 0.0f, screenSize.y); //setup projection and shader data
    shaders.coordAxisNumbersShader.use();
    shaders.coordAxisNumbersShader.setMat4("projection", projection);
    shaders.coordAxisNumbersShader.setVec2("center", Function::xcenter, Function::ycenter);
    shaders.coordAxisNumbersShader.setVec2("size", Function::xsize, Function::ysize);
    shaders.coordAxisNumbersShader.setVec2("updateOffset", 0.0f, 0.0f);
    shaders.textShader.use();
    shaders.textShader.setMat4("projection", projection);

    //generate buffers for GPU
    //GLuint vbo;
    //VBO::generate(vbo, static_cast<GLsizeiptr>(Function::calc_points_count*sizeof(glm::vec2)), NULL, GL_DYNAMIC_DRAW);
    //VBO::bind(vbo);
    VAO::generate(mainGlObjects.coordAxisVAO);
    VAO::bind(mainGlObjects.coordAxisVAO);
    VAO::addAttrib(mainGlObjects.coordAxisVAO, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    //GLuint ebo;
    //EBO::generate(ebo, sizeof(indices), indices, GL_STATIC_DRAW);

    VBO::generate(mainGlObjects.mouseDotVBO, sizeof(glm::vec2), NULL, GL_STATIC_DRAW);
    VAO::generate(mainGlObjects.mouseDotVAO);
    VAO::bind(mainGlObjects.mouseDotVAO);
    VAO::addAttrib(mainGlObjects.mouseDotVAO, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    shaders.funcShader.use();
    //constexpr float markerSize = 0.0f;
    //funcShader.setFloat("markerRadius", markerSize);
    shaders.funcShader.setVec2("resolution", screenSize.x, screenSize.y);
    shaders.funcShader.setFloat("ycenter", 0.0f);
    shaders.funcShader.setVec3("color", glm::vec3(1.0f)); //desmos color: glm::vec3(199.0f, 68.0f, 64.0f)/255.0f
    //glUniform2fv(glGetUniformLocation(funcShader.id, "vPoints"), static_cast<GLsizei>(function.points.size()), reinterpret_cast<float*>(function.points.data()));

    shaders.coordAxisShader.use();
    shaders.coordAxisShader.setVec2("center", 0.0f, 0.0f);
    shaders.coordAxisShader.setVec2("size", Function::xsize, Function::ysize);
    //coordAxisShader.setFloat("gridSize", 10.0f);

    glm::vec3 coordAxisCenterColor = glm::vec3(0.5f);
    glm::vec3 coordAxisGridColor = glm::vec3(0.2f);
    shaders.coordAxisShader.setVec3("centerColor", coordAxisCenterColor);
    shaders.coordAxisShader.setVec3("gridColor", coordAxisGridColor);

    ViewpointUpdateShaderCallback = [&]() { //set up callback
        shaders.funcShader.use();
        shaders.funcShader.setVec2("resolution", screenSize.x, screenSize.y);
        projection = glm::ortho(0.0f, screenSize.x, 0.0f, screenSize.y);
        shaders.coordAxisNumbersShader.use();
        shaders.coordAxisNumbersShader.setMat4("projection", projection);
        shaders.textShader.use();
        shaders.textShader.setMat4("projection", projection);
    };

    //add first function
    functions.push_back(new FuncData()); //add initial blank function
    VBO::generate(functions.back()->vbo, static_cast<GLsizeiptr>(Function::calcPointsCount*sizeof(glm::vec2)), functions.back()->function.points.data(), GL_DYNAMIC_DRAW);
    VAO::generate(functions.back()->vao);
    VAO::bind(functions.back()->vao);
    VAO::addAttrib(functions.back()->vao, 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    //functions.back()->color = glm::vec3(1.0f);
    functions.back()->color = glm::vec3(static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX), static_cast<float>(rand())/(RAND_MAX));
    functions.back()->function.setFunction("");

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
        VAO::bind(mainGlObjects.coordAxisVAO);
        shaders.coordAxisShader.use();
        glDrawArrays(GL_POINTS, 0, 1);

        VAO::bind(mainGlObjects.coordAxisVAO);
        shaders.coordAxisShader.use();
        //if (viewpoint_updated) {
        //    funcShader.use();
        //    funcShader.setVec2("resolution", screenSize.x, screenSize.y);
        //    projection = glm::ortho(0.0f, screenSize.x, 0.0f, screenSize.y);
        //    coordAxisNumbersShader.use();
        //    coordAxisNumbersShader.setMat4("projection", projection);
        //    textShader.use();
        //    textShader.setMat4("projection", projection);
        //}
        renderAxisNumbers(shaders.coordAxisNumbersShader, Function::getCenter(), Function::getSize(), 0.25f, glm::vec3(0.7f)); //render numbers on number axis

        glLineWidth(3);
        shaders.funcShader.use();
        for (std::size_t i = 0; i<functions.size(); i++) { //check if function recalculated and draw it
            auto& function = functions[i];
            if (!function->show) { continue; }
            if (Function::allDirty || function->function.dirty) {
                //functions[i]->futures.push_back({ std::async(std::launch::async, [&, i] {functions[i]->function.recalculatePoints(); }) });
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
                memcpy(ptr, function->function.points.data(), Function::calcPointsCount*sizeof(glm::vec2)); //copy points data to vbo
                glUnmapBuffer(GL_ARRAY_BUFFER);
                function->needRemapVBO = false;
            }

            VAO::bind(function->vao);
            shaders.funcShader.setVec3("color", function->color);
            glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, static_cast<GLsizei>(Function::calcPointsCount));
        }
        Function::allDirty = false;

        if (mouseDot.funcCaptured) { //draw mouse dot and value at it
            VAO::bind(mainGlObjects.mouseDotVAO);
            VBO::setData(mainGlObjects.mouseDotVBO, sizeof(glm::vec2), &mouseDot.screenPos, GL_STATIC_DRAW);
            glPointSize(7);
            shaders.mouseDotShader.use();
            glDrawArrays(GL_POINTS, 0, 1);
        	const std::string dotText = std::to_string(Function::xsize*(mouseDot.screenPos.x-Function::xcenter)) + " " + std::to_string(mouseDot.func->calcAtScrPos(mouseDot.screenPos));
            TextRender::RenderText(shaders.textShader, dotText, screenSize.x*(mouseDot.screenPos.x+1.0f)*0.5f+10.0f, screenSize.y*(mouseDot.screenPos.y+1.0f)*0.5f, 0.3f, glm::vec3(0.7f));
        }

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

void Application::RenderAxisNumbersPrecision::updatePrecision() {
    xprecision = Function::numbers_float_precision;
    yprecision = Function::numbers_float_precision;
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