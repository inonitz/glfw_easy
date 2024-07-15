#pragma once
#include "context.hpp"
#include "gl/renderer.hpp"




struct RectangleData
{
    Renderer::Transform transform;
    TextureBuffer       texture;
    Buffer              vertex;
    Buffer              indices;
    VertexArray         rectVAO;
};


globalContext* createDefaultContext(math::vec2u windowSize);
void           createRectangleData(RectangleData* rectData);
void           destroyRectangleData(RectangleData* rectData);
void           destroyDefaultContext();

int sprite_render(); /* main function in .cpp file */