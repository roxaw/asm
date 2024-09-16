CC=gcc -g #-Wall 
Clang=clang-6.0 -emit-llvm -c -g -O0 -Xclang -disable-O0-optnone 
PATH1=../../stub_model/source-new
PATH2=../../stub_model/header-new
PATH3=../../CA_variants

INC=-I $(PATH1) -I $(PATH2) -I $(PATH3)

EQCA=equivalence
$(EQCA).bc: $(EQCA).c $(wildcard $(PATH1)/*.c) $(wildcard $(PATH3)/*.c) $(wildcard $(PATH2)/*.h)
	$(Clang) $(EQCA).c $(INC)

DOUBLE=property-maxincrement
$(DOUBLE).bc: $(DOUBLE).c $(wildcard $(PATH1)/*.c) $(wildcard $(PATH3)/*.c) $(wildcard $(PATH2)/*.h)
	$(Clang) $(DOUBLE).c $(INC)

all: $(DOUBLE).bc  $(EQCA).bc 


