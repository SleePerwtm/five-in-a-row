#ifndef GAME_H
#define GAME_H

#include "board.h"

#include <stack>
#include <vector>

#include <QObject>
#include <QPoint>
#include <QString>

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
  /* 保存棋局到 JSON 文件 */
  bool saveToFile(const QString& path) const;
  /* 从 JSON 文件加载棋局并进入复盘模式 */
  bool loadFromFile(const QString& path);
  /* 复盘：前进一歩 */
  bool stepForward();
  /* 复盘：后退一歩 */
  bool stepBackward();
  /* 退出复盘模式 */
  void stopReplay();
  /* 是否处于复盘模式 */
  bool isReplayMode() const;
  /* 复盘总步数 */
  int replayTotalSteps() const;
  /* 当前复盘步数 */
  int replayCurrentStep() const;

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
  /* 复盘模式开始 */
  void replayStarted();
  /* 复盘模式结束 */
  void replayEnded();
  /* 复盘步数变化 */
  void replayStepChanged(int step, int total);

private:
  Board board_;
  Chess current_player_ = Chess::kBlack;
  std::stack<QPoint> move_history_;
  bool game_over_ = false;
  // 复盘模式
  bool replay_mode_ = false;
  int replay_step_ = 0;
  std::vector<QPoint> replay_moves_;
  std::vector<Chess> replay_players_;
  Chess replay_result_ = Chess::kEmpty;

  void switchPlayer();
};

#endif // GAME_H
