/**
 * \file CircularEngine.hpp
 * \brief Header for GeographicLib::CircularEngine class
 *
 * Copyright (c) Charles Karney (2011) <charles@karney.com> and licensed under
 * the MIT/X11 License.  For more information, see
 * http://geographiclib.sourceforge.net/
 **********************************************************************/

#if !defined(GEOGRAPHICLIB_CIRCULARENGINE_HPP)
#define GEOGRAPHICLIB_CIRCULARENGINE_HPP "$Id$"

#include <vector>
#include <GeographicLib/Constants.hpp>
#include <GeographicLib/SphericalEngine.hpp>

namespace GeographicLib {

  /**
   * \brief Spherical Harmonic sums for a circle.
   *
   * The class is a companion to SphericalEngine.  If the results of a
   * spherical harmonic sum are needed for several points on a circle of
   * constant latitude \e lat and height \e h, then SphericalEngine::Circle can
   * compute the inner sum, which is independent of longitude \e lon, and
   * produce a CircularEngine object.  CircularEngine::operator()(real) can
   * then be used to perform the outer sum for particular vales of \e lon.
   * This can lead to substantial improvements in computational speed for high
   * degree sum (approximately by a factor of \e N / 2 where \e N is the
   * maximum degree).
   *
   * The constructor for this class is private.  Use SphericalHarmonic::Circle,
   * SphericalHarmonic1::Circle, and SphericalHarmonic2::Circle to create
   * instances of this class.
   **********************************************************************/

  class GEOGRAPHIC_EXPORT CircularEngine {
  private:
    typedef Math::real real;
    enum normalization {
      full = SphericalEngine::full,
      schmidt = SphericalEngine::schmidt,
    };
    int _M;
    bool _gradp;
    normalization _norm;
    real _scale, _a, _r, _u, _t;
    std::vector<real> _wc, _ws, _wrc, _wrs, _wtc, _wts;
    real _q, _uq, _uq2;

    Math::real Value(bool gradp, real cl, real sl,
                     real& gradx, real& grady, real& gradz) const;

    static inline void cossin(real x, real& cosx, real& sinx) {
      x = x >= 180 ? x - 360 : (x < -180 ? x + 360 : x);
      real xi = x * Math::degree<real>();
      cosx = std::abs(x) ==   90 ? 0 : cos(xi);
      sinx =          x  == -180 ? 0 : sin(xi);
    }

    friend class SphericalEngine;
    /**
     * Constructor for CircularEngine with
     *
     * @param[in] M the maximum order of the spherical harmonic sum.
     * @param[in] gradp whether to include the coefficients of the series for
     *   the gradient of the sum.
     * @param[in] norm the normalization of the Legrendre functions (either
     *   full or schmidt).
     * @param[in] scale a scaling that is given to the coefficients to avoid
     *   overflow.
     * @param[in] a the reference radius for the sum.
     * @param[in] r the (spherical) radius of points.
     * @param[in] u the sine of the (spherical) colatiude.
     * @param[in] t the cosine of the (spherical) colatiude.
     *
     * Thus the CircularEngine evaluates the harmonic sum (and its gradient)
     * for points on the circle of radius \e u \e r which lies a distance \e t \e
     * r above the equatorial plane.  The constructor allocates memory for the
     * arrays used to store the coefficients and stores zero in them.  These
     * coefficients are set with calls to CircularEngine::SetCoeff.
     **********************************************************************/
    CircularEngine(int M, bool gradp, SphericalEngine::normalization norm,
                   real scale, real a, real r, real u, real t)
      : _M(M)
      , _gradp(gradp)
      , _norm(normalization(norm))
      , _scale(scale)
      , _a(a)
      , _r(r)
      , _u(u)
      , _t(t)
      , _wc(std::vector<real>(_M + 1, 0))
      , _ws(std::vector<real>(_M + 1, 0))
      , _wrc(std::vector<real>(_gradp ? _M + 1 : 0, 0))
      , _wrs(std::vector<real>(_gradp ? _M + 1 : 0, 0))
      , _wtc(std::vector<real>(_gradp ? _M + 1 : 0, 0))
      , _wts(std::vector<real>(_gradp ? _M + 1 : 0, 0))
      {
        _q = _a / _r;
        _uq = _u * _q;
        _uq2 = Math::sq(_uq);
      }

