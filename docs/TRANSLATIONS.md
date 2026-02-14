# Colobot: Gold Edition - Translation Guide

This document provides comprehensive information for translators and AI agents working on localization.

## Overview

Colobot: Gold Edition uses a multi-stage translation pipeline:
1. **POT files** (`*.pot`) - Translation template files with empty msgstr entries
2. **PO files** (`*.po`) - Human-readable translation files with msgstr translated
3. **MO files** (`*.mo`) - Compiled binary translation files
4. **TXT files** (`*.txt`) - Final game-ready translations

### Translation Pipeline

```
POT → PO → msgfmt → MO → Python scripts → TXT
```

**Detailed Process:**
1. **POT files** are generated from source text files (templates)
2. **PO files** are created from POT files with translations filled in
3. **MO files** are compiled from PO files using `msgfmt`
4. **TXT files** are generated from PO files using Python scripts

The Python scripts in `data/i18n-tools/scripts/` process translations during build.

## POT Files (Translation Templates)

### What are POT files?

POT (Portable Object Template) files are bilingual source files containing:
- Original text to translate (msgid)
- Empty translation space (msgstr "")
- File references and comments
- Metadata (headers)

### POT File Format

```pot
#. type: Plain text
#: ../E/ant.txt:1
#, no-wrap
msgid "Ant"
msgstr ""
```

### POT File Locations

```
data/help/bots/po/bots.pot           # Bots help template
data/help/generic/po/generic.pot       # Generic help template
data/help/object/po/object.pot         # Objects help template
data/help/cbot/po/cbot.pot             # CBot language template
data/help/programs/po/programs.pot     # Programs help template
data/levels/missions/chapter001/level001/po/mission101.pot
data/levels/exercises/chapter001/level001/po/train101.pot
# ... etc
```

### Creating PO Files from POT

#### Method 1: Using msginit (Standard Gettext)

```bash
# Create Spanish PO from template
msginit --locale=es --input=object.pot --output=es.po

# With proper input encoding
msginit --no-translator --locale=es --input=object.pot --output-es.po
```

#### Method 2: Using generate_es_po.py (Colobot Helper Script)

Colobot provides a helper script `data/i18n-tools/scripts/generate_es_po.py`:

```bash
cd /home/rrodriguez/Documentos/GitHub/colobot
python3 data/i18n-tools/scripts/generate_es_po.py object.pot es.po
```

This script automatically:
- Adds proper Spanish header
- Translates common strings (objective → Objetivo, procedure → Procedimiento, etc.)
- Handles basic patterns (power cell → celdas de energía)

#### Method 3: Manual Copy and Edit

```bash
# Copy POT to create PO
cp object.pot es.po

# Edit manually:
# 1. Update header with Spanish info
# 2. Translate each msgid → msgstr
# 3. Keep original comments and file references
```

### Updating PO Files from Updated POT

When source files change and POT files are updated:

```bash
# Merge new translations into existing PO
msgmerge --update es.po object.pot
```

Or use the project's Python scripts which handle this automatically.

### Common POT File Properties

- **Encoding**: UTF-8
- **Line endings**: Unix (LF)
- **Content-Type**: text/plain; charset=UTF-8
- **No-Translate Entries**: Header entry starting with `msgid ""`

### POT vs PO

| File | Purpose | msgstr | Uses |
|------|---------|-------|------|
| POT | Template | Always empty | Source for new translations |
| PO | Translation | Contains translations | Active work file |

## PO Files

### PO File Structure

```po
# Header
msgid ""
msgstr ""
"Project-Id-Version: 0.1.11\n"
"POT-Creation-Date: DATE\n"
"PO-Revision-Date: 2026-02-13 12:00+01:00\n"
"Last-Translator: Spanish Translation Team\n"
"Language-Team: Spanish\n"
"Language: Spanish\n"
"MIME-Version: 1.0\n"
"Content-Type: text/plain; charset=UTF-8\n"
"Content-Transfer-Encoding: 8bit\n"
"Plural-Forms: nplurals=2; plural=(n != 1);\n"
"X-Language: es_ES\n"
"X-Source-Language: en_US\n"

# Translation entries
#. type: \b; header
#: ../help/help.E.txt:1
#, no-wrap
msgid "Objective"
msgstr "Objetivo"
```

### Validating PO Files

```bash
# Check PO file syntax
msgfmt -c file.po

# Check for empty msgstr entries
grep -c '^msgstr ""$' file.po

# Check for duplicate entries (fix with msguniq)
msgfmt -c file.po 2>&1 | grep "duplicada"
msguniq file.po > file.po.fixed
```

## Language Codes

### Standard Language Codes
- `cs` → Czech
- `de` → German
- `en` → English
- `es` → Spanish
- `fr` → French
- `pl` → Polish
- `pt` → Portuguese
- `ru` → Russian

### ⚠️ CRITICAL: Language Code Collision

Spanish uses `Title.S` instead of `Title.E` because English (`en`) already uses `E`:
- English: `help.E.txt`, `scene.E.txt`
- Spanish: `help.S.txt`, `scene.S.txt`

