#!/usr/bin/python

import numpy as np
from scipy import integrate
from scipy.special import erf

def convolve(f, g, t, epsabs = 1e-8, epsrel = 1e-8):
    return integrate.quad(lambda tau: f(tau) * g(t - tau), -np.Inf, np.Inf,
        epsabs = epsabs, epsrel = epsrel)[0]

def max_gauss_pdf(mu, sigma, n, x):
    x = x - mu
    var = sigma ** 2.
    sqrt2 = np.sqrt(2.)
    return (sqrt2 / (2. ** n)) * np.exp(-(x ** 2.) / (2. * var)) / \
        (np.sqrt(np.pi) * sigma) * n * \
        (1. + erf(x / (sqrt2 * sigma))) ** (n - 1.)

def max_gauss_cdf(mu, sigma, n, x):
    x = x - mu
    return ((1 + erf(x / (np.sqrt(2.) * sigma))) / 2.) ** n

def main(argv):
    pass

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])

