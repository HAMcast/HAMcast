AC_DEFUN([AX_HAMCAST],
[

	PROJECT_NAME="hamcast"
    LIB_NAME="hamcast"

	# this regex matches the library file name
	LIB_REGEX="^lib${LIB_NAME}-[[0-9]]\+\\.[[0-9]]\+\\.[[^0-9]]\+$"
	# this regex extracts the library version
	LIBV_REGEX="[[0-9]]\+\\.[[0-9]]\+"

    AC_MSG_CHECKING([for lib${LIB_NAME}])

	m_LDFLAGS=""
	m_CPPFLAGS=""
	m_LIB=""

    # get the newest installed version
	# todo: sort does not sort correctly, e.g.: libfoo12.0 < libfoo9.0
    for dir in /usr/lib /usr/local/lib /opt/lib /opt/local/lib ; do
        lib_version=`ls -1 $dir 2>/dev/null | grep "$LIB_REGEX" | grep -o "$LIBV_REGEX" | sort -r | head -1`
        if test "m_$lib_version" != "m_" ; then
            # got one!
            m_LDFLAGS="-L$dir"
			m_LIB="-l${LIB_NAME}-$lib_version"
            break;
        fi  
    done

    if test "m_$lib_version" == "m_" ; then
		# no installed version found - scan parent directories
		restore_dir=$(pwd)
		curr_dir=$(pwd)
		success="0"
		while [ test "$curr_dir" != "/" && test "$success" = "0" ] ; do
			# look for a folder named "lib${LIB_NAME}"
			dir="${curr_dir}/lib${LIB_NAME}"
			if test -d "$dir" ; then
				# check if there is a compiled library
				lib_version=`ls -1 ${dir}/.libs 2>/dev/null | grep "$LIB_REGEX" | grep -o "$LIBV_REGEX" | sort -r | head -1`
				if test "m_$lib_version" != "m_" ; then
					# check for headers
					if test -d "${dir}/${LIB_NAME}" ; then
						success="1"
						m_CPPFLAGS="-I$dir"
						m_LDFLAGS="-L$dir/.libs"
						m_LIB="-l${LIB_NAME}-$lib_version"
					fi
				fi
			fi
			if test "$success" = "0" ; then
				cd $curr_dir
				cd ..
				curr_dir=$(pwd)
			fi
			# restore environment
		done
		cd $restore_dir
		if test "$success" = "0" ; then
			AC_MSG_ERROR(lib${LIB_NAME} not found!)
		fi
	else
	    # check for installed headers
	    for dir in /usr/include /usr/local/include /opt/include /opt/local/include ; do
	        # check if the acedia.hpp header could be found
	        include_path="${dir}/${LIB_NAME}-${lib_version}"
			if test -d "${include_path}/${PROJECT_NAME}/" ; then
	            m_CPPFLAGS="-I${include_path}"
	            break;
	        fi  
	    done
	fi

    # check if cpp and ld flags are set
    if test "$m_CPPFLAGS" = "" ; then
        AC_MSG_ERROR(no installed headers found!)
    fi

	HAMCAST_CPPFLAGS="$m_CPPFLAGS"
	HAMCAST_LDFLAGS="$m_LDFLAGS"
	HAMCAST_LIB="$m_LIB"

	AC_SUBST(HAMCAST_CPPFLAGS)
	AC_SUBST(HAMCAST_LDFLAGS)
	AC_SUBST(HAMCAST_LIB)
	AC_DEFINE(HAVE_HAMCAST,,[define if the acedia library is available])
	AC_MSG_RESULT(yes; version $lib_version; path: $m_LDFLAGS)

])
