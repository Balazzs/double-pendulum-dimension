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

int main(int argc, char** args)
{
	const int W = 30, N = W*W, NN = 5;

	std::vector<cl_double> states_vec(NN * N * N);

	try{
	//OpenCL cuccok inicializálása
	CLO jani = CLO::fromFile("prog.cl", false);
	
	auto vec = jani.getKernels({"prog", "load"});
	auto stuff = jani.getStuff();
	
	const cl::CommandQueue& queue = stuff[0].second;
	const cl::Kernel& ker = vec[0];
	const cl::Kernel& ker2 = vec[1];
	const auto& context = jani.getContext();
	
	

	auto start = std::chrono::high_resolution_clock::now();

	//Bufferek
	//cl::Buffer output(context, CL_MEM_WRITE_ONLY, N * N * sizeof(cl_double));
	cl::Buffer states(context, CL_MEM_READ_WRITE, NN * N * N * sizeof(cl_double));
	//cl::LocalSpaceArg state = cl::Local(N * N * sizeof(cl_double));




	//Kernel argumentumok beállítása
	_setArgs(ker, states, N, 1E-3, 50);
	_setArgs(ker2, states, N);

	//Load the params
	cl::Event eventt;
	queue.enqueueNDRangeKernel(ker2, 0, cl::NDRange(N, N), cl::NDRange(W, W), NULL, &eventt);

	std::vector<cl::Event> prev({eventt});
	for (int secs = 0; secs < 1000; secs++)
	{
		std::cout << secs << std::endl;

		for (int i = 0; i < 20; i++)
		{
			//Vége a számolásnak event
			cl::Event ev[W];
			//Számol
			for (int j = 0; j < W; j++)
				queue.enqueueNDRangeKernel(ker, cl::NDRange(j*W, 0), cl::NDRange(W, N), cl::NDRange(W, W), &prev, &ev[j]);

			//Következő feltételek vektorba
			prev.clear();
			for (int j = 0; j < W; j++)
				prev.push_back(ev[j]);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		
		//Írjuk ki az eredményt néha
		if (secs % 10 == 0)
		{
			queue.enqueueReadBuffer(states, CL_TRUE, 0, NN * N * N * sizeof(cl_double), &states_vec[0], &prev);

			std::ofstream file("temp.dat", std::ios::out);
			for (int i = 0; i < N; i++)
			{
				for (int j = 0; j < N; j++)
					for (int n = 0; n < NN; n++)
						file << states_vec[(i*N + j)*NN + n] << "\t";
				file << std::endl;
			}

			file.close();
		}
	}
	//És kiolvas amint vége a számolásnak
	auto ev_vec = prev;
	queue.enqueueReadBuffer(states, CL_TRUE, 0, NN * N * N * sizeof(cl_double), &states_vec[0], &ev_vec);

	//Nézzük meg az egész GPU-s dolog mennyi CPU oldali időbe telt (ne rondítsunk bele a std out streambe, errorba inkább...)
	std::cerr << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count() << " us" << std::endl;

	std::ofstream file("out.dat", std::ios::out);

	//Írjuk ki az eredményt
	for (int i = 0; i < N; i++)
	{
		for (int j = 0; j < N; j++)
		{
			file << states_vec[(i*N + j)*NN] << "\t";
		}
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