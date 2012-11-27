/**
 * @file raytacer.cpp
 * @brief Raytracer class
 *
 * Implement these functions for project 2.
 *
 * @author H. Q. Bovik (hqbovik)
 * @bug Unimplemented
 */

#include "raytracer.hpp"
#include "scene/scene.hpp"

#include <SDL/SDL_timer.h>
#include <iostream>


namespace _462 {
const float infinite = 0xFFFFFFFF;
const float slopFactor = 0.001;


Raytracer::Raytracer()
    : scene( 0 ), width( 0 ), height( 0 ) { }

Raytracer::~Raytracer() { }

/**
 * Initializes the raytracer for the given scene. Overrides any previous
 * initializations. May be invoked before a previous raytrace completes.
 * @param scene The scene to raytrace.
 * @param width The width of the image being raytraced.
 * @param height The height of the image being raytraced.
 * @return true on success, false on error. The raytrace will abort if
 *  false is returned.
 */
bool Raytracer::initialize( Scene* scene, size_t width, size_t height )
{
    this->scene = scene;
    this->width = width;
    this->height = height;

    current_row = 0;

	GLfloat matrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX, matrix);
	/*for(int i = 0; i < 16; i ++)
		std::cout<<matrix[i]<<"\n";*/
	r = abs(scene->camera.get_near_clip())/matrix[0];
	t = abs(scene->camera.get_near_clip()) / matrix[5];
	l = -r;
	b = -t;
	std::cout<<"r:"<<r<<"t:"<<t<<"\n";
    // TODO any initialization or precompuation before the trace
	
	coorW = -normalize(scene->camera.get_direction());
	coorV = normalize(scene->camera.get_up());
	coorU = normalize(cross(coorV, coorW));

	for(int i = 0; i < scene->num_geometries(); i++){
		(scene->get_geometries())[i]->matrixInitialize();
		(scene->get_geometries())[i]->bounding_box();
	}

    return true;
}

double getMax(double a, double b){
	if(a >= b)
		return a;
	else
		return b;
}

bool hit(Ray ray, real_t t0, real_t t1, HitRecord* hitRecord, const Scene* scene){
	real_t tMax = t1;
	bool isHit = false;
	for(int i = 0; i < scene->num_geometries(); i++){
		if((scene->get_geometries())[i]->intersectTest(ray, scene->camera.get_near_clip(), tMax, hitRecord)){
			
			isHit = true;
			tMax = hitRecord->time;
			
		}
	}
	
	return isHit;
}



bool refract(Ray originalRay, Vector3 normal, real_t reflectiveIndex, Vector3 &transmissRayDirection){
	real_t squareRoot;
	squareRoot = 1 - pow(reflectiveIndex, 2) * (1 - pow(dot(originalRay.rayDirection, normal),2));
	if(squareRoot < 0 )
		return false;
	else
	{
		transmissRayDirection = normalize(reflectiveIndex * (originalRay.rayDirection - normal * dot(originalRay.rayDirection, normal)) - normal * sqrt(squareRoot));
		return true;
	}
}

Color3 rayColor(Ray ray, real_t t0, real_t t1,  const Scene* scene, int depth){
	HitRecord rec, srec;
	Vector3 hitPoint;
	Color3 color;
	if(hit(ray, t0, t1, &rec, scene)){
		/****************direct illumination********************/
		color = rec.ambColor * scene->ambient_light;	//the ambient color
		Ray shadowRay;
		Vector3 hitPoint = ray.eyePosition + ray.rayDirection * rec.time; //hit point
		shadowRay.eyePosition = hitPoint + rec.normal * slopFactor; 
		
		for(int i = 0; i < scene->num_lights(); i++){ //each light in the scene
			
			shadowRay.rayDirection = normalize( (scene->get_lights())[i].position - hitPoint);
			PointLight const *currentLight = &((scene->get_lights())[i]);
			double dstnc = distance(shadowRay.eyePosition, currentLight->position);
			if(!hit(shadowRay, slopFactor, dstnc, &srec, scene)){
				Color3 ci;
				ci.r = (currentLight->color.r)/(currentLight->attenuation.constant + dstnc * currentLight->attenuation.linear + dstnc * dstnc * currentLight->attenuation.quadratic);
				ci.g = (currentLight->color.g)/(currentLight->attenuation.constant + dstnc * currentLight->attenuation.linear + dstnc * dstnc * currentLight->attenuation.quadratic);
				ci.b = (currentLight->color.b)/(currentLight->attenuation.constant + dstnc * currentLight->attenuation.linear + dstnc * dstnc * currentLight->attenuation.quadratic);
				color += rec.diffColor * ci * getMax(0, dot(rec.normal, shadowRay.rayDirection));	//the diffuse color
			}
		}

		color *= rec.textureColor; //direct illumination
		/**********************************************/

		if(depth == 0)
			return color;

		else {
			/****************reflection and refraction************************/
			if(rec.specularColor != Color3(0,0,0)){  // if specular color is not black, compute specular ray
				Vector3 reflectionDirection = normalize(ray.rayDirection - 2 * dot(ray.rayDirection, rec.normal) * rec.normal);
				Ray specularRay;  // reflection
				specularRay.eyePosition = hitPoint+ slopFactor * rec.normal;
				specularRay.rayDirection = reflectionDirection ;

				/***********************refraction*****************************/
				if(rec.refractiveIndex != 0){     // if refractive index is not 0, compute refraction ray
					Vector3 transmissionRayDirection;
					real_t airAngle;
					if(dot(ray.rayDirection, rec.normal) < 0 ){       //enter
						refract(ray,rec.normal,scene->refractive_index/rec.refractiveIndex, transmissionRayDirection);
						airAngle = -dot(ray.rayDirection, rec.normal);
					}else{       //exit
						if(refract(ray, -rec.normal, rec.refractiveIndex/scene->refractive_index, transmissionRayDirection)){
							airAngle = dot(transmissionRayDirection, rec.normal);
						}
						else{ //total internal refraction
							return rayColor(specularRay, slopFactor, infinite, scene, depth-1);
						}

					}
					//compute Fresnel equations by using Schlick approximation
					real_t R0 = pow((rec.refractiveIndex - 1) , 2) / pow(rec.refractiveIndex + 1, 2);
					real_t R = R0 + (1 - R0) * pow(1 - airAngle, 5);
					Ray refractionRay;
					refractionRay.eyePosition = hitPoint + slopFactor * rec.normal;
					refractionRay.rayDirection = normalize(transmissionRayDirection);
					return R * (color +  rec.specularColor * rec.textureColor * rayColor(specularRay, slopFactor, scene->camera.get_far_clip(), scene, depth-1))
						+ (1 - R) * (rayColor(refractionRay, slopFactor, scene->camera.get_far_clip(), scene, depth-1));
				/***************************************************************/
				}

				return color + rec.specularColor * rec.textureColor * rayColor(specularRay, slopFactor,  scene->camera.get_far_clip(), scene, depth-1);
			/**********************************************************/
			}else {
				return color; //if specular color is 0, just return direct illumination color
			}
				
		}
		
		
	}
	return scene->background_color;

}

