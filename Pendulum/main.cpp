#include "CLO.hpp"
#include "header.hpp"
#include <initializer_list>
#include <memory>
#include <algorithm>
#include <exception>
#include <time.h>
#include <fstream>
#include <limits>
#include <chrono>
#include <thread>
#include <iomanip>

typedef cl_float number;

constexpr int globalSizeMultiplier = 4;
constexpr int workSize = 8;
constexpr int dataWidth = workSize * workSize * globalSizeMultiplier;
constexpr int numberOfStateVariables = 5;
constexpr int byteSizeOfData = numberOfStateVariables * dataWidth * dataWidth * sizeof (number);

constexpr int workersPerSide = workSize * globalSizeMultiplier;
constexpr int numberOfAllWorkers = workersPerSide * workersPerSide / 2;

constexpr int simulationTime = 1;
constexpr int stepsPerSecond = 1024;
constexpr int stepsPerKernel = 64;
constexpr int CPUStepsPerSecond = stepsPerSecond / stepsPerKernel;
constexpr number timeStep = 1.0 / stepsPerSecond;

std::vector<cl::Event> loadParamsToGPU (const cl::CommandQueue& queue, const cl::Kernel& setupKernel, cl::Buffer& statesBuffer)
{
	_setArgs (setupKernel, statesBuffer, dataWidth);
	
	std::vector<cl::Event> events (numberOfAllWorkers);

	for (int x = 0; x < workersPerSide / 2; x++)
		for (int y = 0; y < workersPerSide; y++)
			queue.enqueueNDRangeKernel (setupKernel, cl::NDRange (x*workSize, y*workSize), cl::NDRange (workSize, workSize), cl::NDRange (workSize, workSize), NULL, &events[x * workersPerSide + y]);

	return events;
}

void saveDataToFile (const std::vector<number>& data, std::string filename = "out.dat")
{
	std::ofstream file (filename, std::ios::out);

	for (int i = 0; i < dataWidth; i++) {
		for (int j = 0; j < dataWidth; j++)
			file << data[(i*dataWidth + j) * numberOfStateVariables] << "\t";
		file << std::endl;
	}

	file.close ();
}

int main(int argc, char** args)
{
	std::vector<number> states_vec(numberOfStateVariables * dataWidth * dataWidth);

	try{
		//OpenCL cuccok inicializálása
		CLO clObject = CLO::fromFile ("prog.cl");
	
		auto kernels = clObject.getKernels ({"prog", "load"});
	
		const cl::CommandQueue& queue = clObject.getStuff ()[0].second;
		const cl::Kernel& simulationKernel = kernels[0];
		const cl::Kernel& setupKernel = kernels[1];
		const auto& context = clObject.getContext ();

		auto startTime = std::chrono::high_resolution_clock::now ();

		//Bufferek
		cl::Buffer statesBuffer (context, CL_MEM_READ_WRITE, byteSizeOfData);
	
		//Kernel argumentumok beállítása
		_setArgs (simulationKernel, statesBuffer, dataWidth, timeStep, stepsPerKernel);
		
		std::vector<cl::Event> previousEvents;
		previousEvents = loadParamsToGPU (queue, setupKernel, statesBuffer);
		
		for (int time = 0; time < simulationTime; time++) 
		{
			std::cout << time << std::endl;

			for (int i = 0; i < CPUStepsPerSecond; i++)
			{
				std::vector<cl::Event> currentEvents (numberOfAllWorkers);
				
				for (int x = 0; x < workersPerSide / 2; x++)
					for (int y = 0; y < workersPerSide; y++)
					queue.enqueueNDRangeKernel (simulationKernel, cl::NDRange(x*workSize, y*workSize), cl::NDRange(workSize, workSize), cl::NDRange(workSize, workSize), &previousEvents, &currentEvents[x * workersPerSide + y]);

				previousEvents = std::move (currentEvents);
				//Let the GPU do something else for a short time (like draw windows..)
				std::this_thread::sleep_for (std::chrono::microseconds(10));
			}
		}

		//Read out the results
		queue.enqueueReadBuffer (statesBuffer, CL_TRUE, 0, byteSizeOfData, &states_vec[0], &previousEvents);

		std::cout << std::chrono::duration_cast<std::chrono::seconds> (std::chrono::high_resolution_clock::now() - startTime).count () << " s" << std::endl;

		saveDataToFile (states_vec);
	}
	catch(cl::Error err)
	{
		std::cerr
		 << "ERROR in main:\n"
         << err.what ()
         << "(" << err.err () << ")\n"
		 << error_map[err.err ()]
         << std::endl;
	}
	catch(std::runtime_error err)
	{
		std::cerr << "Runtime error:\n" << err.what () << std::endl;
	}
	catch(std::exception e)
	{
		std::cerr << "\n:(\n";
	}
	return 0;
}