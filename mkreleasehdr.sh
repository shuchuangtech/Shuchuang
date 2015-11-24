#!/bin/sh
GIT_SHA1=`(git show-ref --head --hash=8 2> /dev/null || echo 00000000) | head -n1`
GIT_DIRTY=`git diff --no-ext-diff 2> /dev/null | wc -l`
SC_MKTIME=`date "+%Y-%m-%d %H:%M:%S %z"`
test -f ./Include/release.h || touch ./Include/release.h
(cat ./Include/release.h | grep SHA1 | grep $GIT_SHA1) && \
		(cat ./Include/release.h | grep DIRTY | grep $GIT_DIRTY) && \
		(cat ./Include/release.h | grep MKTIME | grep $SC_MKTIME) && exit 0 # Already uptodate
		echo "#define SC_GIT_SHA1 \"$GIT_SHA1\"" > ./Include/release.h
		echo "#define SC_GIT_DIRTY \"$GIT_DIRTY\"" >> ./Include/release.h
		echo "#define SC_MKTIME \"$SC_MKTIME\"" >> ./Include/release.h
		touch ./Src/Common/release.cpp # Force recompile of release.cpp
