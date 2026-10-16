#!/usr/bin/env python3
"""Deterministic tiny companion UI symbols; no borrowed xdBot artwork."""
from pathlib import Path
from PIL import Image, ImageDraw

OUT = Path(__file__).resolve().parent.parent / "resources" / "icons"
OUT.mkdir(parents=True, exist_ok=True)
S = 4
GOLD = (255, 201, 77, 255)
LIGHT = (255, 239, 185, 255)
BROWN = (73, 41, 27, 230)
BLUE = (105, 196, 236, 255)

def icon(name, draw):
    im = Image.new("RGBA", (32*S, 32*S), (0, 0, 0, 0))
    d = ImageDraw.Draw(im)
    def line(points, fill=GOLD, width=2.6):
        d.line([(int(x*S), int(y*S)) for x, y in points], fill=fill, width=int(width*S), joint="curve")
    def ellipse(box, fill=None, outline=GOLD, width=2.4):
        d.ellipse(tuple(int(v*S) for v in box), fill=fill, outline=outline, width=int(width*S))
    def rect(box, fill=None, outline=GOLD, width=2.4):
        d.rounded_rectangle(tuple(int(v*S) for v in box), radius=2*S, fill=fill, outline=outline, width=int(width*S))
    ellipse((2, 2, 30, 30), fill=BROWN, outline=(174, 120, 65, 220), width=1.2)
    draw(line, ellipse, rect)
    im.resize((32, 32), Image.Resampling.LANCZOS).save(OUT / (name + ".png"))

icon("hype", lambda l,e,r:(e((7,7,25,25),outline=GOLD),l([(7,16),(25,16)],LIGHT),l([(16,7),(13,11),(12,16),(13,21),(16,25)],LIGHT),l([(16,7),(19,11),(20,16),(19,21),(16,25)],LIGHT)))
icon("gdb", lambda l,e,r:(e((7,7,20,20),outline=BLUE),l([(20,20),(26,26)],BLUE,3)))
icon("prev", lambda l,e,r:(l([(20,8),(11,16),(20,24)],GOLD,3),l([(11,16),(25,16)],LIGHT,2)))
icon("next", lambda l,e,r:(l([(12,8),(21,16),(12,24)],GOLD,3),l([(7,16),(21,16)],LIGHT,2)))
icon("all", lambda l,e,r:(r((6,7,26,25),outline=GOLD),l([(9,15),(13,19),(20,11)],LIGHT,3)))
icon("clear", lambda l,e,r:(l([(10,10),(22,22)],GOLD,3),l([(22,10),(10,22)],GOLD,3)))
icon("export", lambda l,e,r:(l([(16,6),(16,21)],LIGHT,3),l([(10,15),(16,21),(22,15)],LIGHT,3),l([(7,22),(7,26),(25,26),(25,22)],GOLD,2.5)))
icon("share", lambda l,e,r:(l([(11,16),(22,9)],GOLD,2.5),l([(11,16),(22,23)],GOLD,2.5),e((6,12,13,19),fill=LIGHT,outline=GOLD,width=1),e((20,5,27,12),fill=LIGHT,outline=GOLD,width=1),e((20,20,27,27),fill=LIGHT,outline=GOLD,width=1)))
icon("gdr", lambda l,e,r:(r((8,5,24,27),outline=GOLD),l([(12,12),(20,12)],LIGHT,2),l([(12,17),(20,17)],LIGHT,2),l([(12,22),(17,22)],LIGHT,2)))
icon("json", lambda l,e,r:(l([(13,7),(10,7),(10,14),(7,16),(10,18),(10,25),(13,25)],BLUE,2),l([(19,7),(22,7),(22,14),(25,16),(22,18),(22,25),(19,25)],BLUE,2)))
icon("cml", lambda l,e,r:(r((7,7,25,25),outline=GOLD),l([(11,12),(20,12)],LIGHT,2),l([(11,17),(18,17)],LIGHT,2),l([(11,22),(21,22)],LIGHT,2)))
icon("gdr2", lambda l,e,r:(r((7,7,20,24),outline=GOLD),r((12,10,25,27),outline=LIGHT)))
icon("selected", lambda l,e,r:(e((6,6,26,26),fill=(90,98,57,255),outline=GOLD),l([(10,16),(14,20),(22,11)],LIGHT,3)))
icon("unselected", lambda l,e,r:e((7,7,25,25),fill=(35,32,31,200),outline=GOLD))
print(f"Created {len(list(OUT.glob('*.png')))} icons in {OUT}")
