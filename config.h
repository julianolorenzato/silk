#pragma once

#include <filesystem>

namespace fs = std::filesystem;

struct Config {
   private:
    std::filesystem::path modelpath;

   public:
    explicit Config(const int argc, const char** argv);

    const fs::path getModelpath() const;
};
