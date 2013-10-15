import os
import sys
import tempfile


def make_model_table(name):
  ''' Creates a query to create and initialize an empty state table
  This tables stores the contents of the model after each iteration
  '''
  q = 'DROP TABLE IF EXISTS %s; CREATE TABLE %s (one int, iter int, model string) stored as PARQUETFILE;' % (name, name)
  q += "INSERT INTO %s VALUES (1, 0, '');" % name
  return q


def bismarck_query(slct, model_table, data_table, epoch):
  slct = bismarck_inject_model(slct, model_table)
  q = 'SELECT %(slct)s FROM %(hist)s, %(dat)s ' \
    'WHERE %(dat)s.one=%(hist)s.one AND %(hist)s.iter=%(epoch)d;' % \
    {'epoch': epoch, 'hist': model_table, 'dat': data_table, 'slct': slct}
  return q

def bismarck_inject_model(qry, model_table):
  return qry.replace('__PREV_MODEL__', '%s.model' % model_table)

def bismarck_epoch(model_table, data_table, uda_gen, epoch):
  ''' Preforms a bismarck epoch using the previous model to initialize
  @param model_table: name of the table that stores the model
                      (see make_model_table)
  @param data_table: the table that stores the examples to iterate over
  @param uda_str: has a __PREV_MODEL__ which will be replaced,
                  e.g. "svm_uda(__PREV_MODEL__, items.foo, items.bar, 0.5)"
  '''
  uda = bismarck_inject_model(uda_gen, model_table)
  q = 'INSERT INTO %(hist)s SELECT 1, %(epoch)d, %(uda)s FROM %(hist)s, %(dat)s ' \
    'WHERE %(dat)s.one=%(hist)s.one AND %(hist)s.iter=%(epoch)d - 1;' % \
    {'epoch': epoch, 'hist': model_table, 'dat': data_table, 'uda': uda, 'dat':data_table}
  return q



def doit(cmd, mayfail=False):
  if os.system(cmd):
    print 'FAILED'
    if not mayfail:
      raise SystemExit

def impala(query, mayfail=False):
  doit('impala-shell -q "%s"' % query, mayfail=mayfail)

def impala_shell_exec(queries, database=None, shell='impala-shell'):
  ''' Takes a list of queries and executes them against impala
  This uses the impala shell
  '''
  for q in queries:
    cmd = q + ';'
    if database is not None:
      cmd = ('use %s; ' % database) + cmd
    impala(cmd, mayfail=True)


