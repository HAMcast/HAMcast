# SYNOPSIS
#
#   AX_OPENSSL()
#
# DESCRIPTION
#
#   Test for OpenSSL library
#
# LAST MODIFICATION
#
#   2008-12-11

AC_DEFUN([AX_OPENSSL],
[
  AC_CHECK_HEADERS(openssl/ssl.h,,
    [AC_MSG_ERROR([OpenSSL headers not found (OpenSSL, http://openssl.org)])])
    
  AC_CHECK_LIB([ssl],[SSL_CTX_set_tmp_rsa_callback],,
    [AC_MSG_ERROR([OpenSSL library not found (OpenSSL, http://openssl.org)])])
])
