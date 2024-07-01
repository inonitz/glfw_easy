# WORKING_DIR_ABS_PATH=/cygdrive/c/"Program Files/Programming Utillities"/Cygwin/home/themc/mglw
WORKING_DIR_ABS_PATH="$(shell pwd)"
SCRIPT_PATH=.vscode/clangd_cc
SCRIPT_NAME=gen_commands.sh
SCRIPT_FULL_ADDRESS=$(WORKING_DIR_ABS_PATH)/$(SCRIPT_PATH)/$(SCRIPT_NAME)




debug:
	@ echo -n Compiling In Debugging Mode ...
	-@ $(SCRIPT_FULL_ADDRESS) compile debug
	@ echo " Done! "

rel:
	@ echo -n Compiling In Release Mode ...
	-@ $(SCRIPT_FULL_ADDRESS) compile release
	@ echo " Done! " 


	


recdbg:
	@ echo -n Recording Compiler Output for clangd-compile_commands_debug.json generation...
	-@ $(SCRIPT_FULL_ADDRESS) record debug
	@ echo " Done! "

recrel:
	@ echo -n Recording Compiler Output for clangd-compile_commands_release.json generation...
	-@ $(SCRIPT_FULL_ADDRESS) record release
	@ echo " Done! "


cleandbg:
# echo $(SCRIPT_FULL_ADDRESS)
	@ echo -n "Cleaning Compiled Debug Files ... "
	-@ $(SCRIPT_FULL_ADDRESS) clean debug
	@ echo " Done! "

cleanrel:
	@ echo -n "Cleaning Compiled Release Files ... "
	-@ $(SCRIPT_FULL_ADDRESS) clean release
	@ echo " Done! "


cleanbindbg:
	@ echo -n "Cleaning Compiled Debug Executable ... "
	-@ $(SCRIPT_FULL_ADDRESS) clean_binary debug
	@ echo " Done! "


cleanbinrel:
	@ echo -n "Cleaning Compiled Release Executable ... "
	-@ $(SCRIPT_FULL_ADDRESS) clean_binary release
	@ echo " Done! "



cleanall: cleandbg
cleanall: cleanbindbg
cleanall: cleanrel
cleanall: cleanbinrel




rundbg:
	@ echo -n "Running Debug Executable ... "
	-@ $(SCRIPT_FULL_ADDRESS) run debug
	@ echo " Done! "

runrel:
	@ echo -n "Running Release Executable ... "
	-@ $(SCRIPT_FULL_ADDRESS) run release
	@ echo " Done! "




cleanclangd:
	@ echo -n "Clearing clangd Cache ... "
	-@ rm $(WORKING_DIR_ABS_PATH)/.vscode/.cache/clangd/index/*
	@ echo " Done! "


setup:
	mkdir -p ext
	mkdir -p misc
	mkdir -p src
	mkdir -p build
	mkdir -p build/debug
	mkdir -p build/debug/bin
	mkdir -p build/debug/obj
	mkdir -p build/release
	mkdir -p build/release/bin
	mkdir -p build/release/obj