#!/usr/bin/env bash
set -e

echo "=== Empaquetando LonetLaunch como Reemplazo de Qlaunch ==="

DEVKITPRO=/opt/devkitpro
PATH=$DEVKITPRO/devkitA64/bin:$DEVKITPRO/tools/bin:$PATH

OUTDIR="atmosphere/contents/0100000000001000"
mkdir -p "$OUTDIR/exefs"

echo "[1/3] Generando NPDM con permisos de sistema..."
npdmtool qlaunch.json "$OUTDIR/exefs/main.npdm"

echo "[2/3] Convirtiendo ejecutable ELF a NSO..."
elf2nso lonetlaunch.elf "$OUTDIR/exefs/main"

echo "[3/3] Estructura completada en: $OUTDIR"
echo ">> Para probar sin riesgo, primero usa lonetlaunch.nro desde el Album (hbmenu)."
echo ">> Cuando estés listo para sustituir el menú oficial, copia la carpeta 'atmosphere' a la raíz de tu tarjeta MicroSD."
