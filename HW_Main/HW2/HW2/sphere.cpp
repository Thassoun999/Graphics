
#include "Angel-yjc.h"
#include <fstream>
#include <iostream>
#include <cmath>
#include <string>
#include <stdio.h>

void file_in(std::string document);

GLuint  model_view;  // model-view matrix uniform shader variable location
GLuint  projection;  // projection matrix uniform shader variable location
GLuint  model_view_2; 
GLuint  projection_2;  

int triangleNumbers;
int shadow = 1;
int spotlight = 0;
int pointsource = 0;
int flatshading = 0;
int smoothshading = 0;
int lighting = 0;

typedef Angel::vec3  color3;
typedef Angel::vec3  point3;
typedef Angel::vec4  point4;
typedef Angel::vec4  color4;	

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program1;      
GLuint program2;
GLuint sphere_buffer;   /* vertex buffer object id for sphere */
GLuint sphere_shadow_buffer;
GLuint floor_buffer;  /* vertex buffer object id for floor */
GLuint axis_buffer; /* vertex buffer object id for axis */

GLuint floor_light_buffer;
GLuint sphere_flat_buffer;
GLuint sphere_smooth_buffer;

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 30.0;

GLfloat angle = 0.0; // rotation angle in degrees
//vec4 init_eye(3.0, 2.0, 0.0, 1.0); // initial viewer position
//vec4 eye = init_eye;               // current viewer position

int animationFlag = 0; // 1: animation; 0: non-animation. Toggled by key 'b' or 'B'
bool bPress = false; //Only happens once
int sphereFlag = 0;   // 1: solid sphere; 0: wireframe sphere. Toggled by key 'c' or 'C'
int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'

const int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point3 floor_points[floor_NumVertices]; // positions for all vertices
color3 floor_colors[floor_NumVertices]; // colors for all vertices

point4 floor_light_points[floor_NumVertices];
vec3   floor_normals[floor_NumVertices];

//CircleStuff
point3 *sphereVertices;
color3 *sphereColors;

point3 *sphere_shadow_data;
color3 *sphere_shadow_color;

point4 *sphere_shade_regular;
point3 *sphere_flat_shade;
point3 *sphere_smooth_shade;

vec4 c_vrp(7, 3, -10, 1); //Initial viewer position
vec4 c_vrp_lighting(-14.0, 12.0, -3.0, 1.0); //the right-handed coordinate system after 
vec4 c_vpn(-7, -3, 10, 0); //View position normal
vec4 up(0.0, 1.0, 0.0, 0.0);

vec4 c_at = c_vrp + c_vpn;
vec4 c_eye = c_vrp;
int sphere_NumVertices;
point3 spherePointA(-4, 1, 4);
point3 spherePointB(-1, 1, -4);
point3 spherePointC(3, 1, 5);

//Transformation values starting at point A
GLfloat x = spherePointA.x;
GLfloat y = spherePointA.y; //Most likely not affecting this one
GLfloat z = spherePointA.z;

