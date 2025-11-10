
#include "TextureManager.h"
#include <filesystem>
#include <algorithm>
#include <include/stb-master/stb_image.h>
std::unordered_map<std::string, GLuint> TextureManager::textures;

std::string TextureManager::ResolvePath(const std::string& originalPath, const std::string& fallbackDir) {
    namespace fs = std::filesystem;

    fs::path path(originalPath);
    if (fs::exists(path)) {
        std::string pathStr = path.string();
        std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
        return pathStr;
    }

    fs::path fallbackPath = fs::path(fallbackDir) / path.filename();
    if (fs::exists(fallbackPath)) {
        std::string pathStr = fallbackPath.string();
        std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
        return pathStr;
    }

    std::string pathStr = fallbackPath.string();
    std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
    std::cerr << "[TextureManager] WARNING: Texture not found: " << pathStr << std::endl;
    return pathStr;
}



Textures::TextureType TextureManager::InferTextureType(const std::string& path) {
    if (path.find("_BC") != std::string::npos || path.find("BaseColor") != std::string::npos)
        return Textures::TextureType::BaseColor;
    if (path.find("_NM") != std::string::npos || path.find("Normal") != std::string::npos)
        return Textures::TextureType::Normal;
    if (path.find("Roughness") != std::string::npos)
        return Textures::TextureType::Roughness;
    if (path.find("ambientocclusion") != std::string::npos || path.find("_AO") != std::string::npos)
        return Textures::TextureType::AmbientOcclusion;
    if (path.find("Mask") != std::string::npos || path.find("_OP") != std::string::npos)
        return Textures::TextureType::Mask;
    if (path.find("_ORS") != std::string::npos)
        return Textures::TextureType::Unpacked;

    return Textures::TextureType::BaseColor;
}

GLuint TextureManager::LoadTexture(const std::string& path, Textures::TextureType type) {
    std::string textureDir = "../resources/textures";
    std::string fixedPath = ResolvePath(path, textureDir);

    auto it = textures.find(fixedPath);
    if (it != textures.end())
        return it->second;

    if (type == Textures::TextureType::BaseColor)
        type = InferTextureType(fixedPath);

    std::cout << "[TextureManager] Loading texture: " << fixedPath << std::endl;
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(fixedPath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "[TextureManager] Failed to load texture: " << fixedPath << std::endl;
        return 0;
    }

    GLenum format;
    if (channels == 1) format = GL_RED;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;
    else {
        stbi_image_free(data);
        std::cerr << "[TextureManager] Unsupported channels in texture: " << fixedPath << std::endl;
        return 0;
    }


    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    textures[fixedPath] = textureID;
    return textureID;
}


MaterialTextures TextureManager::LoadMaterialTextures(
    const std::string& baseColor,
    const std::string& normal,
    const std::string& roughness,
    const std::string& ao,
    const std::string& mask,
    const std::string& unpacked
) {
    MaterialTextures matTex;
    if (!baseColor.empty()) {
        matTex.baseColorID = LoadTexture(baseColor, Textures::TextureType::BaseColor);
        if(matTex.baseColorID == 0) std::cerr << "[TextureManager] Failed to load BaseColor: " << baseColor << std::endl;
    }
    if (!normal.empty()) {
        matTex.normalID = LoadTexture(normal, Textures::TextureType::Normal);
        if(matTex.normalID == 0) std::cerr << "[TextureManager] Failed to load Normal: " << normal << std::endl;
    }
    if (!roughness.empty()) {
        matTex.roughnessID = LoadTexture(roughness, Textures::TextureType::Roughness);
        if(matTex.roughnessID == 0) std::cerr << "[TextureManager] Failed to load Roughness: " << roughness << std::endl;
    }
    if (!ao.empty()) {
        matTex.aoID = LoadTexture(ao, Textures::TextureType::AmbientOcclusion);
        if(matTex.aoID == 0) std::cerr << "[TextureManager] Failed to load AO: " << ao << std::endl;
    }
    if (!mask.empty()) {
        matTex.maskID = LoadTexture(mask, Textures::TextureType::Mask);
        if(matTex.maskID == 0) std::cerr << "[TextureManager] Failed to load Mask: " << mask << std::endl;
    }
    if (!unpacked.empty()) {
        matTex.unpackedID = LoadTexture(unpacked, Textures::TextureType::Unpacked);
        if(matTex.unpackedID == 0) std::cerr << "[TextureManager] Failed to load Unpacked: " << unpacked << std::endl;
    }
    return matTex;
}


void TextureManager::Clear() {
    for (auto& pair : textures) {
        glDeleteTextures(1, &pair.second);
    }
    textures.clear();
}
