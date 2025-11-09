#pragma once

class ISolution;

class IMutation {
public:
    virtual ~IMutation() = default;
    virtual void apply(ISolution& solution) = 0;
};

