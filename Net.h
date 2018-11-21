#ifndef STEINER_NET_H_DEFINED__
#define STEINER_NET_H_DEFINED__

#include "Support.h"
#include "Types.h"

#include <istream>
#include <tuple>
#include <vector>

struct Point {
  Unit x = 0, y = 0;
  Point() = default;

  Point(Unit InX, Unit InY): x(InX), y(InY) {}
};

[[maybe_unused]] static
bool operator<(Point A, Point B) {
  return std::tie(A.x, A.y) < std::tie(B.x, B.y);

}

[[maybe_unused]] static
bool operator==(Point A, Point B) {
  return std::tie(A.x, A.y) == std::tie(B.x, B.y);
}


[[maybe_unused]] static
std::istream &operator>>(std::istream &In, Point &Pt) {
  In >> Pt.x >> Pt.y;
  return In;
}

[[maybe_unused]] static
std::ostream &operator<<(std::ostream &Out, Point Pt) {
  Out << "(" << Pt.x << ";" << Pt.y << ")";
  return Out;
}

class Net {
  using Segment = std::pair<Point, Point>;

  std::vector<Point> Pts;
  std::vector<Point> Via;
  std::vector<Segment> VertSeg;
  std::vector<Segment> HorSeg;
  std::vector<Point> M23Trans;

  Point LBCorner;
  Point RUCorner;

public:
  Net() = default;
  void addCorners(Point LB, Point RU) {
    LBCorner = LB;
    RUCorner = RU;
  }

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
        P.x > RUCorner.x ||
        P.y > RUCorner.y) {
      report_error("Attempt to add point outside of grid.\n"
                   "Grid parameters: (", LBCorner.x, "x", LBCorner.y, "); (",
                   RUCorner.x, "x", RUCorner.y, ");\n",
                   "Point coordinates: (", P.x, "x", P.y, ").\n");
    }
    Pts.emplace_back(P);
  }

  auto begin() { return Pts.begin(); }
  auto end() { return Pts.end(); }
  auto begin() const { return Pts.begin(); }
  auto end() const { return Pts.end(); }
  auto size() const { return Pts.size(); }

  void addViaPoint(Point Pt);
  void addConnection(Point From, Point To);

  // Remove duplicates in transitions layers.
  void finalizeNet();
  void dumpXML(std::ostream &O) const;
};

[[maybe_unused]] static
Unit dist(Point A, Point B) {
  return std::abs(A.x - B.x) + std::abs(A.y - B.y);
}

#endif
