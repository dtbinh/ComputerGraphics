/**
 * @file project.cpp
 * @brief Geometry project
 *
 * @author H. Q. Bovik (hqbovik)
 * @bug Unimplemented
 */

#include "geometry/project.hpp"
#include "application/opengl.hpp"

/*
   A namespace declaration. All project files use this namespace.
   Add this declaration (and its closing) to all source/headers you create.
   Note that all #includes should be BEFORE the namespace declaration.
 */
namespace _462 {

// definitions of functions for the GeometryProject class

// constructor, invoked when object is allocated
GeometryProject::GeometryProject() { }

// destructor, invoked when object is de-allocated
GeometryProject::~GeometryProject() { }

void initLights()
{
    // set up light colors (ambient, diffuse, specular)
    GLfloat lightAmbient[] = {1.0f, 1.0f, 1.0f, 1.0f};  // ambient light
    GLfloat lightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};  // diffuse light
    GLfloat lightSpecular[] = {1, 1, 1, 1};           // specular light
	GLfloat lmodel_ambient[] = {0.2, 0.2, 0.2, 1.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // position the light
    float lightPos[4] = {0, 30, -10, 0}; // positional light
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);

    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration
	
	
	
}

/**
 * Initialize the project, doing any necessary opengl initialization.
 * @param camera An already-initialized camera.
 * @param mesh The mesh to be rendered and subdivided.
 * @param texture_filename The filename of the texture to use with the mesh.
 *  Is null if there is no texture data with the mesh or no texture filename
 *  was passed in the arguments, in which case textures should not be used.
 * @return true on success, false on error.
 */
