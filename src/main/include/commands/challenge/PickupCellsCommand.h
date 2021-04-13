#pragma once

#include <frc2/command/Command.h>
#include <frc2/command/CommandBase.h>
#include <frc2/command/CommandHelper.h>

#include "LayoutDetector.h"

#include "commands/FollowPolybezier.h"

#include "subsystems/Drivetrain.h"
#include "subsystems/Intake.h"
#include "subsystems/Pixycam.h"

class PickupCellsCommand : public frc2::CommandHelper<frc2::CommandBase, PickupCellsCommand> {
    public:
        explicit PickupCellsCommand(
            Drivetrain* drivetrain,
            Intake* intake,
            Pixycam* pixy,
            const FollowPolybezier::Configuration &followerConfig
        );

        void Initialize();
        void Execute();
        void End(bool interrupted);
        bool IsFinished();

    private:
        Drivetrain* m_Drivetrain;
        Intake* m_Intake;
        Pixycam* m_Pixy;

        PathARedDetector m_DetectorARed;
        PathABlueDetector m_DetectorABlue;
        PathBRedDetector m_DetectorBRed;
        PathBBlueDetector m_DetectorBBlue;

        int m_ScoreARed;
        int m_ScoreABlue;
        int m_ScoreBRed;
        int m_ScoreBBlue;

        Command* m_FollowARed;
        Command* m_FollowABlue;
        Command* m_FollowBRed;
        Command* m_FollowBBlue;
};
