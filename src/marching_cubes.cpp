#include "marching_cubes.h"
#include "lookup_tables.h"

/**
 * @brief Initialize the MarchingCubes function. Marching cubes will
 * polygonize all cells in the region from min_pt to max_pt.
 * 
 * @param min_pt the minimum x,y,z values to search
 * @param max_pt the maximum x,y,z values to search
 * @param _resolution The number of gridcells in the x, y, z directions
 */
MarchingCubes::MarchingCubes(Point min_pt, Point max_pt, float _resolution) {
    num_x_steps = ceil((max_pt.x - min_pt.x) / _resolution);
    num_y_steps = ceil((max_pt.y - min_pt.y) / _resolution);
    num_z_steps = ceil((max_pt.z - min_pt.z) / _resolution);
    start_pt = min_pt;
    resolution = _resolution;
    iso_value = 0.f;
}

/**
 * @brief Traverse through the entire region and polygonize the unit sphere function
 * 
 * @param triangle_verts vector to store the surface model in
 */
void MarchingCubes::Polygonize(vector<Vertex> &triangle_verts) {
    // Marching through the Z direction
    for(int z = 0; z < num_z_steps; ++z) {
        // Marching through the Y direction
        for(int y = 0; y < num_y_steps; ++y) {
            // Marching through the X direction
            for(int x = 0; x < num_x_steps; ++x) {
                // Create the Cube - set the location of each corner so we can polygonize the cell
                GridCell cube;
                cube.bounds[0] = Point(start_pt.x + resolution*x, start_pt.y + resolution*y, start_pt.z + resolution*z);
                cube.bounds[1] = Point(start_pt.x + resolution*(x+1), start_pt.y + resolution*y, start_pt.z + resolution*z);
                cube.bounds[2] = Point(start_pt.x + resolution*(x+1), start_pt.y + resolution*(y+1), start_pt.z + resolution*z);
                cube.bounds[3] = Point(start_pt.x + resolution*x, start_pt.y + resolution*(y+1), start_pt.z + resolution*z);
                
                cube.bounds[4] = Point(start_pt.x + resolution*x, start_pt.y + resolution*y, start_pt.z + resolution*(z+1));
                cube.bounds[5] = Point(start_pt.x + resolution*(x+1), start_pt.y + resolution*y, start_pt.z + resolution*(z+1));                
                cube.bounds[6] = Point(start_pt.x + resolution*(x+1), start_pt.y + resolution*(y+1), start_pt.z + resolution*(z+1));
                cube.bounds[7] = Point(start_pt.x + resolution*x, start_pt.y + resolution*(y+1), start_pt.z + resolution*(z+1));
                // Now find the triangles for this grid cell
                PolygonizeCube(cube, triangle_verts);
            }
        }
    }
}

/**
 * @brief Check if there are any surface triangles in this gridcell
 * 
 * @param cube gridcell to check for triangles
 * @param triangle_verts vector to store the generated triangles in
 */
