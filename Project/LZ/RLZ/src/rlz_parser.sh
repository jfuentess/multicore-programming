#!/usr/bin/env bash
set -o errexit
set -o nounset
set -o pipefail 

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
source "${DIR}/utils.sh"
  
main() 
{
    if [ "$#" -ne 5 ]; then
	echo "Script: '${0}'"
	echo "Function name:  ${FUNCNAME}"
	echo "uses 5 params instead of $#" 
	echo "${0} INPUT_FILE OUTPUT_FILE REF_SIZE_MB N_PARTITIONS MAX_MEM_MB"
	utils_die 
    fi
    
    INPUT_FILE=${1}
    OUT_FILE=${2}
    REF_SIZE_MB=${3}
    N_PARTITIONS=${4}
    MAX_MEM_MB=${5}
    
    CODER_BIN="${DIR}/rlz"
    
    utils_assert_file_exists ${INPUT_FILE}
    utils_assert_file_exists ${CODER_BIN}
    
    echo -n "Creating the reference ..."
    REF_FILE=tmp.reference
    REF_SIZE_BYTES=$(( ${REF_SIZE_MB}*1024*1024 ))
    head -c ${REF_SIZE_BYTES} ${INPUT_FILE} > ${REF_FILE}
    echo " Done."
    
    echo -n "Encoding the input text ..."
    ${CODER_BIN} ${REF_FILE} ${INPUT_FILE} ${OUT_FILE} ${N_PARTITIONS} ${MAX_MEM_MB}

    rm tmp.reference
}

main "$@"
