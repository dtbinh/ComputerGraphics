/**
 * @file mesh.hpp
 * @brief Mesh class and OBJ loader.
 *
 * @author Eric Butler (edbutler)
 * @author Zeyang Li (zeyangl)
 */

#ifndef _462_SCENE_MESH_HPP_
#define _462_SCENE_MESH_HPP_

#include "math/vector.hpp"

#include <vector>
#include <cassert>

namespace _462 {

struct MeshVertex
{
    Vector3 position;
};

struct MeshTriangle
{
    // index into the vertex list of the 3 vertices
    unsigned int vertices[3];
};

/**
 * A mesh of triangles.
 */
class Mesh
{
public:

    Mesh();
    virtual ~Mesh();

    typedef std::vector< MeshTriangle > MeshTriangleList;
    typedef std::vector< MeshVertex > MeshVertexList;

    // The list of all triangles in this model.
    MeshTriangleList triangles;

    // The list of all vertices in this model.
    MeshVertexList vertices;

    // HACK scene loader stores the filename of the mesh here
    std::string filename;

    /**
     * Loads the model into a list of triangles and vertices.
     */
    bool load();

private:

    // prevent copy/assignment
    Mesh( const Mesh& );
    Mesh& operator=( const Mesh& );
};


} /* _462 */

#endif /* _462_SCENE_MESH_HPP_ */

