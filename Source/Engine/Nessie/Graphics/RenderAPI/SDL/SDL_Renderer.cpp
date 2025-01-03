// SDL_Renderer.cpp

#include "Application/Platforms/SDL/SDL_WindowContext.h"
#include "Core/Config.h"
#include "Debug/CheckedCast.h"

#ifdef NES_RENDER_API_SDL
#include <SDL.h>
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include "Graphics/Renderer.h"

namespace nes
{
    // Static renderer instance. This assumes that there is only going to be 1 Application Window.
    static SDL_Renderer* s_pRenderer = nullptr;

    static void SetDrawColor(const LinearColor& linearColor);
    static SDL_FRect ToSDL_FRect(const Rectf& rect);
    static SDL_Rect ToSDL_Rect(const Recti& rect);
    
    bool Renderer::Init(Window* pWindow)
    {
        NES_ASSERT(pWindow);
        
        // [TODO]: IMG_Init()
        
        m_pWindow = pWindow;

        // Get the Renderer:
        SDL_WindowContext* pContext = checked_cast<SDL_WindowContext*>(m_pWindow->GetWindowContext());
        NES_ASSERT(pContext);
        s_pRenderer = pContext->m_pNativeRenderer;
        NES_ASSERT(s_pRenderer);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        
        // Setup Platform/Renderer backends
        ImGui_ImplSDL2_InitForSDLRenderer(pContext->m_pNativeWindow, s_pRenderer);
        ImGui_ImplSDLRenderer2_Init(s_pRenderer);
        
        // [TODO]: If you want:
        // Load Fonts:
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != nullptr);
        
        return true;
    }

    void Renderer::Close()
    {
        // Cleanup IMGUI
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        
        // [TODO]: IMG_Quit();
        SDL_VideoQuit();
        
        s_pRenderer = nullptr;
    }

    void Renderer::BeginFrame()
    {
        // Begin the imgui frame:
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
    }

    void Renderer::SubmitFrame()
    {
        // Submit ImGUI Draw Calls:
        const ImGuiIO& io = ImGui::GetIO();
        ImGui::Render();
        SDL_RenderSetScale(s_pRenderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), s_pRenderer);

        // Submit SDL Draw Calls:
        SDL_RenderPresent(s_pRenderer);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Clears the Window Surface with a single Color. 
    //----------------------------------------------------------------------------------------------------
    void Renderer::Clear(const Color& color) const
    {
        NES_ASSERT(s_pRenderer);
        SDL_SetRenderDrawColor(s_pRenderer, color.r, color.g, color.b, color.a);     
        SDL_RenderClear(s_pRenderer);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Clears the Window Surface with a single Color. 
    //----------------------------------------------------------------------------------------------------
    void Renderer::Clear(const LinearColor& color) const
    {
        NES_ASSERT(s_pRenderer);
        SetDrawColor(color);     
        SDL_RenderClear(s_pRenderer);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Draw a line connecting 'from' and 'to'.
    //----------------------------------------------------------------------------------------------------
    void Renderer::DrawLine(Vec2 from, Vec2 to, const LinearColor& color) const
    {
        SetDrawColor(color);
        SDL_RenderDrawLineF(s_pRenderer, from.x, from.y, to.x, to.y);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Draw a wire rect.
    //----------------------------------------------------------------------------------------------------
    void Renderer::DrawRect(const Rectf& rect, const LinearColor& color) const
    {
        SetDrawColor(color);
        const SDL_FRect sdlRect = ToSDL_FRect(rect);
        SDL_RenderDrawRectF(s_pRenderer, &sdlRect);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Draw a filled rect. 
    //----------------------------------------------------------------------------------------------------
    void Renderer::DrawFillRect(const Rectf& rect, const LinearColor& color) const
    {
        SetDrawColor(color);
        const SDL_FRect sdlRect = ToSDL_FRect(rect);
        SDL_RenderFillRectF(s_pRenderer, &sdlRect);
    }

    //----------------------------------------------------------------------------------------------------
    //      NOTES:
    //      Here is my reference. It is an example of the 'Midpoint Circle Algorithm'.
    //      https://stackoverflow.com/questions/38334081/howto-draw-circles-arcs-and-vector-graphics-in-sdl
    //
    ///		@brief : Draw a wire circle.
    //----------------------------------------------------------------------------------------------------
    void Renderer::DrawCircle(Vec2 position, float radius, const LinearColor& color) const
    {
        SetDrawColor(color);

        const float diameter = radius * 2.f;
        float x = radius - 1.f;
        float y = 0.f;
        float tx = 1.f;
        float ty = 1.f;
        float error = (tx - diameter);

        while (x >= y)
        {
            // Each of the following renders an octant of the circle
            SDL_RenderDrawPointF(s_pRenderer, position.x + x, position.y - y);
            SDL_RenderDrawPointF(s_pRenderer, position.x + x, position.y + y);
            SDL_RenderDrawPointF(s_pRenderer, position.x - x, position.y - y);
            SDL_RenderDrawPointF(s_pRenderer, position.x - x, position.y + y);
            SDL_RenderDrawPointF(s_pRenderer, position.x + y, position.y - x);
            SDL_RenderDrawPointF(s_pRenderer, position.x + y, position.y + x);
            SDL_RenderDrawPointF(s_pRenderer, position.x - y, position.y - x);
            SDL_RenderDrawPointF(s_pRenderer, position.x - y, position.y + x);

            if (error <= 0.f)
            {
                ++y;
                error += ty;
                ty += 2.f;
            }

            if (error > 0.f)
            {
                --x;
                tx += 2.f;
                error += (tx - diameter);
            }
        } 
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : NOT SUPPORTED CURRENTLY. This will just draw the wire circle.
    //----------------------------------------------------------------------------------------------------
    void Renderer::DrawFillCircle(Vec2 position, float radius, const LinearColor& color) const
    {
        DrawCircle(position, radius, color);
    }

    void SetDrawColor(const LinearColor& linearColor)
    {
        NES_ASSERT(s_pRenderer);
        const Color color = ToColor(linearColor);
        SDL_SetRenderDrawColor(s_pRenderer, color.r, color.g, color.b, color.a);
    }

    SDL_FRect ToSDL_FRect(const Rectf& rect)
    {
        return SDL_FRect{rect.x, rect.y, rect.width, rect.height};
    }

    SDL_Rect ToSDL_Rect(const Recti& rect)
    {
        return SDL_Rect{rect.x, rect.y, rect.width, rect.height};
    }
}


#endif
