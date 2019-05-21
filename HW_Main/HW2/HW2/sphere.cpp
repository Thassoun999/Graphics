#include "Angel-yjc.h"
#include <fstream>
#include <iostream>
#include <math.h>
#include <string>
#include <stdio.h>

using namespace std;
void display(void);
void idle(void);
typedef Angel::vec4 color4;
typedef Angel::vec4 point4;
typedef Angel::vec3 color3;
typedef Angel::vec3 point3;


#define ImageWidth  32
#define ImageHeight 32
GLubyte Image[ImageHeight][ImageWidth][4];
#define	stripeImageWidth 32
GLubyte stripeImage[4 * stripeImageWidth];



GLuint ModelView, Projection;
GLuint ModelViewFire, ProjectionFire;

int tickFrame = 1;
int floorFlagger = 1;
int fogFlag = 1;
int solidFlag = 1;
float shadeFlag = 1.0;
int texGround = 1;
int spotlight = 0;
int fireFlag = 1;
int vertFlag = 1;


GLfloat fovy = 45.0;
GLfloat aspect;
GLfloat zNear = 0.5, zFar = 13.0;
GLfloat angle = 0.0;
vec4 init_eye(7.0, 3.0, -10.0, 1.0);
vec4 eye = init_eye;

point3 spherePointA(-4, 1, 4);
point3 spherePointB(-1, 1, -4);
point3 spherePointC(3, 1, 5);

//Transformation values starting at point A
GLfloat x = spherePointA.x;
GLfloat y = spherePointA.y; //Most likely not affecting this one
GLfloat z = spherePointA.z;

int slantFlag = 0;
int eyeFlag = 0;
int objFlag = 1;
int texSphereFlag = 1;
int texSphereLine = 1;
int texSphereCheck = 0;
float elapsed = 0.0;
int latOn = 1;
int upLat = 0;
int blendShadFlag = 1;
int shadow = 1;
int flatshade = 0;
int smoothshade = 1;
int lighting = 1;
int pointsource = 1;

mat4 rotationAccumulation(vec4(1.0, 0.0, 0.0, 0.0), vec4(0.0, 1.0, 0.0, 0.0), vec4(0.0, 0.0, 1.0, 0.0), vec4(0.0, 0.0, 0.0, 1.0));
int rotPoint = 0;

vec3 ab = spherePointB - spherePointA;
vec3 bc = spherePointC - spherePointB;
vec3 ca = spherePointA - spherePointC;

//Initially set to this
vec3 rotAxisVector = cross(vec3(0.0, 1.0, 0.0), ab);
vec3 rollDirection = ab;
GLfloat length_points = length(ab);
mat4 trans_p_to_q(12, 0, 0, 0, 14, 0, 3, -1, 0, 0, 12, 0, 0, 0, 0, 12);

GLuint program1;
GLuint program2;

GLuint floor_buffer;
GLuint line_buffer;
GLuint sphere_buffer;
GLuint sphere_shadow_buffer;
GLuint sphere_flat_buffer;
GLuint sphere_smooth_buffer;
GLuint cube_buffer;
GLuint floor_light_buffer;
GLuint floor_tex_buffer;
GLuint fire_buffer;
static GLuint texName;
static GLuint lineTex;


bool startRoll = false, animationFlag = false;

point4 floor_light_points[6];
vec3 floor_normals[6];

const int floor_NumVertices = 6;
point4 floor_points[floor_NumVertices];
color3 floor_colors[floor_NumVertices];

point4 line_points[9];
color3 line_colors[9];

point4 *sphereData;
color3 *sphere_color;

int sphere_NumVertices;
int triangles;

point4 *sphere_shadow_data;
color3 *sphere_shadow_color;
point3 fireVector[300];
point3 fireColor[300];

