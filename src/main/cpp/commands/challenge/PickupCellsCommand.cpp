#include <iostream>

#include <frc2/command/Command.h>
#include <frc2/command/InstantCommand.h>
#include <frc2/command/SequentialCommandGroup.h>

#include "PickupCellsChallenge.h"

#include "commands/challenge/PickupCellsCommand.h"

using challenge::Layout;

static Layout find_best(int ared, int ablue, int bred, int bblue);
static frc2::Command* build_pickup_command(
    Drivetrain* drivetrain,
    Intake* intake,
    const FollowPolybezier::Configuration &followerConfig,
    wpi::Twine path
);

PickupCellsCommand::PickupCellsCommand (
    Drivetrain* drivetrain,
    Intake* intake,
    Pixycam* pixy,
    const FollowPolybezier::Configuration &followerConfig
) {
    AddRequirements(pixy);

    m_Pixy = pixy;

    // This command does not require these subsystems, but subsequent commands
    // scheduled by this command will need these.

    m_Drivetrain = drivetrain;
    m_Intake = intake;

    m_FollowARed = build_pickup_command(
        m_Drivetrain,
        m_Intake,
        followerConfig,
        "/home/lvuser/deploy/paths/pickup-a-red.json"
    );

    m_FollowABlue = build_pickup_command(
        m_Drivetrain,
        m_Intake,
        followerConfig,
        "/home/lvuser/deploy/paths/pickup-a-blue.json"
    );

    m_FollowBRed = build_pickup_command(
        m_Drivetrain,
        m_Intake,
        followerConfig,
        "/home/lvuser/deploy/paths/pickup-b-red.json"
    );

    m_FollowBBlue = build_pickup_command(
        m_Drivetrain,
        m_Intake,
        followerConfig,
        "/home/lvuser/deploy/paths/pickup-b-blue.json"
    );
}

void PickupCellsCommand::Initialize () {
    m_DetectorARed.Reset();
    m_DetectorABlue.Reset();
    m_DetectorBRed.Reset();
    m_DetectorBBlue.Reset();

    m_ScoreARed = 0;
    m_ScoreABlue = 0;
    m_ScoreBRed = 0;
    m_ScoreBBlue = 0;
}

void PickupCellsCommand::Execute () {
    std::vector<PixyBlock> blockList = m_Pixy->GetBlocks();

    m_DetectorARed.ProcessBlocks(blockList);
    m_DetectorABlue.ProcessBlocks(blockList);
    m_DetectorBRed.ProcessBlocks(blockList);
    m_DetectorBBlue.ProcessBlocks(blockList);

    Layout bestLayout = find_best(
        m_DetectorARed.GetError(),
        m_DetectorABlue.GetError(),
        m_DetectorBRed.GetError(),
        m_DetectorBBlue.GetError()
    );

    switch (bestLayout) {
        case Layout::A_RED:
            m_ScoreARed += 1;
            break;

        case Layout::A_BLUE:
            m_ScoreABlue += 1;
            break;

        case Layout::B_RED:
            m_ScoreBRed += 1;
            break;

        case Layout::B_BLUE:
            m_ScoreBBlue += 1;
            break;

        default:
            break;
    }
}

void PickupCellsCommand::End (bool interrupted) {
    if (interrupted) {
        return;
    }

    if (WIN_SCORE <= m_ScoreARed) {
        std::cout << "auto: Pickup path A Red" << std::endl;
        m_FollowARed->Schedule();
    } else if (WIN_SCORE <= m_ScoreABlue) {
        std::cout << "auto: Pickup path A Blue" << std::endl;
        m_FollowABlue->Schedule();
    } else if (WIN_SCORE <= m_ScoreBRed) {
        std::cout << "auto: Pickup path B Red" << std::endl;
        m_FollowBRed->Schedule();
    } else if (WIN_SCORE <= m_ScoreBBlue) {
        std::cout << "auto: Pickup path B Blue" << std::endl;
        m_FollowBBlue->Schedule();
    }
}

bool PickupCellsCommand::IsFinished () {
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

static frc2::Command* build_pickup_command(
    Drivetrain* drivetrain,
    Intake* intake,
    const FollowPolybezier::Configuration &followerConfig,
    wpi::Twine path
) {
    FollowPolybezier follower { drivetrain, path, followerConfig };
    auto start = follower.GetStartPoint();

    return new frc2::SequentialCommandGroup(
        frc2::InstantCommand(
            [intake, drivetrain, start] {
                intake->IntakeExtend();
                intake->IntakeStart();
                drivetrain->SetPose(start.x, start.y, 0);
            },
            { intake }
        ),
        std::move(follower),
        frc2::InstantCommand(
            [intake] {
                intake->IntakeStop();
                intake->IntakeRetract();
            },
            { intake }
        )
    );
}
