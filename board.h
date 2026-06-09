#ifndef BOARD_H
#define BOARD_H

#include <cstddef>
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
  /* 获得 x, y 处的棋子类型 */
  Chess at(int x, int y) const;
  /* 判断指定位置是否为空 */
  bool isEmpty(int x, int y) const;
  /* 清空棋盘 */
  void clear();
  /* 强制清除指定位置的棋子（悔棋专用） */
  void forceClear(int x, int y);

  /* 获取棋盘宽度 */
  int width() const;
  /* 获取棋盘高度 */
  int height() const;

private:
  // 棋盘数据
  int width_;                 // 宽度
  int height_;                // 高度
  std::vector<Chess> pieces_; // 存储棋盘信息

  /* 判断是否在棋盘内 */
  bool inBound(int x, int y) const;
  /* 根据二维坐标求得对应的一维数组中的索引 */
  std::size_t index(int x, int y) const;
  /* 统计从(x,y)出发(不含)，在(dx,dy)方向上连续chess棋子的数量 */
  int countConsecutive(int x, int y, int dx, int dy, Chess chess) const;
};

#endif // BOARD_H
