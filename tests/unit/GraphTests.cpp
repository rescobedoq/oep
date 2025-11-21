#include "gtest/gtest.h"
#include "core/entities/Node.h"
#include "core/entities/Edge.h"
#include "core/entities/Graph.h"
#include "core/value_objects/Coordinate.h"
#include "core/value_objects/Distance.h"

using namespace std;
using core::entities::Node;
using core::entities::Edge;
using core::entities::Graph;
using core::value_objects::Coordinate;
using core::value_objects::Distance;

class NodeTest : public ::testing::Test {
protected:
    Coordinate c1{10.0, 20.0};
    Coordinate c2{10.0, 30.0};
    Node n1{1, c1};
    Node n2{2, c2};
    Node n1_dup{1, c2};
};

// Testeando el constructor y los getters
TEST_F(NodeTest, ConstructorAndGetters) {
    EXPECT_EQ(1, n1.getId());
    EXPECT_EQ(10.0, n1.getCoordinate().getLatitude());
    EXPECT_EQ(20.0, n1.getCoordinate().getLongitude());
}

// Testeando la comparación de nodos basada en su ID
TEST_F(NodeTest, ComparisonOperators) {
    EXPECT_TRUE(n1 == n1_dup) << "Nodos con el mismo ID, deben ser iguales.";
    EXPECT_FALSE(n1 != n1_dup) << "Nodos con el mismo ID, no deben ser diferentes.";
    EXPECT_TRUE(n1 != n2) << "Nodos con diferentes IDs, deben ser diferentes.";
}


class EdgeTest : public ::testing::Test {
protected:
    Node* nA = new Node(10, Coordinate{1.0, 1.0});
    Node* nB = new Node(20, Coordinate{2.0, 2.0});
    Distance d100{100.0};
    unordered_map<string, string> tags_init = {{"carretera", "autopista"}};

    Edge* e1;

    void SetUp() override {
        e1 = new Edge(1001, nA, nB, true, d100, tags_init);
    }

    void TearDown() override {
        delete e1;
        delete nA;
        delete nB;
    }
};

// Testeando el constructor y los getters
TEST_F(EdgeTest, ConstructionAndGetters) {
    EXPECT_EQ(1001, e1->getId());
    EXPECT_EQ(nA, e1->getSource());
    EXPECT_EQ(nB, e1->getTarget());
    EXPECT_TRUE(e1->IsOneWay());
    EXPECT_EQ(100.0, e1->getDistance().getMeters());
}

// Testeando la gestión de tags
TEST_F(EdgeTest, TagManagement) {
    EXPECT_TRUE(e1->hasTag("carretera"));
    EXPECT_FALSE(e1->hasTag("superficie"));

    EXPECT_EQ("autopista", e1->getTag("carretera"));
    EXPECT_THROW(e1->getTag("superficie"), std::out_of_range);

    e1->addTag("superficie", "asfalto");
    EXPECT_TRUE(e1->hasTag("superficie"));
    EXPECT_EQ("asfalto", e1->getTag("superficie"));
    EXPECT_EQ(2, e1->getTags().size());
}


class GraphTest : public ::testing::Test {
protected:
    Graph g;
    Coordinate c1{1.0, 1.0};
    Coordinate c2{2.0, 2.0};
    Coordinate c3{3.0, 3.0};
    Distance d10{10.0};

    void SetUp() override {
        g.addNode(1, c1.getLatitude(), c1.getLongitude());
        g.addNode(2, c2.getLatitude(), c2.getLongitude());
        g.addNode(3, c3.getLatitude(), c3.getLongitude());

        g.addEdge(101, 1, 2, d10, false);
        g.addEdge(102, 2, 3, d10, true);

        g.buildAdjacencyList();
    }
};

// Testeando los nodos
TEST_F(GraphTest, NodeConsults) {
    EXPECT_TRUE(g.hasNode(1));
    EXPECT_FALSE(g.hasNode(99));
    EXPECT_EQ(3, g.getNodeCount());
    EXPECT_NE(nullptr, g.getNode(2));
    EXPECT_EQ(nullptr, g.getNode(99));
}

// Testeando las aristas
TEST_F(GraphTest, EdgeConsults) {
    EXPECT_TRUE(g.hasEdge(101));
    EXPECT_FALSE(g.hasEdge(999));
    EXPECT_EQ(2, g.getEdgeCount());
    EXPECT_NE(nullptr, g.getEdge(101));
}

// Testeando adyacencia y vecinos
TEST_F(GraphTest, AdjacencyLogic) {
    EXPECT_EQ(1, g.getOutgoingEdges(1).size());
    EXPECT_EQ(2, g.getOutgoingEdges(2).size());
    EXPECT_EQ(0, g.getOutgoingEdges(3).size());
    EXPECT_EQ(2, g.getNeighbors(2).size());

    EXPECT_TRUE(g.hasDirectEdge(1, 2)) << "1 -> 2 debe ser verdad (bidireccional)";
    EXPECT_TRUE(g.hasDirectEdge(2, 1)) << "2 -> 1 debe ser verdad (bidireccional)";
    EXPECT_TRUE(g.hasDirectEdge(2, 3)) << "2 -> 3 debe ser verdad (unidireccional)";
    EXPECT_FALSE(g.hasDirectEdge(3, 2)) << "3 -> 2 debe ser falso (unidireccional 2->3)";
}

// Testeando utilidades
TEST_F(GraphTest, UtilityMethods) {
    EXPECT_FALSE(g.isEmpty());
    g.clear();
    EXPECT_TRUE(g.isEmpty());
    EXPECT_EQ(0, g.getNodeCount());
    EXPECT_EQ(0, g.getEdgeCount());
    EXPECT_FALSE(g.hasBoundSet());
}