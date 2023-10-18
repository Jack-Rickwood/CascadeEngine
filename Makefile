TARGET_EXEC := raytrace

BUILD_DIR := ./build
SRC_DIRS := ./src

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

VERT_SRCS = $(shell find $(SRC_DIRS) -type f -name "*.vert")
VERT_OBJS = $(VERT_SRCS:%=$(BUILD_DIR)/%.spv)
FRAG_SRCS = $(shell find $(SRC_DIRS) -type f -name "*.frag")
FRAG_OBJS = $(FRAG_SRCS:%=$(BUILD_DIR)/%.spv)
COMP_SRCS = $(shell find $(SRC_DIRS) -type f -name "*.comp")
COMP_OBJS = $(COMP_SRCS:%=$(BUILD_DIR)/%.spv)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LDFLAGS := -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi -lsfml-graphics -lsfml-window -lsfml-system -ltgui

CXX := clang++
CXXFLAGS := -fcolor-diagnostics -fansi-escape-codes -std=c++17 -O2 -g

CPPFLAGS := $(INC_FLAGS) -MMD -MP

GLSLC := glslc
GLSLFLAGS := --target-env=vulkan1.3

$(BUILD_DIR)/$(TARGET_EXEC): $(VERT_OBJS) $(FRAG_OBJS) $(COMP_OBJS)
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.spv: %
	mkdir -p $(dir $@)
	$(GLSLC) $(GLSLFLAGS) $< -o $@

.PHONY: test clean

test: $(BUILD_DIR)/$(TARGET_EXEC)
	(cd build && exec ./$(TARGET_EXEC))
	
clean:
	rm -r $(BUILD_DIR)

-include $(DEPS)
