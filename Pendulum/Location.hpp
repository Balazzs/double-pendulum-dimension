#pragma once

#include <vector>
#include <array>

using loc_num = unsigned int;

class Location {
	friend struct std::hash<Location>;

private:
	loc_num x;
	loc_num y;
	unsigned int depth;

public:
	Location (loc_num depth, loc_num x, loc_num y) :
		x (x),
		y (y),
		depth (depth)
	{ }

	std::array<double, 2> GetRealCoord (double width, double height) const
	{
		const double ratio = std::pow (2, -(signed) depth);
		return { width * x * ratio, height * y * ratio };
	}

	std::vector<Location> GetUpperLevels () const
	{
		if (depth == 0)
			return {};

		Location upperLevel (depth - 1, x >> 1, y >> 1);

		std::vector<Location> levels = upperLevel.GetUpperLevels ();
		levels.push_back (upperLevel);

		return levels;
	}

	std::vector<Location> GetNeighbourhood (loc_num radius) const
	{
		std::vector<Location> neighbours;
		for (loc_num x_ = std::max(x - radius, 0u); x_ < std::min(x + radius, (loc_num) std::pow(2, depth)); x_++)
			for (loc_num y_ = std::max (y - radius, 0u); y_ < std::min (y + radius, (loc_num)std::pow (2, depth)); y_++)
			{
				neighbours.push_back (Location (depth, x, y));
			}
		return neighbours;
	}

	template <typename F>
	void ForEachSubLocation (F& func) const
	{
		func (Location (depth + 1, x << 1, y << 1));
		func (Location (depth + 1, x << 1 + 1, y << 1));
		func (Location (depth + 1, x << 1, y << 1 + 1));
		func (Location (depth + 1, x << 1 + 1, y << 1 + 1));
	}

	bool operator== (const Location& other) const
	{
		return x == other.x && y == other.y && depth == other.depth;
	}
};

namespace std {

	template <>
	struct hash<Location>
	{
		std::size_t operator()(const Location& p) const
		{
			using std::hash;

			return ( ( hash<loc_num> ()(p.x)
					   ^ (hash<loc_num> () (p.y) << 1)) >> 1 )
				     ^ (hash<loc_num> ()(p.depth) << 1 );
		}
	};

}