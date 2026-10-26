# 🌌 LonetLaunch - Next-Gen Nintendo Switch Launcher

**LonetLaunch** es un launcher / HOME Menu replacement para Nintendo Switch diseñado con estándares modernos de ingeniería de software: **C++20**, arquitectura multi-hilo asíncrona, renderizado acelerado por hardware a 60 FPS estables y un sistema de seguridad **Failsafe Anti-Bootloop**.

Inspirado en la filosofía de proyectos como *slaunch* y *uLaunch*, pero optimizado para evitar cuelgues, tirones de I/O y riesgos para la consola.

---

## ⚡ ¿Por qué es más estable, fluido y seguro?

### 1. Cero Tirones (Asynchronous Asset Loading en C++20)
* **El problema habitual:** En launchers tradicionales, leer la microSD o memoria NAND para cargar los iconos y metadatos de decenas de juegos bloquea el bucle de dibujado, provocando congelamientos molestos (*micro-stutters*).
* **Solución de LonetLaunch:** La lectura del servicio `ns` (`nsGetApplicationControlData`) y la decodificación de las imágenes JPEG se ejecutan en un **hilo worker secundario** (`std::thread` / `std::condition_variable`).
* **Resultado:** La interfaz gráfica mantiene **60 FPS fijos** en todo momento. Mientras los iconos se cargan en segundo plano, se muestran tarjetas con efecto visual *skeleton loading*.

### 2. Cero Peligro de Bootloop (Failsafe & Crash Guard)
* **El peligro real en Switch:** Si un sustituto del HOME Menu (`qlaunch`) sufre un error fatal al inicio, Atmosphère entra en una pantalla de error en bucle que requiere retirar la microSD y conectarla al PC para borrar la carpeta.
* **Mecanismos de Protección de LonetLaunch:**
  1. **Bypass de Emergencia por Hardware:** Si mantienes pulsados los botones **L + R** o el botón **MINUS (-)** al encender o volver al menú, LonetLaunch se desactiva inmediatamente y cede el control al sistema oficial de forma segura.
  2. **Detección de Caída Previa (Lockfile Watchdog):** Al arrancar, se registra una marca de sesión en `/switch/lonetlaunch_lock.tmp`. Si la consola se apaga abruptamente o la sesión crasheó en menos de 5 segundos, el próximo arranque aborta la carga para protegerte.
  3. **Modo Dual Seguro:** Se compila como `.nro` independiente para que puedas probarlo con total tranquilidad desde el *Homebrew Menu* antes de sustituir el menú de Nintendo.

### 3. Fluidez y Renderizado Moderno
* **Aceleración por GPU (SDL2 + OpenGL ES):** Renderizado por hardware con VSync activado.
* **Resolución Dinámica:** Configuración lógica a 1080p con escalado automático e instantáneo cuando pasas de Modo TV a Modo Portátil (720p).
* **Tipografía del Sistema de Nintendo:** Utiliza las fuentes compartidas oficiales de la consola mediante `plGetSharedFontByType`, garantizando soporte de caracteres globales (incluyendo caracteres japoneses, acentos, etc.) sin archivos externos.
* **Física de Desplazamiento Suave:** Animaciones con curvas *ease-out* exponenciales para la selección de juegos y pulsaciones de neón dinámicas.

---

## 📁 Estructura del Código

```text
switch-launcher/
├── Makefile                       # Build system optimizado para ARMv8 Cortex-A57
├── qlaunch.json                   # Descriptor NPDM con capacidades de kernel de sistema
├── package_qlaunch.sh             # Script para generar el reemplazo de Atmosphère
├── lonetlaunch.nro                # Binario listo para Homebrew Menu
│
├── source/
│   ├── main.cpp                   # Punto de entrada y gestión del Failsafe
│   ├── core/
│   │   ├── app.hpp / app.cpp      # Bucle a 60 FPS, física de carrusel y UI
│   │   └── failsafe.hpp / .cpp    # Detección de arranque seguro y monitoreo
│   ├── gfx/
│   │   └── renderer.hpp / .cpp    # Abstracción SDL2, tarjetas redondeadas y fuentes
│   └── sys/
│       ├── status.hpp / .cpp      # Batería, reloj RTC y detección Dock/Handheld
│       └── title_manager.hpp / .cpp # Hilo worker asíncrono y launcher de juegos
```

---

## 🎮 Controles

| Botón | Acción |
| :--- | :--- |
| **D-Pad / Stick Izquierdo** | Navegar entre juegos con desplazamiento suave |
| **Botón A** | Iniciar juego seleccionado |
| **Botón Y** | Recargar catálogo de juegos |
| **Botón +** | Salir al Homebrew Menu |
| **Pantalla Táctil** | Toque en laterales para scroll, toque central para iniciar |
| **L + R (Al arrancar)** | **Failsafe:** Desactivar launcher y entrar al menú seguro |

---

## 🚀 Cómo Usarlo e Instalarlo

### Método A: Probar como aplicación Homebrew (100% Seguro)
1. Copia el archivo `lonetlaunch.nro` a la carpeta `sdmc:/switch/` de tu tarjeta SD.
2. Abre el **Homebrew Menu** en tu consola (desde el Álbum).
3. Selecciona **LonetLaunch** y pruébalo.

### Método B: Usar como Menú HOME Principal (Reemplazo de qlaunch)
1. Ejecuta el script de empaquetado:
   ```bash
   ./package_qlaunch.sh
   ```
2. Esto creará la estructura dentro de `atmosphere/contents/0100000000001000/`.
3. Copia la carpeta `atmosphere` resultante directamente a la raíz de tu tarjeta MicroSD.
4. Reinicia la consola: **LonetLaunch** será tu nuevo menú de inicio.
   *(Recuerda: si en algún momento deseas regresar al menú original, basta con mantener pulsado `L+R` o borrar la carpeta `0100000000001000` de tu SD).*

---

## 🛠️ Cómo Compilar desde las Fuentes

Requiere **devkitPro** con `switch-dev` y las librerías SDL2:

```bash
make clean
make -j$(nproc)
```
Generará `lonetlaunch.nro` automáticamente.
