#pragma once

#include <stdint.h>

struct PixyBlock {
   int m_Index;
   int m_Age;
   int m_X;
   int m_Y;
   int m_Width;
   int m_Height;

    PixyBlock(uint8_t * block);
    PixyBlock(const PixyBlock &block) = default;
    PixyBlock(int index, int age, int x, int y, int width, int height)
        : m_Index(index)
        , m_Age(age)
        , m_X(x)
        , m_Y(y)
        , m_Width(width)
        , m_Height(height) {}

    int Area() const;
};
