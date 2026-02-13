include config.mk

SRC = src/swayxcursor.c
OBJ = src/swayxcursor.o
ASSETS_DIR = assets

REAL_USER = $(shell logname)
REAL_HOME = /home/$(REAL_USER)
SWAY_CONFIG = $(REAL_HOME)/.config/sway/config
SWAY_RULE = for_window [title="SwayXcursor"] floating enable

all: swayxcursor

$(OBJ): $(SRC)
	${CC} -c ${CFLAGS} $(SRC) -o $(OBJ)

swayxcursor: ${OBJ}
	${CC} -o $@ ${OBJ} ${LDFLAGS}

clean:
	rm -f swayxcursor src/*.o

install: all
	@echo "1: Installing binary to ${PREFIX}/bin..."
	@mkdir -p ${DESTDIR}${PREFIX}/bin
	@cp -f swayxcursor ${DESTDIR}${PREFIX}/bin
	@chmod 755 ${DESTDIR}${PREFIX}/bin/swayxcursor

	@echo "2: Installing desktop entry..."
	@mkdir -p ${DESTDIR}${PREFIX}/share/applications
	@cp -f ${ASSETS_DIR}/swayxcursor.desktop ${DESTDIR}${PREFIX}/share/applications/

	@echo "3: Applying Sway configuration rules..."
	@test -f $(SWAY_CONFIG) && (grep -qF "${SWAY_RULE}" $(SWAY_CONFIG) || echo "${SWAY_RULE}" >> $(SWAY_CONFIG)) || true

	@echo "4: Finalizing installation..."
	@update-desktop-database -q ${DESTDIR}${PREFIX}/share/applications || true
	@-su $(REAL_USER) -c "swaymsg reload" >/dev/null 2>&1 || true
	@echo "5: Installation complete! You can find SwayXcursor in your app launcher."

uninstall:
	@echo "Removing files..."
	@rm -f ${DESTDIR}${PREFIX}/bin/swayxcursor
	@rm -f ${DESTDIR}${PREFIX}/share/applications/swayxcursor.desktop
	@echo "Removing Sway rule..."
	@test -f $(SWAY_CONFIG) && sed -i '/for_window \[title="SwayXcursor"\] floating enable/d' $(SWAY_CONFIG) || true
	@-su $(REAL_USER) -c "swaymsg reload" >/dev/null 2>&1 || true

.PHONY: all clean install uninstall
