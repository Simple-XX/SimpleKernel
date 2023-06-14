RISC-V Open Source Supervisor Binary Interface (OpenSBI)
========================================================

Copyright and License
---------------------

The OpenSBI project is copyright (c) 2019 Western Digital Corporation
or its affiliates and other contributors.

It is distributed under the terms of the BSD 2-clause license
("Simplified BSD License" or "FreeBSD License", SPDX: *BSD-2-Clause*).
A copy of this license with OpenSBI copyright can be found in the file
[COPYING.BSD].

All source files in OpenSBI contain the 2-Clause BSD license SPDX short
identifier in place of the full license text.

```
SPDX-License-Identifier:    BSD-2-Clause
```

This enables machine processing of license information based on the SPDX
License Identifiers that are available on the [SPDX] web site.

OpenSBI source code also contains code reused from other projects as listed
below. The original license text of these projects is included in the source
files where the reused code is present.

* The libfdt source code is disjunctively dual licensed
  (GPL-2.0+ OR BSD-2-Clause). Some of this project code is used in OpenSBI
  under the terms of the BSD 2-Clause license. Any contributions to this
  code must be made under the terms of both licenses.

See also the [third party notices] file for more information.

Introduction
------------

The **RISC-V Supervisor Binary Interface (SBI)** is the recommended interface
between:

1. A platform-specific firmware running in M-mode and a bootloader, a
   hypervisor or a general-purpose OS executing in S-mode or HS-mode.
2. A hypervisor running in HS-mode and a bootloader or a general-purpose OS
   executing in VS-mode.

The *RISC-V SBI specification* is maintained as an independent project by the
RISC-V Foundation on [Github].

The goal of the OpenSBI project is to provide an open-source reference
implementation of the RISC-V SBI specifications for platform-specific firmwares
executing in M-mode (case 1 mentioned above). An OpenSBI implementation can be
easily extended by RISC-V platform and system-on-chip vendors to fit a
particular hardware configuration.

The main component of OpenSBI is provided in the form of a platform-independent
static library **libsbi.a** implementing the SBI interface. A firmware or
bootloader implementation can link against this library to ensure conformance
with the SBI interface specifications. *libsbi.a* also defines an interface for
integrating with platform-specific operations provided by the platform firmware
implementation (e.g. console access functions, inter-processor interrupt
control, etc).

To illustrate the use of the *libsbi.a* library, OpenSBI also provides a set of
platform-specific support examples. For each example, a platform-specific
static library *libplatsbi.a* can be compiled. This library implements
SBI call processing by integrating *libsbi.a* with the necessary
platform-dependent hardware manipulation functions. For all supported platforms,
OpenSBI also provides several runtime firmware examples built using the platform
*libplatsbi.a*. These example firmwares can be used to replace the legacy
*riscv-pk* bootloader (aka BBL) and enable the use of well-known bootloaders
such as [U-Boot].

Supported SBI version
---------------------
Currently, OpenSBI fully supports SBI specification *v0.2*. OpenSBI also
supports Hart State Management (HSM) SBI extension starting from OpenSBI v0.7.
HSM extension allows S-mode software to boot all the harts a defined order
rather than legacy method of random booting of harts. As a result, many
required features such as CPU hotplug, kexec/kdump can also be supported easily
in S-mode. HSM extension in OpenSBI is implemented in a non-backward compatible
manner to reduce the maintenance burden and avoid confusion. That's why, any
S-mode software using OpenSBI will not be able to boot more than 1 hart if HSM
extension is not supported in S-mode.

Linux kernel already supports SBI v0.2 and HSM SBI extension starting from
**v5.7-rc1**. If you are using an Linux kernel older than **5.7-rc1** or any
other S-mode software without HSM SBI extension, you should stick to OpenSBI
v0.6 to boot all the harts. For a UMP systems, it doesn't matter.

N.B. Any S-mode boot loader (i.e. U-Boot) doesn't need to support HSM extension,
as it doesn't need to boot all the harts. The operating system should be
capable enough to bring up all other non-booting harts using HSM extension.

Required Toolchain and Packages
-------------------------------

OpenSBI can be compiled natively or cross-compiled on a x86 host. For
cross-compilation, you can build your own toolchain, download a prebuilt one
from the [Bootlin toolchain repository] or install a distribution-provided
toolchain; if you opt to use LLVM/Clang, most distribution toolchains will
support cross-compiling for RISC-V using the same toolchain as your native
LLVM/Clang toolchain due to LLVM's ability to support multiple backends in the
same binary, so is often an easy way to obtain a working cross-compilation
toolchain.

