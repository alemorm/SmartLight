#!/bin/bash

# Set script name variable
SCRIPT=`basename ${0}`

# Set project directory
SCRIPT_FULL_PATH=$(dirname "${0}")

# Set fonts for Help.
NORM=`tput sgr0`
BOLD=`tput bold`
REV=`tput smso`

# Initial variables
srcFile="${SCRIPT_FULL_PATH}/src/TimeLightClient/TimeLightClient.ino"
wifiNameVar="wifiName"
wifikeyVar="wifiKey"
iftttkeyVar="iftttKey"
wifiName=$(grep "${wifiNameVar} =.*" ${SCRIPT_FULL_PATH}/auth.keys)
wifiKey=$(grep "${wifikeyVar} =.*" ${SCRIPT_FULL_PATH}/auth.keys)
iftttKey=$(grep "${iftttkeyVar} =.*" ${SCRIPT_FULL_PATH}/auth.keys)

# Print function for usage help information
print_usage() {
  echo
  echo -e "${BOLD}Help description to anonymize the authentication data in the source files${BOLD}\n"
  echo -e "${REV}Usage:${NORM} ${SCRIPT} [-h] [-a|-b]\n"
  echo -e "${REV}-h ${NORM} show this help message"
  echo -e "${REV}-a ${NORM} anonymize the source files of WiFi and IFTTT sensitive information"
  echo -e "${REV}-b ${NORM} add back the authentication information from the auth.keys file"
  echo -e
  echo -e "Example 1 (anonymize the source file prior to pushing any changes to Github):"
  echo -e "${BOLD}${SCRIPT} -a${BOLD}"
  echo -e "${BOLD}git commit -am 'Commit without any sensitive authentication data'${BOLD}"
  echo -e "${BOLD}git push'${BOLD}\n"
  echo -e "Example 2 (restore the authentication information from the auth.keys file locally 
            in order to successfully compile the source file):"
  echo -e "${BOLD}${SCRIPT} -b${BOLD}\n"
  exit 1
}

# Handle the case with no options given
if [ $# -eq 0 ]
then
    echo -e "Please choose an option."
    print_usage
fi

# Handle the flags
while getopts :hab flag; do
  case ${flag} in
    h) print_usage;;
    a) sed -i 's/\(.*wifiName =\).*/\1/' ${srcFile} &&
       sed -i 's/\(.*Key =\).*/\1/' ${srcFile} ;;
    b) sed -i "s:\(.*\)${wifiNameVar} =.*:\1${wifiName}:" ${srcFile} &&
       sed -i "s:\(.*\)${wifikeyVar} =.*:\1${wifiKey}:" ${srcFile} &&
       sed -i "s:\(.*\)${iftttkeyVar} =.*:\1${iftttKey}:" ${srcFile} ;;
    \?) echo "Option ${BOLD}-${OPTARG}${BOLD} not allowed."; print_usage ;;
  esac
done