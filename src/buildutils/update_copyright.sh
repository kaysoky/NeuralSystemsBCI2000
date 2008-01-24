#!/bin/sh
for i in `find . -iname "*.cpp"` `find . -iname "*.h"` `find . -iname "*.m"` `find . -iname "Makefile"`;
  do echo "$i";
  mv "$i" "$i.bak" && ( sed 's/(C) 2000-2007, BCI2000 Project/(C) 2000-2008, BCI2000 Project/' < "$i.bak" > "$i" )
done;