//1 for point, 2 for spot
void set_point_or_spot_light(mat4 mv, int index) {
	color4 light_ambient(0, 0, 0, 1.0);
	color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
	color4 light_specular(1.0, 1.0, 1.0, 1.0);
	float spot_exp = 15.0;
	float spot_ang = cos(20.0 * M_PI / 180);
	float const_att = 2.0;
	float linear_att = 0.01;
	float quad_att = 0.001;
	point4 light_position(-14, 12.0, -3, 1.0);
	vec4 light_position_eyeFrame = mv * light_position;
	glUniform4fv(glGetUniformLocation(program1, "LightPosition"), 1,
		light_position_eyeFrame);

	glUniform1f(glGetUniformLocation(program1, "ConstAtt"), const_att);
	glUniform1f(glGetUniformLocation(program1, "LinearAtt"), linear_att);
	glUniform1f(glGetUniformLocation(program1, "QuadAtt"), quad_att);
	glUniform4fv(glGetUniformLocation(program1, "light_ambient"), 1,
		light_ambient);
	glUniform4fv(glGetUniformLocation(program1, "light_diffuse"), 1,
		light_diffuse);
	glUniform4fv(glGetUniformLocation(program1, "light_specular"), 1,
		light_specular);

	if (index == 1) { //point
		//glUniform1f(glGetUniformLocation(program1, "pointSource"), 1.0);
		glUniform1f(glGetUniformLocation(program1, "spotSource"), 0.0);
	}
	else if (index == 2) { //spot
		//glUniform1f(glGetUniformLocation(program1, "pointSource"), 0.0);
		glUniform1f(glGetUniformLocation(program1, "spotSource"), 1.0);
		point4 spot_direction = mv * vec4(-6.0, 0.0, -4.5, 1.0);
		glUniform4fv(glGetUniformLocation(program1, "spotDir"), 1,
			spot_direction);
		glUniform1f(glGetUniformLocation(program1, "spotExp"), spot_exp);
		glUniform1f(glGetUniformLocation(program1, "spotAng"), spot_ang);
	}

}

point4 *sphere_shade_d;
vec3 *sphere_flat_shade;
vec3 *sphere_smooth_shade;

vec2 floorTexCoord[6] = {
	vec2(0.0, 0.0),         
	vec2(0.0, 6 ), 
	vec2(5, 6),

	vec2(5, 6), 
	vec2(5, 0.0), 
	vec2(0.0, 0.0),
};

point4 floorTexVert[6] = {
	point4(-5, 0, -4, 1), 
	point4(-5, 0, 8, 1), 
	point4(5, 0, 8, 1),

	point4(5, 0, 8, 1),   
	point4(5, 0, -4, 1), 
	point4(-5, 0, -4, 1),
};



point3 f_tex_normals[6] = {
	point3(0, 1, 0), point3(0, 1, 0), point3(0, 1, 0),
	point3(0, 1, 0), point3(0, 1, 0), point3(0, 1, 0),
};

/*************************************************************
void image_set_up(void):
  generate checkerboard and stripe images.

* Inside init(), call this function and set up texture objects
  for texture mapping.
  (init() is called from main() before calling glutMainLoop().)
***************************************************************/


void image_set_up(void)
{
	int i, j, c;

	/* --- Generate checkerboard image to the image array ---*/
	for (i = 0; i < ImageHeight; i++)
		for (j = 0; j < ImageWidth; j++)
		{
			c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));

			if (c == 1) /* white */
			{
				c = 255;
				Image[i][j][0] = (GLubyte)c;
				Image[i][j][1] = (GLubyte)c;
				Image[i][j][2] = (GLubyte)c;
			}
			else  /* green */
			{
				Image[i][j][0] = (GLubyte)0;
				Image[i][j][1] = (GLubyte)150;
				Image[i][j][2] = (GLubyte)0;
			}

			Image[i][j][3] = (GLubyte)255;
		}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*--- Generate 1D stripe image to array stripeImage[] ---*/
	for (j = 0; j < stripeImageWidth; j++) {
		/* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
		   When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture
		 */
		stripeImage[4 * j] = (GLubyte)255;
		stripeImage[4 * j + 1] = (GLubyte)((j > 4) ? 255 : 0);
		stripeImage[4 * j + 2] = (GLubyte)0;
		stripeImage[4 * j + 3] = (GLubyte)255;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	/*----------- End 1D stripe image ----------------*/

	/*--- texture mapping set-up is to be done in
		  init() (set up texture objects),
		  display() (activate the texture object to be used, etc.)
		  and in shaders.
	 ---*/

} /* end function */



void drawObj_2(GLuint buffer, int num_vertices) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	GLuint vPosition = glGetAttribLocation(program1, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program1, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point4) * num_vertices));
	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
}

