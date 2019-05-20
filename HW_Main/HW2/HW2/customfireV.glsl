//#version 130

in  vec3 velocity;
in  vec3 vColor;
out vec4 color;
out float y;

uniform mat4 model_view;
uniform mat4 projection;
uniform float time;

void main()  {
  float a=time*velocity.x*0.001;
  float b=0.1+0.001*velocity[1]*time+0.5*(-0.00000049)*time*time;
  float c=velocity[2]*time*0.001;
  vec4 vPosition4 = vec4(a, b, c, 1.0);
  vec4 vColor4 = vec4(vColor.r, vColor.g, vColor.b, 1.0); 

  y=b;
  gl_Position = projection * model_view * vPosition4;
  color = vColor4;
} 
