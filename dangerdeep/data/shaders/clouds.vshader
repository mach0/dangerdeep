// -*- mode: C; -*-

varying vec2 texcoord;
varying vec3 lightdir;
varying vec3 lightcolor;
varying vec3 viewerdir;
varying float horizon_alpha;

void main()
{
	texcoord = gl_MultiTexCoord0.xy;

	float d = 1.0 - length(gl_Vertex.xy);
	horizon_alpha = max(1.0 - exp(-d*5.0), 0.0);

	// compute direction to light
	vec3 lightpos_obj = vec3(gl_ModelViewMatrixInverse * gl_LightSource[0].position);
	vec3 lightdir_obj = normalize(lightpos_obj - vec3(gl_Vertex) * gl_LightSource[0].position.w);
	lightdir = lightdir_obj;

	viewerdir = vec3(gl_ModelViewMatrixInverse[3])-vec3(gl_Vertex);

	// finally compute position
	gl_Position = ftransform();

	lightcolor = gl_Color.xyz;

	// set fog coordinate, later take distance in world (along xy plane), not viewer
	gl_FogFragCoord = gl_Position.z;
}