void init() {
	image_set_up();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &texName);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texName);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, Image);

	glGenBuffers(1, &floor_tex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_tex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floorTexVert) + sizeof(f_tex_normals) + sizeof(floorTexCoord), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floorTexVert), floorTexVert);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floorTexVert), sizeof(f_tex_normals), f_tex_normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floorTexVert) + sizeof(f_tex_normals), sizeof(floorTexCoord), floorTexCoord);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &lineTex); // Generate texture obj name(s)

	glActiveTexture(GL_TEXTURE1); // Set the active texture unit to be 0
	glBindTexture(GL_TEXTURE_1D, lineTex); // Bind the texture to this texture unit

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);

	//Sphere
	for (int i = 0; i < triangles * 3; i++) {
		sphere_color[i] = color3(1.0, 0.84, 0.0);
		sphere_shadow_color[i] = color3(0.25, 0.25, 0.25);
	}

	//Floor
	floor_points[0] = point3(5.0, 0.0, 8.0); floor_colors[0] = color3(0.0, 1.0, 0.0);
	floor_points[1] = point3(5.0, 0.0, -4.0); floor_colors[1] = color3(0.0, 1.0, 0.0);
	floor_points[2] = point3(-5.0, 0.0, 8.0); floor_colors[2] = color3(0.0, 1.0, 0.0);

	floor_points[3] = point3(5.0, 0.0, -4.0); floor_colors[3] = color3(0.0, 1.0, 0.0);
	floor_points[4] = point3(-5.0, 0.0, 8.0); floor_colors[4] = color3(0.0, 1.0, 0.0);
	floor_points[5] = point3(-5.0, 0.0, -4.0); floor_colors[5] = color3(0.0, 1.0, 0.0);

	glGenBuffers(1, &floor_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors), floor_colors);

	//Floor Lights
	vec4 u = point4(5, 0, 8, 1.0) - point4(5, 0, -4, 1.0);
	vec4 v = point4(-5, 0, -4, 1.0) - point4(5, 0, -4, 1.0);

	vec3 normalFl = normalize(cross(u, v));

	floor_light_points[0] = point4(5, 0, -4, 1.0); floor_normals[0] = normalFl;
	floor_light_points[1] = point4(5, 0, 8, 1.0); floor_normals[1] = normalFl;
	floor_light_points[2] = point4(-5, 0, 8, 1.0); floor_normals[2] = normalFl;

	floor_light_points[3] = point4(5, 0, -4, 1.0); floor_normals[3] = normalFl;
	floor_light_points[4] = point4(-5, 0, 8, 1.0); floor_normals[4] = normalFl;
	floor_light_points[5] = point4(-5, 0, -4, 1.0); floor_normals[5] = normalFl;

	glGenBuffers(1, &floor_light_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_light_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_light_points) + sizeof(floor_normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_light_points), floor_light_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_light_points), sizeof(floor_normals), floor_normals);

	glGenBuffers(1, &sphere_flat_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_flat_buffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * triangles * sizeof(point4) + 3 * triangles * sizeof(color3), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * triangles * sizeof(point4), sphere_shade_d);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * triangles * sizeof(point4), 3 * triangles * sizeof(color3), sphere_flat_shade);

	glGenBuffers(1, &sphere_smooth_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_smooth_buffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * triangles * sizeof(point4) + 3 * triangles * sizeof(color3), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * triangles * sizeof(point4), sphere_shade_d);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * triangles * sizeof(point4), 3 * triangles * sizeof(color3), sphere_smooth_shade);

	//Axis
	line_colors[0] = color3(1.0, 0.0, 0.0); line_points[0] = point4(0, 0, 0, 1);
	line_colors[1] = color3(1.0, 0.0, 0.0); line_points[1] = point4(10, 0, 0, 1);
	line_colors[2] = color3(1.0, 0.0, 0.0); line_points[2] = point4(20, 0, 0, 1);

	line_colors[3] = color3(1.0, 0.0, 1.0); line_points[3] = point4(0, 0, 0, 1);
	line_colors[4] = color3(1.0, 0.0, 1.0); line_points[4] = point4(0, 10, 0, 1);
	line_colors[5] = color3(1.0, 0.0, 1.0); line_points[5] = point4(0, 20, 0, 1);

	line_colors[6] = color3(0.0, 0.0, 1.0); line_points[6] = point4(0, 0, 0, 1);
	line_colors[7] = color3(0.0, 0.0, 1.0); line_points[7] = point4(0, 0, 10, 1);
	line_colors[8] = color3(0.0, 0.0, 1.0); line_points[8] = point4(0, 0, 20, 1);

	glGenBuffers(1, &line_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line_points) + sizeof(line_colors), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line_points), line_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(line_points), sizeof(line_colors), line_colors);

	//Fireworks
	for (int i = 0; i < 300; i++) {
		vec3 tmp_v = vec3(2.0 * ((rand() % 256) / 256.0 - 0.5), 2.4 * (rand() % 256) / 256.0, 2.0 * ((rand() % 256) / 256.0 - 0.5));
		vec3 tmp_c = vec3((rand() % 256) / 256.0, (rand() % 256) / 256.0, (rand() % 256) / 256.0);
		fireVector[i] = tmp_v;
		fireColor[i] = tmp_c;
	}

	glGenBuffers(1, &fire_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, fire_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fireVector) + sizeof(fireColor), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fireVector), fireVector);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(fireVector), sizeof(fireColor), fireColor);

	//Set Sphere and Shadow Buffers
	glGenBuffers(1, &sphere_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * triangles * sizeof(point4) + 3 * triangles * sizeof(color3), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * triangles * sizeof(point4), sphereData);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * triangles * sizeof(point4), 3 * triangles * sizeof(color3), sphere_color);

	glGenBuffers(1, &sphere_shadow_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_shadow_buffer);
	glBufferData(GL_ARRAY_BUFFER, 3 * triangles * sizeof(point4) + 3 * triangles * sizeof(color3), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * triangles * sizeof(point4), sphere_shadow_data);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * triangles * sizeof(point4), 3 * triangles * sizeof(color3), sphere_shadow_color);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glLineWidth(2.0);

	program1 = InitShader("finalvshader.glsl", "finalfshader.glsl");
	program2 = InitShader("customfireV.glsl", "customfireF.glsl");
}

