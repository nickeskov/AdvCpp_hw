#!/bin/bash

static_check() {
  local olddir
  olddir="$(pwd)"

  local projdir="$1"
  cd "$projdir" || exit 1

  mkdir -p "build" && cd "build" || exit 1
  cmake .. -DENABLE_CPPCHECK=ON || exit 1
  cd ..

  local projname
  if [ "$2" != "" ]; then
    projname=$2
  else
    projname="$(pwd | rev | cut -d "/" -f 1 | rev)"
  fi

  cd "build" || exit 1
  make "$projname"_check || return $?

  cd "$olddir" || exit 1
}

static_check "$2" "hw"

exitstaus=$?

libsdir=$1
if [ -n "$libsdir" ]; then
  cd "$libsdir" || exit 1

  libs="$(ls -d ./*/ || exit 0)"

  for d in $libs; do
      static_check "$d"
      exitstaus=$((exitstaus + $?))
  done
fi

exit $exitstaus
