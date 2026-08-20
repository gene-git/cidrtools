==========
hostcheck
==========

-----------------------
Fast DNS reverse lookup
-----------------------

:Author: Gene C <arch@sapience.com>
:Manual section: 1
:Manual group: Linux Tools
:Date: @DATE@
:Version: @VERSION@

SYNOPSIS
========

**hostcheck** [*options* ...] <cidr>

DESCRIPTION
===========

hostcheck is part of the *cidrtools* package.

It does DNS reverse IP (aka PTR) lookup for IPv4 or IPv6 IP address or cidr blocks.

It returns a list of IP addresses along with each hostname, if available. 

It shows a 20 by defalt, and this can be changed using the command line option *-n num*.
The total number output is capped at 256 IP addresses.

OPTIONS
=======

--help, -h         Show help message and exit.
--num  num         Number of hosts to print from range


EXAMPLES
========

Look up hosts for sample of /24::

    hostcheck -n 10 64.233.180.0/24

or::

    hostcheck 2607:f8b0:4004:c27::8b/64


SEE ALSO
========

dig(1), host(1) 

