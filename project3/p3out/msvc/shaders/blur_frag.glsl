varying vec2 tex_coord;


uniform sampler2DRect preFrame, currFrame;
float weight = 0.8; // should between 0 and 1
void main(void)
{
		gl_FragColor = weight * texture2DRect(currFrame, tex_coord) + (1-weight) * texture2DRect(preFrame, tex_coord); //combine this frame color and the previous frame color to make a motion blur
}
