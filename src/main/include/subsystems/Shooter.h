#pragma once

#include "Constants.h"

#include <frc/SpeedControllerGroup.h>
#include <frc2/command/SubsystemBase.h>
#include <frc/controller/PIDController.h>
#include <rev/CANSparkMax.h>
#include <ctre/phoenix/motorcontrol/can/TalonSRX.h>
#include <networktables/NetworkTableInstance.h>
#include <units/units.h>

class Shooter : public frc2::SubsystemBase {
    public:
        Shooter();
        void Periodic() override;

        void SetShooterMotorSpeed(units::angular_velocity::revolutions_per_minute_t speed);

        inline void SetTracking (bool enabled) { m_TrackingActive = enabled; }
        inline bool GetTracking () { return m_TrackingActive; } 

        void SetTurretSpeed(units::angular_velocity::revolutions_per_minute_t speed);

    private:
        bool m_TrackingActive = false;

        rev::CANSparkMax m_ShooterMotor1 {kShooterMotor1, rev::CANSparkMax::MotorType::kBrushless};
        rev::CANPIDController m_ShooterMotor1PID {m_ShooterMotor1};
        rev::CANEncoder m_ShooterMotor1Encoder {m_ShooterMotor1};

        rev::CANSparkMax m_ShooterMotor2 {kShooterMotor2, rev::CANSparkMax::MotorType::kBrushless};
        rev::CANPIDController m_ShooterMotor2PID {m_ShooterMotor2};
        rev::CANEncoder m_ShooterMotor2Encoder {m_ShooterMotor2};

        ctre::phoenix::motorcontrol::can::TalonSRX m_TurretMotor {kTurretMotor};

        std::shared_ptr<nt::NetworkTable> m_VisionTable;
        frc2::PIDController m_TurretPID {0.05, 0, 0};
};
