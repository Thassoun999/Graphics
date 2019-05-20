#include "Angel-yjc.h"
#pragma comment(lib, "glew32.lib")

#include <fstream>
#include <iostream>
#include <math.h>
#include <string>

using namespace std;

typedef Angel::vec4 color4;
typedef Angel::vec4 point4;
typedef Angel::vec3 color3;
typedef Angel::vec3 point3;

void display(void);
void idle(void);

#define ImageWidth  32
#define ImageHeight 32
GLubyte Image[ImageHeight][ImageWidth][4];

#define	stripeImageWidth 32
GLubyte stripeImage[4 * stripeImageWidth];

#define pie 3.14159265358979323846
#define sqrt3f(x, y, z) sqrt(x *x + y * y + z * z)


GLfloat radius;
GLfloat fovy = 45.0;
GLfloat aspect;
GLfloat zNear = 0.5, zFar = 13.0;
GLfloat angle = 0.0;
vec4 init_eye(7.0, 3.0, -10.0, 1.0);
vec4 eye = init_eye;

point3 position_array[] = { point3(-4, 1.0, 4.0), point3(-1.0, 1.0, -4.0),
				  point3(3.0, 1.0, 5.0) };


point3 *vectors;
GLfloat theta = 0.0, delta = 0.1;
int global_step = 0, totalSegments = 3;
point3 middle_jam = position_array[global_step];
point3 *rotationAxis;


GLuint program;
GLuint custom_prog;

GLuint cube_buffer;
GLuint floor_light_buffer;
GLuint floor_buffer;
GLuint line_buffer;
GLuint sphere_buffer;
GLuint sphere_shadow_buffer;
GLuint s_flat_buffer;
GLuint s_smooth_buffer;
GLuint f_tex_buffer;
GLuint firework_buffer;
static GLuint texName;
static GLuint line_texName;

GLuint ModelView, Projection;


float get_dist(point3 p1, point3 p2);
mat4 acc_matrix = Angel::identity();
bool begin1 = false, rolling = false;

point4 floor_light_points[6];
vec3 floor_normals[6];

const int floor_NumVertices =
6;
point4 floor_points[floor_NumVertices];
color3 floor_colors[floor_NumVertices];

point4 *sphereData;
color3 *sphere_color;

int sphere_NumVertices;
int triangles;

point4 *sphere_shadow_data;
color3 *sphere_shadow_color;

point4 *sphere_shade_d;
vec3 *sphere_flat_shade;
vec3 *sphere_smooth_shade;

vec2 f_tex_coord[6] = {
	vec2(0.0, 0.0),         vec2(0.0, 6), vec2(5 , 6 ),
	vec2(5, 6), vec2(5, 0.0), vec2(0.0, 0.0),
};

color3 vertex_colors[8] = {
	color3(0.0, 0.0, 0.0),
	color3(1.0, 0.0, 0.0),
	color3(1.0, 0.84, 0.0),
	color3(0.0, 1.0, 0.0),
	color3(0.0, 0.0, 1.0),
	color3(1.0, 0.0, 1.0),
	color3(1.0, 1.0, 1.0),
	color3(0.0, 1.0, 1.0)
};


point4 f_tex_vertices[6] = {
	point4(-5, 0, -4, 1), point4(-5, 0, 8, 1), point4(5, 0, 8, 1),
	point4(5, 0, 8, 1),   point4(5, 0, -4, 1), point4(-5, 0, -4, 1),
};

point3 f_tex_normals[6] = {
	point3(0, 1, 0), point3(0, 1, 0), point3(0, 1, 0),
	point3(0, 1, 0), point3(0, 1, 0), point3(0, 1, 0),
};

int frame_ticks = 1;
int floorFlag = 1;
int fog_flag = 1;
int solid_Flag = 1;
float shade_flag = 1.0;
int text_ground_flag = 1;
int blend_shadow_flag = 1;
int shadow = 1;
int flatshade = 0;
int smoothshade = 1;
int lighting = 1;
int pointsource = 1;
int spotlight = 0;
int firework_flag = 1;
int vertical_flag = 1;
int slant_flag = 0;
int eye_space_flag = 0;
int object_space_flag = 1;
int text_sphere_flag = 1;
int text_sphere_line_flag = 1;
int text_sphere_check_flag = 0;
float t_sub = 0.0;
int enable_lat = 1;
int upright_lat_flag = 0;


