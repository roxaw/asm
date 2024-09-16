make all

rm -r -f property_agg_experiment
rm -r -f klee-out*

time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog property-maxincrement.bc A 1 10 1 10
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog property-maxincrement.bc A 1 100 1 100
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog property-maxincrement.bc A 1 1000 1 1000
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog property-maxincrement.bc A 1 10000 1 10000

mkdir property_agg_experiment
mv klee-out* property_agg_experiment
cp plan_agg_property.sh property_agg_experiment
