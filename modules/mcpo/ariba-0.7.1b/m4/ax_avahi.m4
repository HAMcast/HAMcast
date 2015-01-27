# SYNOPSIS
#
#   AX_AVAHI()
#
# DESCRIPTION
#
#   Test for the Avahi library. The library is optional!
#
# LAST MODIFICATION
#
#   2009-07-03

AC_DEFUN([AX_AVAHI],
[
  AC_CHECK_HEADERS(avahi-client/client.h,, )
    
  AC_CHECK_HEADERS(avahi-common/timeval.h,, )
    
  AC_CHECK_LIB([avahi-client],[avahi_entry_group_new],, )
    
  AC_CHECK_LIB([avahi-common],[avahi_free],, )
])
