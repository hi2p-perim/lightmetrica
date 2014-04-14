# -*- coding: utf-8 -*-

"""
	plot_pssmlt_hist2d.py
	Create histogram with 2 selected primary samples.
"""

import os
import argparse
import matplotlib.pyplot as plt

def main():
	
	# Parse comamnd line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('data', type=str, help='Data file')
	parser.add_argument('first_sample', type=int, help='Index of first sample')
	parser.add_argument('second_sample', type=int, help='Index of second sample')
	args = parser.parse_args()

	# Load data file
	x1 = []
	x2 = []
	with open(args.data) as f:
		lines = f.read().splitlines()
		for line in lines:
			floats = [float(x) for x in line.split()]
			x1.append(floats[args.first_sample+1])
			x2.append(floats[args.second_sample+1])

	# Plot
	fig = plt.figure()
	ax = fig.add_subplot(1, 1, 1)

	cax = ax.hist2d(x1, x2, bins=100)
	#fig.colorbar(cax)
	plt.show()

if __name__ == '__main__':
	main()