void set_spot_light(mat4 mv) {
	color4 light_ambient(0, 0, 0, 1.0);
	color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
	color4 light_specular(1.0, 1.0, 1.0, 1.0);
	float const_att = 2.0;
	float linear_att = 0.01;
	float quad_att = 0.001;
	point4 light_position(-14, 12.0, -3, 1.0);

	float spot_exp = 15.0;
	float spot_ang = cos(20.0 * pie / 180);

	vec4 light_position_eyeFrame = mv * light_position;
	glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1,
		light_position_eyeFrame);

	glUniform1f(glGetUniformLocation(program, "ConstAtt"), const_att);
	glUniform1f(glGetUniformLocation(program, "LinearAtt"), linear_att);
	glUniform1f(glGetUniformLocation(program, "QuadAtt"), quad_att);
	glUniform4fv(glGetUniformLocation(program, "input_light_ambient"), 1,
		light_ambient);
	glUniform4fv(glGetUniformLocation(program, "input_light_diffuse"), 1,
		light_diffuse);
	glUniform4fv(glGetUniformLocation(program, "input_light_specular"), 1,
		light_specular);
	glUniform1f(glGetUniformLocation(program, "point_flag"), 0.0);
	glUniform1f(glGetUniformLocation(program, "spot_flag"), 1.0);
	point4 spot_direction = mv * vec4(-6.0, 0.0, -4.5, 1.0);
	glUniform4fv(glGetUniformLocation(program, "spot_direction"), 1,
		spot_direction);
	glUniform1f(glGetUniformLocation(program, "spot_exp"), spot_exp);
	glUniform1f(glGetUniformLocation(program, "spot_ang"), spot_ang);
}

void set_the_floor() {
	color4 ambience(0.2, 0.2, 0.2, 1.0);
	glUniform4fv(glGetUniformLocation(program, "ini_material_ambient"), 1, ambience);

	color4 diffuse(0, 1.0, 0, 1);
	glUniform4fv(glGetUniformLocation(program, "ini_material_diffuse"), 1, diffuse);

	color4 specular(0, 0, 0, 1);
	glUniform4fv(glGetUniformLocation(program, "ini_material_specular"), 1, specular);


}



point4 line_points[9];
color3 line_colors[9];

void floor_setup() {
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

void set_point_light(mat4 mv) {
	color4 light_ambient(0, 0, 0, 1.0);
	color4 light_diffuse(1.0, 1.0, 1.0, 1.0);
	color4 light_specular(1.0, 1.0, 1.0, 1.0);
	float const_att = 2.0;
	float linear_att = 0.01;
	float quad_att = 0.001;
	point4 light_position(-14, 12.0, -3, 1.0);
	vec4 light_position_eyeFrame = mv * light_position;
	glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1,
		light_position_eyeFrame);

	glUniform1f(glGetUniformLocation(program, "ConstAtt"), const_att);
	glUniform1f(glGetUniformLocation(program, "LinearAtt"), linear_att);
	glUniform1f(glGetUniformLocation(program, "QuadAtt"), quad_att);
	glUniform4fv(glGetUniformLocation(program, "input_light_ambient"), 1,
		light_ambient);
	glUniform4fv(glGetUniformLocation(program, "input_light_diffuse"), 1,
		light_diffuse);
	glUniform4fv(glGetUniformLocation(program, "input_light_specular"), 1,
		light_specular);
	glUniform1f(glGetUniformLocation(program, "point_flag"), pointsource * 1.0);
	glUniform1f(glGetUniformLocation(program, "spot_flag"), 0.0);
}

point3 calculateDirection(point3 from, point3 to) {
	point3 v;
	v.x = to.x - from.x;
	v.y = to.y - from.y;
	v.z = to.z - from.z;

	// convert v to unit-length
	float d = sqrt3f(v.x, v.y, v.z);
	v.x = v.x / d;
	v.y = v.y / d;
	v.z = v.z / d;

	return v;
}

point3 crossProduct(point3 u, point3 v) {
	point3 n;
	n.x = u.y * v.z - u.z * v.y;
	n.y = u.z * v.x - u.x * v.z;
	n.z = u.x * v.y - u.y * v.x;
	return n;
}

