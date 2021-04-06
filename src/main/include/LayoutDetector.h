#pragma once

#include "pixy/PixyBlock.h"

#include <stdint.h>
#include <map>
#include <vector>

struct BlockStats;

class PathARedDetector;
class PathABlueDetector;

class PathBRedDetector;
class PathBBlueDetector;

class LayoutDetector {
    // A possible candidate is a potential power cell or really good false
    // positive. We have not, yet, settled on the three we like.
    std::map<uint8_t, BlockStats> possibles;

public:
    struct Point {
        int x, y;

        Point(int x, int y) : x(x), y(y) {}
    };


    virtual std::vector<Point> ActualPoints() = 0;
    virtual bool FilterBlock(const PixyBlock &block) = 0;

    void Reset();
    void ProcessBlocks(const std::vector<PixyBlock> &blocks);
    bool ProcessBlock(const PixyBlock &block);
    int GetError();

    friend PathARedDetector;
    friend PathABlueDetector;

    friend PathBRedDetector;
    friend PathBBlueDetector;
};

class PathARedDetector : public LayoutDetector {
public:
    std::vector<LayoutDetector::Point> ActualPoints() override;
    bool FilterBlock(const PixyBlock &block) override;
};

class PathABlueDetector : public LayoutDetector {
public:
    std::vector<LayoutDetector::Point> ActualPoints() override;
    bool FilterBlock(const PixyBlock &block) override;
};

class PathBRedDetector : public LayoutDetector {
public:
    std::vector<LayoutDetector::Point> ActualPoints() override;
    bool FilterBlock(const PixyBlock &block) override;
};

class PathBBlueDetector : public LayoutDetector {
public:
    std::vector<LayoutDetector::Point> ActualPoints() override;
    bool FilterBlock(const PixyBlock &block) override;
};


// Track useful information about PixyBlocks that allows us to sift through the
// noise and incongruities to distinguish cell layouts.
struct BlockStats {
    int m_Index;
    int m_MaxAge;

    // Average x positions.
    int m_TotalX;
    int m_NumX;

    // Average y positions.
    int m_TotalY;
    int m_NumY;

    // Average widths.
    int m_TotalWidth;
    int m_NumWidth;

    // Average heights.
    int m_TotalHeight;
    int m_NumHeight;

    BlockStats() = default;
    BlockStats(const PixyBlock &block);
    BlockStats(const BlockStats &stats) = default;

    void Update(const PixyBlock &block);
};
