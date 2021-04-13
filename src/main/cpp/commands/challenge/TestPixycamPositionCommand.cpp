#include <iostream>
#include <iomanip>

#include "PickupCellsChallenge.h"

#include "commands/challenge/TestPixycamPositionCommand.h"

using challenge::Layout;

static Layout find_best(int ared, int ablue, int bred, int bblue);

TestPixycamPositionCommand::TestPixycamPositionCommand (Pixycam* pixy) {
    AddRequirements(pixy);

    m_Pixy = pixy;
}

void TestPixycamPositionCommand::Initialize () {}

void TestPixycamPositionCommand::Execute () {
    std::vector<PixyBlock> blockList = m_Pixy->GetBlocks();

    for (auto block : blockList) {
        if (OLD_BLOCK_LIMIT > block.m_Age) {
            continue;
        }

        std::cout
            << "id: "
            << std::setfill('_')
            << std::setw(3)
            << block.m_Index
            << " | age: "
            << std::setfill('_')
            << std::setw(3)
            << block.m_Age
            << " | xy: "
            << std::setfill('_')
            << std::setw(4)
            << block.m_X
            << ","
            << std::setfill('_')
            << std::setw(4)
            << block.m_Y
            << std::endl;
    }

    std::cout << std::endl;
}

void TestPixycamPositionCommand::End (bool interrupted) {}

bool TestPixycamPositionCommand::IsFinished () {
    return false;
}