
#include "application/application.hpp"
#include "application/camera_roam.hpp"
#include "geometry/project.hpp"
#include "scene/mesh.hpp"
#include <SDL/SDL.h>

namespace _462 {

#define KEY_SCREENSHOT SDLK_f
#define KEY_SUBDIVIDE  SDLK_r

static const int WIDTH = 1024;
static const int HEIGHT = 768;
static const real_t FPS = 60.0;
static const char* TITLE = "15462 Project - Geometry";

class GeometryApplication : public Application
{
public:

    GeometryApplication() : mesh_filename( 0 ) { }
    virtual ~GeometryApplication() { }

    virtual bool initialize();
    virtual void destroy();
    virtual void update( real_t dt );
    virtual void render();
    virtual void handle_event( const SDL_Event& event );

    // the project
    GeometryProject project;
    // the camera
    CameraRoamControl camera_control;
    // the mesh filename
    const char* mesh_filename;
    const char* texture_filename;

    // the triangle mesh we're rendering
    Mesh mesh;
    // the data we give initialize
    MeshData meshdata;
};

bool GeometryApplication::initialize()
{
    assert( mesh_filename );

    bool rv = true;
    int width, height;
    this->get_dimension( &width, &height );

    // set camera parameters
    Camera* camera = &camera_control.camera;
    camera_control.camera.aspect = real_t( width ) / real_t( height );
    camera->fov = PI / 4.0;
    camera->near_clip = 0.1;
    camera->far_clip = 100.0;
    camera->position = Vector3( 0.0, 1.5, 1.5 );
    camera->orientation = Quaternion::Identity;
    camera->pitch( -PI / 4.0 );

    mesh.filename = mesh_filename;
    rv = rv && mesh.load();
    if ( !mesh.has_tcoords )
        texture_filename = 0;
    rv = rv && mesh.create_gl_data();

    // copy vertices from loaded mesh
    meshdata.num_vertices = mesh.vertices.size();
    meshdata.vertices = new Vertex[meshdata.num_vertices];
    meshdata.num_triangles = mesh.triangles.size();
    meshdata.triangles = new Triangle[meshdata.num_triangles];
    for ( size_t i = 0; i < meshdata.num_vertices; ++i ) {
        meshdata.vertices[i].position = mesh.vertices[i].position;
        meshdata.vertices[i].normal = mesh.vertices[i].normal;
        meshdata.vertices[i].texture_coord = mesh.vertices[i].tex_coord;
    }
    for ( size_t i = 0; i < meshdata.num_triangles; ++i ) {
        for ( size_t j = 0; j < 3; ++j ) {
            meshdata.triangles[i].vertices[j] = mesh.triangles[i].vertices[j];
        }
    }

    rv = rv &&  project.initialize( &camera_control.camera, &meshdata, texture_filename );

    return rv;
}

void GeometryApplication::destroy()
{
    project.destroy();
    delete [] meshdata.vertices;
    delete [] meshdata.triangles;
}

void GeometryApplication::update( real_t dt )
{
    // update camera
    camera_control.update( dt );
}

void GeometryApplication::render()
{
    // adjust camera aspect
    int width, height;
    this->get_dimension( &width, &height );
    camera_control.camera.aspect = real_t( width ) / real_t( height );

    // render
    project.render( &camera_control.camera );
}

void GeometryApplication::handle_event( const SDL_Event& event )
{
  int start, end;
  switch (event.type) {
  case SDL_KEYDOWN:
    switch(event.key.keysym.sym){
    case KEY_SUBDIVIDE:
      start = SDL_GetTicks();
      project.subdivide();
      end = SDL_GetTicks();
      std::cout << "Subdivison took: " << end-start << " milliseconds" << std::endl;
      break;
    case KEY_SCREENSHOT:
      take_screenshot();
      break;
    default:
      break;
    }
  default:
    break;
  }
  camera_control.handle_event( this, event );
}

} /* _462 */

using namespace _462;

static void print_usage( const char* progname )
{
    std::cout << "Usage : "
        << progname << " <mesh filename> [texture filename]\n";
}

int main( int argc, char* argv[] )
{
    GeometryApplication app;

    // parse arguments
	
    if ( argc < 2 || argc > 3 ) {
        print_usage( argv[0] );
        return 2;
    }

    app.mesh_filename = argv[1];
    app.texture_filename = argc > 2 ? argv[2] : 0;
	std::cout<<(app.texture_filename == NULL)<<"\n";
	/**********for debug**************/
	//app.mesh_filename = "models/cube.obj";
	//app.texture_filename = "textures/texture1.png";
    // start a new application
    return Application::start_application( &app, WIDTH, HEIGHT, FPS, TITLE );
}

