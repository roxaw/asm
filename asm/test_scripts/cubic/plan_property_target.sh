make all

rm -r -f property_experiment_target
rm -r -f klee-out*

time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog equivalence.bc 1 2000 2 100

time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog property-maxincrement.bc A 1 2000 2 100

time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog property-maxincrement.bc O 1 2000 2 100


mkdir property_experiment_target
mv klee-out* property_experiment_target
cp plan_property_target.sh property_experiment_target
