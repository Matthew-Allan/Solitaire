#!/bin/zsh

BIN="$1"
LIB="$2"

mkdir -p "$LIB"
rm -f "$LIB/*"


declare -A SEEN
Q=("$BIN")

canonicalise() {
    echo "$(cd "${1:h}" 2>/dev/null && pwd)/${1:t}"
}

resolve_path() {
    [[ "$1" != @* ]] && echo "$1" && return
    if [[ "$1" == @rpath/* ]]; then
        SUFFIX="${1#@rpath/}"
        for rpath in $(otool -l "$2" | grep -A2 "LC_RPATH" | grep "path" | awk '{print $2}'); do
            local candidate="${rpath/@loader_path/$(dirname "$2")}/$SUFFIX"
            [[ -f "$candidate" ]] && { echo "$candidate"; return; }
        done
    fi
    [[ "$1" == @loader_path/* ]] && { echo "$(dirname "$2")/${1#@loader_path/}"; return; }
    echo "$1"
}

while (( ${#Q[@]} > 0 )); do
    DEP=${Q[1]}
    [[ "$DEP" == "$1" ]] && LIB_DEP="$DEP" || LIB_DEP="$LIB/${DEP:t}"
    Q=("${Q[@]:1}")
    local NAME="$(otool -D "$DEP" | tail -n 1)"
    while read -r RAW_LINE; do
        LINE="$(resolve_path "$RAW_LINE" "$DEP")"
        [[ "$LINE" == /System/* || "$LINE" == /usr/lib/* || "$LINE" == "$NAME" ]] && continue
        local ABS_PATH="$(canonicalise "$LINE")"
        if [[ ! -n "${SEEN["$ABS_PATH"]}" ]]; then
            SEEN["$ABS_PATH"]=1
            cp "$LINE" "$LIB"
            Q+=("$LINE")
            install_name_tool -id "@executable_path/../Frameworks/${LINE:t}" "$LIB/${LINE:t}"
        fi
        
        install_name_tool -change "$RAW_LINE" "@executable_path/../Frameworks/${LINE:t}" "$LIB_DEP"
        
    done < <(otool -L "$DEP" | tail -n +2 | awk '{print $1}')
done