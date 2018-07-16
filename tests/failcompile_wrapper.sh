#!/bin/bash

output=$(${CMAKE_COMMAND} --build . --target ${FAILTARGET} --config ${CMAKE_CONFIGURATION} 2>&1)

if [ $? -eq 0 ]
then
  echo "Failed: Command exited with 0"
  exit 1
fi

printf "%s" "${output}"
exit 0
