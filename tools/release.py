from __future__ import print_function
import fileinput
import re
import os

# Script to use when releasing new versions
# Run from main repo directory with data submodule pulled in data/
#
# Note: this has not yet been tested thoughtly, VERIFY EVERYTHING THIS SCRIPT DOES MANUALLY
#
# Will automatically:
# * Make sure you don't forget to pull any changes before you start
# * Get current version number from CMakeLists.txt
# * Merge dev -> master in the data submodule
# * Tag release in data submodule
# * Update dev in data submodule to point to the new merge commit
# * Merge dev -> master in main repo
# * Bump version number in main repo
# * Tag release in main repo
# * Update dev in main repo to point to the new merge commit
#
# After finished, verify everything is correct and push the changes

print('\033[1;34m[*] Make sure all remote changes are pulled...\033[0m')
os.system('git checkout dev')
os.system('git pull --ff')
os.system('git checkout master')
os.system('git pull --ff')
os.chdir('data')
os.system('git checkout dev')
os.system('git pull --ff')
os.system('git checkout master')
os.system('git pull --ff')
os.chdir('..')

print('\033[1;34m[*] Get version numbers...\033[0m')
os.system('git checkout dev')
major = None
minor = None
revision = None
codename = None
data = open('CMakeLists.txt', 'r').readlines()

for i in range(len(data)):
	m = re.match(r'^set\(COLOBOT_VERSION_(MAJOR|MINOR|REVISION)( +)([0-9])+\)$', data[i])
	if m:
		x = int(m.group(3))
		if m.group(1) == 'MAJOR':
			major = x
		elif m.group(1) == 'MINOR':
			minor = x
		elif m.group(1) == 'REVISION':
			# Increase revision number
			x += 1
			revision = x
		data[i] = 'set(COLOBOT_VERSION_'+m.group(1)+m.group(2)+str(x)+')\n'

	m = re.match(r'^(#?)set\(COLOBOT_VERSION_(UNRELEASED|RELEASE_CODENAME)( +)"(.+)"\)$', data[i])
	if m:
		comment = (m.group(2) == 'UNRELEASED')
		if m.group(2) == 'RELEASE_CODENAME':
			codename = m.group(4)
		data[i] = ('#' if comment else '')+'set(COLOBOT_VERSION_'+m.group(2)+m.group(3)+'"'+m.group(4)+'")\n'

os.system('git checkout master')
version = '%d.%d.%d%s' % (major, minor, revision, codename)
print('\033[1;32m[+] Building version '+version+'\033[0m')

print('\033[1;34m[*] Merge data...\033[0m')
os.chdir('data')
os.system('git merge dev --no-ff -m "Release '+version+': Merge branch \'dev\'"')

print('\033[1;34m[*] Tag data...\033[0m')
os.system('git tag colobot-gold-'+version)

print('\033[1;34m[*] Update dev on data...\033[0m')
os.system('git checkout dev')
os.system('git merge master --ff')
os.system('git checkout master')

print('\033[1;34m[*] Merge main...\033[0m')
os.chdir('..')
os.system('git merge dev --no-ff -m "Release '+version+': Merge branch \'dev\'"')

print('\033[1;34m[*] Bump version number\033[0m')
open('CMakeLists.txt', 'w').writelines(data)
os.system('git commit data CMakeLists.txt -m "Release '+version+': Bump version"')

print('\033[1;34m[*] Tag main...\033[0m')
os.system('git tag colobot-gold-'+version)

print('\033[1;34m[*] Update dev on main...\033[0m')
os.system('git checkout dev')
os.system('git merge master --ff')
for i in range(len(data)):
	m = re.match(r'^(#?)set\(COLOBOT_VERSION_(UNRELEASED|RELEASE_CODENAME)(.*)\)$', data[i])
	if m:
		comment = (m.group(2) == 'RELEASE_CODENAME')
		data[i] = ('#' if comment else '')+'set(COLOBOT_VERSION_'+m.group(2)+m.group(3)+')\n'
open('CMakeLists.txt', 'w').writelines(data)
os.system('git commit CMakeLists.txt -m "Post-release '+version+'"')
os.system('git checkout master')

print('\033[1;32m[+] Done! Push when ready\033[0m')