void drawObj(GLuint buffer, int num_vertices) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	GLuint vPosition = glGetAttribLocation(program1, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program1, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(num_vertices * sizeof(point4)));

	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vNormal);
}




void draw_floor(mat4 mv, mat4 eye_frame) {
	if (lighting == 0) {
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawObj_2(floor_buffer, floor_NumVertices); // draw the floor
	}
	else {
		if (pointsource) {
			set_point_or_spot_light(eye_frame, 1); //point
		}
		else {
			set_point_or_spot_light(eye_frame , 2); //spot
		}
		color4 ambience(0.2, 0.2, 0.2, 1.0);
		glUniform4fv(glGetUniformLocation(program1, "ini_material_ambient"), 1, ambience);

		color4 diffuse(0, 1.0, 0, 1);
		glUniform4fv(glGetUniformLocation(program1, "ini_material_diffuse"), 1, diffuse);

		color4 specular(0, 0, 0, 1);
		glUniform4fv(glGetUniformLocation(program1, "ini_material_specular"), 1, specular);

		float shine = 125.0;

		glUniform1f(glGetUniformLocation(program1, "shine"), 1.0);
	
		mat4 model_view = mv;
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
		mat3 normal_matrix = NormalMatrix(model_view, 1);
		glUniformMatrix3fv(glGetUniformLocation(program1, "Normal_Matrix"), 1,
			GL_TRUE, normal_matrix);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (texGround == 1) {
			glUniform1i(glGetUniformLocation(program1, "texture_1D"), 1);
			glUniform1i(glGetUniformLocation(program1, "texture_2D"), 0);
			glUniform1i(glGetUniformLocation(program1, "text_ground_flag"), 1);
			// drawObj_3(f_tex_buffer, 6);

			glBindBuffer(GL_ARRAY_BUFFER, floor_tex_buffer);

			GLuint vPosition = glGetAttribLocation(program1, "vPosition");
			glEnableVertexAttribArray(vPosition);
			glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				BUFFER_OFFSET(0));

			GLuint vNormal = glGetAttribLocation(program1, "vNormal");
			glEnableVertexAttribArray(vNormal);
			glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
				BUFFER_OFFSET(6 * sizeof(point4)));

			GLuint vTexCoord = glGetAttribLocation(program1, "vTexCoord");
			glEnableVertexAttribArray(vTexCoord);
			glVertexAttribPointer(
				vTexCoord, 2, GL_FLOAT, GL_FALSE, 0,
				BUFFER_OFFSET(6 * (sizeof(point4) + sizeof(point3))));

			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(vTexCoord);

			glDisableVertexAttribArray(vPosition);
			glDisableVertexAttribArray(vNormal);

		}
		else {
			drawObj(floor_light_buffer, 6);
		}
		glUniform1i(glGetUniformLocation(program1, "text_ground_flag"), 0);
	}
};



