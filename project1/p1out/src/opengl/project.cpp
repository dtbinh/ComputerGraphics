/**
 * @file project.cpp
 * @brief OpenGL project
 *
 * @author H. Q. Bovik (hqbovik)
 * @bug Unimplemented
 */

#include "opengl/project.hpp"
// use this header to include the OpenGL headers
// DO NOT include gl.h or glu.h directly; it will not compile correctly.
#include "application/opengl.hpp"

// A namespace declaration. All proejct files use this namespace.
// Add this declration (and its closing) to all source/headers you create.
// Note that all #includes should be BEFORE the namespace declaration.
namespace _462 {


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

	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);                        // MUST enable each light source after configuration
	
	
	
}


// definitions of functions for the OpenglProject class

// constructor, invoked when object is created
OpenglProject::OpenglProject()
{
    // TODO any basic construction or initialization of members
    // Warning: Although members' constructors are automatically called,
    // ints, floats, pointers, and classes with empty contructors all
    // will have uninitialized data!
	SIZE = 192;
	delta = (2.0 / (SIZE - 1));
	
	meshList = 0;
	isWaterUpdated = true;
	isMeshOnly = false;
	isTransparent = true;
	heightmapMeshVertice = new Vector3[SIZE * SIZE];
	heightmapMeshTriangle = new Triangle[ (SIZE - 1) * (SIZE - 1) * 2];
	textureHeight = 0;
	textureWidth = 0;
}

// destructor, invoked when object is destroyed
OpenglProject::~OpenglProject()
{
    // TODO any final cleanup of members
    // Warning: Do not throw exceptions or call virtual functions from deconstructors!
    // They will cause undefined behavior (probably a crash, but perhaps worse).
	
}

/**
 * Initialize the project, doing any necessary opengl initialization.
 * @param camera An already-initialized camera.
 * @param scene The scene to render.
 * @return true on success, false on error.
 */
bool OpenglProject::initialize( Camera* camera, Scene* scene )
{
	
	// TODO opengl initialization code and precomputation of mesh/heightmap

    //copy scene
    this->scene = *scene;
	textureArr = imageio_load_image("texture/image1.png", &textureWidth, &textureHeight);
//	std::cout<<"\nimagesize:"<<textureWidth<<"*"<<textureHeight<<"\n";
	
	/*********some initialization***********/
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_BLEND);

	/**********texture************/
	if(textureHeight != -1 && textureWidth != -1)
	{
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &texture);	
		glBindTexture(GL_TEXTURE_2D, *textureArr);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,textureArr);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	}
	/*********lights**************/
	initLights();

	/**************camera****************/
	glMatrixMode(GL_PROJECTION);
	gluPerspective(camera->get_fov_degrees(), camera->get_aspect_ratio(), camera->get_near_clip(), camera->get_far_clip());
	glMatrixMode(GL_MODELVIEW);
	

	/************calculate the normal ************/
	meshNormal = new Vector3[scene->mesh.num_vertices];
	Vector3 tempPoint1, tempPoint2, tempPoint3, normalVec;
	int index1, index2, index3;

	//initialize the mesh array
	for(int i = 0; i < scene->mesh.num_vertices; i++){
		meshNormal[i] = Vector3(0.0,0.0,0.0);
	}

	//calculate the normal vector for each point
	for( int i = 0; i < scene->mesh.num_triangles; i++){
		index1 = scene->mesh.triangles[i].vertices[0];
		index2 = scene->mesh.triangles[i].vertices[1];
		index3 = scene->mesh.triangles[i].vertices[2];
		tempPoint1 = scene->mesh.vertices[index1];
		tempPoint2 = scene->mesh.vertices[index2];
		tempPoint3 = scene->mesh.vertices[index3];
		normalVec = cross((tempPoint2 - tempPoint1),(tempPoint3 - tempPoint1));
		meshNormal[index1] += normalVec;
		meshNormal[index2] += normalVec;
		meshNormal[index3] += normalVec;
		
	} 

	//normalize the normal
	for( int i = 0; i < scene->mesh.num_vertices; i++){
		
		meshNormal[i] = normalize(meshNormal[i]);
	}

	
	
	

	/****** read the mesh from the scene *****/
	meshList = glGenLists(1);
	glNewList(meshList, GL_COMPILE);
	glBegin(GL_TRIANGLES);
		for(int i = 0 ; i < scene->mesh.num_triangles; i++){
			index1 = scene->mesh.triangles[i].vertices[0];
			index2 = scene->mesh.triangles[i].vertices[1];
			index3 = scene->mesh.triangles[i].vertices[2];	
			
				

					glNormal3f(meshNormal[index1].x, meshNormal[index1].y, meshNormal[index1].z);
					tempPoint1 = scene->mesh.vertices[index1];
					glVertex3f(tempPoint1.x, tempPoint1.y, tempPoint1.z);
					
					//std::cout<<"0:"<<tempPoint;  //for debug
				
					glNormal3f(meshNormal[index2].x, meshNormal[index2].y, meshNormal[index2].z);
					tempPoint2 = scene->mesh.vertices[index2];
					glVertex3f(tempPoint2.x, tempPoint2.y, tempPoint2.z);
					
					//std::cout<<"1:"<<tempPoint;  //for debug
				
					glNormal3f(meshNormal[index3].x, meshNormal[index3].y, meshNormal[index3].z);
					tempPoint3 = scene->mesh.vertices[index3];
					glVertex3f(tempPoint3.x, tempPoint3.y, tempPoint3.z);
					
					//std::cout<<"2:"<<tempPoint;  //for debug

		}
		glEnd();
	glEndList();
	 
	


	/**********precompute of heightmap *********/
	//std::cout<<"\nSIZE:" <<SIZE<<"\n";
	float currHeight;
	int index = 0; 
	/*for ( double  i = -1.0;i <= 1.0; i += delta){
		for ( double j = -1.0; j <= 1.0; j += delta){
			std::cout<<"i:"<<i<<" j:"<<j<<" index"<<index<<"\n";
			currHeight = scene->heightmap->compute_height(Vector2(i,j));
			heightmapMeshVertice[index] = Vector3(i, currHeight,j);
			index++;
		}
		std::cout<<"index:"<<index<<"\n";
	} */
	double positionX = -1.0, positionY = -1.0 ;
	for(int i = 0; i < SIZE; i++){
		positionY = -1.0;
		for(int j = 0; j < SIZE; j++){
	//		std::cout<<"i:"<<i<<" j:"<<j<<" index"<<index<<"pos:"<<positionX<<" "<<positionY<<"\n";
			currHeight = scene->heightmap->compute_height(Vector2(positionX,positionY));
			heightmapMeshVertice[index] = Vector3(positionX, currHeight,positionY);
			positionY += delta;
			index++;
		}
		positionX += delta;
	//	std::cout<<"index:"<<index<<"\n";
	}
