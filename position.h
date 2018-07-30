/**
 * @name position.h 
 * @brief required for Monte Carlo implementation.
 */
#ifndef POSITION_H
#define POSITION_H

#include <stdint.h>

/// @brief typedef : coordinate for strong typing.
using Coordinate = uint8_t; // @todo these should be shifted to a pair
using CoordinatePtr = Coordinate*;

/**
 * @brief class Position : contains coordinates of point in space
 */
class Position {
public:
    Position(const Coordinate& row, const Coordinate& col) :
        m_row(row),
        m_col(col) {
    }
    Position() : Position(0,0) { }

    /// @brief getRow : returns row member
    /// @return Coordinate
    Coordinate getRow() const { return m_row; }

    /// @brief getCol : returns col member
    /// @return Coordinate
    Coordinate getCol() const { return m_col; }

    ~Position() = default;
protected:
    // poisition internals
    Coordinate m_row;
    Coordinate m_col;
};

#endif 
    // POSITION_H

/**********************************end of file**********************************/