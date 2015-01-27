ACLOCAL_AMFLAGS = -I m4
SUBDIRS    = docu source sample
EXTRA_DIST = etc/pingpong/* etc/patch/* INSTALL LICENSE README bootstrap bootstrap_libs

# hook to remove all .svn files before rolling the tarball
dist-hook:
	find $(distdir) -name '.[a-zA-Z0-9]*' | xargs rm -rf
