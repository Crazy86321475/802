CUR_DIR:=./

CC:=gcc

MACRO:=

CFLAGS:=\
    -Wfatal-errors	\
    -Wall			\
	# -std=gnu90

#@echo "TEST : $(TEST)"

ifdef test
    $(warning 'get test 1')
    MACRO+=TEST=1
else
    $(warning $(test))
endif

ifdef pc
    $(warning 'get pc 1')
    MACRO+=PC=1
else
    $(warning $(pc))
endif

CFLAGS+= $(patsubst %, -D%, $(MACRO))

INCLUDE_DIR:=	\
    -I./include

SRC_DIR:=src
CXX_SOURCES:=$(foreach dir, $(SRC_DIR), $(wildcard $(dir)/*.c))


OBJ_DIR:=objs
OBJS:=$(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(CXX_SOURCES)))


$(warning $(CXX_SOURCES))
$(warning $(OBJS))

TARGET:=./run


##############################################################


$(TARGET):$(OBJS)
	$(CC) -o $@ $^


$(OBJS):$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	@ if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi; 
	$(CC)  $(CFLAGS) $(INCLUDE_DIR) -c $< -o $@ 

.PHONY:clean
clean:
	rm -f ./objs/* $(TARGET)

