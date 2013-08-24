MADlib Port
===========

This project brings in-DBMS data analytics to Impala. This leverages previous
work done by two projects:

  - MADlib (http://madlib.net/)
  - Bismarck (http://hazy.cs.wisc.edu/hazy/victor/bismarck/)

Each of these projects use User Defined Aggregates (UDAs) to train analytic
models using an existing DBMS's data management and processing ability. 


Code Base
===========
This code base includes the following components.

MADlib
-----------
There is a fork of MADlib 1.0 which has been modified for use with impala.
The specific changes were:
  - madlib/test with tests for the new code
  - madlib/Makefile to make the tests
  - madlib/src/ports/metaport which is a modified MADlib backend for main memory


