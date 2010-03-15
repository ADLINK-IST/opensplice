dim command_line_args
dim script_name
set command_line_args = wscript.Arguments
if command_line_args.count > 0 then
    script_name = command_line_args(0)
    Set WshShell = CreateObject("WScript.Shell")
    WshShell.Run Chr(34) & script_name & Chr(34), 0
    Set WshShell = Nothing
end if
