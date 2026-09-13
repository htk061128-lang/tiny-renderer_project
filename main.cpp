#include <cmath> //std::round() 반올림, std::abs() 절댓값.
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
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

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color) {
    bool steep = std::abs(ax-bx) < std::abs(ay-by); //기울기가 1보다 큰가.
    if (steep) { // if the line is steep, we transpose the image
        std::swap(ax, ay);
        std::swap(bx, by);
    }
    if (ax>bx) { // make it left−to−right
        std::swap(ax, bx);
        std::swap(ay, by);
    }
    float y = ay;
    float slope = (by-ay) / static_cast<float>(bx-ax);
    for (int x=ax; x<=bx; x++) {
        if (steep) // if transposed, de−transpose
            framebuffer.set(y, x, color);
        else
            framebuffer.set(x, y, color);
        y += slope;
    }
}

void Draw_Triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color)
{
    line(ax, ay, bx, by, framebuffer, color);
    line(bx, by, cx, cy, framebuffer, color);
    line(ax, ay, cx, cy, framebuffer, color);
}

//std::ifstream file("diablo3_pose.obj");
//std::getline(file, line); 파일에서 줄바꿈(\n) 전까지 한 줄 전체를 문자열로 통째로 읽어 std::string 변수에 담아주는 함수입니다.

struct Point {
    float x, y, z;
};

struct Triangle {
    int v1, v2, v3;
};

struct Dot {
    int x_screen, y_screen;
};

struct Polygon {
    int x1, y1, x2, y2, x3, y3;
};

int main(int argc, char** argv) {
    constexpr int width  = 1024;
    constexpr int height = 1024;
    TGAImage framebuffer(width, height, TGAImage::RGB);

    std::ifstream my_file("diablo3_pose.obj"); //wavefront obj 파일 열기. input file stream
    if (!my_file.is_open()) {
        std::cerr << "파일을 열 수 없습니다!" << std::endl;
        return -1;
    }

    std::vector<Point> file_vector; //file_vector.x, file_vector.y, file_vector.z로 접근.
    std::vector<Triangle> file_tri; //각 폴리곤의 세 정점 인덱스를 모아서 접근.

    std::vector<Polygon> polygon_position; //세개의 정점의 좌표를 변환 후 모아서 저장하는 vector배열.
    std::vector<Dot> dotdot; //한 점의 변환후 좌표 저장.
    
    std::string line_data;

    while (std::getline(my_file, line_data)) {
        std::stringstream ss(line_data); //line_data는 문자열인데 그걸 공백기준으로 알아서 끊고 형변환 까지 해줌. >> 연산자로 하나씩 꺼낼 수 있음.
        std::string prefix;
        ss >> prefix;

        // 정점 데이터('v')인 줄만 파싱
        if (prefix == "v") {
            Point p;
            if (ss >> p.x >> p.y >> p.z) {
                file_vector.push_back(p);
            }
        }

        if (prefix == "f") {
            int vt, vn;
            char slash1, slash2;
            Triangle t; //여기에 정점 인덱스 3개 저장하고 file_tri에 push함.

            // "194" >> '/' >> "119" >> '/' >> "194" 순서대로 쏙쏙 뽑아냄
            ss >> t.v1 >> slash1 >> vt >> slash2 >> vn;
            ss >> t.v2 >> slash1 >> vt >> slash2 >> vn;
            ss >> t.v3 >> slash1 >> vt >> slash2 >> vn;

            // OBJ 인덱스는 1부터 시작하므로 0-based 배열에 맞추려면 -1 필요
            t.v1--;
            t.v2--;
            t.v3--;
            file_tri.push_back(t);
        }
    }
    my_file.close(); //file_vector와 file_tri로 파싱 완료. 

    // 저장된 크기만큼만 안전하게 출력
    size_t print_count1 = file_vector.size();
    for (size_t i = 0; i < print_count1; i++) {
        //printf("%f, %f, %f\n", file_vector[i].x, file_vector[i].y, file_vector[i].z);
        Dot a;
        int pixel_y = static_cast<int>((file_vector[i].y+1.0f) * 0.5f * (height - 1) + 0.5f); //-1 ~ 1까지의 소수를 0 ~ (heith - 1)까지의 숫자로 변환.
        int pixel_x = static_cast<int>((file_vector[i].x+1.0f) * 0.5f * (width - 1) + 0.5f); // 반올림 적용
        a.x_screen = pixel_x;
        a.y_screen = pixel_y;
        dotdot.push_back(a);

    }

    size_t print_count2 = file_tri.size();
    for (size_t i = 0; i < print_count2; i++) {
        //printf("%d, %d, %d\n", file_tri[i].v1, file_tri[i].v2, file_tri[i].v3);
        Polygon a;
        a.x1 = dotdot[file_tri[i].v1].x_screen;
        a.y1 = dotdot[file_tri[i].v1].y_screen;
        a.x2 = dotdot[file_tri[i].v2].x_screen;
        a.y2 = dotdot[file_tri[i].v2].y_screen;
        a.x3 = dotdot[file_tri[i].v3].x_screen;
        a.y3 = dotdot[file_tri[i].v3].y_screen;
        polygon_position.push_back(a);
    }

    size_t polygon_count3 = polygon_position.size();
    for(int i = 0; i < polygon_count3; i++)
    {
        Draw_Triangle(polygon_position[i].x1, polygon_position[i].y1, polygon_position[i].x2, polygon_position[i].y2, polygon_position[i].x3, polygon_position[i].y3, framebuffer, red);
    }




    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}