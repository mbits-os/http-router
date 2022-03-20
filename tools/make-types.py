import json
import sys

types = []
with open("types.json", encoding="UTF-8") as f:
    data = json.load(f)
    for T in data:
        for ext in data[T]:
            if len(ext) > 4: continue
            types.append((ext, T))

for ext, T in sorted(types):
    print("P{{ext_u32(\"{}{}\"), {}\"{}\"sv}},".format(ext, "\\0\\0" if len(ext) == 1 else "\\0" if len(ext) == 2 else "", " " if len(ext) == 3 else "", T))