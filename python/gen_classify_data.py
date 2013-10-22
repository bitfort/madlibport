import random
import impala_util as iutil

''' Generates a binary classification dataset for SVM or Logistic regression

The parameters for the dataset are:
  N - the number of features (the number of columns, size of model)
  M - the number of exmaples (number of rows in the table)
'''


def generate_soln(n):
  ''' Generates a solution (model) of the given lenght
  Creates a solution vector of the form [-1, 2, -3, 4, -5, ..., n]
  @param n: the number of features to generate
  '''
  return [(x + 1) * ((x % 2) * 2 - 1) for x in xrange(n)]

def dot(x, y):
  ''' Computes the dot product of x and y
  '''
  tot = 0
  for i in xrange(len(x)):
    tot += x[i] * y[i]
  return tot

def predict(sol, ex):
  ''' Predicts the label (true/false) for the given exmaple and solution
  '''
  return dot(sol, ex) >= 0

def generate_example(n):
  '''  Generates a random example of n features
  '''
  return [random.random() * 10 - 5 for x in xrange(n)]

def create_examp_table_query(name, n, lbl='lbl', prefix='e'):
  ''' Creates a query which will create the example table.
  The table will be of the form:
      (lbl boolean, prefix0 double, prefix1 dobule, ...)
  @param name: the name of the table to create
  @param n: number of features
  @param lbl: name of label column
  @param prefix: prefix for each data column
  '''
  cols = ', '.join(['%s%s double' %(prefix, k) for k in xrange(n)])
  return 'DROP TABLE IF EXISTS %s;CREATE TABLE %s (%s boolean, %s);' % (name, name, lbl, cols)

def generate_ex_queries(name, soln, m):
  ''' generates a set of queries for creating examples
  @param name: name of data table
  @param soln: solution vector
  @param m: the number of examples to make
  '''
  n = len(soln)
  query = 'INSERT INTO %s VALUES ' % (name)
  values = []
  for i in xrange(m):
    e = generate_example(n)
    lbl = predict(soln, e)
    cols = ', '.join(map(str, e))
    values.append('(%s, %s)' % (lbl, cols))
  query += ', '.join(values);
  query += ';'
  return [query]

def generate_all_queries(name, n, m, lbl='lbl', prefix='e'):
  ''' Generates a set of queries for creating an example table
  @param name: name of example table
  @param n: number of features (columns)
  @param m: number of examples (rows)
  @param lbl: label column name
  @param prefix: prefix for column names
  @return: soluton vector, query list
  '''
  qs = [create_examp_table_query(name, n, lbl=lbl, prefix=prefix)]
  sol = generate_soln(n)
  qs.extend(generate_ex_queries(name, sol, m))
  return sol, qs

def main():

  n = 3
  m = 50
  name = 'toy'

  sol, qrs = generate_all_queries(name, n, m)

  for q in qrs:
    print q

  iutil.impala_shell_exec(qrs, database='toysvm')


if __name__ == '__main__':
  main()
