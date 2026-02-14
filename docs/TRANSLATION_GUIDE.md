# Colobot Spanish Translation Guide

This document provides guidelines for creating and maintaining Spanish translations in Colobot: Gold Edition.

## Terminology Guidelines (IMPORTANT)

Colobot uses **"bot"** as the standard term throughout the game. This is part of the game's lore and branding (COLO-BOT = COLOnize + ROBOT).

### Correct Terminology
| English | Spanish |
|---------|----------|
| bot | bot |
| bots | bots |
| robot | **DO NOT USE** - use "bot" instead |
| bots with wheels | bots con ruedas |
| bots with wings | bots alados |
| bots with tracks | bots con oruga |
| shooter bot | bot disparador |
| grabber bot | bot recogedor |
| factory | fábrica de bots |
| bot factory | fábrica de bots |
| power cell | celda de energía |
| defense tower | torre de defensa |
| repair center | centro de reparación |
| power station | central eléctrica |
| research center | centro de investigación |

## Issues Found and Resolved

### Issue 1: Missing PO File Header
**Problem**: Spanish .po files were missing the required metadata header block.

**Example of INCORRECT header**:
```po
# Spanish translations for PACKAGE package
msgid "Prepare yourself..."
msgstr "Prepárate para..."
```

**Example of CORRECT header**:
```po
# This file is part of the Colobot: Gold Edition source code
# Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
# This file is distributed under the same license as the Colobot package.
msgid ""
msgstr ""
"Project-Id-Version: 0.1.11\n"
"POT-Creation-Date: DATE\n"
"PO-Revision-Date: 2025-02-11 12:00+01:00\n"
"Last-Translator: Spanish Translation Team\n"
"Language-Team: Spanish\n"
"Language: Spanish\n"
"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=UTF-8\n"
"Content-Transfer-Encoding: 8bit\n"
"Plural-Forms: nplurals=2; plural=(n != 1);\n"
"X-Language: es_ES\n"
"X-Source-Language: en_US\n"
```

### Issue 2: Missing msgid Entries
**Problem**: The Spanish .po file was missing `msgid "Equipment"` entry that exists in the template.

**Template (scene101.pot)**:
```po
#. type: Title-text
#: ../scene.txt:1
#, no-wrap
msgid "Equipment"
msgstr ""
```

**Must be translated as**:
```po
#. type: Title-text
#: ../scene.txt:1
#, no-wrap
msgid "Equipment"
msgstr "Equipo"
```

### Issue 3: Duplicate Entries
**Problem**: Spanish .po files had duplicate msgid entries with different translations.

**INCORRECT**:
```po
msgid "Prepare yourself to embark on mankind's most thrilling adventure yet."
msgstr "Prepárate para embarcarte en la aventura más emocionante de la humanidad hasta ahora"

# Duplicate (WRONG):
msgid "Prepare yourself to embark on mankind's most thrilling adventure yet."
msgstr "Prepárate para embarcarte en la aventura más emocionante de la humanidad."
```

**CORRECT** (single entry):
```po
msgid "Prepare yourself to embark on mankind's most thrilling adventure yet."
msgstr "Prepárate para embarcarte en la aventura más emocionante de la humanidad."
```

## Translation File Structure

### Level Translation Files
Location: `data/levels/missions/chapter*/level*/po/es.po`

**Required entries for level translation**:
```po
# Title-text (appears as chapter heading)
msgid "Equipment"
msgstr "Equipo"

# Resume-text (mission description)
msgid "Prepare yourself to embark on mankind's most thrilling adventure yet."
msgstr "Prepárate para embarcarte en la aventura más emocionante de la humanidad."

# Help file translations
#. type: \b; header
#: ../help/help.E.txt:1
msgid "From the NASA Training Center"
msgstr "Del Centro de Entrenamiento de la NASA"
```

### Help File Translations
Location: `data/levels/missions/chapter*/level*/help/help.S.txt`

The help files are generated from .po files using the translation build system. DO NOT edit help.S.txt directly - edit es.po instead.

## Build System Translation Process

### How Translations Are Generated

