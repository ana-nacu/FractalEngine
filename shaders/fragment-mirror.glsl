#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D screenTexture;

void main()
{
//    vec3 col = texture(screenTexture, TexCoords).rgb;
//    FragColor = vec4(col, 1.0);
    if (TexCoords.x < 0.05 || TexCoords.x > 0.95 || TexCoords.y < 0.05 || TexCoords.y > 0.95)
    FragColor = vec4(0.1, 0.1, 0.1, 1.0); // Gri închis – rama oglinzii
    else
    FragColor = texture(screenTexture, TexCoords);
}