#!/usr/bin/env python3

# Script to use when releasing new versions
# Run from main repo directory with data submodule pulled in data/
#
# Will automatically:
# * Make sure you don't have any uncommited local changes
# * Make sure you don't forget to pull any changes before you start
# * Get current version number from CMakeLists.txt
# * Merge dev -> master in the data submodule
# * Tag release in data submodule
# * Update dev in data submodule to point to the new merge commit
# * Merge dev -> master in main repo
# * Bump version number in main repo
# * Tag release in main repo
# * Update dev in main repo to point to the new merge commit
# * Push everything to remote
# * Create release drafts on GitHub
#
# After finished, verify everything is correct and push the changes

import os
import re
import sys
import subprocess
import io

version_override = None
if len(sys.argv) > 1:
	m = re.match(r'^(.*?)\.(.*?)\.(.*)$', sys.argv[1])
	if m is None:
		print('\033[1;31m[!] Unable to parse version override argument\033[0m')
		sys.exit(1)
	version_override = (m.group(1), m.group(2), m.group(3))
	print('\033[1;34m[*] Version override: '+('%s.%s.%s' % version_override)+'\033[0m')


try:
	git_root = subprocess.check_output(['git', 'rev-parse', '--show-toplevel']).strip()
except subprocess.CalledProcessError:
	print('\033[1;31m[!] Not inside a git repository!\033[0m')
	sys.exit(1)
os.chdir(git_root)
while not os.path.isdir('.git'):
	# Likely inside a submodule
	# TODO: There is a command called `git rev-parse --show-superproject-working-tree` but it's quite new so not always available :/
	os.chdir('..')

print('\033[1;34m[*] Make sure you don\'t have any uncommited local changes...\033[0m')
if subprocess.check_output(['git', 'status', '--porcelain']): # This also handles data subdirectory automatically
	print('\033[1;31m[!] You have uncommited local changes!\033[0m')
	os.system('git status') # Show the changes
	sys.exit(1)

print('\033[1;34m[*] Make sure all remote changes are pulled...\033[0m')
subprocess.check_call(['git', 'checkout', 'dev'])
subprocess.check_call(['git', 'pull', '--ff'])
subprocess.check_call(['git', 'checkout', 'master'])
subprocess.check_call(['git', 'pull', '--ff'])
os.chdir('data')
subprocess.check_call(['git', 'checkout', 'dev'])
subprocess.check_call(['git', 'pull', '--ff'])
subprocess.check_call(['git', 'checkout', 'master'])
subprocess.check_call(['git', 'pull', '--ff'])
os.chdir('..')

print('\033[1;34m[*] Get version numbers...\033[0m')
subprocess.check_call(['git', 'checkout', 'dev'])
major = None
minor = None
revision = None
codename = None
data = open('CMakeLists.txt', 'r').readlines()

for i in range(len(data)):
	m = re.match(r'^set\(COLOBOT_VERSION_(MAJOR|MINOR|REVISION)( +)([0-9]+)\)$', data[i])
	if m:
		x = m.group(3)
		if m.group(1) == 'MAJOR':
			if version_override is not None:
				x = version_override[0]
			major = x
		elif m.group(1) == 'MINOR':
			if version_override is not None:
				x = version_override[1]
			minor = x
		elif m.group(1) == 'REVISION':
			if version_override is not None:
				x = version_override[2]
			else:
				# Increase revision number
				revision = str(int(x) + 1)
			revision = x
		data[i] = 'set(COLOBOT_VERSION_'+m.group(1)+m.group(2)+x+')\n'

	m = re.match(r'^(#?)set\(COLOBOT_VERSION_(UNRELEASED|RELEASE_CODENAME)( +)"(.+)"\)$', data[i])
	if m:
		comment = (m.group(2) == 'UNRELEASED')
		if m.group(2) == 'RELEASE_CODENAME':
			codename = m.group(4)
		data[i] = ('#' if comment else '')+'set(COLOBOT_VERSION_'+m.group(2)+m.group(3)+'"'+m.group(4)+'")\n'

