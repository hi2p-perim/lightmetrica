# -*- coding: utf-8 -*-

"""
	plot_pssmlt_traceplot.py
	Visualize traceplot for primary samples in PSSMLT.
"""

import os
import argparse
import matplotlib.pyplot as plt

def main():

	# Parse comamnd line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('data', type=str, help='Data file')
	parser.add_argument('num_samples', type=int, default=1, help='Number of recorded samples')
	args = parser.parse_args()

	# Figure and axes
	fig, axs = plt.subplots(args.num_samples, 1, sharex=True, sharey=True)
	if args.num_samples == 1:
		axs = [axs]

	for i, ax in enumerate(axs):

		ax.set_xlabel('# of mutations')
		ax.set_ylabel('Value')
		ax.set_ylim(0, 1)

		# Load data file
		xs = []
		ys = []
		with open(args.data) as f:
			lines = f.read().splitlines()
			for line in lines:
				floats = [float(x) for x in line.split()]
				xs.append(floats[0])
				ys.append(floats[i+1])

		# Plot data
		ax.plot(xs, ys, '-')
		
	plt.show()

if __name__ == '__main__':
	main()