This is defined in `data/i18n-tools/scripts/common.py`:
```python
def language_char(self):
    if self.language == 'pt':
        return 'B';
    elif self.language == 'es':
        return 'S';
    else:
        return self.language[0].upper()
```

## File Locations

### Main UI Translations
```
/home/rrodriguez/Documentos/GitHub/colobot/po/es.po           # Main UI (659 strings)
```

### Desktop File Translations
```
/home/rrodriguez/Documentos/GitHub/colobot/desktop/po/es.po
```

### Help File Translations
```
/home/rrodriguez/Documentos/GitHub/colobot/data/help/bots/po/es.po      # Bot help
/home/rrodriguez/Documentos/GitHub/colobot/data/help/generic/po/es.po  # Generic help
/home/rrodriguez/Documentos/GitHub/colobot/data/help/object/po/es.po   # Object help
/home/rrodriguez/Documentos/GitHub/colobot/data/help/cbot/po/es.po     # CBot language help
/home/rrodriguez/Documentos/GitHub/colobot/data/help/programs/po/es.po # Programs help
```

### Level Translations
```
# Missions (36 levels across 9 chapters)
data/levels/missions/chapter00X/level00Y/po/es.po

# Battles (6 levels)
data/levels/battles/chapter00X/level00Y/po/es.po

# Challenges (14 levels)
data/levels/challenges/chapter00X/level00Y/po/es.po

# Exercises (39 levels across 7 chapters)
data/levels/exercises/chapter00X/level00Y/po/es.po

# Free (9 levels)
data/levels/free/chapter00X/level00Y/po/es.po

# Plus (29 levels across 9 chapters)
data/levels/plus/chapter00X/level00Y/po/es.po
```

### Help Source Files (for exercises)
These are the source text files that get translated:
```
data/levels/exercises/chapter00X/level00Y/help/help.S.txt
data/levels/exercises/chapter00X/level00Y/help/help.E.txt
```

## Terminology Guidelines

### ✅ Use "bot"
Colobot uses "bot" consistently (part of the game lore - COLO-BOT):
- ✅ "bot" → "bot" (keep English)
- ❌ "robot" → AVOID, use "bot" instead

### Energy Cell Translation
- ✅ "celdas de energía" (Spanish)
- ❌ "células de energía" (incorrect)
- ❌ "celulas de energia" (missing accent)

### Common Terms
| English | Spanish | Notes |
|---------|---------|-------|
| power cell | celda de energía | Energy storage |
| titanium | titanio | Building material |
| uranium ore | mineral de uranio | Nuclear fuel source |
| radar | radar | Keep English - it's a function |
| motor | motor | Keep English - CBot keyword |
| fire | disparar | Verb form |
| turn | girar | Verb form |
| while | mientras | CBot keyword |
| if | si | CBot keyword |
| else | si_no | CBot keyword |
| break | romper | CBot keyword |
| null | null | Keep English - CBot keyword |

## PO File Format

### Standard PO Entry
```po
#. type: Plain text
#: ../help/help.E.txt:2
#, no-wrap
msgid "Objective"
msgstr "Objetivo"
```

### Multi-line Entry
```po
#. type: Source code
#: ../help/help.E.txt:16
#, no-wrap
msgid ""
"extern void object::Spider1( )\n"
"{\n"
"\t\n"
"\t<n/>write your program here ...<c/>\n"
"\t\n"
"}"
msgstr ""
"extern void object::Spider1( )\n"
"{\n"
"\t\n"
"\t<n/>escribe tu programa aquí ...<c/>\n"
"\t\n"
"}"
```

### Code Keywords Preservation
**CRITICAL**: Code keywords must NOT be translated:
- Keep `turn(90);` as is
- Keep `fire(1);` as is
- Keep `motor(1, 1);` as is
- Keep `radar(AlienSpider);` as is
- Keep `while (true)` as is

Only translate the surrounding text:
```
msgid "turn 90 degrees left with <code>turn(90)</code>"
msgstr "girar 90 grados a la izquierda con <code>turn(90)</code>"
```

### Help File Tags
These special tags must be preserved:

| Tag | Meaning | Keep/M translate |
|-----|---------|------------------|
| `<c/>` | Code | Keep |
| `<n/>` | Newline | Keep |
| `<button 22/>` | UI button | Keep number |
| `<format key><key help/></format>` | Help key | Keep |
| `<code>...</code>` | Inline code | Keep |
| `<a cbot|keyword>text</a>` | CBot link | Keep keyword |

