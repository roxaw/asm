CC=gcc -g #-Wall 
Clang=clang-6.0 -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone 
PATH1=../../stub_model/source-new
PATH2=../../stub_model/header-new
PATH3=../../CA_variants

INC=-I $(PATH1) -I $(PATH2) -I $(PATH3)

DOUBLE=property-maxincrement
$(DOUBLE).bc: $(DOUBLE).c $(wildcard $(PATH1)/*.c) $(wildcard $(PATH3)/*.c) $(wildcard $(PATH2)/*.h)
	$(Clang) $(DOUBLE).c $(INC)

EQ=equivalence
$(EQ).bc: $(EQ).c $(wildcard $(PATH1)/*.c) $(wildcard $(PATH3)/*.c) $(wildcard $(PATH2)/*.h)
	$(Clang) $(EQ).c $(INC)


all: $(DOUBLE).bc $(EQ).bc 


