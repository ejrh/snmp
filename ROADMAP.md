Roadmap
=======

Proposed milestones
-------------------

  * 0.1 - Reliably poll a set of hosts and OIDs, and write the results to standard output.

  * 1.0 - Efficiently poll hosts and OIDs, and optionally write results to a database.

Upcoming features
-----------------

  * Support for a greater variety of SNMP data types.

  * Log to database (initially SQLite).

Performance ideas
-----------------

  * Keep config items in a priority queue, ordered by next poll time.
  
  * Group multiple requests to the same host in a single message.
  
  * Reuse a message that sends the same set of requests to multiple hosts.
  
  * Also reuse a message that sends the same set of requests at a later time.
  
  * Cache network objects such as addresses.
  
  * Handle incoming responses in a separate thread/process.

  * Optimise ASN.1 build and parse routines.
