#pragma once
#include <glad/glad.h>

// Loads an image file into an OpenGL texture and returns its ID.
// Returns 0 if loading fails.
GLuint loadTexture(const char* filename);
