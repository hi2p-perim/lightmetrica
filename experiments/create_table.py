# -*- coding: utf-8 -*-

"""
	create_table.py
	Create table of sub-path images.
"""

import os
import re
import numpy as np
import cv2
import exputil as exp

def main():

	# Enumerate files
	files = [f for f in os.listdir('.')]

	# Find max value of s and t respectively
	maxS = 0
	maxT = 0
	for f in files:
		# Extract s and t values
		m = re.match(r'^s([0-9]+)t([0-9]+).hdr$', f)
		if m:
			maxS = max(maxS, int(m.group(1)))
			maxT = max(maxT, int(m.group(2)))

	# Size of the image
	size = exp.image_size('s00t00.hdr')

	# Allocate image
	width = size[0] * (maxT + 1)
	height = size[1] * (maxS + 1)
	result = np.zeros((height, width, 3), np.uint8)

	# Draw images
	for s in xrange(0, maxS+1):
		for t in xrange(0, maxT+1):
			image = exp.hdr_compression(exp.load_hdr('s%02dt%02d.hdr' % (s, t)))
			result[t*size[1]:(t+1)*size[1], s*size[0]:(s+1)*size[0]] = image

			# Caption
			cv2.putText(result, 's=%d, t=%d' % (s, t), (s*size[0], (t+1)*size[1]-5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255))

	# Border
	for s in xrange(1, maxS+1):
		cv2.line(result, (0, s*size[1]), (width, s*size[1]), (128, 128, 255), 1)
	for t in xrange(1, maxT+1):
		cv2.line(result, (t*size[0], 0), (t*size[0], height), (255, 128, 128), 1)

	exp.save_image('table.png', result)

if __name__ == '__main__':
	main()
