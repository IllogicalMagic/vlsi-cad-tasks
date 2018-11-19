#ifndef STEINER_MST_H_DEFINED__
#define STEINER_MST_H_DEFINED__

#include "Net.h"
#include "Types.h"

#include <tuple>
#include <vector>

template<typename It>
struct Range {
  It Begin, End;
public:
  Range(It B, It E): Begin(B), End(E) {}
  auto begin() { return Begin; }
  auto end() { return End; }
  auto begin() const { return Begin; }
  auto end() const { return End; }
};

template<typename It>
Range(It B, It E) -> Range<It>;

template<typename T>
class Graph {
public:
  struct EdgeType {
    size_t From, To;

    EdgeType() = default;
    EdgeType(size_t from, size_t to): From(from), To(to) {}

    bool operator==(const EdgeType &O) {
      return std::tie(From, To) == std::tie(O.From, O.To);
    }
    bool operator<(const EdgeType &O) {
      return std::tie(From, To) < std::tie(O.From, O.To);
    }
  };

private:
  using ECTy = std::vector<EdgeType>;
  using VCTy = std::vector<T>;

  ECTy Edges;
  VCTy Vertices;
public:
  Graph() = default;

  template<typename VIt>
  Graph(VIt VBegin, VIt VEnd):
    Edges(), Vertices(VBegin, VEnd) {}

  void connectAllToAll() {
    size_t PNum = Vertices.size();
    Edges.reserve(PNum * PNum);
    for (size_t i = 0; i < PNum; ++i) {
      for (size_t j = i + 1; j < PNum; ++j) {
        Edges.emplace_back(i, j);
      }
    }
  }

  Graph(const Graph &) = delete;
  Graph(Graph &&) = default;
  void operator=(const Graph &) = delete;
  void operator=(Graph &&) = delete;
  ~Graph() = default;

  auto edges_begin() { return Edges.begin(); }
  auto edges_end() { return Edges.end(); }
  auto edges_begin() const { return Edges.begin(); }
  auto edges_end() const { return Edges.end(); }
  auto edges() { return Range(edges_begin(), edges_end()); }
  auto edges() const { return Range(edges_begin(), edges_end()); }
  auto edges_size() const { return Edges.size(); }
  EdgeType &edge(size_t Idx) { return Edges[Idx]; }
  const EdgeType &edge(size_t Idx) const { return Edges[Idx]; }

  auto edges_erase(typename ECTy::const_iterator It) {
    return Edges.erase(It);
  }

  auto edges_erase(typename ECTy::const_iterator First, typename ECTy::const_iterator Last) {
    return Edges.erase(First, Last);
  }

  T &vertice(size_t Idx) { return Vertices[Idx]; }
  const T &vertice(size_t Idx) const { return Vertices[Idx]; }

  auto vertices_begin() { return Vertices.begin(); }
  auto vertices_begin() const { return Vertices.begin(); }
  auto vertices_end() { return Vertices.end(); }
  auto vertices_end() const { return Vertices.end(); }
  auto vertices_size() const { return Vertices.size(); }

  auto vertices_erase(typename VCTy::const_iterator It) {
    return Vertices.erase(It);
  }

  void push_vertice(const T &V) {
    Vertices.push_back(V);
  }
  void push_vertice(T &&V) {
    Vertices.push_back(std::move(V));
  }
  void pop_vertice() {
    Vertices.pop_back();
  }

  auto vertices_erase(typename VCTy::const_iterator First, typename VCTy::const_iterator Last) {
    return Vertices.erase(First, Last);
  }

  void swapVertices(std::vector<T> &Vs) {
    std::swap(Vertices, Vs);
  }

  void swapEdges(std::vector<EdgeType> &Es) {
    std::swap(Edges, Es);
  }
  void swapEdges(std::vector<EdgeType> &&Es) {
    Edges = std::move(Es);
  }


  void dump() const {
    std::cout << "Vertices:" << std::endl;
    for (size_t i = 0, e = Vertices.size(); i < e; ++i) {
      std::cout << i << " " << Vertices[i] << std::endl;
    }
    std::cout << "Edges:" << std::endl;
    for (auto E : Edges) {
      std::cout << E.From << " -> " << E.To << std::endl;
    }
  }

  // TODO: add methods for working with edges and vertices?
};

Unit getMSTLen(const Graph<Point> &G);

std::vector<typename Graph<Point>::EdgeType>
getMSTEdges(const Graph<Point> &G);
#endif
