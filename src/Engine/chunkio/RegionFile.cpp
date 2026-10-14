#include "RegionFile.h"

#include <filesystem>

namespace engine::chunkio
{
RegionFile::RegionFile(const int regionX, const int regionZ, const std::string& worldDirectory)
    : m_RegionX(regionX), m_RegionZ(regionZ) {

    const std::filesystem::path dir(worldDirectory);
    const std::filesystem::path chunkioDir = dir / "world";
    std::filesystem::create_directories(chunkioDir);

    const std::string filename = "r." + std::to_string(regionX) + "." + std::to_string(regionZ) + ".mca";
    const std::filesystem::path fullPath = chunkioDir / filename;
    m_FilePath = fullPath.string();

    m_FileStream.open(m_FilePath, std::ios::in | std::ios::out | std::ios::binary);
    if (!m_FileStream.is_open()) {
        m_FileStream.open(m_FilePath, std::ios::out | std::ios::binary);
        m_FileStream.close();
        m_FileStream.open(m_FilePath, std::ios::in | std::ios::out | std::ios::binary);
    }

    initializeHeaderIfNeeded();
}

RegionFile::~RegionFile() {
    if (m_FileStream.is_open()) {
        m_FileStream.close();
    }
}

uint32_t RegionFile::toBigEndian(const uint32_t val) {
    return ((val >> 24) & 0xff) |
           ((val << 8) & 0xff0000) |
           ((val >> 8) & 0xff00) |
           ((val << 24) & 0xff000000);
}

uint32_t RegionFile::fromBigEndian(const uint32_t val) {
    return toBigEndian(val);
}

void RegionFile::initializeHeaderIfNeeded() const
{
    m_FileStream.seekp(0, std::ios::end);

    if (const std::streampos length = m_FileStream.tellp(); length < 8192) {
        std::vector<uint8_t> emptyHeader(8192, 0);
        m_FileStream.seekp(0, std::ios::beg);
        m_FileStream.write(reinterpret_cast<char*>(emptyHeader.data()), static_cast<long>(emptyHeader.size()));
        m_FileStream.flush();
    }
}

static int getLocalIndex(const int chunkX, const int chunkZ) {
    const int localX = chunkX & 31;
    const int localZ = chunkZ & 31;
    return localX + localZ * 32;
}

bool RegionFile::hasChunk(const int chunkX, const int chunkZ) const {
    const int index = getLocalIndex(chunkX, chunkZ);
    m_FileStream.seekg(index * 4, std::ios::beg);

    uint32_t locationEntry = 0;
    m_FileStream.read(reinterpret_cast<char*>(&locationEntry), 4);
    locationEntry = fromBigEndian(locationEntry);

    return locationEntry != 0;
}

std::optional<SerializedChunk> RegionFile::readChunk(const int chunkX, const int chunkZ) const
{
    const int index = getLocalIndex(chunkX, chunkZ);
    m_FileStream.seekg(index * 4, std::ios::beg);

    uint32_t locationEntry = 0;
    m_FileStream.read(reinterpret_cast<char*>(&locationEntry), 4);
    locationEntry = fromBigEndian(locationEntry);

    const uint32_t offset = (locationEntry >> 8) & 0xFFFFFF;

    if (const uint8_t sectorCount = locationEntry & 0xFF; offset == 0 || sectorCount == 0) {
        return std::nullopt;
    }

    m_FileStream.seekg(offset * 4096, std::ios::beg);

    uint32_t lengthPrefix = 0;
    m_FileStream.read(reinterpret_cast<char*>(&lengthPrefix), 4);
    lengthPrefix = fromBigEndian(lengthPrefix);

    uint8_t compressionType = 0;
    m_FileStream.read(reinterpret_cast<char*>(&compressionType), 1);

    if (compressionType != 2) {
        return std::nullopt;
    }

    const uint32_t payloadLength = lengthPrefix - 1;
    uint16_t subChunkMask = 0;
    m_FileStream.read(reinterpret_cast<char*>(&subChunkMask), 2);

    std::vector<uint8_t> compressedData(payloadLength - 2);
    m_FileStream.read(reinterpret_cast<char*>(compressedData.data()), static_cast<long>(compressedData.size()));

    SerializedChunk result;
    result.subChunkMask = subChunkMask;
    result.compressedData = std::move(compressedData);
    return result;
}

void RegionFile::writeChunk(const int chunkX, const int chunkZ, const SerializedChunk& serialized) const {
    const int index = getLocalIndex(chunkX, chunkZ);

    const uint32_t payloadLength = 1 + 2 + static_cast<uint32_t>(serialized.compressedData.size());
    const uint32_t totalBytesNeeded = payloadLength + 4;
    const uint32_t sectorsNeeded = (totalBytesNeeded + 4095) / 4096;

    m_FileStream.seekg(index * 4, std::ios::beg);
    uint32_t locationEntry = 0;
    m_FileStream.read(reinterpret_cast<char*>(&locationEntry), 4);
    locationEntry = fromBigEndian(locationEntry);

    const uint32_t existingOffset = (locationEntry >> 8) & 0xFFFFFF;
    const uint8_t existingSectors = locationEntry & 0xFF;

    uint32_t sectorOffset = 0;

    if (existingOffset != 0 && existingSectors >= sectorsNeeded) {
        sectorOffset = existingOffset;
    } else {
        m_FileStream.seekp(0, std::ios::end);
        const std::streampos fileEnd = m_FileStream.tellp();
        sectorOffset = static_cast<uint32_t>(fileEnd) / 4096;
        if (sectorOffset < 2) sectorOffset = 2;
    }

    m_FileStream.seekp(sectorOffset * 4096, std::ios::beg);

    const uint32_t bigEndianLength = toBigEndian(payloadLength);
    m_FileStream.write(reinterpret_cast<const char*>(&bigEndianLength), 4);

    uint8_t compressionType = 2;
    m_FileStream.write(reinterpret_cast<char*>(&compressionType), 1);
    m_FileStream.write(reinterpret_cast<const char*>(&serialized.subChunkMask), 2);
    m_FileStream.write(reinterpret_cast<const char*>(serialized.compressedData.data()), static_cast<long>(serialized.compressedData.size()));

    const uint32_t writtenBytes = 4 + payloadLength;
    if (const uint32_t allocatedBytes = sectorsNeeded * 4096; writtenBytes < allocatedBytes) {
        const std::vector<char> padding(allocatedBytes - writtenBytes, 0);
        m_FileStream.write(padding.data(), static_cast<long>(padding.size()));
    }

    m_FileStream.flush();

    const uint32_t newLocationEntry = (sectorOffset << 8) | (sectorsNeeded & 0xFF);
    const uint32_t bigEndianLocation = toBigEndian(newLocationEntry);

    m_FileStream.seekp(index * 4, std::ios::beg);
    m_FileStream.write(reinterpret_cast<const char*>(&bigEndianLocation), 4);
    m_FileStream.flush();
}
} // engine::chunkio