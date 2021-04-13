#include "pixy/PixyBlock.h"

PixyBlock::PixyBlock(uint8_t * block) {
    m_X = (uint16_t)block[2] | (uint16_t)block[3] << 8;     // 00000000xxxxxxxx | (yyyyyyyy << 8)
    m_Y = (uint16_t)block[4] | (uint16_t)block[5] << 8;
    m_Width = (uint16_t)block[6] | (uint16_t)block[7] << 8;
    m_Height = (uint16_t)block[8] | (uint16_t)block[9] << 8;
    m_Index = (uint16_t)block[12];
    m_Age = (uint16_t)block[13];
}

int PixyBlock::Area() const {
    return m_Width *  m_Height;
}
