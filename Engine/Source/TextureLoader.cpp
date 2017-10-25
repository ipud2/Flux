#include "TextureLoader.h"

#include "Path.h"
#include "Log.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h>
#include <vector>

namespace Flux {
    Texture* TextureLoader::loadTexture(Path path) {
        int width, height, bpp;

        unsigned char* data = stbi_load(path.str().c_str(), &width, &height, &bpp, STBI_rgb_alpha);
        
        if (!data) {
            Log::error("Failed to load image at: " + path.str());
            return nullptr;
        }
        else {
            Log::debug("Loaded texture: " + path.str() + " " + std::to_string(width) + " " + std::to_string(height) + " " + std::to_string(bpp));
        }

        Texture* texture = create(width, height, STBI_rgb_alpha, data, Sampling::LINEAR);

        stbi_image_free(data);

        return texture;
    }

    Texture* TextureLoader::loadTextureGreyscale(Path path) {
        int width, height, bpp;

        unsigned char* data = stbi_load(path.str().c_str(), &width, &height, &bpp, STBI_grey);

        if (!data) {
            Log::error("Failed to load image at: " + path.str());
            return nullptr;
        }
        else {
            Log::debug("Loaded texture: " + path.str() + " " + std::to_string(width) + " " + std::to_string(height) + " " + std::to_string(bpp));
        }

        Texture* texture = create(width, height, STBI_grey, data, Sampling::LINEAR);

        stbi_image_free(data);

        return texture;
    }

    Texture* TextureLoader::loadColorAndAlpha(Path color, Path alpha, Sampling sampling) {
        int colorWidth, colorHeight, colorBPP;
        int alphaWidth, alphaHeight, alphaBPP;

        unsigned char* colorData = stbi_load(color.str().c_str(), &colorWidth, &colorHeight, &colorBPP, STBI_rgb_alpha);
        Log::debug("Loaded texture: " + color.str() + " " + std::to_string(colorWidth) + " " + std::to_string(colorHeight) + " " + std::to_string(colorBPP));
        unsigned char* alphaData = stbi_load(color.str().c_str(), &alphaWidth, &alphaHeight, &alphaBPP, STBI_grey);
        Log::debug("Loaded texture: " + alpha.str() + " " + std::to_string(alphaWidth) + " " + std::to_string(alphaHeight) + " " + std::to_string(alphaBPP));

        assert(colorWidth == alphaWidth);
        assert(colorHeight == alphaHeight);
        assert(colorBPP >= 3);

        std::vector<unsigned char> data(colorWidth * colorHeight * 4);
        for (int i = 0; i < data.size() / 4; i++) {
            data[i * 4 + 0] = colorData[i * colorBPP + 0];
            data[i * 4 + 1] = colorData[i * colorBPP + 1];
            data[i * 4 + 2] = colorData[i * colorBPP + 2];
            data[i * 4 + 3] = alphaData[i * alphaBPP + 0];
        }

        Texture* texture = create(colorWidth, colorHeight, 4, data.data(), sampling);

        return texture;
    }

    Texture* TextureLoader::loadTextureHDR(Path path) {
        int width, height, bpp;

        float *data = stbi_loadf(path.str().c_str(), &width, &height, &bpp, STBI_rgb_alpha);

        if (!data) {
            Log::error("Failed to load image at: " + path.str());
        }

        Texture* texture = createHDR(width, height, data, Sampling::NEAREST);

        stbi_image_free(data);

        return texture;
    }

    Texture3D* TextureLoader::loadTexture3D(Path path) {
        int width, height, bpp;

        unsigned char* data = stbi_load(path.str().c_str(), &width, &height, &bpp, STBI_rgb_alpha);

        if (!data) {
            Log::error("Failed to load image at: " + path.str());
        }

        Texture3D* texture = create3DTexture(height, height, height, 4, data, Sampling::LINEAR);

        stbi_image_free(data);

        return texture;
    }

    Texture* TextureLoader::create(const int width, const int height, const int bpp, const unsigned char* data, Sampling sampling) {
        GLuint handle;
        glGenTextures(1, &handle);

        glBindTexture(GL_TEXTURE_2D, handle);

        if (sampling == Sampling::NEAREST) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
        GLfloat maxAnisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy > 5.0f ? 5.0f : maxAnisotropy);

        if (bpp == 1) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        }
        else {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        return new Texture(handle, width, height);
    }

    Texture* TextureLoader::createHDR(const int width, const int height, const float* data, Sampling sampling) {
        GLuint handle;
        glGenTextures(1, &handle);

        glBindTexture(GL_TEXTURE_2D, handle);

        if (sampling == Sampling::NEAREST) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        }
        else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data);

        glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        return new Texture(handle, width, height);
    }

    Texture* TextureLoader::createEmpty(const int width, const int height, GLint internalFormat, GLenum format, GLenum type, Sampling sampling, bool mipmaps) {
        GLuint handle;
        glGenTextures(1, &handle);

        glBindTexture(GL_TEXTURE_2D, handle);

        if (mipmaps) {
            if (sampling == Sampling::NEAREST) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            }
            else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            }
        }
        else {
            if (sampling == Sampling::NEAREST) {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }
            else {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            }
        }


        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, 0);
        if (mipmaps) glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        return new Texture(handle, width, height);
    }

    Texture* TextureLoader::createShadowMap(const int width, const int height) {
        GLuint handle;
        glGenTextures(1, &handle);

        glBindTexture(GL_TEXTURE_2D, handle);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

        // Set the border color to 1 so samples outside of the shadow map have the furthest depth
        float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

        glBindTexture(GL_TEXTURE_2D, 0);

        return new Texture(handle, width, height);
    }

    Texture3D* TextureLoader::create3DTexture(const int width, const int height, const int depth, const int bpp, const unsigned char* data, Sampling sampling) {
        GLuint handle;
        glGenTextures(1, &handle);

        glBindTexture(GL_TEXTURE_3D, handle);

        if (sampling == Sampling::NEAREST) {
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }
        else {
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


        //const unsigned char* organisedData = new unsigned char[width * height * depth];
        //for (int z = 0; z < depth; z++) {
        //    for (int y = 0; y < height; y++) {
        //        for (int x = 0; x < width; x++) {
        //            organisedData[z * width * height + y * width + x] = data[y * width + x]
        //        }
        //    }
        //}

        if (bpp == 1) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, width, height, depth, 0, GL_RED, GL_UNSIGNED_BYTE, data);
        }
        else {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }

        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 0);

        glBindTexture(GL_TEXTURE_3D, 0);

        return new Texture3D(handle, width, height, depth);
    }
}
