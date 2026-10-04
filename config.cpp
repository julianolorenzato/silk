#include "config.h"

#include <string>
#include <vector>

#include "cxxopts.hpp"

Config::Config(const int argc, const char** argv) {
    cxxopts::Options options("silk", "Software Renderer");

    options.add_options()("mp,modelpath", "Model path", cxxopts::value<std::string>());

    auto res = options.parse(argc, argv);

    this->modelpath = res["modelpath"].as<std::string>();
}

const fs::path Config::getModelpath() const { return this->modelpath; }
