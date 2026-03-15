PROG_NAME := Solitaire
PROG_NAME_LC  := $(shell echo $(PROG_NAME) | tr A-Z a-z)

SRC := src
OUT := out
BUILD := build

PROG_VER := 0.0
BUNDLE_VER := 1
BUNDLE_ID := com.suityourselfgames.$(PROG_NAME_LC)

PROG_LOC := $(OUT)/$(PROG_NAME)

FILES := $(shell find src -name '*.c')
OBJS  := $(FILES:$(SRC)/%.c=$(BUILD)/%.o)
DEPS  := $(OBJS:.o=.d)

ASSETS := assets
SHADERS := $(ASSETS)/shaders
MODELS := $(ASSETS)/models
SOURCE_ICON := $(ASSETS)/Icon1024.png

CFLAGS := -fdiagnostics-color=always -g -Wall -Werror -Iinclude -MMD -MP
LDFLAGS := -lSDL2 -lSDL2_image -framework CoreFoundation

ASSET_DIRS := $(shell find $(ASSETS) -mindepth 1 -maxdepth 1 -type d)
ASSET_NAMES := $(filter-out _%,$(notdir $(ASSET_DIRS)))

#OS specifics.
UNAME_S := $(shell uname -s)

all: build run

ifeq ($(UNAME_S),Darwin)

# General vals.
OS := MAC
BUILD_RES := build-res/MacOS
ARGS := $(GEN_ARGS) -framework CoreFoundation

# .app package folders.
APP := $(OUT)/$(PROG_NAME).app
APP_CONTENTS := $(APP)/Contents
APP_RESOURCES := $(APP_CONTENTS)/Resources
APP_FRAMEWORKS := $(APP_CONTENTS)/Frameworks
APP_MAC_OS := $(APP_CONTENTS)/MacOS

# Icon vals.
ICON_NAME := Icon
ICON_SET := $(ICON_NAME).iconset
ICON := $(ICON_NAME).icns

# Substitutions for Info.plist.
SUBS := -e 's:PROG_NAME:$(PROG_NAME):' -e 's:BUNDLE_ID:$(BUNDLE_ID):' -e 's:BUNDLE_VER:$(BUNDLE_VER):' -e 's:PROG_VER:$(PROG_VER):' -e 's:ICON_FILE:$(ICON):'

.PHONY: mac-app mac-zip mac-icns

# Create a .app package.
mac-app: unity $(PROG_LOC)
#Create the required directories.
	mkdir -p $(APP_RESOURCES)
	mkdir $(APP_MAC_OS)
	mkdir $(APP_FRAMEWORKS)
# Place files in the package.
	cp $(BUILD_RES)/libSDL2-2.0.0.dylib $(APP_FRAMEWORKS)/libSDL2-2.0.0.dylib
	cp $(BUILD_RES)/Info.plist $(APP_CONTENTS)/Info.plist
	sed $(SUBS) $(BUILD_RES)/Info.plist > $(APP_CONTENTS)/Info.plist
	cp $(BUILD_RES)/Icon.icns $(APP_RESOURCES)/$(ICON)
	for d in $(ASSET_NAMES); do cp -R $(ASSETS)/$$d $(APP_RESOURCES)/$$d; done
	cp $(PROG_LOC) $(APP_MAC_OS)/$(PROG_NAME)

# Set the program to look in the apps frameworks instead of the pcs frameworks.
# Probably weird and not suggested :/. Only an issue if it doesn't work on another computer.
	install_name_tool -change $(shell brew --prefix sdl2)/lib/libSDL2-2.0.0.dylib @executable_path/../Frameworks/libSDL2-2.0.0.dylib $(APP_MAC_OS)/$(PROG_NAME)

# Create and zip a .app package.
mac-zip: mac-app
	zip -r $(PROG_LOC).app.zip $(PROG_LOC).app

# Create icons from a file named Icon1024.png that is 1024x1024 pixels.
mac-icns:
	mkdir $(ICON_SET)
	sips -z 16 16   $(SOURCE_ICON) --out $(ICON_SET)/icon_16x16.png
	sips -z 32 32   $(SOURCE_ICON) --out $(ICON_SET)/icon_16x16@2x.png
	sips -z 32 32   $(SOURCE_ICON) --out $(ICON_SET)/icon_32x32.png
	sips -z 64 64   $(SOURCE_ICON) --out $(ICON_SET)/icon_32x32@2x.png
	sips -z 128 128 $(SOURCE_ICON) --out $(ICON_SET)/icon_128x128.png
	sips -z 256 256 $(SOURCE_ICON) --out $(ICON_SET)/icon_128x128@2x.png
	sips -z 256 256 $(SOURCE_ICON) --out $(ICON_SET)/icon_256x256.png
	sips -z 512 512 $(SOURCE_ICON) --out $(ICON_SET)/icon_256x256@2x.png
	sips -z 512 512 $(SOURCE_ICON) --out $(ICON_SET)/icon_512x512.png
	cp $(SOURCE_ICON) $(ICON_SET)/icon_512x512@2x.png
	iconutil -c icns $(ICON_SET) -o $(BUILD_RES)/$(ICON)
	rm -r $(ICON_SET)

else ifeq ($(UNAME_S),Windows_NT)

# General vals.
OS := WIN
BUILD_RES := build-res/Windows
ARGS := $(GEN_ARGS) -L$(BUILD_RES)/lib

else

$(error "Unsupported Operating system");

endif

.PHONY: build clean unity assets

# all -> depends on the compiled program
build: assets $(PROG_LOC)

unity: clean assets

assets:
	mkdir -p $(OUT)
	for d in $(ASSET_NAMES); do ln -sf ../$(ASSETS)/$$d $(OUT)/; done
ifeq ($(OS), WIN)
	cp $(BUILD_RES)/SDL2.dll $(OUT)/SDL2.dll
endif

clean:
	rm -fr $(OUT)

clean-total:
	rm -fr $(OUT) $(BUILD)

run:
	./$(PROG_LOC)

# Link
$(PROG_LOC): $(OBJS)
	@mkdir -p $(OUT)
	$(CC) $(OBJS) -o $@ $(LDFLAGS) -g

$(BUILD)/%.o: $(SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

-include $(DEPS)