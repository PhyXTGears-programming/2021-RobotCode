#include "RobotContainer.h"

#include <iostream>
#include <units/angular_velocity.h>

#include <cameraserver/CameraServer.h>

#include <frc/smartdashboard/SmartDashboard.h>
#include <frc2/command/button/JoystickButton.h>
#include <frc2/command/CommandScheduler.h>
#include <frc2/command/FunctionalCommand.h>
#include <frc2/command/InstantCommand.h>
#include <frc2/command/PrintCommand.h>
#include <frc2/command/ParallelCommandGroup.h>
#include <frc2/command/ParallelRaceGroup.h>
#include <frc2/command/SequentialCommandGroup.h>
#include <frc2/command/StartEndCommand.h>
#include <frc2/command/WaitUntilCommand.h>

#include "Units.h"

#include "commands/AimCommand.h"
#include "commands/AimShootCommand.h"
#include "commands/SimpleDriveCommand.h"
#include "commands/autonomous/AutonomousRotateTurretCommand.h"
#include "commands/DriveUntilWallCommand.h"
#include "commands/FollowPolybezier.h"

#include "commands/challenge/PickupCellsCommand.h"
#include "commands/challenge/TestPixycamDetectorCommand.h"
#include "commands/challenge/TestPixycamPositionCommand.h"

using JoystickHand = frc::GenericHID::JoystickHand;

enum Pov {
    POV_RIGHT = 90,
    POV_LEFT = 270,
    POV_UP = 0,
    POV_DOWN = 180,
};

RobotContainer::RobotContainer () {
    std::shared_ptr<cpptoml::table> toml = LoadConfig("/home/lvuser/deploy/" + ConfigFiles::ConfigFile);

    m_Drivetrain = new Drivetrain(toml->get_table("drivetrain"));
    m_Intake = new Intake(toml->get_table("intake"));
    m_Climb = new Climb();
    m_Shooter = new Shooter(toml->get_table("shooter"));
    m_ControlPanel = new ControlPanel(toml->get_table("controlPanel"));
    m_PowerCellCounter = new PowerCellCounter();

    m_Pixy = new Pixycam();

    auto aSpeed = rpm_t{
        toml->get_table("shooter")->get_qualified_as<double>("shootingSpeed.a").value_or(2500.0)
    };

    auto bSpeed = rpm_t{
        toml->get_table("shooter")->get_qualified_as<double>("shootingSpeed.b").value_or(2890)
    };

    m_IntakeBallsCommand        = new IntakeBallsCommand(m_Intake, m_PowerCellCounter);
    m_ExpelIntakeCommand        = new ExpelIntakeCommand(m_Intake);
    m_RetractIntakeCommand      = new RetractIntakeCommand(m_Intake);
    m_ExtendIntakeCommand       = new ExtendIntakeCommand(m_Intake);
    m_TeleopDriveCommand        = new TeleopDriveCommand(m_Drivetrain, &m_DriverJoystick);
    m_TeleopShootCommand        = new ShootCommand(m_Shooter, m_Intake, aSpeed);
    m_TeleopSlowShootCommand    = new ShootCommand(m_Shooter, m_Intake, bSpeed);
    m_ReverseBrushesCommand     = new ReverseBrushesCommand(m_Intake);


    m_ControlWinchCommand   = new ControlWinchCommand(m_Climb, [=] { return m_ClimbJoystick.GetY(JoystickHand::kLeftHand); });
    m_RetractClimbCommand   = new RetractClimbCommand(m_Climb);
    m_ExtendClimbCommand    = new ExtendClimbCommand(m_Climb);
    m_RollClimbLeftCommand  = new RollClimbLeftCommand(m_Climb);
    m_RollClimbRightCommand = new RollClimbRightCommand(m_Climb);
    m_LockWinchCommand      = new LockWinchCommand(m_Climb);
    m_UnlockWinchCommand    = new UnlockWinchCommand(m_Climb);

    m_ClimbCylinderExtendCommand  = new ClimbCylinderExtendCommand(m_Climb);
    m_ClimbCylinderRetractCommand = new ClimbCylinderRetractCommand(m_Climb);

    frc2::CommandScheduler::GetInstance().SetDefaultCommand(m_Drivetrain, *m_TeleopDriveCommand);
    frc2::CommandScheduler::GetInstance().RegisterSubsystem(m_Shooter);
    frc2::CommandScheduler::GetInstance().RegisterSubsystem(m_Intake);
    frc2::CommandScheduler::GetInstance().RegisterSubsystem(m_PowerCellCounter);

    InitAutonomousChooser();
    frc::SmartDashboard::PutData("Auto Modes", &m_DashboardAutoChooser);

    // Configure the button bindings
    ConfigureButtonBindings();

    auto cameraServer = frc::CameraServer::GetInstance();
    cameraServer->StartAutomaticCapture();

    m_Drivetrain->SetBrake(false);
}

