# 🌌 LonetLaunch - Next-Gen Nintendo Switch Launcher

**LonetLaunch** es un reemplazo completo del HOME Menu para Nintendo Switch construido sobre la arquitectura de alta fidelidad de **sLaunch / uLaunch**. Incluye aceleración gráfica por hardware (OpenGL), soporte para fondos de vídeo con hardware NVDEC, widgets interactivos en Lua, música de fondo (MP3/OGG/FLAC), paquetes de iconos, transferencia de archivos por USB (MTP) e instalador integrado en consola.

---

## 🏗️ ¿Cómo funciona la arquitectura para evitar pantallas negras?

En Nintendo Switch, el HOME Menu oficial (`qlaunch`) no es una aplicación normal. Para que funcione sin pantalla negra, el sistema se divide en:

1. **`sSystem` (`atmosphere/contents/0100000000001000/`):**
   * Daemon de sistema que atiende los eventos del SO Horizon (botón HOME, suspensión, cartuchos).
   * Utiliza el servicio **Atmosphere ECS** (*External Content Service*) para cargar la interfaz gráfica en un slot de applet.
2. **`sMenu` (`slaunch/bin/sMenu/`):**
   * El menú gráfico de alta velocidad que renderiza los juegos, temas, carátulas y widgets.
3. **`slaunch/` (Assets, fuentes y widgets):**
   * Contiene las fuentes CJK globales, efectos de sonido de UI, música y widgets en Lua.
4. **`sInstaller` (`switch/sInstaller/`):**
   * Aplicación homebrew para instalar, actualizar o desinstalar el launcher directamente desde la Switch.

---

## 🚀 Guía de Instalación (2 Métodos)

### 🥇 Método 1: Instalación desde la Switch con el Instalador (Recomendado)
Este método es el más seguro y automático:
1. Copia el archivo `switch/sInstaller/sInstaller.nro` a tu tarjeta SD en la ruta:
   ```text
   sdmc:/switch/sInstaller/sInstaller.nro
   ```
2. Enciende tu Switch, abre el **Homebrew Menu** (Álbum) y abre **sInstaller**.
3. Presiona **Install** en la pantalla: el instalador configurará automáticamente los archivos en tu MicroSD.
4. Reinicia la consola y el menú cargará de inmediato.
   *(Para desinstalarlo en cualquier momento, vuelve a abrir `sInstaller` y presiona **Uninstall**).*

---

### 🥈 Método 2: Instalación Manual Directa (Drag & Drop)
1. Descarga el paquete completo **`LonetLaunch-Complete-SD.zip`**.
2. Extrae las 3 carpetas directamente en la **raíz de tu tarjeta MicroSD**:
   ```text
   sdmc:/ (Raíz de la MicroSD)
   ├── atmosphere/
   │   └── contents/
   │       └── 0100000000001000/
   │           └── exefs/
   │               ├── main
   │               └── main.npdm
   ├── slaunch/                       <-- ¡INDISPENSABLE PARA QUE NO QUEDE EN NEGRO!
   │   ├── bin/
   │   │   ├── sMenu/
   │   │   ├── hbloader/
   │   │   └── hbloader_app/
   │   ├── covers/
   │   ├── fonts/
   │   ├── icon_packs/
   │   ├── icons/
   │   ├── music/
   │   ├── sounds/
   │   └── widgets/
   └── switch/
       └── sInstaller/
           └── sInstaller.nro
   ```
3. Reinicia tu Switch y cargará el menú con su interfaz, música y temas.

---

## 🎮 Controles y Atajos

* **D-Pad / Stick Izquierdo:** Navegar por los juegos y aplicaciones.
* **Botón A:** Iniciar juego / Abrir carpeta.
* **Botón Y:** Menú de opciones del título.
* **Botón X:** Cambiar vista (Coverflow, Grid, Lista, XMB).
* **Botón (-):** Ajustes de personalización (Temas, Widgets, Fondos, Música).
* **Botón (+):** Menú de energía (Apagar, Reiniciar, Payload).

---

## 🎨 Personalización

* **Fondos de pantalla:** Coloca imágenes JPG o vídeos MP4 en `sdmc:/slaunch/themes/`.
* **Música de fondo:** Coloca tus canciones (MP3, OGG, FLAC) en `sdmc:/slaunch/music/`.
* **Widgets:** Añade o edita widgets en Lua dentro de `sdmc:/slaunch/widgets/`.
