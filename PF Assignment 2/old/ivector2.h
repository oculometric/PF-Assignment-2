#ifndef IVECTOR2_H
#define IVECTOR2_H

struct ivector2
{
	int x, y;
};

inline ivector2 operator* (ivector2 a, int i) { return ivector2{ a.x * i, a.y * i }; }
inline ivector2 operator/ (ivector2 a, int i) { return ivector2{a.x / i, a.y / i}; }

inline ivector2 operator* (ivector2 a, ivector2 b) { return ivector2{ a.x * b.x, a.y * b.y }; }
inline ivector2 operator/ (ivector2 a, ivector2 b) { return ivector2{ a.x / b.x, a.y / b.y }; }
inline ivector2 operator+ (ivector2 a, ivector2 b) { return ivector2{ a.x + b.x, a.y + b.y }; }
inline ivector2 operator- (ivector2 a, ivector2 b) { return ivector2{ a.x - b.x, a.y - b.y }; }

inline int magnitude_squared (ivector2 a) { return (a.x * a.x) + (a.y * a.y); }

#endif