Basically, we prefer toolchains with Position Independent Executable (PIE)
support like *riscv64-linux-gnu-gcc*, *riscv64-unknown-freebsd-gcc*, or
*Clang/LLVM* as they generate PIE firmware images that can run at arbitrary
address with appropriate alignment. If a bare-metal GNU toolchain (e.g.
*riscv64-unknown-elf-gcc*) is used, static linked firmware images are
generated instead. *Clang/LLVM* can still generate PIE images if a bare-metal
triple is used (e.g. *-target riscv64-unknown-elf*).

Please note that only a 64-bit version of the toolchain is available in
the Bootlin toolchain repository for now.

In addition to a toolchain, OpenSBI also requires the following packages on
the host:

1. device-tree-compiler: The device tree compiler for compiling device
   tree sources (DTS files).
2. python3: The python 3.0 (or compatible) language support for various
   scripts.

Building and Installing the OpenSBI Platform-Independent Library
----------------------------------------------------------------

The OpenSBI platform-independent static library *libsbi.a* can be compiled
natively or it can be cross-compiled on a host with a different base
architecture than RISC-V.

For cross-compiling, the environment variable *CROSS_COMPILE* must be defined
to specify the name prefix of the RISC-V compiler toolchain executables, e.g.
*riscv64-linux-gnu-* if the gcc executable used is *riscv64-linux-gnu-gcc*.

To build *libsbi.a* simply execute:
```
make
```

All compiled binaries as well as the resulting *libsbi.a* static library file
will be placed in the *build/lib* directory. To specify an alternate build root
directory path, run:
```
make O=<build_directory>
```

To generate files to be installed for using *libsbi.a* in other projects, run:
```
make install
```

This will create the *install* directory with all necessary include files
copied under the *install/include* directory and the library file copied into
the *install/lib* directory. To specify an alternate installation root
directory path, run:
```
make I=<install_directory> install
```

Building and Installing a Reference Platform Static Library and Firmware
------------------------------------------------------------------------

When the *PLATFORM=<platform_subdir>* argument is specified on the make command
line, the platform-specific static library *libplatsbi.a* and firmware examples
are built for the platform *<platform_subdir>* present in the directory
*platform* in the OpenSBI top directory. For example, to compile the platform
library and the firmware examples for the QEMU RISC-V *virt* machine,
*<platform_subdir>* should be *generic*.

To build *libsbi.a*, *libplatsbi.a* and the firmware for one of the supported
platforms, run:
```
make PLATFORM=<platform_subdir>
```

An alternate build directory path can also be specified:
```
make PLATFORM=<platform_subdir> O=<build_directory>
```

The platform-specific library *libplatsbi.a* will be generated in the
*build/platform/<platform_subdir>/lib* directory. The platform firmware files
will be under the *build/platform/<platform_subdir>/firmware* directory.
The compiled firmwares will be available in two different formats: an ELF file
and an expanded image file.

To install *libsbi.a*, *libplatsbi.a*, and the compiled firmwares, run:
```
make PLATFORM=<platform_subdir> install
```

