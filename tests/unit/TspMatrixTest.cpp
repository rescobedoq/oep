#include "gtest/gtest.h"
#include "../../src/algorithms/tsp/TspMatrix.h"

class TspMatrixTest : public ::testing::Test {
protected:
    std::vector<int64_t> nodeIds = {100, 200, 300, 400};
    TspMatrix matrix{nodeIds.size(), nodeIds};    
    void SetUp() override {
        matrix.setDistance(0, 1, 10.0);
        matrix.setDistance(0, 2, 15.0);
        matrix.setDistance(0, 3, 20.0);
        matrix.setDistance(1, 0, 10.0);
        matrix.setDistance(1, 2, 35.0);
        matrix.setDistance(1, 3, 25.0);
        matrix.setDistance(2, 0, 15.0);
        matrix.setDistance(2, 1, 35.0);
        matrix.setDistance(2, 3, 30.0);
        matrix.setDistance(3, 0, 20.0);
        matrix.setDistance(3, 1, 25.0);
        matrix.setDistance(3, 2, 30.0);
    }
};

TEST_F(TspMatrixTest, CheckBasicAccessors) {
    EXPECT_EQ(matrix.getSize(), 4);
    EXPECT_NEAR(matrix.getEntry(0, 3).distance, 20.0, 1e-6);
    EXPECT_NEAR(matrix.getEntry(2, 1).distance, 35.0, 1e-6);
    EXPECT_EQ(matrix.getNodeId(0), 100);
    EXPECT_EQ(matrix.getNodeId(3), 400);
}

TEST_F(TspMatrixTest, CalculateOptimalTourCost_Closed) {
    std::vector<int> optimalTour = {0, 1, 3, 2};    
    double cost = matrix.calculateTourCost(optimalTour, true);   
    EXPECT_NEAR(cost, 80.0, 1e-6) << "El costo del tour cerrado debe ser 80.0";
}

TEST_F(TspMatrixTest, CalculateTourCost_Open) {
    std::vector<int> tour = {0, 1, 3, 2};    
    double cost = matrix.calculateTourCost(tour, false);    
    EXPECT_NEAR(cost, 65.0, 1e-6) << "El costo del tour abierto debe ser 65.0";
}

TEST_F(TspMatrixTest, CalculateTourCost_OneNode) {
    std::vector<int> singleNodeTour = {0};    
    double closedCost = matrix.calculateTourCost(singleNodeTour, true);
    double openCost = matrix.calculateTourCost(singleNodeTour, false);    
    EXPECT_NEAR(closedCost, 0.0, 1e-6);
    EXPECT_NEAR(openCost, 0.0, 1e-6);
}

TEST_F(TspMatrixTest, NearestNeighborHeuristic_StartFrom0) {
    std::vector<int> expectedTour = {0, 1, 3, 2};
    std::vector<int> actualTour = matrix.nearestNeighborRoute(0);   
    EXPECT_EQ(actualTour, expectedTour);
    double cost = matrix.calculateTourCost(actualTour, false);
    EXPECT_NEAR(cost, 65.0, 1e-6);
}