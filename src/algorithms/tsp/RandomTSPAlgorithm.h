#pragma once

#include "TspMatrix.h"
#include "../../core/interfaces/ITspAlgorithm.h"
#include <random>
#include <vector>
#include <cstdint>

class RandomTSPAlgorithm : public ITspAlgorithm {
private:
	int maxIterations_;
	mutable std::mt19937 rng_;

public:
	explicit RandomTSPAlgorithm(int maxIterations = 10000);

	// ITspAlgorithm implementation
	std::vector<int> solve(
		const TspMatrix& matrix,
		const std::vector<int64_t>& nodeIds
	) override;

	std::string getName() const override { return "RandomTSP"; }

	void setMaxIterations(int maxIterations) override { maxIterations_ = maxIterations; }

private:
	std::vector<int> ensureStartNode(const std::vector<int>& route, int startIdx) const;
	double calculateTourDistance(const TspMatrix& matrix, const std::vector<int>& route, bool returnToStart) const;
};

