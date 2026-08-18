#ifndef MINECRAFT_RECREATION_RECREATION_MESHDATA_H
#define MINECRAFT_RECREATION_RECREATION_MESHDATA_H


namespace engine::meshdata
{
struct MeshData
{
    inline static unsigned int selVAO {}, selVBO = {};
    inline static unsigned int crossVAO = {}, crossVBO = {};
    inline static unsigned int blockVAO = {}, blockVBO = {};

    static void Init();
};
} // engine::meshdata

#endif //MINECRAFT_RECREATION_RECREATION_MESHDATA_H
