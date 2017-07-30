CXX=g++

INC_DIR  = ./third
LIB_DIR  = ./third

INCFLAGS = -I $(INC_DIR)/cJSON/include
LDFLAGS	 = -L $(LIB_DIR)/cJSON/lib
LDFLAGS += -lcjson -lcjson_utils

CPPFLAGS = -g

APP_NAME = test

all:
	$(CXX) $(CPPFLAGS)  *.cpp -o $(APP_NAME)  $(INCFLAGS) $(LDFLAGS)

clean: rm -rf $(APP_NAME)