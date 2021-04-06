#include "subsystems/Pixycam.h"

Pixycam::Pixycam() {
    // Settings taken from: https://github.com/PseudoResonance/Pixy2JavaAPI/blob/master/src/main/java/io/github/pseudoresonance/pixy2api/links/SPILink.java#L65
    m_Spi.SetClockRate(2000000);
    m_Spi.SetMSBFirst();
    m_Spi.SetSampleDataOnTrailingEdge();
    m_Spi.SetClockActiveLow();
    m_Spi.SetChipSelectActiveLow();
}

void Pixycam::Periodic () {}


std::vector<PixyBlock> Pixycam::GetBlocks() {
    std::vector<PixyBlock> blockVector;

    uint8_t getBlocksRequest[] = {
        0xae,
        0xc1,
        0x20,
        0x02,
        0x01,
        0xff,
    };

    uint8_t recv[256 + 6] = { 0 };


    m_Spi.Write(getBlocksRequest, sizeof(getBlocksRequest));

    if (!WaitForSync()) {
        std::cerr << "pixycam: error response sync no found" << std::endl;
        return blockVector;
    }

    if (4 != m_Spi.Read(false, &recv[0], 4)) {
        std::cerr << "pixycam: error reading header" << std::endl;
        return blockVector;
    }

    uint8_t data_size = recv[1];
    int blocks_count = data_size / 14;

    if (data_size != m_Spi.Read(false, &recv[4], data_size)) {
        std::cerr << "pixycam: error reading data" << std::endl;
        return blockVector;
    }

    for (int a = 0, b = 4; a < blocks_count; a++, b += 14) {
        blockVector.push_back(PixyBlock(&recv[b]));
    }

    return blockVector;
}

bool Pixycam::WaitForSync() {
    uint16_t sync = 0;
    uint8_t buf = 0;

    for (int a = 32; a > 0; a--) {
        m_Spi.Read(false, &buf, 1);
        // Leave sync value in little endian to use fewer cpu cycles.
        sync = ((sync << 8) | (uint16_t)buf) & 0xffff;

        // 0xc1af in little endian is 0xafc1.
        if (0xafc1 == sync) {
            return true;
        }
    }

    return false;
}
