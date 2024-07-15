#version 460 core
in  vec3 particleColor;
out vec4 FragColor;


void main()
{  
    if (length(gl_PointCoord - vec2(0.5)) > 0.5f) {
        discard;
    }
    FragColor = vec4(particleColor, 1.0f);
}


