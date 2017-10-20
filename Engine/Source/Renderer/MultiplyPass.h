#pragma once
#ifndef MULTIPLY_PASS_H
#define MULTIPLY_PASS_H

#include <Engine/Source/RenderPhase.h>

#include <Engine/Source/Shader.h>

#include <memory>

namespace Flux
{
    class Texture;

    class MultiplyPass : public RenderPhase
    {
    public:
        MultiplyPass();

        void SetTextures(std::vector<Texture>);
        void render(const Scene& scene) override;

    private:
        static const unsigned int MAX_SOURCES = 8;

        std::unique_ptr<Shader> shader;

        std::vector<Texture> textures;

        std::vector<int> units;
    };
}

#endif /* MULTIPLY_PASS_H */
