# -*- coding: utf-8 -*-

"""
	render_all.py
	Render same scene with various techniques.
"""

import os
import argparse
import jinja2
import subprocess as sp

def main():
	
	# Parse comamnd line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('infile', type=str, help='Input file')
	parser.add_argument('--entry', '-e', type=str, nargs='*', help='Add an entry for template dictionary')
	args = parser.parse_args()

	# Load template
	template_loader = jinja2.FileSystemLoader(os.path.dirname(args.infile))
	template_env = jinja2.Environment(loader = template_loader)
	template = template_env.get_template(os.path.basename(args.infile))

	# Create dict
	context = {}
	if args.entry is not None:
		for entry in args.entry:
			key, value = entry.split(':')
			context[key] = value

	# Renderer enties
	renderers = [
		'raycast',
		'pathtrace',
		'lighttrace',
		'simplebpt',
		'explicitpathtrace',
		'bpt',
		'pssmlt'
	]

	for renderer in renderers:
		# Message
		print("Begin rendering with '" + renderer + "'")

		# Set 'renderer_type'
		context['renderer_type'] = renderer

		# Resolve template
		resolved_template = template.render(context)

		# Call renderer
		p = sp.Popen(
			[
				'./lightmetrica',
				'-i',
				'-b', '../../../resources/',
				'-o', os.path.basename(args.infile).split('.')[0] + '.' + renderer + '.hdr'
			],
			stdin=sp.PIPE)

		p.communicate(resolved_template.encode('utf-8'))

if __name__ == '__main__':
	main()
