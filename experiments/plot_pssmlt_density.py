# -*- coding: utf-8 -*-

"""
	plot_pssmlt_density.py
	Visualize densities of primary samples.
"""

import os
import argparse
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab

def main():
	
	# Parse comamnd line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('data', type=str, help='Data file')
	parser.add_argument('num_samples', type=int, default=1, help='Number of recorded samples')
	args = parser.parse_args()

	# Figure and axes
	fig, axs = plt.subplots(args.num_samples, 1, sharex=True, sharey=True)

	for i, ax in enumerate(axs):

		ax.set_xlabel('Sample value')
		ax.set_ylabel('Frequency')
		#ax.set_xlim(0, 1)
		#ax.set_ylim(0, 1)

		# Load data file
		vs = []
		with open(args.data) as f:
			lines = f.read().splitlines()
			for line in lines:
				floats = [float(x) for x in line.split()]
				vs.append(floats[i+1])

		# Create histogram
		n, bins, patches = ax.hist(vs, 200, normed=True)
		
		# add a 'best fit' line for the normal PDF
		# bincenters = 0.5*(bins[1:]+bins[:-1])
		# mu, sigma = 100, 15
		# y = mlab.normpdf(bincenters, mu, sigma)
		# ax.plot(bincenters, y, 'r--', linewidth=1)

	plt.show()

if __name__ == '__main__':
	main()
