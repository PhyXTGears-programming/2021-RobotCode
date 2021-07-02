#include "commands/ReverseBrushesCommand.h"

ReverseBrushesCommand::ReverseBrushesCommand (Intake* intake) {
    AddRequirements(intake);
    m_Intake = intake;
}

void ReverseBrushesCommand::Initialize () {}

void ReverseBrushesCommand::Execute () {
    m_Intake->ConveyorReverse();
    m_Intake->FeedReverse();
}

void ReverseBrushesCommand::End (bool interrupted) {
    m_Intake->ConveyorStop();
    m_Intake->FeedStop();
}

bool ReverseBrushesCommand::IsFinished() {
    return false;
}
