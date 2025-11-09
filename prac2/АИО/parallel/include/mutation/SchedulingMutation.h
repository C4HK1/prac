#pragma once

#include "interfaces/IMutation.h"

class SchedulingMutation final : public IMutation {
public:
    void apply(ISolution& solution) override;

private:
    void relocateJob(class SchedulingSolution& solution);
    void swapRandomJobs(class SchedulingSolution& solution);
};

