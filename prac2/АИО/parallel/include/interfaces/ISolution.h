#pragma once

#include <memory>

class ISolution {
public:
    virtual ~ISolution() = default;

    virtual double cost() const = 0;

    virtual std::unique_ptr<ISolution> clone() const = 0;
    virtual std::unique_ptr<ISolution> cloneWithNewSeed(unsigned int seed) const = 0;
};

