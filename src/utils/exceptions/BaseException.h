#pragma once

#include <exception>
#include <string>

/**
 * Base exception for the project
 */

class BaseException : public std::exception {
protected:
    std::string message;
    
public:
    explicit BaseException(const std::string& message) : message(message) {}

    const char* what() const noexcept override {
        return message.c_str();
    }

    const std::string& getMesage() const {
        return message;
    }
};