void set_the_sphere() {
	color4 material_ambient(0.2, 0.2, 0.2, 1.0);
	color4 material_diffuse(1.0, 0.84, 0, 1);
	color4 material_specular(1.0, 0.84, 0, 1);
	float material_shininess = 125;
	glUniform4fv(glGetUniformLocation(program, "ini_material_ambient"), 1,
		material_ambient);
	glUniform4fv(glGetUniformLocation(program, "ini_material_diffuse"), 1,
		material_diffuse);
	glUniform4fv(glGetUniformLocation(program, "ini_material_specular"), 1,
		material_specular);
	glUniform1f(glGetUniformLocation(program, "Shininess"), material_shininess);
}


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

void read_file() {
	ifstream fp;
	string filename;

	int points;
	float temp[3];

	int count = 0;
	cout << "Enter a file name:" << endl;

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

void colorsphere() {
	for (int i = 0; i < triangles * 3; i++) {
		sphere_color[i] = color3(1.0, 0.84, 0.0);
		sphere_shadow_color[i] = color3(0.25, 0.25, 0.25);
	}
}

point3 firework_v[300];
point3 firework_c[300];

void initialize_floor() {

	floor_points[0] = point3(5.0, 0.0, 8.0); floor_colors[0] = color3(0.0, 1.0, 0.0);
	floor_points[1] = point3(5.0, 0.0, -4.0); floor_colors[1] = color3(0.0, 1.0, 0.0);
	floor_points[2] = point3(-5.0, 0.0, 8.0); floor_colors[2] = color3(0.0, 1.0, 0.0);

	floor_points[3] = point3(5.0, 0.0, -4.0); floor_colors[3] = color3(0.0, 1.0, 0.0);
	floor_points[4] = point3(-5.0, 0.0, 8.0); floor_colors[4] = color3(0.0, 1.0, 0.0);
	floor_points[5] = point3(-5.0, 0.0, -4.0); floor_colors[5] = color3(0.0, 1.0, 0.0);

	glGenBuffers(1, &floor_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
		floor_colors);
}

void initialize_floor_lights() {

	
	floor_setup();

	glGenBuffers(1, &floor_light_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_light_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(floor_light_points) + sizeof(floor_normals), NULL,
		GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_light_points),
		floor_light_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_light_points),
		sizeof(floor_normals), floor_normals);

	glGenBuffers(1, &s_flat_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, s_flat_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		3 * triangles * sizeof(point4) + 3 * triangles * sizeof(color3), NULL,
		GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * triangles * sizeof(point4), sphere_shade_d);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * triangles * sizeof(point4),
		3 * triangles * sizeof(color3), sphere_flat_shade);

	glGenBuffers(1, &s_smooth_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, s_smooth_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		3 * triangles * sizeof(point4) + 3 * triangles * sizeof(color3), NULL,
		GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * triangles * sizeof(point4), sphere_shade_d);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * triangles * sizeof(point4),
		3 * triangles * sizeof(color3), sphere_smooth_shade);
}

void initialize_coordinate_system() {

	line_colors[0] = vertex_colors[1];
	line_points[0] = point4(0, 0, 0, 1);
	line_colors[1] = vertex_colors[1];
	line_points[1] = point4(10, 0, 0, 1);
	line_colors[2] = vertex_colors[1];
	line_points[2] = point4(20, 0, 0, 1);

	line_colors[3] = vertex_colors[5];
	line_points[3] = point4(0, 0, 0, 1);
	line_colors[4] = vertex_colors[5];
	line_points[4] = point4(0, 10, 0, 1);
	line_colors[5] = vertex_colors[5];
	line_points[5] = point4(0, 20, 0, 1);

	line_colors[6] = vertex_colors[4];
	line_points[6] = point4(0, 0, 0, 1);
	line_colors[7] = vertex_colors[4];
	line_points[7] = point4(0, 0, 10, 1);
	line_colors[8] = vertex_colors[4];
	line_points[8] = point4(0, 0, 20, 1);

	glGenBuffers(1, &line_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, line_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line_points) + sizeof(line_colors), NULL,
		GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(line_points), line_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(line_points), sizeof(line_colors),
		line_colors);
}

void initialize_fireworks() {
	for (int i = 0; i < 300; i++) {
		vec3 tmp_v =
			vec3(2.0 * ((rand() % 256) / 256.0 - 0.5), 2.4 * (rand() % 256) / 256.0,
				2.0 * ((rand() % 256) / 256.0 - 0.5));

		firework_v[i] = tmp_v;
		vec3 tmp_c = vec3((rand() % 256) / 256.0, (rand() % 256) / 256.0,
			(rand() % 256) / 256.0);
		firework_c[i] = tmp_c;
	}

	glGenBuffers(1, &firework_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, firework_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(firework_v) + sizeof(firework_c), NULL,
		GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(firework_v), firework_v);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(firework_v), sizeof(firework_c),
		firework_c);
}

