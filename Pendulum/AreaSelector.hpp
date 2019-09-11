#pragma once
#include <unordered_set>

template <typename T>
using hash_set = std::unordered_set<T>;


#include "Location.hpp"

using number = float;

class AreaSelector {
private:
	const int					maximumDepth;			//The maximum depth allowed

	hash_set<Location>			scheduledMeasurements;	//Measurements already scheduled

	bool IsAlreadyProcessed (const Location& p) const;
	bool IsMeasurementPointGood (const Location& p, const number& value) const;

	std::vector<Location> ScheduleNeighbourhoodForPoint (const Location& p);

public:
	
	AreaSelector (int maximumDepth = 20);

	std::vector<Location> GetInitialBatch (int initialDepth = 5);
	std::vector<Location> GetNextBatch (const std::vector<std::pair<Location, number>>& newResults);
};