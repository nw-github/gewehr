#pragma once
#include <d3d11.h>
#pragma comment(lib, "D3D11.lib")

struct Color
{
    BYTE r, g, b, a;

    Color()
        : r{0}, g{0}, b{0}, a{255}
    { }
    Color(BYTE r, BYTE g, BYTE b, BYTE a = 255)
        : r{r}, g{g}, b{b}, a{a}
    { }
};

struct DrawListItem
{
    struct Line
    {
        int x2;
        int y2;
    };

    struct Rect
    {
        int w;
        int h;
        enum Style {
            NORMAL,
            FILLED,
            OUTLINED
        } type;
    };

    int x;
    int y;
    Color color;
    std::variant<Line, Rect, std::string> params;
};

class Renderer
{
public:
    inline const POINT &get_position() const
    {
        return m_position;
    }

    inline const POINT &get_resolution() const
    {
        return m_resolution;
    }

    void thread_proc(std::stop_token token);

    void add_line(int x0, int y0, int x1, int y1, Color color);
    void add_rect(int x, int y, int w, int h, Color color, DrawListItem::Rect::Style style = DrawListItem::Rect::NORMAL);
    void add_string(std::string_view str, int x, int y, Color color);

private:
    static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool initialize_dx();
    bool initialize(const std::string &class_name, const std::string &wnd_name, bool topmost);
    void shutdown();
    void render();

    void draw_item(const DrawListItem &item);
    //void draw_line(int x0, int y0, int x1, int y1, D3DCOLOR color);
    //void draw_rect(int x, int y, int w, int h, D3DCOLOR color);
    //void draw_filled_rect(int x, int y, int w, int h, D3DCOLOR color);
    //void draw_outlined_rect(int x, int y, int w, int h, D3DCOLOR color);
    //void draw_string(std::string_view str, int x, int y, D3DCOLOR color);

private:
    POINT                     m_position{};
    POINT                     m_resolution{};
    HWND                      m_hwnd{nullptr};
    ID3D11Device             *m_device{nullptr};
    ID3D11DeviceContext      *m_device_ctx{nullptr};
    IDXGISwapChain           *m_swapchain{nullptr};
    ID3D11RenderTargetView   *m_render_target_view{nullptr};
    std::mutex                m_draw_list_mutex;
    std::queue<DrawListItem>  m_draw_list;

};

namespace g
{
    inline std::unique_ptr<::Renderer> render = std::make_unique<::Renderer>();
}