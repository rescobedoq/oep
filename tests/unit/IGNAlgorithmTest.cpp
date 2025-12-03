#include "gtest/gtest.h"
#include "algorithms/tsp/IGNAlgorithm.h"
#include "algorithms/tsp/TspMatrix.h"
#include <vector>

// Construye una matriz pequeña 4x4 con distancias simétricas
static TspMatrix makeSimpleMatrix() {
    std::vector<int64_t> nodeIds = {10,20,30,40};
    TspMatrix m(4, nodeIds);
    // Seteamos distancias manualmente (índices 0..3)
    // dist(i,j) = abs(i-j) por simplicidad
    for (size_t i=0;i<4;i++){
        for (size_t j=0;j<4;j++){
            if (i==j) m.setDistance(i,j, 0.0);
            else m.setDistance(i,j, double(std::abs((int)i - (int)j)));
        }
    }
    return m;
}

TEST(IGNAlgorithmTest, ProducesValidPermutation) {
    auto matrix = makeSimpleMatrix();
    std::vector<int64_t> ids = {10,20,30,40};
    IGNAlgorithm algo(100);
    auto route = algo.solve(matrix, ids);

    // Debe contener 4 índices únicos en 0..3
    EXPECT_EQ(route.size(), 4u);
    std::vector<bool> seen(4,false);
    for (int idx : route) {
        ASSERT_GE(idx, 0);
        ASSERT_LT(idx, 4);
        seen[idx] = true;
    }
    for (bool s : seen) EXPECT_TRUE(s);
    
    // Opcional: comparar coste con calculateTourCost
    double cost = matrix.calculateTourCost(route, true);
    EXPECT_TRUE(std::isfinite(cost));
}