#version 430 compatibility
//Default fragment shader

out vec4 fFragColor;

uniform sampler2D prevRender; // texture with the previous render pass

 layout(pixel_center_integer) in vec4 gl_FragCoord;
 // will give the screen position of the current fragment

void main()
{
	ivec2 screenpos = ivec2(gl_FragCoord.xy); // convert fragment position to integers
	vec4 currentValue = texelFetch(prevRender, screenpos, 0); // what is currently stored in this pixel
	fFragColor = vec4(currentValue.xy, 0., currentValue.w);
	//fFragColor = vec4(1., 1., 1., 1.);
}