bool GeometryProject::initialize( const Camera* camera, const MeshData* mesh, const char* texture_filename )
{
    this->mesh = *mesh;
	firstSub = true;
    // TODO opengl initialization code

	/************some initialization*****************/
	glClearColor(0.0,0.0,0.0,0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	/**************camera****************/
	glMatrixMode(GL_PROJECTION);
	gluPerspective(camera->get_fov_degrees(), camera->get_aspect_ratio(), camera->get_near_clip(), camera->get_far_clip());
	glMatrixMode(GL_MODELVIEW);
	

	initLights();
	/***************using the new array*****************/
	Vector3 tempPoint1, tempPoint2, tempPoint3, normalVec;
	int index1, index2, index3;
	newVerticeArr = new Vertex[mesh->num_vertices+1];
	oldTriangleArr = new Triangle[mesh->num_triangles+1];
	memcpy(newVerticeArr, mesh->vertices, mesh->num_vertices*sizeof(Vertex)+1);
	memcpy(oldTriangleArr, mesh->triangles, mesh->num_triangles*sizeof(Triangle)+1);
	num_triangles = mesh->num_triangles;
	num_vertex = mesh->num_vertices;
	
	
	
	/******************update display list*********************/
	setDisplayList();
	
	
	 
    return true;
}

/**
 * Clean up the project. Free any memory, etc.
 */
void GeometryProject::destroy()
{
  // TODO any cleanup code
	delete []newVerticeArr;
	delete []oldTriangleArr;
	delete []edges;
	delete []subdivision_triangleArr;
	delete []new_edge;
}

/**
 * Clear the screen, then render the mesh using the given camera.
 * @param camera The logical camera to use.
 * @see scene/camera.hpp
 */
void GeometryProject::render( const Camera* camera )
{
	// TODO render code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear Screen, Depth Buffer and Stencil Buffer
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
  
	Vector3	cam_pos = camera->get_position();
	Vector3 cam_ori = camera->get_direction();
	Vector3 cam_up = camera->get_up();
	gluLookAt(cam_pos.x,cam_pos.y,cam_pos.z,cam_pos.x+cam_ori.x,cam_pos.y+cam_ori.y,cam_pos.z+cam_ori.z,cam_up.x,cam_up.y,cam_up.z);


	/****** draw obj ********/
	glPushMatrix();
		glCallList(meshList);	
	glPopMatrix();
}


/**
 * Subdivide the mesh that we are rendering using Loop subdivison.
 */
void GeometryProject::subdivide()
{
    // TODO perform a single subdivision.
	unsigned int index1, index2, index3, index4;
	num_oddVertex = num_vertex;
	newVertexIndex = num_vertex;
	currentTriLen = num_triangles;
	newTriIndex = 0;
	subdivision_triangleArr = new Triangle[num_triangles *3];
	
	std::cout<<"\nvertex:"<<num_vertex<<"\n";
	std::cout<<"triangle:"<<num_triangles<<"\n";
	/*************for debug*************/
	/*for(int i = 0; i < num_vertex; i++){
		std::cout<<i<<":" <<newVerticeArr[i].position<<"\n";
	}

	for(int i = 0; i < num_triangles; i++){
		std::cout<<i<<":"<<oldTriangleArr[i].vertices[0]<<"+"<<oldTriangleArr[i].vertices[1]<<"+"<<oldTriangleArr[i].vertices[2]<<"\n";
	}

	for(int i = 0; i < num_edges; i++){
		std::cout<<i<<":"<<edges[i].vertices[0]<<"+"<<edges[i].vertices[1]<<"\n";
	}*/
	/************construct the data structure*****************/

	/******initialize the length of the array of adjacent triangles of each vertex***************/
	for(int i = 0; i < num_vertex; i++){
		newVerticeArr[i].num_adjTriangles = 0;
		newVerticeArr[i].arrayLenofAdjTriangles = 6;
		newVerticeArr[i].adjTriangles = new unsigned int[newVerticeArr[i].arrayLenofAdjTriangles];
		newVerticeArr[i].boundary = false;
	}
	
	/***********find the adjacent triangles of each vertex***********/
	std::cout<<"find the adj triangles of each vertex"<<"\n";
	for(int i = 0; i <num_triangles; i++){
		
		index1 = oldTriangleArr[i].vertices[0];
		index2 = oldTriangleArr[i].vertices[1];
		index3 = oldTriangleArr[i].vertices[2];
		
		addAdjTriangle(index1, i);
		addAdjTriangle(index2, i);
		addAdjTriangle(index3, i);

	}
	
	/**********Get all the edges*************/
	std::cout<<"Get all the edge"<<"\n";
	
//	if(firstSub){
		num_edges = 0;
		edgeArrLength = num_triangles*2;
		edges = new Edge[edgeArrLength];
		for(int i = 0; i < num_triangles; i++){
			index1 = oldTriangleArr[i].vertices[0];
			index2 = oldTriangleArr[i].vertices[1];
			index3 = oldTriangleArr[i].vertices[2];
			addEdge(index1, index2,i,0);
			addEdge(index2, index3,i,1);
			addEdge(index1, index3,i,2);
		
		}
//		firstSub = false;
//	}
		
	/*newEdgeArrLength = num_edges * 3;
	new_edge = new Edge[newEdgeArrLength];
	num_new_edges = 0;*/
	
	/************first pass*********************/
	/* create all the new points and triangles */
	/*******************************************/
	
	int triIndex1,triIndex2;
	std::cout<<"first pass"<<"\n";
	
	int newIndex1, newIndex2, newIndex3;
	for(int i = 0; i < num_triangles; i++){
		index1 = oldTriangleArr[i].edges[0];
		index2 = oldTriangleArr[i].edges[1];
		index3 = oldTriangleArr[i].edges[2];
		
		newIndex1 = findZ(index1);
		newIndex2 = findZ(index2);
		newIndex3 = findZ(index3);

		formNewTriangle(oldTriangleArr[i].vertices[0], oldTriangleArr[i].vertices[1], oldTriangleArr[i].vertices[2], newIndex1, newIndex2, newIndex3);


	}
	num_vertex = newVertexIndex;
	num_triangles = newTriIndex;

	
	/*************second pass****************/
	/**** refines all the old points ********/
	/****************************************/
	std::cout<<"second pass"<<"\n";
	
	double beta;
	int num_adjVertex, boundaryIndex;
	bool existed;
	unsigned int *adjVertex, tempIndex, tempBoundary[2];
	Vector3 newVertexPos;
	for(int i = 0; i < num_oddVertex; i++){
			/* find all the adjacent vertex*/
			num_adjVertex = 0;
			
			boundaryIndex = 0;
			adjVertex = new unsigned int[newVerticeArr[i].num_adjTriangles*2];
			for(int j = 0; j < newVerticeArr[i].num_adjTriangles; j++){
				for(int k = 0; k < 3; k++){
					existed = false;
					tempIndex = oldTriangleArr[newVerticeArr[i].adjTriangles[j]].vertices[k];
					if(tempIndex != i){ //check if it the same point
						for(int u = 0; u < num_adjVertex; u++){
							if(adjVertex[u] == tempIndex)
								existed = true;
						}
						if(!existed){
							adjVertex[num_adjVertex] = tempIndex;
							num_adjVertex++;
						}
					}
				}
			}
		if(!newVerticeArr[i].boundary){ //if it is a interior vertex
			/*calculate beta*/
			beta = (0.625 - pow((0.375 + (cos(2 * PI / num_adjVertex) / 4)),2)) / num_adjVertex;
			/*if(num_adjVertex > 3)
				beta = 3.0 / (8.0 * num_adjVertex);
			else if(num_adjVertex = 3)
				beta = 3.0 / 16.0;
			/*calculate v'*/
			newVertexPos = (1 - beta * num_adjVertex) * newVerticeArr[i].position;
			for(int j = 0; j < num_adjVertex; j++){
				newVertexPos += beta * newVerticeArr[adjVertex[j]].position;
			}
			newVerticeArr[i].position = newVertexPos;

		}else{ // if it is an boundary vertex
			//num_adjVertex--;
			for(int j = 0; j < num_adjVertex; j++){
				if(newVerticeArr[adjVertex[j]].boundary){
					tempBoundary[boundaryIndex] = adjVertex[j];
					boundaryIndex++;
				}
			}
			newVertexPos = 3 * newVerticeArr[i].position / 4 
				+ newVerticeArr[tempBoundary[0]].position / 8
				+ newVerticeArr[tempBoundary[1]].position / 8;
			newVerticeArr[i].position = newVertexPos;

		}
	}

	/**********************************************/
	oldTriangleArr = new Triangle[newTriIndex];
	memcpy(oldTriangleArr, subdivision_triangleArr, newTriIndex * sizeof(Triangle));

	


	Vector3 tempPoint1, tempPoint2, tempPoint3, normalVec;
	for(int i = 0; i < num_vertex; i++){
		newVerticeArr[i].normal = Vector3(0,0,0);
	}
	std::cout<<"computing normal vectors"<<"\n";
	
	/**********calculate normals**********************/
	for( int i = 0; i < num_triangles; i++){
		//std::cout<<"triangle nums:"<<i<<":"<<num_triangles<<"\n";
		index1 = oldTriangleArr[i].vertices[0];
		index2 = oldTriangleArr[i].vertices[1];
		index3 = oldTriangleArr[i].vertices[2];
		//std::cout<<"index123:"<<index1<<" + "<<index2<<" + "<<index3<<" total num:" << num_vertex<<"\n";
		tempPoint1 = newVerticeArr[index1].position;
		tempPoint2 = newVerticeArr[index2].position;
		tempPoint3 = newVerticeArr[index3].position;
		normalVec = cross((tempPoint2 - tempPoint1),(tempPoint3 - tempPoint1));
		
		newVerticeArr[index1].normal += normalVec;
		newVerticeArr[index2].normal += normalVec;
		newVerticeArr[index3].normal += normalVec;
	} 
	for( int i = 0; i < num_vertex; i++){
		newVerticeArr[i].normal = normalize(newVerticeArr[i].normal);
	}

	std::cout<<"call display list"<<"\n";
	
	setDisplayList();

	/*edges = new Edge[num_new_edges];
	num_edges = num_new_edges;
	memcpy(edges, new_edge, num_new_edges * sizeof(Edge));*/
}



//having an edge, find one/two containing triangles, get two/four vertice in total
//calculate the new vertex v
int GeometryProject::findZ(int edgeIndex){
	if(!edges[edgeIndex].calculated){
		unsigned int triIndex1, triIndex2, index1, index2, index3, index4;
		newVerticeArr = checkBoundary<Vertex>(newVerticeArr, num_vertex, newVertexIndex);
		Vector3 position;
		index1 = edges[edgeIndex].vertices[0];
		index2 = edges[edgeIndex].vertices[1];

		if(edges[edgeIndex].num_ctnTris == 1){ //boundary
			triIndex1 = edges[edgeIndex].containingTriangels[0];
			position = (newVerticeArr[index1].position + newVerticeArr[index2].position)/2;
			
			newVerticeArr[edges[edgeIndex].vertices[0]].boundary = true;
			newVerticeArr[edges[edgeIndex].vertices[1]].boundary = true;
		//	std::cout<<"boundary:"<<edges[edgeIndex].vertices[0]<<"+"<<edges[edgeIndex].vertices[1]<<"\n";
			
		}else if(edges[edgeIndex].num_ctnTris == 2){ //interior
			
			triIndex1 = edges[edgeIndex].containingTriangels[0];
			triIndex2 = edges[edgeIndex].containingTriangels[1];
			index3 = getThirdVertex(edgeIndex, triIndex1);
			index4 = getThirdVertex(edgeIndex, triIndex2);
			
			 position = (newVerticeArr[index1].position + newVerticeArr[index2].position)*3/8 
				+ (newVerticeArr[index3].position + newVerticeArr[index4].position)/8;
			
			
		}
		
		newVerticeArr[newVertexIndex].position = position;
		edges[edgeIndex].calculated = true;
		edges[edgeIndex].zIndex = newVertexIndex;
		newVertexIndex++;
		
	}
		return edges[edgeIndex].zIndex;
		
	
}

//form four new counter-clockwise triangles using six vertice
void GeometryProject::formNewTriangle(int oldIndex1, int oldIndex2, int oldIndex3, int newIndex1, int newIndex2, int newIndex3){
	subdivision_triangleArr = checkBoundary<Triangle>(subdivision_triangleArr, currentTriLen, newTriIndex);
	subdivision_triangleArr[newTriIndex].vertices[0] = oldIndex1;
	subdivision_triangleArr[newTriIndex].vertices[1] = newIndex1;
	subdivision_triangleArr[newTriIndex].vertices[2] = newIndex3;
	/*setNewEdge(oldIndex1, newIndex1, newTriIndex, 0);
	setNewEdge(newIndex1, newIndex3, newTriIndex, 1);
	setNewEdge(oldIndex1, newIndex3, newTriIndex, 2);*/
	newTriIndex++;
	

	subdivision_triangleArr = checkBoundary<Triangle>(subdivision_triangleArr, currentTriLen, newTriIndex);
	subdivision_triangleArr[newTriIndex].vertices[0] = newIndex1;
	subdivision_triangleArr[newTriIndex].vertices[1] = oldIndex2;
	subdivision_triangleArr[newTriIndex].vertices[2] = newIndex2;
	/*setNewEdge(newIndex1, oldIndex2, newTriIndex, 0);
	setNewEdge(oldIndex2, newIndex2, newTriIndex, 1);
	setNewEdge(newIndex1, newIndex2, newTriIndex, 2);*/
	newTriIndex++;

	subdivision_triangleArr = checkBoundary<Triangle>(subdivision_triangleArr, currentTriLen, newTriIndex);
	subdivision_triangleArr[newTriIndex].vertices[0] = newIndex3;
	subdivision_triangleArr[newTriIndex].vertices[1] = newIndex1;
	subdivision_triangleArr[newTriIndex].vertices[2] = newIndex2;
	/*setNewEdge(newIndex3, newIndex1, newTriIndex, 0);
	setNewEdge(newIndex1, newIndex2, newTriIndex, 1);
	setNewEdge(newIndex1, newIndex2, newTriIndex, 2);*/
	newTriIndex++;

	subdivision_triangleArr = checkBoundary<Triangle>(subdivision_triangleArr, currentTriLen, newTriIndex);
	subdivision_triangleArr[newTriIndex].vertices[0] = newIndex3;
	subdivision_triangleArr[newTriIndex].vertices[1] = newIndex2;
	subdivision_triangleArr[newTriIndex].vertices[2] = oldIndex3;
	/*setNewEdge(newIndex3, newIndex2, newTriIndex, 0);
	setNewEdge(newIndex2, oldIndex3, newTriIndex, 1);
	setNewEdge(newIndex3, oldIndex3, newTriIndex, 2);*/
	newTriIndex++;
}

//check if adding an element into an array will overwrite the boundary
//if so, double the length of the array
template<typename T>
T* GeometryProject::checkBoundary(T* pointer, int& length, int& currentNum){
	if((currentNum + 1) >= length){
		T *tempArr = pointer;
		length *= 2;
		pointer = new T[length];
		memcpy(pointer, tempArr, (length * sizeof(T))/2);
		/* //for debug
		for(int i = 0; i < length; i++){
			std::cout<<"double the array:"<<i<<" : "<<pointer[i]<<"\n";
		}*/
		delete []tempArr;
	}
	return pointer;
}


// Having two vertice of a triangle, get the third one of it
int GeometryProject::getThirdVertex(int edgeIndex, int triIndex){
	unsigned int triIndex1, triIndex2, triIndex3;
	unsigned int edgeIndex1, edgeIndex2;
	
	triIndex1 = oldTriangleArr[triIndex].vertices[0];
	triIndex2 = oldTriangleArr[triIndex].vertices[1];
	triIndex3 = oldTriangleArr[triIndex].vertices[2];
	
	edgeIndex1 = edges[edgeIndex].vertices[0];
	edgeIndex2 = edges[edgeIndex].vertices[1];

	if(triIndex1 != edgeIndex1 && triIndex1 != edgeIndex2)
		return triIndex1;
	if(triIndex2 != edgeIndex1 && triIndex2 != edgeIndex2)
		return triIndex2;
	if(triIndex3 != edgeIndex1 && triIndex3 != edgeIndex2)
		return triIndex3;
}


//check if an edge of index1 and index2 exist or not
//if not, add a new edge to the edge array and update all the related parameters
void GeometryProject::addEdge(unsigned int index1, unsigned int index2, int triIndex, int index){
	int exist = edgeExist(index1, index2, edges, num_edges);
	if(exist == -1){
		edges = checkBoundary<Edge>(edges,edgeArrLength,num_edges);
		edges[num_edges].vertices[0] = index1;
		edges[num_edges].vertices[1] = index2;
		edges[num_edges].containingTriangels[0] = triIndex;
		edges[num_edges].num_ctnTris = 1;
		oldTriangleArr[triIndex].edges[index] = num_edges;
		edges[num_edges].calculated = false;
		num_edges ++;
	}else{
		oldTriangleArr[triIndex].edges[index] = exist;
		edges[exist].containingTriangels[edges[exist].num_ctnTris] = triIndex;
		edges[exist].num_ctnTris++;
	}
}

void GeometryProject::setNewEdge(int index1, int index2, int triIndex, int index){
	int exist = edgeExist(index1, index2, new_edge, num_new_edges);
	if(exist == -1){
		new_edge = checkBoundary<Edge>(new_edge, newEdgeArrLength, num_new_edges);
		new_edge[num_new_edges].vertices[0] = index1;
		new_edge[num_new_edges].vertices[1] = index2;
		new_edge[num_new_edges].containingTriangels[0] = triIndex;
		new_edge[num_new_edges].num_ctnTris = 1;
		subdivision_triangleArr[triIndex].edges[index] = num_new_edges;
		new_edge[num_new_edges].calculated = false;
		num_new_edges++;
	}
	else{
		subdivision_triangleArr[triIndex].edges[index] = exist;
		new_edge[exist].containingTriangels[new_edge[exist].num_ctnTris] = triIndex;
		new_edge[exist].num_ctnTris++;
	}
}


//check if the edge already exist or not
// if exist, return the index
// if not, return -1
int GeometryProject::edgeExist(unsigned int index1, unsigned int index2, Edge *pointer, int length){
	for(int i = 0; i < length; i++){
		if((pointer[i].vertices[0] == index1 && pointer[i].vertices[1] == index2)
			||(pointer[i].vertices[1] == index1 && pointer[i].vertices[0] == index2))
			return i;
	}
	return -1;
}

//find all the adjacent triangles of a vertex
void GeometryProject::addAdjTriangle(unsigned int vertexIndex, unsigned int triangleIndex){
	bool existed = false;
	//if the num of adjacent triangles of this vertex is 0, add it to the array
	if(newVerticeArr[vertexIndex].num_adjTriangles == 0){
		newVerticeArr[vertexIndex].adjTriangles[newVerticeArr[vertexIndex].num_adjTriangles] = triangleIndex;
		newVerticeArr[vertexIndex].num_adjTriangles++;
	}else{
	//if not, check if the triangle exists in the array already or not
		for( int i = 0; i < newVerticeArr[vertexIndex].num_adjTriangles; i++){
			if(newVerticeArr[vertexIndex].adjTriangles[i] == triangleIndex)
				existed = true;
		}
		if(!existed){
	//if exist, check if writing off the end of the array
	//if over the boundary, double the size
		//	std::cout<<"length:"<<newVerticeArr[vertexIndex].arrayLen<<"\n";
			newVerticeArr[vertexIndex].adjTriangles = checkBoundary<unsigned int>(newVerticeArr[vertexIndex].adjTriangles, newVerticeArr[vertexIndex].arrayLenofAdjTriangles,newVerticeArr[vertexIndex].num_adjTriangles);
		//	std::cout<<"length after:"<<newVerticeArr[vertexIndex].arrayLen<<"\n";
			
			newVerticeArr[vertexIndex].adjTriangles[newVerticeArr[vertexIndex].num_adjTriangles] = triangleIndex;
			newVerticeArr[vertexIndex].num_adjTriangles++;
		}
	}
}

//update display list
void GeometryProject::setDisplayList(){
	/***************using the new array*****************/
	Vector3 tempPoint1, tempPoint2, tempPoint3, normalVec;
	int index1, index2, index3;
	
	/***********read the mesh, build the display list***************/
	meshList = glGenLists(1);
	glNewList(meshList, GL_COMPILE);
	glBegin(GL_TRIANGLES);
	for(int i = 0 ; i < num_triangles; i++){
				index1 = oldTriangleArr[i].vertices[0];
				index2 = oldTriangleArr[i].vertices[1];
				index3 = oldTriangleArr[i].vertices[2];	
			
				glNormal3f(newVerticeArr[index1].normal.x, newVerticeArr[index1].normal.y, newVerticeArr[index1].normal.z);
				tempPoint1 = newVerticeArr[index1].position;
				glVertex3f(tempPoint1.x, tempPoint1.y, tempPoint1.z);
					
				glNormal3f(newVerticeArr[index2].normal.x, newVerticeArr[index2].normal.y, newVerticeArr[index2].normal.z);
				tempPoint2 = newVerticeArr[index2].position;
				glVertex3f(tempPoint2.x, tempPoint2.y, tempPoint2.z);
					
				
				glNormal3f(newVerticeArr[index3].normal.x, newVerticeArr[index3].normal.y, newVerticeArr[index3].normal.z);
				tempPoint3 = newVerticeArr[index3].position;
				glVertex3f(tempPoint3.x, tempPoint3.y, tempPoint3.z);
					
				
		}
		glEnd();
	glEndList();
	 
}



} /* _462 */

