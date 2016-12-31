#version 150

in vec4 pos;

void main () {
	//gl_FragColor = vec4((1.0 + pos.xyz)/2.0, 0.0);
	gl_FragColor = vec4(1.0 + pos.z, 1.0 + pos.z, 1.0 + pos.z, 0);
}
