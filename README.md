<div align="center">

# SwayXcursor

![logo](screenshots/icon.svg)

![Language](https://img.shields.io/badge/C-99-blue.svg)
![GTK](https://img.shields.io/badge/GTK-4.0+-red.svg)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey.svg)
![License](https://img.shields.io/badge/license-GPL--3.0-blue.svg)
![Last Commit](https://img.shields.io/github/last-commit/ZaidAljasmi/SwayXcursor)

</div>

---

## Screenshots

<div align="center">
  <table>
    <tr>
      <!-- <td><img src="screenshots/screenshot1.png" width="400px"></td> -->
      <td><img src="screenshots/screenshot2.gif" width="400px"></td>
    </tr>
  </table>
</div>

---

## Install & Run

```bash
sudo apt install build-essential libgtk-4-dev pkg-config libxcursor-dev
cd ~/swayxcursor
sudo make clean install
swaymsg reload
swayxcursor # Or launch it via wofi or any app launcher of your choice
```
<!-- --- -->

