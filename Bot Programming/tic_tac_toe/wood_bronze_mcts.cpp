#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cmath>
#include <utility>


class Action {
public:
  int board, row, col, val;

  Action() {
    board = -1;
  }
  Action(int b, int r, int c, int v) {
    board = b;
    row = r;
    col = c;
    val = v;
  }
};

class Board {
  std::vector<std::vector<int>> board;
  int ply = 0;
  int status = -1;
  bool is_available_actions_calculated = false;
  std::vector<Action> available_actions;

public:
  Board() {
    board = std::vector<std::vector<int>> (3, std::vector<int>(3, 0));
  }

  std::vector<std::vector<int>> get_board() {
    return board;
  }

  // 0 - running, 1 - player 1 win, 2 - player 2 win, 3 - draw
  int get_status() {
    if (status != -1) return status;
    for (int i = 0; i < 3; i++) {
      if (board[i][0] != 0 && board[i][0] != 3 && board[i][0] == board[i][1] && board[i][1] == board[i][2]) return status = board[i][0];
      if (board[0][i] != 0 && board[0][i] != 3 && board[0][i] == board[1][i] && board[1][i] == board[2][i]) return status = board[0][i];
    }
    if (board[0][0] != 0 && board[0][0] != 3 && board[0][0] == board[1][1] && board[1][1] == board[2][2]) return status = board[0][0];
    if (board[0][2] != 0 && board[0][2] != 3 && board[0][2] == board[1][1] && board[1][1] == board[2][0]) return status = board[0][2];

    if (ply == 9) return status = 3;
    return status = 0;
  }

  void do_action(const Action &action) {
    board[action.row][action.col] = action.val;
    ply++;
    status = -1;
    is_available_actions_calculated = false;
  }

  void undo_action(const Action &action) {
    board[action.row][action.col] = 0;
    ply--;
    status = -1;
    is_available_actions_calculated = false;
  }

  std::vector<Action> get_available_actions (const int board_number, const int player) {
    // if (is_available_actions_calculated) return available_actions;
    is_available_actions_calculated = true;
    available_actions.clear();
    if (get_status() == 0) {
      for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
          if (board[i][j] == 0) {
            available_actions.push_back(Action(board_number, i, j, player));
          }
        }
      }
    }
    return available_actions;
  }
};

class Node {
public:
  std::vector<Board> state;

  int visits = 0;
  double reward = 0;
  Node *parent = nullptr;
  Action parent_to_child_action;
  std::vector<Node*> children;

  int player_to_play = 1;
  int board_to_play = 9;

  bool is_actions_calculated = false;
  std::vector<Action> available_actions;

  Node() {
    state = std::vector<Board> (10, Board());
  }

  Node *make_child(const Action &action) {
    Node *child = new Node();
    child->state = state;
    child->visits = 0;
    child->reward = 0;
    child->parent = this;
    child->parent_to_child_action = action;
    child->player_to_play = player_to_play;
    child->board_to_play = board_to_play;
    child->do_action(action);
    return child;
  }

  std::vector<Node*> make_all_children() {
    get_all_actions();
    children.clear();
    for (Action &action : available_actions) {
      children.push_back(make_child(action));
    }
    return children;
  }

  bool is_terminal() {
    return state[9].get_status() != 0;
  }

  bool is_expanded() {
    return get_all_actions().size() == children.size();
  }

  void do_action(const Action &action) {
    /*
      1) Make move on smallere board
      2) update bigger board result according to smaller board's result
      3) switch player
      4) update position of next smaller borad to play
      5) need to calculate actions again
    */

    // std::cerr << "Start \n";
    // print_state();

    state[action.board].do_action(action);
    
    int result = state[action.board].get_status();
    if (result != 0) {
      state[9].do_action(Action(9, action.board/3, action.board%3, result));
    }
    player_to_play = player_to_play == 1 ? 2 : 1;

    int next_board_number = action.row*3 + action.col;
    // std::cerr << action.board << " " << action.row << " " << action.col << ", " << next_board_number << std::endl;
    if (state[next_board_number].get_status() == 0) {
      board_to_play = next_board_number;
    } else {
      board_to_play = 9;
    }

    is_actions_calculated = false;

    // print_state();
  }

  // // TODO: complete it if required
  // void undo_action(const Action &action) {
  //   state[action.board].undo_action(action);

  // }

  std::vector<Action> get_all_actions() {
    if (is_actions_calculated) return available_actions;
    is_actions_calculated = true;
    available_actions.clear();
    if (board_to_play < 9) {
      available_actions = state[board_to_play].get_available_actions(board_to_play, player_to_play);
    } else {
      for (int b = 0; b < 9; b++) {
        if (state[b].get_status() == 0) {
          std::vector<Action> actions = state[b].get_available_actions(b, player_to_play);
          for (Action &action : actions) {
            available_actions.push_back(action);
          }
        }
      }
    }
    return available_actions;
  }

  double get_reward() {
    int result = state[9].get_status();
    if (result == player_to_play) {
      return 10.0;
    } else if (result == 3 - player_to_play) {
      return -10.0;
    } else {
      return 0.0;
    }
  }

  double calculate_ucb1(const double c) {
    if (visits == 0) return 1e18;
    // static const double epsilon = 1e-6;
    return reward / visits + c * sqrt(2.0 * log(parent->visits) / visits);
  }

  
  void print_state() {
    std::vector<std::vector<int>> v(9, std::vector<int>(9, 0));
    for (int i = 0; i < 9; i++) {
      for (int j = 0; j < 9; j++) {
        std::cerr << state[(i/3)*3 + j/3].get_board()[i%3][j%3] << " ";
      }
      std::cerr << "\n";
    }
    std::cerr << "\n";
  }
  
};

