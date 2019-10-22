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

constexpr int workersPerSide = 256;//4x32 would be nice
constexpr int workSize = 16;
constexpr int dataWidth = workSize * workersPerSide;
constexpr int numberOfStateVariables = 5;
constexpr int byteSizeOfData = numberOfStateVariables * dataWidth * dataWidth * sizeof (number);

//the area which we want to calculate
constexpr int fromWorkerX = 0;
constexpr int untilWorkerX = 1;
constexpr int fromWorkerY = 128;
constexpr int untilWorkerY = 256;

constexpr int workerWidth = untilWorkerX - fromWorkerX;
constexpr int workerHeight = untilWorkerY - fromWorkerY;
constexpr int numberOfAllWorkers = workerWidth * workerHeight;

constexpr int simulationTime = 10;
constexpr int stepsPerSecond = 1024;
constexpr int stepsPerKernel = 64;
constexpr int CPUStepsPerSecond = stepsPerSecond / stepsPerKernel;
constexpr number timeStep = 1.0 / stepsPerSecond;

inline int getEventIndex (int x, int y)
{
	return (x - fromWorkerX) * workerHeight + (y - fromWorkerY);
}

std::vector<cl::Event> loadParamsToGPU (const cl::CommandQueue& queue, const cl::Kernel& setupKernel, cl::Buffer& statesBuffer)
{
	_setArgs (setupKernel, statesBuffer, dataWidth);

	std::vector<cl::Event> events (numberOfAllWorkers);

	for (int x = fromWorkerX; x < untilWorkerX; x++)
		for (int y = fromWorkerY; y < untilWorkerY; y++)
			queue.enqueueNDRangeKernel (setupKernel, cl::NDRange (x*workSize, y*workSize), cl::NDRange (workSize, workSize), cl::NDRange (workSize, workSize), NULL, &events[getEventIndex (x, y)]);

	return events;
}

void writeHeader (std::ostream& file)
{
	file << "#" <<
		"workersPerSide" << "\t" <<
		"workSize" << "\t" <<
		"fromWorkerX" << "\t" <<
		"untilWorkerX" << "\t" <<
		"fromWorkerY" << "\t" <<
		"untilWorkerY" << "\t" <<
		"simulationTime" << "\t" <<
		"stepsPerSecond" << "\t" <<
		"stepsPerKernel" << "\t" <<
		"precision" << "\n";

	file << "#" <<
		workersPerSide << "\t" <<
		workSize << "\t" <<
		fromWorkerX << "\t" <<
		untilWorkerX << "\t" <<
		fromWorkerY << "\t" <<
		untilWorkerY << "\t" <<
		simulationTime << "\t" <<
		stepsPerSecond << "\t" <<
		stepsPerKernel << "\t" <<
		typeid(number).name () << "\n";

}

void saveDataToFile (const std::vector<number>& data, std::string filename = "out.dat")
{
	std::ofstream file (filename, std::ios::out);

	writeHeader (file);

	for (int i = 0; i < dataWidth; i++) {
		for (int j = 0; j < dataWidth; j++)
			file << data[(i * dataWidth + j) * numberOfStateVariables] << "\t";
		file << "\n";
	}

	file.close ();
}

void saveDataToFileSparse (const std::vector<number>& data, std::string filename = "out.dat")
{
	std::ofstream file (filename, std::ios::out);

	writeHeader (file);

	for (int i = fromWorkerX * workSize; i < untilWorkerX * workSize; i++)
		for (int j = fromWorkerY * workSize; j < untilWorkerY * workSize; j++)
			file << i << "\t" << j << "\t" << data[(j * dataWidth + i) * numberOfStateVariables] << "\n";

	file.close ();
}


int main (int argc, char** args)
{
	std::vector<number> states_vec (numberOfStateVariables * dataWidth * dataWidth);

	try {
		//OpenCL cuccok inicializálása
		CLO clObject = CLO::fromFile ("prog.cl");

		std::string platformName;
		clObject.getPlatform ().getInfo (CL_PLATFORM_NAME, &platformName);
		std::cout << "Using platform " << platformName << std::endl;

		auto kernels = clObject.getKernels ({ "prog", "load" });

		auto deviceCommandQueuePairs = clObject.getStuff ();
		cl::CommandQueue& queue = deviceCommandQueuePairs[0].second;
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

				for (int x = fromWorkerX; x < untilWorkerX; x++)
					for (int y = fromWorkerY; y < untilWorkerY; y++)
						queue.enqueueNDRangeKernel (simulationKernel, cl::NDRange (x*workSize, y*workSize), cl::NDRange (workSize, workSize), cl::NDRange (workSize, workSize), &previousEvents, &currentEvents[getEventIndex (x, y)]);

				previousEvents = std::move (currentEvents);
				//Let the GPU do something else for a short time (like draw windows..)
				std::this_thread::sleep_for (std::chrono::microseconds (10));
			}
		}

		std::cout << "Waiting for calculations to end..." << std::endl;

		queue.finish ();

		std::cout << "Reading data from buffer..." << std::endl;

		//Read out the results
		queue.enqueueReadBuffer (statesBuffer, CL_TRUE, 0, byteSizeOfData, &states_vec[0]);

		std::cout << "Total simulation time: " << std::chrono::duration_cast<std::chrono::seconds> (std::chrono::high_resolution_clock::now () - startTime).count () << " s" << std::endl;

		std::cout << "Saving results..." << std::endl;
		saveDataToFileSparse (states_vec, "outfile.dat");
	}
	catch (const cl::Error& err)
	{
		std::cerr
			<< "ERROR in main:\n"
			<< err.what ()
			<< "(" << err.err () << ")\n"
			<< error_map[err.err ()]
			<< std::endl;
	}
	catch (const std::runtime_error& err)
	{
		std::cerr << "Runtime error:\n" << err.what () << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "\n:(\n";
	}
	return 0;
}