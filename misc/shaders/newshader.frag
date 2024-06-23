#version 460 core
layout(location = 0) in vec3 _UseAttribute;
layout(location = 1) in vec3 _Col;
layout(location = 2) in vec2 _TexPos;
out vec4 FragColor;


uniform sampler2D Texture0;


void main()
{
    vec4 chooseColor   = vec4(_Col, 1.0f);
    vec4 chooseTexture = texture(Texture0, _TexPos);
    if(_UseAttribute.x > 0.5f) {
        FragColor = chooseColor;
    }
    else if(_UseAttribute.y > 0.5f) {
        FragColor = chooseTexture;
    }
}


