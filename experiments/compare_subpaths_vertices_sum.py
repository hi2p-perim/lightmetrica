# -*- coding: utf-8 -*-

"""
	compare_subpaths_vertices_sum.py
	Compares sub-paths whose sum of vertices is same.
	e.g. If the sum # of vertices is 3, rendered images for sub-paths with
	(s, t) = (0, 3), (1, 2), (2, 1), (3, 0) are compared mutially.
"""

import sys
import argparse
import numpy as np
import cv2
import exputil as exp

def main():
	# Parse comamnd line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('vs', type=int, help='Sum of # of vertices')
	parser.add_argument('min_s', type=int, help='Lower bound of s')
	parser.add_argument('max_s', type=int, help='Upper bound of s')
	parser.add_argument('-k', '--kernel-size', type=int, default=19, help='Kernel size')
	args = parser.parse_args()

	# Size of the image
	size = exp.image_size('s00t00.hdr')

	# Allocate output image
	width = size[0] * (args.vs + 1)
	height = size[1] * (args.vs + 1)
	result = np.zeros((height, width, 3), np.uint8)

	# Enumerate images with sampe sum # of vertices
	for s1 in xrange(args.min_s, args.max_s+1):
		t1 = args.vs - s1

		# Load first image
		image_1 = exp.load_hdr('s%02dt%02d.hdr' % (s1, t1))

		for s2 in xrange(s1+1, args.max_s+1):
			t2 = args.vs - s2

			# Load second image
			image_2 = exp.load_hdr('s%02dt%02d.hdr' % (s2, t2))

			# Compute errors -> Gaussian blur
			comp = cv2.GaussianBlur(image_1 - image_2, (args.kernel_size, args.kernel_size), 0)

			# Write to result
			result[s2*size[1]:(s2+1)*size[1], s1*size[0]:(s1+1)*size[0]] = exp.hdr_compression(abs(comp))

			# Caption
			cv2.putText(result, 's=%d, t=%d vs. s=%d, t=%d' % (s1, t1, s2, t2), (s1*size[0], (s2+1)*size[1]-5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255))

	# Border
	for i in xrange(1, args.vs+1):
		cv2.line(result, (0, i*size[1]), (width, i*size[1]), (128, 128, 128), 1)
	for i in xrange(1, args.vs+1):
		cv2.line(result, (i*size[0], 0), (i*size[0], height), (128, 128, 128), 1)

	# Lines
	cv2.line(result, (0, 0), (width, height), (128, 128, 128), 1)

	# Result
	exp.save_image('compare_vs%02d.png' % args.vs, result)

if __name__ == '__main__':
	main()
