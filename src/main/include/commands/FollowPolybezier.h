#pragma once

#include <string>
#include <utility>

#include <wpi/json.h>
#include <frc2/command/CommandBase.h>
#include <frc2/command/CommandHelper.h>

#include "subsystems/Drivetrain.h"
#include "bezier/bezier.h"

class FollowPolybezier : public frc2::CommandHelper<frc2::CommandBase, FollowPolybezier> {
    public:
        struct Configuration {
            double maximumRadialAcceleration;
            double maximumJerk;
            double maximumReverseAcceleration;
        };

        FollowPolybezier(Drivetrain *drivetrain, const wpi::Twine &filename, Configuration configuration, bool backwards = false);

        void Initialize();
        void Execute();

        void End (bool interrupted) {
            drivetrain->SetAcceleration(0, 0);
            drivetrain->SetAngularVelocity(0);
            drivetrain->SetBrake(true);
            logFile->close();
        }

        bool IsFinished () { return finished; };

        Point::Point GetStartPoint () { return polybezier[0].second[0].p; }
        Point::Point GetEndPoint () { return polybezier.back().second.back().p; }

    private:
        struct DistanceSample {
            Point::Point p;
            double t;
            double d;
            double maxV;
            bool minimum;
        };

        std::pair<double, double> CalculateAcceleration();

        void LoadCurve(wpi::json::value_type controlPoints);
        void AddApproximation(std::pair<Bezier::CubicBezier, std::vector<DistanceSample>> *bezier, double previousCurveFinalSpeed = 0);
        void SetNextMin();

        void ResetCurveProgress();

        Drivetrain *drivetrain;
        Configuration config;
        bool backwards;

        bool finished;

        std::vector<std::pair<Bezier::CubicBezier, std::vector<DistanceSample>>> polybezier {};
        unsigned int currentBezier;
        unsigned int prevVertex;
        std::pair<unsigned int, unsigned int> nextMin;

        double distanceTraveled; // since beginning of curve
        frc::Pose2d lastPose;

        uint64_t lastTime;
        double velocity;
        double acceleration;

        std::ofstream *logFile;
};
