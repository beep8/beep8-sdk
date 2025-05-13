/**
 * @file submath.h
 * @brief Provides mathematical functions and utilities, primarily focused on fixed-point arithmetic.
 *
 * This module offers a set of mathematical functions and utilities, including trigonometric
 * calculations, fixed-point number operations, and random number generation using the
 * Mersenne Twister algorithm. It is particularly useful in resource-constrained environments,
 * such as embedded systems or game development.
 *
 * The module includes:
 * - Trigonometric functions using a 12-bit angle representation
 * - Random number generation within a specified range for fixed-point numbers
 * - Mathematical constants like M_PI, M_E, etc.
 * - Utility functions for fixed-point arithmetic
 * - A structure for representing rectangles with fixed-point coordinates
 *
 * Note: This module relies on fixed-point number representations using the `fx8` and `fx12` types.
 * 
 * Example usage:
 * @code
 * #include <submath.h>
 * #include <iostream>
 *
 * int main() {
 *     // Generate a random fixed-point number within a specified range
 *     fx8 min_value(1.0);
 *     fx8 max_value(10.0);
 *     fx8 random_value = genrand_min_max_fx8(min_value, max_value);
 *     std::cout << "Random value: " << random_value << std::endl;
 *
 *     // Calculate sine and cosine for a given angle
 *     u32 angle = 1024;  // 90 degrees
 *     fx8 radius(5.0);
 *     fx8 sin_result = rad_sin_12(radius, angle);
 *     fx8 cos_result = rad_cos_12(radius, angle);
 *     std::cout << "sin(90 degrees): " << sin_result << std::endl;
 *     std::cout << "cos(90 degrees): " << cos_result << std::endl;
 *
 *     return 0;
 * }
 * @endcode
 */

#pragma once
#include <submath.h>
#include <fixed.h>
#include <fxmath.h>
#include <b8/type.h>

/**
 * @brief Alias for fixed-point type with 8 fractional bits.
 *
 * This type represents a fixed-point number with 8 bits for the fractional part,
 * using a 32-bit integer for storage and a 64-bit integer for intermediate calculations.
 */
using fx8  = fpm::fixed<std::int32_t, std::int64_t, 8>;
inline std::string tostr(const fx8& val) {
  float float_value = static_cast< float >( val );
  return  std::to_string( float_value );
}

constexpr fx8 FX8_E         = fx8(696, 256);  // 2.718281828459045 (e)
constexpr fx8 FX8_LOG2E     = fx8(369, 256);  // 1.4426950408889634 (log2(e))
constexpr fx8 FX8_LOG10E    = fx8(111, 256);  // 0.4342944819032518 (log10(e))
constexpr fx8 FX8_LN2       = fx8(178, 256);  // 0.6931471805599453 (ln(2))
constexpr fx8 FX8_LN10      = fx8(590, 256);  // 2.3025850929940457 (ln(10))
constexpr fx8 FX8_1_PI      = fx8(82, 256);   // 0.3183098861837907 (1/π)
constexpr fx8 FX8_2_PI      = fx8(163, 256);  // 0.6366197723675814 (2/π)
constexpr fx8 FX8_2_SQRTPI  = fx8(289, 256);  // 1.1283791670955126 (2/√π)
constexpr fx8 FX8_SQRT2     = fx8(362, 256);  // 1.4142135623730951 (√2)
constexpr fx8 FX8_SQRT1_2   = fx8(181, 256);  // 0.7071067811865475 (1/√2)
constexpr fx8 FX8_2PI       = fx8(1608, 256); // 6.283185307179586 (2π)
constexpr fx8 FX8_PI        = fx8(804, 256);  // 3.141592653589793 (π)
constexpr fx8 FX8_PI_2      = fx8(402, 256);  // 1.5707963267948966 (π/2)
constexpr fx8 FX8_PI_3      = fx8(268, 256);  // 1.0471975511965976 (π/3)
constexpr fx8 FX8_PI_4      = fx8(201, 256);  // 0.7853981633974483 (π/4)
constexpr fx8 FX8_PI_5      = fx8(161, 256);  // 0.6283185307179586 (π/5)
constexpr fx8 FX8_PI_6      = fx8(134, 256);  // 0.5235987755982988 (π/6)
constexpr fx8 FX8_PI_7      = fx8(115, 256);  // 0.4487989505128276 (π/7)
constexpr fx8 FX8_PI_8      = fx8(100, 256);  // 0.39269908169872414 (π/8)
constexpr fx8 FX8_PI_9      = fx8(89, 256);   // 0.3490658503988659 (π/9)
constexpr fx8 FX8_PI_10     = fx8(80, 256);   // 0.3141592653589793 (π/10)
constexpr fx8 FX8_PI_11     = fx8(73, 256);   // 0.28559933214452665 (π/11)
constexpr fx8 FX8_PI_12     = fx8(67, 256);   // 0.2617993877991494 (π/12)
constexpr fx8 FX8_PI_13     = fx8(62, 256);   // 0.241660973353061 (π/13)
constexpr fx8 FX8_PI_14     = fx8(57, 256);   // 0.2243994752564138 (π/14)
constexpr fx8 FX8_PI_15     = fx8(54, 256);   // 0.20943951023931953 (π/15)
constexpr fx8 FX8_PI_16     = fx8(50, 256);   // 0.19634954084936207 (π/16)

