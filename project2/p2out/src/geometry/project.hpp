/**
 * @file project.hpp
 * @brief Geometry project
 *
 * @author Ying Yuan
 * @bug Unimplemented
 */

#ifndef _462_GEOMETRY_PROJECT_HPP_
#define _462_GEOMETRY_PROJECT_HPP_

#include "math/camera.hpp"
#include "application/imageio.hpp"
#include "application/opengl.hpp"

/*
   A namespace declaration. All project files use this namespace.
   Add this declaration (and its closing) to all source/headers you create.
   Note that all #includes should be BEFORE the namespace declaration.
 */
namespace _462 {

struct Triangle
{
    // index into vertex list of the 3 vertices of this triangle
    unsigned int vertices[3];
};

struct Vertex
{
    // the position of the vertex
    Vector3 position;
    // the normal of the vertex
    Vector3 normal;
    // the texture coordinate of the vertex
    Vector2 texture_coord;
};

struct AdjVertice
{
	unsigned int adjArray[15];
	int num_AdjVertice;
	bool boundary;
};

struct MeshData
{
    // array of vertices
    Vertex* vertices;
    // size of vertex array
    size_t num_vertices;

    // array of triangles
    Triangle* triangles;
    // size of triangle array
    size_t num_triangles;
};

class GeometryProject
{
public:

    // constructor, invoked when object is created
    GeometryProject();
    // destructor, invoked when object is destroyed
    ~GeometryProject();

    // more detailed specifications for each function are in project.cpp.

    // Initialize the project, loading the mesh from the given filename.
    // Returns true on success.
    bool initialize( const Camera* camera, const MeshData* mesh, const char* texture_filename );
    // Clean up the project, free any memory, etc.
    void destroy();
    // Render the mesh using the given camera.
    void render( const Camera* camera );
    // Subdivide the mesh
    void subdivide();


private:

    MeshData mesh;
	int meshList;
	Triangle *oldTriangleArr, *newTriangleArr;
	Vertex *oldVerticeArr, *newVerticeArr;
	AdjVertice *oldAdjVerticeArr, *newAdjVerticeArr;
	
	int num_triangles, triangle_index;
	int num_vertice, vertice_index, num_oddVertice;
	bool firstSubdivision;
	bool hasTexture;
    // TODO add any other private members/functions here.
	void setDisplayList();
	void findAdjVertice(unsigned int vertexInt, unsigned int index1, unsigned int index2, AdjVertice *pointer);
	int createNewPoint(unsigned int index1, unsigned int index2);
	void formNewTriangle(int oldIndex1, int oldIndex2, int oldIndex3, int newIndex1, int newIndex2, int newIndex3);
	void debugAdjVertice();
	unsigned char *textureArr;
	int textureWidth, textureHeight;
	GLuint texture;
    // since this has no meaningful assignment/copy, prevent the compiler from
    // automatically generating those functions
    GeometryProject( const GeometryProject& );
    GeometryProject& operator=( const GeometryProject& );
};

} /* _462 */

#endif /* _462_GEOMETRY_PROJECT_HPP_ */