frc2::Command* RobotContainer::GetAutonomousCommand () {
    return m_DashboardAutoChooser.GetSelected();
}

void RobotContainer::PollInput () {
    // ####################
    // #####  Driver  #####
    // ####################

    // Retract Intake (X)
    if (m_DriverJoystick.GetXButtonPressed()) {
        m_RetractIntakeCommand->Schedule();
        m_IntakeExtended = false;
    }

    // Brake (RB)
    if (m_DriverJoystick.GetBumperPressed(JoystickHand::kRightHand)) {
        m_Drivetrain->SetBrake(false);
    } else if (m_DriverJoystick.GetBumperReleased(JoystickHand::kRightHand)) {
        m_Drivetrain->SetBrake(true);
    }

    // ####################
    // #####   Both   #####
    // ####################

    // Shooting (driver: RB, operator: A)
    if (m_DriverJoystick.GetAButtonPressed() || m_OperatorJoystick.GetAButtonPressed()) {
        m_TeleopShootCommand->Schedule();
    } else if (m_DriverJoystick.GetAButtonReleased() || m_OperatorJoystick.GetAButtonReleased()) {
        m_TeleopShootCommand->Cancel();
    }

    // Slow shooting (operator: B)
    if (m_OperatorJoystick.GetBButtonPressed()) {
        m_TeleopSlowShootCommand->Schedule();
    } else if (m_OperatorJoystick.GetBButtonReleased()) {
        m_TeleopSlowShootCommand->Cancel();
    }

    // Intake (driver: LB, operator: RT)
    bool intakeAxis = m_OperatorJoystick.GetTriggerAxis(JoystickHand::kRightHand) > 0.1;
    if (intakeAxis && !m_IntakeBallsCommand->IsScheduled()) {
        m_IntakeBallsCommand->Schedule();
    } else if (intakeAxis <= 0.1 && m_IntakeBallsCommand->IsScheduled()) {
        m_IntakeBallsCommand->Cancel();
    }

    // ####################
    // ##### Operator #####
    // ####################

    // Camera Aiming (X)
    if (m_OperatorJoystick.GetXButtonPressed() || m_OperatorJoystick.GetYButtonPressed() || m_DriverJoystick.GetXButtonPressed()) {
        m_Shooter->SetTrackingMode(TrackingMode::CameraTracking);
    } else if (m_OperatorJoystick.GetXButtonReleased() || m_OperatorJoystick.GetYButtonReleased() || m_DriverJoystick.GetXButtonReleased()) {
        m_Shooter->ResetTurretPID();
        m_Shooter->SetTrackingMode(TrackingMode::Off);
    }

    // Control Panel Deploy (LB)
    if (m_OperatorJoystick.GetBumperPressed(JoystickHand::kLeftHand)) {
        m_ControlPanel->Extend();
    } else if (m_OperatorJoystick.GetBumperReleased(JoystickHand::kLeftHand)) {
        m_ControlPanel->Retract();
    }

    // Left Stick
    double operatorLeftX = m_OperatorJoystick.GetX(JoystickHand::kLeftHand);
    if (m_OperatorJoystick.GetBumper(JoystickHand::kLeftHand)) {
        // Control Panel Manual Control
        m_ControlPanel->SetSpeed(operatorLeftX);
    } else {
        // Manual Aiming
        if (std::fabs(operatorLeftX) > 0.15) {
            operatorLeftX = std::copysign(std::sqrt(std::fabs(operatorLeftX)), operatorLeftX);
            m_Shooter->SetTrackingMode(TrackingMode::Off);
            m_Shooter->SetTurretSpeed(operatorLeftX * 15_rpm);
            m_TurretManualControl = true;
        } else if (m_TurretManualControl) {
            m_Shooter->SetTurretSpeed(0_rpm);
            m_TurretManualControl = false;
        }
    }

    // Deploy/Retract Intake (RB)
    if (m_OperatorJoystick.GetBumperPressed(JoystickHand::kRightHand) && !m_DriverJoystick.GetXButton()) {
        if (m_Intake->IsExtended()) {
            m_RetractIntakeCommand->Schedule();
        } else {
            m_ExtendIntakeCommand->Schedule();
        }
    }

    // Expel Intake (DP Left)
    if (POV_LEFT == m_OperatorJoystick.GetPOV() && !m_ExpelIntakeCommand->IsScheduled()) {
        m_ExpelIntakeCommand->Schedule();
    } else if (POV_LEFT != m_OperatorJoystick.GetPOV() && m_ExpelIntakeCommand->IsScheduled()) {
        m_ExpelIntakeCommand->Cancel();
    }

    // Reverse Brushes (DP Right)
    if (POV_RIGHT == m_OperatorJoystick.GetPOV() && !m_ReverseBrushesCommand->IsScheduled()) {
        m_ReverseBrushesCommand->Schedule();
    } else if (POV_RIGHT != m_OperatorJoystick.GetPOV() && m_ReverseBrushesCommand->IsScheduled()) {
        m_ReverseBrushesCommand->Cancel();
    }

    // ####################
    // #####  Climb   #####
    // ####################

    // Climb Winch (LS)
    if (std::abs(m_ClimbJoystick.GetY(frc::GenericHID::JoystickHand::kLeftHand)) > 0.2) {
        if (!m_ControlWinchCommand->IsScheduled()) m_ControlWinchCommand->Schedule(true); // interruptible
    } else {
        if (m_ControlWinchCommand->IsScheduled()) m_ControlWinchCommand->Cancel();
    }

    // Climb Roll (Driver Dpad)
    if (m_DriverJoystick.GetPOV() == POV_RIGHT) { // Right
        if (!m_RollClimbRightCommand->IsScheduled()) {
            m_RollClimbRightCommand->Schedule();
        }
    } else if (m_DriverJoystick.GetPOV() == POV_LEFT) { // Left
        if (!m_RollClimbLeftCommand->IsScheduled()) {
            m_RollClimbLeftCommand->Schedule();
        }
    } else {
        m_RollClimbLeftCommand->Cancel();
        m_RollClimbRightCommand->Cancel();
    }

    // Climb Lock Winch (B)
    if (m_ClimbJoystick.GetBButtonPressed()) {
        m_LockWinchCommand->Schedule();
    }

    // Climb Unlock Winch (Y)
    if (m_ClimbJoystick.GetYButtonPressed()) {
        m_UnlockWinchCommand->Schedule();
    }

    // Climb Cylinder Retract (A)
    if (m_ClimbJoystick.GetAButtonPressed()) {
        m_ClimbCylinderExtendCommand->Schedule();
    }

    // Climb Cylinder Extend (X)
    if (m_ClimbJoystick.GetXButtonPressed()) {
        m_ClimbCylinderRetractCommand->Schedule();
    }
}

