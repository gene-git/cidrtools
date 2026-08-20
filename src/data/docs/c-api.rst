.. c-code-api

=========================
Cidrtools C API Reference
=========================

Data Structures
===============
.. c:autodoc:: ./lib/cidrtools.h

.. raw:: latex

   \clearpage
   \markboth{}{}

Core Functions
==============

.. raw:: latex

   \makeatletter
   \def\@dotsep{4.5}
   \makeatother

.. contents::
   :local:
   :depth: 1

Check If CIDR contained Contained within Other CIDR(s)
------------------------------------------------------

.. c:autodoc:: lib/cidr_contains_cidr.c
.. c:autodoc:: lib/cidr_is_subnet.c
.. c:autodoc:: lib/cidr_contains_ip.c

Get Host Bits of an IP Address
------------------------------
.. c:autodoc:: lib/cidr_host_bits.c

Exract the IP Address and Prefix from a CIDR
--------------------------------------------
.. c:autodoc:: lib/cidr_parts.c

Clean Up the Host Bits and Prefix
---------------------------------
.. c:autodoc:: lib/clean_cidrs.c

Compact A List of CIDRs to the Minimal Number
---------------------------------------------
.. c:autodoc:: lib/compact.c

DNS Convenience
---------------
.. c:autodoc:: lib/dns_ptr.c
.. c:autodoc:: lib/dns_forward.c

Exclude one List of CIDRs from another list of CIDRs
----------------------------------------------------
.. c:autodoc:: lib/exclude_cidrs.c

IP Family Checks
----------------
.. c:autodoc:: lib/family_checks.c

Determine The IP That Is Num IPs After Another IP
-------------------------------------------------
.. c:autodoc:: lib/ip_address_increment.c

Convert CIDR / IP To / From Strings
-----------------------------------
.. c:autodoc:: lib/ip_cidr_to_str.c
.. c:autodoc:: lib/str_to_cidr_block.c
.. c:autodoc:: lib/str_to_ip_addr.c

Get the Number of IPs In a CIDR block
-------------------------------------
.. c:autodoc:: lib/num_ips.c

CIDR To/From IP Ranges 
----------------------
.. c:autodoc:: lib/range_to_cidrs.c
.. c:autodoc:: lib/cidr_to_range.c
.. c:autodoc:: lib/ip_address_range.c

Set CIDR Prefix
---------------
.. c:autodoc:: lib/set_prefix.c

Sort A List Of CIDRs
--------------------
.. c:autodoc:: lib/sort.c

Split Cidrs into separate IPv4 and IPv6
---------------------------------------
.. c:autodoc:: lib/split_by_family.c

Split CIDR Into Smaller Subnets
-------------------------------
.. c:autodoc:: lib/subnets_split.c

Memory Management
-----------------
.. c:autodoc:: lib/cidrs.c

Get Cidrtools Version
---------------------
.. c:autodoc:: lib/version.c

.. raw:: latex

   \clearpage