/**
 * @brief Alias for fixed-point type with 12 fractional bits.
 *
 * This type represents a fixed-point number with 12 bits for the fractional part,
 * using a 32-bit integer for storage and a 64-bit integer for intermediate calculations.
 */
using fx12 = fpm::fixed<std::int32_t, std::int64_t, 12>;

constexpr fx12 FX12_E         = fx12(11135, 4096);  // 2.718281828459045 (e)
constexpr fx12 FX12_LOG2E     = fx12(5912, 4096);   // 1.4426950408889634 (log2(e))
constexpr fx12 FX12_LOG10E    = fx12(1785, 4096);   // 0.4342944819032518 (log10(e))
constexpr fx12 FX12_LN2       = fx12(2831, 4096);   // 0.6931471805599453 (ln(2))
constexpr fx12 FX12_LN10      = fx12(9438, 4096);   // 2.3025850929940457 (ln(10))
constexpr fx12 FX12_1_PI      = fx12(1317, 4096);   // 0.3183098861837907 (1/π)
constexpr fx12 FX12_2_PI      = fx12(2634, 4096);   // 0.6366197723675814 (2/π)
constexpr fx12 FX12_2_SQRTPI  = fx12(4613, 4096);   // 1.1283791670955126 (2/√π)
constexpr fx12 FX12_SQRT2     = fx12(5793, 4096);   // 1.4142135623730951 (√2)
constexpr fx12 FX12_SQRT1_2   = fx12(2896, 4096);   // 0.7071067811865475 (1/√2)
constexpr fx12 FX12_PI        = fx12(12868, 4096);  // 3.141592653589793 (π)
constexpr fx12 FX12_PI_2      = fx12(6434, 4096);   // 1.5707963267948966 (π/2)
constexpr fx12 FX12_PI_3      = fx12(4290, 4096);   // 1.0471975511965976 (π/3)
constexpr fx12 FX12_PI_4      = fx12(3217, 4096);   // 0.7853981633974483 (π/4)
constexpr fx12 FX12_PI_5      = fx12(2574, 4096);   // 0.6283185307179586 (π/5)
constexpr fx12 FX12_PI_6      = fx12(2145, 4096);   // 0.5235987755982988 (π/6)
constexpr fx12 FX12_PI_7      = fx12(1838, 4096);   // 0.4487989505128276 (π/7)
constexpr fx12 FX12_PI_8      = fx12(1609, 4096);   // 0.39269908169872414 (π/8)
constexpr fx12 FX12_PI_9      = fx12(1432, 4096);   // 0.3490658503988659 (π/9)
constexpr fx12 FX12_PI_10     = fx12(1287, 4096);   // 0.3141592653589793 (π/10)
constexpr fx12 FX12_PI_11     = fx12(1171, 4096);   // 0.28559933214452665 (π/11)
constexpr fx12 FX12_PI_12     = fx12(1072, 4096);   // 0.2617993877991494 (π/12)
constexpr fx12 FX12_PI_13     = fx12(989, 4096);    // 0.241660973353061 (π/13)
constexpr fx12 FX12_PI_14     = fx12(919, 4096);    // 0.2243994752564138 (π/14)
constexpr fx12 FX12_PI_15     = fx12(858, 4096);    // 0.20943951023931953 (π/15)
constexpr fx12 FX12_PI_16     = fx12(804, 4096);    // 0.19634954084936207 (π/16)
struct  Rect;
class Vec {
public:
  fx8 x;
  fx8 y;

