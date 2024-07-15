#include "renderer.hpp"

#include <ImGui/imgui.h>
#include <glad/glad.h>

#include "context.hpp"

namespace Renderer
{

void ObjectManager::create(f32 left, f32 right, f32 bottom, f32 top, f32 near, f32 far)
{
    m_transform.lr = {left, right};
    m_transform.bt = {bottom, top};
    m_transform.nf = {near, far};
    math::identity(m_transform.proj);
    return;
}

notused void ObjectManager::destroy()
{
    return;
}

void ObjectManager::render() const
{
    auto *ctx = getGlobalContext();
    ElementBufferRenderData currentIBOdata;

    math::mat4f tmp2;
    math::identity(tmp2);

    ctx->shader.bind();
    for (auto const &obj : m_targets)
    {
        obj->texture->bindToUnit(1);
        ctx->shader.uniform1i("uTexture", 1);
        ctx->shader.uniformMatrix4fv("modelMatrix", obj->transform->TRS);
        ctx->shader.uniformMatrix4fv("view", tmp2);
        ctx->shader.uniformMatrix4fv("projection", m_transform.proj);
        obj->vertexData->bind();

        currentIBOdata = obj->vertexData->getRenderData();
        glDrawElements(GL_TRIANGLES, currentIBOdata.count, currentIBOdata.gl_type, nullptr);
    }

    return;
}

void ObjectManager::renderImGui()
{
    using array2u = std::array<u32, 2>;
    std::vector<std::array<char, 20>> objTransformString{m_targets.size()};
    bool shouldRecomputeTransform = false;
    array2u winDims;
    auto *context = getGlobalContext();
    f32 dt = context->glfw.time_dt();
    context->glfw.dimensions(&winDims[0]);

    ImGui::BeginGroup();
    ImGui::BulletText("Currently Using GPU Vendor %s Model %s\n", glad_glGetString(GL_VENDOR),
                      glad_glGetString(GL_RENDERER));
    ImGui::EndGroup();
    ImGui::BeginGroup();
    ImGui::Text("Window Resolution is %ix%i\n", winDims[0], winDims[1]);
    ImGui::Text("Rendering at %.02f Frames Per Second (%.05f ms/frame)", (1.0f / dt), (dt * 1000.0f));
    ImGui::EndGroup();

    ImGui::BeginGroup();
    shouldRecomputeTransform =
        shouldRecomputeTransform || ImGui::SliderFloat2("Near,   Far  ", m_transform.nf.begin(), -10.0f, 10.0f);
    // shouldRecomputeTransform = shouldRecomputeTransform ||
    // ImGui::SliderFloat2("Bottom, Top  ", m_transform.bt.begin(),
    // -10.0f, 10.0f); shouldRecomputeTransform = shouldRecomputeTransform ||
    // ImGui::SliderFloat2("Left,   Right", m_transform.lr.begin(),
    // -10.0f, 10.0f);
    shouldRecomputeTransform =
        shouldRecomputeTransform || ImGui::SliderFloat2("Bottom, Top  ", m_transform.bt.begin(), 0.0f, winDims[1]);
    shouldRecomputeTransform =
        shouldRecomputeTransform || ImGui::SliderFloat2("Left,   Right", m_transform.lr.begin(), 0.0f, winDims[0]);
    if (shouldRecomputeTransform)
    {
        math::orthographic(m_transform.lr.x, m_transform.lr.y, m_transform.bt.x, m_transform.bt.y, m_transform.nf.x,
                           m_transform.nf.y, m_transform.proj);
    }
    ImGui::Text("mat4f %p:\n\
        ( %-5.05f, %-5.05f, %-5.05f, %-5.05f )\n\
        ( %-5.05f, %-5.05f, %-5.05f, %-5.05f )\n\
        ( %-5.05f, %-5.05f, %-5.05f, %-5.05f )\n\
        ( %-5.05f, %-5.05f, %-5.05f, %-5.05f )\n",
                (void *)m_transform.proj.begin(), m_transform.proj.m00, m_transform.proj.m01, m_transform.proj.m02,
                m_transform.proj.m03, m_transform.proj.m10, m_transform.proj.m11, m_transform.proj.m12,
                m_transform.proj.m13, m_transform.proj.m20, m_transform.proj.m21, m_transform.proj.m22,
                m_transform.proj.m23, m_transform.proj.m30, m_transform.proj.m31, m_transform.proj.m32,
                m_transform.proj.m33);
    ImGui::EndGroup();

    for (size_t i = 0; i < context->renderer.targetCount(); ++i)
    { /* Currently the amount of materials is static so this is fine. */
        sprintf(objTransformString[i].data(), "Render Target %2d", __scast(u32, i));
    };
    ImGui::BeginChild("Render Targets");
    for (size_t i = 0; i < context->renderer.targetCount(); ++i)
    {
        shouldRecomputeTransform = false;
        shouldRecomputeTransform =
            shouldRecomputeTransform ||
            ImGui::SliderFloat2("Position", (f32 *)m_targets[i]->transform->position.begin(), -3.0f, 3.0f);
        shouldRecomputeTransform =
            shouldRecomputeTransform ||
            ImGui::SliderFloat2("Scaling ", (f32 *)m_targets[i]->transform->scale.begin(), 0.0f, 1.0f);
        shouldRecomputeTransform =
            shouldRecomputeTransform || ImGui::DragFloat("Rotation", (f32 *)&m_targets[i]->transform->rotateAngle);
        if (shouldRecomputeTransform)
        {
            static math::mat4f tmp;
            math::modelMatrix2d(m_targets[i]->transform->position, m_targets[i]->transform->scale,
                                m_targets[i]->transform->rotateAngle, tmp);
            memcpy((void *)m_targets[i]->transform->TRS.begin(), tmp.begin(), tmp.bytes());
        }
    }
    ImGui::EndChild();
    return;
}

} // namespace Renderer