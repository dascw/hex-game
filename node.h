/**
 * @name node.h 
 * @brief required for Monte Carlo implementation.
 */
#ifndef NODE_H
#define NODE_H

#include <vector>
#include "position.h"

// aliases
using diffCoordinate = int;
using MapSize = uint8_t;
using Connections = std::vector<Position>;

/// @brief class : NodeColour enumeration : defines state of node, WHITE == init
enum class NodeColour : uint8_t { WHITE, RED, GREEN };

/**
  * @brief class node : contains position element.
  * @details generates connectivity based on size of board (not stored).
  *
  * @note Connection Algorithm:
  *     A node is connected if its dRow or dColumn value == 1, i.e. node 0,3 is not connected to 2,1,
  *     because the dRow == 2 and dColumn == 2, however node 0,3 is connected to 1, 0, because
  *     dRow == 1.
  *
  *     if ((dR == -1 && dR == -1)) || ((dR == 1) && (dR == 1))
  *         not connected.
  *     else if (dR == 1 && dC == 0) || (dC == 0 and dC == 1)
  *         connected.
  *     else
  *         not connected (default).
 */
class Node : public Position {
public:
    Node(const Coordinate& row, const Coordinate& col, const MapSize& board) :
        Position(row, col),
        m_colour(NodeColour::WHITE) {

        /// @brief self construct algorithm : based on node position and board size.
        for (Coordinate row_idx = 0; row_idx < board; ++row_idx) {

            for (Coordinate col_idx = 0; col_idx < board; ++col_idx) {

                diffCoordinate dRow = (row_idx - m_row);
                diffCoordinate dCol = (col_idx - m_col);

                // Apply rule for all connected nodes.
                if (((dRow == -1) && (dCol == -1)) || ((dRow == 1) && (dCol == 1))) {
                    // not connected.
                }
                else if ((abs(dRow) < 2) && (abs(dCol) < 2)) {
                    // conected
                    if (!((row_idx == m_row) && (col_idx == m_col))) { // block our own position
                        m_connections.push_back(Position(row_idx, col_idx));
                    }
                }
            }
        }
    }

    // Empty constructor (required for graph init functionality - resize)
    Node() = default;

    /// @brief getConnections : returns reference to connected nodes
    /// @return Connections&
    Connections& getConnections() { return m_connections; }

    /// @brief getColour : returns current colour of node as reference (const)
    /// @brief const NodeColour&
    const NodeColour& getColour() const { return m_colour; }

    /// @brief setColour : sets current colour of node
    /// @param const NodeColour
    void setColour(const NodeColour& colour) { this->m_colour = colour; }

    /// @brief setTraverse : sets traverse semiphore
    /// @param bool
    void setTraverse(const bool& value) { this->m_traversed = value; }

    /// @brief getTraversed : returns traverse semiphore
    /// @return true / false
    bool getTraversed(void) { return (this->m_traversed); }

    ~Node() = default;
private:
    NodeColour  m_colour;
    bool        m_traversed;
    Connections m_connections;
};

#endif 
    // NODE_H

/****************************************end of file****************************************/