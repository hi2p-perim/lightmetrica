# -*- coding: utf-8 -*-

"""
	heatmap.py
	Visualize HDR image as heatmap.
"""

import argparse
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.cm as cm
import exputil as exp

def main():
	# Parse command line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('image', type=str, help='Input image')
	parser.add_argument('--min', type=float, default=0, help='Minimum')
	parser.add_argument('--max', type=float, default=float('inf'), help='Maximum')
	args = parser.parse_args()

	# Load image
	image = exp.load_hdr(args.image)
	image_comp = np.clip(exp.gamma_correction(image[:,:,0], 2.2), args.min, args.max)

	# Draw heatmap
	plt.clf()
	plt.gca().invert_yaxis()
	plt.pcolor(image_comp, cmap=cm.jet)
	plt.colorbar()
	plt.show()

if __name__ == '__main__':
	main()
