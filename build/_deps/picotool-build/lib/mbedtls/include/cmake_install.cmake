# Install script for directory: /data/programming/pico-sdk/lib/mbedtls/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/data/programming/hello-world-pico2/build/_deps")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE MESSAGE_NEVER PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/aes.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/aesni.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/arc4.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/aria.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/asn1.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/asn1write.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/base64.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/bignum.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/blowfish.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/bn_mul.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/camellia.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ccm.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/certs.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/chacha20.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/chachapoly.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/check_config.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/cipher.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/cipher_internal.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/cmac.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/compat-1.3.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/config.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/config_psa.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/constant_time.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ctr_drbg.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/debug.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/des.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/dhm.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ecdh.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ecdsa.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ecjpake.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ecp.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ecp_internal.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/entropy.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/entropy_poll.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/error.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/gcm.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/havege.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/hkdf.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/hmac_drbg.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/md.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/md2.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/md4.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/md5.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/md_internal.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/memory_buffer_alloc.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/net.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/net_sockets.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/nist_kw.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/oid.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/padlock.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/pem.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/pk.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/pk_internal.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/pkcs11.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/pkcs12.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/pkcs5.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/platform.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/platform_time.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/platform_util.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/poly1305.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/psa_util.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ripemd160.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/rsa.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/rsa_internal.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/sha1.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/sha256.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/sha512.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ssl.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ssl_cache.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ssl_ciphersuites.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ssl_cookie.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ssl_internal.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/ssl_ticket.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/threading.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/timing.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/version.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/x509.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/x509_crl.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/x509_crt.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/x509_csr.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/mbedtls/xtea.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/psa" TYPE FILE MESSAGE_NEVER PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_builtin_composites.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_builtin_primitives.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_compat.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_config.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_driver_common.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_driver_contexts_composites.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_driver_contexts_primitives.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_extra.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_platform.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_se_driver.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_sizes.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_struct.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_types.h"
    "/data/programming/pico-sdk/lib/mbedtls/include/psa/crypto_values.h"
    )
endif()

