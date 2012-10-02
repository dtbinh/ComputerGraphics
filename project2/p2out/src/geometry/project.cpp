/**
 * @file project.cpp
 * @brief Geometry project
 *
 * @author Ying Yuan
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

	/***********************************************/
	textureHeight = -1;
	textureWidth = -1;
	if(texture_filename != NULL){
		textureArr = imageio_load_image(texture_filename, &textureWidth, &textureHeight);
	}else{
		hasTexture = false;
	}
	/************some initialization*****************/
	glClearColor(0.0,0.0,0.0,0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);

	/************texture********************/
	if(textureHeight != -1 && textureWidth != -1)
	{
		hasTexture = true;
		
	}else{
		hasTexture = false;
	}
	std::cout<<"has Texture:"<<hasTexture<<"\n";
	if(hasTexture){
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &texture);	
		glBindTexture(GL_TEXTURE_2D, *textureArr);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,textureArr);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}
	/**************camera****************/
	glMatrixMode(GL_PROJECTION);
	gluPerspective(camera->get_fov_degrees(), camera->get_aspect_ratio(), camera->get_near_clip(), camera->get_far_clip());
	glMatrixMode(GL_MODELVIEW);
	/*************lights******************/
	initLights();

	/******read the data from the mesh, do some initialization**********/
	std::cout<<"read the data from the mesh and do some initialization"<<"\n";
	num_vertice = mesh->num_vertices;
	oldVerticeArr = new Vertex[num_vertice];
	oldAdjVerticeArr = new AdjVertice[num_vertice];
	
	for(int i = 0; i < num_vertice; i++){
		oldVerticeArr[i] = mesh->vertices[i];
		oldAdjVerticeArr[i].boundary = false;
		oldAdjVerticeArr[i].num_AdjVertice = 0;
	}
	num_triangles = mesh->num_triangles;
	oldTriangleArr = new Triangle[num_triangles];
	for(int i = 0; i < num_triangles; i++){
		oldTriangleArr[i] = mesh->triangles[i];
	} 
	
	/*********find the adj between vertices*****************/
	std::cout<<"find the adj between vertices"<<"\n";
	unsigned int index1, index2, index3;
	 for(int i = 0; i <num_triangles; i++){
		
		index1 = oldTriangleArr[i].vertices[0];
		index2 = oldTriangleArr[i].vertices[1];
		index3 = oldTriangleArr[i].vertices[2];
		
		findAdjVertice(index1, index2, index3, oldAdjVerticeArr);
		findAdjVertice(index2, index1, index3, oldAdjVerticeArr);
		findAdjVertice(index3, index1, index2, oldAdjVerticeArr);
	}
	
	 setDisplayList();

	 firstSubdivision = true;
    // TODO opengl initialization code
    return true;
}

/**
 * Clean up the project. Free any memory, etc.
 */
void GeometryProject::destroy()
{
  // TODO any cleanup code
	delete []oldAdjVerticeArr;
	delete []oldVerticeArr;
	delete []oldTriangleArr;
	
	delete []newAdjVerticeArr;
	delete []newVerticeArr;
	delete []newTriangleArr;
}

/**
 * Clear the screen, then render the mesh using the given camera.
 * @param camera The logical camera to use.
 * @see scene/camera.hpp
 */
