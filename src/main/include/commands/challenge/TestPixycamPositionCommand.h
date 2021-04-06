#pragma once

#include <frc2/command/CommandBase.h>
#include <frc2/command/CommandHelper.h>

#include "LayoutDetector.h"

#include "subsystems/Pixycam.h"

class TestPixycamPositionCommand : public frc2::CommandHelper<frc2::CommandBase, TestPixycamPositionCommand> {
    public:
        explicit TestPixycamPositionCommand(Pixycam* pixy);

        void Initialize();
        void Execute();
        void End(bool interrupted);
        bool IsFinished();

    private:
        Pixycam* m_Pixy;

        PathARedDetector m_DetectorARed;
        PathABlueDetector m_DetectorABlue;
        PathBRedDetector m_DetectorBRed;
        PathBBlueDetector m_DetectorBBlue;

        int m_ScoreARed;
        int m_ScoreABlue;
        int m_ScoreBRed;
        int m_ScoreBBlue;
};
