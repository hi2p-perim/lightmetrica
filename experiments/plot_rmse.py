# -*- coding: utf-8 -*-

"""
	plot_rmse.py
	Visualize RMSE plot.
"""

import os
import argparse
import matplotlib.pyplot as plt

def main():

	# Parse comamnd line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('data', nargs='+', type=str, help='Data file')
	args = parser.parse_args()

	#plt.rc('axes', color_cycle=['r', 'g', 'b', 'y'])
	plt.xscale('log')
	plt.yscale('log')
	plt.xlabel('# of mutations')
	plt.ylabel('RMSE')

	for data_file in args.data:
		# Load data file
		xs = []
		ys = []
		with open(data_file) as f:
			lines = f.read().splitlines()
			for line in lines:
				floats = [float(x) for x in line.split()]
				xs.append(floats[0])
				ys.append(floats[1])

		# Plot data
		plt.plot(xs, ys, 'o-', label=os.path.splitext(os.path.basename(data_file))[0])

	plt.legend(loc='upper right')
	plt.show()

if __name__ == '__main__':
	main()