void GeometryProject::render( const Camera* camera )
{
  // TODO render code
	// TODO render code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear Screen, Depth Buffer and Stencil Buffer
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	/**************camera******************/
	Vector3	cam_pos = camera->get_position();
	Vector3 cam_ori = camera->get_direction();
	Vector3 cam_up = camera->get_up();
	gluLookAt(cam_pos.x,cam_pos.y,cam_pos.z,cam_pos.x+cam_ori.x,cam_pos.y+cam_ori.y,cam_pos.z+cam_ori.z,cam_up.x,cam_up.y,cam_up.z);

	/*************set the material*************/
	GLfloat matRubberAmb[] = { 0.05, 0.05, 0.0};
	GLfloat matRubberDiff[] = {0.5, 0.5, 0.4};
	GLfloat matRubberSpec[] = {0.7,0.7,0.05};
	GLfloat matRubberShin[] = {0.078125};

	glMaterialfv(GL_FRONT, GL_AMBIENT, matRubberAmb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, matRubberDiff);
	glMaterialfv(GL_FRONT, GL_SPECULAR, matRubberSpec);
	glMaterialfv(GL_FRONT, GL_SHININESS, matRubberShin);
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

	/************switch the data of the changed array(new array) and the old one *****************/
	//std::cout<<"some initialization and switch the new/odd array"<<"\n";
	
	num_oddVertice = num_vertice;
	vertice_index = num_oddVertice;
	triangle_index = 0;
	
	if(firstSubdivision){
		firstSubdivision = false;
	}else{
		delete []newVerticeArr;
		delete []newTriangleArr;
		delete []newAdjVerticeArr;
	}

	newVerticeArr = new Vertex[num_triangles * 3];
	newTriangleArr = new Triangle[num_triangles * 4];
	newAdjVerticeArr = new AdjVertice[num_triangles * 3];
	

	for(int i = 0; i < num_vertice; i++){
		newAdjVerticeArr[i] = oldAdjVerticeArr[i];
	}
	for(int i = num_vertice; i < num_triangles * 3; i++){
		newAdjVerticeArr[i].num_AdjVertice = 0;
		newAdjVerticeArr[i].boundary = false;
	}
	/***********first pass*****************/
	//std::cout<<"first pass"<<"\n";
	unsigned int newIndex1, newIndex2, newIndex3;
	unsigned int index1, index2, index3;
	for(int i = 0; i < num_triangles; i++){
		index1 = oldTriangleArr[i].vertices[0];
		index2 = oldTriangleArr[i].vertices[1];
		index3 = oldTriangleArr[i].vertices[2];
		
		newIndex1 = createNewPoint(index1, index2);
		
		newIndex2 = createNewPoint(index2, index3);
		
		newIndex3 = createNewPoint(index1, index3);
		
		formNewTriangle(index1, index2, index3, newIndex1, newIndex2, newIndex3);
	} 

	num_vertice = vertice_index;
	num_triangles = triangle_index;

	/*******find new adj between vertices**********/
	//std::cout<<"find new adj between vertices"<<"\n";
	for(int i = 0; i < num_triangles; i++){
		index1 = newTriangleArr[i].vertices[0];
		index2 = newTriangleArr[i].vertices[1];
		index3 = newTriangleArr[i].vertices[2];
		if(index1 >= num_oddVertice){
			findAdjVertice(index1, index2, index3, newAdjVerticeArr);
		}
		if(index2 >= num_oddVertice){
			findAdjVertice(index2, index1, index3, newAdjVerticeArr);
		}
		if(index3 >= num_oddVertice){
			findAdjVertice(index3, index1, index2, newAdjVerticeArr);
		}
	}
	/***********second pass****************/
	//std::cout<<"second pass"<<"\n";
	double beta;
	Vector3 newPos;
	Vector2 newTexCord;
	int boundaryIndex = 0;
	unsigned int tempBoundary[2];
	for(int i = 0; i < num_oddVertice; i++){
		if(!oldAdjVerticeArr[i].boundary){ //interior
			beta = (0.625 - pow((0.375 + (cos(2 * PI / oldAdjVerticeArr[i].num_AdjVertice) / 4)),2)) / oldAdjVerticeArr[i].num_AdjVertice;
			newPos = (1 - beta * oldAdjVerticeArr[i].num_AdjVertice) * oldVerticeArr[i].position;
			if(hasTexture){
				newTexCord = (1 - beta * oldAdjVerticeArr[i].num_AdjVertice) * oldVerticeArr[i].texture_coord;
			}
			for(int j = 0; j <oldAdjVerticeArr[i].num_AdjVertice; j++){
				newPos += beta * oldVerticeArr[oldAdjVerticeArr[i].adjArray[j]].position;
				if(hasTexture){
					newTexCord += beta * oldVerticeArr[oldAdjVerticeArr[i].adjArray[j]].texture_coord;
				}
			}
			if(hasTexture){
				newVerticeArr[i].texture_coord = newTexCord;
			}
			newVerticeArr[i].position = newPos;
		}else{ //boundary
			boundaryIndex = 0;
			for(int j = 0; j < oldAdjVerticeArr[i].num_AdjVertice; j++){
				if(oldAdjVerticeArr[oldAdjVerticeArr[i].adjArray[j]].boundary){
					tempBoundary[boundaryIndex] = oldAdjVerticeArr[i].adjArray[j];
					boundaryIndex++;
					if(boundaryIndex == 2) goto endloop; // after finding two adjacent boundary vertices, jump out of the loop
				}
			}
		endloop:
			newPos = 3 * oldVerticeArr[i].position / 4 
				+ oldVerticeArr[tempBoundary[0]].position / 8
				+ oldVerticeArr[tempBoundary[1]].position / 8;
			newVerticeArr[i].position = newPos;
			if(hasTexture){
				newTexCord = 3 * oldVerticeArr[i].texture_coord / 4
					+ oldVerticeArr[tempBoundary[0]].texture_coord / 8
					+ oldVerticeArr[tempBoundary[1]].texture_coord / 8;
				newVerticeArr[i].texture_coord = newTexCord;
			}
			
		}
	}

	/***************************************************************/

	/**********switch the new array to the old one for drawing**********/
	//std::cout<<"swtich the new to the old"<<"\n";
	delete []oldTriangleArr;
	oldTriangleArr = new Triangle[triangle_index];
	for(int i = 0; i < triangle_index; i++){
		oldTriangleArr[i] = newTriangleArr[i];
	} 

	delete []oldVerticeArr;
	oldVerticeArr = new Vertex[vertice_index];
	for(int i = 0; i < vertice_index; i++){
		oldVerticeArr[i] = newVerticeArr[i];
		oldVerticeArr[i].normal = Vector3(0,0,0);
	}

	delete []oldAdjVerticeArr;
	oldAdjVerticeArr = new AdjVertice[vertice_index];
	for(int i = 0; i < vertice_index; i++){
		oldAdjVerticeArr[i] = newAdjVerticeArr[i];
	}

	/**********compute normals********************/
	//std::cout<<"comput normals"<<"\n";
	Vector3 tempPoint1, tempPoint2, tempPoint3, normalVec;
	for( int i = 0; i < num_triangles; i++){
		//std::cout<<"triangle nums:"<<i<<":"<<num_triangles<<"\n";
		index1 = oldTriangleArr[i].vertices[0];
		index2 = oldTriangleArr[i].vertices[1];
		index3 = oldTriangleArr[i].vertices[2];
		//std::cout<<"index123:"<<index1<<" + "<<index2<<" + "<<index3<<" total num:" << num_vertex<<"\n";
		tempPoint1 = oldVerticeArr[index1].position;
		tempPoint2 = oldVerticeArr[index2].position;
		tempPoint3 = oldVerticeArr[index3].position;
		normalVec = cross((tempPoint2 - tempPoint1),(tempPoint3 - tempPoint1));
		
		oldVerticeArr[index1].normal += normalVec;
		oldVerticeArr[index2].normal += normalVec;
		oldVerticeArr[index3].normal += normalVec;
	} 
	for( int i = 0; i < num_vertice; i++){
		oldVerticeArr[i].normal = normalize(oldVerticeArr[i].normal);
	}

	setDisplayList();
}
//given three vertices of a triangle to form new edges
void GeometryProject::findAdjVertice(unsigned int vertexInt, unsigned int index1, unsigned int index2, AdjVertice *pointer){
	bool existed1 = false, existed2 = false;
	//check the edges are existed or not
	for(int i = 0; i < pointer[vertexInt].num_AdjVertice; i++){
		if(index1 == pointer[vertexInt].adjArray[i])
			existed1 = true;
		if(index2 == pointer[vertexInt].adjArray[i])
			existed2 = true;
		if(existed1 && existed2)
			break;
	}
	//if it is not existed, adding it into the adjarray
	if(!existed1){
		pointer[vertexInt].adjArray[pointer[vertexInt].num_AdjVertice] = index1;
		pointer[vertexInt].num_AdjVertice++;
	}
	if(!existed2){
		pointer[vertexInt].adjArray[pointer[vertexInt].num_AdjVertice] = index2;
		pointer[vertexInt].num_AdjVertice++;
	}
}

