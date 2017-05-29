HOST_GCC=g++
TARGET_GCC=gcc
PLUGIN_SOURCE_FILES=bb_stats.c
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include -fPIC -fno-rtti

bb_stats.so: $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) -O0 -g -shared $(CXXFLAGS) $^ -o $@

clean:
	rm -f bb_stats.so
