# -*- coding: utf-8 -*-

"""
	compare_all.py
	Compare images rendered with various techniques.
"""

import os
import argparse
import numpy as np
import cv2
import exputil as exp

def main():

	# Parse comamnd line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('prefix', type=str, help='Prefix of input file (*.tpl.xml)')
	parser.add_argument('--output-dir', '-o', type=str, default='./', help='Output directory')
	parser.add_argument('-k', '--kernel-size', type=int, default=19, help='Kernel size')
	args = parser.parse_args()

	# Renderer enties
	renderers = [
		'pathtrace',
		'lighttrace',
		'simplebpt',
		'explicitpathtrace',
		'bpt',
		'pssmlt'
	]

	output_paths = [ os.path.join(args.output_dir, args.prefix + '.' + renderer + '.hdr') for renderer in renderers ]

	# Image size
	size = exp.image_size(output_paths[0])

	# Allocate output image
	width = size[0] * len(renderers)
	height = size[1] * len(renderers)
	result = np.zeros((height, width, 3), np.uint8)

	for i in xrange(0, len(renderers)):
		# First image
		image_1 = exp.load_hdr(output_paths[i])

		for j in xrange(i+1, len(renderers)):
			# Second image
			image_2 = exp.load_hdr(output_paths[j])
			# Compute errors -> Gaussian blur
			comp = cv2.GaussianBlur(image_1 - image_2, (args.kernel_size, args.kernel_size), 0)
			# Write to result
			result[j*size[1]:(j+1)*size[1], i*size[0]:(i+1)*size[0]] = exp.hdr_compression(abs(comp))
			# Caption
			cv2.putText(result, renderers[i] + ' vs. ' + renderers[j], (i*size[0], (j+1)*size[1]-5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 0, 0))

	# Border
	for i in xrange(1, len(renderers)):
		cv2.line(result, (0, i*size[1]), (width, i*size[1]), (128, 128, 128), 1)
	for i in xrange(1, len(renderers)):
		cv2.line(result, (i*size[0], 0), (i*size[0], height), (128, 128, 128), 1)

	# Lines
	cv2.line(result, (0, 0), (width, height), (128, 128, 128), 1)

	# Result
	exp.save_image(os.path.join(args.output_dir, 'render_all_comparison.png'), result)

if __name__ == '__main__':
	main()
