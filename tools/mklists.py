#!/usr/bin/env python3
"""mklists.py GLC ROMS DISC [--main-title T] [--link-title T]

Lays out a Gens4All disc from a folder of Mega Drive ROMs, with one game list per folder:

  ROMS/*.md|bin|gen|smd          -> DISC/*.bin          + DISC/gamelist.bin   (the list the emulator opens)
  ROMS/SUB/*.md|bin|gen|smd      -> DISC/SUB/*.bin      + DISC/SUB/SUB.bin    (one per sub-folder, one level)

Each list starts with links (Gens4All "Switcher" entries) to the other lists. Titles come from the file names, not
from the ROM headers (glc's are often empty or odd): without a leading "NNN " ranking number, [tags] or the usual
(region / revision) groups, and "Name, The" -> "The Name". Sorted by title with a "Letter:" index. A ROM whose bytes
are already on the disc is left out (first the top folder, then each sub-folder; inside a folder the names sorted,
so "030 Aladdin" is kept and a plain "Aladdin" copy is dropped). Save names, sizes and types are glc's own (-g).
GLC is gens4all/src/glparse/glc, built for the host.
"""
import argparse, hashlib, os, re, shutil, subprocess, sys, tempfile

EXTS = (".md", ".bin", ".gen", ".smd")
DROP = (r"USA|Europe|Japan|World|Korea|J|U|E|UE|JUE|En|En,Ja|Rev [A-Z0-9]+|Alt|J-Cart|Unl|"
        r"Japan, USA|USA, Europe|Japan, Europe|Japan, Korea|USA, Korea|[A-Z][a-z]+, \d{4}|[^)]*Hack[^)]*")


def title(fname):
    t = re.sub(r"^\d{3} ", "", os.path.splitext(fname)[0])
    t = re.sub(r"\s*\[[^\]]*\]", "", t)
    t = re.sub(r"\s*\((%s)\)" % DROP, "", t)
    t = re.sub(r"^(.*), The\b", r"The \1", t).strip()
    return t.encode("ascii", "replace").decode() or fname


def md5(path):
    with open(path, "rb") as f:
        return hashlib.md5(f.read()).hexdigest()


def write_list(glc, folder, cd_dir, links, out_name):
    roms = sorted(f for f in os.listdir(folder) if f.endswith(".bin") and f != out_name)
    with tempfile.TemporaryDirectory() as tmp:
        tpl = os.path.join(tmp, "t.txt")
        subprocess.run([glc, "-g", tpl] + roms, cwd=folder, check=True, stdout=subprocess.DEVNULL)
        text = open(tpl, encoding="latin-1").read()
    ents, names = [], {}
    for b in re.split(r"\n(?=#File )", text):
        if not b.startswith("#File "):
            continue
        b = "\n".join(l for l in b.splitlines() if not l.startswith("Letter:"))
        rom = re.search(r"^Rom: (.*)$", b, re.M).group(1)
        t = title(rom)
        names[t] = names.get(t, 0) + 1
        ents.append([t, rom, b])
    for e in ents:                       # same title twice (two dumps): add what tells them apart
        if names[e[0]] > 1:
            extra = re.findall(r"\(([^)]*)\)", e[1])
            e[0] += " (%s)" % ", ".join(extra) if extra else ""
    out = ["Dir: " + cd_dir, "", "Letter: !"]
    for lt, lrom in links:
        out += ["Game: " + lt, "Rom: " + lrom, "Switcher", ""]
    out += ["Game:", ""]
    last = None
    for t, rom, b in sorted(ents, key=lambda e: e[0].lower().removeprefix("the ")):
        k = t.lower().removeprefix("the ")
        letter = k[0].upper() if k[:1].isalpha() else "#"
        if letter != last:
            out.append("Letter: " + letter)
            last = letter
        b = re.sub(r"^Game: .*$", "Game: " + t, b, flags=re.M)
        b = re.sub(r"^Savetitle: .*$", "Savetitle: " + t[:32], b, flags=re.M)
        b = re.sub(r"^Savevmu: .*$", "Savevmu: " + t[:16], b, flags=re.M)
        out.append(b.rstrip() + "\n")
    src = os.path.join(folder, "list.txt")
    with open(src, "w", encoding="latin-1") as f:
        f.write("\n".join(out) + "\n")
    r = subprocess.run([glc, "-c", "list.txt"], cwd=folder, capture_output=True, text=True)
    if r.returncode or not os.path.exists(os.path.join(folder, "gamelist.bin")):
        sys.exit("glc -c failed in %s:\n%s" % (folder, r.stdout[-2000:]))
    if out_name != "gamelist.bin":
        os.replace(os.path.join(folder, "gamelist.bin"), os.path.join(folder, out_name))
    os.replace(src, os.path.join(folder, os.path.splitext(out_name)[0] + ".txt"))  # the source, kept for reference
    print("%s: %d games" % (os.path.join(cd_dir, out_name), len(ents)))


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("glc"); ap.add_argument("roms"); ap.add_argument("disc")
    ap.add_argument("--main-title", default="[[ Main list ]]")
    ap.add_argument("--link-title", default="[[ %s ]]", help="title of a link to a sub-folder list; %%s = its name")
    a = ap.parse_args()
    glc = os.path.abspath(a.glc)
    os.makedirs(a.disc, exist_ok=True)
    subs = sorted(d for d in os.listdir(a.roms) if os.path.isdir(os.path.join(a.roms, d)))
    seen = {}
    for sub in [""] + subs:
        src = os.path.join(a.roms, sub)
        dst = os.path.join(a.disc, sub)
        os.makedirs(dst, exist_ok=True)
        for f in sorted(os.listdir(src)):
            p = os.path.join(src, f)
            if not (os.path.isfile(p) and f.lower().endswith(EXTS)):
                continue
            h = md5(p)
            if h in seen:
                print("left out (same bytes as %s): %s" % (seen[h], os.path.join(sub, f)))
                continue
            seen[h] = os.path.join(sub, f)
            shutil.copy(p, os.path.join(dst, os.path.splitext(f)[0] + ".bin"))
    lists = [("", "/cd/", "gamelist.bin", a.main_title)] + \
            [(s, "/cd/%s/" % s, s + ".bin", a.link_title % s) for s in subs]
    for folder, cd_dir, name, _ in lists:
        links = [(lt, lcd + lname) for _, lcd, lname, lt in lists if lcd != cd_dir]
        write_list(glc, os.path.join(a.disc, folder), cd_dir, links, name)


if __name__ == "__main__":
    main()