void MarchingCubes::PolygonizeCube(GridCell cube, vector<Vertex> &triangle_verts) {
    // Find the value at each corner in the gridcell
    float values[8];
    values[0] = SphereFuntion(cube.bounds[0].x, cube.bounds[0].y, cube.bounds[0].z);  // (x,y,z)
    values[1] = SphereFuntion(cube.bounds[1].x, cube.bounds[1].y, cube.bounds[1].z);  // (x+resolution, y, z)
    values[2] = SphereFuntion(cube.bounds[2].x, cube.bounds[2].y, cube.bounds[2].z);  // (x, y+resolution, z)
    values[3] = SphereFuntion(cube.bounds[3].x, cube.bounds[3].y, cube.bounds[3].z);  // (x+resolution, y+resolution, z)
    values[4] = SphereFuntion(cube.bounds[4].x, cube.bounds[4].y, cube.bounds[4].z);  // (x,y,z+resolution)
    values[5] = SphereFuntion(cube.bounds[5].x, cube.bounds[5].y, cube.bounds[5].z);  // (x+resolution, y, z+resolution)
    values[6] = SphereFuntion(cube.bounds[6].x, cube.bounds[6].y, cube.bounds[6].z);  // (x, y+resolution, z+resolution)
    values[7] = SphereFuntion(cube.bounds[7].x, cube.bounds[7].y, cube.bounds[7].z);  // (x+resolution, y+resolution, z+resolution)

    // Now check whether each cube corner is inside or outside the scalar field
    int edge_table_lookup = 0;
    for(int i = 0; i < 8; ++i) {
        if(values[i] < iso_value) {
            // If the value at this region is less than the iso value, we flag it as inside
            edge_table_lookup |= 0x01 << i;
        }
    }
    
    // There are 256 possible triangulations of the gridcell, but we know there are 2 cases
    // where we don't have any triangles:
    // 1) All the grid points are in the surface
    // 2) All the grid points are outside the surface
    // In these cases, there is no point in continuing, so we can return early
    if (edge_table[edge_table_lookup] == 0) {
        // this cube is completely inside or outside the surface
        return;
    }

    // This table defines the edges of the gridcell - so {0,1} connects
    // grid points 0 and 1, {1, 2} connects 1 and 2, etc.
    int edges[12][2] = {{0, 1}, {1, 2}, {2, 3}, {3, 0},
                        {4, 5}, {5, 6}, {6, 7}, {7, 4},
                        {0, 4}, {1, 5}, {2, 6}, {3, 7}};
    Point vertex_edges[12];
    // Right now, we only know that some gridcells are inside the surface
    // and some are outside, but we don't know WHERE the surface actually
    // is. So next we are going to find exact location of the surface along
    // each edge of the gridcell by using linear interpolation
    // check for intersections along the iso surface and the edge
    for(int i = 0; i < 12; ++i) {
        if(edge_table[edge_table_lookup] & (0x01 << i)) {
            // linearly interpolate to find the vertex point along the edge
            int index_1 = edges[i][0];
            int index_2 = edges[i][1];
            float delta = values[index_2] - values[index_1];
            float lerp = (iso_value - values[index_1]) / delta;  
            vertex_edges[i].x = cube.bounds[index_1].x + lerp*(cube.bounds[index_2].x - cube.bounds[index_1].x);
            vertex_edges[i].y = cube.bounds[index_1].y + lerp*(cube.bounds[index_2].y - cube.bounds[index_1].y);
            vertex_edges[i].z = cube.bounds[index_1].z + lerp*(cube.bounds[index_2].z - cube.bounds[index_1].z);
        }
    }

    // Finally, we can use the lookup table to construct the triangles
    // All we have to do is look up how to create triangles using the points
    // in vertex_edges
    // construct the triangles for this cell
    // A cell may have at most 5 triangles
    for(int i = 0; i < 5; ++i) {
        // Each triangle is defined by 3 indices; these indices specify an offset into vertex_edges
        int index_1 = tri_table[edge_table_lookup][3*i];
        int index_2 = tri_table[edge_table_lookup][3*i + 1];
        int index_3 = tri_table[edge_table_lookup][3*i + 2];
        // If one of the indices is invalid, then we have no more triangles to polygonize,
        // so end early
        if(index_1 == -1 || index_2 == -1 || index_3 == -1) {
            break;
        }
        Vertex vert_1, vert_2, vert_3;
        vert_1.position = glm::vec3(vertex_edges[index_1].x, vertex_edges[index_1].y, vertex_edges[index_1].z);
        vert_2.position = glm::vec3(vertex_edges[index_2].x, vertex_edges[index_2].y, vertex_edges[index_2].z);
        vert_3.position = glm::vec3(vertex_edges[index_3].x, vertex_edges[index_3].y, vertex_edges[index_3].z);
        // In order to light our model, we are going to want some normal vectors. Fortunately, the 3 triangles points
        // allow us to create a normal. All we have to do is create 2 vectors (1 from point 1 to point 2 and another from point 1 to point 3)
        // then cross them to find a normal.
        // find plane normal
        glm::vec3 to_p2 = vert_2.position - vert_1.position;
        glm::vec3 to_p3 = vert_3.position - vert_1.position;
        // Make sure it has a length of 1, otherwise the lighting will look weird
        glm::vec3 normal = glm::normalize(glm::cross(to_p2, to_p3));
        
        vert_1.normal = 
        vert_2.normal = 
        vert_3.normal = normal;

        triangle_verts.push_back(vert_1);
        triangle_verts.push_back(vert_2);
        triangle_verts.push_back(vert_3);
    }
}

/**
 * @brief A function that checks if point (x,y,z) is inside the unit sphere
 * 
 * @param x 
 * @param y 
 * @param z 
 * @return float. Return is > 0 if the point is outside the sphere, < 0 if it is inside
 * the sphere, and 0 if the point is exactly on the sphere surface
 */
float MarchingCubes::SphereFuntion(float x, float y, float z) {
    // ISO function to test marching cubes implementation
    return x*x + y*y + z*z - 1.f;
}