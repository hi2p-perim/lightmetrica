# -*- coding: utf-8 -*-

"""
	horizontal_arrange.py
	Combine two images horizontally.
"""

import argparse
import numpy as np
import exputil as exp

def main():
	# Parse command line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('image_1', type=str, help='Input image 1')
	parser.add_argument('image_2', type=str, help='Input image 2')
	args = parser.parse_args()

	# Load images
	image_1 = exp.load_hdr(args.image_1)
	image_2 = exp.load_hdr(args.image_2)

	# Combine images


	# Save image
	

if __name__ == '__main__':
	main()
