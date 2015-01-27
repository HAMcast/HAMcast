ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = etc/* INSTALL LICENSE README bootstrap
SUBDIRS    = docu source sample

# hook to remove all .svn files before rolling the tarball
dist-hook:
	find $(distdir) -name '.[a-zA-Z0-9]*' | xargs rm -rf
