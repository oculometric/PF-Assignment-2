#pragma once

using namespace std;

// simple int vector structure
struct ivector2
{
    // coordinates
	int x, y;

    // comparison operator for using ivector2 in STL classes (map, set, etc)
    bool operator<(const ivector2& rhs) const
    {
        if (x < rhs.x) return  true;
        if (x == rhs.x && y < rhs.y) return true;

        return false;
    }
};

// int operators
inline ivector2 operator*(ivector2 a, int i) { return ivector2{ a.x * i, a.y * i }; }
inline ivector2 operator/(ivector2 a, int i) { return ivector2{a.x / i, a.y / i}; }

// ivector2 operators
inline ivector2 operator*(ivector2 a, ivector2 b) { return ivector2{ a.x * b.x, a.y * b.y }; }
inline ivector2 operator/(ivector2 a, ivector2 b) { return ivector2{ a.x / b.x, a.y / b.y }; }
inline ivector2 operator+(ivector2 a, ivector2 b) { return ivector2{ a.x + b.x, a.y + b.y }; }
inline ivector2 operator-(ivector2 a, ivector2 b) { return ivector2{ a.x - b.x, a.y - b.y }; }
inline ivector2 operator%(ivector2 a, ivector2 b) { return ivector2{ a.x % b.x, a.y % b.y }; }

// absolute values of x and y
inline ivector2 abs(ivector2 a) { return ivector2{ abs(a.x), abs(a.y) }; }

// square magnitude of ivector2
inline int magnitude_squared(ivector2 a) { return (a.x * a.x) + (a.y * a.y); }

// clamp ivector2 between and upper and lower bound in both axes
inline ivector2 clamp(ivector2 c, ivector2 vmax, ivector2 vmin) { return ivector2{ min(max(c.x, vmin.x), vmax.x), min(max(c.y, vmin.y), vmax.y) }; }