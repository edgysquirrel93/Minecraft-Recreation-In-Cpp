#ifndef MINECRAFT_RECREATION_RECREATION_MESHDATA_H
#define MINECRAFT_RECREATION_RECREATION_MESHDATA_H


namespace engine::meshdata
{

struct UVCoord {
    float u1, v1, u2, v2;
};

struct MeshData
{
    inline static unsigned int selVAO {}, selVBO = {};
    inline static unsigned int crossVAO = {}, crossVBO = {};
    inline static unsigned int blockVAO = {}, blockVBO = {};

    static void Init();

    static UVCoord getTileUV(int col, int row);
};
} // engine::meshdata

#endif //MINECRAFT_RECREATION_RECREATION_MESHDATA_H
