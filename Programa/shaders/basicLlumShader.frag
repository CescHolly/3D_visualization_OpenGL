#version 330 core

out vec4 FragColor;

in vec4 vertexSCO;
in vec3 normalSCO;

in vec3 matambFrag;
in vec3 matdiffFrag;
in vec3 matspecFrag;
in float matshinFrag;

// Valors per als components que necessitem dels focus de llum
uniform vec3 colFocus;
uniform vec3 llumAmbient;
uniform vec4 posFocus;

// Valors per als efectes de color del shader
uniform bvec4 components;
uniform float greyCoef; //[0..1]


vec3 Lambert (vec3 NormSCO, vec3 L) 
{
    // S'assumeix que els vectors que es reben com a paràmetres estan normalitzats

    // Inicialitzem color a component ambient
    vec3 colRes = llumAmbient * matambFrag;

    // Afegim component difusa, si n'hi ha
    if (dot (L, NormSCO) > 0)
      colRes = colRes + colFocus * matdiffFrag * dot (L, NormSCO);
    return (colRes);
}

vec3 Phong (vec3 NormSCO, vec3 L, vec4 vertSCO) 
{
    // Els vectors estan normalitzats

    // Inicialitzem color a Lambert
    vec3 colRes = Lambert (NormSCO, L);

    // Calculem R i V
    if (dot(NormSCO,L) < 0)
      return colRes;  // no hi ha component especular

    vec3 R = reflect(-L, NormSCO); // equival a: normalize (2.0*dot(NormSCO,L)*NormSCO - L);
    vec3 V = normalize(-vertSCO.xyz);

    if ((dot(R, V) < 0) || (matshinFrag == 0))
      return colRes;  // no hi ha component especular
    
    // Afegim la component especular
    float shine = pow(max(0.0, dot(R, V)), matshinFrag);
    return (colRes + matspecFrag * colFocus * shine); 
}


void main()
{	
    vec3 NormSCO = normalize(normalSCO);
    vec3 LSCO = normalize(posFocus.xyz-vertexSCO.xyz);
    vec3 fcolor = Phong(NormSCO, LSCO, vertexSCO);
    
    vec3 compColor = vec3(0,0,0);
    
    if (components.x) compColor.x = fcolor.x;
    if (components.y) compColor.y = fcolor.y;
    if (components.z) compColor.z = fcolor.z;
    
    if (components.a) {
        float grey = (compColor.x+compColor.y+compColor.z)/3;
        compColor.x -= (compColor.x-grey)*greyCoef;
        compColor.y -= (compColor.y-grey)*greyCoef;
        compColor.z -= (compColor.z-grey)*greyCoef;
    }
    FragColor = vec4(compColor, 1);
}


