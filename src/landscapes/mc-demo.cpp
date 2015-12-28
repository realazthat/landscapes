
#include <ctime>
#include <string>
#include <iostream>
#include <memory>
#include <thread>

#include <tclap/CmdLine.h>

#include "landscapes/svo_render.hpp"
#include "landscapes/svo_formatters.hpp"
#include "landscapes/svo_render.hpp"

#include "sgfxapi/sgfxapi.glcommon.hpp"
#include "sgfxapi/sgfxapi-drawutils.hpp"
#include "sgfxapi/sgfxapi.hpp"


#include "CEGUI/CEGUI.h"
#include "CEGUI/RendererModules/OpenGL/GLRenderer.h"
#include "CEGUI/SchemeManager.h"
#include "CEGUI/WindowManager.h"


#include "cegui.glfw3.hpp"
#include "SMA.hpp"

static void glfwErrorCallback(int error, const char* description)
{
    fputs(description, stderr);
}

static void glfwWindowResizeCallback(GLFWwindow* window, int w, int h);
static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
static void glfwMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset);
static void glfwCharCallback(GLFWwindow* window, unsigned int codepoint);

struct motherclass_t{

    const std::string CEGUIInstallBasePath =
        "K:/realz/dump/code/cegui/cegui-0.8.4/cegui-0.8.4/" ;

    GLFWwindow* mwinder;
    bool d_windowSized;
    int d_newWindowWidth, d_newWindowHeight;

    std::shared_ptr<SGFXAPI::Graphics> graphics;

    std::vector<std::shared_ptr<SGFXAPI::RenderNode> > scene;

    CEGUI::WindowManager* windowmgr;
    CEGUI::GUIContext* guictx;


    std::shared_ptr<svo::svo_render_t> svotest;

    Frustum camera;
    float walkSpeed = 1;
    SMA fpssma;


    bool cegui_has_mouse;
    bool dragging; double xpos0, ypos0;

    std::shared_ptr<SGFXAPI::Mesh> svoquadmesh;
    ///{(pbo,fence,width, height, bytes)}
    std::vector< std::tuple<std::shared_ptr<SGFXAPI::PixelBuffer>, std::shared_ptr<SGFXAPI::Fence>, int, int, std::size_t> > svo_pbos;
    std::size_t svo_pbo_upload_idx, svo_pbo_render_idx;

    motherclass_t()
        : mwinder(nullptr), d_windowSized(false), d_newWindowWidth(0), d_newWindowHeight(0)
        , windowmgr(nullptr), guictx(nullptr)
        , fpssma(10), cegui_has_mouse(false), dragging(false), xpos0(0), ypos0(0)
    {}

    ~motherclass_t(){
        if (mwinder)
            glfwDestroyWindow(mwinder);
        glfwTerminate();
    }
    void glfwWindowResizeCallback(int w, int h)
    {
        // We cache this in order to minimise calls to notifyDisplaySizeChanged,
        // which happens in the main loop whenever d_windowSized is set to true.
        d_windowSized = true;
        d_newWindowWidth = w;
        d_newWindowHeight = h;
    }


