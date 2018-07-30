// hex_game.h 
#ifndef HEX_GAME_H
#define HEX_GAME_H
/// @brief class : Player enumeration : provides simple interface for player API
enum class Player : uint8_t { FIRST, SECOND };

#include <memory>
#include <vector>
#include <algorithm>
#include <thread>
#include <string>
#include <utility>

#include "graph.h"
#include "probability.h"

/// @brief clear screen macro
/// @note this should be compatible for both deployments, however I haven't
/// tested on a windows distribution.
#ifdef WINDOWS
    #define CLEAR_SCREEN() system("CLS")
#else
    #define CLEAR_SCREEN() std::system("clear")
#endif
    // TERMINAL CLEAR

/// @brief half second sleep macro for multi platform deployment
#ifdef WINDOWS
    #include <windows.h> // @note unsure if this include is valid for sleep.
    #define SLEEP() Sleep(1000)
#else
    #include <unistd.h>
    #define SLEEP() usleep(1000000)
#endif
    // SLEEP

// Local constants for game constructor
static const uint8_t cn_MAX_GAME_SIZE       = 11;
static const uint8_t cn_MIN_GAME_SIZE       = 3;
static const uint8_t cn_DEFAULT_GAME_SIZE   = 7;
static const uint8_t cn_NUM_OF_THREADS      = 10;

/**
 * @brief The HexGame class: inherits Graph class.
 * @details extens Graph class with game functionality.
 * @note not additional memeber variable parameters are required.
 */
class HexGame final : public Graph {
public:
    /// @brief HexGame : constructor
    HexGame(const BoardSize& size) :
        Graph((static_cast<MapSize>(size))),
        m_limits(static_cast<Coordinate>(size - 1), static_cast<Coordinate>(size - 1)) {

        // Set play trackers (required for monte carlo alogirithm)
        m_play_total = 0;
        m_play_maximum = (size * size);
    }

    /// @brief Clone method for copying derived class
    /// @details provides uinque_ptr<> object for indirection
    /// use - required for duplicating class with base inheritence. 
    std::unique_ptr<HexGame> clone() const {
        return std::make_unique<HexGame>(*this);
    }

    /// @brief playInterface : used to inteface for OOR move blocking and addplay check.
    /// @param player, row, col
    /// @return true
    bool playInterface(const Player& player, const Coordinate& row, const Coordinate& col) {
        return (checkInputRange(row, col) == true) ?
                    this->addPlay(player, row, col) : (false);
    }

    /// @brief computerPlay : returns computer move
    /// @details generates move based on highest probability of win using
    /// Monte Carlo algorithm.
    void computerPlay() {
        // @note in order to simplifiy the playing method during Monte Carlo
        // prediction we create a local copy of the base class Graph's initial
        // state. We then operate on the HexGame object as is and return
        // the graph to its initial state prior to running the MC algorithm.
        // This makes manipulation of the tree simplistic and intuitive, and
        // introduces minimal overhead (we only copy the tree back after we've 
        // decided on the best move and we can utilise the HexGame object's 
        // methods natively while threading.)
        // This also provides mutual exclusion between the objects being manipulated
        // by each thread.

        std::unique_ptr<Graph> clone = this->clone();
        Graph temp_graph(*clone); // copy inherited object complete, store object for reinit later.
        PlayCount total_temp = this->m_play_total;

        std::vector<Probability> outputs; // vector storing all probability values

        // Attempt to play every free position on the board.
        for (Coordinate row_idx = 0; row_idx < this->getSize(); ++row_idx) {

            for (Coordinate col_idx = 0; col_idx < this->getSize(); ++col_idx) {

                if (this->isNodeFree(row_idx, col_idx)) { // Now traverse graph checking for free nodes.
                    // test play on this position.
                    outputs.push_back(testPlay_threaded(row_idx, col_idx));
                    this->m_tree = temp_graph.getTree();    // reset tree to initial state.
                    this->m_play_total = total_temp;        // reset play counter.
                }
            }
        } // finish : all possible moves have been played and their probability of win stored.

        std::sort(outputs.begin(), outputs.end(), compareProbability());

        // Add move with highest probability of winning.
        this->addPlay(Player::SECOND, outputs[0].getRow(), outputs[0].getCol());
    }

