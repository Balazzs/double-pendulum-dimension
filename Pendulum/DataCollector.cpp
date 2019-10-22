#include "DataCollector.hpp"

#include <fstream>
#include <iostream>

void DataCollector::AddData (std::vector<std::pair<Location, number>> newResults)
{
	results.insert (results.end (), newResults.begin (), newResults.end ());
}


void WriteHeader (std::ostream& file)
{
	file << "#" <<
		"workSize" << "\t" <<
		"simulationTime" << "\t" <<
		"stepsPerSecond" << "\t" <<
		"stepsPerKernel" << "\t" <<
		"precision" << "\n";

	file << "#" <<
		workSize << "\t" <<
		simulationTime << "\t" <<
		stepsPerSecond << "\t" <<
		stepsPerKernel << "\t" <<
		typeid(number).name () << "\n";
}

void DataCollector::SaveData (std::ostream& stream)
{
	WriteHeader (stream);

	for (auto const& [loc, val] : results) {
		std::array<double, 2> coords = loc.GetRealCoord (paramWidth, paramHeight);
		stream << coords[0] << "\t" << coords[1] << "\t" << val << "\n";
	}
}

void DataCollector::SaveData (const std::string & filename)
{
	std::ofstream fileStream (filename);

	if (!fileStream) {
		std::cerr << "Error opening file \"" << filename << "\" for writing " << std::endl;
	}

	SaveData (fileStream);

	fileStream.close ();
}