    void initGLFW(int screenWidth, int screenHeight, const std::string& title)
    {
        glfwSetErrorCallback(glfwErrorCallback);

        if (!glfwInit())
            throw std::runtime_error("Error initializing GLFW");


        //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);



        mwinder = glfwCreateWindow(screenWidth, screenHeight, title.c_str(), NULL, NULL);


        if (!mwinder)
        {
            glfwTerminate();
            throw std::runtime_error("Could not create GLFW window");
        }
        /* Make the window's context current */
        glfwMakeContextCurrent(mwinder);


        glfwSetWindowUserPointer(mwinder, this);
        SGFXAPI::initializeGlew();
        glfwSwapInterval(1);

        glfwSetInputMode(mwinder, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

        glfwSetWindowSizeCallback(mwinder,::glfwWindowResizeCallback);
        // Input callbacks of glfw for CEGUI
        glfwSetKeyCallback(mwinder,glfwKeyCallback);
        glfwSetCharCallback(mwinder,glfwCharCallback);
        glfwSetMouseButtonCallback(mwinder,glfwMouseButtonCallback);
        glfwSetScrollCallback(mwinder,glfwMouseWheelCallback);
        glfwSetCursorPosCallback(mwinder,glfwCursorPosCallback);

    }


    void set_CEGUI_paths()
    {
     
        // Initialises the required directories for the DefaultResourceProvider:

        CEGUI::DefaultResourceProvider & defaultResProvider =
            * static_cast<CEGUI::DefaultResourceProvider*>
                ( CEGUI::System::getSingleton().getResourceProvider() ) ;

        const std::string CEGUIInstallSharePath = CEGUIInstallBasePath
          + "datafiles/" ;

        // For each resource type, sets a corresponding resource group directory:

        std::cout << "Using scheme directory '" << CEGUIInstallSharePath + "schemes/"
             << "'" << std::endl ;

        defaultResProvider.setResourceGroupDirectory( "schemes",
          CEGUIInstallSharePath + "schemes/" ) ;

        defaultResProvider.setResourceGroupDirectory( "imagesets",
          CEGUIInstallSharePath + "imagesets/" ) ;

        defaultResProvider.setResourceGroupDirectory( "fonts",
          CEGUIInstallSharePath + "fonts/" ) ;

        defaultResProvider.setResourceGroupDirectory( "layouts",
          CEGUIInstallSharePath + "layouts/" ) ;

        defaultResProvider.setResourceGroupDirectory( "looknfeels",
          CEGUIInstallSharePath + "looknfeel/" ) ;

        defaultResProvider.setResourceGroupDirectory( "lua_scripts",
          CEGUIInstallSharePath + "lua_scripts/" ) ;

        defaultResProvider.setResourceGroupDirectory( "schemas",
          CEGUIInstallSharePath + "xml_schemas/" ) ;

        defaultResProvider.setResourceGroupDirectory( "animations",
          CEGUIInstallSharePath + "animations/" ) ;

        // Sets the default resource groups to be used:
        //CEGUI::Imageset::setDefaultResourceGroup( "imagesets" ) ;
        CEGUI::Font::setDefaultResourceGroup( "fonts" ) ;
        CEGUI::Scheme::setDefaultResourceGroup( "schemes" ) ;
        CEGUI::WidgetLookManager::setDefaultResourceGroup( "looknfeels" ) ;
        CEGUI::WindowManager::setDefaultResourceGroup( "layouts" ) ;
        CEGUI::ScriptModule::setDefaultResourceGroup( "lua_scripts" ) ;
        CEGUI::AnimationManager::setDefaultResourceGroup( "animations" ) ;

        CEGUI::ImageManager::setImagesetDefaultResourceGroup("imagesets");

        // Set-up default group for validation schemas:
        CEGUI::XMLParser * parser = CEGUI::System::getSingleton().getXMLParser() ;
        if ( parser->isPropertyPresent( "SchemaDefaultResourceGroup" ) )
            parser->setProperty( "SchemaDefaultResourceGroup", "schemas" ) ;

    }

    void initCEGUI()
    {

        std::cout << " - initializing CEGUI" << std::endl ;




        ///http://cegui.org.uk/wiki/CEGUI_In_Practice_-_Introduction

        CEGUI::OpenGLRenderer* renderer = &CEGUI::OpenGLRenderer::bootstrapSystem();
        renderer->enableExtraStateSettings(true);

        set_CEGUI_paths();

        windowmgr = &CEGUI::WindowManager::getSingleton();
        guictx = &CEGUI::System::getSingleton().getDefaultGUIContext();

        // Load the scheme
        CEGUI::SchemeManager::getSingleton().createFromFile( "TaharezLook.scheme" );
        CEGUI::SchemeManager::getSingleton().createFromFile("WindowsLook.scheme");

        // Set the defaults
        //guictx->setDefaultFont("DejaVuSans-10");
        guictx->getMouseCursor().setDefaultImage("TaharezLook/MouseArrow");
     
    }


    void create_gui()
    {
        assert(guictx);
        assert(windowmgr);

        using namespace CEGUI;
        std::cout << " - creating the GUI" << std::endl ;

        CEGUI::Window *guiRoot = windowmgr->createWindow("DefaultWindow", "root");
        guictx->setRootWindow(guiRoot);

        ///frame
    
        CEGUI::FrameWindow* frame = static_cast<CEGUI::FrameWindow*>(windowmgr->createWindow("TaharezLook/FrameWindow", "Demo Window" ));

        frame->setPosition( UVector2( cegui_reldim(0.75f), cegui_reldim(0.0f) ) ) ;
        frame->setSize( USize( cegui_reldim(0.25f), cegui_reldim(1.f) ) ) ;

        frame->setMaxSize( USize( cegui_reldim(1.0f), cegui_reldim(1.0f) ) ) ;
        frame->setMinSize( USize( cegui_reldim(0.1f), cegui_reldim(0.1f) ) ) ;

        frame->setText( "Hello World! This is a minimal GLFW+OpenGL+CEGUI test." ) ;
        guiRoot->addChild( frame ) ;
        frame->setAlwaysOnTop(true);




        auto layout = static_cast<VerticalLayoutContainer*>(windowmgr->createWindow("VerticalLayoutContainer", "infoverticallayout"));
        frame->addChild(layout);



        std::vector< std::tuple< std::string, std::string > > rows{
              std::make_tuple("fps", "FPS")
            , std::make_tuple("camera.up", "c.up")
            , std::make_tuple("camera.front", "c.front")
            , std::make_tuple("camera.pos", "c.pos")
            , std::make_tuple("walkspeed", "walk speed")
        };

        int rowi = 0;
        for (auto rowtuple : rows)
        {
            std::string name, label;
            std::tie(name, label) = rowtuple;
            auto rowwnd = static_cast<HorizontalLayoutContainer*>(windowmgr->createWindow("HorizontalLayoutContainer", name + ".row"));
            rowwnd->setPosition( UVector2( cegui_reldim(0), cegui_absdim(50.0f*rowi) ) );
            rowwnd->setSize( USize(UDim(0.0f, 0.0f), UDim(0.0f, 50.0f)));
            layout->addChild(rowwnd);


            Window* labelwnd = windowmgr->createWindow("TaharezLook/Label", name + ".label");
            rowwnd->addChild(labelwnd);
            labelwnd->setSize(USize(UDim(.4f, 0.0f), UDim(0.0f, 50.0f)));
            labelwnd->setText(label);

            auto editwnd = static_cast<Window*>(windowmgr->createWindow("TaharezLook/Label", name + ".edit"));
            rowwnd->addChild(editwnd);

            editwnd->setPosition( UVector2( cegui_reldim(0.4f), cegui_reldim(0) ) );
            editwnd->setSize(USize(UDim(.6f, 0.0f), UDim(0.0f, 50.0f)));
            editwnd->setText("edit this");

            ++rowi;
        }

    }


    void initialize_svo()
    {
        svotest.reset(new svo::MCSVOTest(float3_t(3,3,-2), "k:/realz/dump/code/mordred.svo/sparse-voxel-octrees/build/tree/"));
        svotest->load_slices();
        svotest->load_blocks();
    }

    void initialize_camera()
    {
        camera.SetPos(float3(0, 0, -10));
        camera.SetFront(float3(0,0,1));
        camera.SetUp(float3(0,1,0));
        camera.SetViewPlaneDistances(.001, 250);

        //camera.horizontalFov = 3.141592654f/2.f;
        
        int width, height;
        glfwGetFramebufferSize(mwinder, &width, &height);

        float aspect_ratio = float(width) / float(height);
        camera.SetHorizontalFovAndAspectRatio(1, aspect_ratio);

        //camera.verticalFov = camera.horizontalFov * (float)height / (float)width;
        

        camera.SetKind(FrustumSpaceGL, FrustumRightHanded);

        std::cout << "HorizontalFov: " << camera.HorizontalFov()
            << ", VerticalFov(): " << camera.VerticalFov() << std::endl;

        assert(camera.Type() == PerspectiveFrustum);
    }


    void
    ComputeMouseOrientation(double delta)
    {
        
        if (cegui_has_mouse || glfwGetMouseButton(mwinder, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS)
        {
            dragging = false;
            return;
        }


        int width, height;
        glfwGetFramebufferSize(mwinder, &width, &height);
        
        double xpos, ypos;
        glfwGetCursorPos(mwinder, &xpos, &ypos);
        
        if (!dragging)
        {
            xpos0 = xpos;
            ypos0 = ypos;
            dragging = true;
            return;
        }

        // Reset mouse position for next frame
        glfwSetCursorPos(mwinder, xpos0, ypos0);
        
        double mouseSpeed = .01;
        double horizontalAngle = mouseSpeed * delta * double(xpos0 - xpos );
        double verticalAngle = mouseSpeed * delta * double(ypos0 - ypos );
        
        Quat rotation = Quat(camera.Up(), horizontalAngle) * Quat(camera.WorldRight(), verticalAngle).Normalized();
        camera.SetUp((rotation * camera.Up()).Normalized());
        camera.SetFront((rotation * camera.Front()).Normalized());
    }

    bool check_upload_pbo()
    {
        size_t next_svo_pbo_upload_idx = modinc(svo_pbo_upload_idx, svo_pbos.size());

        auto& pbo_tuple = svo_pbos[next_svo_pbo_upload_idx];

        auto& pbo = std::get<0>(pbo_tuple);
        auto& fence = *std::get<1>(pbo_tuple);


        if (fence.CheckWaiting())
            return false;


        return true;
    }

    bool upload_data_to_pbo(uint8_t* data, int width, int height, int bytes)
    {
        size_t next_svo_pbo_upload_idx = modinc(svo_pbo_upload_idx, svo_pbos.size());
        svo_pbo_upload_idx = next_svo_pbo_upload_idx;

        auto& pbo_tuple = svo_pbos[svo_pbo_upload_idx];
        auto& pbo = *std::get<0>(pbo_tuple);
        auto& fence = *std::get<1>(pbo_tuple);
        auto& pbowidth = std::get<2>(pbo_tuple);
        auto& pboheight = std::get<3>(pbo_tuple);
        auto& pbobytes = std::get<4>(pbo_tuple);

        assert(!(fence.Waiting()));
        
        auto pbo_bind = SGFXAPI::make_bind_guard(pbo);

        if (pbo.LogicalBufferSizeBytes() < bytes)
        {
            pbo.Resize(bytes);
            pbo.AllocateGpuMemory();
        }

        pbo.UpdateToGpu(data, bytes);
        pbowidth = width;
        pboheight = height;
        pbobytes = bytes;
        fence.Reset(true);

        return true;
    }


    ///Finds the most recently uploaded pbo after the last rendered pbo
    size_t find_next_render_pbo()
    {
        size_t n = svo_pbos.size();
        size_t next_idx = modinc(svo_pbo_render_idx, svo_pbos.size());

        size_t best_next_idx = (size_t)-1;
        for (; next_idx != svo_pbo_upload_idx; next_idx = modinc(next_idx, n))
        {
            auto& pbo_tuple = svo_pbos[next_idx];
            auto& fence = *std::get<1>(pbo_tuple);
            if (!fence.CheckWaiting())
                best_next_idx = next_idx;
        }

        return best_next_idx;
    }

    bool check_render_pbo_to_texture()
    {
        size_t next_svo_pbo_render_idx = find_next_render_pbo();

        if (next_svo_pbo_render_idx == (size_t)-1)
            return false;

        return true;
    }
    bool render_pbo_to_texture(std::shared_ptr<SGFXAPI::Texture>& textureptr)
    {
        using namespace SGFXAPI;

        size_t next_svo_pbo_render_idx = find_next_render_pbo();

        if (next_svo_pbo_render_idx == (size_t)-1)
            return false;

        svo_pbo_render_idx = next_svo_pbo_render_idx;

        auto& pbo_tuple = svo_pbos[svo_pbo_render_idx];
        auto& pbo = *std::get<0>(pbo_tuple);
        auto& fence = *std::get<1>(pbo_tuple);
        auto& width = std::get<2>(pbo_tuple);
        auto& height = std::get<3>(pbo_tuple);
        auto& bytes = std::get<4>(pbo_tuple);

        if (width*height == 0)
            return false;

        assert(!(fence.Waiting()));

        if (width != textureptr->Width() || height != textureptr->Height())
        {
            textureptr = std::make_shared<Texture>(TextureType::Texture2D, TextureInternalFormat::RGBA8, ResourceUsage::UsageImmutable
                                            , width, height, 1);
        }

        auto svotxt_bind = SGFXAPI::make_bind_guard(*textureptr);
        auto pbo_bind = SGFXAPI::make_bind_guard(pbo);

        textureptr->UpdateToGpu(width, height, 1
                                , TextureFormat(TextureElementType::FLOAT, TexturePixelFormat::RGBA)
                                , 0, bytes);

        fence.Reset(true);

        return true;
    }

    void loop()
    {
        std::cout << "beginning loop" << std::endl;

        bool doterminate = false;
        double last_time_pulse = glfwGetTime();

        int width0 = 0, height0 = 0;
        std::vector<float> buffer0((width0*height0)*4, 0);

        uint64_t frame = 0;

        while (!doterminate && !glfwWindowShouldClose(mwinder))
        {
            std::cout << "loop" << std::endl;

            double current_time_pulse = glfwGetTime();
            double delta = current_time_pulse - last_time_pulse;
            last_time_pulse = current_time_pulse;
            ++frame;

            fpssma.add(delta);


            if (glfwWindowShouldClose(mwinder))
            {
                doterminate = true;
                continue;
            }

            cegui_has_mouse = false;
            glfwPollEvents();

            float ratio;
            int width, height;
            glfwGetFramebufferSize(mwinder, &width, &height);
            ratio = width / (float) height;

            ComputeMouseOrientation(delta);

            // Make the window's context current 
            glfwMakeContextCurrent(mwinder);




            {
                std::cout << "moving camera" << std::endl;

                ///keyboard => camera
                float3 walkDirection(0,0,0);

                if (GLFW_PRESS  == glfwGetKey(mwinder,GLFW_KEY_W))
                    walkDirection += camera.Front();
                if (GLFW_PRESS  == glfwGetKey(mwinder,GLFW_KEY_S))
                    walkDirection -= camera.Front();
                if (GLFW_PRESS  == glfwGetKey(mwinder,GLFW_KEY_A))
                    walkDirection -= camera.WorldRight();
                if (GLFW_PRESS  == glfwGetKey(mwinder,GLFW_KEY_D))
                    walkDirection += camera.WorldRight();

                if (GLFW_PRESS  == glfwGetKey(mwinder,GLFW_KEY_LEFT_SHIFT))
                    walkDirection *= walkSpeed*3;
                else
                    walkDirection *= walkSpeed;

                camera.SetPos(camera.Pos() + walkDirection);
            }


            {
                std::cout << "rendering svo tile" << std::endl;

                using namespace SGFXAPI;

                if (width*height > 0)
                {
                    ///svo section
                    if (width != width0 || height != height0)
                    {
                        width0 = width;
                        height0 = height;
                        buffer0.resize((width0*height0)*4);
                    }


                    //glReadPixels(0,0,width, height,GL_DEPTH_COMPONENT,GL_FLOAT,zbuffer.data());

                    int pixel_bytes = 4*sizeof(float);
                    int pixel_stride = pixel_bytes;
                    int buffer_rowalign_adjust = (4 - (width*pixel_bytes) % 4) % 4;
                    int depth_rowalign_adjust = buffer_rowalign_adjust;

                    
                    std::cout << "svotest->render_tile()" << std::endl;
                    
                    struct render_tile_t{
                        void operator()(svo::MCSVOTest* svotest, float* data, int width, int height, glm::vec2 uv0, glm::vec2 uv1, Frustum& camera){
                            svotest->render_tile( data, width, height
                                        , uv0, uv1, camera);
                        }
                    };

                    int job_count = 6;

                    std::vector<std::thread> jobs;

                    int job_height0 = height / job_count;
                    int job_height1 = job_height0 + (height % job_height0);

                    float* data_ptr = buffer0.data();

                    render_tile_t render_tile_functor;

                    for (int i = 0; i < job_count - 1; ++i)
                    {
                        jobs.push_back( std::thread(std::bind(render_tile_functor
                                          , svotest.get()
                                          , data_ptr, width, job_height0
                                          , glm::vec2(0, float(i) / job_count)
                                          , glm::vec2(1, float(i+1) / job_count)
                                          , camera))
                                          );
                        data_ptr += width*job_height0*4;
                    }
                    jobs.push_back( std::thread(std::bind(render_tile_functor
                                          , svotest.get()
                                          , data_ptr, width, job_height1
                                          , glm::vec2(0, float(job_count-1) / job_count)
                                          , glm::vec2(1, float(job_count) / job_count)
                                          , camera)));

                    data_ptr += width*job_height1*4;
                    assert(buffer0.data() + buffer0.size() == data_ptr);

                    for (auto& job : jobs)
                        job.join();

                    std::cout << "  ... done" << std::endl;
                    
                    /*
                    float* buffer_ptr = buffer0.data();
                    for (int y = 0; y < height; ++y)
                    {
                        for (int x = 0; x < width; ++x)
                        {
                            for (int i = 0; i < 3; ++i)
                                *buffer_ptr++ = 1;
                            *buffer_ptr++ = 0;
                        }
                    }
                    assert(buffer_ptr == buffer0.data() + buffer0.size());
                    */

                    std::cout << "frame: " << frame << std::endl;

                    ///upload
                    {
                        if (check_upload_pbo())
                        {
                            uint8_t* data = reinterpret_cast<uint8_t*>(buffer0.data());
                            int bytes = buffer0.size()*sizeof(float);

                            bool success = upload_data_to_pbo(data, width, height, bytes);

                            std::cout << "upload_data_to_pbo: " << (success ? "true" : "false")
                                << ", svo_pbo_upload_idx: " << svo_pbo_upload_idx << std::endl;
                        }
                    }

                    ///render
                    {
                        if (check_render_pbo_to_texture())
                        {
                            bool success = render_pbo_to_texture(svoquadmesh->textures[0]->texture);
                            std::cout << "render_pbo_to_texture: " << (success ? "true" : "false")
                                << ", svo_pbo_render_idx: " << svo_pbo_render_idx << std::endl;
                        }
                    }
                    
                }
            }
            




            std::map<int, std::tuple< std::shared_ptr<SGFXAPI::Texture> > > oldtexstates;

            ///gfxapi section
            {
                std::cout << "drawing graphics" << std::endl;

                graphics->Clear();

                
                SGFXAPI::ClearBindings();
                for (auto& node : scene)
                {

                    node->mesh->Bind();
                    node->mesh->sp->Use();


                    
                    int worldLocation = node->mesh->sp->GetUniformLocation("world");
                    int worldViewProjLocation = node->mesh->sp->GetUniformLocation("worldViewProj");
                    
                    //assert(worldLocation != -1);
                    //assert(worldViewProjLocation != -1);
                    if (worldLocation != -1)
                        node->mesh->sp->SetFloat4x4(worldLocation, node->xform);
                    if (worldViewProjLocation != -1)
                        node->mesh->sp->SetFloat4x4(worldViewProjLocation, camera.ViewProjMatrix() * node->xform);

                    for (auto mesh_texture : node->mesh->textures | indirected)
                    {
                        auto& texture_unit = *mesh_texture.texture_unit;


                        texture_unit.Activate();

                        mesh_texture.texture->Bind();
                        oldtexstates[texture_unit.Index()] = std::make_tuple(mesh_texture.texture);

                        if (!mesh_texture.sampler)
                            texture_unit.UnBindSampler();
                        else
                            texture_unit.BindSampler(*mesh_texture.sampler);

                        node->mesh->sp->BindTexture(texture_unit.Index(), texture_unit, *mesh_texture.texture, mesh_texture.sampler_name);
                    }
                    //glPolygonMode(GL_FRONT, GL_LINE);
                    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
                    node->mesh->Draw();
                    //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
                    

                    node->mesh->sp->Deselect();
                }

                ///Unbind all textures from all texture units used.
                /*
                for (auto unit2texture : oldtexstates)
                {
                    SGFXAPI::TextureUnit unit(unit2texture.first);
                    unit.Activate();

                    auto state = unit2texture.second;

                    std::shared_ptr<SGFXAPI::Texture> texture;
                    std::tie(texture) = state;
                    texture->UnBind();
                }*/

                SGFXAPI::ClearBindings();
            }

            
            ///Unbind texture from texture unit 0, before running cegui
            {
                SGFXAPI::TextureUnit unit(0);
                unit.Activate();
                glBindTexture(GL_TEXTURE_2D, 0);
            }

            ///cegui section
            {
                std::cout << "drawing cegui" << std::endl;

                if (frame % 10 == 0)
                {
                    auto fpsedit = guictx->getRootWindow()->getChildRecursive("fps.edit");

                    if (fpsedit){
                        std::string fpstext = svo::tostr((1.0/fpssma.avg()));
                        fpsedit->setText( fpstext );
                    }
                }

                auto cameraupedit = guictx->getRootWindow()->getChildRecursive("camera.up.edit");
                if (cameraupedit)
                {
                    auto up = camera.Up();
                    std::string cameratext = svo::tostr(up);
                    cameraupedit->setText( cameratext );
                }
                auto camerafrontedit = guictx->getRootWindow()->getChildRecursive("camera.front.edit");
                if (camerafrontedit)
                {
                    auto front = camera.Front();
                    std::string cameratext = svo::tostr(front);
                    camerafrontedit->setText( cameratext );
                }
                auto cameraposedit = guictx->getRootWindow()->getChildRecursive("camera.pos.edit");
                if (cameraposedit)
                {
                    auto pos = camera.Pos();
                    std::string cameratext = svo::tostr(pos);
                    cameraposedit->setText( cameratext );
                }

                auto walkspeededit = guictx->getRootWindow()->getChildRecursive("walkspeed.edit");
                if (walkspeededit)
                {
                    auto pos = camera.Pos();
                    std::string walkspeedtext = svo::tostr(walkSpeed);
                    walkspeededit->setText( walkspeedtext );
                }
                //inject_input( doterminate );
                CEGUI::System::getSingleton().injectTimePulse(delta);

                if (d_windowSized)
                {
                    d_windowSized = false;
                    CEGUI::System::getSingleton().
                        notifyDisplaySizeChanged(
                        CEGUI::Sizef(static_cast<float>(d_newWindowWidth),
                        static_cast<float>(d_newWindowHeight)));
                }




                CEGUI::System::getSingleton().renderAllGUIContexts();
            }
            


            // Swap front and back buffers 
            glfwSwapBuffers(mwinder);
        }
    }





};

static void glfwWindowResizeCallback(GLFWwindow* window, int w, int h)
{
    motherclass_t* motherclassptr = static_cast<motherclass_t*>(glfwGetWindowUserPointer(window));
    assert(motherclassptr);
    if (motherclassptr->guictx)
        motherclassptr->glfwWindowResizeCallback(w,h);
}
static void glfwCharCallback(GLFWwindow* window, unsigned int codepoint)
{
    motherclass_t* motherclassptr = static_cast<motherclass_t*>(glfwGetWindowUserPointer(window));
    assert(motherclassptr);
    if (motherclassptr->guictx)
        motherclassptr->guictx->injectChar(codepoint);
}

static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    motherclass_t* motherclassptr = static_cast<motherclass_t*>(glfwGetWindowUserPointer(window));
    auto cegui_key = translate_glfw3_cegui(key);

    if (cegui_key != CEGUI::Key::Unknown) {
        if (action == GLFW_PRESS){
            if (motherclassptr->guictx)
                motherclassptr->guictx->injectKeyDown(CEGUI::Key::Scan(cegui_key));
        }
        else if (action == GLFW_RELEASE) {
            if (motherclassptr->guictx)
                motherclassptr->guictx->injectKeyUp(CEGUI::Key::Scan(cegui_key));
        }
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window,1);

}

static void glfwCursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    motherclass_t* motherclassptr = static_cast<motherclass_t*>(glfwGetWindowUserPointer(window));
    assert(motherclassptr);
    if (motherclassptr->guictx)
        motherclassptr->guictx->injectMousePosition(xpos, ypos);
}
static CEGUI::MouseButton ConvertMouseButton(int button)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
        return CEGUI::RightButton;
    else if (button == GLFW_MOUSE_BUTTON_LEFT)
        return CEGUI::LeftButton;
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
        return CEGUI::MiddleButton;
    
   return CEGUI::LeftButton;
}

