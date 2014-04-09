# -*- coding: utf-8 -*-

"""
	plot_rmse.py
	Visualize RMSE plot.
"""

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

	# Plot data
	plt.plot(xs, ys, 'bo-')
	plt.xscale('log')
	plt.yscale('log')
	plt.xlabel('# of mutations')
	plt.ylabel('RMSE')
	plt.show()

if __name__ == '__main__':
	main()
