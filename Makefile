TARGET   := test
DEBUG     = 1
BUILD_DIR = objs
WIIU_DIR = .

OBJ := 
OBJ += test.o

ifeq ($(DEBUG),1)
   BUILD_DIR := $(BUILD_DIR)-debug
endif

ifneq ($(V), 1)
   Q := @
endif

DEFINES :=
INCDIRS :=
OBJ += missing_libc_functions.o
OBJ += exception_handler.o
OBJ += entry.o
OBJ += logger.o
OBJ += memory.o
OBJ += cxx_utils.o
OBJ += fs_utils.o
OBJ += sd_fat_devoptab.o
OBJ += gx2_validation.o
OBJ += shader_info.o
OBJ += stubs.o
OBJ += gthr-wup.o
OBJ += shader_disasm.o
OBJ += libfat/cache.o
OBJ += libfat/directory.o
OBJ += libfat/disc.o
OBJ += libfat/fatdir.o
OBJ += libfat/fatfile.o
OBJ += libfat/file_allocation_table.o
OBJ += libfat/filetime.o
OBJ += libfat/libfat.o
OBJ += libfat/lock.o
OBJ += libfat/partition.o
OBJ += libiosuhax/iosuhax.o
OBJ += libiosuhax/iosuhax_devoptab.o
OBJ += libiosuhax/iosuhax_disc_interface.o

OBJ := $(OBJ:.c=.o)
OBJ := $(OBJ:.cpp=.o)
OBJ := $(OBJ:.cc=.o)
OBJ := $(addprefix $(BUILD_DIR)/,$(OBJ))


DEFINES += -D__wiiu__ -D__powerpc__ -DFD_SETSIZE=32 -D_POSIX_THREADS

INCDIRS += -isystem$(WIIU_DIR) -Iinclude

CFLAGS  := -mcpu=750 -meabi -msdata=eabi -mhard-float
CFLAGS  += -ffunction-sections -fdata-sections
CFLAGS  += -ftls-model=global-dynamic
#CFLAGS  += -Werror
#CFLAGS  += -Wall

ifeq ($(DEBUG), 1)
   CFLAGS  += -O0 -g
   DEFINES += -D_DEBUG
else
   CFLAGS  += -O3
endif

ASFLAGS := $(CFLAGS) -mregnames -Wa,--sectname-subst
CXXFLAGS := $(CFLAGS) -std=c++17


LIBDIRS := -L$(BUILD_DIR)
LIBS := 
LDFLAGS := $(CFLAGS) 
LDFLAGS += -z common-page-size=64 -z max-page-size=64 -z nocopyreloc -nostartfiles
LDFLAGS += -Wl,--emit-relocs,--no-tls-optimize,--gc-sections
LDFLAGS += -T link.ld

PREFIX := $(DEVKITPPC)/bin/powerpc-eabi-

CC      := ccache $(PREFIX)gcc
CXX     := ccache $(PREFIX)g++
AS      := ccache $(PREFIX)as
CPP     := $(PREFIX)cpp
AR      := $(PREFIX)ar
OBJCOPY := $(PREFIX)objcopy
STRIP   := $(PREFIX)strip
NM      := $(PREFIX)nm
LD      := $(CXX)
RPLTOOL := $(WIIU_DIR)/rpltool/rpltool

DEPFLAGS    = -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.depend

all: rpltool $(TARGET).rpx

rpltool:
	$(MAKE) -C $(dir $(RPLTOOL))

%: $(BUILD_DIR)/%
	cp $< $@

$(BUILD_DIR)/%.o: %.cpp %.depend Makefile 
	@$(if $(Q), echo CXX $<,)
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -c -o $@ $< $(CXXFLAGS) $(DEFINES) $(INCDIRS) $(DEPFLAGS)

$(BUILD_DIR)/%.o: %.cc %.depend Makefile 
	@$(if $(Q), echo CXX $<,)
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -c -o $@ $< $(CXXFLAGS) $(DEFINES) $(INCDIRS) $(DEPFLAGS)

$(BUILD_DIR)/%.o: %.c %.depend Makefile 
	@$(if $(Q), echo CC $<,)
	@mkdir -p $(dir $@)
	$(Q)$(CC) -c -o $@ $< $(CFLAGS) $(DEFINES) $(INCDIRS) $(DEPFLAGS)

$(BUILD_DIR)/%.o: %.S %.depend Makefile 
	@$(if $(Q), echo AS $<,)
	@mkdir -p $(dir $@)
	$(Q)$(CC) -c -o $@ $< $(ASFLAGS) $(DEFINES) $(INCDIRS) $(DEPFLAGS)

$(BUILD_DIR)/%.o: %.s %.depend Makefile 
	@$(if $(Q), echo AS $<,)
	@mkdir -p $(dir $@)
	$(Q)$(CC) -c -o $@ $< $(ASFLAGS) $(INCDIRS) $(DEPFLAGS)

$(BUILD_DIR)/%.ld: $(WIIU_DIR)/%.ldi %.depend Makefile 
	$(CPP) -P $(INCDIRS) $(DEPFLAGS) $< | grep . > $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJ) $(BUILD_DIR)/imports.ld .$(TARGET).last
	@$(if $(Q), echo LD $@,)
	@touch .$(TARGET).last
	$(Q)$(LD) $(OBJ) $(LDFLAGS) $(LIBDIRS) $(LIBS) -o $@

$(TARGET).rpx: $(TARGET).elf $(RPLTOOL)
	@$(if $(Q), echo rpltool $@,)
	$(RPLTOOL) $(TARGET).elf -S -p -o $@

clean:
	@$(if $(Q), echo $@,)
	$(Q)rm -f $(OBJ) $(TARGET).rpx $(TARGET).elf 
	$(Q)rm -f $(BUILD_DIR)/$(TARGET).elf .$(TARGET).last
	$(Q)rm -f $(OBJ:.o=.depend) $(BUILD_DIR)/imports.depend

%.depend: ;
%.last: ;

.PHONY: clean all rpltool
.PRECIOUS: %.depend %.last

-include $(OBJ:.o=.depend) $(BUILD_DIR)/imports.depend
