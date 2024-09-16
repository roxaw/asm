make all

rm -r -f property_ori_experiment
rm -r -f klee-out*


time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog property-maxincrement.bc O 1 200 2 10
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog property-maxincrement.bc O 1 200 2 100
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog property-maxincrement.bc O 1 200 2 1000


mkdir property_ori_experiment
mv klee-out* property_ori_experiment
cp plan_ori_property.sh property_ori_experiment
