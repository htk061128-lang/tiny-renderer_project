#include <cstdlib>
#include "our_gl.h"
#include "model.h"

extern mat<4,4> ModelView, Perspective; // "OpenGL" state matrices and
extern std::vector<double> zbuffer;     // the depth buffer

struct RandomShader : IShader {
    const Model &model; //이 변수들은 이 구조체 밖에서 넣어줌. 이걸 가져다 쓰면 됨.
    TGAColor color = {};
    vec3 tri[3];  // triangle in eye coordinates
    vec3 light_dir;
    TGAColor base_color = {};
    int ambient = 30;

    RandomShader(const Model &m) : model(m) {
    }

    virtual vec4 vertex(const int face, const int vert) {
        vec3 v = model.vert(face, vert);                          // current vertex in object coordinates
        vec4 gl_Position = ModelView * vec4{v.x, v.y, v.z, 1.};
        tri[vert] = gl_Position.xyz();                            // in eye coordinates
        return Perspective * gl_Position;                         // in clip coordinates
    }

    virtual std::pair<bool,TGAColor> fragment(const vec3 bar) const {
        // 1. 삼각형 두 변의 외적으로 법선 벡터(Normal) 계산
        // tinyrenderer 버전에 따라 cross() 또는 ^ 연산자 사용
        vec3 n = normalized(cross(tri[1] - tri[0], tri[2] - tri[0]));

        // 2. 광원 방향과 내적 (밝기 계산)
        double intensity = std::max(0.0, n * light_dir);

        // 3. TGAColor는 곱셈 연산자가 없으므로 채널(R, G, B)별로 계산
        TGAColor c = base_color;
        for (int i = 0; i < 3; i++) {
            c[i] = static_cast<std::uint8_t>((base_color[i] * intensity) + ambient);
        }
        return {false, c};
    }
};

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
        return 1;
    }

    constexpr int width  = 800;      // output image size
    constexpr int height = 800;
    constexpr vec3    eye{-1, 0, 2}; // camera position
    constexpr vec3 center{ 0, 0, 0}; // camera direction
    constexpr vec3     up{ 0, 1, 0}; // camera up vector

    lookat(eye, center, up);                                   // build the ModelView   matrix
    init_perspective(norm(eye-center));                        // build the Perspective matrix
    init_viewport(width/16, height/16, width*7/8, height*7/8); // build the Viewport    matrix
    init_zbuffer(width, height);
    TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});

    for (int m=1; m<argc; m++) {                    // iterate through all input objects
        Model model(argv[m]);                       // load the data
        RandomShader shader(model);
        for (int f=0; f<model.nfaces(); f++) {      // iterate through all facets
            shader.base_color = { 100, 100, 100, 100 };
            shader.light_dir = {0, 0, 1}; //vertex 쉐이더 이후이므로 카메라가 (0, 0, d)에 있는 좌표계 기준 벡터임. 
            Triangle clip = { shader.vertex(f, 0),  // assemble the primitive
                              shader.vertex(f, 1),
                              shader.vertex(f, 2) };
            rasterize(clip, shader, framebuffer);   // rasterize the primitive
        }
    }

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}