void set_sphere_buffer() {
	glGenBuffers(1, &sphere_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		3 * triangles * sizeof(point4) + 3 * triangles * sizeof(color3), NULL,
		GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * triangles * sizeof(point4), sphereData);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * triangles * sizeof(point4),
		3 * triangles * sizeof(color3), sphere_color);
}

void set_shadow_buffer() {
	glGenBuffers(1, &sphere_shadow_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_shadow_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		3 * triangles * sizeof(point4) + 3 * triangles * sizeof(color3), NULL,
		GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * triangles * sizeof(point4),
		sphere_shadow_data);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * triangles * sizeof(point4),
		3 * triangles * sizeof(color3), sphere_shadow_color);
}

void init() {
	image_set_up();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &texName);      // Generate texture obj name(s)

	glActiveTexture(GL_TEXTURE0);  // Set the active texture unit to be 0 
	glBindTexture(GL_TEXTURE_2D, texName); // Bind the texture to this texture unit

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, Image);

	glGenBuffers(1, &f_tex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, f_tex_buffer);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(f_tex_vertices) + sizeof(f_tex_normals) +
		sizeof(f_tex_coord),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(f_tex_vertices), f_tex_vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(f_tex_vertices),
		sizeof(f_tex_normals), f_tex_normals);
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(f_tex_vertices) + sizeof(f_tex_normals),
		sizeof(f_tex_coord), f_tex_coord);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &line_texName); // Generate texture obj name(s)

	glActiveTexture(GL_TEXTURE1); // Set the active texture unit to be 0
	glBindTexture(GL_TEXTURE_1D,
		line_texName); // Bind the texture to this texture unit

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		stripeImage);

	radius = 1;
	colorsphere();
	totalSegments = sizeof(position_array) / sizeof(point3);
	vectors = new point3[totalSegments];
	rotationAxis = new point3[totalSegments];
	for (int i = 0; i < totalSegments - 1; i++) {
		vectors[i] = calculateDirection(position_array[i], position_array[i + 1]);
	}
	vectors[totalSegments - 1] =
		calculateDirection(position_array[totalSegments - 1], position_array[0]);

	point3 y_axis(0, 1, 0);
	for (int i = 0; i < totalSegments; i++) {
		rotationAxis[i] = crossProduct(y_axis, vectors[i]);
	}

	initialize_floor();
	initialize_floor_lights();
	initialize_coordinate_system();
	initialize_fireworks();
	set_sphere_buffer();
	set_shadow_buffer();

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glLineWidth(2.0);

	program = InitShader("finalvshader.glsl", "finalfshader.glsl");
	custom_prog = InitShader("customfireV.glsl", "customfireF.glsl");
}

void drawObj(GLuint buffer, int num_vertices) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(num_vertices * sizeof(point4)));

	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vNormal);
}

void drawObj_2(GLuint buffer, int num_vertices) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(sizeof(point4) * num_vertices));
	glDrawArrays(GL_TRIANGLES, 0, num_vertices);

	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
}

mat4 translation_array(12, 0, 0, 0, 14, 0, 3, -1, 0, 0, 12, 0, 0, 0, 0, 12);


void draw_floor(mat4 mv, mat4 eye_frame) {
	if (lighting == 0) {
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		drawObj_2(floor_buffer, floor_NumVertices); // draw the floor
	}
	else {
		if (pointsource) {
			set_point_light(eye_frame);
		}
		else {
			set_spot_light(eye_frame);
		}
		set_the_floor();
		mat4 model_view = mv;
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
		mat3 normal_matrix = NormalMatrix(model_view, 1);
		glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1,
			GL_TRUE, normal_matrix);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (text_ground_flag == 1) {
			glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
			glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
			glUniform1i(glGetUniformLocation(program, "text_ground_flag"), 1);
			// drawObj_3(f_tex_buffer, 6);

			glBindBuffer(GL_ARRAY_BUFFER, f_tex_buffer);

			GLuint vPosition = glGetAttribLocation(program, "vPosition");
			glEnableVertexAttribArray(vPosition);
			glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
				BUFFER_OFFSET(0));

			GLuint vNormal = glGetAttribLocation(program, "vNormal");
			glEnableVertexAttribArray(vNormal);
			glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0,
				BUFFER_OFFSET(6 * sizeof(point4)));

			GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
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
		glUniform1i(glGetUniformLocation(program, "text_ground_flag"), 0);
	}
};