mat4 rotationAccumulation(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
int rotPoint = 0;

vec3 ab = spherePointB - spherePointA;
vec3 bc = spherePointC - spherePointB;
vec3 ca = spherePointA - spherePointC;

//Initially set to this
vec3 rotAxisVector = cross(vec3(0.0, 1.0, 0.0), ab);
vec3 rollDirection = ab;
GLfloat length_points = length(ab);

//Axis
const int axis_NumVertices = 6;
point3 axis_points[axis_NumVertices];
color3 axis_colors[axis_NumVertices];

//Shadow Projection Transformation N

mat4 trans_p_to_q(12, 0, 0, 0, 14, 0, 3, -1, 0, 0, 12, 0, 0, 0, 0, 12);
//mat4 trans_p_to_q(-12, 0, 0, 0, -14, 0, -3, 1, 0, 0, -12, 0, 0, 0, 0, -12); We need this to be reversed
//q = (-12x - 14y, 0, -12z - 3y, y-12)
//trans_p_to_q * (x, y, z, 1) = q
//(x, y, z, 1) = p

//Light variables
color4 light_ambient(0, 0, 0, 1.0);
color4 light_diffuse(0.8, 0.8, 0.8, 1.0);
color4 light_specular(0.2, 0.2, 0.2, 0.2);


void set_spot_or_point(mat4 mv, int object) {

	//std::cout << "I am here and here are the following spot and point values " << spotlight << " " << pointsource << std::endl;
	color4 light_ambient_new(0, 0, 0, 1.0);
	color4 light_diffuse_new(1.0, 1.0, 1.0, 1.0);
	color4 light_specular_new(1.0, 1.0, 1.0, 1.0);

	float const_attenuation = 2.0;
	float linear_attenuation = 0.01;
	float quad_attenuation = 0.001;

	color4 ambient_product_new;
	color4 diffuse_product_new;
	color4 specular_product_new;

	//We are a sphere
	if (object == 0) {
		ambient_product_new = light_ambient_new * vec4(0.2, 0.2, 0.2, 1.0);
		diffuse_product_new = light_diffuse_new * vec4(1.0, 0.84, 0, 1);
		specular_product_new = light_specular_new * vec4(1.0, 0.84, 0, 1);
	}
	else if (object == 1) { 	//We are a floor
		ambient_product_new = light_ambient_new * vec4(0.2, 0.2, 0.2, 1.0);
		diffuse_product_new = light_diffuse_new * vec4(0, 1.0, 0, 1);
		specular_product_new = light_specular_new * vec4(0, 0, 0, 1);
	}
	

	//c_vrp_lighting

	vec4 light_position_eyeFrame = mv * c_vrp_lighting;
	glUniform4fv(glGetUniformLocation(program1, "LightPosition"),
		1, light_position_eyeFrame);


	glUniform4fv(glGetUniformLocation(program1, "mod_AmbientProduct"),
		1, ambient_product_new);
	glUniform4fv(glGetUniformLocation(program1, "mod_DiffuseProduct"),
		1, diffuse_product_new);
	glUniform4fv(glGetUniformLocation(program1, "mod_SpecularProduct"),
		1, specular_product_new);


	glUniform1f(glGetUniformLocation(program1, "ConstAtt"),
		const_attenuation);
	glUniform1f(glGetUniformLocation(program1, "LinearAtt"),
		linear_attenuation);
	glUniform1f(glGetUniformLocation(program1, "QuadAtt"),
		quad_attenuation);

	if (spotlight == 1) {
		glUniform1f(glGetUniformLocation(program1, "Spotlight"),
			1.0);
		glUniform1f(glGetUniformLocation(program1, "Pointsource"),
			0.0);
	}
	else if (pointsource == 1) {
		glUniform1f(glGetUniformLocation(program1, "Spotlight"),
			0.0);
		glUniform1f(glGetUniformLocation(program1, "Pointsource"),
			1.0);
	}
	

	if (spotlight == 1) {
		float exponentSpot = 15.0;
		float angleSpot = cos(20.0 * M_PI / 180);

		point4 sDirectionDest = mv * vec4(-6.0, 0.0, -4.5, 1.0);

		glUniform4fv(glGetUniformLocation(program1, "dirSpotDest"), 1,
			sDirectionDest);
		glUniform1f(glGetUniformLocation(program1, "expSpot"), exponentSpot);
		glUniform1f(glGetUniformLocation(program1, "angSpot"), angleSpot);
	}

}


void set_floor_m(mat4 mv)
{
	color4 material_ambient(0.2, 0.2, 0.2, 1.0);
	color4 material_diffuse(0, 1.0, 0, 1);
	color4 material_specular(0, 0, 0, 1);
	float material_shininess = 125.0f;

	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	color4 specular_product = light_specular * material_specular;

	glUniform4fv(glGetUniformLocation(program1, "dist_AmbientProduct"),
		1, ambient_product);
	glUniform4fv(glGetUniformLocation(program1, "dist_DiffuseProduct"),
		1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program1, "dist_SpecularProduct"),
		1, specular_product);

	
	glUniform1f(glGetUniformLocation(program1, "Shininess"),
		material_shininess);

}

