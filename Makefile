# Condenstation Makefile
#   modified from RB3Enhanced PS3 https://github.com/InvoxiPlayGames/RB3Enhanced

# the source code files to compile from
SRC_DIR := source
SOURCES := $(wildcard $(SRC_DIR)/*.c)
SOURCES_CPP := $(wildcard $(SRC_DIR)/*.cpp)
INC_DIR := include

# output directory for final SPRX file
OUTPUT := out
# output filename for final SPRX file
OUTNAME := condenstation_ps3

# definitions for compilation
DEFINES := NDEBUG

# PS3 variables
# =================
# build directory for PS3 compilation
BUILD_P := build_ps3
# executable tool path
TOOLPATH_P := $(PS3SDKHOST)/bin
PPUHOSTPATH_P := $(PS3SDKHOST)/ppu
PPUTARGETPATH_P := $(PS3SDKTARGET)/ppu
COMPILER_P := $(PPUHOSTPATH_P)/bin/ppu-lv2-gcc
MAKESELF_P := $(TOOLPATH_P)/make_fself
MAKESELFNPDRM_P := $(TOOLPATH_P)/make_fself_npdrm
SCETOOL_P := $(SCETOOLPATH)/scetool
# .o object files for PS3 compilation
OBJECTS_P := $(subst $(SRC_DIR),$(BUILD_P),$(patsubst %.c,%.c.o,$(SOURCES))) $(subst $(SRC_DIR),$(BUILD_P),$(patsubst %.cpp,%.cpp.o,$(SOURCES_CPP)))
# include directories
GCC_VER_P := 4.1.1
ifneq ($(strip $(PS3SDKHOST)),) 
    GCC_VER_P := $(shell $(COMPILER_P) -dumpversion)
endif
INCLUDES_P := $(PPUHOSTPATH_P)/lib/gcc/ppu-lv2/$(GCC_VER_P)/include \
				$(PPUTARGETPATH_P)/include $(INC_DIR)
# library directories
LIBDIR_P := $(PPUTARGETPATH_P)/lib 
# library includes
LIBS_P := -lstdc++ -lsupc++ -lfs_stub -lsysmodule_stub -lhttp_stub -lhttp_util_stub -lssl_stub -lnet_stub -lsysutil_stub -lsysutil_userinfo_stub -lc_stub -lnetctl_stub -lsha1
# compiler flags
CFLAGS_P := -O2 -Wall -Wno-format-extra-args -x c -std=gnu99 \
			$(patsubst %,-D%,$(DEFINES)) \
			-mcpu=cell -fPIC \
			$(patsubst %,-I %,$(INCLUDES_P)) -iquote src
# compiler flags (C++)
CFLAGS_CPP_P := -O2 -Wall -Wno-format-extra-args -x c++ \
			$(patsubst %,-D%,$(DEFINES)) \
			-mcpu=cell -fPIC \
			$(patsubst %,-I %,$(INCLUDES_P)) -iquote src
# linker flags for final compile
LFLAGS_P := -mprx -zgenprx -L $(PPUTARGETPATH_P)/lib/hash $(LIBS_P)
# scetool flags for signed output
SCETOOLFLAGS_P := --sce-type=SELF --compress-data=TRUE --skip-sections=FALSE \
				--key-revision=04 \
				--self-ctrl-flags=4000000000000000000000000000000000000000000000000000000000000002 \
				--self-auth-id=1010000001000003 --self-add-shdrs=TRUE --self-vendor-id=01000002 \
				--self-app-version=0000000000000000 --self-type=APP --self-fw-version=0003004000000000
# =================

.PHONY: all
all: ps3

.PHONY: scripts
scripts:
	@bash scripts/version.sh

.PHONY: clean
clean:
	@rm -rf $(wildcard $(BUILD_W) $(BUILD_X) $(BUILD_P) $(OUTPUT))

# PS3 compilation, creates .SPRX file

.PHONY: ps3 ps3_f ps3_fnp rpcs3
ps3_f: $(OUTPUT)/f_$(OUTNAME).sprx
ps3: $(OUTPUT)/$(OUTNAME).sprx
ps3_fnp: $(OUTPUT)/fnp_$(OUTNAME).sprx

rpcs3: ps3_f
	@cp $(OUTPUT)/f_$(OUTNAME).sprx $(RPCS3_DIR)/dev_hdd0/game/BLES01222/USRDIR/portal2_dlc3/addons/condenstation/condenstation_ps3.sprx
	@cp $(OUTPUT)/f_$(OUTNAME).sprx $(RPCS3_DIR)/dev_hdd0/game/NPUB30589/USRDIR/csgo/addons/condenstation/condenstation_ps3.sprx

$(OUTPUT)/f_$(OUTNAME).sprx: $(BUILD_P)/output.prx
	@echo "Creating FSPRX..."
	@mkdir -p $(@D)
	@$(MAKESELF_P) $^ $@

$(OUTPUT)/fnp_$(OUTNAME).sprx: $(BUILD_P)/output.prx
	@echo "Creating NP FSPRX..."
	@mkdir -p $(@D)
	@$(MAKESELFNPDRM_P) $^ $@

$(OUTPUT)/$(OUTNAME).sprx: $(BUILD_P)/output.prx
	@echo "Creating signed SPRX..."
	@mkdir -p $(@D)
	@$(SCETOOL_P) $(SCETOOLFLAGS_P) --encrypt $^ $@

$(BUILD_P)/output.prx: $(OBJECTS_P)
	@echo "Linking PRX..."
	@mkdir -p $(@D)
	@$(COMPILER_P) $^ $(LFLAGS_P) -o $@

$(BUILD_P)/%.c.o: $(SRC_DIR)/%.c | scripts
	@echo $<
	@mkdir -p $(@D)
	@$(COMPILER_P) -c $(CFLAGS_P) $< -o $@

$(BUILD_P)/%.cpp.o: $(SRC_DIR)/%.cpp | scripts
	@echo $<
	@mkdir -p $(@D)
	@$(COMPILER_P) -c $(CFLAGS_CPP_P) $< -o $@
