#!/usr/bin/env bash

# Compose images into atlas
atlas_cache_compose() {
	local W H
	W=$(< "$CACHE/width")
	H=$(< "$CACHE/height")

	# MIN_SIZE is not a misspelling
	# shellcheck disable=SC2153
	(( W < MIN_SIZE )) && W=$MIN_SIZE
	# shellcheck disable=SC2153
	(( H < MIN_SIZE )) && H=$MIN_SIZE

	# don't quote $CACHE/args because this should be word splitted
	# shellcheck disable=SC2046
	[ -f "$CACHE/args" ] && convert \
		-size "${W}x${H}" \
		xc:transparent \
		$(< "$CACHE/args") \
		-strip \
		"${ATLAS:-atlas.png}"
}

# Print JSON and build arguments for convert
atlas_cache_summarize() {
	echo "var atlas={"

	find "$CACHE" -type f -name image | while read -r
	do
		local X Y W H FILE
		read -r X Y W H FILE < "$REPLY"

		echo "$FILE -geometry +$X+$Y -composite" >> "$CACHE/args"

		if (( MARGIN > 0 ))
		then
			(( X += MARGIN ))
			(( Y += MARGIN ))
			(( W -= MARGIN*2 ))
			(( H -= MARGIN*2 ))
		fi

		local NAME=${FILE##*/}
		echo "${NAME%.*}:{x:$X,y:$Y,w:$W,h:$H},"
	done

	echo "};"
}

# Insert image, packing algorithm from
# http://www.blackpawn.com/texts/lightmaps/default.html
#
# @param 1 - image width
# @param 2 - image height
# @param 3 - image file
atlas_cache_insert() {
	[ -f "$CACHE/candidates" ] || return 1

	local INDEX TARGET
	# INDEX is unused but required to consume the first argument
	# shellcheck disable=SC2034
	while read -r INDEX TARGET
	do
		break
	done <<< "$(sort "$CACHE/candidates")"

	[ -f "$TARGET/rect" ] || return 1

	local X Y W H
	read -r X Y W H < "$TARGET/rect"

	if (( W < $1 )) || (( H < $2 ))
	then
		return 1
	fi

	mkdir "$TARGET/child0" "$TARGET/child1" || return $?

	local RW=$(( W-$1 ))
	local RH=$(( H-$2 ))

	if (( RW > RH ))
	then
		# +-------+---+
		# | image |   |
		# +-------+   |
		# |       | r |
		# |   b   |   |
		# |       |   |
		# +-------+---+
		echo $(( X+$1 )) "$Y" "$RW" "$H" > "$TARGET/child0/rect"
		echo "$X" $(( Y+$2 )) "$1" "$RH" > "$TARGET/child1/rect"
	else
		# +-------+---+
		# | image | r |
		# +-------+---+
		# |           |
		# |     b     |
		# |           |
		# +-----------+
		echo $(( X+$1 )) "$Y" "$RW" "$2" > "$TARGET/child0/rect"
		echo "$X" $(( Y+$2 )) "$W" "$RH" > "$TARGET/child1/rect"
	fi

	local RIGHT=$(( X+$1 ))
	(( RIGHT > $(< "$CACHE/width") )) && echo "$RIGHT" > "$CACHE/width"

	local BOTTOM=$(( Y+$2 ))
	(( BOTTOM > $(< "$CACHE/height") )) && echo "$BOTTOM" > "$CACHE/height"

	echo "$X" "$Y" "$1" "$2" "$3" > "$TARGET/image"
}

# Find possible candidates and give them a sort index
#
# @param 1 - image width
# @param 2 - image height
# @param 3 - image file
atlas_cache_find_nodes() {
	if [ -d "$NODE/child0" ]
	then
		NODE="$NODE/child0" atlas_cache_find_nodes "$1" "$2" "$3"
		NODE="$NODE/child1" atlas_cache_find_nodes "$1" "$2" "$3"

		return
	fi

	[ -f "$NODE/rect" ] || return 1

	local X Y W H
	read -r X Y W H < "$NODE/rect"

	if (( W < $1 )) || (( H < $2 ))
	then
		return
	fi

	local MAX_WIDTH
	MAX_WIDTH=$(< "$CACHE/width")
	local MAX_HEIGHT
	MAX_HEIGHT=$(< "$CACHE/height")
	local RIGHT=$(( X+$1 ))
	local BOTTOM=$(( Y+$2 ))

	(( RIGHT > MAX_WIDTH )) && MAX_WIDTH=$RIGHT
	(( BOTTOM > MAX_HEIGHT )) && MAX_HEIGHT=$BOTTOM

	printf '%08d %s\n' \
		$(( MAX_WIDTH+MAX_HEIGHT )) \
		"$NODE" >> "$CACHE/candidates"
}

# Sort files and insert them into the atlas
atlas_cache_compile() {
	local NODE=$CACHE
	local W H FILE

	# MAX is unused but required to consume the sorting index
	# shellcheck disable=SC2034
	while read -r W H FILE
	do
		[ "$FILE" ] || continue

		printf '%08d %d %d %s\n' $(( W+H )) "$W" "$H" "$FILE"
	done | sort -r | while read -r MAX W H FILE
	do
		rm -f "$CACHE/candidates"

		if ! atlas_cache_find_nodes "$W" "$H" "$FILE" ||
			! atlas_cache_insert "$W" "$H" "$FILE"
		then
			echo "error: cannot insert $FILE" >&2
			return 1
		fi
	done
}

# Read 'width height file' from standard input and create atlas
atlas_create_from_list() {
	local CACHE
	CACHE=$(mktemp -d "${0##*/}.XXXXXXXXXX") || return $?

	local MAX_SIZE=${MAX_SIZE:-2048}
	echo 0 0 "$MAX_SIZE" "$MAX_SIZE" > "$CACHE/rect"

	echo 0 > "$CACHE/width"
	echo 0 > "$CACHE/height"

	atlas_cache_compile &&
		atlas_cache_summarize &&
		atlas_cache_compose

	rm -rf "$CACHE"
}

# Create texture atlas from given image files
#
# @param ... - image files
atlas_create() {
	local INKSCAPE=${INKSCAPE:-$(which inkscape)}
	local TMPDIR
	TMPDIR=$(mktemp -d "${0##*/}.XXXXXXXXXX") || return $?

	# prepare source files
	local SRC
	for SRC
	do
		local COPY="$TMPDIR/${SRC##*/}" BORDER=

		case ${SRC##*.} in
			svg)
				COPY="${COPY%.*}.png"

				# use inkscape if available
				[ "$INKSCAPE" ] &&
					$INKSCAPE "$SRC" \
						-z -e "$COPY" &>/dev/null &&
					SRC=${COPY}
				;;
		esac

		if (( MARGIN > 0 ))
		then
			# title_* should do glob matching
			# shellcheck disable=SC2053
			if [[ ${SRC##*/} == ${EXPAND:-tile_*} ]]
			then
				local W
				W=$(identify -format '%w,%h' "$SRC")
				H=$(( MARGIN*2+${W#*,} ))
				W=$(( MARGIN*2+${W%,*} ))
				BORDER="-set option:distort:viewport \
					${W}x${H}-${MARGIN}-${MARGIN} \
					-virtual-pixel Mirror \
					-filter point \
					-distort SRT 0 \
					+repage"
			else
				BORDER="-border $MARGIN"
			fi
		fi

		# BORDER cannot be quoted because it will result in an
		# empty argument which makes convert stall
		# shellcheck disable=SC2086
		convert \
			-background none \
			"$SRC" \
			-bordercolor none -border 3x3 \
			-trim \
			$BORDER \
			"$COPY"
	done

	identify -format '%w %h %d/%f\n' "$TMPDIR/"* | atlas_create_from_list
	rm -rf "$TMPDIR"
}

readonly MARGIN=${MARGIN:-0}

if [ "${BASH_SOURCE[0]}" == "$0" ]
then
	atlas_create "$@"
fi
