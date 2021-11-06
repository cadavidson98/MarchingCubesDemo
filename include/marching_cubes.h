#include <glm/glm.hpp>

#include <vector>
#include <unordered_map>

using namespace std;

/**
 * @struct A 3D Point
 */
struct Point {
    Point(float _x = 0.f, float _y = 0.f, float _z = 0.f) :
    x(_x), y(_y), z(_z) {

    };
    
    float x;
    float y;
    float z;
};

/**
 * @struct A Triangle consisting of 3 points and no normal vector(s)
 * 
 */
struct Triangle {
    Point p1;
    Point p2;
    Point p3;
};

/**
 * @struct A gridcell used for Polygonizing in MarchingCubes::Polygonize 
 * 
 */
struct GridCell {
    Point bounds[8];
};

/**
 * @struct A vertex for rendering triangles in OpenGL. Contains a position and normal vector
 * 
 */
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

class MarchingCubes {
public:
    MarchingCubes(Point min_pt, Point max_pt, float resolution);

    void Polygonize(vector<Vertex> &triangle_verts);
private:
    int num_x_steps;
    int num_y_steps;
    int num_z_steps;
    float resolution;
    float iso_value;
    Point start_pt;

    void PolygonizeCube(GridCell cube, vector<Vertex> &triangle_verts);
    float SphereFuntion(float x, float y, float z);
};