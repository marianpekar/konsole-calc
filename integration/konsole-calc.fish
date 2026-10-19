function =
    konsole-calc $argv
end

if status is-interactive
    function __konsole_calc_enter
        set -l cmd (commandline)
        set -l expr

        if string match -q '=*' -- $cmd
            set expr (string replace -r '^=\s*' '' -- $cmd)
        else if string match -qr '^(?=.*[\d$])(?=.*[-+*/^%])(?:[\d\s.+\-*/^()%\x27"]|\$\w+)+$' -- $cmd
            set expr $cmd
        end

        if set -q expr[1]
            set expr (string trim -c "'\"" -- $expr)
            commandline -r -- "= \"$expr\""
        end
        commandline -f execute
    end

    bind \r __konsole_calc_enter
end