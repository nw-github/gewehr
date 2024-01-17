#include <stdafx.h>

#include "utils/render.hpp"

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

#include "utils/utils.hpp"

LRESULT CALLBACK Renderer::window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    auto self = reinterpret_cast<Renderer *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if ((GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST) == 0)
    {
        // RECT bounding;
        // GetWindowRect(g::memory->window, &bounding);
        // SetWindowPos(
        //     g::memory->window,
        //     self->m_hwnd,
        //     bounding.left,
        //     bounding.top,
        //     bounding.right - bounding.left,
        //     bounding.bottom - bounding.top,
        //     0);
    }

    switch (msg)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
    } return 0;
    }

    return DefWindowProcA(hwnd, msg, wParam, lParam);
}

void Renderer::thread_proc(std::stop_token token)
{
    if (!initialize(utl::randstr(utl::randint(10, 15)), utl::randstr(utl::randint(10, 15)), true))
        return;

    MSG msg = {0};
    while (!token.stop_requested())
    {
        while (PeekMessageA(&msg, m_hwnd, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        render();
    }

    shutdown();
}

bool Renderer::initialize_dx()
{
    DXGI_SWAP_CHAIN_DESC desc               = {0};
    desc.BufferCount                        = 2;
    desc.BufferDesc.Width                   = 0;
    desc.BufferDesc.Height                  = 0;
    desc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferDesc.RefreshRate.Numerator   = 60;
    desc.BufferDesc.RefreshRate.Denominator = 1;
    desc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    desc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.OutputWindow                       = m_hwnd;
    desc.SampleDesc.Count                   = 1;
    desc.SampleDesc.Quality                 = 0;
    desc.Windowed                           = TRUE;
    desc.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    UINT flags = 0;
    D3D_FEATURE_LEVEL feature_level;
    const D3D_FEATURE_LEVEL arr_feature_level[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (FAILED(D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        flags,
        arr_feature_level,
        2,
        D3D11_SDK_VERSION,
        &desc, 
        &m_swapchain,
        &m_device,
        &feature_level,
        &m_device_ctx)))
        return false;

    ID3D11Texture2D* back_buffer;
    if (FAILED(m_swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer))))
        return false;

    if (FAILED(m_device->CreateRenderTargetView(back_buffer, NULL, &m_render_target_view)))
    {
        back_buffer->Release();
        return false;
    }

    back_buffer->Release();
    return true;
}

bool Renderer::initialize(const std::string &class_name, const std::string &wnd_name, bool topmost)
{
    WNDCLASSEXA wc   = {0};
    wc.cbSize        = sizeof(WNDCLASSEXA);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = window_proc;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = HBRUSH(RGB(0, 0, 0));
    wc.lpszClassName = class_name.c_str();
    if (!RegisterClassExA(&wc))
        return false;

    // RECT bounding;
    // GetWindowRect(g::memory->window, &bounding);
    // m_position.x   = bounding.left;
    // m_position.y   = bounding.top;
    // m_resolution.x = bounding.right - bounding.left;
    // m_resolution.y = bounding.bottom - bounding.top;

    DWORD style = WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED;
    if (topmost)
        style |= WS_EX_TOPMOST;

    m_hwnd = CreateWindowExA(style, class_name.c_str(), wnd_name.c_str(), WS_POPUP | WS_VISIBLE,
        m_position.x, m_position.y, m_resolution.x, m_resolution.y, nullptr, nullptr, nullptr, nullptr);
    if (!m_hwnd)
        return false;

    MARGINS margins{-1};
    DwmExtendFrameIntoClientArea(m_hwnd, &margins);
    SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    SetWindowLong(m_hwnd, GWL_STYLE, 0);
    SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR)this);
    ShowWindow(m_hwnd, SW_SHOW);

    if (!initialize_dx())
    {
        shutdown();
        return false;
    }

    return true;
}

void Renderer::shutdown()
{
#define RELEASE_IF_VALID(x) do { if (x != nullptr) { x->Release(); x = nullptr; } } while(0)

    RELEASE_IF_VALID(m_render_target_view);
    RELEASE_IF_VALID(m_swapchain);
    RELEASE_IF_VALID(m_device_ctx);
    RELEASE_IF_VALID(m_device);
    
    if (m_hwnd != nullptr)
    {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }

#undef RELEASE_IF_VALID
}

