#include "Renderer/DirectLightPass.h"

#include "Renderer/RenderState.h"

#include "TextureUnit.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "Util/Matrix4f.h"

#include "DirectionalLight.h"

namespace Flux {
    DirectLightPass::DirectLightPass() : RenderPhase("Direct Lighting")
    {
        shader = std::unique_ptr<Shader>(Shader::fromFile("res/Shaders/Quad.vert", "res/Shaders/DeferredDirect.frag"));
    }

    void DirectLightPass::SetGBuffer(const GBuffer* gBuffer)
    {
        this->gBuffer = gBuffer;
    }

    void DirectLightPass::SetTarget(const Framebuffer* target)
    {
        this->target = target;
    }

    void DirectLightPass::render(RenderState& renderState, const Scene& scene)
    {
        nvtxRangePushA(getPassName().c_str());

        shader->bind();

        renderState.enable(BLENDING);
        glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
        glDepthFunc(GL_LEQUAL);

        Entity* camera = scene.getMainCamera();
        Transform* ct = camera->getComponent<Transform>();

        shader->uniform3f("camPos", ct->position);

        gBuffer->albedoTex->bind(TextureUnit::ALBEDO);
        shader->uniform1i("albedoMap", TextureUnit::ALBEDO);
        gBuffer->normalTex->bind(TextureUnit::NORMAL);
        shader->uniform1i("normalMap", TextureUnit::NORMAL);
        gBuffer->positionTex->bind(TextureUnit::POSITION);
        shader->uniform1i("positionMap", TextureUnit::POSITION);

        for (Entity* light : scene.lights) {
            DirectionalLight* directionalLight = light->getComponent<DirectionalLight>();
            PointLight* pointLight = light->getComponent<PointLight>();
            Transform* transform = light->getComponent<Transform>();

            if (directionalLight) {
                Matrix4f pitchMatrix;
                pitchMatrix.rotate(transform->rotation.x, 1, 0, 0);
                Matrix4f yawMatrix;
                yawMatrix.rotate(transform->rotation.y, 0, 1, 0);
                Vector3f direction = (yawMatrix * pitchMatrix * Vector3f(0, 0, -1)).normalise();

                shader->uniform3f("dirLight.direction", direction);
                shader->uniform3f("dirLight.color", directionalLight->color);
                shader->uniform1i("isDirLight", true);
                shader->uniform1i("isPointLight", false);
                directionalLight->shadowMap->bind(TextureUnit::SHADOW);
                shader->uniform1i("dirLight.shadowMap", TextureUnit::SHADOW);
                shader->uniformMatrix4f("dirLight.shadowMatrix", directionalLight->shadowSpace);
            }
            else if (pointLight) {
                shader->uniform3f("pointLight.position", transform->position);
                shader->uniform3f("pointLight.color", pointLight->color);
                shader->uniform1i("isPointLight", true);
                shader->uniform1i("isDirLight", false);
                pointLight->shadowMap->bind(TextureUnit::TEXTURE7);
                shader->uniform1i("pointLight.shadowMap", TextureUnit::TEXTURE7);
            }
            else {
                continue;
            }

            renderState.drawQuad();
        }

        renderState.disable(BLENDING);

        nvtxRangePop();
    }
}