    /// @brief threadWrapper_testPlay : wrapper function for parsing result of probability
    /// calculation through indirection.
    /// @details reduces complexity of testPlay() method call.
    void threadWrapper_testPlay(Probability * result, const Coordinate& row_idx, const Coordinate& col_idx) {

        *result = testPlay(row_idx, col_idx);
    }

    /// @brief testPlay_Threaded : generates threads of testPlay() method calls
    /// @details sums resulting probability and generates probability object
    /// with average. 
    /// @param row_idx, col_idx
    /// @return Probability object (averaged) 
    ///
    /// @note each probability request is generated in a separate thread. After
    /// all threads requests are generated, all threads are joined to main 
    /// thread and resynchronised. After completing of synchronisation, 
    /// thread results are averaged and returned to calling function.
    Probability testPlay_threaded(const Coordinate& row_idx, const Coordinate& col_idx) {

        // @note this could do with a move constructor.
        std::unique_ptr<HexGame> temp = this->clone(); // returns pointer to current HexGame object.
        std::vector<HexGame> threadObjects; // stores each HexGame object for iterating over result
        std::vector<Probability> results(cn_NUM_OF_THREADS);
        std::vector<std::thread> threads;

        threadObjects.reserve(cn_NUM_OF_THREADS);
        // Create MULTIPLE instances of HexGame for use in threads
        for (int idx = 0; idx < cn_NUM_OF_THREADS; ++idx) {
            
            threadObjects.push_back(*temp); // generates copy of current hexGame through indirection.
        }

        // @note required to track results index (using a reference/iterator object threw an error in
        // the thread library)
        unsigned int results_idx = 0;

        threadObjects.reserve(cn_NUM_OF_THREADS);
        // for all objects, instantiate thread
        for (auto& a : threadObjects) {

            threads.push_back(std::thread(&HexGame::threadWrapper_testPlay, a, &results[results_idx++], row_idx, col_idx));
        }

        // Await thread completion
        for (auto& a : threads)
            a.join();

        // Average results from threading calculations
        return (averageResults(results));
    }

    /// @brief test play for given row/col on graph
    /// @details generates probability based on number of wins / losses 
    /// for a move at the given coordinates.
    /// @note force player two for now
    Probability testPlay(const Coordinate& row_idx, const Coordinate& col_idx) {

        static const PlayCount cn_MAXIMUM_PLAY_LIMIT = 150; // limit for number of plays / thread

        // for the requested coordinates, place first object in graph
        this->addPlay(Player::SECOND, row_idx, col_idx);

        // Create temporary graph object for reassigning values after each play attempt
        Graph temp(*(this->clone()).release()); // copy inherited object complete, store object for recall later.

        Player player = Player::SECOND; // Second player always computer (presumed, no pie rule atm).

        // Counters for generating probability
        PlayCount count = 0;
        PlayCount wins  = 0;
        PlayCount total_temp = this->m_play_total;

        // limit = MAX for small boards, reduced for large boards to minimise calculation delay
        PlayCount limit = (this->getSize() > 5) ?
                    (cn_MAXIMUM_PLAY_LIMIT - 10 * (this->getSize() - 6)) :
                    cn_MAXIMUM_PLAY_LIMIT;

        // @note play count limit should be relative to size of board to reduce CPU overhead / delays
        // when playing on large boards.
        while (count++ < limit) {

            // Generate random numbers check validity, if invalid, generate again until valid
            // do this for ALL free nodes.
            // Populates entire board at random.
            for (PlayCount idx = this->m_play_total; idx < this->m_play_maximum; ) {
           
                // if play was valid, increment counter and play next player's move.
                if (addPlay(player, (rand() % this->m_size), (rand() % this->m_size)) == true) {
                    
                    idx++;
                    (player == Player::FIRST) ? (player = Player::SECOND) : (player = Player::FIRST);
                } 
            } 

            // After board is populated in full, check for win condition on second player,
            // due to the logical rules of hex, Player::FIRST must have won if
            // Player::SECOND has not.
            if (checkWin(Player::SECOND))
                wins++;

            // reset graph for next play round
            player = Player::SECOND; // reset player state.
            this->m_tree = temp.getTree();
            this->m_play_total = total_temp; // reset play tracker.
        }

        // Return number of wins for given coordinates.
        return Probability(static_cast<float>(wins) / (count), row_idx, col_idx);
    }

