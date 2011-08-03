
from warnings import warn

import numpy
from numpy import asarray_chkfinite, complexfloating

import cvxopt
from cvxopt import lapack

# Local imports
from scipy.linalg import special_matrices
from scipy.linalg.blas import get_blas_funcs
from scipy.linalg.lapack import get_lapack_funcs, find_best_lapack_type
from scipy.linalg.misc import _datanotshared

def qr(a, overwrite_a=False, lwork=None, mode='full', pivoting = False):
    """Compute QR decomposition of a matrix.

    Calculate the decomposition :lm:`A = Q R` where Q is unitary/orthogonal
    and R upper triangular.

    Parameters
    ----------
    a : array, shape (M, N)
        Matrix to be decomposed
    overwrite_a : bool, optional
        Whether data in a is overwritten (may improve performance)
    lwork : int, optional
        Work array size, lwork >= a.shape[1]. If None or -1, an optimal size
        is computed.
    mode : {'full', 'r', 'economic'}
        Determines what information is to be returned: either both Q and R
        ('full', default), only R ('r') or both Q and R but computed in
        economy-size ('economic', see Notes).

    Returns
    -------
    Q : double or complex ndarray
        Of shape (M, M), or (M, K) for ``mode='economic'``.  Not returned if
        ``mode='r'``.
    R : double or complex ndarray
        Of shape (M, N), or (K, N) for ``mode='economic'``.  ``K = min(M, N)``.

    Raises LinAlgError if decomposition fails

    Notes
    -----
    This is an interface to the LAPACK routines dgeqrf, zgeqrf,
    dorgqr, and zungqr.

    If ``mode=economic``, the shapes of Q and R are (M, K) and (K, N) instead
    of (M,M) and (M,N), with ``K=min(M,N)``.

    Examples
    --------
    >>> from scipy import random, linalg, dot
    >>> a = random.randn(9, 6)
    >>> q, r = linalg.qr(a)
    >>> allclose(a, dot(q, r))
    True
    >>> q.shape, r.shape
    ((9, 9), (9, 6))

    >>> r2 = linalg.qr(a, mode='r')
    >>> allclose(r, r2)

    >>> q3, r3 = linalg.qr(a, mode='economic')
    >>> q3.shape, r3.shape
    ((9, 6), (6, 6))

    """
    if mode == 'qr':
        # 'qr' was the old default, equivalent to 'full'. Neither 'full' nor
        # 'qr' are used below, but set to 'full' anyway to be sure
        mode = 'full'
    if not mode in ['full', 'qr', 'r', 'economic']:
        raise ValueError(\
                 "Mode argument should be one of ['full', 'r', 'economic']")

    a1 = asarray_chkfinite(a)
    if len(a1.shape) != 2:
        raise ValueError("expected 2D array")
    M, N = a1.shape
    overwrite_a = overwrite_a or (_datanotshared(a1, a))

    if pivoting:
        qr = cvxopt.matrix(0, a1.shape, tc = 'd')
        qr[:, :] = a1

        tau = cvxopt.matrix(0, (min(M, N), 1), tc = 'd')
        jpvt = cvxopt.matrix(0, (N, 1), tc = 'i')

        lapack.geqp3(qr, jpvt, tau)

        qr = numpy.asarray(qr)
        tau = numpy.asarray(tau)
        jpvt = (numpy.asarray(jpvt) - 1).ravel()
    else:
        geqrf, = get_lapack_funcs(('geqrf',), (a1,))
        if lwork is None or lwork == -1:
            # get optimal work array
            qr, tau, work, info = geqrf(a1, lwork=-1, overwrite_a=1)
            lwork = work[0].real.astype(numpy.int)

        qr, tau, work, info = geqrf(a1, lwork=lwork, overwrite_a=overwrite_a)
        if info < 0:
            raise ValueError("illegal value in %d-th argument of internal geqrf"
                                                                        % -info)
    if not mode == 'economic' or M < N:
        R = special_matrices.triu(qr)
    else:
        R = special_matrices.triu(qr[0:N, 0:N])

    if mode == 'r':
        return R

    if find_best_lapack_type((a1,))[0] == 's' or \
                find_best_lapack_type((a1,))[0] == 'd':
        gor_un_gqr, = get_lapack_funcs(('orgqr',), (qr,))
    else:
        gor_un_gqr, = get_lapack_funcs(('ungqr',), (qr,))

    if M < N:
        # get optimal work array
        Q, work, info = gor_un_gqr(qr[:,0:M], tau, lwork=-1, overwrite_a=1)
        lwork = work[0].real.astype(numpy.int)
        Q, work, info = gor_un_gqr(qr[:,0:M], tau, lwork=lwork, overwrite_a=1)
    elif mode == 'economic':
        # get optimal work array
        Q, work, info = gor_un_gqr(qr, tau, lwork=-1, overwrite_a=1)
        lwork = work[0].real.astype(numpy.int)
        Q, work, info = gor_un_gqr(qr, tau, lwork=lwork, overwrite_a=1)
    else:
        t = qr.dtype.char
        qqr = numpy.empty((M, M), dtype=t)
        qqr[:,0:N] = qr
        # get optimal work array
        Q, work, info = gor_un_gqr(qqr, tau, lwork=-1, overwrite_a=1)
        lwork = work[0].real.astype(numpy.int)
        Q, work, info = gor_un_gqr(qqr, tau, lwork=lwork, overwrite_a=1)

    if info < 0:
        raise ValueError("illegal value in %d-th argument of internal gorgqr"
                                                                    % -info)

    if pivoting:
        return Q, R, jpvt

    return Q, R