void set_flag_sphere_tex() {
	glUniform1i(glGetUniformLocation(program1, "vertical_flag"), vertFlag);
	glUniform1i(glGetUniformLocation(program1, "slant_flag"), slantFlag);
	glUniform1i(glGetUniformLocation(program1, "eye_space_flag"), eyeFlag);
	glUniform1i(glGetUniformLocation(program1, "object_space_flag"),
		objFlag);
	glUniform1i(glGetUniformLocation(program1, "sphere_line_flag"),
		texSphereLine);
	glUniform1i(glGetUniformLocation(program1, "f_sphere_flag"),
		texSphereLine);
	glUniform1i(glGetUniformLocation(program1, "sphere_check_flag"),
		texSphereCheck);
	glUniform1i(glGetUniformLocation(program1, "f_sphere_check_flag"),
		texSphereCheck);
	glUniform1i(glGetUniformLocation(program1, "upright_lat_flag"),
		upLat);
	glUniform1i(glGetUniformLocation(program1, "enable_lat"), latOn);
};
void draw_shadow(mat4 mv) {
	glUniform1i(glGetUniformLocation(program1, "draw_shadow_lat"), 1);
	glUniform1i(glGetUniformLocation(program1, "f_draw_shadow_lat"), 1);
	glUniform1i(glGetUniformLocation(program1, "upright_lat_flag"),
		upLat);
	if (shadow == 1) {
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);
		if (solidFlag == 0) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		if (blendShadFlag == 1) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glUniform1f(glGetUniformLocation(program1, "shadeFlag"), 0);
			glUniform1f(glGetUniformLocation(program1, "shadow_semi_flag"), 1.0);
			drawObj_2(sphere_shadow_buffer, sphere_NumVertices);
			glDisable(GL_BLEND);
		}
		else {
			glUniform1f(glGetUniformLocation(program1, "shadeFlag"), 0);
			glUniform1f(glGetUniformLocation(program1, "shadow_semi_flag"), 0);
			drawObj_2(sphere_shadow_buffer, sphere_NumVertices);
		}
	}
	glUniform1i(glGetUniformLocation(program1, "draw_shadow_lat"), 0);
	glUniform1i(glGetUniformLocation(program1, "f_draw_shadow_lat"), 0);
};
void cancel_flag_sphere() {
	glUniform1i(glGetUniformLocation(program1, "f_sphere_flag"), 0);
	glUniform1i(glGetUniformLocation(program1, "f_sphere_check_flag"), 0);
	glUniform1i(glGetUniformLocation(program1, "eye_space_flag"), 0);
	glUniform1i(glGetUniformLocation(program1, "object_space_flag"), 0);
	glUniform1i(glGetUniformLocation(program1, "sphere_line_flag"), 0);
	glUniform1i(glGetUniformLocation(program1, "sphere_check_flag"), 0);
}




