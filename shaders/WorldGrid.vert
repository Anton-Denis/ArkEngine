#version 330 core
layout (location = 0) in vec2 aPos; // z.B. (-1,-1),(3,-1),(-1,3) für 1-Dreieck-Fullscreen

out vec2 vScreenUV;

void main() {
    vScreenUV = aPos * 0.5 + 0.5;   // [0..1] für Reconstruct
    gl_Position = vec4(aPos, 0.0, 1.0);
}
