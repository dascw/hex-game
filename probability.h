/**
 * @name probability.h 
 * @brief required for Monte Carlo implementation.
 */
#ifndef PROBABILITY_H
#define PROBABILITY_H

#include "position.h"

using ProbabilityValue = float; // floating point used for probability value (percentage of 1.0).
using PlayCount = int; // play count (strong type)

/**
 * @brief Probability class. Stores position and probability value of player winning.
 * @note multiple instances of this class shall be created, storing the play position
 * and relative probability value.
 */
class Probability final : public Position {
public:
    // Empty constructor
    Probability() = default;

    Probability(const ProbabilityValue& value, const Coordinate& row, const Coordinate& col) :
        Position(row, col),
        m_value(value) {
        /*constructor */
    }

    ~Probability() = default;

    // exported set/get functions for probability value
    void setProb(const ProbabilityValue& value)    { m_value = value; }
    ProbabilityValue  getProb(void) const { return m_value; }
private:
    ProbabilityValue m_value;
};

/// @brief compareProbability structure
/// @details used to provide iterator sort method operation; for comparing proability values.
struct compareProbability {
    inline bool operator() (Probability& first, Probability& second) {
        return (first.getProb() > second.getProb());
    }
};

#endif 
    // PROBABILITY_H
/****************************************end of file****************************************/
