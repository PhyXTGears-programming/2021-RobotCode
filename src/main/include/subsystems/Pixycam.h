#pragma once

#include <cpptoml.h>

#include <frc/SPI.h>

#include <frc2/command/SubsystemBase.h>

#include "Constants.h"

#include "pixy/PixyBlock.h"

class Pixycam : public frc2::SubsystemBase {
    public:
        Pixycam();
        void Periodic() override;

        std::vector<PixyBlock> GetBlocks();

    private:
        frc::SPI m_Spi{kPixycamSpi};

        bool WaitForSync();
};
