#!/bin/bash
cd $(git rev-parse --show-toplevel)

export PATH=$PATH:$PWD/scripts

if [ -z "$TRAVIS_COMMIT_RANGE" -a -z "$GITHUB_SHA" ]; then
	echo "No commit range given"
	exit 0
fi

if ! type -p astyle.sh >/dev/null; then
	echo astyle.sh not found
	exit 1
fi

set -e

ASTYLEDIFF=/tmp/astyle.diff
>$ASTYLEDIFF

if [ ! -z $GITHUB_BASE_REF ] && [ ! -z $GITHUB_HEAD_REF ]; then
  # on a PR
  echo "GitHub PR COMMIT RANGE: ${GITHUB_BASE_REF}..${GITHUB_HEAD_REF}"
  git branch tmp_${GITHUB_BASE_REF} origin/${GITHUB_BASE_REF}
  BASE_SHA1=$(git rev-parse tmp_${GITHUB_BASE_REF})
  FILES=$(git diff --diff-filter=AMR --name-only ${BASE_SHA1}..${GITHUB_SHA} | tr '\n' ' ' )
elif [ ! -z  $GITHUB_SHA ]; then
  echo "GitHub push COMMIT $GITHUB_SHA"
  FILES=$(git diff --diff-filter=AMR --name-only ${GITHUB_SHA}~1..${GITHUB_SHA} | tr '\n' ' ' )
elif [ ! -z  $TRAVIS_PULL_REQUEST_BRANCH ]; then
  # if on a PR, just analyse the changed files
  echo "TRAVIS PR BRANCH: $TRAVIS_PULL_REQUEST_BRANCH"
  FILES=$(git diff --diff-filter=AMR --name-only $(git merge-base HEAD master) | tr '\n' ' ' )
elif [ ! -z  $TRAVIS_COMMIT_RANGE  ]; then
  echo "TRAVIS COMMIT RANGE: $TRAVIS_COMMIT_RANGE"
  FILES=$(git diff --diff-filter=AMR --name-only ${TRAVIS_COMMIT_RANGE/.../..} | tr '\n' ' ' )
fi

for f in $FILES; do
	if ! [ -f "$f" ]; then
		echo "$f was removed." >>/tmp/ctest-important.log
		continue
	fi

	echo "Checking $f" >>/tmp/ctest-important.log
	case "$f" in
	thirdparty*)
		echo "$f skipped"
		continue
		;;

	*.cpp|*.c|*.h|*.cxx|*.hxx|*.c++|*.h++|*.cc|*.hh|*.C|*.H|*.sip|*.py)
		;;

	*)
		continue
		;;
	esac

	m="$f.prepare"
	cp "$f" "$m"
	astyle.sh "$f"
	if diff -u "$m" "$f" >>$ASTYLEDIFF; then
		rm "$m"
	else
		echo "File $f needs indentation"
	fi
done

if [ -s "$ASTYLEDIFF" ]; then
	echo
	echo "Required indentation updates:"
	cat "$ASTYLEDIFF"

	cat <<EOF

Tips to prevent and resolve:
* Enable WITH_ASTYLE in your cmake configuration to format C++ code
* Install autopep8 (>= 1.2.1) to format python code
* Use "scripts/astyle.sh file" to fix the now badly indented files
* Consider using scripts/prepare-commit.sh as pre-commit hook to avoid this
  in the future (ln -s scripts/prepare-commit.sh .git/hooks/pre-commit) or
  run it manually before each commit.
EOF

	exit 1
fi