void set_sphere_m(mat4 mv)
{
	color4 material_ambient(0.2, 0.2, 0.2, 1.0);
	color4 material_diffuse(1.0, 0.84, 0, 1);
	color4 material_specular(1.0, 0.84, 0, 1);
	float  material_shininess = 125.0f;

	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	color4 specular_product = light_specular * material_specular;

	glUniform4fv(glGetUniformLocation(program1, "dist_AmbientProduct"),
		1, ambient_product);
	glUniform4fv(glGetUniformLocation(program1, "dist_DiffuseProduct"),
		1, diffuse_product);
	glUniform4fv(glGetUniformLocation(program1, "dist_SpecularProduct"),
		1, specular_product);

	glUniform1f(glGetUniformLocation(program1, "Shininess"),
		material_shininess);


}


void floor()
{
	floor_points[0] = point3(5.0, 0.0, 8.0); floor_colors[0] = color3(0.0, 1.0, 0.0);
	floor_points[1] = point3(5.0, 0.0, -4.0); floor_colors[1] = color3(0.0, 1.0, 0.0);
	floor_points[2] = point3(-5.0, 0.0, 8.0); floor_colors[2] = color3(0.0, 1.0, 0.0);

	floor_points[3] = point3(5.0, 0.0, -4.0); floor_colors[3] = color3(0.0, 1.0, 0.0);
	floor_points[4] = point3(-5.0, 0.0, 8.0); floor_colors[4] = color3(0.0, 1.0, 0.0);
	floor_points[5] = point3(-5.0, 0.0, -4.0); floor_colors[5] = color3(0.0, 1.0, 0.0);


}

void floor_l()
{ 
	/*
	//myquad 1, 0, 3, 2
	point4(5, 0, 8, 1.0), //b
		point4(5, 0, -4, 1.0), //a
		point4(-5, 0, -4, 1.0), //d
		point4(-5, 0, 8, 1.0), //c
	*/
	vec4 u = point4(5, 0, 8, 1.0) - point4(5, 0, -4, 1.0);
	vec4 v = point4(-5, 0, -4, 1.0) - point4(5, 0, -4, 1.0);

	vec3 normalFl = normalize(cross(u, v));

	floor_light_points[0] = point4(5, 0, -4, 1.0); floor_normals[0] = normalFl;
	floor_light_points[1] = point4(5, 0, 8, 1.0); floor_normals[1] = normalFl;
	floor_light_points[2] = point4(-5, 0, 8, 1.0); floor_normals[2] = normalFl;

	floor_light_points[3] = point4(5, 0, -4, 1.0); floor_normals[3] = normalFl;
	floor_light_points[4] = point4(-5, 0, 8, 1.0); floor_normals[4] = normalFl;
	floor_light_points[5] = point4(-5, 0, -4, 1.0); floor_normals[5] = normalFl;

}

void axis()
{
	axis_points[0] = point3(0.0, 0.0, 0.0); axis_colors[0] = color3(1.0, 0.0, 0.0);
	axis_points[1] = point3(10.0, 0.0, 0.0); axis_colors[1] = color3(1.0, 0.0, 0.0);

	axis_points[2] = point3(0.0, 0.0, 0.0); axis_colors[2] = color3(1.0, 0.0, 1.0);
	axis_points[3] = point3(0.0, 10.0, 0.0); axis_colors[3] = color3(1.0, 0.0, 1.0);

	axis_points[4] = point3(0.0, 0.0, 0.0); axis_colors[4] = color3(0.0, 0.0, 1.0);
	axis_points[5] = point3(0.0, 0.0, 10.0); axis_colors[5] = color3(0.0, 0.0, 1.0);
}