1. **Template files** (`.pot`) contain source strings in English
2. **Translation files** (`.po`) contain translations for each language
3. **Translation script** (`i18n-tools/scripts/process_translations.py`) merges translations

### Commands to Regenerate Translations

```bash
cd /home/rrodriguez/Documentos/GitHub/colobot/data

# Generate level translations
python3 i18n-tools/scripts/process_translations.py \
    --mode generate \
    --type level \
    --input_dir levels/missions/chapter001/level001 \
    --po_dir levels/missions/chapter001/level001/po \
    --output_dir /tmp/colobot-translations/level001

# Generate help translations
python3 i18n-tools/scripts/process_translations.py \
    --mode generate \
    --type help \
    --input_dir levels/missions/chapter001/level001/help \
    --po_dir levels/missions/chapter001/level001/po \
    --output_dir /tmp/colobot-translations/level001 \
    --output_subdir help
```

### CMake Integration

Translations are automatically built during `cmake --build`:
- `add_level()` function in `data/levels/CMakeLists.txt` calls `generate_translations()`
- The build system compiles `.po` files to `.mo` format
- Translated files are installed to `${COLOBOT_INSTALL_DATA_DIR}`

## Common Issues and Solutions

### Issue: Translation Not Appearing in Game
**Cause**: .po file not properly formatted or missing required entries.

**Solution**:
1. Verify .po file has proper header
2. Check for duplicate msgid entries
3. Regenerate translations: `python3 i18n-tools/scripts/process_translations.py --mode generate`
4. Rebuild and reinstall: `cmake --build --target install`

### Issue: Missing Language Character
**Cause**: Language collision between Spanish ('S') and another language.

**Solution**:
1. Verify `app.cpp` has Spanish special case in `GetLanguageChar()`
2. Language files must use `Title.S` and `Resume.S` (not `Title.E`)
3. Help files must be named `help.S.txt` and `soluce.S.txt`

### Issue: es.mo File Not Updated
**Cause**: .mo file not regenerated after .po changes.

**Solution**:
```bash
cd data/levels/missions/chapter001/level001/po
msgfmt -o es.mo es.po
cp es.mo /tmp/colobot-install/share/locale/es/LC_MESSAGES/colobot.mo
```

## File Locations

| Type | Location |
|------|----------|
| Level templates | `data/levels/*/chapter*/level*/po/*.pot` |
| Level translations | `data/levels/*/chapter*/level*/po/es.po` |
| Help templates | `data/levels/*/chapter*/level001/help/*.txt` |
| Generated help files | `data/levels/*/chapter*/level001/help/help.S.txt` |
| Installed data | `/tmp/colobot-install/share/games/colobot/levels/` |
| Installed locale | `/tmp/colobot-install/share/locale/es/LC_MESSAGES/colobot.mo` |

## Testing Translations

### Run Game with Spanish Language
```bash
/tmp/colobot-install/games/colobot --lang es
```

### Verify Level Translations
Check `scene.txt` contains Spanish entries:
```bash
grep -E "Title\.S|Resume\.S" /tmp/colobot-install/share/games/colobot/levels/missions/chapter001/level001/scene.txt
```

Expected output:
```
Title.S text="Equipo"
Resume.S text="Prepárate para embarcarte en la aventura más emocionante de la humanidad."
```

### Verify Help Files
```bash
head -5 /tmp/colobot-install/share/games/colobot/levels/missions/chapter001/level001/help/help.S.txt
```

Expected output should be Spanish text.

## Best Practices

1. **Use proper header**: Always include metadata header block
2. **Match template structure**: Use same comment format as .pot files
3. **Avoid duplicates**: Each msgid should appear exactly once
4. **Use msgfmt to validate**: Run `msgfmt -c es.po` to check for errors
5. **Regenerate after changes**: Run translation script after modifying .po files
6. **Use consistent terminology**: "Equipo" for Equipment, "Misión" for Mission, etc.

## Language Code Reference

