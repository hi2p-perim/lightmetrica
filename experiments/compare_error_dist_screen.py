# -*- coding: utf-8 -*-

"""
	compare_error_dist_screen.py
	Compares error distributions of the two HDR images in screen space.
"""

import sys
import argparse
import numpy as np
import cv2
import exputil as exp

def main():
	
	# Parse comamnd line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('image_1', type=str, help='First input image')
	parser.add_argument('image_2', type=str, help='Second input image')
	parser.add_argument('-k', '--kernel-size', type=int, default=19, help='Kernel size')
	args = parser.parse_args()

	# Load images
	image_1 = exp.load_hdr(args.image_1)
	image_2 = exp.load_hdr(args.image_2)

	# Gaussian blur on difference image
	diff = image_1 - image_2
	blurred = cv2.GaussianBlur(diff, (args.kernel_size, args.kernel_size), 0)

	exp.show_image_rgb(exp.hdr_compression(abs(blurred)))

if __name__ == '__main__':
	main()
