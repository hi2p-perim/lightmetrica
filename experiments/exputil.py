# -*- coding: utf-8 -*-

"""
	exputil.py
	A module for useful functions usable in experiments.
"""

import array
import smc.freeimage as fi
from PIL import Image
import numpy as np
import cv2

def gamma_correction(a, gamma):
	return np.power(a, 1/gamma)

def image_size(file):
	img = fi.Image(file)
	return (img.width, img.height)

def load_hdr(file):
	img = fi.Image(file).flipVertical()
	floats = array.array('f', img.getRaw())
	return np.array(floats).reshape((img.width, img.height, 3))

def hdr_compression(image):
	t = np.clip(gamma_correction(image, 2.2), 0, 1)
	return (t * 255).astype(np.uint8)

def save_image(file, image):
	img2 = Image.fromarray(image)
	img2.save(file)

def show_image_rgb(image, title=''):
	show_image_bgr(cv2.cvtColor(image, cv2.COLOR_RGB2BGR), title)

def show_image_bgr(image, title=''):
	cv2.imshow('', image)
	cv2.waitKey(0)
	cv2.destroyAllWindows()