### PO File Header
```po
msgid ""
msgstr ""
"Project-Id-Version: 0.1.11\n"
"POT-Creation-Date: DATE\n"
"PO-Revision-Date: 2026-02-13 12:00+01:00\n"
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

## Build and Install Process

### Full Build
```bash
cd /home/rrodriguez/Documentos/GitHub/colobot/build
cmake --preset Linux-CI-gcc
make -j4
cmake --install . --prefix /tmp/colobot-install
```

### Rebuild Translations Only
When PO files are modified, rebuild to regenerate MO and TXT files:
```bash
cd /home/rrodriguez/Documentos/GitHub/colobot/build
make -j4
cmake --install . --prefix /tmp/colobot-install
```

### Force Rebuild of Specific Files
Touch source files to force regeneration:
```bash
touch /path/to/source/help.S.txt
make -j4
cmake --install . --prefix /tmp/colobot-install
```

## Validation

### Check PO File Syntax
```bash
msgfmt -c /path/to/es.po
```

### Verify No Empty Translations
```bash
grep -c '^msgstr ""$' /path/to/es.po
# Should return 0 or very low (header entries are normal)
```

### Check for Mixed Language
```bash
grep -rn "tell the bot exactly\|write your program here\|прямо\|здесь\|и т\.п" /path/to/*.po
```

## Testing Translations

### Run with Language Set
```bash
LANGUAGE=es /tmp/colobot-install/games/colobot
```

### Install Prefix Structure
After install, translations are located at:
```
/tmp/colobot-install/share/locale/es/LC_MESSAGES/colobot.mo    # Main UI
/tmp/colobot-install/share/games/colobot/help/S/object/       # Object help
/tmp/colobot-install/share/games/colobot/levels/exercises/chapter001/level001/help/help.S.txt  # Exercise help
```

## Common Issues and Fixes

### Issue: Mixed Language Text in Output
**Cause**: Corrupted msgid in PO file with mixed Spanish/Russian/English
**Fix**: Update the msgid to match correct Spanish, not mixed text

### Issue: Code Keywords Translated
**Cause**: Translator translated `while`, `if`, `motor`, etc.
**Fix**: Keep CBot keywords in English, only translate surrounding text

### Issue: Energy Cells → "células" Instead of "celdas"
**Cause**: Incorrect terminology choice
**Fix**: Use `sed` to batch replace:
```bash
find . -name "es.po" -exec sed -i 's/células de energía/celdas de energía/g; s/célula de energía/celda de energía/g' {} \;
```

### Issue: "robots" Instead of "bots"
**Cause**: Translator used "robot" instead of game lore "bot"
**Fix**: Replace in context where `<a object|factory>` is used:
```bash
grep -n '<a object|factory>robots' /path/to/es.po
# Manually verify context and fix
```

### Issue: Source Help File Contains Corrupted Text
**Cause**: The source help.S.txt file has mixed language entries
**Fix**: Edit the source file directly:
```bash
# Before
Debes tell the bot exactly step by step what it must do...
# After
Debes decirle al bot exactamente paso a paso lo que debe hacer...
```

Then fix the corresponding msgid in the PO file to match:
```po
msgid "Debes decirle al bot exactamente paso a paso lo que debe hacer..."
msgstr "Debes decirle al bot exactamente paso a paso lo que debe hacer..."
```

### Issue: Translation Not Appearing in Game
**Cause**: Either:
1. Build not regenerated
2. PO file syntax error
3. msgid mismatch with source

**Fix**:
1. Rebuild: `make -j4`
2. Validate: `msgfmt -c es.po`
3. Check generated TXT in build directory:
```bash
cat /home/rrodriguez/Documentos/GitHub/colobot/build/data/levels/po/exercises/chapter001/level001/help/help.S.txt
```

## Translation Workflow for AI Agents

### Adding New Language (Example: French)

1. Create PO files from templates:
```bash
# Main UI
cp po/colobot.pot po/fr.po
# Edit header for French

# Desktop
cp desktop/po/desktop.pot desktop/po/fr.po

# Help files
cp data/help/bots/po/bots.pot data/help/bots/po/fr.po
# Repeat for generic, object, cbot, programs
```

2. Translate entries in each PO file
3. Build and validate:
```bash
msgfmt -c po/fr.po
cd build && make -j4 && cmake --install . --prefix /tmp/colobot-install
```

4. Test:
```bash
LANGUAGE=fr /tmp/colobot-install/games/colobot
```

### Updating Existing Translations

1. Identify missing/incorrect translations:
```bash
grep -c '^msgstr ""$' data/levels/exercises/*/po/es.po
```

2. Fix translations in PO file
3. Rebuild and test

## Python Translation Scripts

Located in `data/i18n-tools/scripts/`:

| Script | Purpose |
|--------|---------|
| `process_translations.py` | Main orchestration script |
| `translate_help.py` | Converts help file translations |
| `translate_level.py` | Converts level file translations |
| `translate_chaptertitles.py` | Converts chapter title translations |
| `common.py` | Shared classes (TranslationJob, TemplateFile, LanguageFile) |

The translation process:
1. Reads source files (help.E.txt, help.S.txt)
2. Looks up translations in PO catalog
3. Falls back to original text if translation not found
4. Writes converted output (help.S.txt in game format)

## Quick Reference

### Validate PO file
```bash
msgfmt -c /path/to/es.po
```

### Find empty translations
```bash
grep -c '^msgstr ""$' /path/to/es.po
```

### Find mixed language issues
```bash
grep -rn "tell the bot exactly\|write your program here" /path/to/*.po
```

### Force reinstall translations
```bash
rm -rf /tmp/colobot-install/share/games/colobot/levels/exercises
cmake --install . --prefix /tmp/colobot-install
```

### Test in game
```bash
LANGUAGE=es /tmp/colobot-install/games/colobot
```