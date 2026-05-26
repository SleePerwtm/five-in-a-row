#include "game.h"

Game::Game(int board_size, QObject* parent)
    : QObject(parent), board_(board_size, board_size) {}

bool Game::placePiece(int row, int col) {
  if (game_over_)
    return false;

  // Board 坐标是 (x, y) = (col, row)
  if (!board_.placeAt(col, row, current_player_))
    return false;

  move_history_.push(QPoint(row, col));

  emit piecePlaced(row, col, current_player_);

  if (board_.hasFiveAt(col, row, current_player_)) {
    game_over_ = true;
    emit gameOver(current_player_);
    return true;
  }

  switchPlayer();
  return true;
}

bool Game::undo() {
  if (move_history_.empty())
    return false;

  QPoint last = move_history_.top();
  move_history_.pop();
  int row = last.x();
  int col = last.y();

  board_.forceClear(col, row);

  game_over_ = false;

  emit pieceRemoved(row, col);

  switchPlayer();
  return true;
}

void Game::reset() {
  board_.clear();
  while (!move_history_.empty()) {
    move_history_.pop();
  }
  current_player_ = Chess::kBlack;
  game_over_ = false;

  emit gameReset();
  emit currentPlayerChanged(current_player_);
}

Chess Game::current_player() const { return current_player_; }

const Board& Game::board() const { return board_; }

bool Game::game_over() const { return game_over_; }

void Game::switchPlayer() {
  current_player_ =
      (current_player_ == Chess::kBlack) ? Chess::kWhite : Chess::kBlack;
  emit currentPlayerChanged(current_player_);
}
