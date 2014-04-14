# -*- coding: utf-8 -*-

"""
	plot_pssmlt_length.py
	Visualize the lengths of light path samples.
"""
import os
import argparse
import matplotlib.pyplot as plt

def main():

	# Parse comamnd line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('data', type=str, help='Data file')
	args = parser.parse_args()

	# Load data file
	xs = []
	ys = []
	with open(args.data) as f:
		lines = f.read().splitlines()
		for line in lines:
			floats = [float(x) for x in line.split()]
			xs.append(floats[0])
			ys.append(floats[1])

	# Figure and axes
	fig = plt.figure()
	ax = fig.add_subplot(1, 1, 1)

	# Plot data
	ax.plot(xs, ys, '-')

	plt.show()

if __name__ == '__main__':
	main()
