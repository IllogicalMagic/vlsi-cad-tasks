#include "MST.h"
#include "Net.h"
#include "Types.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

#include <cstring>

// It is actually just a product of all unique x and y coordinates.
auto getHanansGrid(const Net &N) {
  std::vector<Point> Grid;
  // Collect coordinates.
  std::vector<Unit> Xs, Ys;
  Xs.reserve(N.size());
  Ys.reserve(N.size());
  for (auto Pt : N) {
    Xs.emplace_back(Pt.x);
    Ys.emplace_back(Pt.y);
  }

  // Unique them.
  std::sort(Xs.begin(), Xs.end());
  std::sort(Ys.begin(), Ys.end());
  Xs.erase(std::unique(Xs.begin(), Xs.end()), Xs.end());
  Ys.erase(std::unique(Ys.begin(), Ys.end()), Ys.end());

  // Get a product.
  Grid.reserve(Xs.size() * Ys.size());
  for (auto X : Xs) {
    for (auto Y : Ys) {
      Grid.emplace_back(X, Y);
    }
  }

  // Delete duplicates of original points.
  std::vector<Point> Pts;
  Pts.assign(N.begin(), N.end());
  std::sort(Pts.begin(), Pts.end());
  auto It = std::remove_if(Grid.begin(), Grid.end(), [&](const Point P) {
      return std::binary_search(Pts.cbegin(), Pts.cend(), P);
    });
  Grid.erase(It, Grid.end());

  return Grid;
}

Net buildNet(const std::string In) {
  std::ifstream InFile(In);
  Point LB, RU;
  InFile >> LB >> RU;
  Net N(LB, RU);
  size_t PointsNum;
  InFile >> PointsNum;
  N.reserve(PointsNum);
  Point P;
  while (InFile >> P) {
    --PointsNum;
    N.addPoint(P);
  }
  if (PointsNum)
    report_error("Bad number of points in input file");

  return N;
}

std::string parseArgs(int argc, char **argv) {
  if (argc < 2) {
    report_error("Options should be specified. Try --help.\n");
  }

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--input") == 0) {
      if (i == argc - 1)
        report_error("Filename should be specified after --input option.\n");
      return {argv[i + 1]};
    } else if (strcmp(argv[i], "--help") == 0) {
      std::cout <<
        "Usage: Steiner <options>.\n"
        "Allowed options:\n"
        "  --help           prints usage and exits\n"
        "  --input <file>   specifies input file with net configuration."
                << std::endl;
      exit(0);
    } else {
      report_error("Unknown option. Try --help.\n");
    }
  }
  __builtin_unreachable();
}

__attribute((unused))
void dumpPoints(const std::vector<Point> &Pts) {
  for (Point P : Pts) {
    std::cout << P << std::endl;
  }
}

using EdgeTy = typename Graph<Point>::EdgeType;

// Connect all points to all others.
void connectPoints(std::vector<EdgeTy> &Edges, size_t PNum) {
  for (size_t i = 0; i < PNum; ++i) {
    for (size_t j = i + 1; j < PNum; ++j) {
      Edges.emplace_back(i, j);
    }
  }
}

// Connect new point with others.
void connectNewPoint(std::vector<EdgeTy> &Edges, size_t PNum) {
  for (size_t i = 0; i < PNum; ++i)
    Edges.emplace_back(i, PNum);
}

auto iteratedSteiner(const Net &N, std::vector<Point> Grid) {
  std::vector<Point> Vertices;
  Vertices.reserve(N.size() + Grid.size());
  Vertices.assign(N.begin(), N.end());

  std::vector<typename Graph<Point>::EdgeType> Edges;
  Edges.reserve(Vertices.capacity() * Vertices.capacity());
  connectPoints(Edges, Vertices.size());

  bool Changed = true;
  Graph<Point> G;
  // Initial length.
  // TODO: remove this after special graph methods will be added.
  G.swapVertices(Vertices);
  G.swapEdges(Edges);
  Unit MinLen = getMSTLen(G);
  G.swapVertices(Vertices);
  G.swapEdges(Edges);

  while (Changed && !Grid.empty()) {
    Changed = false;
    size_t GridSize = Grid.size();
    size_t BestCandidateIdx;
    size_t OldPNum = Vertices.size();

    for (size_t i = 0; i < GridSize; ++i) {
      Point Pt = Grid[i];

      // Create new state with added point.
      Vertices.push_back(Pt);
      connectNewPoint(Edges, OldPNum);

      // Create graph.
      G.swapVertices(Vertices);
      G.swapEdges(Edges);

      Unit NewLen = getMSTLen(G);

      // Save point if it is the best solution.
      if (NewLen < MinLen) {
        Changed = true;
        BestCandidateIdx = i;
        MinLen = NewLen;
      }

      // Restore initial state.
      G.swapVertices(Vertices);
      G.swapEdges(Edges);

      Vertices.pop_back();
      Edges.erase(Edges.end() - OldPNum, Edges.end());
    }

    // Add new point.
    if (Changed) {
      Vertices.push_back(Grid[BestCandidateIdx]);
      connectNewPoint(Edges, OldPNum);
      // Remove selected point from list of candidates.
      std::swap(Grid[BestCandidateIdx], Grid.back());
      Grid.pop_back();
    }
  }

  G.swapVertices(Vertices);
  G.swapEdges(Edges);
  auto MSTEdges = getMSTEdges(G);
  G.swapEdges(MSTEdges);
  return G;
}

int main(int argc, char **argv) {
  std::string In = parseArgs(argc, argv);
  Net N = buildNet(std::move(In));
  std::vector<Point> C = getHanansGrid(N);
  dumpPoints(C);
  Graph<Point> G = iteratedSteiner(N, std::move(C));
  G.dump();
  return 0;
}