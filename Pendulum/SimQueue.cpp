#include "SimQueue.hpp"

#include "header.hpp"
#include <initializer_list>
#include <algorithm>
#include <exception>
#include <thread>
#include <chrono>

SimQueue::SimQueue (const CallbackFunction& callback) :
	callback	(callback),
	clObject	(CLO::fromFile ("prog.cl"))
{
	auto openCLKernels = clObject.getKernels ({ "prog"});
	if (openCLKernels.size () < 1) {
		std::cerr << "Simulation kernel not found :(";
		return;
	}
	simulationKernel = openCLKernels[0];

	auto deviceCommandQueuePairs = clObject.getStuff ();
	if (deviceCommandQueuePairs.size () == 0) {
		std::cerr << "Device and CommandQueue not found :(";
		return;
	}
	commandQueue = std::move (deviceCommandQueuePairs[0].second);
}

std::string SimQueue::GetPlatformName () const
{
	std::string platformName;
	clObject.getPlatform ().getInfo (CL_PLATFORM_NAME, &platformName);
	return platformName;
}

cl::Event SimQueue::LoadBatchParams (const std::vector<Location>&	simPoints,
									 const cl::Buffer&				buffer)
{
	try {
		cl::Event loadEvent;
		const size_t sizeOfBatch = simPoints.size () * numberOfStateVariables * sizeof (number);

		std::vector<number> data;
		data.reserve (simPoints.size () * numberOfStateVariables);

		for (const Location& p : simPoints) {
			std::array<double, 2> angles = p.GetRealCoord (paramWidth, paramHeight);

			data.push_back (0);
			data.push_back ((number) (angles[0] - paramWidth / 2.));
			data.push_back (0);
			data.push_back ((number) (angles[1] - paramHeight / 2.));
			data.push_back (0);
		}

		commandQueue.enqueueWriteBuffer (buffer, CL_BLOCKING, 0, sizeOfBatch, &data[0], nullptr, &loadEvent);

		return loadEvent;
	}
	catch (cl::Error err)
	{
		std::cerr
			<< "ERROR:\n"
			<< err.what ()
			<< "(" << err.err () << ")\n"
			<< error_map[err.err ()]
			<< std::endl;

		return cl::Event();
	}
}

std::vector<cl::Event> SimQueue::ScheduleBatch (const std::vector<Location>&	simPoints,
												const cl::Buffer&				buffer, 
												const std::vector<cl::Event>&	loadEvents)
{
	try {
		//Setting kernel arguments
		_setArgs (simulationKernel, buffer, timeStep, stepsPerKernel);

		std::vector<cl::Event> previousEvents;
		
		for (int time = 0; time < simulationTime; time++)
		{
			for (int i = 0; i < CPUStepsPerSecond; i++)
			{
				const size_t numberOfWorkers = simPoints.size () / workSize;

				std::vector<cl::Event> currentEvents (numberOfWorkers);

				for (int ind = 0; ind < numberOfWorkers; ind++)
					commandQueue.enqueueNDRangeKernel (simulationKernel, cl::NDRange (ind * workSize), cl::NDRange (workSize), cl::NDRange (workSize), &previousEvents, &currentEvents[ind]);

				previousEvents = std::move (currentEvents);
				//Lets do something else for a short time (like draw windows..)
				std::this_thread::sleep_for (std::chrono::microseconds (10));
			}
		}

		return previousEvents;
	}
	catch (cl::Error err)
	{
		std::cerr
			<< "ERROR:\n"
			<< err.what ()
			<< "(" << err.err () << ")\n"
			<< error_map[err.err ()]
			<< std::endl;
	}
	catch (std::runtime_error err)
	{
		std::cerr << "Runtime error:\n" << err.what () << std::endl;
	}
	catch (std::exception e)
	{
		std::cerr << "\n:(\n";
	}
	return {};
}

std::vector<Measurement> SimQueue::ReadBatchResult (const std::vector<Location>&	simPoints,
													const cl::Buffer&				buffer,
													const std::vector<cl::Event>&	runEvents)
{
	try {
		const size_t sizeOfBatch = simPoints.size () * numberOfStateVariables * sizeof (number);

		std::vector<Measurement> measurements;
		measurements.reserve (simPoints.size ());
		
		std::vector<number> data (simPoints.size () * numberOfStateVariables, (number) 0);
		commandQueue.enqueueReadBuffer (buffer, CL_BLOCKING, 0, sizeOfBatch, &data[0], &runEvents);

		size_t ind = 0;
		for (const Location& p : simPoints) {
			measurements.push_back ({ p, data[ind] });
			ind += numberOfStateVariables;
		}

		return measurements;
	}
	catch (cl::Error err)
	{
		std::cerr
			<< "ERROR:\n"
			<< err.what ()
			<< "(" << err.err () << ")\n"
			<< error_map[err.err ()]
			<< std::endl;

		return {};
	}
}

void SimQueue::Schedule (const std::vector<Location>& simPoints)
{
	if (simPoints.size () == 0)
		return;

	const size_t sizeOfBatch = simPoints.size () * numberOfStateVariables * sizeof(number);

	cl::Buffer buffer (clObject.getContext (), CL_MEM_READ_WRITE, sizeOfBatch);

	const std::vector<cl::Event> loadEvents = { LoadBatchParams (simPoints, buffer) };
	const std::vector<cl::Event> runEvents  = ScheduleBatch (simPoints, buffer, loadEvents);
	
	callback (ReadBatchResult (simPoints, buffer, runEvents));
}
