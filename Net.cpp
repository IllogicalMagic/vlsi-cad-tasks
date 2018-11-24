#include "Net.h"

#include <algorithm>
#include <iostream>

void Net::addConnection(Point From, Point To) {
  Unit XDiff = std::abs(From.x - To.x);
  Unit YDiff = std::abs(From.y - To.y);

  // Non-zero horizontal case.
  if (XDiff != 0) {
    Unit XB = From.x;
    Unit XE = To.x;
    // Order from minimal x to maximal.
    // Needed for later removal of zero-length segments.
    if (From.x > To.x)
      std::swap(XB, XE);
    HorSeg.emplace_back(Point(XB, From.y), Point(XE, From.y));
  } else {
    // Add zero-length segment.
    HorSeg.emplace_back(Point(From.x, From.y), Point(From.x, From.y));
    HorSeg.emplace_back(Point(To.x, To.y), Point(To.x, To.y));
  }

  if (YDiff != 0) {
    Point B {To.x, From.y};
    Point E {To.x, To.y};
    VertSeg.emplace_back(B, E);
    // Add transitions from m3 to m2.
    M23Trans.push_back(B);
    M23Trans.push_back(E);
  }
}

// Remove duplicates in transitions layers.
void Net::finalizeNet() {
  // Remove excess transitions form m3 to m2.
  std::sort(M23Trans.begin(), M23Trans.end());
  M23Trans.erase(std::unique(M23Trans.begin(), M23Trans.end()), M23Trans.end());

  // Remove duplicates of horizontal segments.
  std::sort(HorSeg.begin(), HorSeg.end());
  HorSeg.erase(std::unique(HorSeg.begin(), HorSeg.end()), HorSeg.end());

  // Remove excess zero-length horizonal segments (covered by non-zero ones).
  auto PartIt = std::partition(HorSeg.begin(), HorSeg.end(), [](const Segment &S) {
      return S.first.x != S.second.x;
    });
  auto RmIt = std::remove_if(PartIt, HorSeg.end(), [&](const Segment &ZeroSeg) {
      return std::any_of(HorSeg.begin(), PartIt, [&ZeroSeg](const Segment &S) {
          Unit ZX = ZeroSeg.first.x;
          return S.first.y == ZeroSeg.first.y && S.first.x <= ZX && ZX <= S.second.x;
        });
    });
  HorSeg.erase(RmIt, HorSeg.end());
}

enum PtType {
  Pin,
  ViaPin
};

enum Layer {
  Pins,
  Pins_M2,
  M2,
  M2_M3,
  M3
};

static void
dumpLayer(std::ostream &O, Layer L) {
  switch (L) {
  case Pins:
    O << "pins";
    break;
  case Pins_M2:
    O << "pins_m2";
    break;
  case M2:
    O << "m2";
    break;
  case M2_M3:
    O << "m2_m3";
    break;
  case M3:
    O << "m3";
    break;
  }
}

static void
dumpPoint(std::ostream &O, Point P, PtType PType, Layer PLayer) {
  O << "<point x=\"" << P.x << "\" y=\"" << P.y << "\" ";
  O << "layer=\"";
  dumpLayer(O, PLayer);
  O << "\" type=\"";
  switch (PType) {
  case Pin:
    O << "pin";
    break;
  case ViaPin:
    O << "via";
    break;
  }
  O << "\" />" << std::endl;
}

static void
dumpSegment(std::ostream &O, Point P1, Point P2, Layer L) {
  O << "<segment ";
  O << "x1=\"" << P1.x << "\" y1=\"" << P1.y << "\" ";
  O << "x2=\"" << P2.x << "\" y2=\"" << P2.y << "\" ";
  O << "layer=\"";
  dumpLayer(O, L);
  O << "\" />" << std::endl;
}

void Net::dumpXML(std::ostream &O) const {
  O << "<root>" << std::endl;
  O << "  <grid min_x=\"" << LBCorner.x << "\" max_x=\"" << RUCorner.x;
  O << "\" min_y=\"" << LBCorner.y << "\" max_y=\"" << RUCorner.y << "\" />" << std::endl;
  O << "  <net>" << std::endl;
  for (const auto P : Pts) {
    O << "    ";
    dumpPoint(O, P, Pin, Pins);
  }
  // Lift pins to m2 layer as vias.
  for (const auto P : Pts) {
    O << "    ";
    dumpPoint(O, P, ViaPin, Pins_M2);
  }
  for (const auto P : M23Trans) {
    O << "    ";
    dumpPoint(O, P, ViaPin, M2_M3);
  }
  for (const auto &S : VertSeg) {
    O << "    ";
    dumpSegment(O, S.first, S.second, M3);
  }
  for (const auto &S : HorSeg) {
    O << "    ";
    dumpSegment(O, S.first, S.second, M2);
  }
  O << "  </net>" << std::endl;
  O << "</root>" << std::endl;
}
