#include "math/math.hpp"
#include "math/vector.hpp"
#include "math/quaternion.hpp"
#include "math/matrix.hpp"
#include "physics/spring.hpp"
#include "physics/body.hpp"
#include "physics/spherebody.hpp"
#include <iostream>

namespace _462 {

Spring::Spring()
{
    body1_offset = Vector3::Zero;
    body2_offset = Vector3::Zero;
    damping = 0.0;
}

void Spring::step( real_t dt )
{
    // TODO apply forces to attached bodies

	/************apply the orientation to the offset**************/
	Vector3 rotatedOffset1, rotatedOffset2;
	rotatedOffset1 = body1_offset;
	rotatedOffset2 = body2_offset;
	if( body1->orientation.z != 0 && (body1->orientation.w !=0 || body1->orientation.x != 0 || body1->orientation.y != 0))
		rotatedOffset1 = body1->orientation * body1_offset;
	if( body2->orientation.z != 0 && (body2->orientation.w !=0 || body2->orientation.x != 0 || body2->orientation.y != 0))
		rotatedOffset2 = body2->orientation * body2_offset;
	
	/**********calculate the direction and the displacement between two bodys of the spring************/
	Vector3 pos1, pos2, force1, force2;
	pos1 = body1->position + rotatedOffset1;
	pos2 = body2->position + rotatedOffset2;
	Vector3 relativeDirection = normalize(pos1 - pos2);
	real_t displacement = distance(pos2, pos1) - equilibrium;

	/*********calculate the force******************/
	force1 = Vector3(0,0,0);
	
	if(body1->velocity != Vector3(0,0,0)){
		force1 = - constant * displacement * relativeDirection - damping * body1->velocity * dot(body1->velocity, relativeDirection) / (length(body1->velocity) * length(relativeDirection));
	}
	force2 = - force1;
	
	body1->apply_force(force1, body1_offset);
	body2->apply_force(force2, body2_offset); 
}

}


