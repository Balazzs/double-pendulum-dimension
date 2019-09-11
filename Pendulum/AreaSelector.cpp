#include "AreaSelector.hpp"
#include <algorithm>

AreaSelector::AreaSelector (int maximumDepth /*= 20*/) :
	maximumDepth (maximumDepth)
{ }


bool AreaSelector::IsAlreadyProcessed (const Location& p) const
{
	return scheduledMeasurements.find (p) == scheduledMeasurements.end ();
}

bool AreaSelector::IsMeasurementPointGood (const Location& /*p*/, const number& value) const
{
	//TODO hard coded
	return value > 9.99 && value < 10.01;
}

std::vector<Location> AreaSelector::ScheduleNeighbourhoodForPoint (const Location & p)
{
	//TODO
	return {};
}

static hash_set<Location> GetAllLocationsAtDepthLevel (unsigned int depth)
{
	hash_set<Location> locations;

	for (unsigned int x = 0; x < 2 << depth; x++)
		for (unsigned int y = 0; y < 2 << depth; y++)
			locations.insert (Location (depth, x, y));

	return locations;
}

std::vector<Location> AreaSelector::GetInitialBatch (int initialDepth /*= 5*/)
{
	scheduledMeasurements = GetAllLocationsAtDepthLevel (initialDepth);

	return std::vector<Location> (scheduledMeasurements.begin (),
								  scheduledMeasurements.end ());
}

std::vector<Location> AreaSelector::GetNextBatch (const std::vector<std::pair<Location, number>>& newResults)
{
	std::vector<Location> newSchedules;

	for (auto const&[loc, val] : newResults)
	{
		if (IsMeasurementPointGood (loc, val)) {
			std::vector<Location> neighbours = ScheduleNeighbourhoodForPoint (loc);

			newSchedules.insert (newSchedules.end(),
								 std::move_iterator(neighbours.begin ()),
								 std::move_iterator (neighbours.end ()));
		}
	}

	return newSchedules;
}