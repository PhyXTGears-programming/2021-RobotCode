#pragma once

#include <frc2/command/CommandBase.h>
#include <frc2/command/CommandHelper.h>

#include "Units.h"

#include "subsystems/Intake.h"
#include "subsystems/PowerCellCounter.h"
#include "subsystems/Shooter.h"

class AimShootCommand : public frc2::CommandHelper<frc2::CommandBase, AimShootCommand> {
    public:
        explicit AimShootCommand(rpm_t shootSpeed, Shooter* shooter, Intake* intake, PowerCellCounter* counter);
        void Initialize();
        void Execute();
        void End(bool interrupted);
        bool IsFinished();

    private:
        Shooter* m_Shooter;
        Intake* m_Intake;
        PowerCellCounter* m_PowerCellCounter;

        rpm_t m_ShootSpeed = 0_rpm;

        bool feederActivated = false;
};
