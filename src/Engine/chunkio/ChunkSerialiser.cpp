//
// Created by alex on 10/12/26.
//

#include "ChunkSerialiser.h"

#include <cstring>
#include <zlib.h>

namespace engine::chunkio
{
SerializedChunk ChunkSerializer::serialize(const rendering::ChunkRendering& chunk) {
    SerializedChunk result;
    std::vector<uint8_t> rawPayload;
    const auto& blockIDs = chunk.getBlockIDs();

    for (int subY = 0; subY < 16; ++subY) {
        bool isEmpty = true;
        constexpr int subChunkBlockCount = 16 * 16 * 16;
        const int startIndex = subY * subChunkBlockCount;

        for (int i = 0; i < subChunkBlockCount; ++i) {
            if (blockIDs[startIndex + i] != 0) {
                isEmpty = false;
                break;
            }
        }

        if (!isEmpty) {
            result.subChunkMask |= (1 << subY);
            const uint8_t* subChunkPtr = &blockIDs[startIndex];
            rawPayload.insert(rawPayload.end(), subChunkPtr, subChunkPtr + subChunkBlockCount);
        }
    }

    uLongf compressedSize = compressBound(rawPayload.size());
    result.compressedData.resize(compressedSize);

    if (compress(result.compressedData.data(), &compressedSize,
                 rawPayload.data(), rawPayload.size()) != Z_OK) {
        throw std::runtime_error("Failed to compress chunk data with zlib.");
                 }

    result.compressedData.resize(compressedSize);
    return result;
}
void ChunkSerializer::deserialize(rendering::ChunkRendering& chunk, const SerializedChunk& data) {
    auto& blockIDs = chunk.getBlockIDs();
    std::ranges::fill(blockIDs, 0);

    int activeSubChunkCount = 0;
    for (int i = 0; i < 16; ++i) {
        if ((data.subChunkMask & (1 << i)) != 0) {
            activeSubChunkCount++;
        }
    }

    uLongf uncompressedSize = activeSubChunkCount * 4096;
    std::vector<uint8_t> uncompressedData(uncompressedSize);

    if (uncompress(uncompressedData.data(), &uncompressedSize,
                   data.compressedData.data(), data.compressedData.size()) != Z_OK) {
        throw std::runtime_error("Failed to uncompress chunk data with zlib.");
                   }

    int srcOffset = 0;
    for (int subY = 0; subY < 16; ++subY) {
        if ((data.subChunkMask & (1 << subY)) != 0) {
            const int destStartIndex = subY * 4096;
            std::memcpy(&blockIDs[destStartIndex], &uncompressedData[srcOffset], 4096);
            srcOffset += 4096;
        }
    }

    chunk.makeDirty();
}
} // engine::chunkio