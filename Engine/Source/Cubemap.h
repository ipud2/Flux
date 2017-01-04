#pragma once
#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "String.h"

#include "stb_image.h"

#include <glad/glad.h>

namespace Flux {
    class Cubemap {
    public:
        Cubemap() : resolution(DEFAULT_RESOLUTION) { }

        void create(const char* paths[6], bool mipmaps) {
            glGenTextures(1, &handle);
            bind();

            for (int i = 0; i < 6; i++) {
                int width, height, bpp;
                unsigned char* data = stbi_load(paths[i], &width, &height, &bpp, STBI_rgb_alpha);
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            if (mipmaps) {
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            }
            else {
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            }
            
            release();
        }

        void createEmpty(const unsigned int width, const unsigned int height, bool mipmaps) {
            glGenTextures(1, &handle);
            bind();

            for (int i = 0; i < 6; i++) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            if (mipmaps) {
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
            }
            else {
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            }

            release();
        }

        void bind() const {
            glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
        }

        void release() const {
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }

        GLuint getHandle() {
            return handle;
        }
    private:
        const unsigned int DEFAULT_RESOLUTION = 256;

        GLuint handle;
        unsigned int resolution;
    };
}

#endif /* CUBEMAP_H */
