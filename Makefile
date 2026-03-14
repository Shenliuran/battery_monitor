CC				=	gcc
OBJ_DIR			=	$(CURDIR)/obj
SRC_DIR			=	$(CURDIR)/src
INC_DIR			=	$(CURDIR)/include
TARGET			=	battery_monitor
PKG_CONFIG_LIBS	=	libnotify glib-2.0 libudev
CFLAGS			=	-std=c99 -Wall -ggdb
INCS			=	-I$(INC_DIR) $(shell pkg-config --cflags $(PKG_CONFIG_LIBS)) 
LIBS			=	$(shell pkg-config --libs $(PKG_CONFIG_LIBS))
SRCS			:=	$(wildcard $(SRC_DIR)/*.c)
OBJS			=	$(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(INCS) -o $@ $^ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

clean:
	rm -r $(OBJ_DIR) $(TARGET)
