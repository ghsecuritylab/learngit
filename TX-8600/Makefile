# The TARGET variable determines what target system the application is
# compiled for. It either refers to an XN file in the source directories
# or a valid argument for the --target option when compiling
TARGET = XL216-512-TQ128-C20

# ENABLE_STAGED_BUILD=0

# The APP_NAME variable determines the name of the final .xe file. It should
# not include the .xe postfix. If left blank the name will default to
# the project name
APP_NAME = TX-8600

# The USED_MODULES variable lists other module used by the application.
USED_MODULES = lib_ethernet lib_gpio(>=1.1.0)  lib_locks lib_logging lib_otpinfo lib_xtcp module_fatfs lib_uart lib_sdram lib_sdcard

# The flags passed to xcc when building the application
# You can also set the following to override flags for a particular language:
# XCC_XC_FLAGS, XCC_C_FLAGS, XCC_ASM_FLAGS, XCC_CPP_FLAGS
# If the variable XCC_MAP_FLAGS is set it overrides the flags passed to
# xcc for the final link (mapping) stage.
XCC_FLAGS = -O2  -fxscope -lquadflash  -DUIP_CONF_IPV4=1 -DDEBUG_PRINT_ENABLE=1 -report -DETHERNET_SUPPORT_HP_QUEUES=1 -Wno-reinterpret-alignment -g
XCC_FLAGS += -DUIP_CONF_UDP_CONNS=180  -DUIP_CONF_ARPTAB_SIZE=180

# XCC_FLAGS = $(XCC_FLAGS_Release)

# The VERBOSE variable, if set to 1, enables verbose output from the make system.
VERBOSE = 0


XMOS_MAKE_PATH ?= ../..
-include $(XMOS_MAKE_PATH)/xcommon/module_xcommon/build/Makefile.common
