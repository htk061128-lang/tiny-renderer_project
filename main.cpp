
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

//처음에 시작하면 cd tinyrenderer로 파일안으로 들어와야 함. 
//코드를 실행후 생성된 framebuffer.tga를 framebuffer.png로 변환하는것 까지 한번에 하는 명령어.
//다른 bash 창에서 명령어: python3 -m http.server 8000 을 실행하고 거기서 png파일 확인 가능.
//명령어: cmake --build build && build/tinyrenderer && convert framebuffer.tga framebuffer.png

//이미지에서 x, y좌표는 이미지의 왼쪽 아래 픽셀이 0, 0으로 설정되어 있음.

int main(int argc, char** argv) {
    constexpr int width  = 512;
    constexpr int height = 512;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax =  7, ay =  3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53;

    float t;
    float slope_x = bx - ax;
    float slope_y = by - ay;
    float slope_x_2 = cx - ax;
    float slope_y_2 = cy - ay;

    for(t = 0; t < 1; t = t + 0.02)
    {
        framebuffer.set(ax + t*(slope_x), ay + t*(slope_y), white);
        framebuffer.set(ax + t*(slope_x_2), ay + t*(slope_y_2), blue);
    }

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}