#!/bin/sh
libtoolize && aclocal -I m4 && autoconf && automake --add-missing --foreign
