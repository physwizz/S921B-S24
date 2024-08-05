################################################################################
1. Download and unzip the kernel source of S921BXXU1AXCA.

2. Unzip and update the kernel source of S921BXXS2AXD6.

3. How to Build
        - get Toolchain
                get the proper toolchain packages from AOSP or Samsung Open Source or ETC.
                
                (1) AOSP Kernel
                https://source.android.com/docs/setup/build/building-kernels
                $ repo init -u https://android.googlesource.com/kernel/manifest -b common-android14-6.1
                $ repo sync
                
                (2) Samsung Open Source
                https://opensource.samsung.com/uploadSearch?searchValue=toolchain
                
                copy the following list to the root directory
                - build/
                - external/
                - prebuilts/
                - tools/

        - to Build
                $ tools/bazel run --nocheck_bzl_visibility --config=stamp --sandbox_debug --verbose_failures --debug_make_verbosity=I //projects/s5e9945:s5e9945_user_dist

4. Output files
        - out/s5e9945_user/dist
################################################################################
