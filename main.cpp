/**
  * Simple hex board game, no swap(pie) supported, Monte Carlo algorithm
  * implemented for computer only.
  * Can be either two player (Human) or Single player with computer.
  * (Player One == Human ALWAYS), (Player Two == Human || Computer)
  *
  * @note must have C++14 enabled for build due to dependencies and threading
  * flag set for g++.
  */

// C++ headers
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <string>
#include <utility> // required for pair object.
#include <memory> // required for unique_ptr
#include <thread> // required for theading - sits on pthread/posix

// C header files
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

// local headers
#include "hex_game.h"

/**
 * @details on play:
 *
 * - User is prompted for board size, limit is 11, minimum is 3.
 * - User must select either human input or computer input to be used for player 2.
 * - User must then input row, then column coordiantes.
 * - "Invalid Move" is printed if move not valid and player must re-enter.
 */
int main(void) {

    int game_size = cn_DEFAULT_GAME_SIZE;
    bool computer = false;
    bool computer_one = false;
    std::string player_select;

    srand(time(static_cast<time_t>(0))); // feed seed for random number generator (used when computer playing)

    CLEAR_SCREEN();
    std::cout << "Hex Game : S. Whittaker (2018)" << std::endl;

    // initiliase gameplay - get game size and player selection (human or computer)
    std::cout << "Board size: " << std::endl;
    std::cin >> game_size;
    std::cout << "Second Player : Computer or Human? (Enter C or H)" << std::endl;
    std::cin >> player_select;

    if (std::string(1, std::tolower(player_select[0], std::locale())).compare("c") == 0) {

        computer = true;
    } else if (std::string(1, std::tolower(player_select[0], std::locale())).compare("b") == 0) {

        // set first and second player as computer.
        // @note computer generated moves for player one are currently not implemented.
        // @todo implement
        computer_one = true;
        computer = true;
    }

    int row_idx, col_idx; // used for player input

    HexGame hex_game(((game_size <= cn_MAX_GAME_SIZE) && (game_size >= cn_MIN_GAME_SIZE) ? game_size : cn_DEFAULT_GAME_SIZE));

    CLEAR_SCREEN();

    Player player = Player::FIRST; // default (no swap)

    while (true) {

        hex_game.display();

        std::cout << ((player == Player::FIRST) ? "First Player (G) Move" : "Second Player (R) Move") << std::endl;

        if ((player == Player::SECOND) && (computer == true)) {
            // Run Monte Carlo algorithm for computer player
            hex_game.computerPlay();
        } else {
            // get user input for first player ALWAYS, second player only if not computer
            std::string input;
            do {
                std::getline(std::cin, input);
            } while (input.empty() == true); // catch empty triggers (terminal in linux triggers empty captures.)

            auto delim_index = input.find_first_of(',');
            auto row_sub = input.substr(0, delim_index);
            auto col_sub = input.substr((delim_index + 1),
                                    std::distance(input.begin() + delim_index, input.end()));
            std::stringstream(row_sub) >> row_idx;
            std::stringstream(col_sub) >> col_idx;

        }

        // polls for computer move first, then polls for human move returns false if move invalid
        if (((player == Player::SECOND) && computer == true) || hex_game.playInterface(player, row_idx, col_idx)) {

            hex_game.display(); // draw graph

            if (hex_game.checkWin(player) == true)
                break;

            (player == Player::FIRST) ? (player = Player::SECOND) : (player = Player::FIRST);
        }
        else {

            std::cout << "Invalid move!" << std::endl;
            SLEEP();
        }
    }

    // Print winner
    std::cout << "Player " << ((player == Player::FIRST) ? "One" : "Two" ) << " has won!" << std::endl;

    return 0;
}

/**********************************************end of file**********************************************/