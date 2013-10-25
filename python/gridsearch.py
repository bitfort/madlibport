'''
A generic framework to grid search parameters.
'''

from multiprocessing import Pool


def parallel_run_trainer(params, trainer, concur=1):
  ''' Executes the trainer on the set of params in parallel
  @type params: list of any type `param'
  @param params: the parameters to try out
  @type trainer: function: param -> void
  @param trainer: a function which trains (blocking) until done
  '''
  print 'Starting process pool for training'
  p = Pool(concur)
  p.map(trainer, params)
  print 'Done training models'


def evaludate_tester(params, tester):
  ''' Executes testers in parallel 
  @type params: list of any type `param'
  @param params: the parameters to try out
  @type tester: function: param -> double
  @param tester: returns the quality of the solution -- lower is better
  @rtype: [ (double, param), ... ]
  @return unsorted list of quality + param pairs
  '''
  results = []
  for param in params:
    results.append( (param, tester(param)) )
  return results


def gridsearch(params, trainer, tester, concur=1):
  ''' Executes trainers in parallel and then evaluates the quality
  @type params: list of any type `param'
  @param params: the parameters to try out
  @type trainer: function: param -> void
  @param trainer: a function which trains (blocking) until done
  @type tester: function: param -> double
  @param tester: returns the quality of the solution -- lower is better
  @rtype: [ (double, param), ... ]
  @return sorted list of quality + param pairs
  '''
  parallel_run_trainer(params, trainer)
  results = evaludate_tester(params, tester)
  return sorted(results)
