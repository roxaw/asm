make all

rm -r -f  property_experiment

rm -r -f klee-out*

time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc A 1 100 2 38 2
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc A 1 100 2 38 3
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc A 1 100 2 38 4
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc A 1 100 2 38 5
#time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc A 1 100 2 38 6
#time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc A 1 100 2 38 7
#time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc A 1 100 2 38 8
date
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc O 1 100 2 38 2
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc O 1 100 2 38 3
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc O 1 100 2 38 4
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc O 1 100 2 38 5
#time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc O 1 100 2 38 6
#time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc O 1 100 2 38 7
#time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog  property-friendliness.bc O 1 100 2 38 8

mkdir property_experiment
mv klee-out* property_experiment
cp plan.sh property_experiment


