/**
 * @file project.cpp
 * @brief GLSL project
 *
 * @author H. Q. Bovik (hqbovik)
 * @bug Unimplemented
 */

#include "glsl/project.hpp"
#include <fstream>

// A namespace declaration. All proejct files use this namespace.
// Add this declration (and its closing) to all source/headers you create.
// Note that all #includes should be BEFORE the namespace declaration.
namespace _462 {

// shader loading code

/**
 * Load a file as either a vertex shader or a fragment shader, and attach
 * it to a program.
 * @param file The filename to load
 * @param type Either GL_VERTEX_SHADER_ARB, or GL_FRAGMENT_SHADER_ARB
 * @param program The shader program to which to attach the loaded shader.
 * @return True on success.
 */
static bool load_shader( const char* file, GLint type, GLhandleARB program )
{
    std::ifstream infile;
    char* buffer;
    char error_msg[2048];
    GLhandleARB shader;

    infile.open( file );

    if( infile.fail() ) {
        std::cout << "ERROR: cannot open file: " << file << std::endl;
        infile.close();
        return false;
    }

    // calculate length
    infile.seekg( 0, std::ios::end );
    int length = infile.tellg();
    infile.seekg( 0, std::ios::beg );
    // allocate space for entire program
    buffer = (char *) malloc( (length + 1) * sizeof *buffer );
    if ( !buffer )
        return false;
    // copy entire program into memory
    infile.getline( buffer, length, '\0' );
    infile.close();

    // create shader object
    shader = glCreateShaderObjectARB( type );
    // link shader source
    const char* src = buffer; // workaround for const correctness
    glShaderSourceARB( shader, 1, &src, NULL );
    // compile shaders
    glCompileShaderARB( shader );
    // check success
    GLint result;
    glGetObjectParameterivARB( shader, GL_OBJECT_COMPILE_STATUS_ARB, &result );
    if ( result != GL_TRUE ) {
        glGetInfoLogARB( shader, sizeof error_msg, NULL, error_msg );
        std::cout << "GLSL COMPILE ERROR in " << file << ": " << error_msg << std::endl;
        return false;
    } else {
        std::cout << "Compiled shaders successfully" << std::endl;
    }

    // attach the shader object to the program object
    glAttachObjectARB( program, shader );

    free( buffer );
    return true;
}

/**
 * Loads a vertex and fragment shader from the given files and attaches them to the given
 * shader program object.
 * @param vert_file The filename of the vetex shader.
 * @param frag_file The filename of the fragment shader.
 * @return True on success.
 */
static bool create_shader( GLhandleARB program, const char* vert_file, const char* frag_file )
{
    bool rv = true;

    std::cout
        << "Loading vertex shader " << vert_file
        << "and fragment shader " << frag_file << std::endl;

    // Load vertex shader
    rv = rv && load_shader( vert_file, GL_VERTEX_SHADER_ARB, program );
    // Load fragment shader
    rv = rv && load_shader( frag_file, GL_FRAGMENT_SHADER_ARB, program );

    if ( !rv )
        return false;

    // link
    glLinkProgramARB( program );

    // check for success
    GLint result;
    glGetProgramiv( program, GL_LINK_STATUS, &result );
    if ( result == GL_TRUE ) {
        std::cout << "Successfully linked shader" << std::endl;
        return true;
    } else {
        std::cout << "FAILED to link shader" << std::endl;
        return false;
    }


}


// definitions of functions for the GlslProject class

// constructor, invoked when object is created
GlslProject::GlslProject()
{
    // TODO any basic construction or initialization of members
    // Warning: Although members' constructors are automatically called,
    // ints, floats, pointers, and classes with empty contructors all
    // will have uninitialized data!
}

// destructor, invoked when object is destroyed
GlslProject::~GlslProject()
{
    // Warning: Do not throw exceptions or call virtual functions from deconstructors!
    // They will cause undefined behavior (probably a crash, but perhaps worse).
}

/**
 * Initialize the project, loading the mesh from the given filename.
 * Also do any necessary opengl initialization.
 * @param renderer Object that renders the scene.
 * @param width The width in pixels of the framebuffer.
 * @param height The height in pixels of the framebuffer.
 * @return true on success, false on error.
 * @see scene/mesh.hpp
 */
bool GlslProject::initialize( const SceneRenderer* renderer, int width, int height )
{
    bool rv = true;

    // copy renderer for later use
    this->renderer = renderer;
	this->width = width;
	this->height = height;
	firstFrame = 1;
   
	/*******create the shader for drawing outline ***************/
    program  = glCreateProgramObjectARB();
	rv = rv && create_shader( program, "shaders/outline_vert.glsl", "shaders/outline_frag.glsl" );
	//textures used in shaders
	colorLoc = glGetUniformLocationARB(program, "color");
	depthLoc = glGetUniformLocationARB(program, "depth");
	normalLoc = glGetUniformLocationARB(program, "normal");
	//near and far plane used to linearize the depth in the shader
	nearLoc = glGetUniformLocationARB(program, "near");
	farLoc = glGetUniformLocationARB(program, "far");

	/******create the shader for drawing normal into color buffer***********/
	normalProgram = glCreateProgramObjectARB();
	rv = rv && create_shader( normalProgram,"shaders/norm_vert.glsl", "shaders/norm_frag.glsl" );

	/*****create the shader for motion blur****************/
	blurProgram = glCreateProgramObjectARB();
	rv = rv && create_shader( blurProgram, "shaders/blur_vert.glsl", "shaders/blur_frag.glsl");
	thisFrameLoc = glGetUniformLocationARB(blurProgram, "currFrame");
	previousFrameLoc = glGetUniformLocationARB(blurProgram, "preFrame");
	
	/*****create the shader to draw out the texture**********/
	textureProgram = glCreateProgramObjectARB();
	rv = rv && create_shader( textureProgram, "shaders/texture_vert.glsl", "shaders/texture_frag.glsl" );
	preFrameLoc = glGetUniformLocationARB( textureProgram, "preFrame");

	// TODO opengl initialization code
	nearClip = renderer->get_camera()->get_near_clip();	//get the near plane from the camera
	farClip = renderer->get_camera()->get_far_clip(); // get the far plane from the camera
	
	
	//create a framebuffer object
	glGenFramebuffersEXT(1, &frameBuffer);		//create a new framebuffer
	
	//create a texture object for the color buffer
	glGenTextures(1, &colorBuffer);		
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, colorBuffer);	
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);		
		
	//create a texture object for the depth buffer
	glGenTextures(1, &depthBuffer);		
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, depthBuffer);		
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);	//create the texture data
	
	//create a texture object for the normal buffer
	glGenTextures(1, &normalBuffer);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, normalBuffer);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);		//create the texture data
	
	//create a texture object for the previous color buffer
	glGenTextures(1, &previousFrame);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, previousFrame);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	//create a texture object for the current color buffer
	glGenTextures(1, &currFrame);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, currFrame);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	//bind the textures to different slots
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	setActiveTexture(GL_TEXTURE0_ARB, colorBuffer);
	setActiveTexture(GL_TEXTURE1_ARB, depthBuffer);
	setActiveTexture(GL_TEXTURE2_ARB, normalBuffer);
	setActiveTexture(GL_TEXTURE3_ARB, previousFrame);
	setActiveTexture(GL_TEXTURE4_ARB, currFrame);

	glActiveTextureARB(GL_TEXTURE0_ARB);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE)
		return false;
	
    return rv;
}