void draw_shadow(mat4 mv) {
	glUniform1i(glGetUniformLocation(program, "draw_shadow_lat"), 1);
	glUniform1i(glGetUniformLocation(program, "f_draw_shadow_lat"), 1);
	glUniform1i(glGetUniformLocation(program, "upright_lat_flag"),
		upright_lat_flag);
	if (shadow == 1) {
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);
		if (solid_Flag == 0) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		if (blend_shadow_flag == 1) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glUniform1f(glGetUniformLocation(program, "shade_flag"), 0);
			glUniform1f(glGetUniformLocation(program, "shadow_semi_flag"), 1.0);
			drawObj_2(sphere_shadow_buffer, sphere_NumVertices);
			glDisable(GL_BLEND);
		}
		else {
			glUniform1f(glGetUniformLocation(program, "shade_flag"), 0);
			glUniform1f(glGetUniformLocation(program, "shadow_semi_flag"), 0);
			drawObj_2(sphere_shadow_buffer, sphere_NumVertices);
		}
	}
	glUniform1i(glGetUniformLocation(program, "draw_shadow_lat"), 0);
	glUniform1i(glGetUniformLocation(program, "f_draw_shadow_lat"), 0);
};

void set_flag_sphere_tex() {
	glUniform1i(glGetUniformLocation(program, "vertical_flag"), vertical_flag);
	glUniform1i(glGetUniformLocation(program, "slant_flag"), slant_flag);
	glUniform1i(glGetUniformLocation(program, "eye_space_flag"), eye_space_flag);
	glUniform1i(glGetUniformLocation(program, "object_space_flag"),
		object_space_flag);
	glUniform1i(glGetUniformLocation(program, "sphere_line_flag"),
		text_sphere_line_flag);
	glUniform1i(glGetUniformLocation(program, "f_sphere_flag"),
		text_sphere_line_flag);
	glUniform1i(glGetUniformLocation(program, "sphere_check_flag"),
		text_sphere_check_flag);
	glUniform1i(glGetUniformLocation(program, "f_sphere_check_flag"),
		text_sphere_check_flag);
	glUniform1i(glGetUniformLocation(program, "upright_lat_flag"),
		upright_lat_flag);
	glUniform1i(glGetUniformLocation(program, "enable_lat"), enable_lat);
};

void cancel_flag_sphere() {
	glUniform1i(glGetUniformLocation(program, "f_sphere_flag"), 0);
	glUniform1i(glGetUniformLocation(program, "f_sphere_check_flag"), 0);
	glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 0);
	glUniform1i(glGetUniformLocation(program, "object_space_flag"), 0);
	glUniform1i(glGetUniformLocation(program, "sphere_line_flag"), 0);
	glUniform1i(glGetUniformLocation(program, "sphere_check_flag"), 0);
}
float get_dist(point3 p1, point3 p2) {
	return sqrt3f((p1.x - p2.x), (p1.y - p2.y), (p1.z - p2.z));
}

int step() { return (global_step + 1) % totalSegments; }

bool boundscheck() {
	int next = step();
	return get_dist(middle_jam, position_array[global_step]) >
		get_dist(position_array[next], position_array[global_step]);
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP && begin1) {
		rolling = !rolling;
	}
	if (rolling) {
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
		t_sub = glutGet(GLUT_ELAPSED_TIME);
		begin1 = true;
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
		vertical_flag = 1;
		slant_flag = 0;
		break;
	case 's':
	case 'S':
		vertical_flag = 0;
		slant_flag = 1;
		break;
	case 'o':
	case 'O':
		object_space_flag = 1;
		eye_space_flag = 0;
		break;
	case 'e':
	case 'E':
		object_space_flag = 0;
		eye_space_flag = 1;
		break;
	case 'u':
	case 'U':
		upright_lat_flag = 0;
		break;
	case 't':
	case 'T':
		upright_lat_flag = 1;
		break;
	case 'l':
	case 'L':
		enable_lat = 1 - enable_lat;
		break;

	case 'a':
	case 'A':
		frame_ticks = 1 - frame_ticks;
		if (frame_ticks == 1)
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
		solid_Flag = !solid_Flag;
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
	solid_Flag = 1;
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
		fog_flag = index;
	else
		fog_flag = 1;
	display();
};

