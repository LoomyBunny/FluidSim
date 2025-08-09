# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/data/programming/pico-sdk/tools/pioasm"
  "/data/programming/hello-world-pico2/build/pioasm"
  "/data/programming/hello-world-pico2/build/pioasm-install"
  "/data/programming/hello-world-pico2/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/tmp"
  "/data/programming/hello-world-pico2/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp"
  "/data/programming/hello-world-pico2/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src"
  "/data/programming/hello-world-pico2/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/data/programming/hello-world-pico2/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/data/programming/hello-world-pico2/build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
