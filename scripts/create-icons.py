#!/usr/bin/env python3
"""Bold, unframed 64px game-style PNG icons for xdBot Companion.

Not tiny illustrations/thumbnail images: each icon is a simple filled pictogram
with transparent surroundings and a dark game-UI outline. Original artwork.
"""
from pathlib import Path
from PIL import Image, ImageDraw

OUT=Path(__file__).resolve().parent.parent/'resources/icons'
OUT.mkdir(parents=True,exist_ok=True)
S=4
INK='#252037'; GOLD='#ffcb53'; CREAM='#fff5d1'; BLUE='#53beff'; DEEP='#27569e'
GREEN='#59d97b'; RED='#f56367'; ORANGE='#ff983a'; TEAL='#5be2db'

def draw_icon(name, paint):
    canvas=Image.new('RGBA',(64*S,64*S),(0,0,0,0));d=ImageDraw.Draw(canvas)
    def box(coords, fill, outline=INK, width=3, radius=5):
        d.rounded_rectangle(tuple(int(x*S) for x in coords),radius=int(radius*S),fill=fill,outline=outline,width=int(width*S))
    def circle(coords, fill, outline=INK, width=3):
        d.ellipse(tuple(int(x*S) for x in coords),fill=fill,outline=outline,width=int(width*S))
    def polygon(coords, fill, outline=INK, width=3):
        pts=[(int(x*S),int(y*S)) for x,y in coords];d.polygon(pts,fill=fill)
        if outline:d.line(pts+[pts[0]],fill=outline,width=int(width*S),joint='curve')
    def line(coords, fill=INK, width=3):
        d.line([(int(x*S),int(y*S)) for x,y in coords],fill=fill,width=int(width*S),joint='curve')
    paint(box,circle,polygon,line)
    canvas.resize((64,64),Image.Resampling.LANCZOS).save(OUT/(name+'.png'),optimize=True)

def hype(b,c,p,l):
    c((8,8,56,56),BLUE,width=4);c((14,14,50,50),TEAL,outline=GOLD,width=3)
    l([(8,32),(56,32)],INK,3);l([(31,10),(23,20),(22,32),(23,45),(31,55)],DEEP,3)
    l([(33,10),(41,20),(42,32),(41,45),(33,55)],DEEP,3)
    l([(12,19),(20,15),(32,14),(46,19)],CREAM,3)
def gdb(b,c,p,l):
    c((6,5,44,43),BLUE,width=5);c((11,10,39,38),TEAL,outline=CREAM,width=3)
    l([(40,40),(58,58)],INK,12);l([(40,40),(58,58)],GOLD,7)
    c((16,13,21,18),CREAM,outline=CREAM,width=1)
def prev(b,c,p,l):
    p([(5,32),(30,7),(30,20),(58,20),(58,44),(30,44),(30,57)],BLUE,width=4)
    l([(9,32),(28,13)],CREAM,4);l([(31,24),(51,24)],CREAM,3)
def next_(b,c,p,l):
    p([(59,32),(34,7),(34,20),(6,20),(6,44),(34,44),(34,57)],BLUE,width=4)
    l([(55,32),(36,13)],CREAM,4);l([(11,24),(31,24)],CREAM,3)
def all_(b,c,p,l):
    b((6,8,58,56),GOLD,radius=9,width=4)
    l([(15,32),(26,44),(49,19)],INK,13);l([(15,32),(26,44),(49,19)],GREEN,8)
def clear(b,c,p,l):
    p([(18,6),(32,20),(46,6),(58,18),(44,32),(58,46),(46,58),(32,44),(18,58),(6,46),(20,32),(6,18)],RED,width=4)
    l([(13,17),(47,51)],CREAM,2)
def export(b,c,p,l):
    b((5,37,59,58),DEEP,radius=4,width=4)
    p([(23,5),(41,5),(41,30),(53,30),(32,50),(11,30),(23,30)],GOLD,width=4)
    l([(27,10),(27,29)],CREAM,4)
def share(b,c,p,l):
    c((6,25,23,42),GOLD,width=3);c((42,5,59,22),GOLD,width=3)
    c((42,43,59,60),GOLD,width=3)
    l([(21,32),(45,14)],INK,11);l([(21,32),(45,51)],INK,11)
    l([(21,32),(45,14)],CREAM,6);l([(21,32),(45,51)],CREAM,6)
def gdr(b,c,p,l):
    p([(10,5),(42,5),(56,19),(56,59),(10,59)],CREAM,width=4)
    p([(42,5),(56,19),(42,19)],GOLD,width=2)
    p([(23,23),(23,49),(46,36)],BLUE,width=4)
def json_(b,c,p,l):
    b((7,6,57,58),DEEP,radius=7,width=4)
    l([(28,14),(20,14),(20,25),(14,31),(20,38),(20,50),(28,50)],CREAM,5)
    l([(36,14),(44,14),(44,25),(50,31),(44,38),(44,50),(36,50)],CREAM,5)
    c((29,28,35,34),GOLD,outline=GOLD,width=1)
def cml(b,c,p,l):
    p([(10,6),(44,6),(54,16),(54,57),(10,57)],GOLD,width=4)
    l([(18,20),(43,20)],INK,4);l([(18,28),(40,28)],INK,4)
    l([(18,36),(30,36)],INK,4)
    p([(26,49),(48,27),(56,35),(34,57)],ORANGE,width=3)
def gdr2(b,c,p,l):
    b((7,5,42,49),BLUE,radius=5,width=4)
    b((19,16,57,59),CREAM,radius=5,width=4)
    p([(29,27),(29,49),(48,38)],DEEP,width=3)
def selected(b,c,p,l):
    c((5,5,59,59),GREEN,width=4)
    l([(15,32),(27,44),(49,18)],INK,14)
    l([(15,32),(27,44),(49,18)],CREAM,8)
def unselected(b,c,p,l):
    c((5,5,59,59),BLUE,width=5)
    c((14,14,50,50),DEEP,outline=CREAM,width=3)

for name,paint in {'hype':hype,'gdb':gdb,'prev':prev,'next':next_,'all':all_,
    'clear':clear,'export':export,'share':share,'gdr':gdr,'json':json_,
    'cml':cml,'gdr2':gdr2,'selected':selected,'unselected':unselected}.items():
    draw_icon(name,paint)
print(f'Wrote {len(list(OUT.glob("*.png")))} 64px transparent game-style icons')