| Code | Language | File Extension | Scene Key |
|------|----------|----------------|-----------|
| en | English | .txt | Title.E |
| es | Spanish | .S.txt | Title.S |
| fr | French | .F.txt | Title.F |
| de | German | .D.txt | Title.D |
| cs | Czech | .C.txt | Title.C |
| pl | Polish | .P.txt | Title.P |
| pt | Portuguese | .B.txt | Title.B |
| ru | Russian | .R.txt | Title.R |

## Creating New es.po Files from Templates

When a level is missing an es.po file, create one from the template:

### Method 1: Using msginit
```bash
cd /home/rrodriguez/Documentos/GitHub/colobot/data/levels/battles/chapter001/level001/po
msginit --input=battle101.pot --locale=es_ES --output=es.po
```

### Method 2: Manual Creation from Template
```bash
cd /home/rrodriguez/Documentos/GitHub/colobot/data/levels/exercises/chapter001/level001/po
cp train101.pot es.po
# Add header block to beginning of file
# Add msgstr translations for each msgid
```

### Minimum Header for es.po
```po
# This file is part of the Colobot: Gold Edition source code
# Copyright (C) 2001-2018, Daniel Roux, EPSITEC SA & TerranovaTeam
# This file is distributed under the same license as the Colobot package.
msgid ""
msgstr ""
"Project-Id-Version: 0.1.11\n"
"POT-Creation-Date: DATE\n"
"PO-Revision-Date: 2026-02-11 12:00+01:00\n"
"Last-Translator: Your Name\n"
"Language-Team: Spanish\n"
"Language: Spanish\n"
"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=UTF-8\n"
"Content-Transfer-Encoding: 8bit\n"
"Plural-Forms: nplurals=2; plural=(n != 1);\n"
"X-Language: es_ES\n"
"X-Source-Language: en_US\n"
```

## Generating Missing es.po Files

Use this script to generate all missing level es.po files:

```bash
#!/bin/bash
# generate_missing_es_po.sh

BASE_DIR="/home/rrodriguez/Documentos/GitHub/colobot/data"

for category in missions exercises battles challenges free plus; do
    find "$BASE_DIR/levels/$category" -name "*.pot" -type f | while read pot; do
        po_dir=$(dirname "$pot")
        po_file="$po_dir/es.po"
        if [ ! -f "$po_file" ]; then
            echo "Creating: $po_file"
            cp "$pot" "$po_file"
            # Prepend proper header to $po_file
        fi
    done
done
```

## Verifying Translation Coverage

Run this command to audit all es.po files:

```bash
cd /home/rrodriguez/Documentos/GitHub/colobot/data

# Check for missing es.po files
echo "=== Missing es.po files ==="
find . -name "*.pot" -type f | while read pot; do
    po_dir=$(dirname "$pot")
    if [ ! -f "$po_dir/es.po" ]; then
        echo "MISSING: $po_dir/es.po (template: $(basename $pot))"
    fi
done

# Check for malformed headers
echo ""
echo "=== Files with incorrect headers ==="
find . -name "es.po" -type f | while read f; do
    if ! grep -q "X-Language: es_ES" "$f"; then
        echo "BAD HEADER: $f"
    fi
done

# Check for empty translations
echo ""
echo "=== Files with empty translations ==="
find . -name "es.po" -type f | while read f; do
    empty=$(grep -c 'msgstr ""' "$f" 2>/dev/null || echo 0)
    if [ "$empty" -gt 0 ]; then
        echo "$f: $empty empty translations"
    fi
done
```

## References

- Original template: `data/levels/missions/chapter001/level001/po/scene101.pot`
- Czech translation reference: `data/levels/missions/chapter001/level001/po/cs.po`
- Build system: `data/i18n-tools/I18NTools.cmake`
- Translation script: `data/i18n-tools/scripts/process_translations.py`
- CMake integration: `data/levels/CMakeLists.txt`

## Changelog

- 2026-02-11: Fixed chapter001/level001 es.po (Equipment, mission description)
- 2026-02-11: Fixed chapter001/level002 es.po header format
- 2026-02-11: Created help.S.txt and soluce.S.txt for chapter001/level001
- 2026-02-11: Documented audit findings (114 missing level es.po files)