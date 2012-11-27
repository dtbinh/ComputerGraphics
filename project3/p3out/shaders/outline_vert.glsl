varying vec2 tex_coord;

void main(void)
{
    // set vertex position using fixed-functionality transform
    // gl_Position is the primary "output" of the vertex shader
	
	gl_Position =ftransform();
    // store texture coordinate in a varying
    // The ".st" grabs the first two components of the vector.
    // This is called a "swizzle"
	// tex_coord = 0.5 * gl_Position.xy + vec2(0.5);
   tex_coord = gl_MultiTexCoord0.st;
	
}
