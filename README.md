# NapBuddy 💤

> **Avans Hogeschool — Periode 2.1 (LU1)**

NapBuddy is een embedded project ontwikkeld in C/C++ met **PlatformIO**. Dit project is gebouwd en getest op zowel
Linux- als Windows-omgevingen.

---

## Vereisten

Zorg ervoor dat de volgende tools geïnstalleerd zijn op je systeem:

* **[Git](https://git-scm.com/)**
* **Python 3.7+** én **`pip`** (PlatformIO heeft `pip` en `virtualenv` nodig om toolchains op te bouwen)
* **IDE naar keuze:**
    * **[JetBrains CLion](https://www.jetbrains.com/clion/)** met de **PlatformIO for CLion** plugin. (Aangeraden)
    * **[VS Code](https://code.visualstudio.com/)** met de **PlatformIO IDE** extensie.
    * *Of:* **[PlatformIO Core (CLI)](https://docs.platformio.org/en/latest/core/installation/index.html)** als je liever vanuit de terminal werkt.

<small>NOTE: We hebben niks geprobeerd met Visual Studio dus we weten niet wat daar de mogelijkheden binnen zijn.</small>

---

## Installatie & Setup

1. **Kloon de repository:**
   ```bash
   git clone https://github.com/YP501/Avans-2.1-LU1-NapBuddy.git
   cd Avans-2.1-LU1-NapBuddy
   ```

2. **Open het project in je IDE:**
    * **VS Code:** Open de map via `File -> Open Folder` of typ `code .` in de terminal.
    * **CLion:** Open de map via `Open` rechtsbovenin.

---

## Building & Uploading

### Methode 1: JetBrains CLion (Plug & Play)

CLion heeft ingebouwde ondersteuning voor PlatformIO en is nagenoeg **plug & play**:

1. Zorg dat de plugin **PlatformIO for CLion** geïnstalleerd is (`Settings/Preferences` -> `Plugins`).
2. Zorg ervoor dat **PlatformIO Core** op je systeem is geïnstalleerd. Veel linux distros hebben hier een package voor:
   ```bash
   sudo pacman -S platformio-core
   ```
3. Open de map van het project in CLion.
4. Rechtsboven in de statusbalk zie je direct de run/build target selectie (zoals `PlatformIO`).
5. **Build & Upload:**
    * Selecteer de `PlatformIO` build-target in het drop-down menu rechtsbovenin.
    * Sluit de microcontroller aan en klik op de groene **Run-knop**.
6. **Serial Monitor:**
    * `Right-click` -> `PlatformIO` -> `PlatformIO Serial Monitor` (let erop dat de Serial Monitor uit moet staan tijdens het uploaden!)

---

### Methode 2: VS Code

1. Open het project in VS Code.
2. Zorg ervoor dat de extension [PlatformIO IDE for VSCode](https://docs.platformio.org/en/latest/integration/ide/vscode.html) is geïnstalleerd.
3. Sluit de microcontroller aan.
4. In de balk onderin, klik op het vinkje om te builden en op het pijltje om te uploaden.

---

### Methode 3: PlatformIO CLI

Als je de voorkeur geeft aan de terminal:

#### 1. Project compileren (Build)

```bash
pio run
```

#### 2. Firmware uploaden (Upload)

```bash
pio run --target upload
```

#### 3. Seriële monitor openen

```bash
pio device monitor
```

---

## Linux-specifieke instructies

Op Linux moet je gebruiker toegangsrechten hebben tot seriële poorten (`/dev/ttyUSB*` of `/dev/ttyACM*`).

1. **Voeg jezelf toe aan de `dialout` / `uucp` groep:**

      ```bash
      sudo usermod -aG dialout,uucp $USER
      ```
   Herstart je computer of log opnieuw in om de groepsrechten toe te passen.

2. **PlatformIO udev-regels instellen (aanbevolen):**
   ```bash
   curl -fsSL https://raw.githubusercontent.com/platformio/platformio-core/develop/platformio/assets/system/99-platformio-udev.rules | sudo tee /etc/udev/rules.d/99-platformio-udev.rules
   sudo udevadm control --reload-rules
   sudo udevadm trigger
   ```
   Het kan zijn dat jouw distro een hiervoor een package in zijn repository heeft. Arch linux heeft hier bijvoorbeeld de volgende package voor:
    ```bash
   sudo pacman -S platformio-core-udev
   ```

---