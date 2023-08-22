#!/bin/bash

# Tool for showing submodule history vs. associated and currently checked-out commits

module="data"
tail_count=5

branch_name="$(git symbolic-ref HEAD 2>/dev/null)"

if [ -z "$branch_name" ]; then
    display_branch_name="(detached head)"
else
    display_branch_name="$branch_name"
fi

module_branch_name="$(cd $module && git symbolic-ref HEAD 2>/dev/null)"
module_working_commit="$(cd $module && git rev-parse HEAD)"
module_assoc_commit="$(git ls-tree $branch_name $module | awk '{print $3}' -)"

if [ -z "$module_branch_name" ]; then
    module_assoc_branches=($(cd $module && git branch --contains $module_assoc_commit | awk 'NR > 1 { print $1 }'))
    if [ ${#module_assoc_branches[@]} -ne 1 ]; then
        echo "$module module is in detached head, but referenced commit is in multiple branches"
        echo "Sorry, can't help you"
        exit 1
    else
        module_branch_name="${module_assoc_branches[0]}"
        display_module_branch_name="(detached head, detected: $module_branch_name)"
    fi
else
    display_module_branch_name="$module_branch_name"
fi

echo -e "Repository branch: \033[32m$display_branch_name\033[0m"
echo -e "$module module branch: \033[32m$display_module_branch_name\033[0m"

if [ -z "$(cd $module && git rev-list HEAD | grep $module_assoc_commit)" ]; then
    echo -e "$module module associated commit: \033[33m$module_assoc_commit\033[0m is not in module history!"
    echo "You probably have checked out different branch!"
    exit 1
fi

echo ""

i=0
c=0
h=1
(cd $module && git log --format='%H%x01%h%x01%s%x00' $module_branch_name) | while read -d $'\0' info; do
    commit=$(echo "$info" | cut -d$'\1' -f1)
    short_commit=$(echo "$info" | cut -d$'\1' -f2)
    message=$(echo "$info" | cut -d$'\1' -f3)

    if [ $h -eq 1 ]; then
        echo -n -e "\033[34m H \033[0m"
    else
        echo -n "   "
    fi

    h=0

    if [ "$commit" == "$module_working_commit" ]; then
        echo -n -e "\033[31m * \033[0m"
        c=$(($c+1))
    else
        echo -n "   "
    fi

    if [ "$commit" == "$module_assoc_commit" ]; then
        echo -n -e "\033[32m x \033[0m"
        c=$(($c+1))
    else
        echo -n "   "
    fi

    echo -e "\033[33m$short_commit\033[0m $message"

    if [ $c -eq 2 ]; then
        i=$(($i+1))
        if [ $i -gt $tail_count ]; then
            echo "         ..."
            break
        fi
    fi
done

echo ""
echo -e "\033[34m H \033[0m -- $module_branch_name HEAD"
echo -e "\033[31m * \033[0m -- checked-out commit"
echo -e "\033[32m x \033[0m -- associated commit"

