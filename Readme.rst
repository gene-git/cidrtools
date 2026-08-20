=========
cidrtools
=========


Overview
========

Cidrtools is a library with a collection of tools that examine and manipulate 
network cidr blocks.  The functions cover many common tasks and are designed to be extremely fast.
These can be called directly from C as well as other languages that can bind to compiled 
shared libraries.

The source is available in `Github cidrtools <https://github.com/gene-git/cidrtools>`_
as well as the `Arch AUR <https://aur.archlinux.org/packages/py-cidr>`_.

There is a companion `cidrtools-cffi <https://github.com/gene-git/cidrtools-cffi>`_ 
package providing Python bindings to the library.
This module is significantly faster than a pure Python implementation using the *ipaddr* module.

hostcheck application
=====================

The package includes the *hostcheck* application which uses the library. 

It takes a cidr block as an argument and prints a sample of IP addresses 
that are taken from the range of IPs spanned by the cidr block.

It also displays the hostname of each IP if DNS provides one.

The application ues three cidrtool functions:

* ct_cidr_to_range() - get the first and last IP in the cidr block
* ct_ip_str_to_hostname()  - DNS PTR lookup.
* ct_ip_address_increment() - Find the IP address a number of IPs past an IP.

Source is in the *apps* directory and a man page is also provided.

Compact Benchmark
=================

This benchmark has each implementation *compact* the same
random set of 100,000 cidr blocks. This is repeated with different random sets.
Some sets compact a lot and some very little.

While there is variation across trial sets, results are pretty consistent.
The actual time to compact the set of cidr blocks using CFFI bindings is 
close to the raw C-code. The remainder of the time in the Python module
is the overhead crossing from Python to C and back. The overhead is in around 35% or so.
In absolute time this is still pretty small.

For example to compact one set 100,000 cidr blocks that compacts down
to 54,941 cidr blocks::

    C-code/compact only     0.012630286 seconds
    Cidrtools-CFFI          0.025331798 seconds
    Python/ipaddress        0.937566086 seconds     37 x slower

The take away, from a performance perspective, is to use compiled code and minimize 
the role Python itself plays in some performance sensitive areas. 

One alternative to CFFI is using Cython code directly. This produces it's own compiled
C-code and also has a significant performance boost over pure Python, but we found this approach 
complicated, brittle and less maintainable than having core functions
written in pure C. 

Cython can also be used to bind to the library as can Py03.

Code Checks
===========

The static code checker is run using:

.. code-block:: bash

   ./scripts/check-source

The unit test suite can be run with or without *valgrind*.
In both caes all tests should complete without any errors.

.. code-block:: bash

   ./scripts/do-build debug
   ./scripts/run-tests valgrind

To run the tests standard tests without using valgrind, do:

.. code-block:: bash

   ./scripts/do-build 
   ./scripts/run-tests 


Recent Changes
==============

**1.1.0**

* Bug fix: Original code assumed excluded cidrs are subnets. 
  Enhance code to handle excluded networks being supernets. 

AI Tooling
==========
    
Assisted by:
    
* Anthropic's `Claude <https://claude.ai>`_

* Google's `Gemini <https://gemini.google.com/>`_


.. rubric:: Footnotes