//	std::cout<<"delta:"<<delta<<"\n";
	num_heightmap_vertice = index;
//	std::cout<<"index for vertice:"<<index<<"\n"; // for debug
	
	/*********create the mesh for the water surface ***********/
	int indexOfTri = 0; // index of triangels;
	double tempResult;
	for( double j = -1.0; j < 0.99 ; j += delta){
		for ( double i = -1.0; i < 0.99; i += delta){
			tempResult = (((i - (-1.0)) * ( SIZE-1 ) ) / 2.0) * SIZE + ( ( (j - (-1.0)) * (SIZE - 1) ) / 2.0);
			index1 = static_cast<int>(tempResult+0.5);
			if(fmod((index1+1), SIZE) != 0){
				index2 = index1 + 1;
				index3 = index1 +1 +SIZE;

				heightmapMeshTriangle[indexOfTri].vertices[0] = index1;
				heightmapMeshTriangle[indexOfTri].vertices[1] = index2;
				heightmapMeshTriangle[indexOfTri].vertices[2] = index3;
				indexOfTri ++;

				index2 = index1 + SIZE + 1;
				index3 = index1 + SIZE;

				heightmapMeshTriangle[indexOfTri].vertices[0] = index1;
				heightmapMeshTriangle[indexOfTri].vertices[1] = index2;
				heightmapMeshTriangle[indexOfTri].vertices[2] = index3;
				indexOfTri++;
			}
			/*else{
				std::cout<<"index1:"<<index1<<"\n";
			}*/
		}
	}



	num_heightmap_triangles = indexOfTri;
//	std::cout<<"indexoftri:"<<indexOfTri; // for debug

	/**********average the normal**************/
	heightmapMeshNormal = new Vector3[num_heightmap_vertice];
	for(int i = 0; i < num_heightmap_vertice; i++){
		heightmapMeshNormal[i] = Vector3(0.0,0.0,0.0);
	}
   
		
    return true;
}

/**
 * Clean up the project. Free any memory, etc.
 */
void OpenglProject::destroy()
{
    // TODO any cleanup code, e.g., freeing memory
	delete []heightmapMeshTriangle;
	delete []heightmapMeshVertice;
	delete []meshNormal;
	delete []heightmapMeshNormal;
	delete []textureArr;
	
}