void tex_ground_menu(int index) {
	text_ground_flag = (index == 2) ? 1 : 0;
	display();
}

void tex_sphere_menu(int index) {
	if (index == 1) {
		text_sphere_flag = 0;
	}
	if (index == 2) {
		text_sphere_flag = 1;
		text_sphere_line_flag = 1;
		text_sphere_check_flag = 0;
	}
	if (index == 3) {
		text_sphere_flag = 1;
		text_sphere_line_flag = 0;
		text_sphere_check_flag = 1;
	}
	display();
}

void blend_shadow_menu(int index) {
	blend_shadow_flag = (index == 2) ? 1 : 0;
	display();
}

void firework_menu(int index) {
	firework_flag = (index == 2) ? 1 : 0;
	display();
}

void menu_init_stuff() {
	int shadow = glutCreateMenu(shadow_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int shade = glutCreateMenu(shade_menu);
	glutAddMenuEntry("flat shading", 1);
	glutAddMenuEntry("smooth shading", 2);

	int lighting = glutCreateMenu(lighting_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int text_ground_tmp = glutCreateMenu(tex_ground_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int text_sphere_tmp = glutCreateMenu(tex_sphere_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes-Contour Lines", 2);
	glutAddMenuEntry("Yes-Checkerboard", 3);

	int blend_shadow_tmp = glutCreateMenu(blend_shadow_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int firework_tmp = glutCreateMenu(firework_menu);
	glutAddMenuEntry("No", 1);
	glutAddMenuEntry("Yes", 2);

	int spotlight = glutCreateMenu(spotlight_menu);
	glutAddMenuEntry("Spot light", 1);
	glutAddMenuEntry("Point light", 2);

	int fog_option = glutCreateMenu(fog_menu);
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
	glutAddSubMenu("Fog option", fog_option);
	glutAddSubMenu("Texture Mapped Ground", text_ground_tmp);
	glutAddSubMenu("Texture Mapped Sphere", text_sphere_tmp);
	glutAddSubMenu("Blending Shadow", blend_shadow_tmp);
	glutAddSubMenu("Fireworks", firework_tmp);

	glutAttachMenu(GLUT_LEFT_BUTTON);
}

void reshape(int w, int h) {
	glViewport(0, 0, w, h);
	aspect = (GLfloat)w / (GLfloat)h;
	glutPostRedisplay();
}

void idle(void) {
	theta += delta;
	if (theta > 360.0) {
		theta = theta - 360;
	}

	float off = (radius * delta * pie) / 180;

	middle_jam.x = middle_jam.x + vectors[global_step].x * off;
	middle_jam.y = middle_jam.y + vectors[global_step].y * off;
	middle_jam.z = middle_jam.z + vectors[global_step].z * off;

	if (boundscheck()) {
		global_step = step();
		middle_jam = position_array[global_step];
	}

	glutPostRedisplay();
}


void display(void) {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.529, 0.807, 0.92, 0.0);
	mat4 p = Perspective(fovy, aspect, zNear, zFar);

	vec4 at(0.0, 0.0, 0.0, 1.0);
	vec4 up(0.0, 1.0, 0.0, 0.0);
	mat4 mv = LookAt(eye, at, up);
	mat4 eye_frame = LookAt(eye, at, up);

	glUseProgram(program);


	shade_flag = 0;
	if (solid_Flag == 0 || lighting == 0) {
		flatshade = 0;
		smoothshade = 0;
	}

	if (lighting == 1) {
		shade_flag = 1.0;
	}

	glUniform1f(glGetUniformLocation(program, "shade_flag"), shade_flag * 1.0);

	glUniform1i(glGetUniformLocation(program, "text_ground_flag"), 0);
	glUniform1i(glGetUniformLocation(program, "f_sphere_flag"), 0);
	glUniform1i(glGetUniformLocation(program, "vertical_flag"), 1);
	glUniform1i(glGetUniformLocation(program, "slant_flag"), 0);
	glUniform1i(glGetUniformLocation(program, "eye_space_flag"), 0);
	glUniform1i(glGetUniformLocation(program, "object_space_flag"), 1);

	glUniform1f(glGetUniformLocation(program, "fog_flag"), fog_flag * 1.0);

	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");
	glUniformMatrix4fv(Projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
	glEnable(GL_DEPTH_TEST);

	if (blend_shadow_flag == 1) {
		glDepthMask(GL_FALSE);
		draw_floor(mv, eye_frame);
		mv = LookAt(eye, at, up) * translation_array *
			Translate(middle_jam.x, middle_jam.y, middle_jam.z) * acc_matrix;
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
		mv = LookAt(eye, at, up) * translation_array *
			Translate(middle_jam.x, middle_jam.y, middle_jam.z) * acc_matrix;
		if (eye[1] > 0)
			draw_shadow(mv);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		mv = LookAt(eye, at, up);
		draw_floor(mv, eye_frame);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}

	glUniform1f(glGetUniformLocation(program, "shade_flag"), 0);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, eye_frame);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawObj_2(line_buffer, 9);

	if (lighting == 1)
		glUniform1f(glGetUniformLocation(program, "shade_flag"), 1.0);

	acc_matrix =
		Rotate(delta, rotationAxis[global_step].x, rotationAxis[global_step].y,
			rotationAxis[global_step].z) *
		acc_matrix;
	mv = LookAt(eye, at, up) *
		Translate(middle_jam.x, middle_jam.y, middle_jam.z) * acc_matrix;

	if (flatshade == 1) {
		if (pointsource) {
			set_point_light(eye_frame);
		}
		else {
			set_spot_light(eye_frame);
		}
		set_the_sphere();
		mat4 model_view = mv;
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
		mat3 normal_matrix = NormalMatrix(model_view, 1);
		glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1,
			GL_TRUE, normal_matrix);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (text_sphere_flag == 1) {
			set_flag_sphere_tex();
			glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
			glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
			drawObj(s_flat_buffer, triangles * 3);
			cancel_flag_sphere();
		}
		else {
			drawObj(s_flat_buffer, triangles * 3);
		}
	}

	else if (smoothshade == 1) {
		if (pointsource) {
			set_point_light(eye_frame);
		}
		else {
			set_spot_light(eye_frame);
		}
		set_the_sphere();
		mat4 model_view = mv;
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
		mat3 normal_matrix = NormalMatrix(model_view, 1);
		glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1,
			GL_TRUE, normal_matrix);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		if (text_sphere_flag == 1) {
			set_flag_sphere_tex();
			glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
			glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
			glUniform1i(glGetUniformLocation(program, "my_texture_2D"), 2);
			drawObj(s_smooth_buffer, triangles * 3);
			cancel_flag_sphere();
		}
		else {
			drawObj(s_smooth_buffer, triangles * 3);
		}
	}

	else {
		glUniform1f(glGetUniformLocation(program, "shade_flag"), 0);
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);
		if (solid_Flag == 0) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		drawObj_2(sphere_buffer, sphere_NumVertices);
	}

	glUseProgram(custom_prog);
	

	glPointSize(3.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	float t = glutGet(GLUT_ELAPSED_TIME);
	int time = int((t - t_sub)) % 5000;
	if (begin1 == true)
		glUniform1f(glGetUniformLocation(custom_prog, "time"), time);
	else
		glUniform1f(glGetUniformLocation(custom_prog, "time"), 0);

	if (firework_flag == 1) {

		glBindBuffer(GL_ARRAY_BUFFER, firework_buffer);

		GLuint velocity = glGetAttribLocation(custom_prog, "velocity");
		glEnableVertexAttribArray(velocity);
		glVertexAttribPointer(velocity, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

		GLuint vColor = glGetAttribLocation(custom_prog, "vColor");
		glEnableVertexAttribArray(vColor);
		glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0,
			BUFFER_OFFSET(sizeof(point3) * 300));
		glDrawArrays(GL_TRIANGLES, 0, 300);
		glDisableVertexAttribArray(velocity);
		glDisableVertexAttribArray(vColor);
	}
	glutSwapBuffers();
}

#define SIZE 512
int main(int argc, char **argv) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(SIZE, SIZE);
	glutCreateWindow("assignment4");

	int glewVal = glewInit();
	if (GLEW_OK != glewVal) {
		cout << "The bad happend :(" << endl;
		exit(1);
	}
	read_file();
	menu_init_stuff();
	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	init();
	glutMainLoop();

}
