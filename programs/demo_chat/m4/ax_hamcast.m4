# SYNOPSIS
#
#   AX_HAMCAST
#
# DESCRIPTION
#
#   Test for libhamcast.
#
#   This macro calls:
#
#     AC_SUBST(HAMCAST_CPPFLAGS)
#     AC_SUBST(HAMCAST_LDFLAGS)
#     AC_SUBST(HAMCAST_LIB)
#
#   And sets:
#
#     HAVE_HAMCAST

AC_DEFUN([AX_HAMCAST],
[

	PROJECT_NAME="hamcast"
    LIB_NAME="hamcast"

	AC_ARG_WITH([hamcast-libdir],
				AS_HELP_STRING([--with-hamcast-libdir=LIB_DIR],
							   [Force given directory for hamcast libraries]),
							   [
							   if test -d "$withval"; then
							       hc_forced_dir="$withval"
							   else
							       AC_MSG_ERROR(--with-hamcast-libdir expected directory name)
							   fi
							   ],
							   [hc_forced_dir=""])

	AC_ARG_ENABLE(hamcast-logging,
				  [  --enable-hamcast-logging Enable logging -- this is a debugging feature which should not be usually enabled],
				  [hamcast_logging=yes])

	# this regex matches the library file name
	LIB_REGEX="^lib${LIB_NAME}-[[0-9]]\+\\.[[0-9]]\+\\.[[^0-9]]\+$"
	
	# this regex extracts the library version
	LIBV_REGEX="[[0-9]]\+\\.[[0-9]]\+"

	m_CPPFLAGS=""
	m_LDFLAGS=""
	m_VERSION=""

	flags_ok() {
		if test "$m_CPPFLAGS" != "" ; then return 0 ; else return 1 ; fi
	}

	AC_MSG_CHECKING([for lib${LIB_NAME}])

	# scan forced directory only
	if test "$hc_forced_dir" != "" ; then
		# forced dir has "lib/" and "include/" subdirs?
		if test -d "$hc_forced_dir/lib" && test -d "$hc_forced_dir/include" ; then
			m_VERSION=`ls -1 "${hc_forced_dir}/lib" 2>/dev/null | grep "$LIB_REGEX" | grep -o "$LIBV_REGEX" | sort -rn | head -1`
			if test "$m_VERSION" ; then
				# lib subdir ok, check include dir
				include_path="${hc_forced_dir}/${LIB_NAME}-${lib_version}"
				if test -d "${include_path}/${PROJECT_NAME}" ; then
					# forced dir ok, set variables
					m_CPPFLAGS="-I${include_path}"
					m_LDFLAGS="-L${hc_forced_dir}/lib"
				fi
			fi
		# forced dir might be a path to an "uninstalled" library
		else
			m_VERSION=`ls -1 "${hc_forced_dir}/.libs" 2>/dev/null | grep "$LIB_REGEX" | grep -o "$LIBV_REGEX" | sort -rn | head -1`
			if test "$m_VERSION" && test -d "${hc_forced_dir}/${PROJECT_NAME}" ; then
				m_CPPFLAGS="-I${hc_forced_dir}"
				m_LDFLAGS="-L${hc_forced_dir}/.libs"
			fi
		fi
		# success?
		if test "$m_LDFLAGS" = "" ; then
			AC_MSG_ERROR(no compiled lib${LIB_NAME} found in forced directory)
		fi
	# scan all
	else
		# get the newest installed version
		for dir in /usr/lib /usr/local/lib /opt/lib /opt/local/lib ; do
		m_VERSION=`ls -1 "$dir" 2>/dev/null | grep "$LIB_REGEX" | grep -o "$LIBV_REGEX" | sort -rn | head -1`
			if test "$m_VERSION" ; then
				# got one!
				include_path=`dirname "$dir"`
				include_path="${include_path}/include/${LIB_NAME}-${lib_version}"
				if test -d "${include_path}/${PROJECT_NAME}" ; then
					m_CPPFLAGS="-I${include_path}"
					m_LDFLAGS="-L${dir}"
					break
				fi
			fi
		done
		# scan parent directorys for uninstalled version if needed
		if ! flags_ok ; then
			restore_dir=$(pwd)
			curr_dir=$(pwd)
			success="0"
			while test "$curr_dir" != "/" && test "$success" = "0" ; do
				# look for a folder named "lib${LIB_NAME}"
				dir="${curr_dir}/lib${LIB_NAME}"
				if test -d "$dir" ; then
					# check if there is a compiled library
					m_VERSION=`ls -1 "${dir}/.libs" 2>/dev/null | grep "$LIB_REGEX" | grep -o "$LIBV_REGEX" | sort -rn | head -1`
					if test "$m_VERSION" ; then
						# check for headers
						if test -d "${dir}/${LIB_NAME}" ; then
							success="1"
							m_CPPFLAGS="-I$dir"
							m_LDFLAGS="-L$dir/.libs"
						fi
					fi
				fi
				if test "$success" = "0" ; then
					cd $curr_dir
					cd ..
					curr_dir=$(pwd)
				fi
			done
			# restore environment
			cd $restore_dir
		fi
	fi

	if test "$m_LDFLAGS" = "" ; then
		AC_MSG_ERROR(lib${LIB_NAME} not found!)
	fi

	if test "$hamcast_logging" = "yes" ; then
		m_CPPFLAGS="$m_CPPFLAGS -DHC_ENABLE_LOGGING"
	fi

	HAMCAST_CPPFLAGS="$m_CPPFLAGS"
	HAMCAST_LDFLAGS="$m_LDFLAGS"
	HAMCAST_LIB="-l${LIB_NAME}-${m_VERSION}"

	AC_SUBST(HAMCAST_CPPFLAGS)
	AC_SUBST(HAMCAST_LDFLAGS)
	AC_SUBST(HAMCAST_LIB)
	AC_DEFINE(HAVE_HAMCAST,,[define if the HAMcast library is available])
	AC_MSG_RESULT(yes; version $m_VERSION; path: $HAMCAST_LDFLAGS)

])