    /// @brief averageResults() : returns average result from probability threading
    /// @return Probability object
    Probability averageResults(std::vector<Probability>& in) {

        ProbabilityValue total = 0;
        for (auto a : in)
            total += a.getProb();

        return Probability((total / in.size()), in[0].getRow(), in[0].getCol());
    }


    /// @brief checkWin : check if player has won after valid play entered.
    /// @param player
    /// @return true for win.
    bool checkWin(const Player& player) {

        NodeColour colour = convertPlayer(player);
        bool ret = false;
        Coordinate idx;
        CoordinatePtr row_idx, col_idx;; // pointers for indexing @todo is this the best method for handling this?
        Coordinate zero_idx = 0;

        if (player == Player::FIRST) {
            row_idx = &idx;         // row increment check
            col_idx = &zero_idx;
        } else {
            row_idx = &zero_idx;    // column increment check
            col_idx = &idx;
        }

        // Attempt to traverse from top to bottom of graph or right to left (depending on player)
        // If can't traverse first step, break out, else call findPath method to look for next step in tree.
        for (idx = 0; idx < this->m_size; ++idx) {

            if ((this->isNodeColour(colour, *row_idx, *col_idx) == true) && (this->getTraverse(*row_idx, *col_idx) == false)) {
                // set traverse block for parent node.
                this->setTraverse(*row_idx, *col_idx);
                // get all connected nodes and continue.
                ret = this->findPath(colour, this->getConnections(*row_idx, *col_idx));
                if (ret == true)
                    break; // game won
            } else {

                continue; // check next node.
            }
        }

        this->clearAllTraverse(); // set init state for all nodes
        return ret;
    }


    /// @brief display : draws hex game object out to console window.
    /// @details is malleable for size based manipulation depending on
    /// board size.
    /// @note uses special character look up of '%', replaces character
    /// with 'R' or 'G' if set or '.' if unset.
    void display() {

        CLEAR_SCREEN(); // reinit display

        /// @brief buildLine : string manipulation function
        auto buildLine = [](std::string &st, std::string append, int size)->void {

            for (int idx = 0; idx < size; ++idx) {
                st.append(append);
            }
        };

        std::string space; // scope required

        std::string topBottom(" "); // generate border
        buildLine(topBottom, "R ", (this->m_size + 1) * 2);

        /// @details buildColNum : function for building column 
        /// number std::string.
        auto buildColNum = [](std::string top, int limit)->std::string {

            std::string ret;
            int col_mod = 0;
            int row_print_val = 0;

            for (auto a : top) {

                if (std::string(1, a).compare(" ") == 0) {

                    ret.append(std::string(1, a));
                } else {
                    // must be R, only print every second R found
                    if (col_mod++ == 1) {
                        col_mod = 0;

                        if (row_print_val != limit) {

                            ret.append(std::to_string(row_print_val++));
                        }
                    } else {
                        ret.append(" ");
                    }
                }
            }
            return ret;
        };

        std::string columnNumbering = buildColNum(topBottom, this->getSize());
        std::cout << columnNumbering << std::endl;
        std::cout << topBottom << std::endl;

        std::string first(" G ");
        buildLine(first, " % _", (this->m_size - 1));
        first.append(" \%  G"); // final

        std::string second(" G ");
        buildLine(second, " \\ /", (this->m_size - 1));
        second.append(" \\  G");

        // control number of spaces from left border of console window (starts with 1 space)
        int space_idx = 1; 

        for (Coordinate row_idx = 0; row_idx < this->m_size; ++row_idx) {
            // For each instance of the key %, find and replace with . or color
            // representation if set.
            Coordinate col_idx = 0;

            std::cout << static_cast<int>(row_idx); // print row number
            
            for (auto a : first) {

                if (a == '%') {
                    // process special character
                    NodeColour state = this->drawNode(row_idx, col_idx++);
                    if (state == NodeColour::WHITE) {

                        std::cout << '.';
                    } else if (state == NodeColour::GREEN) {

                        std::cout << 'G';
                    } else {

                        std::cout << 'R';
                    }
                } else {

                    std::cout << a;
                }
            }
            std::cout << ' ' << static_cast<int>(row_idx);
            std::cout << std::endl;

            // Buffer screen
            space = std::string(space_idx++, ' ');
            std::cout << space << ' '; // extra space to account for row numbering

            // avoid printing separator row on last instance
            if ((row_idx + 1) != this->m_size) {

                std::cout << second << std::endl;
                space = std::string(space_idx++, ' ');
                std::cout << space;
            }
        }

        std::cout << topBottom << std::endl; // extra space required for indexing

        // print column numbering
        std::cout << " " << space << columnNumbering << std::endl;
    }

