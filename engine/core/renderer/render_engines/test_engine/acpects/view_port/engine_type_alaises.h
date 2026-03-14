// engine_type_aliases.h
#pragma once

// Replace these with your actual class names from your headers:
class test_render_engine_FBO;
class camera_handeling;
class test_render_engine;
class Scene; // or `scene` if that's the class name

// Aliases the viewport UI expects:
using ViewportFBO = test_render_engine_FBO;

using Renderer = test_render_engine;
using Scene = Scene;
// If your scene class is lowercase `scene`, then do:
// class scene; using Scene = scene;