/**
 * Perform an update step. This happens on a regular interval.
 * @param dt The time difference from the previous frame to the current.
 */
void OpenglProject::update( real_t dt )
{
	if(isWaterUpdated){

		// update our heightmap
		scene.heightmap->update( dt );
	

		// TODO any update code, e.g. commputing heightmap mesh positions and normals

		/*******calculate heightmap mesh positions******/
		/*float currHeight;
		int index = 0; 
		for ( double  i = -1.0; i <= 1.0; i += delta){
			for ( double j = -1.0; j <= 1.0; j += delta){
				currHeight = scene.heightmap->compute_height(Vector2(i,j));
				heightmapMeshVertice[index].y =  currHeight;
				index++;
			}
		} */
		float currHeight;
		int index = 0;
		double positionX = -1.0, positionY = -1.0 ;
		for(int i = 0; i < SIZE; i++){
		positionY = -1.0;
		for(int j = 0; j < SIZE; j++){
	//		std::cout<<"i:"<<i<<" j:"<<j<<" index"<<index<<"pos:"<<positionX<<" "<<positionY<<"\n";
			currHeight = scene.heightmap->compute_height(Vector2(positionX,positionY));
			heightmapMeshVertice[index] = Vector3(positionX, currHeight,positionY);
			positionY += delta;
			index++;
		}
		positionX += delta;
	//	std::cout<<"index:"<<index<<"\n";
	}
	

		/*********calculate the normal vector for each point*************/
		int index1, index2, index3;
		Vector3 tempPoint1, tempPoint2, tempPoint3, normalVec;
		for( int i = 0; i < num_heightmap_triangles; i++){
			index1 = heightmapMeshTriangle[i].vertices[0];
			index2 = heightmapMeshTriangle[i].vertices[1];
			index3 = heightmapMeshTriangle[i].vertices[2];
			tempPoint1 = heightmapMeshVertice[index1];
			tempPoint2 = heightmapMeshVertice[index2];
			tempPoint3 = heightmapMeshVertice[index3];
			normalVec = cross((tempPoint2 - tempPoint1),(tempPoint3 - tempPoint1));
			heightmapMeshNormal[index1] += normalVec;
			heightmapMeshNormal[index2] += normalVec;
			heightmapMeshNormal[index3] += normalVec;
		} 

		/********average the normal**************/
		for( int i = 0; i < num_heightmap_vertice; i++){
			heightmapMeshNormal[i] = normalize(heightmapMeshNormal[i]);
		}

		
	}

}

/*********draw normal Vector********/
void debugNormalVector(Vector3& position, Vector3& normalVector, int ratio){
	glBegin(GL_LINES);
	glColor3f(1.0, 1.0, 1.0);
		glVertex3f(position.x, position.y, position.z);
		glVertex3f(position.x + normalVector.x * ratio, position.y + normalVector.y *ratio, position.z + normalVector.z * ratio);
	glEnd();
}

/**
 * Clear the screen, then render the mesh using the given camera.
 * @param camera The logical camera to use.
 * @see math/camera.hpp
 */
