#pragma once

#include <string>
#include <functional>

#include "CommonDefinitions.hpp"
#include "Location.hpp"
#include "CLO.hpp"


class SimQueue {
private:
	CallbackFunction	callback;

	CLO					clObject;
	cl::Kernel			simulationKernel;
	cl::CommandQueue	commandQueue;
	

	cl::Event					LoadBatchParams (const std::vector<Location>&	simPoints,
												 const cl::Buffer&				buffer);

	std::vector<cl::Event>		ScheduleBatch   (const std::vector<Location>&	simPoints,
												 const cl::Buffer&				buffer,
												 const std::vector<cl::Event>&	loadEvents);

	std::vector<Measurement>	ReadBatchResult (const std::vector<Location>&	simPoints,
												 const cl::Buffer&				buffer,
												 const std::vector<cl::Event>&	runEvents);

public:
	SimQueue (const CallbackFunction& callback);

	std::string					GetPlatformName () const;

	void						Schedule (const std::vector<Location>& simPoints);
};
