#pragma once

#include "CommonDefinitions.hpp"
#include "Location.hpp"

class DataCollector {
private:
	std::vector<std::pair<Location, number>> results;

public:

	void AddData (std::vector<std::pair<Location, number>> newResults);

	void SaveData (std::ostream& stream);
	void SaveData (const std::string& filename);
};