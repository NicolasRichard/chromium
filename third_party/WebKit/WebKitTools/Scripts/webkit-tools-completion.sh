# Copyright (C) 2009 Google Inc. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Command line completion for common commands used in WebKit development.
#
# Set-up:
#   Add a line like this to your .bashrc:
#     source /path/to/WebKitCode/WebKitTools/Scripts/webkit-tools-completion.sh

__webkit-patch_generate_reply()
{
    COMPREPLY=( $(compgen -W "$1" -- "${COMP_WORDS[COMP_CWORD]}") )
}

_webkit-patch_complete()
{
    local command current_command="${COMP_WORDS[1]}"
    case "$current_command" in
        -h|--help)
            command="help";
            ;;
        *)
            command="$current_command"
            ;;
    esac

    if [ $COMP_CWORD -eq 1 ]; then
        __webkit-patch_generate_reply "--help apply-from-bug bugs-to-commit commit-message land land-from-bug obsolete-attachments patches-to-commit post upload tree-status rollout reviewed-patches"
        return
    fi

    case "$command" in
        apply-from-bug)
            __webkit-patch_generate_reply "--force-clean --local-commit --no-clean --no-update"
            return
            ;;
        commit-message)
            return
            ;;
        land)
            __webkit-patch_generate_reply "--no-build --no-close --no-test --reviewer= -r"
            return
            ;;
        land-from-bug)
            __webkit-patch_generate_reply "--force-clean --no-build --no-clean --no-test"
            return
            ;;
        obsolete-attachments)
            return
            ;;
        post)
            __webkit-patch_generate_reply "--description --no-obsolete --no-review --request-commit -m --open-bug"
            return
            ;;
        upload)
            __webkit-patch_generate_reply "--description --no-obsolete --no-review --request-commit --cc -m --open-bug"
            return
            ;;
        post-commits)
            __webkit-patch_generate_reply "--bug-id= --no-comment --no-obsolete --no-review -b"
            return
            ;;
    esac
}

complete -F _webkit-patch_complete webkit-patch
complete -W "--continue --fix-merged --help --no-continue --no-warnings --warnings -c -f -h -w" resolve-ChangeLogs
complete -W "--bug --diff --git-commit --git-index --git-reviewer --help --no-update --no-write --open --update --write -d -h -o" prepare-ChangeLog
complete -W "--clean --debug --help -h" build-webkit
complete -o default -W "--add-platform-exceptions --complex-text --configuration --guard-malloc --help --http --ignore-tests --launch-safari --leaks --merge-leak-depth --new-test-results --no-http --no-launch-safari --no-new-test-results --no-sample-on-timeout --no-strip-editing-callbacks --pixel-tests --platform --port --quiet --random --reset-results --results-directory --reverse --root --sample-on-timeout --singly --skipped --slowest --strict --strip-editing-callbacks --threaded --timeout --tolerance --use-remote-links-to-tests --valgrind --verbose -1 -c -g -h -i -l -m -o -p -q -t -v" run-webkit-tests
