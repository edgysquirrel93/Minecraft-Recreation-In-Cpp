#ifndef MINECRAFT_RECREATION_RECREATION_CHUNKSERIALISER_H
#define MINECRAFT_RECREATION_RECREATION_CHUNKSERIALISER_H

#include <vector>

#include "Engine/rendering/ChunkRendering.h"

namespace engine::chunkio
{
struct SerializedChunk {
    uint16_t subChunkMask{0};
    std::vector<uint8_t> compressedData;
};

class ChunkSerializer {
public:
    static SerializedChunk serialize(const rendering::ChunkRendering& chunk);
    static void deserialize(rendering::ChunkRendering& chunk, const SerializedChunk& data);
};
} // engine::chunkio

#endif //MINECRAFT_RECREATION_RECREATION_CHUNKSERIALISER_H
