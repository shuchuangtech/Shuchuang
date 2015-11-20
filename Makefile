#在x86平台下, 直接make, 不要加-j8等多线程编译选项
#工作路径
WORK_DIR = $(shell pwd)
PLAT = x86
ifneq ($(platform), )
PLAT = $(platform)
include Build/config/$(PLAT).conf
endif

ifneq ($(test), )
ifeq ($(test), DEVICE)
TEST = Device
SRC_DIR += Src/Common \
		  Src/Device \
		  Src/Device/RegProxy \
		  Src/Device/RPCServer \
		  Src/Device/RPCClient \
		  Src/Device/Network \
		  Src/Device/Component \
		  Src/Device/Component/User \
		  Src/Device/Component/Task	\
		  Src/Device/Util
else
ifeq ($(test), SERVER)
TEST = TransmitServer
SRC_DIR += Src/Common \
		   Src/TransmitServer \
		   Src/TransmitServer/RegServer \
		   Src/TransmitServer/HTTPServer \
		   Src/TransmitServer/HTTPSAcceptor
else
TEST = $(test)
SRC_DIR +=Src/Test/$(test) \
		  Src/Common
endif
endif
else
TEST = Function
SRC_DIR += Src/Test
endif

		

CC = @echo " g++ $@"; $(CROSS)gcc
#CPP = @echo " g++ $@"; $(CROSS)g++
CPP = $(CROSS)g++
LD = @echo " g++ $@"; $(CROSS)ld
AR = @echo " g++ $@"; $(CROSS)ar
#STRIP = @echo " g++ $@"; $(CROSS)strip
STRIP = $(CROSS)strip

SRC_FILE = $(foreach dir, ${SRC_DIR}, $(wildcard $(dir)/*.cpp))
OBJ_FILE = $(SRC_FILE:%.cpp=$(COMPILE_DIR)/%.o)
#OBJ_FILE = $(subst Src,Compile, $(patsubst %.cpp, %.o, ${SRC_FILE}))


$(foreach dir,$(SRC_DIR),$(shell mkdir -p $(COMPILE_DIR)/$(dir)))
$(foreach dir, $(BIN_DIR), $(shell mkdir -p $(dir)))
all :$(TARGET)

$(TARGET) : $(OBJ_FILE) $(DEPEND_LIBS)
	$(CPP) $^ $(LDFLAGS) -o $@
	$(STRIP) $@

$(COMPILE_DIR)/%.o : %.cpp
	$(CPP) -c $(CFLAGS) $< -o $@

debug:$(TARGET_DEBUG)

$(TARGET_DEBUG) : $(OBJ_FILE) $(DEPEND_LIBS_DEBUG)
	$(CPP) $^ $(LDFLAGS) -o $@
	
$(COMPILE_DIR)/%.o : %.cpp
	$(CPP) -c $(CFLAGS) $< -o $@

print:
	@echo "INC_DIR:"$(INC_DIR)
	@echo "SRC_DIR:"$(SRC_DIR)
	@echo "COMPILE_DIR:"$(COMPILE_DIR)
	@echo "TARGET:"$(TARGET)
	@echo "SRC_FILE:"$(SRC_FILE)
	@echo "OBJ_FILE:"$(OBJ_FILE)
	
clean:
	rm -rf $(OBJ_FILE)
	rm -rf $(TARGET)
	rm -rf $(TARGET_DEBUG)