/**
 * Clean up the project. Free any memory, etc.
 */
void GlslProject::destroy()
{
	// TODO any cleanup code
	/*****delete all the shaders**********/
    glDeleteObjectARB( program );
	glDeleteObjectARB( normalProgram);
	glDeleteObjectARB( blurProgram);
	glDeleteObjectARB( textureProgram);
	/******delete all the textures*********/
	glDeleteTextures(1, &colorBuffer);
	glDeleteTextures(1, &depthBuffer);
	glDeleteTextures(1, &normalBuffer);
	glDeleteTextures(1, &previousFrame);
	glDeleteTextures(1, &currFrame);
	/******delete the fbo*********/
	glDeleteFramebuffersEXT(1, &frameBuffer);

    
}



/**
 * Render the scene with outlines using shaders.
 */
void GlslProject::render()
{
	/********render the current scene to the framebuffer************/
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer); //bind the framebuffer to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, colorBuffer, 0);	//bind the texture color buffer to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_RECTANGLE_ARB, depthBuffer, 0); //bind the texture depth buffer to the FBO
	glUseProgramObjectARB(0);	// didn't use any shader in this pass
	renderer->render_scene(); //render as normal
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); //unbind the framebuffer when finished

	/***********use normal shaders to write normals into texture************/
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer); //bind the framebuffer to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, normalBuffer, 0); //bind the texture normal buffer to the FBO
	glUseProgramObjectARB(normalProgram); // bind normalprogram to the shader
	renderer->render_scene();//render as normal
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);//unbind the framebuffer when finished
	glUseProgramObjectARB(0);// unbind the shader when finished

	/**********use the normals and depth value to compute the outline ********/
	/**********store the outlined texture in current frame********************/
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer); //bind the framebuffer to the FBO
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, currFrame, 0); //bind the texture current frame to the FBO
	glUseProgramObjectARB(program); // bind (outline) program to the shader
	glUniform1fARB(nearLoc, nearClip); // pass the near plane to the shader
	glUniform1fARB(farLoc, farClip); // pass the far plane to the shader
	glUniform1iARB(colorLoc, 0); // pass the color buffer texture to the shader
	glUniform1iARB(depthLoc, 1); // pass the depth buffer texture to the shader
	glUniform1iARB(normalLoc, 2); //  pass the normal buffer texture to the shader
	setDisplayQuads(); // render the scene to the texture buffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // unbind the framebuffer when finished
	glUseProgramObjectARB(0); // unbind the shader when finished
	glPushAttrib(GL_VIEWPORT_BIT); 
	glViewport(0,0,width, height); // set the viewport
	glPopAttrib();

	/*********combine the previous and current frame to make the motion blur************/
	if(firstFrame == 1){
		firstFrame = 0;	 // if it was the first frame, didn't do anything
	}else{ // if not the first frame 
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer); //bind the framebuffer to the FBO
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, previousFrame, 0);//bind the texture previous frame to the FBO
		glUseProgramObjectARB(blurProgram); // bind blur program to the shader
		glUniform1iARB(previousFrameLoc, 3); // pass the previous frame buffer texture to the shader
		glUniform1iARB(thisFrameLoc, 4); // pass the current frame buffer texture to the shader
		setDisplayQuads(); // render the scene to the texture buffer
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // unbind the framebuffer when finished
		glUseProgramObjectARB(0); // unbind the shader when finished
		glPushAttrib(GL_VIEWPORT_BIT);
		glViewport(0,0,width, height); // set the viewport
		glPopAttrib();
	}

	/*************render the scene to the screen****************/
	glUseProgramObjectARB(textureProgram); // bind texture program to the shader
	glUniform1iARB(preFrameLoc, 3);  // pass the previous frame buffer texture to the shader
	setDisplayQuads(); // render the scene to the texture buffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // unbind the framebuffer when finished
	glUseProgramObjectARB(0); // unbind the shader when finished
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,width, height); // set the viewport
	glPopAttrib();
				
}


//render the framebuffer
void GlslProject::setDisplayQuads(){
	//set projection and modelview to identity
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		glTexCoord2f(width, 0.0f);
		glVertex3f(1.0f, -1.0f, -1.0f);
		glTexCoord2f(width, height);
		glVertex3f(1.0f, 1.0f, -1.0f);
		glTexCoord2f(0.0f, height);
		glVertex3f(-1.0f, 1.0f, -1.0f);
	glEnd();
}

//set the texture to the slot
void GlslProject::setActiveTexture(GLenum texture, GLuint &TexID){
	glActiveTextureARB(texture);
	glEnable(GL_TEXTURE_RECTANGLE_ARB);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, TexID);
	glDisable(GL_TEXTURE_RECTANGLE_ARB);
}

} /* _462 */