  // Constructs the vector with optional initial coordinates
  Vec(fx8 x_ = 0, fx8 y_ = 0) : x(x_), y(y_) {}

  // Sets the coordinates
  Vec& set(fx8 x_ = 0, fx8 y_ = 0);

  // Adds specified values to the vector
  Vec& add(fx8 x_ = 0, fx8 y_ = 0);

  // Subtracts specified values from the vector
  Vec& sub(fx8 x_ = 0, fx8 y_ = 0);

  // Multiplies the vector by a scalar
  Vec& mul(fx8 v);

  // Divides the vector by a scalar
  Vec& div(fx8 v);

  // Clamps the vector within a specified range
  Vec& clamp(fx8 xLow, fx8 xHigh, fx8 yLow, fx8 yHigh);

  // Wraps the vector's coordinates around the specified range
  Vec& wrap(fx8 xLow, fx8 xHigh, fx8 yLow, fx8 yHigh);

  // Sets the vector by using an angle and a length
  Vec& setWithAngle(fx8 angle, fx8 length);

  // Adds to the vector by using an angle and a length
  Vec& addWithAngle(fx8 angle, fx8 length);

  // Swaps the x and y components
  Vec& swapXy();

  // Normalizes the vector
  Vec& normalize();

  // Rotates the vector by a specified angle
  Vec& rotate(fx8 angle);

  // Returns the angle from this vector to given coordinates
  fx8 angleTo(fx8 x_, fx8 y_) const;

  // Returns the distance from this vector to given coordinates
  fx8 distanceTo(fx8 x_, fx8 y_) const;

  // Returns the distance from this vector to another vector
  fx8 distanceTo(const Vec& xy_) const;

  // Checks if the vector is inside the specified rectangle
  bool isInRect(fx8 x_, fx8 y_, fx8 width, fx8 height) const;
  bool isInRect(const Rect& rc ) const;

  // Checks if this vector equals another vector
  bool equals(const Vec& other) const;

  // Floors the vector's coordinates
  Vec& floor();

  // Rounds the vector's coordinates
  Vec& round();

  // Ceils the vector's coordinates
  Vec& ceil();

  // Returns the length of the vector
  fx8 length() const;

  // Returns the angle of the vector
  fx8 angle() const;

  // Member function that negates the vector components
  // (reverses the signs of x and y)
  Vec& negative() {
    x = -x;
    y = -y;
    return *this;
  }

  // Returns the result of vector addition
  Vec operator+(const Vec& other) const {
    return Vec(x + other.x, y + other.y);
  }

  // Returns the result of vector subtraction
  Vec operator-(const Vec& other) const {
    return Vec(x - other.x, y - other.y);
  }

  // Adds a vector to this vector
  Vec& operator+=(const Vec& other) {
    x += other.x;
    y += other.y;
    return *this;
  }

