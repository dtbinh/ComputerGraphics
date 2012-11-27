// set by the vertex shader
varying vec2 tex_coord;
// set by the C program
uniform sampler2DRect color;
uniform sampler2DRect depth;
uniform sampler2DRect normal;

uniform float near;
uniform float far;

float depthMagnitude;
float normalMagnitude;

float depthMagnitudeThreshold = 10;
float normalMagnitudeThreshold = 0.3;

float convertZ(float depthBufferValue) //linearize the depth
{
	return (near * far) / (far - depthBufferValue * (far - near)); 
}

float computeDepthMagnitude() // get the difference of the depth for the outline
{
	float horizontalValue = convertZ(texture2DRect(depth, vec2(tex_coord.x + 1, tex_coord.y)).x)  //get the depth value at (this.x+1, this.y) and linearized
						- convertZ(texture2DRect(depth, vec2(tex_coord.x - 1, tex_coord.y)).x);  //get the depth value at (this.x-1, this.y) and linearized
	float verticalValue = convertZ(texture2DRect(depth, vec2(tex_coord.x, tex_coord.y + 1)).x) //get the depth value at (this.x, this.y+1) and linearized
						- convertZ(texture2DRect(depth, vec2(tex_coord.x, tex_coord.y - 1)).x); //get the depth value at (this.x, this.y-1) and linearized
	
	return abs(horizontalValue) + abs(verticalValue); // return the sum 
}




float getAngle(vec3 normal1, vec3 normal2) // get the angle between two vectors
{
	float dotProduct = dot(normal1.xyz, normal2.xyz); //dot product
	float magnitudeProduct = length(normal1.xyz) * length(normal2.xyz); // product of lengths
	float angle = acos(dotProduct / magnitudeProduct); // get the angle between two vectors
	return abs(angle);
}

float computeNormalMagnitude()
{
	vec3 normal1, normal2;
	normal1 = (texture2DRect(normal, vec2(tex_coord.x + 1, tex_coord.y)).xyz - vec3(0.5)) * 2; //get the normal value at (this.x+1, this.y) and linearized
	normal2 = (texture2DRect(normal, vec2(tex_coord.x - 1, tex_coord.y)).xyz - vec3(0.5)) * 2; //get the normal value at (this.x-1, this.y) and linearized
	float horizontalValue = abs(getAngle(normal1, normal2));
	normal1 = (texture2DRect(normal, vec2(tex_coord.x, tex_coord.y + 1)).xyz - vec3(0.5)) * 2; //get the depth value at (this.x, this.y+1) and linearized
	normal2 = (texture2DRect(normal, vec2(tex_coord.x, tex_coord.y - 1)).xyz - vec3(0.5)) * 2; //get the depth value at (this.x, this.y-1) and linearized
	float verticalValue = abs(getAngle(normal1, normal2));
	
	return (horizontalValue + verticalValue);
}


void main(void)
{
	depthMagnitude = computeDepthMagnitude(); // get the depth difference
	normalMagnitude = computeNormalMagnitude(); // get the normal difference
	if( (normalMagnitude > normalMagnitudeThreshold)||(depthMagnitude > depthMagnitudeThreshold)) //if normal or depth difference value is greater than some threshold, set it a outline vertex
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
	else
		gl_FragColor = vec4(texture2DRect(color, tex_coord).xyz, 1.0); //otherwise, output the normal color
	
}