This will copy the compiled platform-specific libraries and firmware files
under the *install/platform/<platform_subdir>/* directory. An alternate
install root directory path can be specified as follows:
```
make PLATFORM=<platform_subdir> I=<install_directory> install
```

In addition, platform-specific configuration options can be specified with the
top-level make command line. These options, such as *PLATFORM_<xyz>* or
*FW_<abc>*, are platform-specific and described in more details in the
*docs/platform/<platform_name>.md* files and
*docs/firmware/<firmware_name>.md* files.

All OpenSBI platforms support Kconfig style build-time configuration. Users
can change the build-time configuration of a platform using a graphical
interface as follows:
```
make PLATFORM=<platform_subdir> menuconfig
```

Alternately, an OpenSBI platform can have multiple default configurations
and users can select a custom default configuration as follows:
```
make PLATFORM=<platform_subdir> PLATFORM_DEFCONFIG=<platform_custom_defconfig>
```

Building 32-bit / 64-bit OpenSBI Images
---------------------------------------
By default, building OpenSBI generates 32-bit or 64-bit images based on the
supplied RISC-V cross-compile toolchain. For example if *CROSS_COMPILE* is set
to *riscv64-linux-gnu-*, 64-bit OpenSBI images will be generated. If building
32-bit OpenSBI images, *CROSS_COMPILE* should be set to a toolchain that is
pre-configured to generate 32-bit RISC-V codes, like *riscv32-linux-gnu-*.

However it's possible to explicitly specify the image bits we want to build with
a given RISC-V toolchain. This can be done by setting the environment variable
*PLATFORM_RISCV_XLEN* to the desired width, for example:

```
export CROSS_COMPILE=riscv64-linux-gnu-
export PLATFORM_RISCV_XLEN=32
```

will generate 32-bit OpenSBI images. And vice vesa.

Building with Clang/LLVM
------------------------

OpenSBI can also be built with Clang/LLVM. To build with just Clang but keep
the default binutils (which will still use the *CROSS_COMPILE* prefix if
defined), override the *CC* make variable with:
```
make CC=clang
```

To build with a full LLVM-based toolchain, not just Clang, enable the *LLVM*
option with:
```
make LLVM=1
```

When using Clang, *CROSS_COMPILE* often does not need to be defined unless
using GNU binutils with prefixed binary names. *PLATFORM_RISCV_XLEN* will be
used to infer a default triple to pass to Clang, so if *PLATFORM_RISCV_XLEN*
itself defaults to an undesired value then prefer setting that rather than the
full triple via *CROSS_COMPILE*. If *CROSS_COMPILE* is nonetheless defined,
rather than being used as a prefix for the executable name, it will instead be
passed via the `--target` option with the trailing `-` removed, so must be a
valid triple.

These can also be mixed; for example using a GCC cross-compiler but LLVM
binutils would be:
```
make CC=riscv64-linux-gnu-gcc LLVM=1
```

These variables must be passed for all the make invocations described in this
document.

NOTE: Using Clang with a `riscv*-linux-gnu` GNU binutils linker has been seen
to produce broken binaries with missing relocations; it is therefore currently
recommended that this combination be avoided or *FW_PIC=n* be used to disable
building OpenSBI as a position-independent binary.

Building with timestamp and compiler info
-----------------------------------------

When doing development, we may want to know the build time and compiler info
for debug purpose. OpenSBI can also be built with timestamp and compiler info.
To build with those info and print it out at boot time, we can just simply add
`BUILD_INFO=y`, like:
```
make BUILD_INFO=y
```

But if you have used `BUILD_INFO=y`, and want to switch back to `BUILD_INFO=n`,
you must do
```
make clean
```
before the next build.

NOTE: Using `BUILD_INFO=y` without specifying SOURCE_DATE_EPOCH will violate
[reproducible builds]. This definition is ONLY for development and debug
purpose, and should NOT be used in a product which follows "reproducible
builds".

Contributing to OpenSBI
-----------------------

The OpenSBI project encourages and welcomes contributions. Contributions should
follow the rules described in the OpenSBI [Contribution Guideline] document.
In particular, all patches sent should contain a Signed-off-by tag.

The [Contributors List] document provides a list of individuals and
organizations actively contributing to the OpenSBI project.

Documentation
-------------

Detailed documentation of various aspects of OpenSBI can be found under the
*docs* directory. The documentation covers the following topics.

* [Contribution Guideline]: Guideline for contributing code to OpenSBI project
* [Library Usage]: API documentation of OpenSBI static library *libsbi.a*
* [Platform Requirements]: Requirements for using OpenSBI on a platform
* [Platform Support Guide]: Guideline for implementing support for new platforms
* [Platform Documentation]: Documentation of the platforms currently supported.
* [Firmware Documentation]: Documentation for the different types of firmware
  examples build supported by OpenSBI.
* [Domain Support]: Documentation for the OpenSBI domain support which helps
  users achieve system-level partitioning using OpenSBI.

OpenSBI source code is also well documented. For source level documentation,
doxygen style is used. Please refer to the [Doxygen manual] for details on this
format.

Doxygen can be installed on Linux distributions using *.deb* packages using
the following command.
```
sudo apt-get install doxygen doxygen-latex doxygen-doc doxygen-gui graphviz
```

For *.rpm* based Linux distributions, the following commands can be used.
```
sudo yum install doxygen doxygen-latex doxywizard graphviz
```
or
```
sudo yum install doxygen doxygen-latex doxywizard graphviz
```

To build a consolidated *refman.pdf* of all documentation, run:
```
make docs
```
or
```
make O=<build_directory> docs
```

the resulting *refman.pdf* will be available under the directory
*<build_directory>/docs/latex*. To install this file, run:
```
make install_docs
```
or
```
make I=<install_directory> install_docs
```

*refman.pdf* will be installed under *<install_directory>/docs*.

[Github]: https://github.com/riscv/riscv-sbi-doc
[U-Boot]: https://www.denx.de/wiki/U-Boot/SourceCode
[Bootlin toolchain repository]: https://toolchains.bootlin.com/
[COPYING.BSD]: COPYING.BSD
[SPDX]: http://spdx.org/licenses/
[Contribution Guideline]: docs/contributing.md
[Contributors List]: CONTRIBUTORS.md
[Library Usage]: docs/library_usage.md
[Platform Requirements]: docs/platform_requirements.md
[Platform Support Guide]: docs/platform_guide.md
[Platform Documentation]: docs/platform/platform.md
[Firmware Documentation]: docs/firmware/fw.md
[Domain Support]: docs/domain_support.md
[Doxygen manual]: http://www.doxygen.nl/manual/index.html
[Kendryte standalone SDK]: https://github.com/kendryte/kendryte-standalone-sdk
[third party notices]: ThirdPartyNotices.md
[reproducible builds]: https://reproducible-builds.org
