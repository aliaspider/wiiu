## --------------------------------------------------------- ##

TARGET   := rpltool
BUILD_DIR = objs
DEBUG    ?= 0
V        ?= 0

OBJ :=
OBJ += main.o
OBJ += options.o
OBJ += table.o
OBJ += rpl.o
OBJ += rpl_info.o

DEFINES :=
INCDIRS := -I.
FLAGS   := -ffunction-sections -fdata-sections
#FLAGS   += -Wall 
FLAGS   += -Werror

CFLAGS   := 
ASFLAGS  := -mregnames -Wa,--sectname-subst
CXXFLAGS := -std=c++17

LDFLAGS  := -Wl,--gc-sections
LIBS     := -lz

## --------------------------------------------------------- ##

PREFIX := 

CC      := ccache $(PREFIX)gcc
CXX     := ccache $(PREFIX)g++
AS      := ccache $(PREFIX)as
AR      := $(PREFIX)ar
OBJCOPY := $(PREFIX)objcopy
STRIP   := $(PREFIX)strip
NM      := $(PREFIX)nm
LD      := $(CXX)

ifeq ($(DEBUG), 1)
   BUILD_DIR := $(BUILD_DIR)-debug
   FLAGS  += -O0 -g
   DEFINES += -D_DEBUG
else
   FLAGS  += -O3
endif

ifneq ($(V), 1)
   Q := @
endif

OBJ := $(OBJ:.c=.o)
OBJ := $(OBJ:.cpp=.o)
OBJ := $(OBJ:.cc=.o)
OBJ := $(OBJ:.s=.o)
OBJ := $(OBJ:.S=.o)
OBJ := $(addprefix $(BUILD_DIR)/,$(OBJ))

DEPFLAGS = -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.depend

all: $(TARGET)

%: $(BUILD_DIR)/%
	cp $< $@

$(BUILD_DIR)/%.o: %.cpp %.depend
	@$(if $(Q), echo CXX $<,)
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -c -o $@ $< $(FLAGS) $(CXXFLAGS) $(DEFINES) $(INCDIRS) $(DEPFLAGS)

$(BUILD_DIR)/%.o: %.cc %.depend
	@$(if $(Q), echo CXX $<,)
	@mkdir -p $(dir $@)
	$(Q)$(CXX) -c -o $@ $< $(FLAGS) $(CXXFLAGS) $(DEFINES) $(INCDIRS) $(DEPFLAGS)

$(BUILD_DIR)/%.o: %.c %.depend
	@$(if $(Q), echo CC $<,)
	@mkdir -p $(dir $@)
	$(Q)$(CC) -c -o $@ $< $(FLAGS) $(CFLAGS) $(DEFINES) $(INCDIRS) $(DEPFLAGS)

$(BUILD_DIR)/%.o: %.s %.depend
	@$(if $(Q), echo AS $<,)
	@mkdir -p $(dir $@)
	$(Q)$(CC) -c -o $@ $< $(FLAGS) $(ASFLAGS) $(INCDIRS) $(DEPFLAGS)

$(BUILD_DIR)/%.o: %.S %.depend

	@$(if $(Q), echo AS $<,)
	@mkdir -p $(dir $@)
	$(Q)$(CC) -c -o $@ $< $(FLAGS) $(ASFLAGS) $(DEFINES) $(INCDIRS) $(DEPFLAGS)

$(BUILD_DIR)/$(TARGET): $(OBJ) .$(TARGET).last
	@$(if $(Q), echo LD $@,)
	@mkdir -p $(dir $@)
	@touch .$(TARGET).last
	$(Q)$(LD) $(OBJ) $(FLAGS) $(LDFLAGS) $(LIBS) -o $@

%.depend: ;
%.last: ;

clean:
	@$(if $(Q), echo $@,)
	$(Q)rm -f $(OBJ) $(TARGET) .$(TARGET).last $(BUILD_DIR)/$(TARGET) 
	$(Q)rm -f $(OBJ:.o=.depend)

.PHONY: clean all
.PRECIOUS: %.depend %.last

-include $(OBJ:.o=.depend)
