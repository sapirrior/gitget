#pragma once

#include <stdexcept>
#include <string>

namespace gitget {

class Error : public std::runtime_error {
public:
    explicit Error(const std::string& message) : std::runtime_error(message) {}
};

class ArgumentError : public Error {
public:
    explicit ArgumentError(const std::string& message) : Error(message) {}
};

class NetworkError : public Error {
public:
    explicit NetworkError(const std::string& message) : Error(message) {}
};

class HttpError : public Error {
public:
    HttpError(long status_code, const std::string& message)
        : Error(message), status_code_(status_code) {}

    [[nodiscard]] long status_code() const noexcept {
        return status_code_;
    }

private:
    long status_code_;
};

class FileSystemError : public Error {
public:
    explicit FileSystemError(const std::string& message) : Error(message) {}
};

} // namespace gitget
