#pragma once

#include <units/angular_velocity.h>

#include <frc2/command/CommandBase.h>
#include <frc2/command/CommandHelper.h>

#include "Units.h"

#include "subsystems/Shooter.h"
#include "subsystems/Intake.h"

class ShootCommand : public frc2::CommandHelper<frc2::CommandBase, ShootCommand> {
    public:
        explicit ShootCommand(Shooter* shooter, Intake* intake, rpm_t speed);
        void Initialize();
        void Execute();
        void End(bool interrupted);

    private:
        Shooter* m_Shooter;
        Intake* m_Intake;

        rpm_t m_Speed;

        bool feederActivated = false;
};
