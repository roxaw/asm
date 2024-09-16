make all

rm -r -f property_ori_experiment
rm -r -f klee-out*

time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog property-maxincrement.bc O 1 10 1 10
time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog property-maxincrement.bc O 1 100 1 100
#time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog property-maxincrement.bc O 1 1000 1 1000
#time ../../../klee_build/bin/klee --max-memory=0 --max-time=60000s --watchdog property-maxincrement.bc O 1 10000 1 10000


mkdir property_ori_experiment
mv klee-out* property_ori_experiment
cp plan_ori_property.sh property_ori_experiment