    /**
     * Store coefficients for the sum for the order \e m term.
     *
     * @param[in] m the order of the term.
     * @param[in] wc the coefficient for the cos(\e m \e lam) term.
     * @param[in] ws the coefficient for the sin(\e m \e lam) term.
     *
     * \e m must lie in [0, \e M].
     **********************************************************************/
    void SetCoeff(int m, real wc, real ws)
    { _wc[m] = wc; _ws[m] = ws; }

    /**
     * Store coefficients for the sum and its gradient for the order \e m term.
     *
     * @param[in] m the order of the term
     * @param[in] wc the coefficient for the cos(\e m \e lam) term.
     * @param[in] ws the coefficient for the sin(\e m \e lam) term.
     * @param[in] wrc the coefficient for the radial derivative of the cosine
     *   term.
     * @param[in] wrs the coefficient for the radial derivative of the sine
     *   term.
     * @param[in] wtc the coefficient for the \e theta derivative of the cosine
     *   term.
     * @param[in] wts the coefficient for the \e theta derivative of the sine
     *   term.
     *
     * \e m must lie in [0, \e M].  Here \e theta is the spherical colatitude.
     **********************************************************************/
    void SetCoeff(int m, real wc, real ws,
                  real wrc, real wrs, real wtc, real wts) {
      _wc[m] = wc; _ws[m] = ws;
      if (_gradp) {
        _wrc[m] = wrc; _wrs[m] = wrs;
        _wtc[m] = wtc; _wts[m] = wts;
      }
    }

  public:
    /**
     * Evaluate the sum for a particular longitude.
     *
     * @param[in] lon the longitude (degrees).
     * @return[in] \e V the value of the sum.
     **********************************************************************/
    Math::real operator()(real lon) const {
      real coslon, sinlon;
      cossin(lon, coslon, sinlon);
      return (*this)(coslon, sinlon);
    }

    /**
     * Evaluate the sum for a particular longitude given in terms of its
     * cosine and sine.
     *
     * @param[in] coslon the cosine of the longitude.
     * @param[in] sinlon the sine of the longitude.
     * @return[in] \e V the value of the sum.
     **********************************************************************/
    Math::real operator()(real coslon, real sinlon) const {
      real dummy;
      return Value(false, coslon, sinlon, dummy, dummy, dummy);
    }

    /**
     * Evaluate the sum and its gradient for a particular longitude.
     *
     * @param[in] lon the longitude (degrees).
     * @param[out] gradx \e x component of the gradient
     * @param[out] grady \e y component of the gradient
     * @param[out] gradz \e z component of the gradient
     * @return[in] \e V the value of the sum.
     **********************************************************************/
    Math::real operator()(real lon,
                          real& gradx, real& grady, real& gradz) const {
      real coslon, sinlon;
      cossin(lon, coslon, sinlon);
      return (*this)(coslon, sinlon, gradx, grady, gradz);
    }

    /**
     * Evaluate the sum and its gradient for a particular longitude given in
     * terms of its cosine and sine.
     *
     * @param[in] coslon the cosine of the longitude.
     * @param[in] sinlon the sine of the longitude.
     * @param[out] gradx \e x component of the gradient
     * @param[out] grady \e y component of the gradient
     * @param[out] gradz \e z component of the gradient
     * @return[in] \e V the value of the sum.
     **********************************************************************/
    Math::real operator()(real coslon, real sinlon,
                          real& gradx, real& grady, real& gradz) const {
      return Value(true, coslon, sinlon, gradx, grady, gradz);
    }
  };

} // namespace GeographicLib

#endif  // GEOGRAPHICLIB_CIRCULARENGINE_HPP