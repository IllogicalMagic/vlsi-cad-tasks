#include "MST.h"
#include "Net.h"
#include "Types.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <numeric>
#include <string>
#include <tuple>
#include <utility>
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

// Connect new point with at most 8 others.
// Divide all grid into octants and pick the closest
// point in each octant.
void connectNewPoint(std::vector<EdgeTy> &Edges, size_t PNum, const Graph<Point> &G) {
  Point This = G.vertice(PNum);
  std::array<size_t, 8> Selected;
  std::array<Unit, 8> Dists;
  Selected.fill(PNum);
  Dists.fill(std::numeric_limits<Unit>::max());

  for (size_t i = 0; i < PNum; ++i) {
    Point To = G.vertice(i);
    Unit XDiff = This.x - To.x;
    Unit YDiff = This.y - To.y;
    // Encode quadrant.
    size_t Octant = ((static_cast<size_t>(XDiff < 0) << 1) |
                     (static_cast<size_t>(YDiff < 0)));
    // Based on result quadrant, select proper octant.
    switch (Octant) {
    case 0:
    case 2:
      Octant |= (static_cast<size_t>(XDiff < YDiff) << 2);
      break;
    case 1:
    case 3:
      Octant |= (static_cast<size_t>(XDiff >= YDiff) << 2);
      break;
    default:
      __builtin_unreachable();
    }
    Unit Dist = dist(This, To);
    if (Dist < Dists[Octant]) {
      Selected[Octant] = i;
      Dists[Octant] = Dist;
    }
  }

  for (auto PtIdx : Selected) {
    if (PtIdx != PNum)
      Edges.emplace_back(PtIdx, PNum);
  }
}

Unit getEdgesWeight(const Graph<Point> &G) {
  return std::accumulate(G.edges_begin(), G.edges_end(), Unit(),
                         [&](Unit TotalLen, EdgeTy Edge) {
                           return TotalLen + dist(G.vertice(Edge.From), G.vertice(Edge.To));
                         });
}

// Add new point and prepare sorted edges.
template<typename Compare>
void prepareNewGraphEdges(Graph<Point> &G, std::vector<EdgeTy> &Edges,
                          size_t PNum, Compare Comp) {
  connectNewPoint(Edges, PNum, G);
  auto B = Edges.begin();
  auto E = Edges.end();
  G.swapEdges(Edges);
  // All old edges are already sorted so there is no need to sort all range.
  // Just sort new edges and then merge.
  std::sort(E - PNum, E, Comp);
  std::inplace_merge(B, E - PNum, E, Comp);
}

auto iteratedSteiner(const Net &N, std::vector<Point> Grid) {

  bool Changed = true;
  Graph<Point> G(N.begin(), N.end());
  G.connectAllToAll();

  std::vector<EdgeTy> TmpEdges;
  TmpEdges.reserve(G.edges_size());

  auto EdgeSort = [&](const EdgeTy &A, const EdgeTy &B) {
    auto ADist = dist(G.vertice(A.From), G.vertice(A.To));
    auto BDist = dist(G.vertice(B.From), G.vertice(B.To));
    return ADist < BDist;
  };

  // Initial length.
  // TODO: remove this after special graph methods will be added.
  std::sort(G.edges_begin(), G.edges_end(), EdgeSort);
  G.swapEdges(getMSTEdges(G));
  Unit MinLen = getEdgesWeight(G);

  while (Changed && !Grid.empty()) {
    Changed = false;
    size_t GridSize = Grid.size();
    size_t BestCandidateIdx;
    size_t OldPNum = G.vertices_size();

    for (size_t i = 0; i < GridSize; ++i) {
      Point Pt = Grid[i];

      // Create new state with added point and add it to graph.
      G.push_vertice(Pt);
      TmpEdges.assign(G.edges_begin(), G.edges_end());
      prepareNewGraphEdges(G, TmpEdges, OldPNum, EdgeSort);
      Unit NewLen = getMSTLen(G);

      // Save point if it is the best solution.
      if (NewLen < MinLen) {
        Changed = true;
        BestCandidateIdx = i;
        MinLen = NewLen;
      }

      // Restore initial state.
      G.pop_vertice();
      G.swapEdges(TmpEdges);
    }

    // Add new point.
    if (Changed) {
      G.push_vertice(Grid[BestCandidateIdx]);
      TmpEdges.assign(G.edges_begin(), G.edges_end());
      prepareNewGraphEdges(G, TmpEdges, OldPNum, EdgeSort);
      G.swapEdges(getMSTEdges(G));

      // Remove selected point from list of candidates.
      std::swap(Grid[BestCandidateIdx], Grid.back());
      Grid.pop_back();
    }
  }

  G.swapEdges(getMSTEdges(G));
  std::cout << "Total length of Steiner tree: " << getEdgesWeight(G) << "\n";
  return G;
}

int main(int argc, char **argv) {
  std::string In = parseArgs(argc, argv);
  Net N = buildNet(std::move(In));
  std::vector<Point> C = getHanansGrid(N);
  Graph<Point> G = iteratedSteiner(N, std::move(C));
  G.dump();
  return 0;
}