static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    motherclass_t* motherclassptr = static_cast<motherclass_t*>(glfwGetWindowUserPointer(window));
    assert(motherclassptr);
    if (action == GLFW_PRESS)
        if (motherclassptr->guictx)
            motherclassptr->cegui_has_mouse |= motherclassptr->guictx->injectMouseButtonDown( ConvertMouseButton(button) );
    if (action == GLFW_RELEASE)
        if (motherclassptr->guictx)
            motherclassptr->cegui_has_mouse |= motherclassptr->guictx->injectMouseButtonUp( ConvertMouseButton(button) );

}
static void glfwMouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    motherclass_t* motherclassptr = static_cast<motherclass_t*>(glfwGetWindowUserPointer(window));

    assert(motherclassptr);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    float walkSpeed0 = motherclassptr->walkSpeed;

    float stride = float(height) / 16.0;
    float walkSpeed1 = walkSpeed0 * (1 + (float(yoffset) / stride));

    motherclassptr->walkSpeed = std::max<float>(walkSpeed1, .000001);


}


int main()
{



    glewExperimental = GL_TRUE;

    motherclass_t motherclass;

    motherclass.initGLFW(420, 420, "window title");

    motherclass.graphics.reset(new SGFXAPI::Graphics());

    motherclass.initCEGUI();


    motherclass.create_gui();


    motherclass.initialize_svo();

    
    motherclass.initialize_camera();

    using SGFXAPI::RenderNode;
    {
        //auto node = std::make_shared<RenderNode>();
        //node->mesh = simpletri();
        //node->xform = float4x4::FromTRS(float3(0,0,0), Quat::identity, float3(1,1,1));
        //motherclass.scene.push_back( node );
    }
    {
        auto node = std::make_shared<RenderNode>();
        node->mesh = axes();
        node->xform = float4x4::FromTRS(float3(0,0,0), Quat::identity, float3(1,1,1));
        motherclass.scene.push_back( node );
    }
    
    
    {
        for (int i = 0; i < 3; ++i)
        {
            int bytes = 1024*1024*4*sizeof(float);

            auto pbo = boost::make_shared<SGFXAPI::PixelBuffer>(SGFXAPI::Usage::STREAM_DRAW, bytes, false/* allocateCpu */);
            auto fence = boost::make_shared<SGFXAPI::Fence>(false);

            auto pbo_bind = make_bind_guard(*pbo);

            pbo->AllocateGpuMemory();
            motherclass.svo_pbos.push_back( std::make_tuple(pbo, fence, 0, 0, 0) );
        }

        motherclass.svo_pbo_upload_idx = 0;
        motherclass.svo_pbo_render_idx = 1;

        auto node = std::make_shared<RenderNode>();
        node->mesh = motherclass.svoquadmesh = SSTQRGBD(true);
        node->xform = float4x4::FromTRS(float3(0,0,0), Quat::identity, float3(1,1,1));
        motherclass.scene.push_back( node );
    }
    

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS); 
    motherclass.loop();

    return 0;
}

