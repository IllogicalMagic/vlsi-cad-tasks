#ifndef STEINER_NET_H_DEFINED__
#define STEINER_NET_H_DEFINED__

#include "Support.h"
#include "Types.h"

#include <istream>
#include <vector>

struct Point {
  Unit x = 0, y = 0;
  Point() = default;

  Point(Unit InX, Unit InY): x(InX), y(InY) {}
};

static __attribute__((unused))
bool operator<(Point A, Point B) {
  if (A.x == B.x)
    return A.y < B.y;
  return A.x < B.x;
}

static __attribute__((unused))
std::istream &operator>>(std::istream &In, Point &Pt) {
  In >> Pt.x >> Pt.y;
  return In;
}

static __attribute__((unused))
std::ostream &operator<<(std::ostream &Out, Point Pt) {
  Out << "(" << Pt.x << ";" << Pt.y << ")";
  return Out;
}

class Net {
  std::vector<Point> Pts;
  Point LBCorner;
  Point RUCorner;

public:
  Net(Point LB, Point RU): LBCorner(LB), RUCorner(RU) {}

  Net(const Net &) = delete;
  Net(Net &&) = default;
  void operator=(const Net &) = delete;
  Net &operator=(Net &&) = default;
  ~Net() = default;

  void reserve(size_t Size) {
    Pts.reserve(Size);
  }

  void addPoint(Point P) {
    if (P.x < LBCorner.x ||
        P.y < LBCorner.y ||
        P.x >= RUCorner.x ||
        P.y >= RUCorner.y) {
      report_error("Attempt to add point outside of grid.\n"
                   "Grid parameters: (", LBCorner.x, "x", LBCorner.y, ");\n",
                   "Point coordinates: (", P.x, "x", P.y, ").\n");
    }
    Pts.emplace_back(P);
  }

  auto begin() { return Pts.begin(); }
  auto end() { return Pts.end(); }
  auto begin() const { return Pts.begin(); }
  auto end() const { return Pts.end(); }
  auto size() const { return Pts.size(); }

  // TODO: Add dump method.
};

static __attribute__((unused))
Unit dist(Point A, Point B) {
  return std::abs(A.x - B.x) + std::abs(A.y - B.y);
}

#endif
