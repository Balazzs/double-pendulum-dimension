#pragma once

#define __CL_ENABLE_EXCEPTIONS

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#include <functional>

class Location;

using number = cl_float;
using CallbackFunction = std::function <void (const std::vector<number>&)>;
using Measurement = std::pair<Location, number>;

constexpr int workSize = 32*32;
constexpr int numberOfStateVariables = 5;

constexpr int simulationTime = 20;
constexpr int stepsPerSecond = 1024;
constexpr int stepsPerKernel = 64;
constexpr int CPUStepsPerSecond = stepsPerSecond / stepsPerKernel;
constexpr number timeStep = 1.0 / stepsPerSecond;

constexpr double paramWidth = 6.28, paramHeight = 6.28;