  // Subtracts a vector from this vector
  Vec& operator-=(const Vec& other) {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  // Returns the result of multiplying this vector by a scalar
  Vec operator*(fx8 scalar) const {
    return Vec(x * scalar, y * scalar);
  }

  // Multiplies this vector by a scalar
  Vec& operator*=(fx8 scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
  }

  // Returns the result of element-wise vector multiplication
  Vec operator*(const Vec& other) const {
    return Vec(x * other.x, y * other.y);
  }

  // Performs element-wise vector multiplication
  Vec& operator*=(const Vec& other) {
    x *= other.x;
    y *= other.y;
    return *this;
  }

  // Returns the result of element-wise vector division
  Vec operator/(const Vec& other) const {
      fx8 rx = (other.x == 0) ? fx8(0) : (x / other.x);
      fx8 ry = (other.y == 0) ? fx8(0) : (y / other.y);
      return Vec(rx, ry);
  }

  // Performs element-wise vector division
  Vec& operator/=(const Vec& other) {
      if (other.x == 0) {
          x = fx8(0);
      } else {
          x /= other.x;
      }
      if (other.y == 0) {
          y = fx8(0);
      } else {
          y /= other.y;
      }
      return *this;
  }

  // Returns the result of dividing this vector by a scalar
  Vec operator/(fx8 scalar) const {
    Vec tmp(*this);
    tmp.div(scalar);
    return tmp;
  }

  // Divides this vector by a scalar
  Vec& operator/=(fx8 scalar) {
    return div(scalar);
  }

  /**
   * @brief Clamps the length of the vector to the specified maximum value.
   *
   * If the current length of the vector exceeds the specified `maxLength`,
   * the vector's components are scaled down proportionally to match `maxLength`.
   * If the current length is less than or equal to `maxLength`, no changes are made.
   *
   * @param maxLength The maximum allowable length for the vector.
   *                  Must be a non-negative value.
   * @return A reference to the modified vector.
   *
   * @note If the current length of the vector is zero, no scaling is performed.
   */
  Vec& clampLength(fx8 maxLength) {
      fx8 currentLen = length();  // Get the current length of the vector
      if (currentLen > maxLength && currentLen != fx8(0)) {
          fx8 scale = maxLength / currentLen;  // Calculate the scaling factor
          x *= scale;
          y *= scale;
      }
      return *this;
  }
};

inline std::string tostr(const Vec& val) {
  return  '(' + tostr( val.x ) + " , "  + tostr( val.y ) + ')';
}

/**
 * @brief Represents a line segment in 2D space defined by two points.
 *
 * The `Line` structure defines a line segment with two endpoints, `pos0` and `pos1`, 
 * each represented by a `vec` structure. This structure can be used to represent 
 * any straight line between two coordinates in a 2D plane.
 * 
 * @note The default constructor initializes both points to the origin (0, 0).
 */
struct Line {
  Vec pos0;  ///< The starting point of the line.
  Vec pos1;  ///< The ending point of the line.

  /**
   * @brief Default constructor that initializes both points to (0, 0).
   */
  Line() = default;

  /**
   * @brief Constructor that initializes the line with given starting and ending points.
   * 
   * @param p0 The starting point of the line.
   * @param p1 The ending point of the line.
   */
  Line(const Vec& p0, const Vec& p1) : pos0(p0), pos1(p1) {}
};

/**
 * @brief Represents a triangle in 2D space using three points.
 *
 * The `Poly` structure defines a triangle with three vertices, `pos0`, `pos1`, and `pos2`,
 * each represented by a `vec` structure. This structure can be used to define and manipulate
 * triangles in 2D graphics applications.
 * 
 * @note The default constructor initializes the triangle without setting any vertex positions.
 * It is expected that the positions of the vertices will be set manually after initialization.
 */
struct Poly {
  Vec pos0;  ///< The first vertex of the triangle.
  Vec pos1;  ///< The second vertex of the triangle.
  Vec pos2;  ///< The third vertex of the triangle.

  Poly() = default;  ///< Default constructor.
  Poly( fx8 x0,fx8 y0,fx8 x1,fx8 y1,fx8 x2,fx8 y2 ){
    pos0.x = x0; pos0.y = y0;
    pos1.x = x1; pos1.y = y1;
    pos2.x = x2; pos2.y = y2;
  }
};


/**
 * @brief A structure for representing rectangles with fixed-point coordinates.
 */
struct Rect{
  fx8 x;
  fx8 y;
  fx8 w;
  fx8 h;

  /**
   * @brief Set the coordinates and dimensions of the rectangle.
   * 
   * @param xx_ The x-coordinate of the rectangle.
   * @param yy_ The y-coordinate of the rectangle.
   * @param ww_ The width of the rectangle.
   * @param hh_ The height of the rectangle.
   */
  void  SetXYWH( fx8 x_ , fx8 y_ , fx8 w_ , fx8 h_ ){
    x = x_;
    y = y_;
    w = w_;
    h = h_;
  }

