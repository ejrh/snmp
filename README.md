SNMP Poller
===========

This program periodically polls a set of SNMP agents for the values of certain
OIDs.  The responses are logged to a file or database.

Current limitations
------------------

The hostname:port argument from the command line is used for all requests,
rather than the host names from the configuration file.  This argument
should be removed and the configuration file values used.

The results are printed to standard output, and not logged to a database.
The first database to be supported will be someting like SQLite.

There's a seeming bug where the first two requests are sent in quick
succession, before the specified delay starts being used.

Only ASN.1 values of type string, integer, and null, are supported.  Responses
containing other values can't be parsed properly.  General ASN.1 values should
be supported.

The polling isn't especially efficient: no priority queues, and no batching
of requests to the same host.


Command line
------------

poller [options] HOSTNAME[:PORT]

Options include:

    -p PORT   port to listen on
    -v        verbose mode

Configuration file
------------------

The configuration file is a plain text file, with fields separated by white
space.  Each line is of the form:

    hostname[:port]    oid    frequency

Hostname is a host name or address; port is optional and defaults to 161.
OID is an OID in numeric form.  Frequency is a positive value and represents
the delay in seconds between successive requests for that OID from that host.

Text after the # character is treated as a comment, and blank lines are
ignored.

See sample.conf for an example configuration file.
