#include <cmath>
#include <tuple>
#include <algorithm>
#include "geometry.h"
#include "model.h"
#include "tgaimage.h"

/*
argc (Argument Count): 프로그램으로 전달된 인자의 총 개수 (정수형 int)
argv (Argument Vector): 전달된 문자열들의 배열 (char** 또는 char*[])

argv[0]: 실행된 프로그램의 실행 파일 이름/경로
argv[1]: 첫 번째로 넘겨준 실제 인자 값
*/

//처음에 시작하면 cd tinyrenderer로 파일안으로 들어와야 함. 
//코드를 실행후 생성된 framebuffer.tga를 framebuffer.png로 변환하는것 까지 한번에 하는 명령어.
//다른 bash 창에서 명령어: python3 -m http.server 8000 을 실행하고 거기서 png파일 확인 가능.
//명령어: cmake --build build && build/tinyrenderer && convert framebuffer.tga framebuffer.png
//cmake --build build && build/tinyrenderer diablo3_pose.obj && convert framebuffer.tga framebuffer.png

constexpr int width  = 800;
constexpr int height = 800;
constexpr int depth = 255; //8비트 기준.

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by);
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    int y = ay;
    int ierror = 0;
    for (int x=ax; x<=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        ierror += 2 * std::abs(by-ay);
        if (ierror > bx - ax) {
            y += by > ay ? 1 : -1;
            ierror -= 2 * (bx-ax);
        }
    }
}

std::tuple<int,int,int> project(vec4 v) { // First of all, (x,y) is an orthogonal projection of the vector (x,y,z).
    return { (v.x + 1.) *  width/2,   // Second, since the input models are scaled to have fit in the [-1,1]^3 world coordinates,
             (v.y + 1.) * height/2, (v.z + 1.0) * depth/2 }; // we want to shift the vector (x,y) and then scale it to span the entire screen.
}

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy) { //삼각형 ABC의 넓이를 계산하는 함수. 정점들이 A -> B -> C 가 반시계 방향이야 양수로 넓이가 나옴.
    return .5*((by-ay)*(bx+ax) + (cy-by)*(cx+bx) + (ay-cy)*(ax+cx));
}

void triangle(int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, TGAImage &zbuffer, TGAImage &framebuffer) {
    int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle
    int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
    int bbmaxx = std::max(std::max(ax, bx), cx);
    int bbmaxy = std::max(std::max(ay, by), cy);
    double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
    if (total_area<1) return; // backface culling + discarding triangles that cover less than a pixel

#pragma omp parallel for
    for (int x=bbminx; x<=bbmaxx; x++) {
        for (int y=bbminy; y<=bbmaxy; y++) {
            double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
            double beta  = signed_triangle_area(x, y, cx, cy, ax, ay) / total_area;
            double gamma = signed_triangle_area(x, y, ax, ay, bx, by) / total_area;
            if (alpha<0 || beta<0 || gamma<0) continue; // negative barycentric coordinate => the pixel is outside the triangle
            unsigned char z = static_cast<unsigned char>(alpha * az + beta * bz + gamma * cz);
            if(z <= zbuffer.get(x, y)[0]) continue; //z값은 카메라에서 가까우면 가까울수록 더 커짐.
            // 2. 0.0 ~ 1.0 비율로 정규화 (0~255 범위 기준)
            double t = std::clamp(z / 255.0, 0.0, 1.0);

            // 3. Z값이 클수록 빨간색에 가까워 짐. 
            unsigned char r = static_cast<unsigned char>(t * 255.0);
            unsigned char g = 0;
            unsigned char b = static_cast<unsigned char>((1.0 - t) * 255.0);
            zbuffer.set(x, y, {z, 0, 0, 0});
            framebuffer.set(x, y, {b, g, r, 255});
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    Model model(argv[1]);
    TGAImage framebuffer(width, height, TGAImage::RGB);
    TGAImage zbuffer(width, height, TGAImage::GRAYSCALE);

    for (int i=0; i<model.nfaces(); i++) { // iterate through all triangles
        auto [ax, ay, az] = project(model.vert(i, 0));
        auto [bx, by, bz] = project(model.vert(i, 1));
        auto [cx, cy, cz] = project(model.vert(i, 2));
        triangle(ax, ay, az, bx, by, bz, cx, cy, cz, zbuffer, framebuffer);
    }
    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}