
import numpy
from scipy.linalg.decomp import asarray_chkfinite, _datanotshared, \
    get_lapack_funcs, basic, find_best_lapack_type
import cvxopt
from cvxopt import lapack

def qr(a, overwrite_a=0, lwork=None, mode='qr', pivoting = False):
    """Compute QR decomposition of a matrix.

    Calculate the decomposition :lm:`A = Q R` where Q is unitary/orthogonal
    and R upper triangular.

    Parameters
    ----------
    a : array, shape (M, N)
        Matrix to be decomposed
    overwrite_a : boolean
        Whether data in a is overwritten (may improve performance)
    lwork : integer
        Work array size, lwork >= a.shape[1]. If None or -1, an optimal size
        is computed.
    mode : {'qr', 'r', 'economic'}
        Determines what information is to be returned: either both Q and R
        or only R, or Q and R and P, a permutation matrix. Any of these can
        be combined with 'economic' using the '+' sign as a separator.
        Economic mode means the following:
            Compute the economy-size QR decomposition, making shapes
            of Q and R (M, K) and (K, N) instead of (M,M) and (M,N).
            K=min(M,N).
    pivoting : bool, optional
        Whether or not factorization should include pivoting for rank-revealing
        qr decomposition. If pivoting, compute the decomposition
        :lm:`A P = Q R` as above, but where P is chosen such that the diagonal
        of R is non-increasing. P represents the new column order of A.

    Returns
    -------
    (if mode == 'qr')
    Q : double or complex array, shape (M, M) or (M, K) for econ==True

    (for any mode)
    R : double or complex array, shape (M, N) or (K, N) for econ==True
        Size K = min(M, N)
    P : integer ndarray
        Of shape (N,) for ``pivoting=True``.
        Not returned if ``pivoting=False``.

    Raises LinAlgError if decomposition fails

    Notes
    -----
    This is an interface to the LAPACK routines dgeqrf, zgeqrf,
    dorgqr, and zungqr.

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
    mode = mode.split("+")
    if "economic" in mode:
        econ = True
    else:
        econ = False

    a1 = asarray_chkfinite(a)
    if len(a1.shape) != 2:
        raise ValueError("expected 2D array")
    M, N = a1.shape
    overwrite_a = overwrite_a or (_datanotshared(a1,a))

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
        geqrf, = get_lapack_funcs(('geqrf',),(a1,))
        if lwork is None or lwork == -1:
            # get optimal work array
            qr,tau,work,info = geqrf(a1,lwork=-1,overwrite_a=1)
            lwork = work[0]

        qr,tau,work,info = geqrf(a1,lwork=lwork,overwrite_a=overwrite_a)
        if info<0:
            raise ValueError("illegal value in %d-th argument of internal geqrf"
                % -info)

    if not econ or M<N:
        R = basic.triu(qr)
    else:
        R = basic.triu(qr[0:N,0:N])

    if 'r' in mode:
        return R

    if find_best_lapack_type((a1,))[0]=='s' or find_best_lapack_type((a1,))[0]=='d':
        gor_un_gqr, = get_lapack_funcs(('orgqr',),(qr,))
    else:
        gor_un_gqr, = get_lapack_funcs(('ungqr',),(qr,))

    if M<N:
        # get optimal work array
        Q,work,info = gor_un_gqr(qr[:,0:M],tau,lwork=-1,overwrite_a=1)
        lwork = work[0]
        Q,work,info = gor_un_gqr(qr[:,0:M],tau,lwork=lwork,overwrite_a=1)
    elif econ:
        # get optimal work array
        Q,work,info = gor_un_gqr(qr,tau,lwork=-1,overwrite_a=1)
        lwork = work[0]
        Q,work,info = gor_un_gqr(qr,tau,lwork=lwork,overwrite_a=1)
    else:
        t = qr.dtype.char
        qqr = numpy.empty((M,M),dtype=t)
        qqr[:,0:N]=qr
        # get optimal work array
        Q,work,info = gor_un_gqr(qqr,tau,lwork=-1,overwrite_a=1)
        lwork = work[0]
        Q,work,info = gor_un_gqr(qqr,tau,lwork=lwork,overwrite_a=1)

    if info < 0:
        raise ValueError("illegal value in %-th argument of internal gorgqr"
            % -info)

    if pivoting:
        return Q, R, jpvt

    return Q, R

