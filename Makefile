#在x86平台下, 直接make, 不要加-j8等多线程编译选项
#工作路径
WORK_DIR = $(shell pwd)
CROSS = 
STL_LIBS = 
PLAT = x86
ifneq ($(platform), )
ifeq ($(platform), x86)
PLAT = x86
else
ifeq ($(platform), arm)
PLAT = arm
CROSS = arm-linux-
STL_LIBS = Lib/arm/STLport/libstlport.a \
		Lib/arm/STLport/libstlportg.a \
		Lib/arm/STLport/libstlportstlg.a
endif
endif
endif

#LIB目录
DEPEND_LIB_DIR = Lib/$(PLAT)

ifneq ($(test), )
ifeq ($(test), DEVICE)
TEST = Device
SRC_DIR += Src/Common \
		  Src/Device \
		  Src/Device/RegProxy \
		  Src/Device/DevController \
		  Src/Device/RPCServer \
		  Src/Device/RPCClient \
		  Src/Device/Component
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

#头文件路径
INC_DIR = -IInclude/
ifeq ($(PLAT), arm)

INC_DIR += -IInclude/stlport/
endif

#依赖库文件
DEPEND_LIBS =	$(DEPEND_LIB_DIR)/Poco/libPocoDataSQLite.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoDataMySQL.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoMongoDB.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoData.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoNetSSL.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoNet.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoCrypto.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoUtil.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoXML.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoZip.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoJSON.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoFoundation.a \
				$(DEPEND_LIB_DIR)/openssl/libssl.a \
				$(DEPEND_LIB_DIR)/openssl/libcrypto.a \
				$(STL_LIBS)

DEPEND_LIBS_DEBUG = $(DEPEND_LIB_DIR)/Poco/libPocoDataSQLited.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoDataMySQLd.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoMongoDBd.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoData.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoNetSSLd.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoNetd.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoCryptod.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoUtild.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoXMLd.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoZipd.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoJSONd.a \
				$(DEPEND_LIB_DIR)/Poco/libPocoFoundationd.a \
				$(DEPEND_LIB_DIR)/openssl/libssl.a \
				$(DEPEND_LIB_DIR)/openssl/libcrypto.a \
				$(STL_LIBS)

		
CFLAGS += $(INC_DIR)
CFLAGS += -Wall -O2 -g -static
ifeq ( $(PLAT), arm)
CFLAGS += -D__SC_ARM__
endif

#依赖库链接路径
LDFLAGS += -lmysqlclient -lrt -ldl -lpthread

#临时文件
COMPILE_DIR = Compile/$(PLAT)

#目标文件
BIN_DIR = Bin/$(PLAT)

TARGET = $(BIN_DIR)/$(TEST)Test
TARGET_DEBUG = $(BIN_DIR)/$(TEST)Test_debug

#目标


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
