varying vec2 tex_coord;
varying vec3 norm;
vec3 normal;
vec3 normClipped;
void main(void)
{
	
	normal = normalize( gl_NormalMatrix * norm); //normalize the normal value to squeeze into a color buffer
	normClipped = norm  * 0.5 + vec3(0.5); // map the [-1,1] to [0,1]
	gl_FragColor = vec4(normClipped, 0.0); //write it into the color buffer
	
}
