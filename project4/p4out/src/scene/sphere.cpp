/**
 * @file sphere.cpp
 * @brief Function defnitions for the Sphere class.
 *
 * @author Kristin Siu (kasiu)
 * @author Eric Butler (edbutler)
 */

#include "scene/sphere.hpp"
#include "application/opengl.hpp"

namespace _462 {

#define SPHERE_NUM_LAT 80
#define SPHERE_NUM_LON 100

#define SPHERE_NUM_VERTICES ( ( SPHERE_NUM_LAT + 1 ) * ( SPHERE_NUM_LON + 1 ) )
#define SPHERE_NUM_INDICES ( 6 * SPHERE_NUM_LAT * SPHERE_NUM_LON )
// index of the x,y sphere where x is lat and y is lon
#define SINDEX(x,y) ((x) * (SPHERE_NUM_LON + 1) + (y))
#define VERTEX_SIZE 8
#define TCOORD_OFFSET 0
#define NORMAL_OFFSET 2
#define VERTEX_OFFSET 5

static unsigned int Indices[SPHERE_NUM_INDICES];
static float Vertices[VERTEX_SIZE * SPHERE_NUM_VERTICES];



static void init_sphere()
{
    static bool initialized = false;
    if ( initialized )
        return;

    for ( int i = 0; i <= SPHERE_NUM_LAT; i++ ) {
        for ( int j = 0; j <= SPHERE_NUM_LON; j++ ) {
            real_t lat = real_t( i ) / SPHERE_NUM_LAT;
            real_t lon = real_t( j ) / SPHERE_NUM_LON;
            float* vptr = &Vertices[VERTEX_SIZE * SINDEX(i,j)];

            vptr[TCOORD_OFFSET + 0] = lon;
            vptr[TCOORD_OFFSET + 1] = 1-lat;

            lat *= PI;
            lon *= 2 * PI;
            real_t sinlat = sin( lat );

            vptr[NORMAL_OFFSET + 0] = vptr[VERTEX_OFFSET + 0] = sinlat * sin( lon );
            vptr[NORMAL_OFFSET + 1] = vptr[VERTEX_OFFSET + 1] = cos( lat ),
            vptr[NORMAL_OFFSET + 2] = vptr[VERTEX_OFFSET + 2] = sinlat * cos( lon );
        }
    }

    for ( int i = 0; i < SPHERE_NUM_LAT; i++ ) {
        for ( int j = 0; j < SPHERE_NUM_LON; j++ ) {
            unsigned int* iptr = &Indices[6 * ( SPHERE_NUM_LON * i + j )];

            unsigned int i00 = SINDEX(i,  j  );
            unsigned int i10 = SINDEX(i+1,j  );
            unsigned int i11 = SINDEX(i+1,j+1);
            unsigned int i01 = SINDEX(i,  j+1);

            iptr[0] = i00;
            iptr[1] = i10;
            iptr[2] = i11;
            iptr[3] = i11;
            iptr[4] = i01;
            iptr[5] = i00;
        }
    }
	
    initialized = true;
	
}

Sphere::Sphere()
    : radius(0), material(0) {
}

Sphere::~Sphere() {}

void Sphere::render() const
{
    // create geometry if we haven't already
    init_sphere();
	

    if ( material )
        material->set_gl_state();

    // just scale by radius and draw unit sphere
    glPushMatrix();
    glScaled( radius, radius, radius );
    glInterleavedArrays( GL_T2F_N3F_V3F, VERTEX_SIZE * sizeof Vertices[0], Vertices );
    glDrawElements( GL_TRIANGLES, SPHERE_NUM_INDICES, GL_UNSIGNED_INT, Indices );
    glPopMatrix();

    if ( material )
        material->reset_gl_state();
}



bool Sphere::intersectTest(Ray ray, real_t t0, real_t t1, HitRecord* hitRecord) 
{
	Ray instancingRay;
	instancingRay.rayDirection = inverseMtx.transform_vector(ray.rayDirection);
	instancingRay.eyePosition = inverseMtx.transform_point(ray.eyePosition);
	if (boundingBoxIntersectTest(instancingRay,t0, t1))
	{

		float discriminant = pow(dot(instancingRay.rayDirection, instancingRay.eyePosition),2)  
			- dot(instancingRay.rayDirection, instancingRay.rayDirection) * (dot(instancingRay.eyePosition,instancingRay.eyePosition) - radius * radius);
		real_t t, t_1, t_2;
		if(discriminant < 0){
			return false;
		}
		else{
		
			t_1 = (-dot(instancingRay.rayDirection, instancingRay.eyePosition) + sqrt(discriminant))/dot(instancingRay.rayDirection, instancingRay.rayDirection);
			t_2 = (-dot(instancingRay.rayDirection, instancingRay.eyePosition) - sqrt(discriminant))/dot(instancingRay.rayDirection, instancingRay.rayDirection);

			if(t_1 >= 0 && t_2 < 0)
				t = t_1;
			if(t_2 >= 0 && t_1 < 0)
				t = t_2;
			if(t_1 < 0 && t_2 < 0)
				return false;
			if(t_1 >= 0 && t_2 >= 0)
			{
				if(t_1 > t_2)
					t = t_2;
				else 
					t = t_1;
			}
			if ((t < t0) || ( t > t1))
				return false;

			//set the hit time
			hitRecord->time = t;

			//set the normal
			Vector3 hitPoint = ray.eyePosition + ray.rayDirection * t;
			hitRecord->normal = normalize(hitPoint - position);
		
			//set the diffuse, ambient, specular color and the refractive index
			hitRecord->diffColor = material->diffuse;
			hitRecord->refractiveIndex = material->refractive_index;
			hitRecord->ambColor = material->ambient;
			hitRecord->specularColor = material->specular;
		
			//get the tex coord then get the tex color
			real_t u = atan(hitPoint.x / hitPoint.z) / (2 * PI);
			real_t v = acos(-hitPoint.y) / PI;
			int width, height;
			material->get_texture_size(&width, &height);
			hitRecord->textureColor = material->get_texture_pixel((int)(u*width), (int)(v*height));
			return true;
		}
	}
	return false;
	
}



void Sphere::bounding_box(){
	boundingBox.min = -Vector3(radius, radius, radius);
	boundingBox.max =  Vector3(radius, radius, radius);

}


} /* _462 */