//given an edge, to find a new point(odd vertex)
int GeometryProject::createNewPoint(unsigned int index1, unsigned int index2){
	int num_commonAdjVertice = 0;
	unsigned int commonAdjVertice[2];
	unsigned int index1InIndex2Array = -1, index2InIndex1Array = -1;
	bool calculated = false;
	//find the index of vertex index2 in the adjacent vertices array of vertex index1
	for(int i = 0; i < oldAdjVerticeArr[index1].num_AdjVertice; i++){
		if((oldAdjVerticeArr[index1].adjArray[i]) == index2){
			index2InIndex1Array = i;
			break;
		}
	}
	//find the index of vertex index1 in the adjacent vertices array of vertex index2
	for(int i = 0; i < oldAdjVerticeArr[index2].num_AdjVertice; i++){
		if((oldAdjVerticeArr[index2].adjArray[i]) == index1){
			index1InIndex2Array = i;
			break;
		}
	}
	//find out the common adjacent vertices of index1 and index2
	for(int i = 0; i < oldAdjVerticeArr[index1].num_AdjVertice; i++){
		for(int j = 0; j < oldAdjVerticeArr[index2].num_AdjVertice; j++){
			if((oldAdjVerticeArr[index1].adjArray[i]) == (oldAdjVerticeArr[index2].adjArray[j]))
			{
				// if the two adjacent vertices have not been updated in the new adjacent vertices array,
				// that means this edge has not been calcualted
				if(((oldAdjVerticeArr[index1].adjArray[index2InIndex1Array]) == (newAdjVerticeArr[index1].adjArray[index2InIndex1Array]))
					&& ((oldAdjVerticeArr[index2].adjArray[index1InIndex2Array]) == (newAdjVerticeArr[index2].adjArray[index1InIndex2Array]))){
						// I found two vertices can share three common adjacent vertices, which doesn't make sense to me if it is one layer mesh
						// so I just overwrite the second common adjacent vertex
						if(num_commonAdjVertice == 2) 
							num_commonAdjVertice --;
						commonAdjVertice[num_commonAdjVertice] = oldAdjVerticeArr[index1].adjArray[i];
						num_commonAdjVertice++;
					
				}else{
					// if the two adjacent vertices have been updated in the new adjacent vertices array,
					// that means this edge has already been calcualted
					calculated = true;
					break;
				}
			}
				
		}
	}

	if(!calculated){
		Vector3 position;
		if(num_commonAdjVertice == 1){ //if they only have one common adjacent vertex, they're boundary vertices
			position = (oldVerticeArr[index1].position + oldVerticeArr[index2].position)/2;
			if(hasTexture){
				newVerticeArr[vertice_index].texture_coord = (oldVerticeArr[index1].texture_coord + oldVerticeArr[index2].texture_coord) / 2;
			}
			oldAdjVerticeArr[index1].boundary = true;
			oldAdjVerticeArr[index2].boundary = true;
		}else if(num_commonAdjVertice == 2){ //if they have two common adjacent vertices, they're interior vertices
			position = (oldVerticeArr[index1].position + oldVerticeArr[index2].position) * 3 / 8
				+ (oldVerticeArr[commonAdjVertice[0]].position + oldVerticeArr[commonAdjVertice[1]].position) / 8;
			if(hasTexture){
				newVerticeArr[vertice_index].texture_coord = (oldVerticeArr[index1].texture_coord + oldVerticeArr[index2].texture_coord) * 3/8
					+ (oldVerticeArr[commonAdjVertice[0]].texture_coord + oldVerticeArr[commonAdjVertice[1]].texture_coord) /8;
			}
		}
		
		newVerticeArr[vertice_index].position = position;
		//update the new adjacent vertices array
		newAdjVerticeArr[index1].adjArray[index2InIndex1Array] = vertice_index;
		newAdjVerticeArr[index2].adjArray[index1InIndex2Array] = vertice_index;
		
		vertice_index++;		
		// if this edge hasn't been calculated, return the new vertex index
		return (vertice_index-1);
	}else{
		// if this edge has been calculated, return the calculated result which is stored in the new adjacent vertices array
		return newAdjVerticeArr[index1].adjArray[index2InIndex1Array] ;
	}

}

