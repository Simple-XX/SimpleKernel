#!/usr/bin/env bash

function usage()
{
	echo "Usage:"
	echo " $0 [options]"
	echo "Options:"
	echo "     -h                   Display help or usage"
	echo "     -i <input_config>    Input config file"
	echo "     -l <variable_list>   List of variables in the array (Optional)"
	exit 1;
}

# Command line options
CONFIG_FILE=""
VAR_LIST=""

while getopts "hi:l:" o; do
	case "${o}" in
	h)
		usage
		;;
	i)
		CONFIG_FILE=${OPTARG}
		;;
	l)
		VAR_LIST=${OPTARG}
		;;
	*)
		usage
		;;
	esac
done
shift $((OPTIND-1))

if [ -z "${CONFIG_FILE}" ]; then
	echo "Must specify input config file"
	usage
fi

if [ ! -f "${CONFIG_FILE}" ]; then
	echo "The input config file should be a present"
	usage
fi

TYPE_HEADER=`cat ${CONFIG_FILE} | awk '{ if ($1 == "HEADER:") { printf $2; exit 0; } }'`
if [ -z "${TYPE_HEADER}" ]; then
	echo "Must specify HEADER: in input config file"
	usage
fi

TYPE_NAME=`cat ${CONFIG_FILE} | awk '{ if ($1 == "TYPE:") { printf $2; for (i=3; i<=NF; i++) printf " %s", $i; exit 0; } }'`
if [ -z "${TYPE_NAME}" ]; then
	echo "Must specify TYPE: in input config file"
	usage
fi

ARRAY_NAME=`cat ${CONFIG_FILE} | awk '{ if ($1 == "NAME:") { printf $2; exit 0; } }'`
if [ -z "${ARRAY_NAME}" ]; then
	echo "Must specify NAME: in input config file"
	usage
fi

printf "#include <%s>\n\n" "${TYPE_HEADER}"

for VAR in ${VAR_LIST}; do
	printf "extern %s %s;\n" "${TYPE_NAME}" "${VAR}"
done
printf "\n"

printf "%s *%s[] = {\n" "${TYPE_NAME}" "${ARRAY_NAME}"
for VAR in ${VAR_LIST}; do
	printf "\t&%s,\n" "${VAR}"
done
printf "};\n\n"

printf "unsigned long %s_size = sizeof(%s) / sizeof(%s *);\n" "${ARRAY_NAME}" "${ARRAY_NAME}" "${TYPE_NAME}"