void RobotContainer::ConfigureButtonBindings () {
    // frc2::JoystickButton aButton{&m_DriverJoystick, 1};
    // aButton.WhenPressed(m_VisionAimingCommand);
    // aButton.WhenPressed(frc2::PrintCommand("Testing")).WhenReleased(frc2::PrintCommand("Released"));
}

std::shared_ptr<cpptoml::table> RobotContainer::LoadConfig (std::string path) {
    try {
        return cpptoml::parse_file(path);
    } catch (const cpptoml::parse_exception & ex) {
        std::cerr << "Unable to load config file: " << path << std::endl << ex.what() << std::endl;
        exit(1);
    }
}

void RobotContainer::InitAutonomousChooser () {
    const rpm_t kShooterSpeed = 3750_rpm;

    frc2::SequentialCommandGroup* threeCellAutoCommand = new frc2::SequentialCommandGroup(
        frc2::StartEndCommand {
            [=]() { m_Shooter->SetTurretSpeed(0.8); },
            [=]() { m_Shooter->SetTurretSpeed(0.0); },
            m_Shooter
        }.WithTimeout(0.5_s),
        AimCommand{m_Shooter}.WithTimeout(2.0_s),
        AimShootCommand{kShooterSpeed, m_Shooter, m_Intake, m_PowerCellCounter}.WithTimeout(3.5_s),
        SimpleDriveCommand{0.25, 0.0, m_Drivetrain}.WithTimeout(1.0_s)
    );

    frc2::SequentialCommandGroup driveThruTrench {
        // Drive thru trench picking up power cells.
        frc2::ParallelCommandGroup{
            frc2::SequentialCommandGroup{
                SimpleDriveCommand{0.6 * (0.85/1.0), 0.0, m_Drivetrain}.WithTimeout(1.6_s * (1.0/0.85)),
                // Decelerate.
                SimpleDriveCommand{0.4, 0.0, m_Drivetrain}.WithTimeout(0.3_s),
                SimpleDriveCommand{0.2, 0.0, m_Drivetrain}.WithTimeout(0.3_s)
            },
            // Run intake until 3 cells are collected, or timeout expires.
            // frc2::ParallelRaceGroup{
            IntakeBallsCommand{m_Intake, m_PowerCellCounter}.WithTimeout(2.5_s)
            //     frc2::WaitUntilCommand{
            //         [=]() { return 3 == m_PowerCellCounter->GetCount(); }
            //     }
            // }
        },
        // Reverse back to line.
        frc2::ParallelRaceGroup{
            SimpleDriveCommand{-0.6, 0.0, m_Drivetrain}.WithTimeout(1.6_s),
            IntakeBallsCommand{m_Intake, m_PowerCellCounter}
        },
        // Decelerate.
        SimpleDriveCommand{-0.4, 0.0, m_Drivetrain}.WithTimeout(0.3_s),
        SimpleDriveCommand{-0.2, 0.0, m_Drivetrain}.WithTimeout(0.4_s)
    };

    static hal::fpga_clock::time_point startTime;

    frc2::SequentialCommandGroup* sixCellAutoCommand = new frc2::SequentialCommandGroup(
        frc2::InstantCommand{
            [=]() {
                startTime = hal::fpga_clock::now();
                m_Shooter->SetLimelightLight(true);
            }
        },
        ExtendIntakeCommand{m_Intake},
        PreheatShooterCommand{m_Shooter},
        AutonomousRotateTurretCommand{m_Shooter}.WithTimeout(0.3_s),
        AimCommand{m_Shooter}.WithTimeout(1.0_s),
        AimShootCommand{kShooterSpeed, m_Shooter, m_Intake, m_PowerCellCounter}.WithTimeout(4.0_s),
        std::move(driveThruTrench),
        PreheatShooterCommand{m_Shooter},
        AimCommand{m_Shooter}.WithTimeout(0.5_s),
        AimShootCommand{kShooterSpeed, m_Shooter, m_Intake, m_PowerCellCounter}.WithTimeout(4.0_s),
        // RetractIntakeCommand{m_Intake},
        frc2::InstantCommand{
            [=] {
                auto now = hal::fpga_clock::now();
                auto delta = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count() / 1.0E6;
                std::cout << "Auto done in " << delta << " seconds" << std::endl;
            }
        }
    );

    frc2::SequentialCommandGroup positionBot {
        SimpleDriveCommand{-0.4, 0.0, m_Drivetrain}.WithTimeout(1.0_s),
        DriveUntilWallCommand{m_Drivetrain},
        SimpleDriveCommand{0.1, 0.0, m_Drivetrain}.WithTimeout(0.1_s)
    };

    frc2::SequentialCommandGroup* closeShotAutoCommand = new frc2::SequentialCommandGroup(
        AutonomousRotateTurretCommand{m_Shooter}.WithTimeout(0.5_s),
        AimCommand{m_Shooter}.WithTimeout(1.0_s),
        PreheatShooterCommand{m_Shooter},
        std::move(positionBot),
        ShootCommand{m_Shooter, m_Intake, 2700_rpm}.WithTimeout(4.0_s)
    );

    FollowPolybezier::Configuration followerConfig {
        5.0,    // maximumRadialAcceleration
        3.0,    // maximumJerk
        8.0     // maximumReverseAcceleration
    };

    auto getResetPose = [=](Point::Point p) {
        return frc2::InstantCommand {
            [=]() {
                m_Drivetrain->SetPose(p.x, p.y, 0);
            }
        };
    };

    FollowPolybezier barrel_racing_follower {m_Drivetrain, "/home/lvuser/deploy/paths/autonav6.json", followerConfig};
    FollowPolybezier slalom_follower {m_Drivetrain, "/home/lvuser/deploy/paths/autonav-slalom-4.json", followerConfig};
    FollowPolybezier test_follower {m_Drivetrain, "/home/lvuser/deploy/paths/testPath.json", followerConfig, true};

    FollowPolybezier bounce_follower_a {m_Drivetrain, "/home/lvuser/deploy/paths/bounce-a.json", followerConfig};
    FollowPolybezier bounce_follower_b {m_Drivetrain, "/home/lvuser/deploy/paths/bounce-b.json", followerConfig, true};
    FollowPolybezier bounce_follower_c {m_Drivetrain, "/home/lvuser/deploy/paths/bounce-c.json", followerConfig};
    FollowPolybezier bounce_follower_d {m_Drivetrain, "/home/lvuser/deploy/paths/bounce-d.json", followerConfig, true};

    frc2::SequentialCommandGroup* followPathBR = new frc2::SequentialCommandGroup(
        getResetPose(barrel_racing_follower.GetStartPoint()),
        std::move(barrel_racing_follower)
    );

    frc2::SequentialCommandGroup* followPathS = new frc2::SequentialCommandGroup(
        getResetPose(slalom_follower.GetStartPoint()),
        std::move(slalom_follower)
    );

    frc2::SequentialCommandGroup* followPathBounce = new frc2::SequentialCommandGroup(
        getResetPose(bounce_follower_a.GetStartPoint()),
        std::move(bounce_follower_a),
        std::move(bounce_follower_b),
        std::move(bounce_follower_c),
        std::move(bounce_follower_d)
    );

    frc2::SequentialCommandGroup* followPathTest = new frc2::SequentialCommandGroup(
        getResetPose(test_follower.GetStartPoint()),
        std::move(test_follower)
    );

    PickupCellsCommand* pickupCellsChallenge = new PickupCellsCommand(
        m_Drivetrain,
        m_Intake,
        m_Pixy,
        followerConfig
    );

    frc2::SequentialCommandGroup driveThroughTrenchFar {
        // Drive through trench picking up power cells
        frc2::ParallelRaceGroup{
            frc2::SequentialCommandGroup{
                SimpleDriveCommand{0.3, 0.0, m_Drivetrain}.WithTimeout(4.7_s),
                SimpleDriveCommand{0.2, 0.0, m_Drivetrain}.WithTimeout(0.4_s)
            },
            IntakeBallsCommand{m_Intake, m_PowerCellCounter}
        },
        // Reverse back to line
        frc2::ParallelRaceGroup{
            frc2::ParallelCommandGroup{
                SimpleDriveCommand{-0.6, -0.02, m_Drivetrain}.WithTimeout(2.1_s),
                AutonomousRotateTurretCommand{m_Shooter}.WithTimeout(0.5_s),
            },
            IntakeBallsCommand{m_Intake, m_PowerCellCounter},
        },
        // Decelerate
        frc2::ParallelRaceGroup{
            SimpleDriveCommand{-0.4, 0.0, m_Drivetrain}.WithTimeout(0.7_s),
            IntakeBallsCommand{m_Intake, m_PowerCellCounter},
            frc2::SequentialCommandGroup{
                PreheatShooterCommand{m_Shooter},
                AimCommand{m_Shooter}
            }
        },
        frc2::ParallelRaceGroup{
            SimpleDriveCommand{-0.2, 0.0, m_Drivetrain}.WithTimeout(0.5_s),
            IntakeBallsCommand{m_Intake, m_PowerCellCounter},
            AimCommand{m_Shooter}
        },
    };

    frc2::SequentialCommandGroup* eightCellAutoCommand = new frc2::SequentialCommandGroup(
        frc2::InstantCommand{
            [=]() {
                startTime = hal::fpga_clock::now();
                m_Shooter->SetLimelightLight(true);
            }
        },
        ExtendIntakeCommand{m_Intake},
        frc2::ParallelRaceGroup{
            frc2::SequentialCommandGroup{
                PreheatShooterCommand{m_Shooter},
                AutonomousRotateTurretCommand{m_Shooter}.WithTimeout(0.3_s),
                AimCommand{m_Shooter}.WithTimeout(1.0_s)
            },
            IntakeBallsCommand{m_Intake, m_PowerCellCounter},
        },
        AimShootCommand{kShooterSpeed, m_Shooter, m_Intake, m_PowerCellCounter}.WithTimeout(2.2_s),
        std::move(driveThroughTrenchFar),
        PreheatShooterCommand{m_Shooter},
        AimCommand{m_Shooter}.WithTimeout(0.5_s),
        AimShootCommand{kShooterSpeed, m_Shooter, m_Intake, m_PowerCellCounter}.WithTimeout(4.0_s),
        frc2::InstantCommand{
            [=] {
                auto now = hal::fpga_clock::now();
                auto delta = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime).count() / 1.0E6;
                std::cout << "Auto done in " << delta << " seconds" << std::endl;
            }
        }
    );

    TestPixycamDetectorCommand* testPixycamDetector = new TestPixycamDetectorCommand(m_Pixy);
    TestPixycamPositionCommand* testPixycamPosition = new TestPixycamPositionCommand(m_Pixy);

    m_DashboardAutoChooser.SetDefaultOption("3 cell auto", threeCellAutoCommand);
    m_DashboardAutoChooser.AddOption("6 cell auto", sixCellAutoCommand);
    m_DashboardAutoChooser.AddOption("8 cell auto", eightCellAutoCommand);
    m_DashboardAutoChooser.AddOption("close auto", closeShotAutoCommand);
    m_DashboardAutoChooser.AddOption("follow path - barrel racing", followPathBR);
    m_DashboardAutoChooser.AddOption("follow path - slalom", followPathS);
    m_DashboardAutoChooser.AddOption("follow path - bounce", followPathBounce);
    m_DashboardAutoChooser.AddOption("follow path - test", followPathTest);

    m_DashboardAutoChooser.AddOption("pickup cells : challenge", pickupCellsChallenge);
    m_DashboardAutoChooser.AddOption("test pixycam detector", testPixycamDetector);
    m_DashboardAutoChooser.AddOption("test pixycam position", testPixycamPosition);
}

void RobotContainer::ReportSelectedAuto () {
    std::string name;

    if (m_DashboardAutoChooser.HasSelected()) {
        name = m_DashboardAutoChooser.GetSelectedName();
    } else {
        name = m_DashboardAutoChooser.GetDefaultName();
    }

    frc::SmartDashboard::PutString("Robot sees autonomous", name);
}
