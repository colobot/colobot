# Agregar traducción al español (es)

Las traducciones en Colobot se dividen en **dos partes**:

## Parte 1: Interfaz principal del juego

Se encuentra en `po/` del repositorio principal (este repo).

### Pasos para añadir español:

#### 1. Generar el archivo POT actualizado (template)
```bash
cd /workspaces/colobot
cmake --preset Linux-CI-gcc -DCOLOBOT_LINT_BUILD=ON
cmake --build --preset Linux-CI-gcc --target update-pot
```

Esto crea/actualiza `po/colobot.pot` con todas las cadenas translatable del código.

#### 2. Crear archivo PO para español
```bash
cd po
msginit --input=colobot.pot --locale=es --output=es.po
```

Esto crea `es.po` con todas las cadenas sin traducir.

#### 3. Editar `es.po`
Abre `po/es.po` con tu editor favorito (o Poedit, herramienta gráfica):
- Cada entrada tiene `msgid` (original en inglés) y `msgstr` (tu traducción al español).
- Ejemplo:
  ```
  msgid "Robot"
  msgstr "Robot"
  
  msgid "Start mission"
  msgstr "Iniciar misión"
  ```

#### 4. Compilar traducciones
```bash
cd /workspaces/colobot/build
cmake --build . -- -j$(nproc)
```

CMake detectará `es.po` y compilará automáticamente `es.gmo`.

#### 5. Instalar y probar
```bash
cmake --install . --prefix /tmp/colobot-es
LANG=es_ES.UTF-8 /tmp/colobot-es/games/colobot
```

## Parte 2: Archivos de niveles y ayuda

Se encuentran en el repositorio **colobot-data** (submodulo en `data/`).

Documentación completa: https://github.com/colobot/colobot-data/blob/dev/README.i18n.md

⚠️ **NOTA IMPORTANTE**: No traducir aún los niveles de ejercicios de programación ni desafíos, ya que están siendo remodelados.

## Archivos implicados (Parte 1)
- `po/colobot.pot`: Template (plantilla de traducción)
- `po/es.po`: Tu traducción (a crear)
- `po/CMakeLists.txt`: Configuración de gettext/msgmerge/msgfmt
- Código fuente: Busca `TR("string")` en `colobot-base/src/common/restext.cpp` y similares
