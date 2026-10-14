#ifndef MINECRAFT_RECREATION_RECREATION_REGIONFILE_H
#define MINECRAFT_RECREATION_RECREATION_REGIONFILE_H

#include "ChunkSerialiser.h"
#include <string>
#include <fstream>
#include <optional>

namespace engine::chunkio
{
class RegionFile {
    std::string m_FilePath;
    mutable std::fstream m_FileStream;
    int m_RegionX;
    int m_RegionZ;

    void initializeHeaderIfNeeded() const;
    static uint32_t toBigEndian(uint32_t val);
    static uint32_t fromBigEndian(uint32_t val);

public:
    RegionFile(int regionX, int regionZ, const std::string& worldDirectory);
    ~RegionFile();

    [[nodiscard]] bool hasChunk(int chunkX, int chunkZ) const;
    [[nodiscard]] std::optional<SerializedChunk> readChunk(int chunkX, int chunkZ) const;
    void writeChunk(int chunkX, int chunkZ, const SerializedChunk& serialized) const;
};
} //  engine::chunkio

#endif //MINECRAFT_RECREATION_RECREATION_REGIONFILE_H
