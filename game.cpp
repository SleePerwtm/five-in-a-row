#include "game.h"

#include <algorithm>

#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Game::Game(int board_size, QObject* parent)
    : QObject(parent), board_(board_size, board_size) {}

bool Game::placePiece(int row, int col) {
  if (replay_mode_ || game_over_)
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
  if (replay_mode_ || move_history_.empty())
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
  // 如果正在复盘，先退出复盘
  if (replay_mode_) {
    stopReplay();
  }

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

bool Game::saveToFile(const QString& path) const {
  QJsonObject root;
  root["version"] = 1;
  root["boardSize"] = board_.width();
  root["date"] = QDateTime::currentDateTime().toString(Qt::ISODate);

  // 确定结果
  QString result = "in_progress";
  if (game_over_) {
    // 游戏结束时 current_player_ 已经被切换了，所以赢家是对方
    result = (current_player_ == Chess::kBlack) ? "white_wins" : "black_wins";
  }
  root["result"] = result;

  // 将 move_history_（栈）中的步转换为 JSON 数组（需要倒序得到正序）
  QJsonArray moves;
  // 把栈中的步转移到临时 vector 并反转
  std::vector<QPoint> history;
  {
    // 不能直接修改 move_history_，用拷贝的方式
    std::stack<QPoint> copy = move_history_;
    while (!copy.empty()) {
      history.push_back(copy.top());
      copy.pop();
    }
    // 反转：栈顶是最后一步，反转为从第一步开始
    std::reverse(history.begin(), history.end());
  }

  // 确定每步的玩家：第一步是黑棋
  Chess player = Chess::kBlack;
  for (const auto& move : history) {
    QJsonObject step;
    step["row"] = move.x();
    step["col"] = move.y();
    step["player"] = (player == Chess::kBlack) ? "black" : "white";
    moves.append(step);
    player = (player == Chess::kBlack) ? Chess::kWhite : Chess::kBlack;
  }
  root["moves"] = moves;

  QJsonDocument doc(root);
  QFile file(path);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return false;
  file.write(doc.toJson(QJsonDocument::Indented));
  file.close();
  return true;
}

bool Game::loadFromFile(const QString& path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  QByteArray data = file.readAll();
  file.close();

  QJsonParseError error;
  QJsonDocument doc = QJsonDocument::fromJson(data, &error);
  if (error.error != QJsonParseError::NoError)
    return false;

  QJsonObject root = doc.object();
  if (root["version"].toInt() != 1)
    return false;

  int board_size = root["boardSize"].toInt(15);
  // 棋盘大小不匹配时拒绝加载
  if (board_size != board_.width())
    return false;

  // 清空 replay 数据
  replay_moves_.clear();
  replay_players_.clear();
  replay_step_ = 0;

  QJsonArray moves = root["moves"].toArray();
  for (const auto& val : moves) {
    QJsonObject step = val.toObject();
    int row = step["row"].toInt();
    int col = step["col"].toInt();
    QString player_str = step["player"].toString();
    Chess player = (player_str == "black") ? Chess::kBlack : Chess::kWhite;
    replay_moves_.push_back(QPoint(row, col));
    replay_players_.push_back(player);
  }

  // 解析结果
  QString result_str = root["result"].toString();
  if (result_str == "black_wins")
    replay_result_ = Chess::kBlack;
  else if (result_str == "white_wins")
    replay_result_ = Chess::kWhite;
  else
    replay_result_ = Chess::kEmpty;

  // 进入复盘模式：清空棋盘，显示初始状态
  replay_mode_ = true;

  // 清空棋盘用于复盘显示
  board_.clear();

  emit replayStarted();
  emit replayStepChanged(0, static_cast<int>(replay_moves_.size()));
  return true;
}

bool Game::stepForward() {
  if (!replay_mode_)
    return false;
  if (replay_step_ >= static_cast<int>(replay_moves_.size()))
    return false;

  int row = replay_moves_[replay_step_].x();
  int col = replay_moves_[replay_step_].y();
  Chess player = replay_players_[replay_step_];

  // 直接在棋盘上放子（不经过 placePiece，不触发胜负判定）
  board_.placeAt(col, row, player);

  ++replay_step_;
  emit piecePlaced(row, col, player);
  emit replayStepChanged(replay_step_, static_cast<int>(replay_moves_.size()));

  // 到达最后一步时检查结果
  if (replay_step_ == static_cast<int>(replay_moves_.size()) &&
      replay_result_ != Chess::kEmpty) {
    // 显示对局结果
    // 注意：不 emit gameOver，因为这不是当前对局
  }
  return true;
}

bool Game::stepBackward() {
  if (!replay_mode_)
    return false;
  if (replay_step_ <= 0)
    return false;

  --replay_step_;

  int row = replay_moves_[replay_step_].x();
  int col = replay_moves_[replay_step_].y();

  board_.forceClear(col, row);

  emit pieceRemoved(row, col);
  emit replayStepChanged(replay_step_, static_cast<int>(replay_moves_.size()));
  return true;
}

void Game::stopReplay() {
  if (!replay_mode_)
    return;

  replay_mode_ = false;
  replay_step_ = 0;
  replay_moves_.clear();
  replay_players_.clear();
  replay_result_ = Chess::kEmpty;

  // 清空棋盘，准备新游戏
  board_.clear();

  emit replayEnded();
}

bool Game::isReplayMode() const { return replay_mode_; }

int Game::replayTotalSteps() const {
  return static_cast<int>(replay_moves_.size());
}

int Game::replayCurrentStep() const { return replay_step_; }

void Game::switchPlayer() {
  current_player_ =
      (current_player_ == Chess::kBlack) ? Chess::kWhite : Chess::kBlack;
  emit currentPlayerChanged(current_player_);
}
