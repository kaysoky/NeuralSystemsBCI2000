#!/bin/sh
for i in `find . -iname "*.cpp"` `find . -iname "*.h"` `find . -iname "*.m"` `find . -iname "Makefile"` `find . -iname "*.bat"` `find . -iname "*.y"`;
  do echo "$i";
  mv "$i" "$i.bak" && ( sed -b 's/(C) 2000-2008, BCI2000 Project/(C) 2000-2009, BCI2000 Project/' < "$i.bak" > "$i" )
done;