  /**
   * @brief Calculate the middle point x-coordinate of the rectangle.
   * 
   * @return The middle point x-coordinate.
   */
  fx8 MiddlePointX(){
    fx8 ww(w);
    ww.asr(1);
    return x + w;
  }

  Rect( fx8 x_ , fx8 y_ , fx8 w_ , fx8 h_ ){
    SetXYWH( x_ , y_ , w_ , h_ );
  }
  Rect(){}
};

inline std::string tostr(const Rect& val) {
  return '(' + tostr( val.x ) + "," + tostr( val.y ) + " , " +
               tostr( val.w ) + "," + tostr( val.h )  + ')';

};

/**
 * @brief Get the minimum of two fixed-point values.
 * 
 * @param lhs_ The first value.
 * @param rhs_ The second value.
 * @return The minimum value.
 */
inline  const fx8& _min(const fx8& lhs_ , const fx8& rhs_ ){
  return  lhs_ < rhs_ ? lhs_ : rhs_;
}

/**
 * @brief Get the maximum of two fixed-point values.
 * 
 * @param lhs_ The first value.
 * @param rhs_ The second value.
 * @return The maximum value.
 */
inline  const fx8& _max(const fx8& lhs_ , const fx8& rhs_ ){
  return  lhs_ > rhs_ ? lhs_ : rhs_;
}

/**
 * @brief Limit a fixed-point value within a specified range.
 * 
 * @param x_ The value to limit.
 * @param lhs_ The lower bound.
 * @param rhs_ The upper bound.
 * @return The limited value.
 */
inline  const fx8& _lim( const fx8& x_, const fx8& lhs_, const fx8& rhs_){
  return  _min( _max( lhs_, x_ ), rhs_ );
}

/**
 * @brief Get the absolute value of a fixed-point number.
 * 
 * @param x_ The value.
 * @return The absolute value.
 */
inline  const fx8 _abs(const fx8& x_ ){
  return x_ > fx8(0) ? x_ : -x_;
}

/**
 * @brief Limit a value within a specified range.
 * 
 * @tparam T The type of the value.
 * @tparam U The type of the lower bound.
 * @tparam V The type of the upper bound.
 * 
 * @param x_ The value to limit.
 * @param lhs_ The lower bound.
 * @param rhs_ The upper bound.
 * 
 * @return The limited value.
 */
template <typename T, typename U, typename V>
inline T lim(const T& x_, const U& lhs_, const V& rhs_) {
    return std::min(static_cast<T>(std::max(x_, static_cast<T>(lhs_))), static_cast<T>(rhs_));
}

/**
 * @brief Calculate the sine of an angle using a 12-bit angle representation.
 * 
 * @param th The angle in 12-bit representation.
 * @return The sine of the angle.
 */
extern  s16 sin_12( u32 th  );

/**
 * @brief Calculate the cosine of an angle using a 12-bit angle representation.
 * 
 * @param th The angle in 12-bit representation.
 * @return The cosine of the angle.
 */
extern  s16 cos_12( u32 th  );

/**
 * @brief Calculate the cosine of an angle and scale a fixed-point value by it.
 * 
 * @param rad The fixed-point value to scale.
 * @param th The angle in 12-bit representation.
 * @return The scaled fixed-point value.
 */
extern  fx8 rad_cos_12( fx8 rad, u32 th );

/**
 * @brief Calculate the sine of an angle and scale a fixed-point value by it.
 * 
 * @param rad The fixed-point value to scale.
 * @param th The angle in 12-bit representation.
 * @return The scaled fixed-point value.
 */
extern  fx8 rad_sin_12( fx8 rad, u32 th );

/**
 * @brief Generate a random fixed-point number within a specified range.
 * 
 * @param min_ The minimum value.
 * @param max_ The maximum value.
 * @return The random fixed-point number within the range.
 */
extern  fx8 genrand_min_max_fx8(fx8 min_ , fx8 max_ );

/**
 * @brief Computes the remainder of x divided by N using an optimized method.
 *
 * This function calculates `x % N` efficiently using different methods based on the divisor `N`:
 * - If `N` is zero, the function returns zero to prevent division by zero errors.
 * - If `N` is a power of two, the remainder is computed using `x & (N - 1)`,
 *   which is significantly faster than division.
 * - If `N < 128`, a precomputed reciprocal table (`inv_tbl`) is used to approximate
 *   the division, leveraging fixed-point multiplication.
 * - If `N >= 128`, the function directly falls back to the standard `%` operator.
 *
 * @param x The dividend.
 * @param N The divisor (must be greater than zero).
 * @return The remainder of `x` divided by `N` (`x % N`).
 *
 * @note This function optimizes the modulo operation by avoiding slow division instructions
 *       whenever possible, making use of bitwise and multiplication-based methods.
 */
extern  uint32_t qmod(uint32_t x, uint32_t N);

/**
 * @brief Computes the quotient of x divided by N using an optimized method.
 *
 * This function calculates the quotient (integer division result) of `x / N` efficiently using
 * different strategies based on the value of the divisor `N`:
 * - If `N` is zero, the function returns zero to avoid division by zero errors.
 * - If `N` is a power of two, the quotient is computed directly using `x / N`.
 * - If `N` is less than 128, a precomputed reciprocal table (`inv_tbl`) is employed to approximate
 *   the division through fixed-point multiplication.
 * - If `N` is 128 or greater, the function falls back to the standard division operator.
 *
 * @param x The dividend.
 * @param N The divisor (should be greater than zero).
 * @return The quotient of `x` divided by `N`.
 *
 * @note This function is designed to optimize division by avoiding slow division instructions whenever
 *       possible, leveraging bitwise and multiplication-based methods.
 */
extern uint32_t qdiv(uint32_t x, uint32_t N);

#define M_E         2.71828182845904523536028747135266250   /* e              */
#define M_LOG2E     1.44269504088896340735992468100189214   /* log2(e)        */
#define M_LOG10E    0.434294481903251827651128918916605082  /* log10(e)       */
#define M_LN2       0.693147180559945309417232121458176568  /* loge(2)        */
#define M_LN10      2.30258509299404568401799145468436421   /* loge(10)       */
#define M_PI        3.14159265358979323846264338327950288   /* pi             */
#define M_PI_2      1.57079632679489661923132169163975144   /* pi/2           */
#define M_PI_4      0.785398163397448309615660845819875721  /* pi/4           */
#define M_1_PI      0.318309886183790671537767526745028724  /* 1/pi           */
#define M_2_PI      0.636619772367581343075535053490057448  /* 2/pi           */
#define M_2_SQRTPI  1.12837916709551257389615890312154517   /* 2/sqrt(pi)     */
#define M_SQRT2     1.41421356237309504880168872420969808   /* sqrt(2)        */
#define M_SQRT1_2   0.707106781186547524400844362104849039  /* 1/sqrt(2)      */

struct Xorshift32 {
  u32 state;

