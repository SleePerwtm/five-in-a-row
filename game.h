#ifndef GAME_H
#define GAME_H

#include <QObject>
#include <QPoint>
#include <stack>

#include "board.h"

class Game : public QObject {
  Q_OBJECT

public:
  explicit Game(int board_size = 15, QObject* parent = nullptr);

  /* 落子 */
  bool placePiece(int row, int col);
  /* 悔棋 */
  bool undo();
  /* 重置 */
  void reset();

  Chess current_player() const;
  const Board& board() const;
  bool game_over() const;

signals:
  /* 下棋成功后发出的信号 */
  void piecePlaced(int row, int col, Chess chess);
  /* 悔棋的时候发出的信号 */
  void pieceRemoved(int row, int col);
  /* 游戏结束发出的信号 */
  void gameOver(Chess winner);
  /* 玩家切换时发出的信号 */
  void currentPlayerChanged(Chess player);
  /* 游戏重置发出的信号 */
  void gameReset();

private:
  Board board_;
  Chess current_player_ = Chess::kBlack;
  std::stack<QPoint> move_history_;
  bool game_over_ = false;

  void switchPlayer();
};

#endif // GAME_H
