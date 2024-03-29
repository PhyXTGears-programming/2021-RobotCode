#include "subsystems/Climb.h"

#include <ctre/phoenix/motorcontrol/ControlMode.h>
#include <frc/smartdashboard/SmartDashboard.h>

#include <algorithm>

#define MAX_WINCH_SPEED 1.0
#define MIN_WINCH_SPEED 0.0

Climb::Climb () {}

void Climb::Periodic () {
    frc::SmartDashboard::PutBoolean("Winch Locked", m_IsWinchLocked);
    frc::SmartDashboard::PutBoolean("Climb Piston Out", !m_IsPistonExtended);
}

void Climb::PistonExtend () {
    m_ClimbExtendSolenoid.Set(true);
    m_ClimbRetractSolenoid.Set(false);
    m_IsPistonExtended = true;
}

void Climb::PistonRetract () {
    m_ClimbExtendSolenoid.Set(false);
    m_ClimbRetractSolenoid.Set(true);
    m_IsPistonExtended = false;
}

void Climb::WinchCableOut(double percentSpeed) {
    // Winch motor cannot turn when latch is engaged.
    if (m_IsWinchLocked) {
        return;
    }

    percentSpeed = std::clamp(percentSpeed, 0.0, 1.0);
    auto speed = (MAX_WINCH_SPEED - MIN_WINCH_SPEED) * percentSpeed + MIN_WINCH_SPEED;
    SetWinchSpeed(-speed);

    SetWinchCableOutFlag();
}

void Climb::WinchCableIn(double percentSpeed) {
    // Winch motor cannot turn when latch is engaged.
    if (m_IsWinchLocked) {
        return;
    }

    percentSpeed = std::clamp(percentSpeed, 0.0, 1.0);
    auto speed = (MAX_WINCH_SPEED - MIN_WINCH_SPEED) * percentSpeed + MIN_WINCH_SPEED;
    SetWinchSpeed(speed);
}

void Climb::WinchStop () {
    SetWinchSpeed(0.0);
}

void Climb::WinchLock () {
    m_BrakeLockSolenoid.Set(true);
    m_BrakeUnlockSolenoid.Set(false);
    
    // Stop the winch motor, we stalled it.
    WinchStop();

    SetWinchLockFlag();
}

void Climb::WinchUnlock () {
    m_BrakeLockSolenoid.Set(false);
    m_BrakeUnlockSolenoid.Set(true);

    ResetWinchLockFlag();
}

void Climb::RollLeft () {
    m_ClimbRoller.Set(-1.0);
}

void Climb::RollRight () {
    m_ClimbRoller.Set(1.0);
}

void Climb::RollStop () {
    m_ClimbRoller.Set(0.0);
}

void Climb::SetWinchSpeed (double speed) {
    m_ClimbWinchMotor.Set(ctre::phoenix::motorcontrol::ControlMode::PercentOutput, speed);
}

void Climb::SetPistonFlag () {
    m_IsPistonExtended = true;
}

void Climb::ResetPistonFlag () {
    m_IsPistonExtended = false;
}

void Climb::SetWinchCableOutFlag () {
    m_IsWinchCableOut = true;
    m_IsClimbing = true;
}

void Climb::SetWinchLockFlag () {
    m_IsWinchLocked = true;
}

void Climb::ResetWinchLockFlag () {
    m_IsWinchLocked = false;
}
