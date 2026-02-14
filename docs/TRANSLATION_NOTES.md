# Colobot: Gold Edition - Translation Session Notes

## Session: 2026-02-13

### Translation Rules Confirmed

1. **"robot" vs "bot"**:
   - Use "robot" if English source uses "robot"
   - Use "bot" only when it's clearly game lore referencing bot types
   - Example: "grabber robot" → "robot recogedor", "factory production of bots" → "producción de bots"

2. **Código CBot**:
   - DO NOT translate code blocks (type: Source code)
   - Keep keywords in English: `turn()`, `fire()`, `motor()`, `while`, `if`, `break`
   - Only translate explanatory text around code

3. **Energy cells**: "celdas de energía" (not "células")

### Empty Msgstr Detection

⚠️ **False Positives**: Counts of empty msgstr included:
- PO file header (legitimately empty)
- Multiline msgstr (first line empty, content follows)
- Source code blocks (stay untranslated)

### Actual Translation Work

Files with genuine untranslated entries (typically 1-20 per file):
- Help files (generic, object descriptions)
- Exercise/challenge explanations
- Mission help text

### Test Method

After translations:
```bash
cd /home/rrodriguez/Documentos/GitHub/colobot/build
make -j4
cmake --install . --prefix /tmp/colobot-install

LANGUAGE=es /tmp/colobot-install/games/colobot
```

### Common Patterns Fix Commands

```bash
# Fix "Equip" conjugation
sed -i 's/^msgstr "Equip /msgstr "Equipa /g' file.po

# Find files needing work
find ... -name "es.po" -exec sh -c 'grep -c "^msgstr \"\"$" "$1" | grep -v "^0$" && echo "$1"' _ {} \;

# Validate PO syntax
msgfmt -c file.po
```

### Build Process Reminder

1. ALWAYS cd to build directory first:
   ```bash
   cd /home/rrodriguez/Documentos/GitHub/colobot/build
   ```

2. Touch source files to force rebuild:
   ```bash
   touch data/level/po/es.po
   make -j4
   ```

3. Install changes:
   ```bash
   cmake --install . --prefix /tmp/colobot-install
   ```

4. Test in game:
   ```bash
   LANGUAGE=es /tmp/colobot-install/games/colobot
   ```

### CBOT Help Hyperlinks Issue (NEW)

**Problem**: Spanish CBOT help hyperlinks don't work

**Cause**: No `/data/help/cbot/po/es.po` file exists → Individual CBot instruction files (aim.txt, turn.txt) not generated for Spanish

**Root Cause**: Missing es.po for cbot category in data/help/cbot/po/

**Detection**: 
```
# No Spanish cbot directory
ls /tmp/colobot-install/share/games/colobot/help/S/cbot/
# → No such directory
```

**Fix**: Create `/data/help/cbot/po/es.po` with translations

- File: data/help/cbot/po/cbot.pot (1144 entries)
- Translate: ~885 plain text entries (skip 259 Source code blocks)
- Preserve: `\l;text\u type;` hyperlink format
- Format: Individual entries per CBOT instruction

**Status**: Under way (~20% complete, 870/1144 remaining)

**Test**:
```bash
cd /home/rrodriguez/Documentos/GitHub/colobot/build
make -j4
cmake --install . --prefix /tmp/colobot-install
LANGUAGE=es /tmp/colobot-install/games/colobot
```

### Files Status

#### Complete:
- po/es.po (Main UI)
- data/help/bots/po/es.po
- data/help/generic/p/es.po  
- data/help/object/po/es.po
- All battle/challenge/exercise/mission levels (except cbot)

#### In Progress:
- data/help/cbot/po/es.po (creating & translating, ~20% complete)

#### Terminology Reference

#### Terminology Reference
| English | Spanish | Context |
|---------|---------|---------|
| power cell | celda de energía | NOT célula |
| power cells | celdas de energía | plural |
| bot (type) | bot | game unit |
| robot (source uses robot) | robot | literal |
| titanium | titanio | material |
| titanium cube | cubo de titanio | item |
| uranium ore | mineral de uranio | material |
| black box | caja negra | item |
| radar | radar | keep English |
| fire(1) | fire(1) | code keyword |
| while(true) | while(true) | code keyword |