  explicit Xorshift32(uint32_t seed = 0xA5A5A5A5 ) : state(seed) {
    if (state == 0) state = 1;
  }

  u32 next() {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
  }

  /**
   * @brief Generates a random number on [0, 0x7fffffff] interval.
   *
   * This function generates a 31-bit random number in the range [0, 0x7fffffff].
   *
   * @return A 31-bit signed random number.
   */
  s32 next_int31(void){
    return  static_cast< s32 >( next()>>1 );
  }

  /**
   * @brief Generates a random number in the specified range [min_, max_].
   *
   * This function generates a random number in the inclusive range [min_, max_].
   *
   * @param min_ The minimum value of the range.
   * @param max_ The maximum value of the range.
   * @return A random number in the specified range.
   */
  s32 next_range(int32_t min_, int32_t max_) {
    if (min_ > max_) std::swap(min_, max_);
    const uint64_t range = static_cast<uint64_t>(max_) - static_cast<uint64_t>(min_) + 1;
    if (range > static_cast<uint64_t>(0x7fffffff)) {
      int32_t random_val;
      do {
        random_val = this->next_int31();
      } while (static_cast<uint64_t>(random_val) >= range);
      return min_ + random_val;
    } else {
      return min_ + qmod( this->next_int31() , static_cast<int32_t>(range) );
    }
  }
};

#define MAXFLOAT    0x1.fffffep+127f