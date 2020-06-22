#version 330 core

in vec3 vertex;
in vec3 normal;

in vec3 matamb;
in vec3 matdiff;
in vec3 matspec;
in float matshin;

out vec4 vertexSCO;
out vec3 normalSCO;

out vec3 matambFrag;
out vec3 matdiffFrag;
out vec3 matspecFrag;
out float matshinFrag;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 TG;
uniform mat3 NormalMatrix;

void main()
{	
    vertexSCO = view*TG*vec4(vertex, 1.0);
    normalSCO = NormalMatrix*normal;
    matambFrag = matamb;
    matdiffFrag = matdiff;
    matspecFrag = matspec;
    matshinFrag = matshin;
    
    gl_Position = proj * view * TG * vec4 (vertex, 1.0);
}

