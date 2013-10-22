import optparse
import impala_util as iutil


def main():
  parser = optparse.OptionParser('Impala logistic regression.')
  parser.add_option("-b", "--db", dest="database", default=None,
                        help="the database which holds data table", metavar="DB")
  parser.add_option("-t", "--table", dest="table", default=None,
                        help="data table to iterate over", metavar="TABLE")
  parser.add_option("-y", "--history", dest="history", default='history',
                        help="name of table to story iteratoin history (default history)", metavar="HIST")
  parser.add_option("-n", "--noact",
                        action="store_true", dest="noact", default=False,
                                          help="just print queries, don't execute over impala")

  parser.add_option("-s", "--step", dest="step", default=0.1, type="float",
                                          help="step size for SGD (default 0.1)")
  parser.add_option("-d", "--decay", dest="decay", default=0.95, type="float",
                                          help="step size decay (default 0.95)")
  parser.add_option("-u", "--mu", dest="mu", default=0, type="float",
                                          help="regularizer weight (defualt 0)")

  parser.add_option("-e", "--epochs", dest="epochs", default=1, type="int",
                                          help="number of epochs to run (default 1)")

  (options, args) = parser.parse_args()

  if options.database is None:
    print 'use --db to specify a database to use.'
    return
  if options.table is None:
    print 'use --table to specify the data table.'
    return

  qry = []

  mod_table = options.history
  dat_table = options.table
  step = options.step
  mu = options.mu
  label = args[0]
  arr = 'toarray(%s)' % (', '.join(map(lambda f: '%s.%s' % (dat_table, f), args[1:])))

  qry.append(iutil.make_model_table(mod_table))
  for i in xrange(1, options.epochs+1):
    qry.append(logr_epoch(mod_table, dat_table, label, arr, i, step=step, mu=mu))
    step = step * options.decay
  qry.append(logr_loss(mod_table, dat_table, label, arr, epoch=options.epochs))

  for q in qry:
    print q
  if not options.noact:
    iutil.impala_shell_exec(qry, database=options.database)

def logr_epoch(model_table, dat_table, label, arr, epoch, step=0.1, mu=0.1):
  return iutil.bismarck_epoch(model_table, dat_table, 'logr(__PREV_MODEL__, %(arr)s, '
      '%(label)s, %(step)s, %(mu)s)' % {'arr':arr, 'label':label, 'step':step,
        'mu':mu}, epoch, label)

def logr_loss(model_table, dat_table, label, arr, epoch):
  return iutil.bismarck_query('logrloss(__PREV_MODEL__, %(arr)s, %(label)s)' %
      {'arr':arr, 'label':label}, model_table, dat_table, epoch, label)

if __name__ == '__main__':
  main()

