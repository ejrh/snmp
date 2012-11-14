SNMP Poller
===========

This program periodically polls a set of SNMP agents for the values of certain
OIDs.  The responses are logged to a file or database.

Current limitations
------------------

The results are printed to standard output, and not logged to a database.
The first database to be supported will be someting like SQLite.

There's a seeming bug where the first two requests are sent in quick
succession, before the specified delay starts being used.

Only ASN.1 values of type string, integer, and null, are supported.  Responses
containing other values can't be parsed properly.  General ASN.1 values should
be supported.

The polling isn't especially efficient: no priority queues, and no batching
of requests to the same host.  Additionally, host name and port are used each
time a request is sent, rather than a cached `sockaddr` object.

Range queries have some support, but the poller will only iterate one key at
a time, will not stop when the end of the range is reached, and may
allow overlapping queries for the same range at a time.


Command line
------------

    poller [options]

Options include:

    -v            verbose mode
    -p PORT       port to listen for replies on
    -c FILENAME   config file to use

The list of hosts and OIDs to poll is read from the config file, which defaults
to `poller.conf`.  If this file cannot be read, the poller will act as if an
empty file is specified.

The poller will reread the config file when it receives a SIGQUIT signal.


Configuration file
------------------

The configuration file is a plain text file, with fields separated by white
space.  Each line is of the form:

    hostname[:port]    oid    frequency

Hostname is a host name or address; port is optional and defaults to 161.
OID is an OID in numeric form.  Frequency is a positive value and represents
the delay in seconds between successive requests for that OID from that host.

If OID is of the form `a.*`, then the line specifies a range query.  All
OIDs with the prefix `a.` will be queried for.

Text after the `#` character is treated as a comment, and blank lines are
ignored.

See `sample.conf` for an example configuration file.
