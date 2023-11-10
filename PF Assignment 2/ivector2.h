#pragma once

using namespace std;

struct ivector2
{
	int x, y;

    bool operator()(const ivector2& lhs, const ivector2& rhs)
    {
        if (lhs.x < rhs.x) return  true;
        if (lhs.x == rhs.x && lhs.y < rhs.y) return true;

        return false;
    }
};

inline ivector2 operator* (ivector2 a, int i) { return ivector2{ a.x * i, a.y * i }; }
inline ivector2 operator/ (ivector2 a, int i) { return ivector2{a.x / i, a.y / i}; }

inline ivector2 operator* (ivector2 a, ivector2 b) { return ivector2{ a.x * b.x, a.y * b.y }; }
inline ivector2 operator/ (ivector2 a, ivector2 b) { return ivector2{ a.x / b.x, a.y / b.y }; }
inline ivector2 operator+ (ivector2 a, ivector2 b) { return ivector2{ a.x + b.x, a.y + b.y }; }
inline ivector2 operator- (ivector2 a, ivector2 b) { return ivector2{ a.x - b.x, a.y - b.y }; }

inline int magnitude_squared (ivector2 a) { return (a.x * a.x) + (a.y * a.y); }
inline ivector2 clamp(ivector2 c, ivector2 vmax, ivector2 vmin) { return ivector2{ min(max(c.x, vmin.x), vmax.x), min(max(c.y, vmin.y), vmax.y) }; }