void display(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.529, 0.807, 0.92, 0.0);
	mat4 p = Perspective(fovy, aspect, zNear, zFar);

	vec4 at(0.0, 0.0, 0.0, 1.0);
	vec4 up(0.0, 1.0, 0.0, 0.0);
	mat4 mv = LookAt(eye, at, up);
	mat4 eye_frame = LookAt(eye, at, up);

	glUseProgram(program1);


	shadeFlag = 0;
	if (solidFlag == 0 || lighting == 0) {
		flatshade = 0;
		smoothshade = 0;
	}

	if (lighting == 1) {
		shadeFlag = 1.0;
	}

	glUniform1f(glGetUniformLocation(program1, "shadeFlag"), shadeFlag * 1.0);

	glUniform1i(glGetUniformLocation(program1, "text_ground_flag"), 0);
	glUniform1i(glGetUniformLocation(program1, "f_sphere_flag"), 0);
	glUniform1i(glGetUniformLocation(program1, "vertical_flag"), 1);
	glUniform1i(glGetUniformLocation(program1, "slant_flag"), 0);
	glUniform1i(glGetUniformLocation(program1, "eye_space_flag"), 0);
	glUniform1i(glGetUniformLocation(program1, "object_space_flag"), 1);

	glUniform1f(glGetUniformLocation(program1, "fog_flag"), fogFlag * 1.0);

	ModelView = glGetUniformLocation(program1, "ModelView");
	Projection = glGetUniformLocation(program1, "Projection");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
	glEnable(GL_DEPTH_TEST);

	if (blendShadFlag == 1) {
		glDepthMask(GL_FALSE);
		draw_floor(mv, eye_frame);
		mv = LookAt(eye, at, up) * trans_p_to_q * Translate(x, y, z) * Rotate(angle, rotAxisVector.x, rotAxisVector.y, rotAxisVector.z) * rotationAccumulation;
		if (eye[1] > 0)
			draw_shadow(mv);

		glDepthMask(GL_TRUE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		mv = LookAt(eye, at, up);
		draw_floor(mv, eye_frame);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	else {
		glDepthMask(GL_FALSE);
		draw_floor(mv, eye_frame);

		glDepthMask(GL_TRUE);
		mv = LookAt(eye, at, up) * trans_p_to_q * Translate(x, y, z) * Rotate(angle, rotAxisVector.x, rotAxisVector.y, rotAxisVector.z) * rotationAccumulation;
		if (eye[1] > 0)
			draw_shadow(mv);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		mv = LookAt(eye, at, up);
		draw_floor(mv, eye_frame);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

	glUniform1f(glGetUniformLocation(program1, "shadeFlag"), 0);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, eye_frame);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawObj_2(line_buffer, 9);

	if (lighting == 1)
		glUniform1f(glGetUniformLocation(program1, "shadeFlag"), 1.0);


	mv = LookAt(eye, at, up) * Translate(x, y, z) * Rotate(angle, rotAxisVector.x, rotAxisVector.y, rotAxisVector.z) * rotationAccumulation;

	if (flatshade == 1) {
		if (pointsource) {
			set_point_or_spot_light(eye_frame, 1); // point
		}
		else {
			set_point_or_spot_light(eye_frame, 2); // spot
		}
		color4 material_ambient(0.2, 0.2, 0.2, 1.0);
		color4 material_diffuse(1.0, 0.84, 0, 1);
		color4 material_specular(1.0, 0.84, 0, 1);
		float material_shininess = 125;
		glUniform4fv(glGetUniformLocation(program1, "ini_material_ambient"), 1,
			material_ambient);
		glUniform4fv(glGetUniformLocation(program1, "ini_material_diffuse"), 1,
			material_diffuse);
		glUniform4fv(glGetUniformLocation(program1, "ini_material_specular"), 1,
			material_specular);
		glUniform1f(glGetUniformLocation(program1, "Shininess"), material_shininess);

		mat4 model_view = mv;
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
		mat3 normal_matrix = NormalMatrix(model_view, 1);
		glUniformMatrix3fv(glGetUniformLocation(program1, "Normal_Matrix"), 1,
			GL_TRUE, normal_matrix);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (texSphereFlag == 1) {
			set_flag_sphere_tex();
			glUniform1i(glGetUniformLocation(program1, "texture_1D"), 1);
			glUniform1i(glGetUniformLocation(program1, "texture_2D"), 0);
			drawObj(sphere_flat_buffer, triangles * 3);
			cancel_flag_sphere();
		}
		else {
			drawObj(sphere_flat_buffer, triangles * 3);
		}
	}

	else if (smoothshade == 1) {
		if (pointsource) {
			set_point_or_spot_light(eye_frame, 1); // point
		}
		else {
			set_point_or_spot_light(eye_frame, 2); // spot
		}
		color4 material_ambient(0.2, 0.2, 0.2, 1.0);
		color4 material_diffuse(1.0, 0.84, 0, 1);
		color4 material_specular(1.0, 0.84, 0, 1);
		float material_shininess = 125;
		glUniform4fv(glGetUniformLocation(program1, "ini_material_ambient"), 1,
			material_ambient);
		glUniform4fv(glGetUniformLocation(program1, "ini_material_diffuse"), 1,
			material_diffuse);
		glUniform4fv(glGetUniformLocation(program1, "ini_material_specular"), 1,
			material_specular);
		glUniform1f(glGetUniformLocation(program1, "Shininess"), material_shininess);
		mat4 model_view = mv;
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
		mat3 normal_matrix = NormalMatrix(model_view, 1);
		glUniformMatrix3fv(glGetUniformLocation(program1, "Normal_Matrix"), 1,
			GL_TRUE, normal_matrix);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (texSphereFlag == 1) {
			set_flag_sphere_tex();
			glUniform1i(glGetUniformLocation(program1, "texture_1D"), 1);
			glUniform1i(glGetUniformLocation(program1, "texture_2D"), 0);
			glUniform1i(glGetUniformLocation(program1, "my_texture_2D"), 2);
			drawObj(sphere_smooth_buffer, triangles * 3);
			cancel_flag_sphere();
		}
		else {
			drawObj(sphere_smooth_buffer, triangles * 3);
		}
	}

	else {
		glUniform1f(glGetUniformLocation(program1, "shadeFlag"), 0);
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);
		if (solidFlag == 0) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		drawObj_2(sphere_buffer, sphere_NumVertices);
	}

	glUseProgram(program2);
	ModelViewFire = glGetUniformLocation(program2, "model_view");
	ProjectionFire = glGetUniformLocation(program2, "projection");
	glUniformMatrix4fv(ProjectionFire, 1, GL_TRUE, p);
	glUniformMatrix4fv(ModelViewFire, 1, GL_TRUE, eye_frame);
	glPointSize(3.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	float t = glutGet(GLUT_ELAPSED_TIME);
	int time = int((t - elapsed)) % 5000;
	if (startRoll == true)
		glUniform1f(glGetUniformLocation(program2, "time"), time);
	else
		glUniform1f(glGetUniformLocation(program2, "time"), 0);

	if (fireFlag == 1) {

		glBindBuffer(GL_ARRAY_BUFFER, fire_buffer);

		GLuint velocity = glGetAttribLocation(program2, "velocity");
		glEnableVertexAttribArray(velocity);
		glVertexAttribPointer(velocity, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

		GLuint vColor = glGetAttribLocation(program2, "vColor");
		glEnableVertexAttribArray(vColor);
		glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(point3) * 300));
		glDrawArrays(GL_TRIANGLES, 0, 300);
		glDisableVertexAttribArray(velocity);
		glDisableVertexAttribArray(vColor);
	}
	glutSwapBuffers();
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	aspect = (GLfloat)w / (GLfloat)h;
	glutPostRedisplay();
}

