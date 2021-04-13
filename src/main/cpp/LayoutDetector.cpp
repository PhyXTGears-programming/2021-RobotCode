#include <algorithm>
#include <iostream>
#include <limits>
#include <set>

#include "LayoutDetector.h"
#include "PickupCellsChallenge.h"

LayoutDetector::Point to_average_position(const BlockStats &stats) {
    return LayoutDetector::Point {
        stats.m_TotalX / stats.m_NumX,
        stats.m_TotalY / stats.m_NumY,
    };
}

int compute_error(const LayoutDetector::Point &actual, const LayoutDetector::Point &expected) {
    int dx = std::abs(actual.x - expected.x);
    int dy = std::abs(actual.y - expected.y);
    return (dx * dx + dy * dy);
}

int compute_layout_error(std::vector<LayoutDetector::Point> actualPoints, std::vector<LayoutDetector::Point> expectedPoints) {
    // Compute error by finding sum of squared distances (dot product) between
    // nearest actual and expected points.

    // Brute force. Use index table to compare every actual point with every
    // expected point. This ensures we don't compare one expected point with two
    // actual points for a given error calculation.
    static size_t indexMaps[6][3] = {
        { 0, 1, 2 },
        { 0, 2, 1 },
        { 1, 0, 2 },
        { 1, 2, 0 },
        { 2, 0, 1 },
        { 2, 1, 0 }
    };

    int bestError = std::numeric_limits<int>::max();

    for (auto indices : indexMaps) {
        int error = 0;
        for (size_t a : { 0, 1, 2 }) {
            if (actualPoints.size() > a && expectedPoints.size() > indices[a]) {
                error += compute_error(actualPoints[a], expectedPoints[indices[a]]);
            }
        }

        bestError = std::min(error, bestError);
    }

    return bestError;
}


int LayoutDetector::GetError() {
    std::vector<LayoutDetector::Point> actualPoints = ActualPoints();
    std::vector<BlockStats> liked;

    // All liked blocks must have "old" age, but "old" is slow, so we don't
    // want to wait for oldest blocks.
    for (auto pair : possibles) {
        auto stats = pair.second;
        if (OLD_BLOCK_LIMIT < stats.m_MaxAge) {
            liked.push_back(BlockStats(stats));
        }

        if (3 == liked.size()) {
            break;
        }
    }

    // Need at least two blocks.
    if (2 > liked.size()) {
        return std::numeric_limits<int>::max();
    }

    std::vector<LayoutDetector::Point> expectedPoints;

    for (auto stats: liked) {
        expectedPoints.push_back(to_average_position(stats));
    }

    return compute_layout_error(actualPoints, expectedPoints);
}

void LayoutDetector::ProcessBlocks(const std::vector<PixyBlock> &blocks) {
    std::set<int> visited;

    for (auto& block : blocks) {
        if (ProcessBlock(block)) {
            visited.insert(block.m_Index);
        }
    }

    std::set<int> notFound;

    // Find blocks that didn't update.
    for (auto pair : possibles) {
        if (visited.end() == visited.find(pair.first)) {
            notFound.insert(pair.first);
        }
    }

    // Throw away blocks that didn't update.
    for (auto index : notFound) {
        possibles.erase(index);
    }
}

bool LayoutDetector::ProcessBlock(const PixyBlock &block) {
    if (!FilterBlock(block)) {
        return false;
    }

    // If possibles lacks this block...
    if (possibles.end() != possibles.find(block.m_Index)) {
        // Add the block to possibles.
        possibles[block.m_Index] = BlockStats(block);
    } else {
        // Update the stats of the possible with this block.
        possibles[block.m_Index].Update(block);
    }

    return true;
}

void LayoutDetector::Reset() {
    possibles.clear();
}


std::vector<LayoutDetector::Point> PathARedDetector::ActualPoints() {
    static std::vector<LayoutDetector::Point> actualPoints = {
        LayoutDetector::Point(166, 136),
        LayoutDetector::Point(231, 91),
        LayoutDetector::Point(50, 79),
    };

    return actualPoints;
}

std::vector<LayoutDetector::Point> PathABlueDetector::ActualPoints() {
    static std::vector<LayoutDetector::Point> actualPoints = {
        LayoutDetector::Point(271, 83),
        LayoutDetector::Point(115, 72),
        LayoutDetector::Point(160, 66),
    };

    return actualPoints;
}

std::vector<LayoutDetector::Point> PathBRedDetector::ActualPoints() {
    static std::vector<LayoutDetector::Point> actualPoints = {
        LayoutDetector::Point(36, 136),
        LayoutDetector::Point(231, 91),
        LayoutDetector::Point(115, 72),
    };

    return actualPoints;
}

std::vector<LayoutDetector::Point> PathBBlueDetector::ActualPoints() {
    static std::vector<LayoutDetector::Point> actualPoints = {
        LayoutDetector::Point(217, 81),
        LayoutDetector::Point(191, 64),
        LayoutDetector::Point(121, 68),
    };

    return actualPoints;
}

bool PathARedDetector::FilterBlock(const PixyBlock &block) {
    return
        // Ignore tiny blocks.
        40 < block.Area()
        // Ignore young blocks.
        && YOUNG_BLOCK_LIMIT < block.m_Age;
}

bool PathABlueDetector::FilterBlock(const PixyBlock &block) {
    return
        // Ignore large blocks.
        300 > block.Area()
        // Ignore young blocks.
        && YOUNG_BLOCK_LIMIT < block.m_Age;
}

bool PathBRedDetector::FilterBlock(const PixyBlock &block) {
    return
        // Ignore large blocks.
        300 > block.Area()
        // Ignore young blocks.
        && YOUNG_BLOCK_LIMIT < block.m_Age;
}

bool PathBBlueDetector::FilterBlock(const PixyBlock &block) {
    return
        // Ignore young blocks.
        YOUNG_BLOCK_LIMIT < block.m_Age;
}

BlockStats::BlockStats(const PixyBlock &block)
    : m_Index(block.m_Index)
    , m_MaxAge(block.m_Age)
    , m_TotalX(block.m_X)
    , m_TotalY(block.m_Y)
    , m_TotalWidth(block.m_Width)
    , m_TotalHeight(block.m_Height)
    , m_NumX(1)
    , m_NumY(1)
    , m_NumWidth(1)
    , m_NumHeight(1)
{}

void BlockStats::Update(const PixyBlock &block) {
    if (m_MaxAge > block.m_Age) {
        // Something is wrong. Age of next block should never be younger than
        // what was recorded in stats.
        return;
    }

    m_MaxAge = block.m_Age;

    m_TotalX += block.m_X;
    m_NumX += 1;

    m_TotalY += block.m_Y;
    m_NumY += 1;

    m_TotalWidth += block.m_Width;
    m_NumWidth += 1;

    m_TotalHeight += block.m_Height;
    m_NumHeight += 1;
}
