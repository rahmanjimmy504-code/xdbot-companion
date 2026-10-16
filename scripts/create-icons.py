#!/usr/bin/env python3
"""Crop 14 independently generated arcade-game icon images into transparent PNG sprites.

The images in art/source-icons/ are generated image assets, not traced or
programmatically drawn pictograms. This script ONLY extracts their silhouettes,
cleans the plain background and scales them to 96x96 for the native UI. Pillow,
OpenCV and NumPy are development dependencies, not packaged with the mod.
"""
import sys
from pathlib import Path
import cv2
import numpy as np
from PIL import Image

ROOT=Path(__file__).resolve().parent.parent
SOURCE=ROOT/'art/source-icons'
OUT=ROOT/'resources/icons'
NAMES=('hype','gdb','prev','next','all','clear','export','share',
       'gdr','json','cml','gdr2','selected','unselected')

def convert(name):
    image=Image.open(SOURCE/(name+'.png')).convert('RGB')
    rgb=np.asarray(image)
    hi=rgb.max(axis=2).astype(np.int16)
    lo=rgb.min(axis=2).astype(np.int16)
    # The generator makes off-white backdrops even when asked for alpha.
    foreground=((lo<230)|((hi-lo)>27)).astype('uint8')*255
    contours,_=cv2.findContours(foreground,cv2.RETR_EXTERNAL,cv2.CHAIN_APPROX_SIMPLE)
    assert contours, f'No icon object: {name}'
    largest=max(contours,key=cv2.contourArea)
    assert cv2.contourArea(largest) > image.width*image.height*.10, f'Object is too small: {name}'
    assert cv2.contourArea(largest) < image.width*image.height*.94, f'Background may have been selected: {name}'
    mask=np.zeros(foreground.shape,dtype=np.uint8)
    cv2.drawContours(mask,[largest],-1,255,cv2.FILLED)
    # Anti-alias just the edge without altering filled white areas inside art.
    mask=cv2.GaussianBlur(mask,(3,3),.65)
    x,y,w,h=cv2.boundingRect(largest)
    rgba=np.dstack([rgb,mask]);object_image=Image.fromarray(rgba,'RGBA').crop((x,y,x+w,y+h))
    object_image.thumbnail((91,91),Image.Resampling.LANCZOS)
    tile=Image.new('RGBA',(96,96),(0,0,0,0))
    tile.alpha_composite(object_image,((96-object_image.width)//2,(96-object_image.height)//2))
    OUT.mkdir(parents=True,exist_ok=True)
    tile.save(OUT/(name+'.png'),optimize=True)
    assert tile.getpixel((0,0))[3]==0,name
    print(name,'source',image.size,'sprite',object_image.size)

if __name__=='__main__':
    partial='--partial' in sys.argv[1:]
    missing=[n for n in NAMES if not (SOURCE/(n+'.png')).exists()]
    if missing and not partial:raise SystemExit('Missing generated images: '+', '.join(missing))
    for n in NAMES:
        if n not in missing: convert(n)
    print(f'Prepared {len(NAMES)-len(missing)} independently generated 96px sprites; missing {missing}')
