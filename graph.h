/**
 * @name graph.h 
 * @brief provides graph tree for hex game.
 */
#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <memory>
#include <assert.h> // @note replaceable with c++ assert?
#include <stdint.h>

#include "node.h"



using BoardSize = uint8_t;

/**
 * @brief class Graph : contains map of nodes based on size.
 * @note each node contains its connections.
 *
 * @details provides basic interface for interacting with group
 * of connected nodes based on traversing algorithm, colour and position.
 */
class Graph {
public:    
    /// @brief Graph : constructor
    /// @param MapSize - size of board parsed (relates to number of nodes)
    Graph(const MapSize& size) :
        m_size(size) {
        this->m_tree.resize(m_size, std::vector<Node>(m_size));
        for (Coordinate row_idx = 0; row_idx < m_size; ++row_idx) {

            for (Coordinate col_idx = 0; col_idx < m_size; ++col_idx) {
                Node temp(row_idx, col_idx, m_size); // build map from board size
                this->m_tree[row_idx][col_idx] = temp;
            }
        }
    }

    /// @brief Copy constructor
    Graph(const Graph& in) :
        m_size(in.m_size),
        m_tree(in.m_tree) {
    }

    Graph() = delete;

    /// @brief Clone method for copying derived class
    std::unique_ptr<Graph> clone() const {
        return std::make_unique<Graph>(*this);
    }

    /// @brief getNode : exposes node through lookup in graph tree.
    /// @param row, col
    /// @return Node&
    Node& getNode(const Coordinate& row, const Coordinate& col) {
        assert((row < this->m_size) && (col < this->m_size)); // use this API for getting node,
        // reduces number of asserts required for accesing protected members.
        return m_tree[row][col];
    }


    /// @brief isNodeFree : returns true if node is unoccupied (wraps nodeColour function).
    /// @param row, col
    /// @return true / false
    bool isNodeFree(const Coordinate& row, const Coordinate& col) { return (isNodeColour(NodeColour::WHITE, row, col)); }

    /// @brief isNodeColour : returns true if node matches colour parameter.
    /// @param colour, row, col
    /// @return true / false
    bool isNodeColour(const NodeColour& colour, const Coordinate& row, const Coordinate& col) {
        return (this->getNode(row,col).getColour() == colour);
    }

    /// @brief setNode : sets node colour based on coordinates.
    /// @param colour, row, col
    void setNode(const NodeColour& colour, const Coordinate& row, const Coordinate& col) {
        this->getNode(row,col).setColour(colour);
    }

    /// @brief setTraverse : sets traverse member in node to block recursive loop
    /// @param row, col
    void setTraverse(const Coordinate& row, const Coordinate& col) {
        this->getNode(row,col).setTraverse(true);
    }

    /// @brief clearTraverse : clears traverse member in node to unblock recursive loop
    /// @param row, col
    void clearTraverse(const Coordinate& row, const Coordinate& col) {
        this->getNode(row,col).setTraverse(false);
    }

    /// @brief getTraverse : rerturns traverse member state
    /// @param row, col
    /// @return true / false
    bool getTraverse(const Coordinate& row, const Coordinate& col) {
        return (this->getNode(row,col).getTraversed());
    }

    /// @brief getConnections : returns connection reference for given node
    /// @param row, col
    /// @return std::vector<Position> &
    Connections& getConnections(const Coordinate& row, const Coordinate& col) {
        return this->getNode(row,col).getConnections();
    }

    /// @brief getSize : returns size of graph
    /// @return MapSize
    MapSize getSize() const {
        return (this->m_size);
    }

    /// @brief getTree : returns reference to tree object
    /// @details only required for pseudo copy constructors.
    /// @return std::vector< std::vector<Node> >&
    std::vector<std::vector<Node>>& getTree() {
        return m_tree;
    }

    ~Graph() { /* deconstructor */ }
protected:
    MapSize m_size;
    std::vector<std::vector<Node>> m_tree;
};

#endif 
    // GRAPH_H

/************************************end of file************************************/