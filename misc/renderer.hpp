#pragma once
#include "gl/vertexArray.hpp"
#include "gl/texture.hpp"




namespace Renderer {


struct SceneTransform
{
    math::vec2f lr;
    math::vec2f bt;
    math::vec2f nf;
    math::mat4f proj;
};


struct Transform
{
    math::vec2f position;
    math::vec2f scale;
    f32         rotateAngle;
    math::mat4f TRS; /* Translate Rotate Scale */
};
using TransformHandle = imut_type_handle<Transform>;
using SceneTransformHandle = value_ptr<SceneTransform>;
using TextureHandle   = value_ptr<TextureBuffer>;
using VAOHandle       = imut_type_handle<VertexArray>;


struct RenderTarget
{
    VAOHandle       vertexData;
    TextureHandle   texture;
    TransformHandle transform;
};
using RenderHandle  = imut_type_handle<RenderTarget>;




class ObjectManager {
private:
    SceneTransform            m_transform;
    std::vector<RenderHandle> m_targets;
    std::vector<bool>         m_toRender;
    u32                       m_activeTargets;
public:
    void create(
        f32 left,   f32 right, 
        f32 bottom, f32 top, 
        f32 near,   f32 far
    );
    void destroy();

    void push(RenderHandle renderData) 
    { 
        m_targets.push_back(renderData); 
        m_toRender.push_back(true); 
        ++m_activeTargets;
        return; 
    }
    void pop()
    { 
        m_targets.pop_back();
        m_toRender.pop_back();
        --m_activeTargets; 
        return; 
    }
    void setRenderStatus(u8 renderID, bool shouldRender) {
        m_toRender[renderID] = shouldRender; 
        return;
    }
    void render() const;
    void renderImGui();
    
    
    const math::mat4f& getSceneTransform() const { return m_transform.proj; }
          math::mat4f& getSceneTransform()       { return m_transform.proj; }
    size_t             targetCount()       const { return m_targets.size(); }
    size_t             activeTargetCount() const { return m_activeTargets;  }
};




}