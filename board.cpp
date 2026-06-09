#include "board.h"

#include <algorithm>
#include <iostream>

Board::Board(int width, int height)
    : width_(width), height_(height), pieces_(width * height, Chess::kEmpty) {}

/* 在指定位置下棋 */
bool Board::placeAt(int x, int y, Chess chess) {
  // 越界判断
  if (!inBound(x, y)) {
    return false;
  }
  // 检查是否已有棋子
  if (at(x, y) != Chess::kEmpty) {
    return false;
  }
  // 计算出来对应一维数组的索引
  pieces_[index(x, y)] = chess; // 更新该位置棋子信息
  return true;
}

/* 扫描整个棋盘，判断是否出现五连 */
bool Board::hasFiveInBoard(Chess chess) const {
  const int kDirections[4][2] = {
      {1, 0},  // 横向
      {0, 1},  // 竖向
      {1, 1},  // 对角线方向
      {1, -1}, // 反对角线方向
  };

  for (int i = 0; i < width_; ++i) {
    for (int j = 0; j < height_; ++j) {
      // 如果当前位置不是当前要判断的棋子，跳过
      if (at(i, j) != chess) {
        continue;
      }

      // 判断该方向上是否有五个棋子相连
      for (const auto kDirection : kDirections) {
        int count = 1;
        count += countConsecutive(i, j, kDirection[0], kDirection[1], chess);

        // 有大于等于五个棋子相连
        if (count >= 5) {
          return true;
        }
      }
    }
  }

  return false;
}

/* 以指定位置为中心，判断是否出现五连 */
bool Board::hasFiveAt(int x, int y, Chess chess) const {
  if (!inBound(x, y) || at(x, y) != chess) {
    return false;
  }

  const int kDirections[4][2] = {
      {1, 0},  // 横向
      {0, 1},  // 竖向
      {1, 1},  // 对角线方向
      {1, -1}, // 反对角线方向
  };

  // 判断该方向上是否有五个棋子相连
  for (const auto kDirection : kDirections) {
    int cnt = 1;
    cnt += countConsecutive(x, y, kDirection[0], kDirection[1], chess);
    cnt += countConsecutive(x, y, -kDirection[0], -kDirection[1], chess);
    // 有大于等于五个棋子相连
    if (cnt >= 5) {
      return true;
    }
  }

  return false;
}

/* 将棋盘打印到终端中 */
void Board::printBoardToTerminal() const {
  for (std::size_t i = 0; i < pieces_.size(); ++i) {
    char c;
    switch (pieces_[i]) {
    case Chess::kEmpty:
      c = '*';
      break;
    case Chess::kBlack:
      c = 'X';
      break;
    case Chess::kWhite:
      c = 'O';
      break;
    }
    std::cout << c;
    // 到达行尾进行换行
    if (i % width_ == width_ - 1) {
      std::cout << std::endl;
    }
  }
}

/* 判断是否在棋盘内 */
bool Board::inBound(int x, int y) const {
  return x >= 0 && x < width_ && y >= 0 && y < height_;
}

/* 根据二维坐标求得对应的一维数组中的索引 */
std::size_t Board::index(int x, int y) const { return y * width_ + x; }

/* 获得 x, y 处的棋子类型 */
Chess Board::at(int x, int y) const {
  if (!inBound(x, y)) {
    return Chess::kEmpty;
  }
  return pieces_[index(x, y)];
}

/* 判断指定位置是否为空 */
bool Board::isEmpty(int x, int y) const {
  return inBound(x, y) && pieces_[index(x, y)] == Chess::kEmpty;
}

/* 清空棋盘 */
void Board::clear() {
  std::fill(pieces_.begin(), pieces_.end(), Chess::kEmpty);
}

/* 强制清除指定位置的棋子（悔棋专用） */
void Board::forceClear(int x, int y) {
  if (inBound(x, y)) {
    pieces_[index(x, y)] = Chess::kEmpty;
  }
}

int Board::width() const { return width_; }
int Board::height() const { return height_; }

/* 统计从(x,y)出发(不含)，在(dx,dy)方向上连续chess棋子的数量 */
int Board::countConsecutive(int x, int y, int dx, int dy, Chess chess) const {
  int count = 0;
  for (int step = 1; step <= 5; ++step) {
    int nx = x + dx * step;
    int ny = y + dy * step;
    if (!inBound(nx, ny) || at(nx, ny) != chess) {
      break;
    }
    ++count;
  }
  return count;
}
