#pragma once

#include <frc2/command/SubsystemBase.h>
#include <ctre/phoenix/motorcontrol/can/TalonSRX.h>
#include <frc/Relay.h>
#include <frc/Solenoid.h>
#include <frc/Relay.h>
#include <rev/CANSparkMax.h>

#include "Constants.h"

class Climb : public frc2::SubsystemBase {
    public:
        Climb();
        void Periodic() override;

        void PistonExtend();
        void PistonRetract();

        void WinchCableOut(double percentSpeed);
        void WinchCableIn(double percentSpeed);
        void WinchStop();

        void WinchLock();
        void WinchUnlock();

        void RollLeft();
        void RollRight();
        void RollStop();

        // Contract: m_IsClimbing = m_IsClimbing || m_IsWinchOut
        bool IsClimbing() { return m_IsClimbing; }
        bool IsPistonExtended() { return m_IsPistonExtended; }
        bool IsWinchCableOut() { return m_IsWinchCableOut; }
        bool IsWinchLocked() { return m_IsWinchLocked; }

    private:
        void SetWinchSpeed(double speed);

        void SetPistonFlag();
        void ResetPistonFlag();

        void SetWinchCableOutFlag();
        // No reset winch flag.  Cannot detect when winch is fully reset.
        // Restart robot code if need cleared.

        void SetWinchLockFlag();
        void ResetWinchLockFlag();
        
        bool m_IsClimbing = false;
        bool m_IsPistonExtended = false;
        bool m_IsWinchCableOut = false;
        bool m_IsWinchLocked = false;

        ctre::phoenix::motorcontrol::can::TalonSRX m_ClimbWinchMotor {ClimbPins::kClimbWinchMotor};

        frc::Solenoid m_ClimbExtendSolenoid {kPCM2, ClimbPins::kClimbExtendSolenoid};
        frc::Solenoid m_ClimbRetractSolenoid {kPCM2, ClimbPins::kClimbRetractSolenoid};

        frc::Solenoid m_BrakeLockSolenoid {ClimbPins::kBrakeLockSolenoid};
        frc::Solenoid m_BrakeUnlockSolenoid {ClimbPins::kBrakeUnlockSolenoid};

        rev::CANSparkMax m_ClimbRoller {ClimbPins::kClimbRollerMotor, rev::CANSparkMax::MotorType::kBrushless};
};

