## LLVM for VideoCore4
This is mostly based on the work done by David Given (cowlark). It works, and can compile and produce reasonable code. I did this so I could have a decent compiler to develop my Raspberry Pi firmware on.

### What works
* Pretty much everything that isn't listed under the 'Issues' section. Some corner cases may still lead to LLC errors such as using certain builtins.

### Issues
* Variable length arrays do not work (because of FP not being implemented)
* MC code emisssion is not supported
* Use any floating point arithmetic at your own risk, I have not tested it
* BB reordering is not implemented so you'll have to live with redundant branches being generated
* TableGen code is not ideal, I tried to improve it and get rid of some of the hacks used in the original one but there's still a long way to go.
* Some directives emmited by `LLC` confuse `VASM` so at the moment I just `sed` them out in the Makefile (I've included an example).

### Using it
For now, you should just build `LLC` and use a stock Clang that can target VideoCore. As the VideoCore4 target does not currently support MC code emission, you will have to rely on an external assembler/linker, namely `VASM` and `VLINK`. 

`VASM` requires a small patch in order to work but since the license does not allow redistribution of modifications, I cannot include it, but to summarise in the VideoCore `cpu.c` file, in `translate` the handler for for 48 bit arithmetic instructions needs to have the first conditional commented out (the one that checks if the instruction number is less than 32).

Once you build `VASM` and `VLINK` and applied this patch, you can use it as shown in the Makefile excerpt here:
```
CC = clang
CXX = clang++
AS = ../vasm/vasmvidcore_std
LINK = ../vlink/vlink
CFLAGS = -S -O3 -D__VIDEOCORE4__ -no-integrated-as -fno-vectorize -fno-slp-vectorize -emit-llvm -target xcore-none-none-none -nostdlib
ASFLAGS = -Fvobj -quiet
LLC =  ../llvm-vc4/build/bin/llc -O3 --mtriple vc4

$(TARGET_BUILD_DIR)/%.o: %.c $(HEADERS)
	$(CREATE_SUBDIR)
	@echo $(WARN_COLOR)CC $(NO_COLOR)  $@
	@$(CC) $(CFLAGS) $< -o $@.ll

	@echo $(WARN_COLOR)LLC$(NO_COLOR)  $@.ll
	@$(LLC) $@.ll -o $@.s

	@sed -i '/^\t.section/d' $@.s
	@sed -i '/^\t.bss/d' $@.s
	@sed -i '/^\t.align	16/c\\t.align 2' $@.s
	@echo $(WARN_COLOR)AS $(NO_COLOR)  $@
	@$(AS) $(ASFLAGS) $@.s -o $@

$(TARGET_BOOTCODE): create_build_directory $(OBJ)
	@echo $(WARN_COLOR)LD $(NO_COLOR)  $@
	$(SET_BINUTILS_ENV)
	@$(LINK) $(OBJ) -b rawbin1 -o $(PRODUCT_DIRECTORY)/$@
```