void sphere_init_flat_normal() {
	int IndexSphereShade = 0;
	for (int x = 0; x < triangleNumbers; x++) {
		vec4 p1 = sphere_shade_regular[3 * x];
		vec4 p2 = sphere_shade_regular[3 * x + 1];
		vec4 p3 = sphere_shade_regular[3 * x + 2];

		vec4 u = p2 - p1;
		vec4 v = p3 - p1;

		vec3 normalSF = normalize(cross(u, v));

		sphere_flat_shade[IndexSphereShade] = normalSF; IndexSphereShade++;
		sphere_flat_shade[IndexSphereShade] = normalSF; IndexSphereShade++;
		sphere_flat_shade[IndexSphereShade] = normalSF; IndexSphereShade++;
	}
}

void sphere_init_smooth_normal() {
	for (int x = 0; x < sphere_NumVertices; x++) {
		vec4 p = sphere_shade_regular[x];	
		vec3 normalSS = vec3(p[0], p[1], p[2]);
		normalSS = normalize(normalSS);
		sphere_smooth_shade[x] = normalSS;
	}
}

// OpenGL initialization
void init()
{
	// Create and initialize a vertex buffer object for sphere, to be used in display()
	glGenBuffers(1, &sphere_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);



	glBufferData(GL_ARRAY_BUFFER,
		sizeof(point3)*sphere_NumVertices + sizeof(color3)*sphere_NumVertices,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(point3) * sphere_NumVertices, sphereVertices);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(point3) * sphere_NumVertices,
		sizeof(color3) * sphere_NumVertices,
		sphereColors);
	

	


	glGenBuffers(1, &sphere_shadow_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_shadow_buffer);

	glBufferData(GL_ARRAY_BUFFER,
		sizeof(point3)*sphere_NumVertices + sizeof(color3)*sphere_NumVertices,
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(point3) * sphere_NumVertices, sphere_shadow_data);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(point3) * sphere_NumVertices,
		sizeof(color3) * sphere_NumVertices,
		sphere_shadow_color);

	floor();
	// Create and initialize a vertex buffer object for floor, to be used in display()
	glGenBuffers(1, &floor_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
		floor_colors);

	floor_l();
	glGenBuffers(1, &floor_light_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_light_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_light_points) + sizeof(floor_normals),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_light_points), floor_light_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_light_points), sizeof(floor_normals),
		floor_normals);

	glGenBuffers(1, &sphere_flat_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_flat_buffer);
	glBufferData(GL_ARRAY_BUFFER, sphere_NumVertices * sizeof(point4) + sphere_NumVertices * sizeof(vec3),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sphere_NumVertices * sizeof(point4), sphere_shade_regular);
	glBufferSubData(GL_ARRAY_BUFFER, sphere_NumVertices * sizeof(point4),
		sphere_NumVertices * sizeof(vec3), sphere_flat_shade);

	glGenBuffers(1, &sphere_smooth_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_smooth_buffer);
	glBufferData(GL_ARRAY_BUFFER, sphere_NumVertices * sizeof(point4) + sphere_NumVertices * sizeof(vec3),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sphere_NumVertices * sizeof(point4), sphere_shade_regular);
	glBufferSubData(GL_ARRAY_BUFFER, sphere_NumVertices * sizeof(point4),
		sphere_NumVertices * sizeof(vec3), sphere_smooth_shade);

	axis();
	//Create and initiazlie a vertex buffer object for axis, to be used in display()
	glGenBuffers(1, &axis_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points) + sizeof(axis_colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points), axis_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points), sizeof(axis_colors),
		axis_colors);

	// Load shaders and create a shader program (to be used in display())
	program2 = InitShader("vshader42.glsl", "fshader42.glsl");
	program1 = InitShader("vshader53.glsl", "fshader53.glsl");

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.529, 0.807, 0.92, 0.0);

}
//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObj(GLuint buffer, int num_vertices)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program2, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program2, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point3) * num_vertices));
	// the offset is the (total) size of the previous vertex attribute array(s)

	/* Draw a sequence of geometric objs (triangles) from the vertex buffer
	 (using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
}

void drawObj2(GLuint buffer, int num_vertices)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program1, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program1, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point4) * num_vertices));
	// the offset is the (total) size of the previous vertex attribute array(s)

	/* Draw a sequence of geometric objs (triangles) from the vertex buffer
	 (using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vNormal);
}
//----------------------------------------------------------------------------
void drawLine(GLuint buffer, int num_vertices)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program2, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program2, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point3) * num_vertices));
	// the offset is the (total) size of the previous vertex attribute array(s)

	glDrawArrays(GL_LINES, 0, num_vertices);

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
}
//----------------------------------------------------------------------------

void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

	/*---  Set up and pass on Projection matrix to the shader ---*/
	mat4  p = Perspective(fovy, aspect, zNear, zFar);

	
	/*----- Set up the Mode-View matrix for the floor and axis -----*/
	mat4 mv = LookAt(c_eye, c_at, up);


	glDepthMask(GL_FALSE);
	if (lighting == 0) {
		glUseProgram(program2); // Use the shader program
		model_view = glGetUniformLocation(program2, "model_view");
		projection = glGetUniformLocation(program2, "projection");

		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
		glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
		if (floorFlag == 1) // Filled floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawObj(floor_buffer, floor_NumVertices);  // draw the floor
	}
	else {
		glUseProgram(program1); // Use the shader program
		model_view_2 = glGetUniformLocation(program1, "ModelView");
		projection_2 = glGetUniformLocation(program1, "Projection");

		glUniformMatrix4fv(model_view_2, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
		glUniformMatrix4fv(projection_2, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

		if (spotlight == 1 || pointsource == 1) {
			set_spot_or_point(mv, 1);
		}

		set_floor_m(mv);
		mat4 model_view_mod = mv;
		mat3 normal_matrix = NormalMatrix(model_view_mod, 1);
		glUniformMatrix3fv(glGetUniformLocation(program1, "Normal_Matrix"),
			1, GL_TRUE, normal_matrix);
		if (floorFlag == 1) // Filled floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawObj2(floor_light_buffer, floor_NumVertices);
	}
	glDepthMask(GL_TRUE);

	mv = LookAt(c_eye, c_at, up);
	//Shadow Garbage
	if (shadow == 1) {
		glUseProgram(program2);
		mv = LookAt(c_eye, c_at, up) * trans_p_to_q * Translate(x, y, z) *  Rotate(angle, rotAxisVector.x, rotAxisVector.y, rotAxisVector.z) * rotationAccumulation;

		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
		if (sphereFlag == 1) // Filled sphere
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe sphere
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(1.0);
		drawObj(sphere_shadow_buffer, sphere_NumVertices);
	}

	mv = LookAt(c_eye, c_at, up);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	if (lighting == 0) {
		glUseProgram(program2); // Use the shader program
		model_view = glGetUniformLocation(program2, "model_view");
		projection = glGetUniformLocation(program2, "projection");

		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
		glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
		if (floorFlag == 1) // Filled floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawObj(floor_buffer, floor_NumVertices);  // draw the floor
	}
	else {
		glUseProgram(program1); // Use the shader program
		model_view_2 = glGetUniformLocation(program1, "ModelView");
		projection_2 = glGetUniformLocation(program1, "Projection");

		glUniformMatrix4fv(model_view_2, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
		glUniformMatrix4fv(projection_2, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

		if (spotlight == 1 || pointsource == 1) {
			set_spot_or_point(mv, 1);
		}

		set_floor_m(mv);
		mat4 model_view_mod = mv;
		mat3 normal_matrix = NormalMatrix(model_view_mod, 1);
		glUniformMatrix3fv(glGetUniformLocation(program1, "Normal_Matrix"),
			1, GL_TRUE, normal_matrix);
		if (floorFlag == 1) // Filled floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawObj2(floor_light_buffer, floor_NumVertices);
	}
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	mv = LookAt(c_eye, c_at, up);


	glUseProgram(program2);
	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(2.0);
	drawLine(axis_buffer, axis_NumVertices);  // draw the axis


	/*---  Set up and pass on Model-View matrix to the shader ---*/
	mv = LookAt(c_eye, c_at, up);

	/*----- Set Up the Model-View matrix for the sphere -----*/
	mv = mv * Translate(x, y, z) * Rotate(angle, rotAxisVector.x, rotAxisVector.y, rotAxisVector.z) * rotationAccumulation * Scale(1, 1, 1);

	if (flatshading == 1) {
		glUseProgram(program1); // Use the shader program
		model_view_2 = glGetUniformLocation(program1, "ModelView");
		projection_2 = glGetUniformLocation(program1, "Projection");

		glUniformMatrix4fv(model_view_2, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
		glUniformMatrix4fv(projection_2, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

		if (spotlight == 1 || pointsource == 1) {
			set_spot_or_point(mv, 0);
		}

		set_sphere_m(mv);
		mat4 model_view_mod = mv;
		mat3 normal_matrix = NormalMatrix(model_view_mod, 1);
		glUniformMatrix3fv(glGetUniformLocation(program1, "Normal_Matrix"),
			1, GL_TRUE, normal_matrix);
		if (floorFlag == 1) // Filled floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawObj2(sphere_flat_buffer, sphere_NumVertices);
	}
	else if (smoothshading == 1) {
		glUseProgram(program1); // Use the shader program
		model_view_2 = glGetUniformLocation(program1, "ModelView");
		projection_2 = glGetUniformLocation(program1, "Projection");

		glUniformMatrix4fv(model_view_2, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
		glUniformMatrix4fv(projection_2, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

		if (spotlight == 1 || pointsource == 1) {
			set_spot_or_point(mv, 0);
		}

		set_sphere_m(mv);
		mat4 model_view_mod = mv;
		mat3 normal_matrix = NormalMatrix(model_view_mod, 1);
		glUniformMatrix3fv(glGetUniformLocation(program1, "Normal_Matrix"),
			1, GL_TRUE, normal_matrix);
		if (floorFlag == 1) // Filled floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe floor
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawObj2(sphere_smooth_buffer, sphere_NumVertices);

	}
	else {
		glUseProgram(program2);
		model_view = glGetUniformLocation(program2, "model_view");
		projection = glGetUniformLocation(program2, "projection");

		glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
		glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
		
		if (sphereFlag == 1) // Filled sphere
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		else              // Wireframe sphere
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(1.0);
		drawObj(sphere_buffer, sphere_NumVertices);  // draw the sphere
	}
	

	
	glutSwapBuffers();
}
//---------------------------------------------------------------------------
void idle(void)
{
	angle += 0.09;
	//angle += 1.0;    //YJC: change this value to adjust the sphere rotation speed.

	if (angle > 360.0) {
		angle = 0.0;
	}

	x += (0.02 * M_PI / 180.0) * (rollDirection[0] / length_points);
	z += (0.02 * M_PI / 180.0) * (rollDirection[2] / length_points);
	if (rotPoint == 0) {
		if (x > spherePointB.x || z < spherePointB.z) {
			rotationAccumulation = Rotate(angle, rotAxisVector.x, rotAxisVector.y, rotAxisVector.z) * rotationAccumulation;
			rotPoint = 1;
			rotAxisVector = cross(vec3(0.0, 1.0, 0.0), bc);

			rollDirection = bc;

			x = spherePointB.x;
			y = spherePointB.y;
			z = spherePointB.z;
			angle = 0;
			length_points = length(bc);
		}
	}
	else if (rotPoint == 1) {
		if (x > spherePointC.x || z > spherePointC.z) {
			rotationAccumulation = Rotate(angle, rotAxisVector.x, rotAxisVector.y, rotAxisVector.z) * rotationAccumulation;
			rotPoint = 2;
			rotAxisVector = cross(vec3(0.0, 1.0, 0.0), ca);

			rollDirection = ca;

			x = spherePointC.x;
			y = spherePointC.y;
			z = spherePointC.z;
			angle = 0;
			length_points = length(ca);
		}

	}
	else if (rotPoint == 2) {
		if (x < spherePointA.x || z < spherePointA.z) {
			rotationAccumulation = Rotate(angle, rotAxisVector.x, rotAxisVector.y, rotAxisVector.z) * rotationAccumulation;
			rotPoint = 0;
			rotAxisVector = cross(vec3(0.0, 1.0, 0.0), ab);

			rollDirection = ab;

			x = spherePointA.x;
			y = spherePointA.y;
			z = spherePointA.z;
			angle = 0;
			length_points = length(ab);
		}
	}



	glutPostRedisplay();
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033: // Escape Key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;

	case 'X': c_eye[0] += 1.0; break;
	case 'x': c_eye[0] -= 1.0; break;
	case 'Y': c_eye[1] += 1.0; break;
	case 'y': c_eye[1] -= 1.0; break;
	case 'Z': c_eye[2] += 1.0; break;
	case 'z': c_eye[2] -= 1.0; break;

	case 'b': case 'B': // Toggle between animation and non-animation
		animationFlag = 1 - animationFlag;
		if (bPress == false) bPress = true;
		if (animationFlag == 1) glutIdleFunc(idle);
		else                    glutIdleFunc(NULL);
		break;

	
	case 'c': case 'C': // Toggle between filled and wireframe sphere
		sphereFlag = 1 - sphereFlag;
		break;

	case 'f': case 'F': // Toggle between filled and wireframe floor
		floorFlag = 1 - floorFlag;
		break;
	
	}
	glutPostRedisplay();
}


void myMouse(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && bPress) {
		animationFlag = 1 - animationFlag;
		if (animationFlag == 1) glutIdleFunc(idle);
		else                    glutIdleFunc(NULL);
	}
}

void myMenu(int identification) {
	switch (identification) {
	case 1:
		c_eye = c_vrp;
		break;
	case 2:
		exit(EXIT_SUCCESS);
		break;
	case 3:
		sphereFlag = 0;
		smoothshading = 0;
		flatshading = 0;
		break;

	}
	
	glutPostRedisplay();
}

//----------------------------------------------------------------------------
void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	aspect = (GLfloat)width / (GLfloat)height;
	glutPostRedisplay();
}
//----------------------------------------------------------------------------


//Read Circle Vertices File
void file_in(std::string document) {
	std::ifstream myfile;
	myfile.open(document.c_str());
	if (!myfile) {
		std::cerr << "Unable to open file datafile.txt";
		exit(1);   // call system to stop
	}
	myfile >> triangleNumbers;
	sphere_NumVertices = triangleNumbers * 3;
	int threeRead;
	float one;
	float two;
	float three;
	sphereVertices = new point3[sphere_NumVertices];
	sphere_shadow_data = new point3[sphere_NumVertices];
	sphereColors = new color3[sphere_NumVertices];
	sphere_shadow_color = new color3[sphere_NumVertices];

	sphere_shade_regular = new point4[sphere_NumVertices];
	sphere_flat_shade = new point3[sphere_NumVertices];
	sphere_smooth_shade = new point3[sphere_NumVertices];


	int indexSphere = 0;
	while (myfile >> threeRead) {
		for (int i = 0; i < 3; i++) {
			for (int y = 0; i < 3; i++) {
				myfile >> one;
				myfile >> two;
				myfile >> three;
				point3 unmod_Point(one, two, three);
				point3 unmod_Point2(one, two, three);
				point4 unmod_Point3(one, two, three, 1);

				sphereVertices[indexSphere] = unmod_Point;
				sphere_shadow_data[indexSphere] = unmod_Point2;
				sphereColors[indexSphere] = color3(1.0, 0.84, 0);
				sphere_shadow_color[indexSphere] = color3(0.25, 0.25, 0.25);

				sphere_shade_regular[indexSphere] = unmod_Point3;

				indexSphere++;
				//std::cout << one << " " << two << " " << three << std::endl;
			}
		}
	}

	sphere_init_flat_normal();
	sphere_init_smooth_normal();




	myfile.close();
}

void shadow_menu(int index) {
	switch (index) {
	case 1:
		shadow = 0;
		break;
	case 2:
		shadow = 1;
		break;
	}
}

void lighting_menu(int index) {
	switch (index) {
	case 1:
		lighting = 0;
		smoothshading = 0;
		flatshading = 0;
		break;
	case 2:
		lighting = 1;
		break;
	}
}

void shading_menu(int index) {
	switch (index) {
	case 1:
		flatshading = 1;
		smoothshading = 0;
		if (lighting == 0) {
			flatshading = 0;
		}
		sphereFlag = 1;
		break;
	case 2:
		flatshading = 0;
		smoothshading = 1;
		if (lighting == 0) {
			smoothshading = 0;
		}
		sphereFlag = 1;
		break;
	}
}

void spotlight_menu(int index) {
	switch (index) {
	case 1:
		spotlight = 1;
		pointsource = 0;
		if (lighting == 0) {
			spotlight = 0;
		}
		break;
	case 2:
		pointsource = 1;
		spotlight = 0;
		if (lighting == 0) {
			pointsource = 0;
		}
		break;
	}

}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
#ifdef __APPLE__ // Enable core profile of OpenGL 3.2 on macOS.
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
#else
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
	glutInitWindowSize(512, 512);
	glutCreateWindow("Color Sphere");
	

#ifdef __APPLE__ // on macOS
	// Core profile requires to create a Vertex Array Object (VAO).
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
#else           // on Linux or Windows, we still need glew
	/* Call glewInit() and error checking */
	int err = glewInit();
	if (GLEW_OK != err)
	{
		printf("Error: glewInit failed: %s\n", (char*)glewGetErrorString(err));
		exit(1);
	}
#endif

	// Get info of GPU and supported OpenGL version
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

	//Here is file retrieval!
	std::string documentName;
	std::cout << "Insert document name" << std::endl;
	std::cin >> documentName;
	file_in(documentName);


	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(NULL);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(myMouse);

	int shadow_m = glutCreateMenu(shadow_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int shading_m = glutCreateMenu(shading_menu);
	glutAddMenuEntry("Flat Shading", 1);
	glutAddMenuEntry("Smooth Shading", 2);

	int lighting_m = glutCreateMenu(lighting_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	
	int spotlight_m = glutCreateMenu(spotlight_menu);
	glutAddMenuEntry("Spot Light", 1);
	glutAddMenuEntry("Point Light", 2);
	

	glutCreateMenu(myMenu);
	glutAddMenuEntry("Default View Point", 1);
	glutAddMenuEntry("Quit", 2);
	glutAddMenuEntry("Wireframe Sphere", 3);

	glutAddSubMenu("Shadow", shadow_m);
	glutAddSubMenu("Enable Lighting", lighting_m);
	glutAddSubMenu("Shading", shading_m);
	glutAddSubMenu("Light Source", spotlight_m);
	//glutAddSubMenu("Spot Light", spotlight);


	glutAttachMenu(GLUT_LEFT_BUTTON);



	init();
	glutMainLoop();
	return 0;
}
