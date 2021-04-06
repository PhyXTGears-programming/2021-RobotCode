#include <iostream>
#include <iomanip>

#include "PickupCellsChallenge.h"

#include "commands/challenge/TestPixycamDetectorCommand.h"

using challenge::Layout;

static Layout find_best(int ared, int ablue, int bred, int bblue);

TestPixycamDetectorCommand::TestPixycamDetectorCommand (Pixycam* pixy) {
    AddRequirements(pixy);

    m_Pixy = pixy;
}

void TestPixycamDetectorCommand::Initialize () {
    m_DetectorARed.Reset();
    m_DetectorABlue.Reset();
    m_DetectorBRed.Reset();
    m_DetectorBBlue.Reset();

    m_ScoreARed = 0;
    m_ScoreABlue = 0;
    m_ScoreBRed = 0;
    m_ScoreBBlue = 0;
}

void TestPixycamDetectorCommand::Execute () {
    std::vector<PixyBlock> blockList = m_Pixy->GetBlocks();

    m_DetectorARed.ProcessBlocks(blockList);
    m_DetectorABlue.ProcessBlocks(blockList);
    m_DetectorBRed.ProcessBlocks(blockList);
    m_DetectorBBlue.ProcessBlocks(blockList);

    int errorARed  = m_DetectorARed.GetError();
    int errorABlue = m_DetectorABlue.GetError();
    int errorBRed  = m_DetectorBRed.GetError();
    int errorBBlue = m_DetectorBBlue.GetError();

    Layout bestLayout = find_best(
        errorARed,
        errorABlue,
        errorBRed,
        errorBBlue
    );

    std::cout
        << "a-red: "
        << std::setfill('_')
        << std::setw(6)
        << std::min(errorARed, 999999)
        << " | a-blue: "
        << std::setfill('_')
        << std::setw(6)
        << std::min(errorABlue, 999999)
        << " | b-red: "
        << std::setfill('_')
        << std::setw(6)
        << std::min(errorBRed, 999999)
        << " | b-blue: "
        << std::setfill('_')
        << std::setw(6)
        << std::min(errorBBlue, 999999);

    switch (bestLayout) {
        case Layout::A_RED:
            m_ScoreARed += 1;
            std::cout << " | best: A RED";
            break;

        case Layout::A_BLUE:
            m_ScoreABlue += 1;
            std::cout << " | best: A BLUE";
            break;

        case Layout::B_RED:
            m_ScoreBRed += 1;
            std::cout << " | best: B RED";
            break;

        case Layout::B_BLUE:
            m_ScoreBBlue += 1;
            std::cout << " | best: B BLUE";
            break;

        default:
            std::cout << " | best: NONE";
            break;
    }

    std::cout << std::endl;
}

void TestPixycamDetectorCommand::End (bool interrupted) {
    if (interrupted) {
        return;
    }

    if (WIN_SCORE <= m_ScoreARed) {
        std::cout << "auto: Pickup path A Red" << std::endl;
    } else if (WIN_SCORE <= m_ScoreABlue) {
        std::cout << "auto: Pickup path A Blue" << std::endl;
    } else if (WIN_SCORE <= m_ScoreBRed) {
        std::cout << "auto: Pickup path B Red" << std::endl;
    } else if (WIN_SCORE <= m_ScoreBBlue) {
        std::cout << "auto: Pickup path B Blue" << std::endl;
    }
}

bool TestPixycamDetectorCommand::IsFinished () {
    return (
        WIN_SCORE <= m_ScoreARed
        || WIN_SCORE <= m_ScoreABlue
        || WIN_SCORE <= m_ScoreBRed
        || WIN_SCORE <= m_ScoreBBlue
    );
}

// Check errors from detectors and find a winner, else CONFUSED layout if no
// clear winner.
static Layout find_best(int ared, int ablue, int bred, int bblue) {
    Layout bestLayout = Layout::CONFUSED;

    std::map<Layout, int> errorMap;

    errorMap[Layout::A_RED] = ared;
    errorMap[Layout::A_BLUE] = ablue;
    errorMap[Layout::B_RED] = bred;
    errorMap[Layout::B_BLUE] = bblue;

    for (auto pair : errorMap) {
        if (MAX_ERROR_FOR_MATCH > pair.second) {
            if (Layout::CONFUSED != bestLayout) {
                return Layout::CONFUSED;
            } else {
                bestLayout = pair.first;
            }
        }
    }

    return bestLayout;
}