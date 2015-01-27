# SYNOPSIS
#
#   AX_BLUETOOTH()
#
# DESCRIPTION
#
#   Test for the Bluez Bluetooth library. The library is optional!
#
# LAST MODIFICATION
#
#   2009-07-13

AC_DEFUN([AX_BLUETOOTH],
[
  AC_CHECK_HEADERS(bluetooth/bluetooth.h,, )
  AC_CHECK_LIB([bluetooth],[sdp_connect],, )
])
