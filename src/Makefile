APP_EXE_PATH = /usr/lib/ecn
DAT_FILE_PATH = /usr/share/ecn
MOD_CONF_PATH = /etc/modules-load.d
MOD_OBJ_PATH = /lib/modules/$(shell uname -r)/kernel/ecn

install: lpc.c
	@if [ ! -d "$(APP_EXE_PATH)" ]; then mkdir $(APP_EXE_PATH); fi
	@if [ ! -d "$(DAT_FILE_PATH)" ]; then mkdir $(DAT_FILE_PATH); fi
	@if [ ! -d "$(DAT_FILE_PATH)/lpc" ]; then mkdir $(DAT_FILE_PATH)/lpc; fi
	@cd $(DAT_FILE_PATH)/lpc && touch lpc.dat
	@gcc -o $(APP_EXE_PATH)/lpc lpc.c
	@cp -rf mod build
	@cd build && make

	@# @cp mod-build/lpc-mod.conf $(MOD_CONF_PATH)
	@# @if [ ! -d "$(MOD_OBJ_PATH)" ]; then mkdir $(MOD_OBJ_PATH); fi
	@# @cp mod-build/lpc-mod.ko $(MOD_OBJ_PATH)
	@# @depmod
	@# @rm -rf build

clean:
	@rm -f $(APP_EXE_PATH)/lpc
	@if [ ! "$(ls -A $(APP_EXE_PATH))" ]; then rm -rf $(APP_EXE_PATH); fi
	@rm -rf $(DAT_FILE_PATH)/lpc
	@if [ ! "$(ls -A $(DAT_FILE_PATH))" ]; then rm -rf $(DAT_FILE_PATH); fi
	@rm -rf build

	@# rm -f $(MOD_CONF_PATH)/lpc-mod.conf
	@# rm -f $(MOD_OBJ_PATH)/lpc-mod.ko
	@# depmod
	@# if [ ! "$(ls -A $(MOD_OBJ_PATH))" ]; then rm -rf $(MOD_OBJ_PATH); fi