class MCTS_algo {
public:
  // main function for the Monte Carlo Tree Search
  Node* MCTS_search(Node *root, int iterations) {
    // Either some number of iteration or time
    while (iterations--) {
      Node *leaf = tree_policy(root);
      double simulation_result = default_policy(leaf);
      backup_negamax(leaf, simulation_result);
    }
    return best_child(root, 0);
    // return expand(root);
    // return best_child(root);
  }

  // function for node traversal
  Node *tree_policy(Node *node) {
    while (!node->is_terminal()) {
      if (!node->is_expanded()) {
        return expand(node);
      } else {
        static const double Cp = sqrt(2);
        node = best_child(node, Cp);
      }
    }
    return node;
  }

  Node *expand(Node *node) {
    if (!node->is_expanded()) {
      node->make_all_children();
    }
    if (node->children.size() == 0) return nullptr;
    return node->children[0];
    // return node->children[rand() % node->children.size()];
  }

  // take node that has maximum value of uct function 
  Node *best_child(Node *node, double c) {
    Node *best = nullptr;
    long double best_value = -1e18;
    for (Node *child : node->children) {
      double value = child->calculate_ucb1(c);
      if (value > best_value) {
        best = child;
        best_value = value;
      }
    }
    if (best == nullptr) {
      std::cerr << "Null child\n";
    }
    return best;
  }

  // function for randomly selecting a child node
  double default_policy(Node *node) {
    // while (!node->is_terminal()) {
    //   Action action = node->get_random_action();
    //   node->do_action(action);
    // }
    // /*
    //   maybe need to undo all actions at the end,
    //   by setting node.state = starting_state (Maybe wrong)
    // */
    // return node->get_reward();

    while (!node->is_terminal()) {
      // expand and choose random child
      node = expand(node);
    }
    return node->get_reward();
  }

  // function for backup
  void backup(Node *node, double result) {
    while (node != nullptr) {
      node->visits += 1;
      node->reward += result;
      node = node->parent;
    }
  }

  // UCT backup for 2 players
  void backup_negamax(Node *node, double result) {
    while (node != nullptr) {
      node->visits += 1;
      node->reward += result;
      result = -result;
      node = node->parent;
    }
  }
};


/*
Possible improvements:
  1. Instead of expanding fully, expand only one child 
    (maintain an array of remaining expandable actions)
  2. Bit boards
  3. 

*/





int main() {
  static const int iterations = 1; 
  std::srand(static_cast<unsigned int>(time(0)));

  Node *node = new Node();
  MCTS_algo mcts;

  // Action action(7, 2, 1, 1);
  // std::cout << action.board << " " << action.row << " " << action.col << " " << action.val << "\n";
  // int row = (action.board/3)*3 + action.row, col = (action.board%3)*3 + action.col;
  // std::cout << row << " " << col << std::endl;

  // int board = (row/3)*3 + col/3;
  // Action action2(board, row%3, col%3, action.val);
  // std::cout << action2.board << " " << action2.row << " " << action2.col << " " << action2.val << "\n";

  // node->print_state();
  // for (int i = 0; i < 5; i++) {
  //   std::cerr << "start\n";
  //   // node->print_state();
  //   node->make_all_children();
  //   node = node->children[0];
  //   // node->print_state();
  //   Action action = node->parent_to_child_action;
  //   std::cerr << action.board << " " << action.row << " " << action.col << " " << action.val << "\n\n";
  //   // node->print_state();
  // }



  // int n = 5;
  while (1) {
    int oppo_row = -1, oppo_col = -1;
    std::cin >> oppo_row >> oppo_col; std::cin.ignore();
    Action oppo_action;
    oppo_action.board = (oppo_row/3)*3 + oppo_col/3;
    oppo_action.row =  oppo_row % 3;
    oppo_action.col = oppo_col % 3;

    int valid_action_count;
    std::cin >> valid_action_count; std::cin.ignore();
    std::vector<Action> valid_actions;
    for (int i = 0; i < valid_action_count; i++) {
      int row, col;
      std::cin >> row >> col; std::cin.ignore();
    }

    int next_board = node->board_to_play;
    
    if (oppo_row != -1) {
      int board = (oppo_row/3)*3 + oppo_col/3;
      // std::cerr << board << std::endl;
      Node *new_node = mcts.MCTS_search(node, iterations);
      for (Node *c : node->children) {
        Action &a = c->parent_to_child_action;
        if (a.board == oppo_action.board && a.row == oppo_action.row && a.col == oppo_action.col) {
          new_node = c;
          break;
        }
      }
      if (new_node == nullptr) {
        std::cerr << "Break 1\n";
        break;
      }
      node = new_node;
      next_board = node->board_to_play;
    }

    // node->print_state();    

    Node* new_node = mcts.MCTS_search(node, iterations);

    if (new_node == nullptr) {
      std::cerr << "Break 2\n";
      break;
    }
    node = new_node;

    // std::cerr << next_board << std::endl;

    Action action = new_node->parent_to_child_action;
    int row = (action.board/3)*3 + action.row, col = (action.board%3)*3 + action.col;

    std::cerr << action.board << " " << action.row << " " << action.col << " " << action.val << std::endl;
    std::cout << row << " " << col << std::endl;
  }
  return 0;
}





