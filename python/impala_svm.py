import optparse
import impala_util as iutil


def main():


  qry = []

  qry.append(iutil.make_model_table('history'))

  mod_table = 'history'
  dat_table = 'toy'
  step = 0.1
  mu = 0.1
  label = 'toy.lbl'
  arr = 'toarray(toy.x, toy.y, toy.z)'

  n = 5
  for i in xrange(1, n):
    qry.append(svm_epoch(mod_table, dat_table, label, arr, i, step=step, mu=mu))
    qry.append(svm_loss(mod_table, dat_table, label, arr, epoch=i))

  for q in qry:
    print q

  #iutil.impala_shell_exec(qry, database='toysvm')




def svm_epoch(model_table, dat_table, label, arr, epoch, step=0.1, mu=0.1):
  return iutil.bismarck_epoch(model_table, dat_table, 'svm(__PREV_MODEL__, %(arr)s, '
      '%(label)s, %(step)s, %(mu)s)' % {'arr':arr, 'label':label, 'step':step,
        'mu':mu}, epoch)


def svm_loss(model_table, dat_table, label, arr, epoch):
  return iutil.bismarck_query('svmloss(__PREV_MODEL__, %(arr)s, %(label)s)' %
      {'arr':arr, 'label':label}, model_table, dat_table, epoch)
   


if __name__ == '__main__':
  main()
