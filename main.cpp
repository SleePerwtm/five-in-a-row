#include <cstddef>
#include <iostream>
#include <vector>

enum class Chess { kEmpty, kBlack, kWhite };

// 棋盘类
class Board {
public:
  Board() = delete;
  Board(int width, int height);

  /* 在指定位置下棋 */
  bool placeAt(int x, int y, Chess chess);
  /* 扫描整个棋盘，判断是否出现五连 */
  bool hasFiveInBoard(Chess chess) const;
  /* 以指定位置为中心，判断是否出现五连 */
  bool hasFiveAt(int x, int y, Chess chess) const;
  /* 将棋盘打印到终端中 */
  void printBoardToTerminal() const;

private:
  // 棋盘数据
  int width_;
  int height_;
  std::vector<Chess> board_;

  /* 判断是否在棋盘内 */
  bool inBound(int x, int y) const;
  /* 根据二维坐标求得对应的一维数组中的索引 */
  std::size_t index(int x, int y) const;
  /* 获得 x, y 处的棋子类型 */
  Chess at(int x, int y) const;
};

Board::Board(int width, int height)
    : width_(width), height_(height), board_(width * height, Chess::kEmpty) {}

/* 在指定位置下棋 */
bool Board::placeAt(int x, int y, Chess chess) {
  // 越界判断
  if (!inBound(x, y)) {
    return false;
  }
  // 计算出来对应一维数组的索引
  board_[index(x, y)] = chess; // 更新该位置棋子信息
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

      for (const auto kDirection : kDirections) {
        int count = 1;
        // 判断该方向上是否有五个棋子相连
        for (int step = 1; step <= 4; ++step) {
          int next_x = i + kDirection[0] * step;
          int next_y = j + kDirection[1] * step;
          if (!inBound(next_x, next_y) || at(next_x, next_y) != chess) {
            break;
          }
          ++count;
        }

        if (count == 5) {
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

  for (const auto kDirection : kDirections) {
    int count = 1;

    for (int step = 1; step <= 4; ++step) {
      int next_x = x + kDirection[0] * step;
      int next_y = y + kDirection[1] * step;
      if (!inBound(next_x, next_y) || at(next_x, next_y) != chess) {
        break;
      }
      ++count;
    }

    for (int step = 1; step <= 4; ++step) {
      int next_x = x - kDirection[0] * step;
      int next_y = y - kDirection[1] * step;
      if (!inBound(next_x, next_y) || at(next_x, next_y) != chess) {
        break;
      }
      ++count;
    }

    if (count >= 5) {
      return true;
    }
  }

  return false;
}

/* 将棋盘打印到终端中 */
void Board::printBoardToTerminal() const {
  for (std::size_t i = 0; i < board_.size(); ++i) {
    char c;
    switch (board_[i]) {
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
Chess Board::at(int x, int y) const { return board_[index(x, y)]; }

int main(int argc, char** argv) {
  std::cout << "Hello, from five-in-a-row!\n";
  Board board = Board(10, 10);
  board.placeAt(0, 1, Chess::kWhite);
  board.placeAt(1, 3, Chess::kBlack);
  for (int i = 0; i < 5; ++i) {
    int a = i + 4;
    board.placeAt(a, 5, Chess::kBlack);
  }
  board.printBoardToTerminal();
  std::cout << "Black win:" << (board.hasFiveAt(1, 5, Chess::kBlack) ? "yes" : "no") << std::endl;
  return 0;
}
