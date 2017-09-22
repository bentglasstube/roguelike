UNAME=$(shell uname)

SOURCES=$(wildcard *.cc)
CONTENT=$(wildcard content/*)
BUILDDIR=build
OBJECTS=$(patsubst %.cc,$(BUILDDIR)/%.o,$(SOURCES))
NAME=roguelike
APP_NAME="Roguelike"
VERSION=$(shell git describe --dirty)

CC=g++
CFLAGS=-O3 --std=c++11 -Wall -Wextra -Werror -pedantic -I$(HOME)/source/gam $(shell sdl2-config --cflags) -DVERSION=\"$(VERSION)\"

ifeq ($(UNAME), Linux)
	PACKAGE=$(NAME)-linux-$(VERSION).tgz
	LDFLAGS=-static-libstdc++ -static-libgcc
	LDLIBS=$(shell sdl2-config --libs) -lSDL2_mixer -lSDL2_image -lSDL2_ttf
endif
ifeq ($(UNAME), Darwin)
	PACKAGE=$(NAME)-osx-$(VERSION)tgz
	LDLIBS=-framework SDL2 -framework SDL2_mixer -framework SDL2_image -framework SDL2_ttf -rpath @executable_path/../Frameworks
	CFLAGS+=-mmacosx-version-min=10.9
endif

EXECUTABLE=$(BUILDDIR)/$(NAME)

all: $(EXECUTABLE)

run: $(EXECUTABLE)
	./$(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS) $(HOME)/source/gam/libgam.a

$(BUILDDIR)/%.o: %.cc
	@mkdir -p $(BUILDDIR)
	$(CC) -c $(CFLAGS) -o $@ $<

package: $(PACKAGE)

$(NAME)-linux-%.tgz: $(EXECUTABLE)
	mkdir $(NAME)
	cp $(EXECUTABLE) README.md $(NAME)
	cp -R content $(NAME)/content
	tar zcf $@ $(NAME)
	rm -rf $(NAME)

$(NAME)-osx-%.tgz: $(APP_NAME).app
	mkdir $(NAME)
	cp -r $(APP_NAME).app $(NAME)/.
	tar zcf $@ $(NAME)
	rm -rf $(NAME)

$(APP_NAME).app: $(EXECUTABLE) launcher $(CONTENT) Info.plist
	rm -rf $(APP_NAME).app
	mkdir -p $(APP_NAME).app/Contents/{MacOS,Frameworks}
	cp $(EXECUTABLE) $(APP_NAME).app/Contents/MacOS/game
	cp launcher $(APP_NAME).app/Contents/MacOS/launcher
	cp -R content $(APP_NAME).app/Contents/MacOS/content
	cp Info.plist $(APP_NAME).app/Contents/Info.plist
	cp -R /Library/Frameworks/SDL2.framework $(APP_NAME).app/Contents/Frameworks/SDL2.framework
	cp -R /Library/Frameworks/SDL2_mixer.framework $(APP_NAME).app/Contents/Frameworks/SDL2_mixer.framework
	cp -R /Library/Frameworks/SDL2_image.framework $(APP_NAME).app/Contents/Frameworks/SDL2_image.framework

clean:
	rm -rf $(BUILDDIR) $(APP_NAME).app $(PACKAGE)

.PHONY: all clean run package
