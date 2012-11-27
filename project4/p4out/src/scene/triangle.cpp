/**
 * @file triangle.cpp
 * @brief Function definitions for the Triangle class.
 *
 * @author Eric Butler (edbutler)
 */

#include "scene/triangle.hpp"
#include "application/opengl.hpp"

namespace _462 {

Triangle::Triangle()
{
    vertices[0].material = 0;
    vertices[1].material = 0;
    vertices[2].material = 0;
	
}

Triangle::~Triangle() { }

void Triangle::render() const
{
    bool materials_nonnull = true;
    for ( int i = 0; i < 3; ++i )
        materials_nonnull = materials_nonnull && vertices[i].material;

    // this doesn't interpolate materials. Ah well.
    if ( materials_nonnull )
        vertices[0].material->set_gl_state();

    glBegin(GL_TRIANGLES);

    glNormal3dv( &vertices[0].normal.x );
    glTexCoord2dv( &vertices[0].tex_coord.x );
    glVertex3dv( &vertices[0].position.x );

    glNormal3dv( &vertices[1].normal.x );
    glTexCoord2dv( &vertices[1].tex_coord.x );
    glVertex3dv( &vertices[1].position.x);

    glNormal3dv( &vertices[2].normal.x );
    glTexCoord2dv( &vertices[2].tex_coord.x );
    glVertex3dv( &vertices[2].position.x);

    glEnd();

    if ( materials_nonnull )
        vertices[0].material->reset_gl_state();
}

bool Triangle::intersectTest(Ray ray, real_t t0, real_t t1, HitRecord* hitRecord) 
{
	
	Ray instancingRay;
	instancingRay.eyePosition = inverseMtx.transform_point(ray.eyePosition);
	instancingRay.rayDirection = inverseMtx.transform_vector(ray.rayDirection);
	if(boundingBoxIntersectTest(ray, t0, t1)){
		double gamma, beta, t, mag;
		double a, b, c, d, e, f, g, h, i, j, k, l;
		a = vertices[0].position.x - vertices[1].position.x;
		b = vertices[0].position.y - vertices[1].position.y;
		c = vertices[0].position.z - vertices[1].position.z;

		d = vertices[0].position.x - vertices[2].position.x;
		e = vertices[0].position.y - vertices[2].position.y;
		f = vertices[0].position.z - vertices[2].position.z;

		g = instancingRay.rayDirection.x;
		h = instancingRay.rayDirection.y;
		i = instancingRay.rayDirection.z;
	
		j = vertices[0].position.x - instancingRay.eyePosition.x;
		k = vertices[0].position.y - instancingRay.eyePosition.y;
		l = vertices[0].position.z - instancingRay.eyePosition.z;

		mag = a * (e * i - h * f) + b * (g * f - d * i) + c * (d * h - e * g);

		t = -(f * (a * k - j * b) + e * (j * c - a * l) + d * (b * l - k * c)) / mag ;
		if ((t < t0) || ( t > t1))
			return false;

		gamma = (i * (a * k - j * b) + h * (j * c - a * l) + g * (b * l - k * c)) / mag;
		if( (gamma < 0.0) || (gamma > 1.0))
			return false;

		beta = (j * (e * i - h * f) + k * (g * f - d * i) + l * (d * h - e * g)) / mag ;
		if( (beta < 0.0) || ( beta > (1.0- gamma)) )
			return false;

		//set the time
		hitRecord->time = t;
		//set the normal
		Vector3 tempNormal;
		tempNormal = ( 1- beta - gamma ) * vertices[0].normal + beta * vertices[1].normal + gamma * vertices[2].normal;
		hitRecord->normal = normalize(normalMtx * tempNormal);
		//set the diffuse, ambient, specular color
		hitRecord->diffColor = (1- beta - gamma ) * vertices[0].material->diffuse + beta * vertices[1].material->diffuse + gamma * vertices[2].material->diffuse;
		hitRecord->refractiveIndex = (1-beta-gamma) * vertices[0].material->refractive_index + beta * vertices[1].material->refractive_index + gamma * vertices[2].material->refractive_index;
	
		hitRecord->ambColor = (1 - beta - gamma) * vertices[0].material->ambient + beta * vertices[1].material->ambient + gamma * vertices[2].material->ambient;
		hitRecord->specularColor = (1 - beta - gamma) * vertices[0].material->specular + beta * vertices[1].material->specular + gamma * vertices[2].material->specular;
		//set the texture color
		textureLookup(hitRecord, beta, gamma);


		return true;
	}
	return false;
}

void Triangle::textureLookup(HitRecord* hitRecord, real_t beta, real_t gamma){
	int width, height;
	real_t tex_x_interpolated, tex_y_interpolated;
	//interpolate the coordinates
	tex_x_interpolated = (1- gamma - beta) * vertices[0].tex_coord.x + beta * vertices[1].tex_coord.x + gamma * vertices[2].tex_coord.x;
	tex_y_interpolated = (1- gamma - beta) * vertices[0].tex_coord.y + beta * vertices[1].tex_coord.y + gamma * vertices[2].tex_coord.y;
	//only get the fraction part
	if(tex_x_interpolated > 1 || tex_x_interpolated < 0)
		tex_x_interpolated -= (int)tex_x_interpolated;
	if(tex_y_interpolated > 1 || tex_y_interpolated < 0)
		tex_y_interpolated -= (int)tex_y_interpolated;
	//get color from each vertex
	Color3 colors[3];
	for(int i = 0; i < 3; i++){
		vertices[i].material->get_texture_size(&width, &height);
		colors[i] = vertices[i].material->get_texture_pixel(tex_x_interpolated * width, tex_y_interpolated * height);
	}
	//interpolate the texture color
	hitRecord->textureColor = (1 - gamma - beta) * colors[0] + beta * colors[1] + gamma * colors[2];

}

void Triangle::bounding_box(){
	boundingBox.max = vmax(vmax(vertices[0].position, vertices[1].position), vertices[2].position);
	boundingBox.min = vmin(vmin(vertices[0].position, vertices[1].position), vertices[2].position);
	boundingBox.max = transformMtx.transform_point(boundingBox.max);
	boundingBox.min = transformMtx.transform_point(boundingBox.min);
}



} /* _462 */

