#!/bin/bash

# Configure in MCUXpresso in the workspace a build variable for the .hex output directory
# 'Window->Preferences->C/C++->Build->Build Variables->Add...'
# Variable name: hexDir
# Type:          String
# Value:         absolute path to .hex output directory

# Configure in MCUXpresso for ALL build configurations:

# 'Project->Properties->C/C++ Build->Build Variables->Add...'
# Variable name: sw_version
# Type:          String
# Value:         x.xx

# 'Project->Properties->C/C++ Build->Settings->Build Artifact->Artifact name:'
# ${ProjName}_${ConfigName}_v${sw_version}

# 'Project->Properties->C/C++ Build->Settings->Build steps->Post-build steps->Edit...'
# "${ProjDirPath}/../../../script/build-script/post-build-steps.sh" "${CWD}" "${TargetChip}" "${BuildArtifactFileName}" "${BuildArtifactFileBaseName}" "${ConfigName}" "${sw_version}" "${hexDir}"

# fail on error
set -e

#error handler
exit_on_error()
{
    echo "Last command failed. Exiting..."
    exit 1
}

# enable error handling
trap exit_on_error ERR

# Selfbus library version
sbLibVersion="2.02"

# Selfbus library version prefix
sbLibprefix="libv"

# App version prefix
appVersionPrefix="v"

# searchstrings and subfolders for .hex output directory
debug="debug"
release="release"
flashstart="flashstart"

CurrentWorkingDirectory="${1}"
TargetChip="${2}"
BuildArtifactFileName="${3}"
BuildArtifactFileBaseName="${4}"
appConfigName="${5}"
appVersion="${6}"
hexDir="${7}"

echo ""
echo "Selfbus post build steps:"
echo "Creating .hex/.bin files and adding sblib version "${sbLibVersion}" to filename"

newName="${BuildArtifactFileBaseName}"
# append build config name and app version
newName="${newName}"_"${appConfigName}"_"${appVersionPrefix}""${appVersion}"
# replace spaces with underscores
newName="${newName// /_}"
# replace missing options (--)
newName="${newName//--/_}"
# fix trailing missing option
newName="${newName//-_/_}"
# fix leading missing option
newName="${newName//_-/_}"
# replace duplicate underscores
newName="${newName//__/_}"
# convert to lowercase
newName=`echo "${newName}" | tr '[:upper:]' '[:lower:]'`
# add lib version
newName="${newName}_${sbLibprefix}${sbLibVersion}"
echo "${newName}"

# post-build-step from eclipse
arm-none-eabi-size "${BuildArtifactFileName}"
# create .bin file
arm-none-eabi-objcopy -v -O binary "${BuildArtifactFileName}" "${newName}.bin"
# create .hex file
arm-none-eabi-objcopy -v -O ihex "${BuildArtifactFileName}" "${newName}.hex"

# check if hexDir is defined
if [ -z "${hexDir}" ]; then
    echo "Workspace build variable hexDir not defined or empty, exiting post-build-script."
    exit 1
fi

# add release/debug to hex output directory
case ${newName} in
  *"${debug}"*)
    hexDir="${hexDir}"/"${debug}"
    ;;

  *"${release}"*)
    hexDir="${hexDir}"/"${release}"
    ;;
esac

# check adding flashstart to hex output directory
if [[ "$newName" =~ .*"$flashstart".* ]]; then
  hexDir="${hexDir}"/"${flashstart}"
fi

# check for .hex output directory
if [ ! -d "${hexDir}" ]; then
  echo "Creating "${hexDir}""
  mkdir -p "${hexDir}"
fi

cp --verbose "${CurrentWorkingDirectory}"/"${newName}.hex" "${hexDir}"/

# Do not activate checksum, not sure why, but at least .hex files gets corrupted
# see also https://community.nxp.com/t5/Blogs/Hex-file-settings-in-MCUxpresso/bc-p/1131124/highlight/true#M53
# add checksums
#checksum -p ${TargetChip} -d "${newName}.bin"
#checksum -p ${TargetChip} -d "${newName}.hex"

