HOST_GCC=g++
TARGET_GCC=gcc
PLUGIN_SOURCE_FILES=bb_stats.c
GCCPLUGINS_DIR:= $(shell $(TARGET_GCC) -print-file-name=plugin)
CXXFLAGS+= -I$(GCCPLUGINS_DIR)/include -fPIC -fno-rtti

bb_stats.so: $(PLUGIN_SOURCE_FILES)
	$(HOST_GCC) -O0 -g -shared $(CXXFLAGS) $^ -o $@

test: bb_stats.so
	gcc -fplugin=./bb_stats.so test.c -fplugin-arg-bb_stats-output="test.out" -g -O0

test-clean:
	rm -f a.out test.out *.bb
clean: test-clean
	rm -f bb_stats.so
