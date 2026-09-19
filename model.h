#include "geometry.h"
#include "tgaimage.h"

class Model {
    std::vector<vec4> verts = {};    // array of vertices        ┐ generally speaking, these arrays
    std::vector<vec4> norms = {};    // array of normal vectors  │ do not have the same size
    std::vector<vec2> tex = {};      // array of tex coords      ┘ check the logs of the Model() constructor
    std::vector<int> facet_vrt = {}; //  ┐ 정점들 인덱스 배열. [0], [1], [2]가 하나의 삼각형이고 [3], [4], [5]의 인덱스를 가지는 정점들이 하나의 삼각형임.
    std::vector<int> facet_nrm = {}; //  │ the size is supposed to be
    std::vector<int> facet_tex = {}; //  ┘ nfaces()*3
    TGAImage diffusemap  = {};       // diffuse color texture
    TGAImage normalmap   = {};       // normal map texture
    TGAImage specularmap = {};       // specular texture -> 여기 데이터들은 private라서 Model model(argv[1]); model.verts[5].x 이렇게 접근이 불가능함. 그래서 밑의 함수들로 접근해야 함. 
public:
    Model(const std::string filename);
    int nverts() const; // number of vertices
    int nfaces() const; // number of triangles
    vec4 vert(const int i) const;                          // 0 <= i < nverts()
    vec4 vert(const int iface, const int nthvert) const;   // 0 <= iface <= nfaces(), 0 <= nthvert < 3
    vec4 normal(const int iface, const int nthvert) const; // normal coming from the "vn x y z" entries in the .obj file
    vec4 normal(const vec2 &uv) const;                     // normal vector from the normal map texture
    vec2 uv(const int iface, const int nthvert) const;     // uv coordinates of triangle corners
    const TGAImage& diffuse() const;
    const TGAImage& specular() const;

};

