# -*- coding: utf-8 -*-

"""
	plot_pssmlt_traceplot_with_length.py
	Visualize traceplot for primary samples in PSSMLT along with path length.
"""

import os
import argparse
import matplotlib.pyplot as plt

def main():

	# Parse comamnd line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('data', type=str, help='Data file (traceplot)')
	parser.add_argument('length_data', type=str, help='Data file (length)')
	parser.add_argument('num_samples', type=int, default=1, help='Number of recorded samples')
	args = parser.parse_args()

	# Figure and axes
	fig, axs = plt.subplots(args.num_samples + 1, 1, sharex=True, sharey=False)

	# Plot length
	xs = []
	lengths = []
	with open(args.length_data) as f:
		lines = f.read().splitlines()
		for line in lines:
			floats = [float(x) for x in line.split()]
			xs.append(floats[0])
			lengths.append(floats[1])	

	ax = axs[0]
	ax.set_xlabel('# of mutations')
	ax.set_ylabel('Length')
	ax.plot(xs, lengths, '-')

	# Plot sample traces
	for i in xrange(0, args.num_samples):
		ax = axs[i+1]
		ax.set_xlabel('# of mutations')
		ax.set_ylabel('Value')
		ax.set_ylim(0, 1)

		# Load data file
		ys = []
		with open(args.data) as f:
			lines = f.read().splitlines()
			for line in lines:
				floats = [float(x) for x in line.split()]
				ys.append(floats[i+1])

		# Plot data
		ax.plot(xs, ys, '-')
		
	plt.show()

if __name__ == '__main__':
	main()