void idle(void) {
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


void mouse(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && startRoll) {
		animationFlag =  1- animationFlag;
		if (animationFlag == 1) glutIdleFunc(idle);
		else                    glutIdleFunc(NULL);
		
		
	}
	if (animationFlag) {
		glutIdleFunc(idle);
	}
	else {
		glutIdleFunc(NULL);
	}
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 033: // Escape Key
	case 'q':
	case 'Q':
		exit(EXIT_SUCCESS);
		break;
	case 'b':
	case 'B': // start Rolling
		elapsed = glutGet(GLUT_ELAPSED_TIME);
		startRoll = true;
		glutIdleFunc(idle);
		break;
	case 'X':
		eye[0] += 1.0;
		break;
	case 'x':
		eye[0] -= 1.0;
		break;
	case 'Y':
		eye[1] += 1.0;
		break;
	case 'y':
		eye[1] -= 1.0;
		break;
	case 'Z':
		eye[2] += 1.0;
		break;
	case 'z':
		eye[2] -= 1.0;
		break;

	case 'v':
	case 'V':
		vertFlag = 1;
		slantFlag = 0;
		break;
	case 's':
	case 'S':
		vertFlag = 0;
		slantFlag = 1;
		break;
	case 'o':
	case 'O':
		objFlag = 1;
		eyeFlag = 0;
		break;
	case 'e':
	case 'E':
		objFlag = 0;
		eyeFlag = 1;
		break;
	case 'u':
	case 'U':
		upLat = 0;
		break;
	case 't':
	case 'T':
		upLat = 1;
		break;
	case 'l':
	case 'L':
		latOn = 1 - latOn;
		break;

	case 'a':
	case 'A':
		tickFrame = 1 - tickFrame;
		if (tickFrame == 1)
			glutIdleFunc(idle);
		else
			glutIdleFunc(NULL);
		break;

	case ' ':
		eye = init_eye;
		break;
	}
	glutPostRedisplay();
}

void main_menu(int index) {
	if (index == 0) {
		eye = init_eye;
	}
	else if (index == 1) {
		exit(1);
	}
	else if (index == 2) {
		solidFlag = !solidFlag;
	}
	display();
}

void shadow_menu(int index) {
	if (index == 1) {
		shadow = false;
	}
	else {
		shadow = true;
	}
	display();
}

void shade_menu(int index) {
	flatshade = (index == 1) ? true : false;
	if (flatshade) {
		flatshade = flatshade;
		
	}
	else {
		smoothshade = !smoothshade;
		if (smoothshade == 1) {
			flatshade = 0;
		}
		
	}
	solidFlag = 1;
	display();
};

void lighting_menu(int index) {
	lighting = (index == 1) ? false : true;
	display();
};

void spotlight_menu(int index) {
	spotlight = (index == 1) ? true : false;
	if (spotlight) {
		spotlight = 1;
		pointsource = 0;
	}
	else {
		spotlight = 0;
		pointsource = 1;
	}
	display();
};

void fog_menu(int index) {
	if (index >= 0)
		fogFlag = index;
	else
		fogFlag = 1;
	display();
};

void tex_ground_menu(int index) {
	texGround = (index == 2) ? 1 : 0;
	display();
}

void tex_sphere_menu(int index) {
	if (index == 1) {
		texSphereFlag = 0;
	}
	if (index == 2) {
		texSphereFlag = 1;
		texSphereLine = 1;
		texSphereCheck = 0;
	}
	if (index == 3) {
		texSphereFlag = 1;
		texSphereLine = 0;
		texSphereCheck = 1;
	}
	display();
}

void blend_shadow_menu(int index) {
	blendShadFlag = (index == 2) ? 1 : 0;
	display();
}

void firework_menu(int index) {
	fireFlag = (index == 2) ? 1 : 0;
	display();
}




