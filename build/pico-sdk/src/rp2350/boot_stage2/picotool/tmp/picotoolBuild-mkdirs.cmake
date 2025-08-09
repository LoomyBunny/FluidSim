# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/data/programming/hello-world-pico2/build/_deps/picotool-src"
  "/data/programming/hello-world-pico2/build/_deps/picotool-build"
  "/data/programming/hello-world-pico2/build/_deps"
  "/data/programming/hello-world-pico2/build/pico-sdk/src/rp2350/boot_stage2/picotool/tmp"
  "/data/programming/hello-world-pico2/build/pico-sdk/src/rp2350/boot_stage2/picotool/src/picotoolBuild-stamp"
  "/data/programming/hello-world-pico2/build/pico-sdk/src/rp2350/boot_stage2/picotool/src"
  "/data/programming/hello-world-pico2/build/pico-sdk/src/rp2350/boot_stage2/picotool/src/picotoolBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/data/programming/hello-world-pico2/build/pico-sdk/src/rp2350/boot_stage2/picotool/src/picotoolBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/data/programming/hello-world-pico2/build/pico-sdk/src/rp2350/boot_stage2/picotool/src/picotoolBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
