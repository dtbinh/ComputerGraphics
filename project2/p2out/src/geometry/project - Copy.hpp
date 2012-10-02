/**
 * @file project.hpp
 * @brief Geometry project
 *
 * @author H. Q. Bovik (hqbovik)
 * @bug Unimplemented
 */

#ifndef _462_GEOMETRY_PROJECT_HPP_
#define _462_GEOMETRY_PROJECT_HPP_

#include "math/camera.hpp"


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
	// array of edges
	unsigned int edges[3];
};

struct Vertex
{
    // the position of the vertex
    Vector3 position;
    // the normal of the vertex
    Vector3 normal;
    // the texture coordinate of the vertex
    Vector2 texture_coord;
	// the adjacent triangles
	unsigned int *adjTriangles;
	// the number of adjacent triangles
	int num_adjTriangles;
	// the length of the array
	int arrayLenofAdjTriangles;
	// boundary or interior
	bool boundary;
};

struct Edge
{
	//index into vertex list of the 2 vertices of this edge
	unsigned int vertices[2];
	//array of triangles which contain this edge
	int containingTriangels[2];
	//array of whether the index in containingTriangels is old index or the new one
	//if old, false; else, true;
	bool updateIndex[2];
	//number of containing triangles 
	int num_ctnTris;
	//check if it has been calculated before
	bool calculated;
	// the previous calculation result in vertice array
	int zIndex;
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

    // TODO add any other private members/functions here.
	//mesh display list
	int meshList;
	//the number of odd vertex;
	int num_oddVertex;
	// edge array
	Edge *edges, *new_edge;
	// vertice array
	Vertex *newVerticeArr;
	// old triangle array, new triangle array
	Triangle *oldTriangleArr,*subdivision_triangleArr;
	//the current length of edge array
	int edgeArrLength;
	//the current length of new edge array
	int newEdgeArrLength;
	//the number of edges
	int num_edges;
	//the num of new edges
	int num_new_edges;
	//the length of new vertice arr and new triangle arr
	int num_vertex, num_triangles, newVertexIndex, newTriIndex, currentTriLen;
	//check if it is the first subdivision
	bool firstSub;
	//compare two triangles to see if they are adjacent
	//bool trianglesAdjacency(Triangle &a, Triangle &b);
	//add some triangle to the vertice adjacent triangle arrays and also make sure there is no redundence.
	void addAdjTriangle(unsigned int vertexIndex, unsigned int triangleIndex);
	//tell if this edge already exist
	int edgeExist(unsigned int index1, unsigned int index2, Edge *pointer, int length);
	//add edge to the array after judging it is not already exist, if over the boundary, double the array length
	//void addEdge(unsigned int index1, unsigned int index2, int triIndex, int index, Edge *pointer, int length, int currentIndex);
	void addEdge(unsigned int index1, unsigned int index2, int triIndex, int index);
	//get the third vertex of a triangle
	int getThirdVertex(int edgeIndex, int triIndex);
	//check if over writing one array; if over the boundary, double the length
	template<typename T>
	T* checkBoundary(T *pointer, int &length, int &currentNum);
	//the display list function
	void setDisplayList();
	//find the position of v 
	int findZ(int edgeIndex);
	//form new triangles
	void formNewTriangle(int oldIndex1, int oldIndex2, int oldIndex3, int newIndex1, int newIndex2, int newIndex3);
	//update edges after finding new vertex
	void setNewEdge(int index1, int index2, int triIndex, int index);
    // since this has no meaningful assignment/copy, prevent the compiler from
    // automatically generating those functions
    GeometryProject( const GeometryProject& );
    GeometryProject& operator=( const GeometryProject& );
};

} /* _462 */

#endif /* _462_GEOMETRY_PROJECT_HPP_ */

