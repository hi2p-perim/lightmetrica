# -*- coding: utf-8 -*-

"""
	convert_hdr_to_png.py
	Convert HDR image to PNG image.
"""

import os
import argparse
import exputil as exp

def main():

	# Parse comamnd line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('image', type=str, help='Input image')
	args = parser.parse_args()

	# Load image
	image = exp.load_hdr(args.image)

	# Convert and save
	filename = os.path.splitext(os.path.basename(args.image))[0] + '.png'
	exp.save_image(filename, exp.hdr_compression(image))

if __name__ == '__main__':
	main()
