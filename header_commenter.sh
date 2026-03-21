#!/bin/bash

HEADER='/*
 * Copyright 2026 Simone Pelascini and Aurélien Bollin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 */
'

find . -type f \( -name "*.c" -o -name "*.h" \) \
  -not -path "*/.git/*" \
  -not -path "*/build/*" \
  -not -path "*/.pio/*" \
  -not -path "*/.vscode/*" \
  -not -path "*/.cache/*" \
  -not -path "*/managed_components/*" \
  -not -path "*/circuit_pcb/*" \
  -not -path "*/lib/sensirion/*" \
| while IFS= read -r file; do
    if ! grep -q "Apache License, Version 2.0" "$file"; then
        tmp=$(mktemp)
        printf '%s\n\n' "$HEADER" > "$tmp"
        cat "$file" >> "$tmp"
        mv "$tmp" "$file"
        echo "✔ Added: $file"
    else
        echo "➜ Skipped: $file"
    fi
done