function dc=dircos(xyz1, xyz2)
nXyz1=xyz1./repmat(sqrt(sum(xyz1.^2, 2)), [1 3]);
nXyz2=xyz2./repmat(sqrt(sum(xyz2.^2, 2)), [1 3]);
dc=nXyz1*nXyz2';