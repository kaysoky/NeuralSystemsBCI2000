function s = regexprepCompat(string, expression, replace)

if strncmp(version, '6', 1)
  s = regexprep(string, expression, replace, 'tokenize');
else
  s = regexprep(string, expression, replace);
end