subprocess.check_call(['git', 'checkout', 'master'])
version = '%s.%s.%s%s' % (major, minor, revision, codename)
version_human = '%s %s.%s.%s' % (codename.strip('-'), major, minor, revision)
print('\033[1;32m[+] Building version '+version+'\033[0m')

print('\033[1;34m[*] Merge data...\033[0m')
os.chdir('data')
subprocess.check_call(['git', 'merge', 'dev', '--no-ff', '-m', 'Release '+version+': Merge branch \'dev\''])

print('\033[1;34m[*] Tag data...\033[0m')
subprocess.check_call(['git', 'tag', 'colobot-gold-'+version])

print('\033[1;34m[*] Update dev on data...\033[0m')
subprocess.check_call(['git', 'checkout', 'dev'])
subprocess.check_call(['git', 'merge', 'master', '--ff'])
subprocess.check_call(['git', 'checkout', 'master'])

print('\033[1;34m[*] Merge main...\033[0m')
os.chdir('..')
subprocess.check_call(['git', 'merge', 'dev', '--no-ff', '-m', 'Release '+version+': Merge branch \'dev\''])

print('\033[1;34m[*] Bump version number\033[0m')
open('CMakeLists.txt', 'w').writelines(data)
subprocess.check_call(['git', 'commit', 'data', 'CMakeLists.txt', '-m', 'Release '+version+': Bump version'])

print('\033[1;34m[*] Tag main...\033[0m')
subprocess.check_call(['git', 'tag', 'colobot-gold-'+version])

print('\033[1;34m[*] Update dev on main...\033[0m')
subprocess.check_call(['git', 'checkout', 'dev'])
subprocess.check_call(['git', 'merge', 'master', '--ff'])
for i in range(len(data)):
	m = re.match(r'^(#?)set\(COLOBOT_VERSION_(UNRELEASED|RELEASE_CODENAME)(.*)\)$', data[i])
	if m:
		comment = (m.group(2) == 'RELEASE_CODENAME')
		data[i] = ('#' if comment else '')+'set(COLOBOT_VERSION_'+m.group(2)+m.group(3)+')\n'
open('CMakeLists.txt', 'w').writelines(data)
subprocess.check_call(['git', 'commit', 'CMakeLists.txt', '-m', 'Post-release '+version])
subprocess.check_call(['git', 'checkout', 'master'])

print('\033[1;32m[+] Done preparing!\033[0m')
resp = ""
while resp != "yes":
	resp = input("\033[1;35m[?] Ready to push? (type \'yes\'): \033[0m")

print('\033[1;34m[*] Pushing...\033[0m')
os.chdir('data')
subprocess.check_call(['git', 'push', 'origin', 'master', 'dev', 'colobot-gold-'+version])
os.chdir('..')
subprocess.check_call(['git', 'push', 'origin', 'master', 'dev', 'colobot-gold-'+version])

hub_available = True
try:
	subprocess.check_call(['hub', '--version'])
except subprocess.CalledProcessError:
	hub_available = False
	print('\033[1;33m[!] hub is not available, will skip creating release drafts!\033[0m')

if hub_available:
	print('\033[1;34m[*] Making release drafts on GitHub...\033[0m')
	os.chdir('data')
	release_notes = io.StringIO()
	release_notes.write('Colobot: Gold Edition - '+version_human+' - data files\n')
	release_notes.write('\n')
	release_notes.write('Data files for release '+version)
	subprocess.check_call(['hub', 'release', 'create', '--draft', '-m', release_notes.getvalue(), 'colobot-gold-'+version])
	os.chdir('..')
	release_notes = io.StringIO()
	release_notes.write('Colobot: Gold Edition - '+version_human+'\n')
	release_notes.write('\n')
	release_notes.write('Release '+version+'\n')
	release_notes.write('\n')
	release_notes.write('[put release notes URL here]')
	subprocess.check_call(['hub', 'release', 'create', '--draft', '-m', release_notes.getvalue(), 'colobot-gold-'+version])

print('\033[1;32m[+] Done!\033[0m')
