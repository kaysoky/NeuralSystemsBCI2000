#!/usr/bin/python

"""
The function ``whiten(data)'' take an M by N array of data. Each column
represents a sample, and each row represents a dimension. Data could be viewed
as an array of column vectors

"""

from __future__ import division

import numpy as np

__all__ = ['whiten']

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
    reorder = np.argsort(eigvars)[::-1]
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

def classify(data, type):
    target = data[:, type]
    nontarget = data[:, ~type]

    primary_transformation = whiten(data)

    target_transformed = np.dot(primary_transformation, target)
    nontarget_transformed = np.dot(primary_transformation, nontarget)

    target_basis = unbiased_basis(target_transformed)
    nontarget_basis = unbiased_basis(nontarget_transformed)
    secondary_basis = add_bases(target_basis, nontarget_basis)
    secondary_transformation = [np.linalg.inv(secondary_basis)[-1]]

    transformation = np.dot(secondary_transformation, primary_transformation)

    target_transformed = np.dot(transformation, target)
    nontarget_transformed = np.dot(transformation, nontarget)
    if target_transformed.mean() < nontarget_transformed.mean():
        transformation *= -1
        target_transformed *= -1
        nontarget_transformed *= -1

    candidates = np.concatenate([target_transformed,
        nontarget_transformed], axis = 1)
    candidates = np.unique(candidates).reshape((-1, 1))
    scores = ((target_transformed > candidates).sum(axis = 1).astype(float) / \
        target.size + \
        (nontarget_transformed < candidates).sum(axis = 1).astype(float) / \
        nontarget.size) ** 2
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

    return transformation, bias

def test():
    sine = np.sin(np.arange(3000) / 72)# + \
        #np.random.randn(3000) / 10
    #np.random.shuffle(sine)
    edge = abs((np.arange(3000) % 100) - 50) / \
        25 - 1# + \
        #np.random.randn(3000) / 10
    #np.random.shuffle(edge)

    type = edge > 0.9
    #type = (0.2 < edge) & (edge < 0.4)

    pure = np.concatenate([[sine], [edge]])

    mixer = np.asarray(np.random.randn(2, 2))
    shifter = np.random.randn(1)

    mixed = np.dot(mixer, pure) + shifter# + \
        #np.random.randn(2, 3000) / 40

    from pylab import ion, figure, plot, scatter, arrow, close, title, axvline
    ion()
    close('all')
    figure()
    title('Original Data')
    scatter(*pure[:, ~type], c = 'r', marker = 'd')
    scatter(*pure[:, type], c = 'b', marker = 'd')

    figure()
    title('Mixed Data')
    scatter(*mixed[:, ~type], c = 'r', marker = 'd')
    scatter(*mixed[:, type], c = 'b', marker = 'd')

    transformation = whiten(mixed)
    target_transformed = np.dot(transformation, mixed[:, type])
    nontarget_transformed = np.dot(transformation, mixed[:, ~type])
    figure()
    title('Decorrelated Data (with Decorrelation Bases of' + \
        ' Target and Non-Target Data)')
    scatter(*nontarget_transformed, c = 'r', marker = 'd')
    scatter(*target_transformed, c = 'b', marker = 'd')

    target_basis = unbiased_basis(target_transformed)
    target_mean = target_transformed.mean(axis = 1).reshape((-1, 1))
    nontarget_basis = unbiased_basis(nontarget_transformed)
    nontarget_mean = nontarget_transformed.mean(axis = 1).reshape((-1, 1))
    for axis in add_bases(target_basis, nontarget_basis).transpose():
        ar = np.concatenate([((target_mean + nontarget_mean) / 2).ravel(),
            axis])
        arrow(*ar, linewidth = 5, color = 'g')
    for axis in target_basis.transpose():
        ar = np.concatenate([target_mean.ravel(), axis])
        arrow(*ar, linewidth = 4)
    for axis in nontarget_basis.transpose():
        ar = np.concatenate([nontarget_mean.ravel(), axis])
        arrow(*ar, linewidth = 4)

    classifier, bias = classify(mixed, type)
    nontarget = np.dot(classifier, mixed[:, ~type])
    target = np.dot(classifier, mixed[:, type])
    figure()
    title('Data Classification')
    ntx = np.random.randn(nontarget.size)
    tx = np.random.randn(target.size)
    scatter(nontarget, ntx, c = 'r', marker = 'd')
    scatter(target, tx, c = 'b', marker = 'd')
    axvline(bias, color = 'k')

def main(argv = []):
    test()

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])

