#!/bin/bash
# Script para instalar ESP-IDF en macOS

set -e

echo "=========================================="
echo "ESP-IDF Installation Script for macOS"
echo "=========================================="
echo ""

# Verificar si ya está instalado
if [ -d "$HOME/esp/esp-idf" ]; then
    echo "✓ ESP-IDF ya está instalado en $HOME/esp/esp-idf"
    echo ""
    echo "Para usarlo, ejecuta:"
    echo "  source $HOME/esp/esp-idf/export.sh"
    echo "  cd /Users/e.baena/CascadeProjects/walter-nbiot-espidf"
    echo "  idf.py build"
    exit 0
fi

echo "Instalando ESP-IDF..."
echo ""

# Crear directorio
mkdir -p ~/esp
cd ~/esp

# Clonar ESP-IDF v5.3 (versión estable recomendada)
echo "1. Clonando ESP-IDF v5.3..."
git clone -b v5.3 --recursive https://github.com/espressif/esp-idf.git

# Instalar herramientas
echo ""
echo "2. Instalando herramientas de ESP-IDF..."
cd ~/esp/esp-idf
./install.sh esp32s3

echo ""
echo "=========================================="
echo "✓ ESP-IDF instalado correctamente!"
echo "=========================================="
echo ""
echo "Para usar ESP-IDF, ejecuta estos comandos:"
echo ""
echo "  # Activar ESP-IDF (necesario en cada terminal nueva)"
echo "  source ~/esp/esp-idf/export.sh"
echo ""
echo "  # Ir al proyecto"
echo "  cd /Users/e.baena/CascadeProjects/walter-nbiot-espidf"
echo ""
echo "  # Compilar"
echo "  idf.py build"
echo ""
echo "Para hacerlo permanente, agrega esto a tu ~/.zshrc:"
echo "  alias get_idf='. ~/esp/esp-idf/export.sh'"
echo ""