//form four new triangles using six vertices in counter-clock wise direction
void GeometryProject::formNewTriangle(int oldIndex1, int oldIndex2, int oldIndex3, int newIndex1, int newIndex2, int newIndex3){
	newTriangleArr[triangle_index].vertices[0] = oldIndex1;
	newTriangleArr[triangle_index].vertices[1] = newIndex1;
	newTriangleArr[triangle_index].vertices[2] = newIndex3;
	triangle_index++;
	
	newTriangleArr[triangle_index].vertices[0] = newIndex1;
	newTriangleArr[triangle_index].vertices[1] = oldIndex2;
	newTriangleArr[triangle_index].vertices[2] = newIndex2;
	triangle_index++;

	newTriangleArr[triangle_index].vertices[0] = newIndex3;
	newTriangleArr[triangle_index].vertices[1] = newIndex1;
	newTriangleArr[triangle_index].vertices[2] = newIndex2;
	triangle_index++;

	newTriangleArr[triangle_index].vertices[0] = newIndex3;
	newTriangleArr[triangle_index].vertices[1] = newIndex2;
	newTriangleArr[triangle_index].vertices[2] = oldIndex3;
	triangle_index++;
}

//set the display list
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
				
				glTexCoord2f(oldVerticeArr[index1].texture_coord.x, oldVerticeArr[index1].texture_coord.y);
				glNormal3f(oldVerticeArr[index1].normal.x, oldVerticeArr[index1].normal.y, oldVerticeArr[index1].normal.z);
				tempPoint1 = oldVerticeArr[index1].position;
				glVertex3f(tempPoint1.x, tempPoint1.y, tempPoint1.z);
				
				glTexCoord2f(oldVerticeArr[index2].texture_coord.x, oldVerticeArr[index2].texture_coord.y);
				glNormal3f(oldVerticeArr[index2].normal.x, oldVerticeArr[index2].normal.y, oldVerticeArr[index2].normal.z);
				tempPoint2 = oldVerticeArr[index2].position;
				glVertex3f(tempPoint2.x, tempPoint2.y, tempPoint2.z);
					
				glTexCoord2f(oldVerticeArr[index3].texture_coord.x, oldVerticeArr[index3].texture_coord.y);
				glNormal3f(oldVerticeArr[index3].normal.x, oldVerticeArr[index3].normal.y, oldVerticeArr[index3].normal.z);
				tempPoint3 = oldVerticeArr[index3].position;
				glVertex3f(tempPoint3.x, tempPoint3.y, tempPoint3.z);
					
				
		}
		glEnd();
	glEndList();
}

//a debug function to print out the adjacent array
void GeometryProject::debugAdjVertice(){
	for(int i = 0; i < num_oddVertice; i++){
		std::cout<<i<<":";
		for(int j = 0; j < oldAdjVerticeArr[i].num_AdjVertice; j++){
			std::cout<<oldAdjVerticeArr[i].adjArray[j]<<",";
		}
		std::cout<<"num:"<<oldAdjVerticeArr[i].num_AdjVertice;
		std::cout<<"\n\n";
	} 
}

} /* _462 */

