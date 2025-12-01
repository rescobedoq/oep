#pragma once

#include "BaseException.h"
#include <vector>
#include <cstdint>

class TspException : public BaseException {
public:
    enum class ErrorCode {
        UNREACHABLE_NODES,      // Some nodes are unreachable
        INVALID_NODES,          // Nodes do not exist in the graph
        INSUFFICIENT_NODES,     // Less than 2 nodes for TSP
        MATRIX_CONSTRUCTION_FAILED, // Error constructing distance matrix
        NO_VALID_SOLUTION,      // Cannot construct a valid tour
        TIMEOUT,                // Timeout during calculation
        INVALID_PROFILE         // Invalid vehicle profile
    };

    TspException(ErrorCode code, const std::string& message)
        : BaseException(message), errorCode_(code) {}

    TspException(ErrorCode code, const std::string& message, 
                 const std::vector<int64_t>& problematicNodes)
        : BaseException(message), errorCode_(code), problematicNodes_(problematicNodes) {}

    ErrorCode getErrorCode() const { return errorCode_; }
    
    const std::vector<int64_t>& getProblematicNodes() const { 
        return problematicNodes_; 
    }

    /**
     * @brief Gets a user-friendly message
     */
    std::string getUserFriendlyMessage() const {
        switch (errorCode_) {
            case ErrorCode::UNREACHABLE_NODES:
                return "Some waypoints are unreachable with the selected vehicle profile. "
                       "Try a different profile or check the waypoints.";
            
            case ErrorCode::INVALID_NODES:
                return "Some waypoints do not exist on the map. "
                       "Check that the node IDs are correct.";
            
            case ErrorCode::INSUFFICIENT_NODES:
                return "At least 2 waypoints are required to calculate a route.";
            
            case ErrorCode::MATRIX_CONSTRUCTION_FAILED:
                return "Error calculating distances between waypoints. "
                       "Check graph connectivity.";
            
            case ErrorCode::NO_VALID_SOLUTION:
                return "Cannot construct a valid tour with the given waypoints. "
                       "Some nodes may be disconnected.";
            
            case ErrorCode::TIMEOUT:
                return "Calculation exceeded the time limit. "
                       "Try fewer waypoints or a smaller area.";
            
            case ErrorCode::INVALID_PROFILE:
                return "The specified vehicle profile is not valid.";
            
            default:
                return "Unknown error in TSP calculation.";
        }
    }

    /**
     * @brief Gets recovery suggestions
     */
    std::vector<std::string> getRecoverySuggestions() const {
        std::vector<std::string> suggestions;
        
        switch (errorCode_) {
            case ErrorCode::UNREACHABLE_NODES:
                suggestions.push_back("Use vehicle profile 'no restrictions'");
                suggestions.push_back("Remove problematic waypoints ");
                if (!problematicNodes_.empty()) {
                    suggestions.push_back("Problematic nodes: " + formatNodeList());
                }
                break;
            
            case ErrorCode::INVALID_NODES:
                if (!problematicNodes_.empty()) {
                    suggestions.push_back("Invalid nodes: " + formatNodeList());
                }
                suggestions.push_back("Check node IDs on the map");
                break;
            
            case ErrorCode::INSUFFICIENT_NODES:
                suggestions.push_back("Add more waypoints to the tour");
                break;
            
            case ErrorCode::NO_VALID_SOLUTION:
                suggestions.push_back("Verify that all waypoints are in the same connected region");
                suggestions.push_back("Try with closer waypoints");
                break;
            
            default:
                break;
        }
        
        return suggestions;
    }

private:
    ErrorCode errorCode_;
    std::vector<int64_t> problematicNodes_;

    std::string formatNodeList() const {
        std::string result;
        for (size_t i = 0; i < problematicNodes_.size() && i < 5; i++) {
            if (i > 0) result += ", ";
            result += std::to_string(problematicNodes_[i]);
        }
        if (problematicNodes_.size() > 5) {
            result += "... (+" + std::to_string(problematicNodes_.size() - 5) + " more)";
        }
        return result;
    }
};