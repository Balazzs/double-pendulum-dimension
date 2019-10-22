#include <condition_variable>

#include "AreaSelector.hpp"
#include "SimQueue.hpp"
#include "DataCollector.hpp"


std::mutex m;
std::condition_variable cv;
bool ready = false;

int main () {
	AreaSelector selector;
	DataCollector dataColl;

	int gen = 0;

	SimQueue* simQueuePtr;
	SimQueue simQueue ([&](const std::vector<Measurement>& results)
	{
		dataColl.AddData (results);

		const std::vector<Location> newPoints = selector.GetNextBatch (results);

		gen++;
		if (gen == 5 || newPoints.empty ()) {
			std::lock_guard<std::mutex> lk (m);
			ready = true;
			cv.notify_one ();
		}
		else {
			std::cout << "Calculating gen " << gen << std::endl;
			simQueuePtr->Schedule (newPoints);
		}
	});
	
	simQueuePtr = &simQueue;

	simQueue.Schedule (selector.GetInitialBatch (7));

	std::unique_lock<std::mutex> lk (m);
	cv.wait (lk, [] {return ready; });

	dataColl.SaveData ("test.txt");
}