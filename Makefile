include .env

CFLAGS = -g -Wall
LDFLAGS = -lcglm `pkg-config --static --libs glfw3` -lvulkan

vert_sources = $(shell find ./shaders -type f -name "*.vert")
vert_obj_files = $(patsubst %.vert, %.vert.spv, $(vert_sources))
frag_sources = $(shell find ./shaders -type f -name "*.frag")
frag_obj_files = $(patsubst %.frag, %.frag.spv, $(frag_sources))

TARGET = bean
$(TARGET): $(vert_obj_files) $(frag_obj_files)
$(TARGET): *.c
	gcc $(CFLAGS) -o bean *.c $(LDFLAGS)

%.spv: %
	${GLSLC} $< -o $@

.PHONY: test clean

test: bean
	./bean

clean:
	rm -f bean
	rm -f shaders/*.spv