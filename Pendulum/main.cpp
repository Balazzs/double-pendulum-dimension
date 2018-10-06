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

constexpr int simulationTime = 1;
constexpr int stepsPerSecond = 1024;
constexpr int stepsPerKernel = 64;
constexpr int CPUStepsPerSecond = stepsPerSecond / stepsPerKernel;
constexpr number timeStep = 1.0 / stepsPerSecond;

int main(int argc, char** args)
{
	std::vector<number> states_vec(numberOfStateVariables * dataWidth * dataWidth);

	try{
		//OpenCL cuccok inicializálása
		CLO jani = CLO::fromFile("prog.cl");
	
		auto vec = jani.getKernels({"prog", "load"});
		auto stuff = jani.getStuff();
	
		const cl::CommandQueue& queue = stuff[0].second;
		const cl::Kernel& ker = vec[0];
		const cl::Kernel& ker2 = vec[1];
		const auto& context = jani.getContext();


		auto start = std::chrono::high_resolution_clock::now();

		//Bufferek
		cl::Buffer states(context, CL_MEM_READ_WRITE, byteSizeOfData);
	

		//Kernel argumentumok beállítása
		_setArgs(ker, states, dataWidth, timeStep, stepsPerKernel);
		_setArgs(ker2, states, dataWidth);


		std::vector<cl::Event> prev;
		cl::Event ev[workSize*workSize*globalSizeMultiplier*globalSizeMultiplier];

		//Load the params
		for (int x = 0; x < workersPerSide / 2; x++)
			for (int y = 0; y < workersPerSide; y++)
				queue.enqueueNDRangeKernel (ker2, cl::NDRange (x*workSize, y*workSize), cl::NDRange (workSize, workSize), cl::NDRange (workSize, workSize), NULL, &ev[x * workersPerSide + y]);


		for (int x = 0; x < workersPerSide / 2; x++)
			for (int y = 0; y < workersPerSide; y++)
				prev.push_back (ev[x * workersPerSide + y]);

		for (int seconds = 0; seconds < simulationTime; seconds++) 
		{
			std::cout << seconds << std::endl;

			for (int i = 0; i < CPUStepsPerSecond; i++)
			{
				//Vége a számolásnak event
				cl::Event ev[workSize*workSize*globalSizeMultiplier*globalSizeMultiplier];
				//Számol
				for (int x = 0; x < workersPerSide / 2; x++)
					for (int y = 0; y < workersPerSide; y++)
					queue.enqueueNDRangeKernel(ker, cl::NDRange(x*workSize, y*workSize), cl::NDRange(workSize, workSize), cl::NDRange(workSize, workSize), &prev, &ev[x * workersPerSide + y]);

				//Következő feltételek vektorba
				prev.clear();
				for (int x = 0; x < workersPerSide / 2; x++)
					for (int y = 0; y < workersPerSide; y++)
						prev.push_back(ev[x * workersPerSide + y]);
				std::this_thread::sleep_for(std::chrono::microseconds(10));
			}
		}
		//És kiolvas amint vége a számolásnak
		auto ev_vec = prev;
		queue.enqueueReadBuffer(states, CL_TRUE, 0, byteSizeOfData, &states_vec[0], &ev_vec);

		//Nézzük meg az egész GPU-s dolog mennyi CPU oldali időbe telt (ne rondítsunk bele a std out streambe, errorba inkább...)
		std::cerr << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() << " us" << std::endl;

		std::ofstream file("out.dat", std::ios::out);

		//Írjuk ki az eredményt
		for (int i = 0; i < dataWidth; i++) {
			for (int j = 0; j < dataWidth; j++)
				file << states_vec[(i*dataWidth + j) * numberOfStateVariables] << "\t";
			file << std::endl;
		}

		file.close();
	}
	//Ha valami gond lenne
	catch(cl::Error err)
	{
		std::cerr
		 << "ERROR in main:\n"
         << err.what()
         << "(" << err.err() << ")\n"
		 << error_map[err.err()]
         << std::endl;
	}
	catch(std::runtime_error err)
	{
		std::cerr << "Runtime error:\n" << err.what() << std::endl;
	}
	catch(std::exception e)
	{
		std::cerr << "\n:(\n";
	}
	return 0;
}