#include "MST.h"
#include "Net.h"
#include "StlHelpers.hpp"
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

using VertEdges = std::pair<EdgeTy *, EdgeTy *>;

static void
rememberEdge(EdgeTy *Edge, std::vector<VertEdges> &EdgesToConnect,
             int Degree, size_t VertIdx) {
  if (Degree == 1)
    EdgesToConnect[VertIdx].first = Edge;
  else if (Degree == 2)
    EdgesToConnect[VertIdx].second = Edge;
  else {
    EdgesToConnect[VertIdx].first = nullptr;
    EdgesToConnect[VertIdx].second = nullptr;
  }
}

void remove2DegreePoints(Graph<Point> &G, size_t NetPts) {
  std::vector<int> Degrees(G.vertices_size() - NetPts);
  std::vector<VertEdges> EdgesToConnect(Degrees.size());

  // Find all added vertices with degree <= 2.
  for (auto &Edge : G.edges()) {
    if (Edge.From >= NetPts) {
      int DFrom = ++Degrees[Edge.From - NetPts];
      rememberEdge(&Edge, EdgesToConnect, DFrom, Edge.From - NetPts);
    }
    if (Edge.To >= NetPts) {
      int DTo = ++Degrees[Edge.To - NetPts];
      rememberEdge(&Edge, EdgesToConnect, DTo, Edge.To - NetPts);
    }
  }

  // Connect edges or remove them.
  for (size_t VertIdx = 0, VE = Degrees.size(); VertIdx < VE; ++VertIdx) {
    int D = Degrees[VertIdx];
    // Degree == one -- remove. Mark edge for later removal.
    if (D == 1) {
      auto &Edge = *EdgesToConnect[VertIdx].first;
      Edge.From = 0;
      Edge.To = 0;
    }
    // Degree == two -- connect.
    if (D == 2) {
      auto &Edge1 = *EdgesToConnect[VertIdx].first;
      auto &Edge2 = *EdgesToConnect[VertIdx].second;
      size_t Vert = VertIdx + NetPts;
      // x -> a
      if (Edge1.From == Vert) {
        // x -> b
        if (Edge2.From == Vert)
          // b -> a
          Edge1.From = Edge2.To;
        // b -> x
        else
          // b -> a
          Edge1.From = Edge2.From;
      // a -> x
      } else {
        // x -> b
        if (Edge2.From == Vert)
          // a -> b
          Edge1.To = Edge2.To;
        // b -> x
        else
          // a -> b
          Edge1.To = Edge2.From;
      }
      // Save all info since next iterations could use this info.
      Edge2 = Edge1;
    }
  }

  // Remove all deleted edges.
  G.edges_erase(std::remove_if(G.edges_begin(), G.edges_end(),
                               [](EdgeTy E) {
        return E.From == 0 && E.To == 0;
      }), G.edges_end());

  // Remove joined edges.
  std::sort(G.edges_begin(), G.edges_end());
  G.edges_erase(std::unique(G.edges_begin(), G.edges_end()), G.edges_end());

  auto Res = remove_if_with_index(G.vertices_begin() + NetPts,
                                  G.vertices_end(),
                                  [&](Point Pt, size_t Idx) {
                                    return Degrees[Idx] <= 2;
                                  });
  G.vertices_erase(Res, G.vertices_end());

  // Shift points. Removed point can be in the middle
  // so we need to adjust all edges that contain points
  // after removed ones.
  for (int i = Degrees.size() - 1, e = 0; i >= e; --i) {
    if (Degrees[i] <= 2) {
      size_t VIdx = i + NetPts;
      for (auto &Edge : G.edges()) {
        if (Edge.From > VIdx)
          --Edge.From;
        if (Edge.To > VIdx)
          --Edge.To;
      }
    }
  }
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

      remove2DegreePoints(G, N.size());
      std::sort(G.edges_begin(), G.edges_end(), EdgeSort);

      // Remove selected point from list of candidates.
      std::swap(Grid[BestCandidateIdx], Grid.back());
      Grid.pop_back();
    }
  }

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