void OpenglProject::render( const Camera* camera )
{
    // TODO render code
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear Screen, Depth Buffer and Stencil Buffer
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	

	/**** set camera  *********/
	
	cam_pos = camera->get_position();
	cam_ori = camera->get_direction();
	cam_up = camera->get_up();
	gluLookAt(cam_pos.x,cam_pos.y,cam_pos.z,cam_pos.x+cam_ori.x,cam_pos.y+cam_ori.y,cam_pos.z+cam_ori.z,cam_up.x,cam_up.y,cam_up.z);

	
	if(!isTransparent){
		glDisable(GL_BLEND);
	}
	else{
		glEnable(GL_BLEND);
	}

	/*******matrix for material************/
	GLfloat matPoolAmb[] = { 0.25, 0.0, 0.0,0.05};
	GLfloat matPoolDiff[] = {0.4, 0.0, 0.0,1};
	GLfloat matPoolSpec[] = {0.5,0.0,0.0};
	GLfloat matPoolShin[] = {50};
	GLfloat matWaterAmb[] = { 0.25, 0.25, 0.5,1};
	GLfloat matWaterDiff[] = {0.6, 0.6,0.91,0.5,1};
	GLfloat matWaterSpec[] = { 0.0, 0.0, 1.0, 1.0};
	GLfloat matWaterShin[] = { 50};
	GLfloat floorColor[] = {1.0f,1.0f,1.0f,0.7f};
	
	
	/*********set the floor material**********/
	glMaterialfv(GL_FRONT, GL_SPECULAR, floorColor);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, floorColor);
	glMaterialf(GL_FRONT, GL_SHININESS, 180.0);
	/***********draw the floor***************/
	glPushMatrix();
		if(isTransparent)
			glBlendFunc(GL_ONE, GL_ZERO);
		glTranslatef(scene.mesh_position.position.x, scene.mesh_position.position.y - 0.8, scene.mesh_position.position.z);
		double w = scene.mesh_position.orientation.w * 180 / PI;
		double x = scene.mesh_position.orientation.x * 180 / PI;
		double y = scene.mesh_position.orientation.y * 180 / PI;
		double z = scene.mesh_position.orientation.z * 180 / PI;
		glRotated(w,x,y,z);
		//glRotated(scene.mesh_position.orientation.w, scene.mesh_position.orientation.x, scene.mesh_position.orientation.y, scene.mesh_position.orientation.z);
		glScalef(scene.mesh_position.scale.x, scene.mesh_position.scale.y, scene.mesh_position.scale.z);
	
		glBegin(GL_QUADS);
			glNormal3f(0,1.0f,0);
			glTexCoord2f(1.0f, 1.0f); glVertex3f( 10.0f, 0,-10.0f);
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-10.0f, 0,-10.0f);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-10.0f, 0, 10.0f);
			glTexCoord2f(1.0f, 0.0f); glVertex3f( 10.0f, 0, 10.0f);
		glEnd();

	glPopMatrix();

	/***********set pool material**************/
	glMaterialfv(GL_FRONT, GL_AMBIENT, matPoolAmb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, matPoolDiff);
	glMaterialfv(GL_FRONT, GL_SPECULAR, matPoolSpec);
	glMaterialfv(GL_FRONT, GL_SHININESS, matPoolShin);

	/****** draw pool ********/
	glPushMatrix();
		if(isTransparent)
			glBlendFunc(GL_ONE, GL_ZERO);
		glTranslatef(scene.mesh_position.position.x, scene.mesh_position.position.y, scene.mesh_position.position.z);
		glRotated(w,x,y,z);
		//glRotated(scene.mesh_position.orientation.w, scene.mesh_position.orientation.x, scene.mesh_position.orientation.y, scene.mesh_position.orientation.z);
		glScalef(scene.mesh_position.scale.x, scene.mesh_position.scale.y, scene.mesh_position.scale.z);
		
		if(!isMeshOnly){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}else{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}

		glCallList(meshList);	
	glPopMatrix();
	

	
	
	/********set water surface material***********/
	glMaterialfv(GL_FRONT, GL_AMBIENT, matWaterAmb);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, matWaterDiff);
	glMaterialfv(GL_FRONT, GL_SPECULAR, matWaterSpec);
	glMaterialfv(GL_FRONT, GL_SHININESS, matWaterShin);

	/***********draw water surface**************/
	if(isTransparent)
		glBlendFunc(GL_SRC_ALPHA,GL_DST_ALPHA);
	glPushMatrix();
		
		glTranslatef(scene.heightmap_position.position.x, scene.heightmap_position.position.y, scene.heightmap_position.position.z);
		double wWater = scene.heightmap_position.orientation.w;
		double xWater = scene.heightmap_position.orientation.x;
		double yWater = scene.heightmap_position.orientation.y;
		double zWater = scene.heightmap_position.orientation.z;
		//glRotated(wWater,xWater,yWater,zWater);
		glRotated(w,x,y,z);
	//	glRotated(scene.heightmap_position.orientation.w, scene.heightmap_position.orientation.x, scene.heightmap_position.orientation.y, scene.heightmap_position.orientation.z);
		glScalef(scene.heightmap_position.scale.x, scene.heightmap_position.scale.y, scene.heightmap_position.scale.z);
		
		if(!isMeshOnly){
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}else{ 
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_DOUBLE, 0, heightmapMeshNormal);
		glVertexPointer(3, GL_DOUBLE, 0, heightmapMeshVertice);
		
			glDrawElements(GL_TRIANGLES, (SIZE - 1) * (SIZE - 1) * 2 * 3, GL_UNSIGNED_INT, heightmapMeshTriangle);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_VERTEX_ARRAY);
	
	glPopMatrix();

	
	
	
	
}




void OpenglProject::modifyWaterStatus(bool updateStatus){
	isWaterUpdated = updateStatus;
}

void OpenglProject::modifyMesh(bool updateMesh){
	isMeshOnly = updateMesh;
}

void OpenglProject::modifyTransparency(bool updateTrans){
	isTransparent = updateTrans;
}



} /* _462 */

