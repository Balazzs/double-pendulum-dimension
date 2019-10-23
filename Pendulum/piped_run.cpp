#include <iostream>
#include <sstream>

#include "SimQueue.hpp"

std::vector<Position> readPoints ()
{
	std::vector<Position> locations;

	std::string line;
	
	std::getline (std::cin, line);
	std::istringstream str (line);
	
	int numLines = -1;

	str >> numLines;
	locations.reserve (numLines);

	for (int n = 0; n < numLines; n++) {
		std::getline (std::cin, line);
		std::istringstream str (line);

		number x, y;
		str >> x >> y;

		locations.push_back ({x, y});
	}

	return locations;
}

void writeResults (std::vector<number> results)
{
	std::cout << results.size () << std::endl;

	for (const number& result : results) {
		std::cout << result << std::endl;
	}
}

int main () {
	int gen = 0;

	SimQueue* simQueuePtr;
	SimQueue simQueue ([&](const std::vector<number>& results)
	{
		writeResults (results);
		
		const std::vector<Position> newPoints = readPoints ();

		if (!newPoints.empty ()) {
			simQueuePtr->Schedule (newPoints);
		}
	});
	
	simQueuePtr = &simQueue;

	simQueue.Schedule (readPoints ());
}