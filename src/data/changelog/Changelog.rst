Changelog
=========

Tags
====

.. code-block:: text

	0.1.0 (2026-08-05) -> HEAD (2026-08-20)
	97 commits.

Commits
=======


* 2026-08-20  : **HEAD**

.. code-block:: text

              - Move unit tests/ dir into src/

* 2026-08-20  : **0.9.0**

.. code-block:: text

              - prep for next releae
              - More tidying of unit tests
 2026-08-19   ⋯

.. code-block:: text

              - Clean up code in tests
              - Want code to be lint clean and valgrind clean
              - fix doc generator with correct hawkmoth args got C23 code
 2026-08-18   ⋯

.. code-block:: text

              - readme
 2026-08-17   ⋯

.. code-block:: text

              - change error message when dropping prefix <= 4
              - small clean up of compact - remove unused omp multi threaded func
              - add more protection for small prefixes in compact
              - snap
 2026-08-16   ⋯

.. code-block:: text

              - skip any cidr with prefix 4 or less in str_to_cidr_block()
              - strtok -> strtok_r in ct_flat_buffer_to_cidrs(). Also zero malloced memory
               - compact - performance - only re-sort if needed in loop
              - sort/compact fixes
              - Change ct_flat_buffer_to_cidrs() to now skip any bad cidr strings instead of returning an error - caller can check the number in versus number returned to know if any were dropped. More robust
              - Update readme/manual
              - update manual
              - reduce malloc/free in cidrs_to_flat_buffer.c by using ct_cidr_to_str_r with fixed buffer
              - Make openmp optional - only ct_cidrs_to_flat_buffer() currently has mp option
 2026-08-15   ⋯

.. code-block:: text

              - stop using omp multi-threading - need to hide the code and take out omp dep in meson.build still
              - use omp in flat_buffer_to_cidrs.c
              - Fix mem leakin split_by_family test
                Add 2 new tests tests/t_cidrs_to_flat_buffer.c tests/t_flat_buffer_to_cidrs.c
              - add the new code to repo
              - add flat_buffer_cidrs() / cidrs_to_flat_buffer()
              - add ct_cidrs_to_str_array()
              - Add ct_str_array_to_cidrs()
              - add tests/t_cidr_to_range_mid.c to the repo
              - Add new ct_cidr_to_range_mid() - now computes first, middle and last ip in cidr
 2026-08-14   ⋯

.. code-block:: text

              - move version.txt to src/
 2026-08-10   ⋯

.. code-block:: text

              - doc updates
              - update manual
              - Comment tweaks
              - small readme tweak
              - Add new : split_by_family()
              - update manual
              - Add ct_split_by_iptype
              - rename add_cidr_to_cidrs() to ct_add_cidr_to_cidrs()
                Add new function: ct_allocate_cidrs()
              - oops - forgot to new test to repo
              - Add ct_clean_cidr()
 2026-08-09   ⋯

.. code-block:: text

              - tidy unit test code
              - snap
              - subtle 128 bit boundary in range_to_cidrs_v6.c
              - snap
              - snap
              - cleanup continues
              - clean snap
              - more clean ups
              - lint snap
              - cleaning up range_to_cidrs - v6 needs a little more work still
              - clean num_ips
              - change ip/cidr to string to return int for caller provided buffer versions
              - lint snap
              - our own headers should be first to be included
               - more of same
              - more dns_xx tidy ups
              - clean up dns
              - add unit test for ip_str_to_hostname.c
              - more clean ups
 2026-08-08   ⋯

.. code-block:: text

              - cidr_parts - handle trailing white space also cidr ip/xx or just an ip (no prefix)
              - clean up cidr_parts.c
              - clean snap
              - clean cidr_host_bits.c
              - lint picking
              - cidr_parts - accept trailing witespace again
              - more clean ups
              - performance improvements
                Remove some code duplication
              - optimize cidr_contains_cidr.c cidr_fix_host_bits.c cidr_to_range.c
                  Use 32 but network blocks instead of 8 bit - faster!
              - cleanup continues - snap
              - Add script to rebuild the manual
              - impreove manual makefile to track changes instead of always rebuildind the pdf
              - Share ct_cidr_contains_cidr() which is now public function - avoid duplication
              - doc gene to gene c
 2026-08-07   ⋯

.. code-block:: text

              - Update meson to install the new cidrtools.pdf into /usr/share/cidrtools
              - pdf docs now ok - html not working - only care about pdf anyway
              - snap doc fixes
              - manual is working - snap it
              - use libertine font for pdf manual
              - Add sphinx doc comments to all public .c and .h files
                Fixup sphinx doc builder for PDF
 2026-08-06   ⋯

.. code-block:: text

              - add unit test : t_version

* 2026-08-06  : **0.5.0**

.. code-block:: text

              - prep release
              - do-build as new argument to use pre-2013 cpu.
                Add packaging/PKGBUILD
              - Rename the 3 base structs: CtAddress, CtCidr and CtCidrs
              - snap
              - snap - next step is rename our ip and cidr structures with better names
              - fix couple missed spots converting all function names to ct_xxx()
              - change function names to all have leading ct_ to ensure they are unique
              - install header files
              - snap
 2026-08-05   ⋯

.. code-block:: text

              - snap after adding src/

* 2026-08-05  : **0.1.0**

.. code-block:: text

              - Tidy up man page for hostcheck
                Tidy up hostcheck app
              - New version of hostcheck app build on the cidrtools library
              - snap
              - snap
              - snap
              - refactor compact.c
              - initial snap


