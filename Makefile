include config.mk

SRC = src/swayxcursor.c
OBJ = $(SRC:.c=.o)
ASSETS_DIR = assets

REAL_USER = $(shell logname)
REAL_HOME = $(shell getent passwd $(REAL_USER) | cut -d: -f6)
SWAY_CONFIG = $(REAL_HOME)/.config/sway/config
SWAY_RULE = for_window [title="SwayXcursor"] floating enable, border normal

all: swayxcursor

.c.o:
	${CC} -c ${CFLAGS} $< -o $@

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
	@mkdir -p $(dir $(SWAY_CONFIG))
	@test -f $(SWAY_CONFIG) && (grep -qF "${SWAY_RULE}" $(SWAY_CONFIG) || echo "${SWAY_RULE}" >> $(SWAY_CONFIG)) || true
	@echo "4: Finalizing installation and forcing reload..."
	@update-desktop-database -q ${DESTDIR}${PREFIX}/share/applications || true
	@-runuser -l $(REAL_USER) -c "SWAYSOCK=$$(ls /run/user/$$(id -u $(REAL_USER))/sway-ipc.* | head -n 1) swaymsg reload" >/dev/null 2>&1 || true
	@echo "5: Installation complete! You can find SwayXcursor in your app launcher."

uninstall:
	@echo "1: Removing binary from ${PREFIX}/bin..."
	@rm -f ${DESTDIR}${PREFIX}/bin/swayxcursor
	@echo "2: Removing desktop entry..."
	@rm -f ${DESTDIR}${PREFIX}/share/applications/swayxcursor.desktop
	@echo "3: Reverting Sway configuration rules..."
	@test -f $(SWAY_CONFIG) && sed -i '/SwayXcursor/d; /xcursor_theme/d' $(SWAY_CONFIG) || true
	@echo "4: Finalizing uninstallation and forcing reload..."
	@-runuser -l $(REAL_USER) -c "DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$$(id -u $(REAL_USER))/bus gsettings reset org.gnome.desktop.interface cursor-theme" || true
	@-runuser -l $(REAL_USER) -c "DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/$$(id -u $(REAL_USER))/bus gsettings reset org.gnome.desktop.interface cursor-size" || true
	@-runuser -l $(REAL_USER) -c "SWAYSOCK=$$(ls /run/user/$$(id -u $(REAL_USER))/sway-ipc.* | head -n 1) swaymsg reload" >/dev/null 2>&1 || true
	@echo "5: Uninstallation complete!" 
