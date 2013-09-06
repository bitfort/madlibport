import optparse


def bismarck_sql_epoch(outtbl, intbl, labelcol, udf, cols, epoch, step=0.1):
  prev_state = 'SELECT model FROM %s WHERE epoch=%s' % (outtbl, epoch-1)
  uda = '%s( (%s), %s )' % (udf, prev_state, ', '.join(map(str, cols)))
  return 'INSERT INTO %s SELECT %s, %s FROM %s' % (outtbl, epoch, uda, intbl)

def logistic_train(outtbl, intbl, labelcol, cols, verbose=False, epochs=1, 
    step=0.1, decay=0.95):

  print 'CREATE TABLE %s (model StringVal, epoch int);' % outtbl
  args = cols[:]
  args.append(step)
  print bismarck_sql_epoch(outtbl, intbl, labelcol, 'logr', args, 1)


def main():
  usage = "usage: %prog [options] outtable datatable labelcol excols ..."
  parser = optparse.OptionParser(usage=usage)
  parser.add_option("-e", "--epochs", dest="epochs", default=0.1, type="int",
                        help="number of passes over the data table", metavar="EPOCHS")
  parser.add_option("-v", "--verbose",
                        action="store_true", dest="verbose", default=False,
                                          help="log actions taken [default=false]")
  parser.add_option("-s", "--step", dest="stepsize", default=0.1, type="float",
                        help="IGD stepsize [default=0.1]", metavar="STEPSIZE")
  parser.add_option("-d", "--decay", dest="stepdecay", default=0.95, type="float",
                        help="decay rate of the stepsize [default=0.95]", metavar="STEPDECAY")
  (options, args) = parser.parse_args()

  if len(args) < 4:
    parser.error("incorrect number of arguments")
    return

  logistic_train(args[0], args[1], args[2], args[3:],
      verbose=options.verbose, step=options.stepsize, decay=options.stepdecay, 
      epochs=options.epochs)

if __name__ == '__main__':
  main()
