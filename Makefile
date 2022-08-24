#project
PRODUCT_NAME = testImPlot

#libraries
STATIC_LIBS =
SHARED_LIBS = dl pthread

#include PATHS
INC_PATHS = imgui

################################################################################

#project folders
BIN_PATH = bin
SOURCES_PATH = $(INC_PATHS) ./
BUILD_PATH = obj

#source files extension
SOURCES_EXT_C = c
SOURCES_EXT_CPP = cpp

#compilers and flags (gcc -g to produce debug symbols)
COMPILER = gcc
COMPILER_FLAGS = -Wall -g -c
LINKER = g++

#OS Specifig flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	TARGET_OS_PATH = linux
	COMPILER_FLAGS += -D LINUX
	SHARED_LIBS += rt
endif
ifeq ($(UNAME_S),Darwin)
	TARGET_OS_PATH = osx
	COMPILER_FLAGS += -D OSX
endif

OBJECTS_PATH = $(BUILD_PATH)/$(TARGET_OS_PATH)
SOURCES_C=$(foreach SRC,$(SOURCES_PATH),$(wildcard $(SRC)/*.$(SOURCES_EXT_C)))
SOURCES_CPP=$(foreach SRC,$(SOURCES_PATH),$(wildcard $(SRC)/*.$(SOURCES_EXT_CPP)))
OBJECTS_C=$(patsubst %.$(SOURCES_EXT_C),$(OBJECTS_PATH)/%.o,$(SOURCES_C))
OBJECTS_CPP=$(patsubst %.$(SOURCES_EXT_CPP),$(OBJECTS_PATH)/%.o,$(SOURCES_CPP))

INCOMPILER_FLAGS=$(foreach TMP,$(INC_PATHS),-I$(TMP))
LIB_FLAGS=$(foreach TMP,$(LIB_PATHS),-L$(TMP))
SHARED_LIB_FLAGS=$(addprefix -l,$(SHARED_LIBS))
STATIC_LIB_FLAGS=$(STATIC_LIBS)

BINARY=$(BIN_PATH)/$(TARGET_OS_PATH)/$(PRODUCT_NAME)

.PHONY: all distclean clean

all: $(BINARY)

buildrepo:
	@echo ". Target:	$(UNAME_S)"
	@$(call make-repo)

$(BINARY): buildrepo $(OBJECTS_C) $(OBJECTS_CPP)
	@echo ". Linking:	[$(LINKER)]	$@"
	@mkdir -p $(BIN_PATH)/$(TARGET_OS_PATH)
	@$(LINKER) $(LIB_FLAGS) $(OBJECTS_C) $(OBJECTS_CPP) $(SHARED_LIB_FLAGS) $(STATIC_LIB_FLAGS) -o $@ $(LIBS)
	@$(OBJECTS_DELETE_C)
	@$(OBJECTS_DELETE_CPP)

$(OBJECTS_PATH)/%.o: %.$(SOURCES_EXT_C)
	@echo ". Compiling:	[$(COMPILER)] as c   	$< ..."
	@$(COMPILER) $(INCOMPILER_FLAGS) $(COMPILER_FLAGS) $< -o $@

$(OBJECTS_PATH)/%.o: %.$(SOURCES_EXT_CPP)
	@echo ". Compiling:	[$(COMPILER)] as c++	$< ..."
	@$(COMPILER) $(INCOMPILER_FLAGS) $(COMPILER_FLAGS) $< -o $@

run: all
	@echo ". Running:	[$(PRODUCT_NAME)]"
	@echo ""
	@$(BINARY)

distclean: clean
	@echo ". Cleaning:	[$(PRODUCT_NAME)]"
	@rm -f $(BINARY)

clean:
	@echo ". Cleaning objects & IDE data"
	@rm -rf $(OBJECTS_PATH)/*

githubrequirements:
	@echo ". Install project requirements on $(UNAME_S)"
	@echo ". clone imgui"
	@git clone https://github.com/ocornut/imgui.git
	@echo ". create main.cpp"
	@echo "#include \"imgui.h\"" > main.cpp
	@echo "#include \"implot.h\"" >> main.cpp
	@echo "int main(int, char**)" >> main.cpp
	@echo "{" >> main.cpp
	@echo "    IMGUI_CHECKVERSION();" >> main.cpp
	@echo "    ImGui::CreateContext();" >> main.cpp
	@echo "    ImPlot::CreateContext();" >> main.cpp
	@echo "    ImGuiIO& io = ImGui::GetIO();" >> main.cpp
	@echo "    unsigned char* tex_pixels = NULL;" >> main.cpp
	@echo "    int tex_w, tex_h;" >> main.cpp
	@echo "    io.Fonts->GetTexDataAsRGBA32(&tex_pixels, &tex_w, &tex_h);" >> main.cpp
	@echo "    io.DisplaySize = ImVec2(1920, 1080);" >> main.cpp
	@echo "    io.DeltaTime = 1.0f / 60.0f;" >> main.cpp
	@echo "    ImGui::NewFrame();" >> main.cpp
	@echo "    ImGui::ShowDemoWindow(NULL);" >> main.cpp
	@echo "    ImPlot::ShowDemoWindow(NULL);" >> main.cpp
	@echo "    ImGui::Render();" >> main.cpp
	@echo "    ImPlot::DestroyContext();" >> main.cpp
	@echo "    ImGui::DestroyContext();" >> main.cpp
	@echo "    return 0;" >> main.cpp
	@echo "}" >> main.cpp
	
help:
	@echo ". Makefile commands"
	@echo "make [github]requirements -> install requirements"
	@echo "make                      -> build app (compiler + linker)"
	@echo "make distclean            -> clean built app and objectes"
	@echo "make clean                -> clean built objectes"
	@echo "make run                  -> build and run app"

define make-repo
	for dir in $(SOURCES_PATH); \
	do \
		mkdir -p $(OBJECTS_PATH)/$$dir; \
	done
endef
