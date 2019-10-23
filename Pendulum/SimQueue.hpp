#pragma once

#include <string>
#include <functional>

#include "CommonDefinitions.hpp"
#include "CLO.hpp"

using Position = std::array<number, 2>;

class SimQueue {
private:
	CallbackFunction	callback;

	CLO					clObject;
	cl::Kernel			simulationKernel;
	cl::CommandQueue	commandQueue;
	
	cl::Event 				LoadBatchParams (const std::vector<Position>&	simPoints,
											 const cl::Buffer&				buffer);

	std::vector<cl::Event>	ScheduleBatch   (const std::vector<Position>&	simPoints,
											 const cl::Buffer&				buffer,
											 const std::vector<cl::Event>&	loadEvents);

	std::vector<number>		ReadBatchResult (const std::vector<Position>&	simPoints,
											 const cl::Buffer&				buffer,
											 const std::vector<cl::Event>&	runEvents);

public:
	SimQueue (const CallbackFunction& callback);

	std::string					GetPlatformName () const;

	void						Schedule (const std::vector<Position>& simPoints);
};
