make all

rm -r -f eq_experiment
rm -r -f klee-out*

time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog equivalence.bc 1 10 1 10
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog equivalence.bc 1 100 1 100
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog equivalence.bc 1 1000 1 1000
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog equivalence.bc 1 10000 1 10000

mkdir eq_experiment
mv klee-out* eq_experiment
cp plan_eq.sh eq_experiment