void Renderer::render()
{
    const float clear_color[] = { 0, 0, 0, 255 };
    m_device_ctx->OMSetRenderTargets(1, &m_render_target_view, NULL);
    m_device_ctx->ClearRenderTargetView(m_render_target_view, clear_color);
    
    std::queue<DrawListItem> draw_list;
    {
        std::scoped_lock lock(m_draw_list_mutex);
        std::swap(draw_list, m_draw_list);
    }

    // if (utl::is_csgo_focused())
    // {
    //     while (!draw_list.empty())
    //     {
    //         draw_item(draw_list.front());
    //         draw_list.pop();
    //     }
    // }

    m_swapchain->Present(0, 0);
}

void Renderer::add_line(int x0, int y0, int x1, int y1, Color color)
{
    std::scoped_lock lock(m_draw_list_mutex);
    m_draw_list.push(DrawListItem{ 
        .x      = x0,
        .y      = y0,
        .color  = color,
        .params = DrawListItem::Line{x1, y1}
    });
}

void Renderer::add_rect(int x, int y, int w, int h, Color color, DrawListItem::Rect::Style style)
{
    std::scoped_lock lock(m_draw_list_mutex);
    m_draw_list.push(DrawListItem{ 
        .x      = x,
        .y      = y,
        .color  = color,
        .params = DrawListItem::Rect{w, h, style}
    });
}

void Renderer::add_string(std::string_view str, int x, int y, Color color)
{
    std::scoped_lock lock(m_draw_list_mutex);
    m_draw_list.push(DrawListItem{
        .x        = x,
        .y        = y,
        .color     = color,
        .params = std::string(str)
    });
}

void Renderer::draw_item(const DrawListItem &item)
{
    //switch (item.params.index()) {
    //case 0: {
    //    const auto &info = std::get<0>(item.params);
    //    draw_line(item.x, item.y, info.x2, info.y2, item.color);
    //} break;
    //case 1: {
    //    const auto &info = std::get<1>(item.params);
    //    switch (info.type) {
    //    case draw_list_item::rect::NORMAL:
    //        draw_rect(item.x, item.y, info.w, info.h, item.color);
    //        break;
    //    case draw_list_item::rect::FILLED:
    //        draw_filled_rect(item.x, item.y, info.w, info.h, item.color);
    //        break;
    //    case draw_list_item::rect::OUTLINED:
    //        draw_outlined_rect(item.x, item.y, info.w, info.h, item.color);
    //        break;
    //    };
    //} break;
    //case 2: {
    //    const auto &info = std::get<2>(item.params);
    //    draw_string(std::get<std::string>(item.params), item.x, item.y, item.color);
    //}
    //}
}



//void renderer::draw_string(std::string_view str, int x, int y, D3DCOLOR color)
//{
//    RECT rect;
//    SetRect(&rect, x, y, m_resolution.X, m_resolution.Y);
//
//    m_font->DrawTextA(nullptr, str.data(), -1, &rect, DT_LEFT | DT_NOCLIP | DT_SINGLELINE, color);
//}
//
//void renderer::draw_line(int x0, int y0, int x1, int y1, D3DCOLOR color)
//{
//    D3DXVECTOR2 lines[2] =
//    {
//        D3DXVECTOR2(x0, y0),
//        D3DXVECTOR2(x1, y1)
//    };
//
//    m_line->Begin();
//    m_line->Draw(lines, 2, color);
//    m_line->End();
//}
//
//void renderer::draw_rect(int x, int y, int w, int h, D3DCOLOR color)
//{
//    draw_line(x, y, x + w, y, color);
//    draw_line(x, y, x, y + h, color);
//    draw_line(x + w, y, x + w, y + h, color);
//    draw_line(x, y + h, x + w + 1, y + h, color);
//}
//
//void renderer::draw_filled_rect(int x, int y, int w, int h, D3DCOLOR color)
//{
//    D3DTLVERTEX qV[4] =
//    {
//        { float(x), float(y + h), 0.f, 1.f, color },
//        { float(x), float(y), 0.f, 1.f, color },
//        { float(x + w), float(y + h), 0.f, 1.f, color },
//        { float(x + w), float(y) , 0.f, 1.f, color }
//    };
//
//    m_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
//    m_d3d_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
//    m_d3d_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
//    m_d3d_device->SetTexture(0, nullptr);
//    m_d3d_device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, qV, sizeof(D3DTLVERTEX));
//}
//
//void renderer::draw_outlined_rect(int x, int y, int w, int h, D3DCOLOR color)
//{
//    draw_rect(x - 1, y - 1, w + 2, h + 2, D3DCOLOR_RGBA(1, 1, 1, 255));
//    draw_rect(x + 1, y + 1, w - 2, h - 2, D3DCOLOR_RGBA(1, 1, 1, 255));
//    draw_rect(x, y, w, h, color);
//}