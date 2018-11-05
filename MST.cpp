#include "MST.h"

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include <cassert>

// Helper structure for MST building.
template<typename T>
class DisjointSet {
  struct Node {
    Node *Parent;
    size_t Size;
    T Value;

    template<typename U>
    Node(U &&Val): Parent(this), Size(1), Value(std::forward<U>(Val)) {}

    Node(Node &&N): Parent(this), Size(N.Size), Value(std::move(N.Value)) {
      assert(N.Parent == &N && "Can handle only initialization cases");
    }

    Node(const Node &) = delete;
    void operator=(Node) = delete;
  };

  Node This;

  Node *findReprNode() {
    Node *Cur = &This;
    while (Cur->Parent != Cur) {
      Cur = Cur->Parent;
    }
    This.Parent = Cur;
    return Cur;
  }

public:
  template<typename U>
  DisjointSet(U &&Val): This(std::forward<U>(Val)) {}

  void unite(DisjointSet &Other) {
    Node *Lhs = findReprNode();
    Node *Rhs = Other.findReprNode();
    if (Lhs->Size < Rhs->Size) {
      std::swap(Lhs, Rhs);
    }

    Rhs->Parent = Lhs;
    Lhs->Size += Rhs->Size;
  }

  const T& findReprMember() {
    return findReprNode()->Value;
  }
};

using EdgeTy = typename Graph<Point>::EdgeType;

template<typename InitAcc, typename UpdateAcc>
auto getMSTCommon(const Graph<Point> &G, InitAcc Initializer, UpdateAcc Updater) {
  std::vector<DisjointSet<int>> Segments;
  Segments.reserve(G.vertices_size());
  // Assign each segment it's own color.
  std::generate_n(std::back_inserter(Segments), G.vertices_size(), [N = 0]() mutable {
      int X = N;
      ++N;
      return X;
    });

  std::vector<EdgeTy> EdgeCandidates;
  EdgeCandidates.assign(G.edges_begin(), G.edges_end());
  std::sort(EdgeCandidates.begin(), EdgeCandidates.end(),
            [&](const EdgeTy &A, const EdgeTy &B) {
              auto ADist = dist(G.vertice(A.From), G.vertice(A.To));
              auto BDist = dist(G.vertice(B.From), G.vertice(B.To));
              return ADist < BDist;
            });

  // Pick an edge and update accumulator
  // if this edge is not in MST and unite segments.
  // Otherwise skip the edge.
  size_t AddedEdges = 0;
  size_t MaxEdges = G.vertices_size() - 1;

  auto Accumulator = Initializer();
  for (auto Edge : EdgeCandidates) {
    auto &FromSet = Segments[Edge.From];
    auto &ToSet = Segments[Edge.To];
    if (FromSet.findReprMember() == ToSet.findReprMember())
      continue;
    FromSet.unite(ToSet);
    Updater(Accumulator, std::move(Edge));

    ++AddedEdges;
    if (AddedEdges == MaxEdges)
      break;
  }

  return Accumulator;
}

Unit getMSTLen(const Graph<Point> &G) {
  return getMSTCommon(G,
                      []() -> Unit { return 0; },
                      [&](Unit &TotalLen, EdgeTy Edge) {
                        TotalLen += dist(G.vertice(Edge.From), G.vertice(Edge.To));
                      });
}

std::vector<EdgeTy>
getMSTEdges(const Graph<Point> &G) {
  return getMSTCommon(G,
                      [&]() -> std::vector<EdgeTy> {
                        std::vector<EdgeTy> Edges;
                        Edges.reserve(G.vertices_size() - 1);
                        return Edges;
                      },
                      [&](std::vector<EdgeTy> &Edges, EdgeTy Edge) {
                        Edges.emplace_back(std::move(Edge));
                      });
}