void read_file() {
	ifstream fp;
	string filename;

	int points;
	float temp[3];

	int count = 0;
	cout << "Enter the file name: " << endl;

	cin >> filename;

	fp.open(filename);
	if (!fp) {
		cerr << "Couldn't open " << filename << " !" << endl;
		exit(0);
	}

	fp >> triangles;
	cout << triangles << endl;

	sphereData = new point4[triangles * 3];
	sphere_shadow_data = new point4[triangles * 3];
	sphere_color = new point3[triangles * 3];
	sphere_shadow_color = new point3[triangles * 3];
	sphere_shade_d = new point4[triangles * 3];
	sphere_flat_shade = new point3[triangles * 3];
	sphere_smooth_shade = new point3[triangles * 3];

	for (int i = 0; i < triangles; i++) {
		fp >> points;
		for (int j = 0; j < points; j++) {
			for (int k = 0; k < 3; k++) {
				fp >> temp[k];
			}
			point4 tmp = vec4(temp[0], temp[1], temp[2], 1);
			point4 tmp2 = vec4(temp[0], temp[1], temp[2], 1);
			point4 tmp_shade = vec4(temp[0], temp[1], temp[2], 1);

			sphereData[count] = tmp;
			sphere_shadow_data[count] = tmp2;
			sphere_shade_d[count] = tmp_shade;
			count++;
		}
	}
	sphere_NumVertices = triangles * 3;
	fp.close();

	int flat_index = 0;
	for (int j = 0; j < triangles; j++) {

		vec4 curr_p1 = sphere_shade_d[3 * j];
		vec4 curr_p2 = sphere_shade_d[3 * j + 1];
		vec4 curr_p3 = sphere_shade_d[3 * j + 2];

		vec4 u = curr_p2 - curr_p1;
		vec4 v = curr_p3 - curr_p1;

		vec3 tmp_normal = normalize(cross(u, v));
		sphere_flat_shade[flat_index] = tmp_normal;
		flat_index++;
		sphere_flat_shade[flat_index] = tmp_normal;
		flat_index++;
		sphere_flat_shade[flat_index] = tmp_normal;
		flat_index++;
	}

	for (int i = 0; i < triangles * 3; i++) {
		vec4 curr_p = sphere_shade_d[i];
		vec3 tmp = vec3(curr_p[0], curr_p[1], curr_p[2]);
		tmp = normalize(tmp);
		sphere_smooth_shade[i] = tmp;
	}
}

int main(int argc, char **argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutCreateWindow("assignment4");

	int glewVal = glewInit();
	if (GLEW_OK != glewVal) {
		cout << "The bad happend :(" << endl;
		exit(1);
	}
	read_file();
	int shadow = glutCreateMenu(shadow_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int shade = glutCreateMenu(shade_menu);
	glutAddMenuEntry("flat shading", 1);
	glutAddMenuEntry("smooth shading", 2);

	int lighting = glutCreateMenu(lighting_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int tex_ground_m = glutCreateMenu(tex_ground_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int tex_sphere_m = glutCreateMenu(tex_sphere_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes-Contour Lines", 2);
	glutAddMenuEntry("Yes-Checkerboard", 3);

	int blend_shadow_m = glutCreateMenu(blend_shadow_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int firework_m = glutCreateMenu(firework_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int spotlight = glutCreateMenu(spotlight_menu);
	glutAddMenuEntry("Spot light", 1);
	glutAddMenuEntry("Point light", 2);

	int fog_m_option = glutCreateMenu(fog_menu);
	glutAddMenuEntry("No fog", 1);
	glutAddMenuEntry("Linear fog", 2);
	glutAddMenuEntry("Exponential fog", 3);
	glutAddMenuEntry("Exponential square fog", 4);

	glutCreateMenu(main_menu);
	glutAddMenuEntry("Default View Point", 0);
	glutAddMenuEntry("Quit", 1);
	glutAddMenuEntry("Wire Frame Sphere", 2);
	glutAddSubMenu("Enable Lighting", lighting);
	glutAddSubMenu("Shadow", shadow);
	glutAddSubMenu("Shading", shade);
	glutAddSubMenu("Lighting", spotlight);
	glutAddSubMenu("Fog option", fog_m_option);
	glutAddSubMenu("Texture Mapped Ground", tex_ground_m);
	glutAddSubMenu("Texture Mapped Sphere", tex_sphere_m);
	glutAddSubMenu("Blending Shadow", blend_shadow_m);
	glutAddSubMenu("Fireworks", firework_m);

	glutAttachMenu(GLUT_LEFT_BUTTON);


	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	init();
	glutMainLoop();

}