Vector3 Raytracer::getRayDirection(const Scene* scene, size_t px, size_t py, size_t width, size_t height){
	//ray.direction <- -d *w + u * u(vector) + v * v(vector)
	//canonical view volume in openGL us a 2*2*2 cube?(suppose so)
	//std::cout<<scene->camera.get_position()<<" "<<scene->camera.get_near_clip()<<" "<<scene->camera.get_direction()<<"\n";
	double u = l + (r-l) * (px + 0.5) / width;
	double v = b + (t-b) * (py + 0.5) / height;
	double d =  scene->camera.get_near_clip();

	return  normalize(u * coorU + v * coorV - d * coorW);

}



/**
 * Performs a raytrace on the given pixel on the current scene.
 * The pixel is relative to the bottom-left corner of the image.
 * @param scene The scene to trace.
 * @param x The x-coordinate of the pixel to trace.
 * @param y The y-coordinate of the pixel to trace.
 * @param width The width of the screen in pixels.
 * @param height The height of the screen in pixels.
 * @return The color of that pixel in the final image.
 */
static Color3 trace_pixel( const Scene* scene, size_t x, size_t y, size_t width, size_t height, Ray ray)
{
    assert( 0 <= x && x < width );
    assert( 0 <= y && y < height );
	

	bool intersecting = false;
	Color3 pixelColor;
	
	

	pixelColor =  rayColor(ray, scene->camera.near_clip, infinite, scene, 3);

	
	return pixelColor;
}

/**
 * Raytraces some portion of the scene. Should raytrace for about
 * max_time duration and then return, even if the raytrace is not copmlete.
 * The results should be placed in the given buffer.
 * @param buffer The buffer into which to place the color data. It is
 *  32-bit RGBA (4 bytes per pixel), in row-major order.
 * @param max_time, If non-null, the maximum suggested time this
 *  function raytrace before returning, in seconds. If null, the raytrace
 *  should run to completion.
 * @return true if the raytrace is complete, false if there is more
 *  work to be done.
 */
bool Raytracer::raytrace( unsigned char *buffer, real_t* max_time )
{
    // TODO Add any modifications to this algorithm, if needed.

    static const size_t PRINT_INTERVAL = 64;

/*	std::cout<<"geometries Num:"<<scene->num_geometries()<<"\n"
		<<"mesh Num:"<<scene->num_meshes()<<"\n"
		<<"material Num"<<scene->num_materials()<<"\n";*/

    // the time in milliseconds that we should stop
    unsigned int end_time = 0;
    bool is_done;
	Ray ray;
    if ( max_time ) {
        // convert duration to milliseconds
        unsigned int duration = (unsigned int) ( *max_time * 1000 );
        end_time = SDL_GetTicks() + duration;
    }

    // until time is up, run the raytrace. we render an entire row at once
    // for simplicity and efficiency.
    for ( ; !max_time || end_time > SDL_GetTicks(); ++current_row ) {

        if ( current_row % PRINT_INTERVAL == 0 ) {
            printf( "Raytracing (row %u)...\n", current_row );
        }

        // we're done if we finish the last row
        is_done = current_row == height;
        // break if we finish
        if ( is_done )
            break;

        for ( size_t x = 0; x < width; ++x ) {
			ray.eyePosition = scene->camera.get_position();
			ray.rayDirection = getRayDirection(scene,x,current_row, width, height);
			
            // trace a pixel
            Color3 color = trace_pixel( scene, x, current_row, width, height,ray );
            // write the result to the buffer, always use 1.0 as the alpha
            color.to_array( &buffer[4 * ( current_row * width + x )] );
        }
    }

    if ( is_done ) {
        printf( "Done raytracing!\n" );
    }

    return is_done;
}

} /* _462 */

