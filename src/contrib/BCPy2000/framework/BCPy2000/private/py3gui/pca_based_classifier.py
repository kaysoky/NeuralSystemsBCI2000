#!/usr/bin/python

"""
The function ``whiten(data)'' take an M by N array of data. Each column
represents a sample, and each row represents a dimension. Data could be viewed
as an array of column vectors

"""

from __future__ import division

import numpy as np

__all__ = ['classify']

def add_bases(basis1, basis2):
    unit1 = basis1 / np.sqrt((basis1 ** 2).sum(axis = 0))
    unit2 = basis2 / np.sqrt((basis2 ** 2).sum(axis = 0))
    reorder = abs(np.dot(unit2.transpose(), unit1)).argmax(axis = 0)
    basis2 = basis2[:, reorder]
    negate = np.sign((basis1 * basis2).sum(axis = 0))
    new_basis = basis1 + basis2 * negate
    magnitudes_squared = (new_basis ** 2).sum(axis = 0)
    reorder = np.argsort(magnitudes_squared)[::-1]
    new_basis = new_basis[:, reorder]
    return new_basis

def unbiased_basis(data, unit = False):
    mean = data.mean(axis = 1).reshape((-1, 1))
    data = data - mean
    covariance = np.dot(data, data.transpose()) / (data.shape[1] - 1)
    eigvars, eigvects = np.linalg.eig(covariance)
    reorder = np.argsort(eigvars)
    eigvects = eigvects[:, reorder]
    if unit:
        return eigvects
    eigvars = eigvars[reorder]
    eigvars = np.sqrt(abs(eigvars)) * np.sign(eigvars)
    basis = eigvars * eigvects
    return basis

def whiten(data):
    """
    Takes an M by N array of data. Each column represents a sample, and each
    row represents a dimension. Data could be viewed as a column-major array
    of column vectors.

    Returns:
        transformation: the transformation matrix that would whiten this data

    """
    basis = unbiased_basis(data)
    transformation = np.linalg.inv(basis)
    return transformation

def fair_bias(target_transformed, nontarget_transformed):
    candidates = np.concatenate([target_transformed,
        nontarget_transformed], axis = 1)
    candidates = np.unique(candidates).reshape((-1, 1))
    scores = ((target_transformed > candidates).sum(axis = 1).astype(float) / \
        target_transformed.size + \
        (nontarget_transformed < candidates).sum(axis = 1).astype(float) / \
        nontarget_transformed.size) ** 2

    candidates = candidates.ravel()
    scores = scores.ravel()

    min = scores.min() - 1

    middleindex = np.argmax(scores)
    middlescore = scores[middleindex]
    middlecandidate = candidates[middleindex]

    scores[scores == middlescore] = min

    leftindex = np.argmax(scores * (candidates < middlecandidate))
    leftscore = scores[leftindex]
    leftcandidate = candidates[leftindex]

    scores[scores == leftscore] = min

    rightindex = np.argmax(scores * (candidates > middlecandidate))
    rightscore = scores[rightindex]
    rightcandidate = candidates[rightindex]

    bias = (middlecandidate * middlescore + \
        leftcandidate * leftscore + \
        rightcandidate * rightscore) / \
        (middlescore + leftscore + rightscore)

    return bias

def classify(data, type):
    target = data[:, type]
    nontarget = data[:, ~type]

    primary_transformation = whiten(data)

    target_transformed = np.dot(primary_transformation, target)
    nontarget_transformed = np.dot(primary_transformation, nontarget)

    target_basis = unbiased_basis(target_transformed)
    nontarget_basis = unbiased_basis(nontarget_transformed)
    if (target_basis[:, 0] ** 2).sum() < (nontarget_basis[:, 0] ** 2).sum():
        secondary_basis = target_basis
    else:
        secondary_basis = nontarget_basis
    secondary_transformation = np.linalg.inv(secondary_basis)

    transformation = np.dot(secondary_transformation, primary_transformation)

    target_transformed = np.dot(transformation, target)
    nontarget_transformed = np.dot(transformation, nontarget)
    if target_transformed.mean() < nontarget_transformed.mean():
        transformation *= -1
        target_transformed *= -1
        nontarget_transformed *= -1

    #bias = fair_bias(target_transformed, nontarget_transformed)

    """
    from pylab import ion, figure, title, scatter, axvline
    ion()
    figure()
    title('Data Classification')
    ntx = np.random.randn(nontarget_transformed.size)
    tx = np.random.randn(target_transformed.size)
    scatter(nontarget_transformed, ntx, c = 'r', marker = 'd')
    scatter(target_transformed, tx, c = 'b', marker = 'd')
    axvline(bias, color = 'k')
    """
    return transformation#, bias

