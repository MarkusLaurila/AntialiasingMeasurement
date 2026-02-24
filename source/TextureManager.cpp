
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



void TextureManager::LoadAllTextures() {
    std::cout << "[TextureManager::LoadAllTextures] Preloading all textures..." << std::endl;


    auto loadList = [](const std::initializer_list<std::string>& paths) {
        for (const auto& path : paths) {
            if (!path.empty()) {
                GLuint texID = TextureManager::LoadTexture(path);
                std::cout << "[TextureManager::LoadAllTextures] Loading texture " << path << std::endl;
                if (texID == 0)
                    std::cerr << "[TextureManager::LoadAllTextures] Failed to preload: " << path << std::endl;
            }
        }
    };
    loadList({
        Textures::BaseColors::WOODEN_BOARD_BC,
        Textures::BaseColors::GRASS_BC,
        Textures::BaseColors::SAND_BEACH_BC,
        Textures::BaseColors::T_GRASS_BC,
        Textures::BaseColors::TOWER_BC,
        Textures::BaseColors::WOOD_DOOR_BC
    });
    loadList({
        Textures::Normals::WOODEN_BOARD_NORMAL,
        Textures::Normals::GRASS_NORMAL,
        Textures::Normals::SAND_BEACH_NORMAL,
        Textures::Normals::TOWER_NORMAL,
        Textures::Normals::WOOD_DOOR_NORMAL,
        Textures::Normals::TALL_GRASS_NORMAL
    });
    loadList({
        Textures::Roughness::WOODEN_BOARD_ROUGHNESS,
        Textures::Roughness::GRASS_ROUGHNESS,
        Textures::Roughness::SAND_BEACH_ROUGHNESS,
        Textures::Roughness::TOWER_ROUGHNESS,
        Textures::Roughness::WOOD_DOOR_ROUGHNESS
    });
    loadList({
        Textures::AmbientOcclusion::GRASS_AO,
        Textures::AmbientOcclusion::SAND_ALPHA,

    });
    loadList({
        Textures::Metallic::SAND_METALLIC
    });
    loadList({
        Textures::Displacement::SAND_DISPLACEMENT
        });
    loadList({
        Textures::Mask::GRASS_MASK,
        Textures::Mask::TALL_GRASS_MASK
    });
    loadList({
        Textures::UnpackedTexture::TALL_GRASS_ORS
    });
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
    if (path.find("_Metallic") != std::string::npos)
        return Textures::TextureType::Metallic;
    if (path.find("_Displacement") != std::string::npos)
        return Textures::TextureType::Displacement;

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


    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(fixedPath.c_str(), &width, &height, &channels, 0);
    if (!data) {
        std::cerr << "[TextureManager::LoadTexture] Failed to load texture: " << fixedPath << std::endl;
        return 0;
    }

    GLenum format;
    if (channels == 1) format = GL_RED;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;
    else {
        stbi_image_free(data);
        std::cerr << "[TextureManager::LoadTexture] Unsupported channels in texture: " << fixedPath << std::endl;
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
    const std::string& unpacked,
    const std::string& metallic,
    const std::string& displacement
) {

    MaterialTextures matTex;
    if (!baseColor.empty()) {
        matTex.baseColorID = LoadTexture(baseColor, Textures::TextureType::BaseColor);
        if(matTex.baseColorID == 0) std::cerr << "[TextureManager::LoadMaterialTextures] Failed to load BaseColor: " << baseColor << std::endl;
    }
    if (!normal.empty()) {
        matTex.normalID = LoadTexture(normal, Textures::TextureType::Normal);
        if(matTex.normalID == 0) std::cerr << "[TextureManager::LoadMaterialTextures] Failed to load Normal: " << normal << std::endl;
    }
    if (!roughness.empty()) {
        matTex.roughnessID = LoadTexture(roughness, Textures::TextureType::Roughness);
        if(matTex.roughnessID == 0) std::cerr << "[TextureManager::LoadMaterialTextures] Failed to load Roughness: " << roughness << std::endl;
    }
    if (!ao.empty()) {
        matTex.aoID = LoadTexture(ao, Textures::TextureType::AmbientOcclusion);
        if(matTex.aoID == 0) std::cerr << "[TextureManager::LoadMaterialTextures] Failed to load AO: " << ao << std::endl;
    }
    if (!mask.empty()) {
        matTex.maskID = LoadTexture(mask, Textures::TextureType::Mask);
        if(matTex.maskID == 0) std::cerr << "[TextureManager::LoadMaterialTextures] Failed to load Mask: " << mask << std::endl;
    }
    if (!unpacked.empty()) {
        matTex.unpackedID = LoadTexture(unpacked, Textures::TextureType::Unpacked);
        if(matTex.unpackedID == 0) std::cerr << "[TextureManager::LoadMaterialTextures] Failed to load Unpacked: " << unpacked << std::endl;
    }
    if (!metallic.empty()) {
        matTex.metallicID = LoadTexture(unpacked, Textures::TextureType::Metallic);
        if(matTex.metallicID == 0) std::cerr << "[TextureManager::LoadMaterialTextures] Failed to load Metallic: " << unpacked << std::endl;
    }
    if (!displacement.empty()) {
        matTex.displacementID = LoadTexture(unpacked, Textures::TextureType::Displacement);
        if(matTex.displacementID == 0) std::cerr << "[TextureManager::LoadMaterialTextures] Failed to load Displacement: " << unpacked << std::endl;
    }
    return matTex;
}


void TextureManager::Clear() {
    for (auto& pair : textures) {
        glDeleteTextures(1, &pair.second);
    }
    textures.clear();
}
