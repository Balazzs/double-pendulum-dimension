#pragma once

#include <vector>

using loc_num = unsigned int;

class Location {
	friend struct std::hash<Location>;

private:
	loc_num x;
	loc_num y;
	loc_num depth;

public:
	Location (loc_num depth, loc_num x, loc_num y) :
		x (x),
		y (y),
		depth (depth)
	{ }

	template <typename F>
	void ForEachSubLocation (F& func)
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