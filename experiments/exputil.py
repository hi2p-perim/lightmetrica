# -*- coding: utf-8 -*-

"""
	exputil.py
	A module for useful functions usable in experiments.
"""

import os
import array
import smc.freeimage as fi
from PIL import Image
import numpy as np
import cv2
import OpenEXR

def gamma_correction(a, gamma):
	return np.power(a, 1/gamma)

def image_size(file):
	img = fi.Image(file)
	return (img.width, img.height)

def load_hdr(file):
	img = fi.Image(file).flipVertical()
	floats = array.array('f', img.getRaw())
	return np.array(floats).reshape((img.width, img.height, 3))

def load_image(file):
	_, ext = os.path.splitext(file)
	if ext == '.hdr' or ext == '.exr':
		return load_hdr(file)
	else:
		return np.array(Image.open(file))

def hdr_compression(image):
	t = np.clip(gamma_correction(image, 2.2), 0, 1)
	return (t * 255).astype(np.uint8)

def save_image_radiancehdr(file, image):
	base, _ = os.path.splitext(file)
	tmpfile = '_' + base + '.exr'
	save_image_openexr(tmpfile, image)
	fi.Image(tmpfile).save(base + '.hdr')
	os.remove(tmpfile)

def save_image_openexr(file, image):
	(w, h, _) = image.shape
	image = image.reshape(w * h, 3)
	r = image[:,0]
	g = image[:,1]
	b = image[:,2]
	(rs, gs, bs) = [ array.array('f', channel).tostring() for channel in (r, g, b) ]
	out = OpenEXR.OutputFile(file, OpenEXR.Header(w, h))
	out.writePixels({'R' : rs, 'G' : gs, 'B' : bs})

def save_image(file, image):
	_, ext = os.path.splitext(file)
	if ext == '.exr':
		save_image_openexr(file, image)
	elif ext == '.hdr':
		save_image_radiancehdr(file, image)
	else:
		img = Image.fromarray(image)
		img.save(file)

def show_image_rgb(image, title=''):
	show_image_bgr(cv2.cvtColor(image, cv2.COLOR_RGB2BGR), title)

def show_image_bgr(image, title=''):
	cv2.imshow('', image)
	cv2.waitKey(0)
	cv2.destroyAllWindows()