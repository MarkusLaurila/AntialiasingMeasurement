#pragma once
#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include <iostream>

namespace Textures {

    enum class TextureType {
        BaseColor,
        Normal,
        Roughness,
        AmbientOcclusion,
        Mask,
        Unpacked,

    };

    // Texture paths
    namespace BaseColors {
        const std::string WOODEN_BOARD_BC = "../resources/textures/Wooden_Board_Fence_BaseColor.jpg";
        const std::string GRASS_BC = "../resources/textures/Grass_basecolor.jpg";
        const std::string SAND_BEACH_BC = "../resources/textures/sand_beach_BaseColor.png";
        const std::string T_GRASS_BC = "../resources/textures/T_Grass_Generic_b1_BC.jpg";
        const std::string TOWER_BC = "../resources/textures/Tower_BaseC.png";
        const std::string WOOD_DOOR_BC = "../resources/textures/WoodDoor_BaseC.png";
    }

    namespace Normals {
        const std::string WOODEN_BOARD_NORMAL = "../resources/textures/Wooden_Board_Fence_Normal.jpg";
        const std::string GRASS_NORMAL = "../resources/textures/Grass_normal.jpg";
        const std::string SAND_BEACH_NORMAL = "../resources/textures/sand_beach_Normal.jpg";
        const std::string TOWER_NORMAL = "../resources/textures/Tower_normal.jpg";
        const std::string WOOD_DOOR_NORMAL = "../resources/textures/WoodDoor_normal.jpg";
        const std::string TALL_GRASS_NORMAL = "../resources/textures/T_Grass_Generic_b1_NM.jpg";
    }

    namespace Roughness {
        const std::string WOODEN_BOARD_ROUGHNESS = "../resources/textures/Wooden_Board_Fence_Roughness.jpg";
        const std::string GRASS_ROUGHNESS = "../resources/textures/Grass_roughness.jpg";
        const std::string SAND_BEACH_ROUGHNESS = "../resources/textures/sand_beach_Roughness.jpg";
        const std::string TOWER_ROUGHNESS = "../resources/textures/Tower_Roughness.png";
        const std::string WOOD_DOOR_ROUGHNESS = "../resources/textures/WoodDoor_roughness.png";
    }

    namespace AmbientOcclusion {
        const std::string GRASS_AO = "../resources/textures/Grass_ambientocclusion.jpg";
        const std::string SAND_ALPHA = "../resources/textures/sand_beach_Alpha.jpg";
        const std::string SAND_METALLIC = "../resources/textures/sand_beach_Metallic.jpg";
        const std::string SAND_DISPLACEMENT = "../resources/textures/sand_beach_Displacement.jpg";
    }

    namespace Mask {
        const std::string GRASS_MASK = "../resources/textures/grassMask.png";
        const std::string TALL_GRASS_MASK = "../resources/textures/T_Grass_Generic_b1_OP.png";
    }

    namespace UnpackedTexture {
        const std::string TALL_GRASS_ORS = "../resources/textures/T_Grass_Generic_b1_ORS.jpg";
    }

}

struct MaterialTextures {
    GLuint baseColorID = 0;
    GLuint normalID = 0;
    GLuint roughnessID = 0;
    GLuint aoID = 0;
    GLuint maskID = 0;
    GLuint unpackedID = 0;
};

class TextureManager {
public:

    static GLuint LoadTexture(const std::string& path, Textures::TextureType type = Textures::TextureType::BaseColor);
    static std::string ResolvePath(const std::string& originalPath, const std::string& fallbackDir);

    static MaterialTextures LoadMaterialTextures(
        const std::string& baseColor = "",
        const std::string& normal = "",
        const std::string& roughness = "",
        const std::string& ao = "",
        const std::string& mask = "",
        const std::string& unpacked = ""
    );


    static void Clear();

private:
    static std::unordered_map<std::string, GLuint> textures;


    static Textures::TextureType InferTextureType(const std::string& path);
};
