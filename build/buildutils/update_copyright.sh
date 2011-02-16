#!/bin/sh
for i in `find . -iname "*.cpp"` `find . -iname "*.h"` `find . -iname "*.m"` `find . -iname "Makefile"` `find . -iname "*.bat"` `find . -iname "*.y"`;
  do echo "$i";
  mv "$i" "$i.bak" && ( sed 's/(C) 2000-2010, BCI2000 Project/(C) 2000-2011, BCI2000 Project/' < "$i.bak" > "$i" )
done;