    ~HexGame() { /* destructor */ }
private:
    Position  m_limits;

    int m_play_total;
    int m_play_maximum;

    /// @brief  convertPlayer : returns colour representation of player.
    /// @details allows interface to be player based rather than colour based.
    NodeColour convertPlayer(const Player& player) const {
        return (player == Player::FIRST) ? NodeColour::GREEN : NodeColour::RED;
    }

    /// @brief drawNode : updates node based on colour result
    NodeColour drawNode(const Coordinate& row, const Coordinate& col) {
        return (this->getNode(row, col).getColour());
    }

    /// @brief addPlay : add play based on coordinates if not already played.
    /// @param player, row, col
    /// @return true for play added.
    bool addPlay(const Player& player, const Coordinate& row, const Coordinate& col) {
        return (this->isNodeFree(row, col) == true) ?
                    (this->setNode(this->convertPlayer(player), row, col), this->m_play_total++, true) : (false);
    }

    /// @brief checkInputRange : returns true if valid input range
    bool checkInputRange(const Coordinate& row, const Coordinate& col) {
        return ((checkRangeSingle(row) && checkRangeSingle(col)));
    }

    /// @brief checkRangeSingle : returns true if input less than size
    bool checkRangeSingle(const Coordinate& input) const {
        return (input < this->m_size);
    }

    /// @brief clearAllTraverse : clears all traverse blocks in node to allow
    /// free iteration each time.
    void clearAllTraverse() {

        for (Coordinate row_idx = 0; row_idx < this->m_size; ++row_idx) {
            for (Coordinate col_idx = 0; col_idx < this->m_size; ++col_idx) {
                this->clearTraverse(row_idx, col_idx);
            }
        }
    }

    /// @brief findPath : breadth first recursive traverse algorithm
    /// @param player - colour to find.
    /// @param net - connection net.
    /// @return true when destination found.
    bool findPath(const NodeColour& colour, const Connections& net) {

        bool ret = false;

        for (auto& node : net) {
            auto& temp = this->getNode(node.getRow(), node.getCol()); // move to new node

            /// @brief build compare function
            /// @note used to reduce condition levels below.
            auto compare = [&]()->std::pair<int, int> {
                return ((colour == NodeColour::GREEN) ? // green we want column index, red we want row index.
                            std::pair<int,int>(temp.getCol(), this->m_limits.getCol()) :
                            std::pair<int,int>(temp.getRow(), this->m_limits.getRow()));
            };

            auto result = compare();

            // if new node == border node and its colour is set, game is won.
            if ((result.first == result.second) && (temp.getColour() == colour)) {
                ret = true; // game won
                break;
            } else if ((temp.getColour() == colour) && (temp.getTraversed() == false)) {
                temp.setTraverse(true); // block
                ret |= findPath(colour, temp.getConnections()); // recursive call for findPath.
            }
        }
        return ret;
    }
};

#endif 
    // HEX_GAME_H

/********************************************end of file********************************************/
