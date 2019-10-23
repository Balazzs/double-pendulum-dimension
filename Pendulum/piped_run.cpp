#include <iostream>
#include <sstream>

#include "SimQueue.hpp"
#include "DataCollector.hpp"

std::vector<Location> readPoints ()
{
	std::vector<Location> locations;

	std::string line;
	
	std::getline (std::cin, line);
	std::istringstream str (line);
	
	int numLines = -1;

	str >> numLines;
	locations.reserve (numLines);

	for (int n = 0; n < numLines; n++) {
		std::getline (std::cin, line);
		std::istringstream str (line);

		int x, y;
		uint depth;
		str >> depth >> x >> y;

		locations.push_back (Location (depth, x, y));
	}

	return locations;
}

void writeResults (std::vector<Measurement> results)
{
	std::cout << results.size () << std::endl;

	for (const Measurement& measurement : results) {
		std::cout << measurement.second << std::endl;
	}
}

int main () {
	DataCollector dataColl;

	int gen = 0;

	SimQueue* simQueuePtr;
	SimQueue simQueue ([&](const std::vector<Measurement>& results)
	{
		dataColl.AddData (results);

		writeResults (results);
		
		const std::vector<Location> newPoints = readPoints ();

		if (!newPoints.empty ()) {
			simQueuePtr->Schedule (newPoints);
		}
	});
	
	simQueuePtr = &simQueue;

	simQueue.Schedule (readPoints ());

	dataColl.SaveData ("test.txt");
}