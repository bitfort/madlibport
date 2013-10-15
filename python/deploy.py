import os
import sys

import impala_util as iutil


def doit(cmd, mayfail=False):
  if os.system(cmd):
    print 'FAILED'
    if not mayfail:
      raise SystemExit

def impala(query, mayfail=False):
  doit('impala-shell -q "%s"' % query, mayfail=mayfail)

# compile all of our files
doit("make -B all")


libs = [
    ('lib/libsvm.so', 'libsvm.so'),
    ('lib/libbismarckarray.so', 'bisarray.so')
    ]


for lb, tar in libs:
  doit('hadoop fs -rm /user/cloudera/%s' % tar, mayfail=True)
  doit('hadoop fs -put %s /user/cloudera/%s' % (lb, tar))



queries = [
    #
    # Utilities
    #
    "DROP FUNCTION arrayget(bigint, string)",
    "CREATE FUNCTION arrayget(bigint, string) returns double location '/user/cloudera/bisarray.so' SYMBOL='ArrayGet'",

    "DROP FUNCTION toarray(double...);",
    "create FUNCTION toarray(double...) returns string location '/user/cloudera/bisarray.so' SYMBOL='_Z7ToArrayPN10impala_udf15FunctionContextEiPNS_9DoubleValE';",

    "DROP FUNCTION allbytes();",
    "create FUNCTION allbytes() returns string location '/user/cloudera/bisarray.so' SYMBOL='AllBytes';",

    #
    # SVM
    #
    "DROP aggregate function svm(string, string, boolean, double, double);",
    "create aggregate function svm(string, string, boolean, double, double) returns string location '/user/cloudera/libsvm.so' UPDATE_FN='SVMUpdate';"

    "DROP function svmpredict(string, string);",
    "create function svmpredict(string, string) returns boolean location '/user/cloudera/libsvm.so' SYMBOL='SVMPredict';"

    "DROP function svmloss(string, string, boolean);",
    "create function svmloss(string, string, boolean) returns double location '/user/cloudera/libsvm.so' SYMBOL='SVMLoss';"
    ]



iutil.impala_shell_exec(queries, 'toysvm')
