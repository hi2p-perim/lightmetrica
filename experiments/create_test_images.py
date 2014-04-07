# -*- coding: utf-8 -*-

"""
	create_test_images.py
	Create small 4x4 images which is used by lightmetrica.test.
"""

import sys
import argparse
import numpy as np
import exputil as exp

def main():
	
	# Parse comamnd line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('prefix', type=str, help='Prefix of output images')
	args = parser.parse_args()

	# Image data
	image = np.array(
		[[[1,1,1], [1,0,0]],
		 [[0,0,1], [0,0,0]]])

	# Save
	exp.save_image(args.prefix + '.exr', image)
	exp.save_image(args.prefix + '.hdr', image)
	exp.save_image(args.prefix + '.png', (np.clip(image, 0, 1) * 255).astype(np.uint8))

	# Open
	# print('EXR:')
	# print(exp.load_image(args.prefix + '.exr'))
	# print('HDR:')
	# print(exp.load_image(args.prefix + '.hdr'))
	# print('PNG:')
	# print(exp.load_image(args.prefix + '.png'))

if __name__ == '__main__':
	main()
