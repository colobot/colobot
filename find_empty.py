#!/bin/env python3
import sys

def find_empty_entries_with_context(file_path, max_results=20):
    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    empty_entries = []
    i = 0
    while i < len(lines) and len(empty_entries) < max_results:
        line = lines[i].strip()
        
        # Skip header msgid ""
        if i == 0 and line == 'msgid ""':
            while i < len(lines) and lines[i].strip() != '':
                i += 1
            i += 1
            continue
        
        # Look for msgid with content
        if line.startswith('msgid '):
            msgid_start = i
            msgid_has_content = len(line) > 7 and line[7:-1] != ''
            
            # Collect full msgid
            msgid_parts = []
            j = i
            while j < len(lines) and (lines[j].strip().startswith('msgid ') or lines[j].strip().startswith('"')):
                line_j = lines[j].strip()
                if line_j.startswith('msgid '):
                    if line_j == 'msgid ""':
                        msgid_has_content = False
                        break
                    msgid_parts.append(line_j[7:-1] if line_j.endswith('"') else line_j[7:])
                elif line_j.startswith('"'):
                    msgid_parts.append(line_j[1:-1] if line_j.endswith('"') else line_j[1:])
                    msgid_has_content = True
                j += 1
            
            # Now look for msgstr after this
            msgstr_line = -1
            while j < len(lines):
                line_j = lines[j].strip()
                if line_j.startswith('msgstr'):
                    msgstr_line = j
                    break
                if line_j.startswith('msgid '):
                    break
                j += 1
            
            # Check if msgstr is empty
            if msgstr_line > 0:
                msgstr = lines[msgstr_line].strip()
                if msgstr == 'msgstr ""':
                    empty_entries.append((msgid_start + 1, msgstr_line + 1, ''.join(msgid_parts)))
                    i = msgstr_line + (1 if len(empty_entries) >= max_results else 0)
                    continue
        i += 1
    
    return empty_entries

if __name__ == '__main__':
    if len(sys.argv) > 1:
        file_path = sys.argv[1]
        empty_entries = find_empty_entries_with_context(file_path)
        print(f"File: {file_path}")
        print(f"Found {len(empty_entries)} empty msgstr entries:")
        for idx, (msgid_line, msgstr_line, msgid) in enumerate(empty_entries, 1):
            print(f"\n{idx}. Line {msgid_line}: msgid")
            print(f"   Line {msgstr_line}: msgstr \"\"")
            print(f"   Content: {msgid[:100]}...")
    else:
        print("Usage: find_empty.py <po_file>")